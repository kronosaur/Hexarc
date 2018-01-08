//	ArchonUtilities.h
//
//	Archon helper classes
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by ArchonEngine.h

#pragma once

class CMessagePort;
class CMessageTransporter;

//	CMessagePortMap ------------------------------------------------------------

class CMessagePortMap
	{
	public:
		enum Flags
			{
			flagProcessOnly =			0x00000001,
			flagMachineOnly =			0x00000002,

			flagGlobalPorts =			0x00000100,
			flagMachineWidePorts =		0x00000200,
			flagProcessWidePorts =		0x00000400,
			};

		CMessagePortMap (void);

		void AddPort (LPSTR sName, IArchonMessagePort *pPort, DWORD dwFlags);
		void GetPorts (LPSTR sName, DWORD dwFlags, TArray<IArchonMessagePort *> *retResult);

	private:
		struct SPortDesc
			{
			CString sPortName;					//	Name of port
			int iCount;							//	Number of ports with the same name
			IArchonMessagePort *pPort;			//	Port interface (may be NULL if not in this process)
			DWORD dwFlags;						//	Port flags
			};

		void FixupPortCount (void);
		bool IsVisible (SPortDesc *pDesc, DWORD dwFlags);

		TSortMap<CString, SPortDesc> m_PortList;
		bool m_bPortCountValid;
	};

//	CMessageQueue --------------------------------------------------------------

class CMessageQueue
	{
	public:
		CMessageQueue (int iMaxSize) : m_Queue(iMaxSize) { m_HasMessages.Create(); }

		bool Dequeue (int iMaxCount, CArchonMessageList *retList);
		bool Enqueue (const SArchonMessage &Msg);
		inline CManualEvent &GetEvent (void) { return m_HasMessages; }
		void Mark (void);

	private:
		CCriticalSection m_cs;
		TQueue<SArchonMessage> m_Queue;
		CManualEvent m_HasMessages;
	};

//	CInterprocessMessageQueue --------------------------------------------------

class CInterprocessMessageQueue
	{
	public:
		CInterprocessMessageQueue (void) : m_pHeader(NULL) { }

		bool Create (const CString &sMachine, const CString &sProcess, int iMaxSize);
		bool Dequeue (int iMaxCount, TArray<CString> *retList);
		bool Enqueue (const SArchonEnvelope &Env, CString *retsError);
		inline CManualEvent &GetEvent (void) { return m_HasMessages; }
		inline bool Open (const CString &sMachine, const CString &sProcess) { return Open(strPattern("%s-%s", sMachine, sProcess)); }

		static bool DecodeFileMsg (const SArchonMessage &Msg, SArchonMessage *retMsg);
		static bool DeserializeMessage (const CString &sEnv, SArchonEnvelope *retEnv);
		static void SerializeMessage (const SArchonEnvelope &Env, IByteStream &Stream);

	private:
		struct SHeader
			{
			DWORD dwTotalSize;
			DWORD dwSize;
			DWORD dwHead;
			DWORD dwTail;
			DWORD dwFirstFree;

			char szTempPath[256];
			};

		struct SQueueSlot
			{
			DWORD dwOffset;
			};

		struct SEntry
			{
			int iSize;
			};

		struct SFreeEntry
			{
			int iSize;
			DWORD dwNext;
			};

		DWORD AllocEntry (int iSize);
		void CombineFreeBlocks (DWORD dwFirst, DWORD dwSecond);
		bool CreateTempFile (CString *retsFilespec, CFile *retFile, CString *retsError = NULL);
		void DebugDumpBlocks (void);
		static bool DeserializeMessage (IByteStream &Stream, SArchonEnvelope *retEnv);
		bool Enqueue (CMemoryBuffer &Buffer, CString *retsError);
		DWORD FindFreeBlock (int iMinSize, SFreeEntry **retpPrev);
		void FreeEntry (DWORD dwOffset);
		inline SEntry *GetEntry (DWORD dwOffset) { return (SEntry *)(((char *)m_pHeader) + dwOffset); }
		inline SFreeEntry *GetFreeEntry (DWORD dwOffset) { return (dwOffset == 0 ? NULL : (SFreeEntry *)(((char *)m_pHeader) + dwOffset)); }
		inline DWORD GetNextEntry (DWORD dwOffset) { return dwOffset + sizeof(SEntry) + Abs(GetEntry(dwOffset)->iSize); }
		inline SQueueSlot *GetQueueSlot (DWORD dwPos) { return (SQueueSlot *)(((char *)m_pHeader) + (sizeof(SHeader) + (dwPos * sizeof(DWORD)))); }
		inline CString GetEventName (const CString &sName) { return strPattern("%s-Event", sName); }
		inline CString GetMemoryName (const CString &sName) { return strPattern("%s-Memory", sName); }
		inline CString GetSemaphoreName (const CString &sName) { return strPattern("%s-Semaphore", sName); }
		inline bool IsEmpty (void) const { return (m_pHeader->dwHead == m_pHeader->dwTail); }
		inline bool IsFree (DWORD dwOffset) { return (GetFreeEntry(dwOffset)->iSize < 0); }
		inline bool IsFull (void) const	{ return (((m_pHeader->dwTail + 1) % m_pHeader->dwSize) == m_pHeader->dwHead); }
		inline bool IsOpen (void) const { return (m_pHeader != NULL); }
		bool Open (const CString &sName);
		inline void SetFree (SFreeEntry *pEntry, int iSize, DWORD dwNext) { pEntry->iSize = -iSize; pEntry->dwNext = dwNext; }

		CString m_sName;
		CSemaphore m_Lock;
		CSharedMemoryBuffer m_Queue;
		CManualEvent m_HasMessages;

		//	Valid only after we open
		SHeader *m_pHeader;
		CString m_sTempPath;
	};

