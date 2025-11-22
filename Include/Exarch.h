//	Exarch.h
//
//	Exarch Archon Implementation
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#pragma once

#ifdef DEBUG
#define ENABLE_AMP1_FABRIC
#endif

//	Arcology Membership --------------------------------------------------------

struct SMachineDesc
	{
	CString sNodeID;							//	Unique ID (persistent across node restart)
	CString sName;								//	Unique name of machine
	CString sAddress;							//	Address to reach machine
	CIPInteger Key;								//	Machine key
	};

struct SModuleDesc
	{
	CString sName;								//	Module name (unique in machine)
	CString sFilespec;							//	Absolute filespec to executable
	CString sVersion;							//	Module version (from .exe)
	CString sStatus;							//	Current status
	DWORD dwProcessID;							//	ID of process
	};

class CMecharcologyDb
	{
	public:
		struct SInit
			{
			CString sMachineName;
			CString sMachineHost;

			CString sModulePath;
			CString sCurrentModule;

			CString sArcologyPrimeAddress;
			};

		void Boot (const SInit &Init);
		CDatum GetModuleList (void) const;
		bool SetHostAddress (const CString &sName, const CString &sHostAddress);

		//	Machine functions

		bool AddMachine (CStringView sNodeID, CStringView sDisplayName, CStringView sAddress, const CIPInteger &SecretKey, bool bCheckUpgrade, CString *retsError);
		CString AllocNodeID () const;
		bool AuthenticateMachine (const CString &sAuthName, const CIPInteger &AuthKey);
		bool CheckForUpgrade (const CString &sMachineName);
		bool DeleteMachine (const CString &sName);
		bool DeleteMachine (const SMachineDesc &Desc);
		bool DeleteMachineByKey (const CIPInteger &Key);
		bool FindArcologyPrime (SMachineDesc *retDesc = NULL) const;
		bool FindMachineByAddress (const CString &sAddress, SMachineDesc *retDesc = NULL);
		bool FindMachineByName (const CString &sName, SMachineDesc *retDesc = NULL) const;
		bool FindMachineByNodeID (CStringView sNodeID, SMachineDesc* retDesc = NULL) const;
		bool FindMachineByPartialName (const CString &sName, SMachineDesc *retDesc = NULL) const;
		bool GetMachine (const CString &sName, SMachineDesc *retDesc = NULL);
		bool GetMachine (int iIndex, SMachineDesc *retDesc) const;
		int GetMachineCount (void) const { return m_Machines.GetCount(); }
		bool GetStatus (CDatum *retStatus);
		bool HasArcologyKey (CIPInteger* retsKey = NULL) const;
		bool IsArcologyPrime () const { return m_iArcologyPrime == -1; }
		bool IsArcologyPrime (const CString &sName) const;
		bool JoinArcology (const CString &sPrimeName, const CIPInteger &PrimeKey, CString *retsError);
		bool LeaveArcology (CString *retsError);
		static CString MakeNodeID (DWORD dwID);
		static CString ArcologyPrimeNodeID () { return MakeNodeID(1); }
		bool OnCompleteAuth (CStringView sName, CString& retsNodeID);
		bool ProcessOldMachines (TArray<CString> &OldMachines);
		bool SetArcologyKey (const CIPInteger &PrimeKey, CString *retsError);

		//	Module functions

		bool CanRestartModule (const CString &sName) const;
		bool DeleteModule (const CString &sName);
		bool FindModuleByFilespec (const CString &sFilespec, SModuleDesc *retDesc = NULL);
		bool GetCentralModule (SModuleDesc *retDesc = NULL);
		bool GetModule (const CString &sName, SModuleDesc *retDesc = NULL) const;
		int GetModuleCount (void) { CSmartLock Lock(m_cs); return m_Modules.GetCount(); }
		const CString &GetModuleName (int iIndex) { CSmartLock Lock(m_cs); return m_Modules[iIndex].sName; }
		CProcess *GetModuleProcess (const CString &sName);
		CProcess &GetModuleProcess (int iIndex) { CSmartLock Lock(m_cs); return m_Modules[iIndex].hProcess; }
		CTimeSpan GetModuleRunTime (const CString &sName) const;
		MnemosynthSequence GetModuleSeq (int iIndex) { CSmartLock Lock(m_cs); return m_Modules[iIndex].dwSeq; }
		bool IsModuleRemoved (const CString &sName) const;
		bool IsModuleRunning (const CString &sName) const;
		bool LoadModule (const CString &sFilespec, bool bDebug, CString *retsName, CString *retsError);
		void OnMnemosynthUpdated (void);
		void OnModuleStart (const CString &sName, MnemosynthSequence dwMnemosynthSeq, bool *retbAllComplete);
		void OnModuleRestart (const CString &sName);
		void SetModuleRemoved (const CString &sName);

	private:
		static constexpr DWORDLONG MIN_RUN_TIME_TO_RESTART =				60;	//	seconds

		enum EConnectStates
			{
			connectNone,						//	No connection

			connectAuth,						//	Authenticated, but not yet fully connected
			connectActive,						//	Connection is active
			};

		struct SMachineEntry
			{
			CString sNodeID;					//	Unique ID (persistent across node restart)
			CString sName;						//	Machine name (generated at connect time)
			CString sDisplayName;				//	Name assigned by user
			CString sAddress;					//	Machine address
			CIPInteger SecretKey;				//	Shared secret
			bool bCheckUpgrade = false;			//	If TRUE, ask this machine to check for upgrades

			EConnectStates iStatus = connectNone;	//	Status of our connection to the machine
			};

		enum EModuleStates
			{
			moduleLaunched,						//	Process has been launched
			moduleOnStart,						//	We've received an OnModuleStart
			moduleRunning,						//	Module is running and we have its initial Mnemosynth state
			moduleRemoved,						//	Module has been removed

			moduleRestarted,					//	We've deliberately shutdown the module and are
												//	expecting it to restart.
			};

		struct SModuleEntry
			{
			CString sName;						//	Module name (unique in machine)
			CString sFilespec;					//	Absolute filespec to executable

			CProcess hProcess;					//	Running process
			EModuleStates iStatus = moduleLaunched;		//	Current module status
			MnemosynthSequence dwSeq = 0;		//	Seq (used during startup)
			CString sVersion;					//	Module version
			CDateTime StartTime;				//	Time when module started
			};

		static SMachineDesc AsMachineDesc (const SMachineEntry& Entry);
		int FindMachine (const SMachineDesc &Desc) const;
		int FindMachine (const CString &sName);
		int FindModule (const CString &sName) const;
		void SetCurrentModule (const CString &sName);
		CString StatusToID (EModuleStates iStatus) const;

		CCriticalSection m_cs;

		CString m_sModulePath;					//	Path of modules

		SMachineDesc m_MachineDesc;				//	Data about our machine
		int m_iArcologyPrime;					//	Index of Arcology Prime in m_Machines
		TArray<SMachineEntry> m_Machines;		//	List of machines and our connection status
		TArray<SModuleEntry> m_Modules;			//	List of modules on this machine (0 is
												//		CentralModule)
		TArray<CString> m_OldMachines;			//	Stale machine names
	};

