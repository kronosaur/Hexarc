//	Mnemosynth.h
//
//	Mnemosynth Archon Implementation
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by ArchonEngine.h

#pragma once

using MnemosynthSequence = DWORD;
static constexpr MnemosynthSequence NULL_MNEMO_SEQ = 0xffffffff;

//	Mnemosynth

struct SEntryDelta
	{
	DWORD dwCollection;
	CString sKey;
	CDatum dValue;

	MnemosynthSequence dwSequence;
	};

struct SEndpointDelta
	{
	CString sName;
	bool bCentralModule;
	bool bLocalMachine;
	MnemosynthSequence dwSeqRecv;
	MnemosynthSequence dwSeqSent;

	TArray<SEntryDelta> Entries;
	};

struct SMnemosynthDelta
	{
	TArray<CString> Collections;
	TArray<SEndpointDelta> Endpoints;
	};

struct SMnemosynthUpdate
	{
	CString sDestEndpoint;						//	Endpoint that we want to update (destination)
	TArray<CDatum> Payloads;					//	Array of payloads (one for each endpoint who updated)
												//		Each payload:
												//			collections : {array of collection names}
												//			endpoint: {name of endpoint that updated data}
												//			entries: {array of entries}
												//				{each entry}
												//					0: collection index
												//					1: key
												//					2: value
												//					3: sequence
	};

struct SEndpointWatermark
	{
	CString sEndpoint;
	DWORD dwEndpointID;
	MnemosynthSequence dwSeq;
	};

typedef TArray<SEndpointWatermark> CMnemosynthWatermark;

class CMnemosynthDb
	{
	public:
		DWORD AddEndpoint (const CString &sName, DWORD dwProcessID, bool bSyncNow = false);
		void Boot (IArchonProcessCtx *pProcess);
		void DebugDump (IByteStream &Stream);
		void Delete (const CString &sCollection, const CString &sKey);
		void GenerateDelta (TArray<SMnemosynthUpdate> *retUpdates, CDatum *retdLocalUpdates);
		void GenerateEndpointList (CDatum *retdList);
		SequenceNumber GetDbSeq () const { return m_Seq; }
		DWORD GetEndpointID (const CString &sEndpoint);
		const CString &GetEndpointName (DWORD dwEndpointID);
		inline CManualEvent &GetModifiedEvent (void) { return m_ModifiedEvent; }
		MnemosynthSequence GetSequence (DWORD dwEndpointID);
		MnemosynthSequence GetSequence (const CString &sEndpoint);
		MnemosynthSequence GetSequence (void) { CSmartLock Lock(m_cs); return m_Endpoints[0].dwSeqRecv; }
		void GetWatermark (CMnemosynthWatermark *retWatermark);
		void IncorporateDelta (CDatum dPayload);
		void Mark (void);
		CDatum Read (const CString &sCollection, const CString &sKey) const;
		void ReadCollection (const CString &sCollection, TArray<CString> *retKeys) const;
		void ReadCollection (const CString &sCollection, TSortMap<CString, CDatum> &retCollection) const;
		void ReadCollectionList (TArray<CString> *retCollections) const;
		void RemoveEndpoint (const CString &sName);
		void RemoveMachineEndpoints (const CString &sName);
		void Write (const CString &sCollection, const CString &sKey, CDatum dValue);

		static CString GenerateEndpointName (const CString &sMachine, const CString &sModule);
		static void ParseEndpointName (const CString &sEndpoint, CString *retsMachine, CString *retsModule = NULL);

	private:
		struct SEntry
			{
			DWORD dwOwnerID;					//	ID of endpoint that last wrote this
			MnemosynthSequence dwSequence;		//	Sequence number relative to owner
			CDatum dValue;
			};

		struct SCollection
			{
			TSortMap<CString, SEntry> Entries;
			};

		struct SEndpoint
			{
			DWORD dwID;							//	Endpoint ID
			CString sName;						//	Name of endpoint "machineName/moduleName"
			bool bCentralModule;				//	TRUE if this is a CentralModule (for some machine)
			bool bLocalMachine;					//	TRUE if this is an endpoint on our machine
			bool bFullUpdate;					//	TRUE if we need to send this endpoint a full update
			DWORD dwProcessID;					//	If local machine, this is the process ID of the endpoint
			IArchonMessagePort *pPort;			//	Port to send messages to this endpoint
			MnemosynthSequence dwSeqRecv;		//	We've assimilated up to this sequence for this endpoint
			MnemosynthSequence dwSeqSent;		//	We've sent out up to this sequence for this endpoint
			};

		void DeleteEntry (const CString &sCollection, const CString &sKey);
		CDatum GenerateCollectionsArray (void);
		CDatum GenerateEntry (int iCollectionIndex, const CString &sKey, SEntry *pEntry);
		SEndpoint &GetLocalEndpoint (void) { return m_Endpoints[0]; }
		MnemosynthSequence GetNextSequence (void);
		SEndpoint *GetOrAddEndpoint (const CString &sName, DWORD dwProcessID);
		SEntry *GetWriteEntry (const CString &sCollection, const CString &sKey);
		int FindEndpointIndex (DWORD dwID);
		int FindEndpointIndex (const CString &sName);
		SEndpoint *FindEndpoint (DWORD dwID);
		SEndpoint *FindEndpoint (const CString &sName);

		mutable CCriticalSection m_cs;
		IArchonProcessCtx *m_pProcess;			//	Process interface
		TSortMap<DWORD, SEndpoint> m_Endpoints;	//	List of endpoints in the arcology (0 is always us)
		DWORD m_dwNextID;						//	Next endpoint ID.

		TSortMap<CString, SCollection> m_Collections;	//	Collections

		SequenceNumber m_Seq = 1;				//	Db sequence number
		CManualEvent m_ModifiedEvent;			//	Signalled when db is modified
	};