//	CMessageTransporter --------------------------------------------------------

class CMessagePort
	{
	public:
		CDatum GetStatus (void) const;
		bool SendMessage (const SArchonMessage &Msg);

		static bool IsNullAddr (const CString &sAddr);

	private:
		inline CMessagePort (CMessageTransporter &Transporter, const CString &sAddress) : 
				m_Transporter(Transporter),
				m_sAddress(sAddress),
				m_pPort(NULL),
				m_bFreePort(false)
			{ }

		inline ~CMessagePort (void) { CleanUpPort(); }

		//	CMessageTransporter must either lock this port or otherwise 
		//	ensure exclusive access before calling these methods.

		inline void CleanUpPort (void) { if (m_pPort && m_bFreePort) delete m_pPort; m_pPort = NULL; }
		inline const CString &GetAddress (void) const { return m_sAddress; }
		inline CCriticalSection &GetLock (void) { return m_cs; }
		inline IArchonMessagePort *GetPort (void) const { return m_pPort; }
		inline void InvalidatePort (void) { CSmartLock Lock(m_cs); CleanUpPort(); }
		inline void SetPort (IArchonMessagePort *pPort, bool bFreePort = false) { CleanUpPort(); m_pPort = pPort; m_bFreePort = bFreePort; }

		CCriticalSection m_cs;				//	This controls access to m_pPort
		CMessageTransporter &m_Transporter;
		CString m_sAddress;
		IArchonMessagePort *m_pPort;
		bool m_bFreePort;

	friend CMessageTransporter;
	};

class CIntermachinePort : public IArchonMessagePort
	{
	public:
		CIntermachinePort (IArchonProcessCtx *pProcess, const CString &sMachine, const CString &sAddr) : 
				m_pProcess(pProcess), 
				m_sMachine(sMachine),
				m_sAddr(sAddr)
			{ }

		//	IArchonMessagePort
		virtual CDatum GetPortStatus (void) const override;
		virtual bool SendMessage (const SArchonMessage &Msg) override;

	private:
		IArchonProcessCtx *m_pProcess;
		CString m_sMachine;
		CString m_sAddr;
	};

class CInterprocessPort : public IArchonMessagePort
	{
	public:
		CInterprocessPort (IArchonProcessCtx *pProcess, CInterprocessMessageQueue *pQueue, const CString &sAddr) : 
				m_pProcess(pProcess), 
				m_pQueue(pQueue), 
				m_sAddr(sAddr)
			{ }

		//	IArchonMessagePort
		virtual CDatum GetPortStatus (void) const override;
		virtual bool SendMessage (const SArchonMessage &Msg) override;

	private:
		IArchonProcessCtx *m_pProcess;
		CInterprocessMessageQueue *m_pQueue;
		CString m_sAddr;
	};