//	CSessionManager ------------------------------------------------------------

enum SessionTypes
	{
	stypeUnknown,
	stypeArcology,								//	Connected to another member of the arcology
	};

class CArcologySession : public ISessionCtx
	{
	public:
		CArcologySession (SOCKET hSocket) : ISessionCtx(hSocket) { }

		virtual int GetType (void) override { return stypeArcology; }
	};

//	CExarchEngine --------------------------------------------------------------

class CExarchEngine : public TSimpleEngine<CExarchEngine>,
		public IArchonExarch,
		public IAMP1CommunicatorEvents
	{
	public:

		static constexpr DWORD DEFAULT_AMP1_PORT =					7397;
		static constexpr DWORD DEFAULT_AMP1_PORT_NEW =				7398;

		struct SOptions
			{
			SOptions (void) :
					dwAMP1Port(0)
				{ }

			CString sArcologyPrime;			//	If set, then we should connect to this arcology
			CString sConfigFilename;		//	Name of config file (default to "Config.ars")
			DWORD dwAMP1Port;				//	Port to listen to for AMP1 messages
			};

		CExarchEngine (const SOptions &Options);
		virtual ~CExarchEngine (void);

		CMecharcologyDb &GetMecharcologyDb () { return m_MecharcologyDb; }
		bool IsArcologyPrime () const { return m_sArcologyPrime.IsEmpty(); }
		bool IsLocalModule (const CString &sModuleName) const { return m_MecharcologyDb.GetModule(sModuleName); }
		bool IsRestartRequired (const CString &sFilename) const;
		void RemoveMachine (const SMachineDesc &MachineToRemove);
		bool SendAMP1Command (const CString &sMachineName, const CString &sCommand, CDatum dData);
		void SetAeonInitialized (bool bInitialized = true) { m_bAeonInitialized = bInitialized; }

		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

		//	IArchonExarch

		virtual bool ExarchAuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) override;

		//	IAMP1CommunicatorEvents

		virtual void OnAMP1ClientConnected (CStringView sNodeID) override;
		virtual void OnAMP1ClientDisconnected (CStringView sNodeID) override;
		virtual void OnAMP1ConnectedToServer () override;
		virtual void OnAMP1FatalError (CStringView sError) override;
		virtual void OnAMP1Message (CStringView sNodeID, CStringView sMsg, CBuffer&& retData) override;

	protected:

		//	TSimpleEngine override
		virtual void OnBoot (void) override;
		virtual void OnMarkEx (void) override;
		virtual void OnSignalPause (void) override { CSmartLock Lock(m_cs); m_bInGC = true; }
		virtual void OnSignalResume (void) override { CSmartLock Lock(m_cs); m_bInGC = false; m_AMP1Queue.ReplayEvents(*this); }
		virtual void OnStartRunning (void) override;
		virtual void OnStopRunning (void) override;

	private:
		static constexpr DWORDLONG MAX_LOG_ROWS =					1000;
		//	Message handlers
		void MsgAddMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgAddModule (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgAddVolume (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCheckUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCompleteUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCreateTestVolume (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgDeleteTestDrive (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgEsperOnAMP1 (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetLogRows (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetMachineStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetModuleList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetStorageList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgMnemosynthEndpointList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgMnemosynthOnArcologyUpdated (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgMnemosynthRead (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgOnLog (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgOnModuleStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgOnModuleStop (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgPing (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRemoveMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRemoveModule (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRemoveVolume (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgReportDiskError (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRequestUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRestartMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRestartModule (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgSendToMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgShutdown (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgUploadUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		//	AMP1
		void AMP1Auth (CDatum dData, CDatum dConnection);
		void AMP1Join (CDatum dData);
		void AMP1Leave (void);
		void AMP1Ping (const CString &sSender);
		void AMP1Rejoin (void);
		void AMP1Send (CDatum dData);
		void AMP1Welcome (void);

		//	AI1 Commands
		bool AddMachineConnection (const CString &sHostname, CString *retsError);
		bool AddModule (const CString &sModuleInput, bool bDebug, CString *retsModuleName, CString *retsError);
		bool AddStorage (const CString &sName, const CString &sLocalPath, DWORD dwQuota, const CString &sTestDrive, CString *retsError);
		bool DeleteStorage (const CString &sVolume, CString *retsError);
		bool CreateArcology (const CString &sName, const CString &sHostAddress, CString *retsError);
		bool RemoveModule (const CString &sModuleName, CString *retsError);

		//	Other helper methods
		bool CleanUpUpgrade (void);
		bool CheckArcology (CString *retsError);
		void DeleteMachineResources (const CString &sName);
		void DeleteModuleResources (const CString &sName, bool bDeleteModule = false);
		void ExecuteProtocol (ISessionCtx *pCtx, const CString &sInput, CString *retsOutput);
		void ExecuteProtocolAMP1 (CArcologySession *pCtx, const CString &sInput, DWORD dwVersion, CString *retsOutput);
		bool FindVolume (const CString &sVolume, CDatum *retdVolumeDesc = NULL, CString *retsKey = NULL);
		CString GenerateResourceNameFromFilespec (const CString &sFilespec);
		CString GetMachineStatus (void);
		void OnMachineConnection (CStringView sNodeID, CStringView sName);
		void OnMachineStart (void);
		bool ParseMachineName (const CString &sValue, CString *retsMachineName) const;
		bool ParseModuleName (const CString &sValue, CString *retsMachineName, CString *retsModuleName) const;
		SessionTypes ParseProtocol (const CString &sInput, CString *retsCommand);
		bool ReadConfig (void);
		void RegisterForMnemosynthUpdate (void);
		bool SendAMP1Command (const SMachineDesc &Desc, const CString &sCommand, CDatum dData);
		void SendReadRequest (CDatum dSocket);
		void SendWriteRequest (CDatum dSocket, const CString &sData);
		void SetMachineData (const CString &sHostAddress, const CString &sStatus);
		bool TestFileRead (const CString &sFilespec);
		bool WriteConfig (void);

		//	Storage
		bool CreateStorageConfig (void);
		bool LoadStorageConfig (CDatum dStorageConfig);
		bool WriteStorageConfig (void);

		CCriticalSection m_cs;
		CString m_sConfigFilename;				//	Filename to load configuration from

		CString m_sArcologyPrime;				//	Address for Arcology Prime machine (if NULL, then we are Arcology Prime)
		CIPInteger m_ArcologyKey;				//	Key to use to communicate
		DWORD m_dwAMP1Port = DEFAULT_AMP1_PORT;	//	Port on which we listen to AMP1 commands

		CDatum m_dMachineConfig;				//	Configuration for this machine
		CMecharcologyDb m_MecharcologyDb;		//	Machine arcology database
		CAMP1CommunicatorServer m_AMP1Server;	//	Communicator for AMP1 messages
		CAMP1Queue m_AMP1Queue;					//	Queue for deferred AMP1 messages
		CSessionManagerOld m_Sessions;			//	Keeping track of sessions
		CProxyPort *m_pLoggingPort = NULL;

		DWORD m_dwNextVolume = 0;				//	Next volume number
		TArray<CString> m_DeletedVolumes;		//	Keep track of deleted volumes (for UI purposes)
		bool m_bInGC = false;					//	If TRUE, we're in the middle of a garbage collection
		bool m_bInStopRunning = false;			//	If TRUE, we're shutting down the machine
		bool m_bAeonInitialized = false;		//	If TRUE, Aeon has already started
		bool m_bBroadcastCheckUpgrade = false;	//	If TRUE, remember to tell other machines to check for upgrade
	};
