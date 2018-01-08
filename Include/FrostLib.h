//	FrostLib.h
//
//	Game AI Classes and Utilities
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	1. Requires Foundation
//	2. Requires AEON
//	3. Include FrostLib.h
//	4. Link with FrostLib.lib

#pragma once

#include "Foundation.h"
#include "AEON.h"

#define TRY						try {
#define CATCH_AND_THROW(sMsg)	} catch (...) { throw CException(errFail, (sMsg)); }

class CBlackboard;
class IBBAgent;
struct SBBEntry;

enum EBBCategories
	{
	bbcatNone,

	bbcatEvent,								//	An event entry
	bbcatMissionOpen,						//	An open (not yet assigned) mission
	bbcatMissionAssigned,					//	An assigned mission

	bbcatDeleted,							//	Deleted mission
	};

struct SBBLoadCtx
	{
	SBBLoadCtx (CBlackboard &BBArg, IByteStream &StreamArg) :
			BB(BBArg),
			Stream(StreamArg),
			pCtx(NULL)
		{ }

	inline IBBAgent *ReadAgentRef (void);
	inline double ReadDouble (void) { double rValue; Stream.ReadChecked(&rValue, sizeof(double)); return rValue; }
	inline DWORD ReadDWORD (void) { DWORD dwValue; Stream.ReadChecked(&dwValue, sizeof(DWORD)); return dwValue; }
	inline SBBEntry *ReadEntryRef (void);
	inline void ReadString (CString *retsString) { retsString->TakeHandoff(CString::Deserialize(Stream)); }

	CBlackboard &BB;
	IByteStream &Stream;
	void *pCtx;

	TSortMap<DWORD, IBBAgent *> AgentMap;
	};

struct SBBSaveCtx
	{
	SBBSaveCtx (CBlackboard &BBArg, IByteStream &StreamArg) :
			BB(BBArg),
			Stream(StreamArg),
			pCtx(NULL)
		{ }

	inline void Write (int iValue) { Write((DWORD)iValue); }
	inline void Write (DWORD dwValue) { Stream.Write(&dwValue, sizeof(DWORD)); }
	inline void Write (double rValue) { Stream.Write(&rValue, sizeof(double)); }
	inline void Write (const CString &sValue) { sValue.Serialize(Stream); }
	inline void WriteEntryRef (SBBEntry *pEntry);
	inline void WriteAgentRef (IBBAgent *pAgent);

	CBlackboard &BB;
	IByteStream &Stream;
	void *pCtx;
	};

struct SBBEntry
	{
	SBBEntry (void) :
			dwID(0),
			rPriority(0.0),
			iCategory(bbcatNone),
			pOwner(NULL),
			dwExpiresOn(0),
			rMinCapacity(0.0),
			rCapacity(0.0),
#ifdef DEBUG
			pBB(NULL),
#endif
			pNext(NULL)
		{ }

	~SBBEntry (void);

	inline void AssignAgent (IBBAgent *pAgent) { ASSERT(pAgent); m_Agents.Insert(pAgent); }

	inline IBBAgent *GetAssignedAgent (int iIndex) { return m_Agents[iIndex]; }
	inline int GetAssignedAgentsCount (void) const { return m_Agents.GetCount(); }
	inline void ReadAgents (SBBLoadCtx &Ctx);
	inline bool UnassignAgent (IBBAgent *pAgent) { return (m_Agents.DeleteValue(pAgent) > 0); }
	inline void UnassignAllAgents (void) { m_Agents.DeleteAll(); }
	inline void WriteAgents (SBBSaveCtx &Ctx);

	DWORD dwID;								//	ID of entry
	CString sType;							//	Entry type
	double rPriority;						//	Priority (or strength for events)
	CDatum dData;							//	data (by type)

	EBBCategories iCategory;				//	Entry category
	IBBAgent *pOwner;						//	Agent who owns entry (may be NULL)
	DWORD dwExpiresOn;						//	Tick on which we expire

	double rMinCapacity;					//	Assignment fails if we don't get this much capacity
	double rCapacity;						//	Desired capacity for mission

	SBBEntry *pNext;						//	Next entry

#ifdef DEBUG
	CBlackboard *pBB;
#endif

	private:
		TArray<IBBAgent *> m_Agents;		//	Agents assigned to this mission
	};