class CMessagePortSplitter : public IArchonMessagePort
	{
	public:
		void AddPort (CMessagePort *pPort);

		//	IArchonMessagePort
		virtual CDatum GetPortStatus (void) const override;
		virtual bool IsValid (void) const override { return (m_Ports.GetCount() > 0); }
		virtual bool SendMessage (const SArchonMessage &Msg) override;

	private:
		TArray<CMessagePort *> m_Ports;
	};

class CProxyPort : public IArchonMessagePort
	{
	public:
		CProxyPort (const CString &sMsg, CMessagePort *pPort);

		//	IArchonMessagePort
		virtual CDatum GetPortStatus (void) const override;
		virtual bool SendMessage (const SArchonMessage &Msg) override;

	private:
		CString m_sMsg;
		CMessagePort *m_pPort;
	};

enum EVirtualPortFlags
	{
	FLAG_PORT_ALWAYS =				0x00000001,	//	Always send to this port
	FLAG_PORT_NEAREST =				0x00000002,	//	Send to this port if it is the nearest (of all marked NEAREST)
	FLAG_PORT_RANDOM =				0x00000004,	//	Send to 1 random port among all marked as RANDOM
	FLAG_PORT_ALWAYS_MODULE =		0x00000008,	//	Always send to this port, if in the same module
	FLAG_PORT_ALWAYS_MACHINE =		0x00000010,	//	Always send to this port, if in the same machine

	FLAG_PORT_LOCAL_MODULE =		0x08000000,	//	This port is local to the module
	FLAG_PORT_LOCAL_MACHINE =		0x10000000,	//	This port is local to the machine
	FLAG_PORT_LOCATION_MASK =		0x18000000,
	};

class CMessageTransporter
	{
	public:
		~CMessageTransporter (void);

		void AddLocalPort (const CString &sPort, IArchonMessagePort *pPort);
		void AddVirtualPort (const CString &sPort, const CString &sAddress, DWORD dwFlags);
		CMessagePort *Bind (const CString &sAddress);
		void Boot (IArchonProcessCtx *pProcess);
		CString GenerateAbsoluteAddress (const CString &sAddress);
		static CString GenerateAddress (const CString &sPort, const CString &sProcessName = NULL_STR, const CString &sMachineName = NULL_STR);
		CString GenerateMachineAddress (const CString &sMachineName, const CString &sAddress);
		TArray<CMessagePort *> GetPortCacheList (void) const;
		bool IsLocalMachine (const CString &sMachineName);
		bool IsLocalProcess (const CString &sProcessName);
		void OnModuleDeleted (const CString &sName);
		static bool ParseAddress (const CString &sAddress, CString *retsPort, CString *retsProcessName, CString *retsMachineName, bool *retbVirtual = NULL);
		static CString ParseAddressPort (const CString &sAddress);
		void PublishToMnemosynth (void);

	private:
		struct SCacheEntry
			{
			IArchonMessagePort *pPort;
			};

		struct SVirtualPortEntry
			{
			CString sAddress;
			DWORD dwFlags;
			};

		typedef TArray<SVirtualPortEntry> CVirtualPortArray;

		IArchonMessagePort *BindRaw (const CString &sAddress, bool *retbFree);
		IArchonMessagePort *CreateVirtualPortBinding (const CString &sAddress);
		CDatum GetVirtualPortList (void);

		CCriticalSection m_cs;
		IArchonProcessCtx *m_pProcess;
		TSortMap<CString, CMessagePort *> m_Ports;

		TArray<CMessagePort *> m_EngineBinding;
		TSortMap<CString, CMessagePortSplitter *> m_LocalPorts;
		TSortMap<CString, CVirtualPortArray> m_VirtualPorts;
		TSortMap<CString, CInterprocessMessageQueue *> m_ProcessQueues;

	friend CMessagePort;
	};