class CMnemosynthEngine : public CSimpleEngine
	{
	public:
		CMnemosynthEngine (CMnemosynthDb *pDb);

	protected:
		virtual void OnBoot (void);
		virtual void OnProcessMessages (CArchonMessageList &List);
		virtual void OnStartRunning (void);

	private:
		struct SEndpointSequence
			{
			CString sEndpoint;
			DWORD dwEndpointID;
			MnemosynthSequence dwSeqToCheck;	//	We wait until the endpoint gets to this sequence at least
			MnemosynthSequence dwSeqActual;		//	This is the actual sequence (or 0 if not yet updated)
			};

		struct SLocalCheckpoint
			{
			CString sAddress;					//	Address to notify
			CString sMsg;
			DWORD dwTicket;
			TArray<SEndpointSequence> Endpoints;
			};

		struct SArcologyCheckpoint
			{
			DWORD dwID;
			CString sAddress;
			CString sMsg;
			DWORD dwTicket;
			TArray<SEndpointSequence> Endpoints;
			};

		bool IsNotifyRequestUpToDate (SLocalCheckpoint *pEntry);
		void MsgEndpointList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRead (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void NotifyOnArcologyUpdate (const CString &sAddress, DWORD dwTicket, CDatum dPayload);
		void NotifyOnEndpointUpdate (const CString &sAddress, DWORD dwTicket, CDatum dPayload);
		void NotifyOnModified (CDatum dUpdates, bool bLocalUpdate);
		void OnDbModified (void);
		void OnEndpointUpdated (const CString &sReplyAddr, DWORD dwTicket, CDatum dWatermark);
		void ProcessMessage (const SArchonMessage &Msg);
		void ProcessNotifyRequests (void);
		void SendOnEndpointUpdated (SLocalCheckpoint *pEntry);

		CMnemosynthDb *m_pDb;

		CCriticalSection m_cs;
		TArray<SLocalCheckpoint> m_LocalCheckpoints;

		DWORD m_dwNextID;
		TArray<SArcologyCheckpoint> m_ArcologyCheckpoints;
		bool m_bDebug;
	};