class CBBMap
	{
	public:
		CBBMap (void) : m_iCount(0)
			{ }

		void AccumulateDebugInfo (CComplexArray *pArray) const;
		void AccumulateEntries (const CString &sType, TArray<SBBEntry *> *retResult);
		void AddEntry (SBBEntry *pEntry);
		void DeleteAllEntries (void);
		void DeleteExpiredEntries (DWORD dwTick, TArray<SBBEntry *> *retResult = NULL);
		bool FindEntry (DWORD dwID, SBBEntry **retpEntry = NULL) const;
		inline int GetEntryCount (void) const { return m_iCount; }
		SBBEntry *GetFirstEntry (const CString &sType) const;
		void Mark (void);
		void Read (SBBLoadCtx &Ctx);
		void RemoveAgent (IBBAgent *pAgent);
		bool RemoveEntry (SBBEntry *pEntry);
		void Write (SBBSaveCtx &Ctx);

	private:
		CDatum GetDebugInfo (SBBEntry *pEntry) const;

		TSortMap<CString, SBBEntry *> m_Entries;
		int m_iCount;
	};

class CBBApplicationList
	{
	public:
		CBBApplicationList (void) : m_ByPriority(DescendingSort)
			{ }

		void AddApplication (SBBEntry *pMission, IBBAgent *pAgent, double rProficiency, double rCapacity);
		inline SBBEntry *GetMission (int iIndex) { return m_ByPriority[iIndex]; }
		inline int GetMissionCount (void) const { return m_ByPriority.GetCount(); }
		bool GetWinningApplications (int iIndex, TArray<IBBAgent *> *retAgents);

	private:
		struct SApplication
			{
			SBBEntry *pMission;				//	Mission
			IBBAgent *pAgent;				//	Agent applying
			double rProficiency;			//	Agent's proficiency mission
			double rCapacity;				//	Agent's capacity
			};

		TSortMap<SBBEntry *, TSortMap<double, SApplication>> m_Apps;
		TSortMap<double, SBBEntry *> m_ByPriority;	//	High to low
	};

class IBBAgentFactory
	{
	public:
		virtual IBBAgent *CreateAgentStub (SBBLoadCtx &Ctx, DWORD dwAgentID, DWORD dwClassID) = 0;
	};