//	CBlackBox ------------------------------------------------------------------

class CBlackBox
	{
	public:
		CBlackBox (void) : m_bConsoleOut(false)
			{ }

		~CBlackBox (void) { Shutdown(); }

		void Boot (const CString &sPath);
		void Log (const CString &sLine);
		inline void SetConsoleOutput (bool bEnabled = true) { ::SetConsoleOutputCP(65001); m_bConsoleOut = bEnabled; }
		void Shutdown (void);

		static bool ReadRecent (const CString &sPath, const CString &sFind, int iLines, TArray<CString> *retLines);

	private:
		CFile m_File;
		bool m_bConsoleOut;					//	If TRUE, we also printf to the console
	};

//	CTimedMessageQueue ---------------------------------------------------------

class CTimedMessageQueue
	{
	public:
		CTimedMessageQueue (void) : m_dwNextID(1) { m_HasMessages.Create(); }

		void AddMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload, DWORD *retdwID = NULL);
		void AddTimeoutMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, DWORD dwTicket, DWORD *retdwID = NULL);
		void DeleteMessage (DWORD dwID);
		inline CManualEvent &GetEvent (void) { return m_HasMessages; }
		DWORD GetTimeForNextMessage (void);
        void KeepAliveMessage (DWORD dwID);
		void Mark (void);
		void ProcessMessages (IArchonProcessCtx *pProcess);

	private:
		struct SEntry
			{
			DWORD dwID;
			DWORD dwTime;

			CString sAddr;
			CString sMsg;
			CString sReplyAddr;
			DWORD dwTicket;
			CDatum dPayload;
			};

		DWORD GetTimeToWait (DWORD dwNow, DWORD dwTime);
		DWORD GetSmallestTimeToWait (DWORD dwNow);

		CCriticalSection m_cs;
		CManualEvent m_HasMessages;
		TArray<SEntry> m_Queue;
		DWORD m_dwNextID;
	};

//	TTicketManager -------------------------------------------------------------

template <class VALUE> class TTicketManager
	{
	public:
		TTicketManager (void) : m_dwTick(0) { }

		~TTicketManager (void)
			{
			int i;

			for (i = 0; i < m_Array.GetCount(); i++)
				if (m_Array[i])
					delete m_Array[i];

			for (i = 0; i < m_Array.GetCount(); i++)
				if (m_Deleted[i])
					delete m_Array[i];
			}

		void Delete (DWORD dwTicket)
			{
			CSmartLock Lock(m_cs);

			int iIndex = GetIndex(dwTicket);
			if (iIndex < 0 || iIndex >= m_Array.GetCount())
				return;

			m_Deleted.Insert(m_Array[iIndex]);
			m_Array[iIndex] = NULL;
			}

		VALUE *GetAt (DWORD dwTicket)
			{
			CSmartLock Lock(m_cs);

			int iIndex = GetIndex(dwTicket);
			if (iIndex < 0 || iIndex >= m_Array.GetCount())
				return NULL;
		
			if (m_Array[iIndex] == NULL || m_Array[iIndex]->dwTicket != dwTicket)
				return NULL;

			return &m_Array[iIndex]->Ctx;
			}

		DWORD Insert (VALUE **retpCtx = NULL)
			{
			CSmartLock Lock(m_cs);

			SEntry *pNew = new SEntry;

			int iIndex = FindEmpty();
			pNew->dwTicket = MakeTicket(iIndex);
			m_Array[iIndex] = pNew;

			if (retpCtx)
				*retpCtx = &pNew->Ctx;

			return pNew->dwTicket;
			}

		void Mark (void)
			{
			int i;

			//	NOTE: No need to lock because this is always called while
			//	the world is stopped.

			for (i = 0; i < m_Array.GetCount(); i++)
				if (m_Array[i])
					m_Array[i]->Ctx.Mark();

			//	NOTE: No need to mark m_Deleted because they are no longer
			//	valid. In fact, we take this opportunity to purge deleted
			//	stuff.

			for (i = 0; i < m_Deleted.GetCount(); i++)
				delete m_Deleted[i];

			m_Deleted.DeleteAll();
			}

	private:
		struct SEntry
			{
			DWORD dwTicket;
			VALUE Ctx;
			};

		int FindEmpty (void)
			{
			for (int i = 0; i < m_Array.GetCount(); i++)
				if (m_Array[i] == NULL)
					return i;

			m_Array.Insert(NULL);
			return m_Array.GetCount() - 1;
			}

		int GetIndex (DWORD dwTicket)
			{ return LOWORD(dwTicket); }

		DWORD MakeTicket (int iIndex)
			{ return MAKELONG((WORD)iIndex, (WORD)m_dwTick++); }

		CCriticalSection m_cs;
		TArray<SEntry *> m_Array;
		TArray<SEntry *> m_Deleted;
		DWORD m_dwTick;
	};