class IBBAgent
	{
	public:
		IBBAgent (void) : m_iMissionsLeft(0) { }
		virtual ~IBBAgent (void) { }

		//	For all agents

		inline void GatherData (CBlackboard &B) { OnGatherData(B); }
		inline void Execute (CBlackboard &B) { OnExecute(B); }
		inline SBBEntry *GetCurrentMission (void) const { return OnGetCurrentMission(); }
		inline DWORD GetAgentID (void) const { return m_dwID; }
		inline CString GetAgentName (void) const { return OnGetAgentName(); }
		inline DWORD GetClassID (void) const { return OnGetClassID(); }
		inline double MessageSent (IBBAgent *pSender, const CString &sMsg, CDatum dData, CDatum *retResponse) { return OnMessageSent(pSender, sMsg, dData, retResponse); }
		void Read (SBBLoadCtx &Ctx);
		inline void SetAgentID (DWORD dwID) { m_dwID = dwID; }
		void Write (SBBSaveCtx &Ctx);

		//	For agents assigning missions

		inline void MissionAbandoned (CBlackboard &B, SBBEntry *pMission) { OnMissionAbandoned(B, pMission); }
		inline void MissionAssigned (CBlackboard &B, SBBEntry *pMission, const TArray<IBBAgent *> &Agents) { OnMissionAssigned(B, pMission, Agents); }
		inline void MissionCompleted (CBlackboard &B, SBBEntry *pMission) { OnMissionCompleted(B, pMission, true); }
		inline void MissionFailed (CBlackboard &B, SBBEntry *pMission) { OnMissionCompleted(B, pMission, false); }

		//	For agents executing missions

		inline int GetMissionSlotsLeft (void) { return m_iMissionsLeft; }
		inline bool HasMissionSlotsLeft (void) { return (m_iMissionsLeft > 0); }
		inline void RequestMissions (CBlackboard &B, const CBBMap &OpenMissions, CBBApplicationList &Requests) { ResetMissionSlots(); OnRequestMissions(B, OpenMissions, Requests); }
		inline void SetMissionSlotsLeft (int iMissions) { m_iMissionsLeft = iMissions; }
		inline void SetCurrentMission (CBlackboard &B, SBBEntry *pMission) { if (pMission) m_iMissionsLeft--; else m_iMissionsLeft++; OnSetCurrentMission(B, pMission); }

	protected:
		virtual void OnAgentRead (SBBLoadCtx &Ctx) { }
		virtual void OnAgentWrite (SBBSaveCtx &Ctx) { }
		virtual void OnGatherData (CBlackboard &B) { }
		virtual CString OnGetAgentName (void) const { return NULL_STR; }
		virtual DWORD OnGetClassID (void) const = 0;
		virtual SBBEntry *OnGetCurrentMission (void) const { return NULL; }
		virtual int OnGetMissionSlots (void) { return 1; }
		virtual double OnMessageSent (IBBAgent *pSender, const CString &sMsg, CDatum dData, CDatum *retResponse) { return 0.0; }
		virtual void OnMissionAbandoned (CBlackboard &B, SBBEntry *pMission) { }
		virtual void OnMissionAssigned (CBlackboard &B, SBBEntry *pMission, const TArray<IBBAgent *> &Agents) { }
		virtual void OnMissionCompleted (CBlackboard &B, SBBEntry *pMission, bool bSuccess) { }
		virtual void OnExecute (CBlackboard &B) { }
		virtual void OnRequestMissions (CBlackboard &B, const CBBMap &OpenMissions, CBBApplicationList &Requests) { }
		virtual void OnSetCurrentMission (CBlackboard &B, SBBEntry *pMission) { }

	private:
		inline int GetMissionSlots (void) { return OnGetMissionSlots(); }
		inline void ResetMissionSlots (void) { m_iMissionsLeft = GetMissionSlots(); }

		int m_iMissionsLeft;
		DWORD m_dwID;
	};

class CBlackboard
	{
	public:
		CBlackboard (void) : 
				m_dwTick(1),
				m_dwNextID(1),
				m_pCtx(NULL),
				m_bInAssignment(false)
			{ }

		~CBlackboard (void);

		void AbandonMission (SBBEntry *pMission, bool bNotifyOwner);
		void AbortMission (SBBEntry *pMission, bool bNotifyOwner);
		void AddAgent (IBBAgent *pAgent, bool bFree = true);
		void AddEvent (IBBAgent *pOwner, const CString &sType, CDatum dData, DWORD dwLifetime);
		void AddMission (IBBAgent *pOwner, 
						 const CString &sType, 
						 double rPriority,
						 double rCapacity, 
						 double rMinCapacity, 
						 CDatum dData, 
						 DWORD dwTimeout, 
						 SBBEntry **retpMission = NULL);
		void CompleteMission (SBBEntry *pMission, bool bNotifyOwner);
		void CompleteMissionPartial (SBBEntry *pMission, IBBAgent *pAgent, bool bNotifyOwner);
		inline void ClearData (void) { m_Data.DeleteAll(); }
		void DeleteAgent (IBBAgent *pAgent);
		bool FindAgent (DWORD dwAgentID, IBBAgent **retpAgent = NULL);
		bool FindAgent (IBBAgent *pAgent);
		bool FindEntry (DWORD dwID, SBBEntry **retpEntry = NULL);
		IBBAgent *GetAgent (int iIndex) const { return m_Agents[iIndex].pAgent; }
		inline int GetAgentCount (void) const { return m_Agents.GetCount(); }
		inline void *GetCtx (void) { return m_pCtx; }
		CDatum GetData (const CString &sField) const;
		CDatum GetDebugInfo (void) const;
		SBBEntry *GetFirstEvent (const CString &sType) const { return m_Events.GetFirstEntry(sType); }
		SBBEntry *GetFirstActiveMission (const CString &sType) const { return m_ActiveMissions.GetFirstEntry(sType); }
		SBBEntry *GetFirstOpenMission (const CString &sType) const { return m_OpenMissions.GetFirstEntry(sType); }
		CDatum IncData (const CString &sField, CDatum dData);
		void Mark (void);
		void Read (IByteStream &Stream, IBBAgentFactory *pFactory, void *pCtx);
		CDatum SendMessage (IBBAgent *pSender, const CString &sMsg, CDatum dData);
		void SetData (const CString &sField, CDatum dData);
		void Update (void *pCtx);
		void UpdateData (void *pCtx);
		void Write (IByteStream &Stream, void *pCtx);

	private:
		struct SAgentDossier
			{
			IBBAgent *pAgent;
			bool bFree;						//	If TRUE, we own the agent
			};

		struct SApplication
			{
			int iAgentIndex;				//	Agent requesting assignment
			double rCapacity;				//	Agent capacity
			};

		DWORD m_dwTick;						//	Current tick
		TArray<SAgentDossier> m_Agents;		//	List of agents
		CBBMap m_Events;					//	Current events
		CBBMap m_ActiveMissions;			//	Assigned missions
		CBBMap m_OpenMissions;				//	Open (not assigned) missions
		TSortMap<CString, CDatum> m_Data;	//	Opaque data
		DWORD m_dwNextID;					//	Next agent ID

		void *m_pCtx;						//	Temp inside of Update
		TArray<SBBEntry *> m_DeletedMissions;	//	Temp while inside of Update
		bool m_bInAssignment;				//	Temp
	};

class CBBMissionAssigner
	{
	public:
		CBBMissionAssigner (void) :
				m_pAssigner(NULL)
			{ }

		void AbortAll (CBlackboard &B);
		void AddMission (CBlackboard &B,
						 const CString &sType, 
						 double rPriority,
						 double rCapacity, 
						 double rMinCapacity, 
						 CDatum dData, 
						 DWORD dwTimeout);
		inline bool HasMissions (void) const { return (m_Missions.GetCount() > 0); }
		bool IsInProgress (void) const;
		void OnMissionCompleted (CBlackboard &B, SBBEntry *pMission);
		void Read (SBBLoadCtx &Ctx);
		inline void SetAssigner (IBBAgent *pAgent) { m_pAssigner = pAgent; }
		void Write (SBBSaveCtx &Ctx);

	private:
		IBBAgent *m_pAssigner;				//	Agent who is assigning missions
		TArray<SBBEntry *> m_Missions;		//	List of missions that we've assigned
	};

inline IBBAgent *SBBLoadCtx::ReadAgentRef (void)
	{
	DWORD dwID = ReadDWORD();
	if (dwID)
		{
		IBBAgent *pAgent;
		if (!AgentMap.Find(dwID, &pAgent))
			{
			//	LATER: Bug with agents not being saved.
			return NULL;
			//throw CException(errFail, strPattern("Unknown agent ID: %x.", dwID));
			}
		return pAgent;
		}
	else
		return NULL;
	}

inline SBBEntry *SBBLoadCtx::ReadEntryRef (void)
	{
	DWORD dwID = ReadDWORD();
	if (dwID)
		{
		SBBEntry *pEntry;
		if (!BB.FindEntry(dwID, &pEntry))
			throw CException(errFail, strPattern("Unknown entry ID: %x.", dwID));
		return pEntry;
		}
	else
		return NULL;
	}

inline void SBBSaveCtx::WriteAgentRef (IBBAgent *pAgent) { if (pAgent) Write(pAgent->GetAgentID()); else Write(0); }
inline void SBBSaveCtx::WriteEntryRef (SBBEntry *pEntry) { if (pEntry) Write(pEntry->dwID); else Write(0); }

inline SBBEntry::~SBBEntry (void)
	{
	ASSERT(pOwner == NULL || pOwner->GetCurrentMission() != this);
	}

inline void SBBEntry::ReadAgents (SBBLoadCtx &Ctx)
	{
	int i;

	int iCount = Ctx.ReadDWORD();
	m_Agents.InsertEmpty(iCount);
	for (i = 0; i < iCount; i++)
		m_Agents[i] = Ctx.ReadAgentRef();
	}

inline void SBBEntry::WriteAgents (SBBSaveCtx &Ctx)
	{
	int i;

	Ctx.Write(m_Agents.GetCount());
	for (i = 0; i < m_Agents.GetCount(); i++)
		Ctx.Write(m_Agents[i]->GetAgentID());
	}