//	CSessionManagerOld ---------------------------------------------------------

class ISessionCtx
	{
	public:
		ISessionCtx (SOCKET hSocket) : m_hSocket(hSocket) { }
		virtual ~ISessionCtx (void) { }

		inline SOCKET GetSocket (void) { return m_hSocket; }
		virtual int GetType (void) = 0;

	private:
		SOCKET m_hSocket;						//	Socket for the session
	};

typedef DWORD_PTR SESSIONID;

class CSessionManagerOld
	{
	public:
		~CSessionManagerOld (void);

		void Delete (SESSIONID dwSessionID);
		void DeleteBySocket (SOCKET hSocket);
		ISessionCtx *Insert (ISessionCtx *pSession);
		ISessionCtx *Find (SESSIONID dwSessionID) { return GetSession(dwSessionID); }
		ISessionCtx *FindBySocket (SOCKET hSocket);
		void PurgeDeleted (void);

	private:
		inline ISessionCtx *GetSession (SESSIONID dwSessionID) { return (ISessionCtx *)dwSessionID; }
		inline SESSIONID GetSessionID (ISessionCtx *pSession) { return (SESSIONID)pSession; }

		CCriticalSection m_cs;
		TArray<ISessionCtx *> m_Sessions;
		TArray<ISessionCtx *> m_Deleted;
	};

//	Storage volume structures --------------------------------------------------

class CMachineStorage
	{
	public:
		inline const CString &operator [] (int iIndex) const { return GetPath(iIndex); }

		CString CanonicalToMachine (const CString &sFilespec);
		CString CanonicalRelativeToMachine (const CString &sVolume, const CString &sFilespec);
		bool FindVolume (const CString &sVolume, int *retiIndex = NULL) const;
		inline int GetCount (void) const { CSmartLock Lock(m_cs); return m_Volumes.GetCount(); }
		inline const CString &GetPath (int iIndex) const { CSmartLock Lock(m_cs); return m_Volumes[iIndex].sMachineRoot; }
		const CString &GetPath (const CString &sVolume) const;
		const CString &GetRedundantVolume (const CString &sVolume) const;
		inline const CString &GetVolume (int iIndex) const { CSmartLock Lock(m_cs); return m_Volumes[iIndex].sVolumeName; }
		CString GetVolumePath (const CString &sFilespec);
		bool Init (IArchonProcessCtx *pProcess, const CString &sEngineDirectory, CString *retsError);
		bool InitLocal (const CString &sStoragePath, const CString &sEngineDirectory, CString *retsError);
		CString MachineToCanonical (const CString &sFilespec);
		CString MachineToCanonicalRelative (const CString &sFilespec, CString *retsVolume = NULL);
		bool Reinit (IArchonProcessCtx *pProcess, TArray<CString> *retVolumesAdded, TArray<CString> *retVolumesDeleted, CString *retsError);

	private:
		struct SVolumeDesc
			{
			CString sResourceName;				//	Globally unique resource name (but unstable across boots)
			CString sVolumeName;				//	Locally unique, stable name for the volume
			CString sPath;						//	Locally unique physical path

			CString sCanonicalRoot;				//	Volume name + engine directory
			CString sMachineRoot;				//	Path + engine directory
			bool bMarked;						//	Temp while reinit
			};

		CCriticalSection m_cs;
		CString m_sEngineDirectory;
		TArray<SVolumeDesc> m_Volumes;
	};
