//	ArchonEngineImpl.h
//
//	Engine Implementations
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by ArchonEngine.h

#pragma once

class CArchonProcess;

//  Basic Classes --------------------------------------------------------------

class CMsgProcessCtx
	{
	public:
		CMsgProcessCtx (IArchonProcessCtx &Process, const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx) :
				m_Process(Process),
				m_Msg(Msg),
				m_pSecurityCtx(pSecurityCtx)
			{ }

		const SArchonMessage &GetMsg (void) const { return m_Msg; }
		IArchonProcessCtx &GetProcess (void) const { return m_Process; }
		void SendMessageReply (const CString &sReplyMsg, CDatum dPayload)
			{
			m_Process.SendMessageReply(sReplyMsg, dPayload, m_Msg);
			}

	private:
		IArchonProcessCtx &m_Process;
		const SArchonMessage &m_Msg;
		const CHexeSecurityCtx *m_pSecurityCtx;
	};

//	CSimpleEngine --------------------------------------------------------------

class CSimpleEngine;

constexpr int DEFAULT_THREAD_COUNT = 3;

class CSimpleProcessingThread : public TThread<CSimpleProcessingThread>
	{
	public:
		CSimpleProcessingThread (CSimpleEngine *pEngine);

		EProcessingState GetState (DWORD *retdwDuration = NULL);
		void WaitForPause (void) { m_PausedEvent.Wait(); }

		void Run (void);

	private:
		void SetState (EProcessingState iState) { m_iState = iState; m_dwLastStateChange = ::GetTickCount(); }

		CSimpleEngine *m_pEngine;
		int m_iProcessingChunk;

		EProcessingState m_iState;			//	Current thread state
		DWORD m_dwLastStateChange;			//	Tick when thread changed state
		CManualEvent m_PausedEvent;			//	If set, then the thread has stopped
	};

class CSimpleEventThread : public TThread<CSimpleEventThread>
	{
	public:
		CSimpleEventThread (CSimpleEngine *pEngine);

		void WaitForPause (void) { m_PausedEvent.Wait(); }

		void Run (void);

	private:
		CSimpleEngine *m_pEngine;
		CManualEvent m_PausedEvent;			//	If set, then the thread has stopped
	};

class CSimpleEngine : public IArchonEngine, public IArchonMessagePort, protected IArchonProcessCtx
	{
	public:
		CSimpleEngine (const CString &sName, int iInitialThreads = DEFAULT_THREAD_COUNT);
		virtual ~CSimpleEngine (void);

		//	IArchonMessagePort
		virtual CDatum GetPortStatus (void) const override;
		virtual bool SendMessage (const SArchonMessage &Msg) override { return m_Queue.Enqueue(Msg); }

		//	IArchonEngine
		virtual void Boot (IArchonProcessCtx *pProcess, DWORD dwID) override;
		virtual const CString &GetName (void) override { return m_sName; }
		virtual bool IsIdle (void) override;
		virtual void Mark (void) override { m_Queue.Mark(); m_TimedQueue.Mark(); OnMark(); }
		virtual void SignalShutdown (void) override { OnStopRunning(); }
		virtual void StartRunning (CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent) override;
		virtual void WaitForShutdown (void) override;
		virtual void WaitForPause (void) override;

		//	Used by threads
		int AddEvents (CWaitArray *retWait);
		bool Dequeue (int iMaxCount, CArchonMessageList *retList) { return m_Queue.Dequeue(iMaxCount, retList); }
		IArchonProcessCtx *GetProcessCtx (void) const { return m_pProcess; }
		CManualEvent &GetQueueEvent (void) { return m_Queue.GetEvent(); }
		CManualEvent &GetQuitEvent (void) { return *m_pQuitEvent; }
		CManualEvent &GetRefreshEvent (void) { return m_RefreshEvent; }
		CManualEvent &GetRunEvent (void) { return *m_pRunEvent; }
		CManualEvent &GetPauseEvent (void) { return *m_pPauseEvent; }
		CManualEvent &GetTimedMessageEvent (void) { return m_TimedQueue.GetEvent(); }
		DWORD GetTimedMessageWait (void) { return m_TimedQueue.GetTimeForNextMessage(); }
		void ProcessEvent (const CString &sEventID);
		void ProcessMessages (CArchonMessageList &List) { OnProcessMessages(List); }
		void ProcessTimedMessages (void) { m_TimedQueue.ProcessMessages(m_pProcess); }
		void ProcessRefresh (void);

		//	Used by CSession
		void DeleteTimedMessage (DWORD dwID) { m_TimedQueue.DeleteMessage(dwID); }
		void KeepAliveTimedMessage (DWORD dwID) { m_TimedQueue.KeepAliveMessage(dwID); }
		void SendTimedMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload, DWORD *retdwID = NULL)	{ m_TimedQueue.AddMessage(dwDelay, sAddr, sMsg, sReplyAddr, dwTicket, dPayload, retdwID); }
		void SendTimeoutMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, DWORD dwTicket, DWORD *retdwID) { m_TimedQueue.AddTimeoutMessage(dwDelay, sAddr, sMsg, dwTicket, retdwID); }

		//	Helpers
		static bool IsError (const SArchonMessage &Msg);
		static CDatum MessageToHexeResult (const SArchonMessage &Msg);
		void SendMessageNotify (const CString &sAddress, const CString &sMsg, CDatum dPayload) { m_pProcess->SendMessageCommand(sAddress, sMsg, NULL_STR, 0, dPayload); }
		void SendMessageReplyError (const CString &sMsg, const CString &sText, const SArchonMessage &OriginalMsg) { m_pProcess->SendMessageReply(sMsg, CDatum(sText), OriginalMsg); }
		bool ValidateSandbox (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, const CString &sSandbox);
		bool ValidateSandboxAdmin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

	protected:
		struct SThreadStatus
			{
			EProcessingState iState;
			DWORD dwDuration;
			};

		//	IArchonProcessCtx
		virtual void AddEventRequest (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket) override;
		virtual void AddPort (const CString &sPort, IArchonMessagePort *pPort) override { m_pProcess->AddPort(sPort, pPort); }
		virtual void AddVirtualPort (const CString &sPort, const CString &sAddress, DWORD dwFlags) override { m_pProcess->AddVirtualPort(sPort, sAddress, dwFlags); }
		virtual bool AuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) override { return m_pProcess->AuthenticateMachine(sMachineName, Key); }
		virtual CMessagePort *Bind (const CString &sAddr) override { return m_pProcess->Bind(sAddr); }
		virtual CString GenerateAbsoluteAddress (const CString &sAddress) override { return m_pProcess->GenerateAbsoluteAddress(sAddress); }
		virtual CString GenerateAddress (const CString &sPort) override { return m_pProcess->GenerateAddress(sPort); }
		virtual CString GenerateMachineAddress (const CString &sMachineName, const CString &sAddress) override { return m_pProcess->GenerateMachineAddress(sMachineName, sAddress); }
		virtual const CString &GetMachineName (void) const override { return m_pProcess->GetMachineName(); }
		virtual CMnemosynthDb &GetMnemosynth (void) override { return m_pProcess->GetMnemosynth(); }
		virtual const CString &GetModuleName (void) override { return m_pProcess->GetModuleName(); }
		virtual CMessageTransporter &GetTransporter (void) override { return m_pProcess->GetTransporter(); }
		virtual void InitiateShutdown (void) override { m_pProcess->InitiateShutdown(); }
		virtual bool IsCentralModule (void) override { return m_pProcess->IsCentralModule(); }
		virtual void Log (const CString &sMsg, const CString &sText) override { m_pProcess->Log(sMsg, sText); }
		virtual void LogBlackBox (const CString &sText) override { m_pProcess->LogBlackBox(sText); }
		virtual CDatum MnemosynthRead (const CString &sCollection, const CString &sKey, SWatermark *retWatermark = NULL) const override { return m_pProcess->MnemosynthRead(sCollection, sKey, retWatermark); }
		virtual void MnemosynthReadCollection (const CString &sCollection, TArray<CString> *retKeys, SWatermark *retWatermark = NULL) const override { m_pProcess->MnemosynthReadCollection(sCollection, retKeys, retWatermark); }
		virtual bool MnemosynthWrite (const CString &sCollection, const CString &sKey, CDatum dValue, const SWatermark &Watermark = NULL_WATERMARK) override { return m_pProcess->MnemosynthWrite(sCollection, sKey, dValue, Watermark); }
		virtual void OnMnemosynthDbModified (CDatum dLocalUpdates) override { m_pProcess->OnMnemosynthDbModified(dLocalUpdates); }
		virtual bool ReadBlackBox (const CString &sFind, int iLines, TArray<CString> *retLines) override { return m_pProcess->ReadBlackBox(sFind, iLines, retLines); }
		virtual void ReportVolumeFailure (const CString &sFilespec, const CString &sOperation = NULL_STR) override { m_pProcess->ReportVolumeFailure(sFilespec, sOperation); }
		virtual bool SendMessage (const CString &sAddress, const SArchonMessage &Msg) override { return m_pProcess->SendMessage(sAddress, Msg); }
		virtual bool SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload) override { return m_pProcess->SendMessageCommand(sAddress, sMsg, sReplyAddr, dwTicket, dPayload); }
		virtual void SendMessageReply (const CString &sReplyMsg, CDatum dPayload, const SArchonMessage &OriginalMsg) override { m_pProcess->SendMessageReply(sReplyMsg, dPayload, OriginalMsg); }
		virtual void TranspaceDownload (const CString &sAddress, const CString &sReplyAddr, DWORD dwTicket, CDatum dDownloadDesc, const CHexeSecurityCtx *pSecurityCtx) override { m_pProcess->TranspaceDownload(sAddress, sReplyAddr, dwTicket, dDownloadDesc, pSecurityCtx); }

		//	Helpers
		void AddPort (const CString &sPort) { AddPort(sPort, this); }
		int GetThreadCount (void) { return m_Threads.GetCount(); }
		void GetThreadStatus (int iThread, SThreadStatus *retStatus);
		bool IsFileMsg (const SArchonMessage &Msg, CArchonMessageList *retList);
		bool IsSandboxMsg (const SArchonMessage &Msg, SArchonMessage *retMsg, CHexeSecurityCtx *retSecurityCtx);
		void LogCrashProcessingMessage (const SArchonMessage &Msg, const CException &e);
		void LogMessageTiming (const SArchonMessage &Msg, DWORD dwTime);
		bool ProcessMessageDefault (const SArchonMessage &Msg);

		//	Overridden by descendants
		virtual void OnBoot (void) { }
		virtual void OnMark (void) { }
		virtual void OnProcessMessages (CArchonMessageList &List) { }
		virtual void OnProcessReply (const SArchonMessage &Msg) { }
		virtual void OnStartRunning (void) { }
		virtual void OnStopRunning (void) { }
		virtual void OnWaitForPause (void) { }
		virtual void OnWaitForShutdown (void) { }

	private:
		struct SEvent
			{
			CString sName;					//	Name of event registration
			HANDLE hObj;					//	Event object to wait on

			IArchonMessagePort *pClient;	//	Client port
			CString sMsg;					//	Message
			DWORD dwTicket;					//	Ticket number when event fires
			};

		IArchonProcessCtx *m_pProcess;		//	Parent process
		CString m_sName;					//	Name of the engine
		DWORD m_dwID;						//	Our ID

		int m_iInitialThreadCount;			//	Number of threads to start initially
		TArray<CSimpleProcessingThread *> m_Threads;
		CMessageQueue m_Queue;				//	When set, there are messages in the queue

		CSimpleEventThread *m_pTimedMessageThread;	//	Processes timed queue
		CTimedMessageQueue m_TimedQueue;	//	Stores delayed messages

		CCriticalSection m_cs;				//	Protects event list
		TArray<SEvent> m_Events;			//	List of events to wait on
		TArray<SEvent> m_NewEvents;			//	List of new events
		CManualEvent m_RefreshEvent;		//	If set, then m_NewEvents has more events

		CManualEvent *m_pRunEvent;			//	When set, threads should run
		CManualEvent *m_pPauseEvent;		//	When set, threads should stop
		CManualEvent *m_pQuitEvent;			//	When set, threads should quit
	};

//	CSessionManager ------------------------------------------------------------

class CSessionManager;

class ISessionHandler
	{
	public:
		ISessionHandler (void) : m_pProcess(NULL), m_pEngine(NULL), m_pSessionManager(NULL), m_dwTicket(0), m_dwTimeoutID(0) { }
		virtual ~ISessionHandler (void) { }

		void EndSession (DWORD dwTicket) { OnEndSession(dwTicket); }
		CDatum GetStatusReport (void) const;
		void Mark (void) { m_OriginalMsg.dPayload.Mark(); OnMark(); }
		bool ProcessKeepAlive (const SArchonMessage &Msg);
		bool ProcessMessage (const SArchonMessage &Msg);
		bool ProcessTimeout (const SArchonMessage &Msg);
		bool StartSession (IArchonProcessCtx *pProcess, CSimpleEngine *pEngine, CSessionManager *pSessionManager, DWORD dwTicket, const SArchonMessage &Msg) { m_pProcess = pProcess; m_pEngine = pEngine; m_pSessionManager = pSessionManager; m_dwTicket = dwTicket; m_OriginalMsg = Msg; return OnStartSession(Msg, dwTicket); }
		
		//	Utilities
		CString GenerateAddress (const CString &sPort) { return m_pProcess->GenerateAddress(sPort); }
		const SArchonMessage &GetOriginalMsg (void) { return m_OriginalMsg; }
		IArchonProcessCtx *GetProcessCtx (void) { return m_pProcess; }
		CSimpleEngine *GetSimpleEngine () { return m_pEngine; }
		CSessionManager *GetSessionManager () { return m_pSessionManager; }
		DWORD GetTicket (void) const { return m_dwTicket; }
		bool IsError (const SArchonMessage &Msg);
		void ResetTimeout (const CString &sReplyAddr, DWORD dwTimeout);
		bool SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, CDatum dPayload, DWORD dwTimeout = 0);
		void SendMessageNotify (const CString &sAddress, const CString &sMsg, CDatum dPayload);
		void SendMessageReply (const CString &sMsg, CDatum dData = CDatum());
		void SendMessageReplyError (const CString &sMsg, const CString &sText);
		void SendMessageReplyProgress (const CString &sText, int iProgress = -1);
		void TranspaceDownload (const CString &sAddress, const CString &sReplyAddr, CDatum dDownloadDesc, const CHexeSecurityCtx *pSecurityCtx, DWORD dwTimeout = 0);

	protected:
		//	ISession virtuals
		virtual void OnEndSession (DWORD dwTicket) { }
		virtual void OnGetStatusReport (CComplexStruct *pStatus) const { }
		virtual void OnMark (void) { }
		virtual bool OnProcessMessage (const SArchonMessage &Msg) { return false; }
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) { return false; }
		virtual bool OnTimeout (const SArchonMessage &Msg) { return OnProcessMessage(Msg); }

	private:
		void SetTimeout (const CString &sReplyAddr, DWORD dwTimeout);

		CCriticalSection m_cs;

		IArchonProcessCtx *m_pProcess;
		CSimpleEngine *m_pEngine;
		CSessionManager *m_pSessionManager;
		DWORD m_dwTicket;
		SArchonMessage m_OriginalMsg;

		DWORD m_dwTimeoutID;
	};

class CSessionManager
	{
	public:
		CSessionManager (void) { }
		~CSessionManager (void);

		void Delete (DWORD dwTicket);
		ISessionHandler *GetAt (DWORD dwTicket);
		int GetCount (void) { CSmartLock Lock(m_cs); return m_Sessions.GetCount(); }
		void GetSessions (TArray<ISessionHandler *> *retSessions);
		DWORD Insert (ISessionHandler *pHandler);
		bool IsCustomTicket (DWORD dwTicket) const { return m_Sessions.IsCustomID(dwTicket); }
		DWORD MakeCustomTicket (void) { return m_Sessions.MakeCustomID(); }
		void Mark (void);
		void ProcessMessage (CSimpleEngine *pEngine, const SArchonMessage &Msg);

	private:
		CCriticalSection m_cs;
		TIDTable<ISessionHandler *> m_Sessions;
		TArray<ISessionHandler *> m_Deleted;
	};

//	TSimpleEngine --------------------------------------------------------------
//
//	USAGE
//
//	1.	Derive from TSimpleEngine (VALUE = derived class)
//	2.	In derived class declaration:
//
//		static SMessageHandler m_MsgHandlerList[];
//		static int m_iMsgHandlerListCount;
//	
//	3.	Define the static members in the class implementation
//
//	4.	When processing a message from a handler, call StartSession
//		to begin a new session.

template <class VALUE> class TSimpleEngine : public CSimpleEngine
	{
	public:
		TSimpleEngine (const CString &sName, int iInitialThreads = DEFAULT_THREAD_COUNT) : CSimpleEngine(sName, iInitialThreads)
			{ }

	protected:
		struct SMessageHandler
			{
			const CString &sMsg;
			void (VALUE::* MsgHandler)(const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
			};

		int GetSessionCount (void) { return m_Sessions.GetCount(); }
		void GetSessions (TArray<ISessionHandler *> *retSessions) { m_Sessions.GetSessions(retSessions); }

		DWORD MakeCustomTicket (void) { return m_Sessions.MakeCustomTicket(); }

		void MsgNull (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx) { }

		void StartSession (const SArchonMessage &Msg, ISessionHandler *pSession)
			{
			DWORD dwTicket = m_Sessions.Insert(pSession);

			//	NOTE: It is OK that we don't run this inside a try/catch block because
			//	any crashes will be caught by OnProcessMessage. (If this fails we want to
			//	stop further processing of the message).

			bool bContinue = pSession->StartSession(GetProcessCtx(), this, &m_Sessions, dwTicket, Msg);
			if (!bContinue)
				m_Sessions.Delete(dwTicket);
			}

		//	Virtuals for derived class to override

		virtual void OnMarkEx (void) { }

		//	CSimpleEngine virtuals

		virtual void OnMark (void) override
			{
			m_Sessions.Mark();
			OnMarkEx();
			}

		virtual void OnProcessMessages (CArchonMessageList &List) override
			{
			int i;
			CHexeSecurityCtx SecurityCtx;
			SArchonMessage NewMsg;
			CArchonMessageList NewList;

			for (i = 0; i < List.GetCount(); i++)
				{
				const SArchonMessage &Msg = List[i];

				DWORD dwStartTime = sysGetTickCount();

				//	Look for the handler in the table. If we find it, then
				//	we're done

				if (ProcessMessageFromTable(Msg, NULL))
					;

				//	Otherwise, see if this is a special message

				else if (IsFileMsg(Msg, &NewList))
					OnProcessMessages(NewList);

				else if (IsSandboxMsg(Msg, &NewMsg, &SecurityCtx)
						&& ProcessMessageFromTable(NewMsg, &SecurityCtx))
					;

				else if (ProcessMessageDefault(Msg))
					;

				//	If we didn't find a handler, then this must be a reply

				else if (m_Sessions.IsCustomTicket(Msg.dwTicket))
					OnProcessReply(Msg);

				else
					m_Sessions.ProcessMessage(this, Msg);

				DWORD dwTime = sysGetTicksElapsed(dwStartTime);
				if (dwTime >= 1000)
					LogMessageTiming(Msg, dwTime);
				}
			}

	private:

		bool ProcessMessageFromTable (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)
			{
			int j;

			for (j = 0; j < VALUE::m_iMsgHandlerListCount; j++)
				if (strEquals(Msg.sMsg, VALUE::m_MsgHandlerList[j].sMsg))
					{
					try
						{
						(((VALUE *)this)->*VALUE::m_MsgHandlerList[j].MsgHandler)(Msg, pSecurityCtx);
						}
					catch (CException e)
						{
						LogCrashProcessingMessage(Msg, e);
						}
					catch (...)
						{
						LogCrashProcessingMessage(Msg, CException(errUnknownError));
						}

					return true;
					}

			//	Not found

			return false;
			}

		CSessionManager m_Sessions;
	};

//	Other utility classes ------------------------------------------------------

class CConsoleThread : public TThread<CConsoleThread>
	{
	public:
		CConsoleThread (CArchonProcess *pProcess) : m_pProcess(pProcess)
			{
			m_Busy.Create(1);
			}

		void OnEndHousekeeping (void) { m_Busy.Decrement(); }
		void OnStartHousekeeping (void) { m_Busy.Increment(); }
		void Run (void);

	private:
		CString ProcessCommand (const CString &sCmd, const TArray<CDatum> &Args);

		static CString GetInputLine (const CString &sPrompt);
		static bool ParseArray (char *pPos, char *pPosEnd, char **retpPos, CDatum *retdArg);
		static bool ParseInputLine (const CString &sInput, CString *retsCmd, TArray<CDatum> *retArgs);
		static bool ParseString (char *pPos, char *pPosEnd, char **retpPos, CDatum *retdArg);
		static bool ParseStruct (char *pPos, char *pPosEnd, char **retpPos, CDatum *retdArg);
		static void PrintUTF8 (const CString sString);

		CArchonProcess *m_pProcess;
		bool m_bHousekeeping = false;
		CSemaphore m_Busy;
	};

class CEventProcessingThread : public TThread<CEventProcessingThread>
	{
	public:
		CEventProcessingThread (CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent);

		void AddEvent (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket);
		void Mark (void) { }
		void WaitForPause (void) { m_PausedEvent.Wait(); }

		void Run (void);

	private:
		struct SEvent
			{
			CString sName;					//	Name of event registration
			COSObject *pObj;				//	Event object to wait on

			IArchonMessagePort *pClient;	//	Client port
			CString sMsg;					//	Message
			DWORD dwTicket;					//	Ticket number when event fires
			};

		CCriticalSection m_cs;
		TArray<SEvent> m_Events;
		TArray<SEvent> m_NewEvents;

		CManualEvent *m_pRunEvent;			//	When set, thread should run
		CManualEvent *m_pPauseEvent;		//	When set, thread should stop
		CManualEvent *m_pQuitEvent;			//	When set, thread should quit

		CManualEvent m_PausedEvent;			//	If set, then the thread has stopped
		CManualEvent m_RefreshEvent;		//	If set, then the event list has changed.
	};

class CInterprocessMessageThread : public TThread<CInterprocessMessageThread>
	{
	public:
		CInterprocessMessageThread (void);

		void Boot (IArchonProcessCtx *pProcess, const CString &sName, CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent);
		void Mark (void) { }
		void WaitForPause (void) { m_PausedEvent.Wait(); }

		void Run (void);

	private:
		IArchonProcessCtx *m_pProcess;		//	Parent process
		CString m_sName;					//	Name of process
		CInterprocessMessageQueue m_Queue;	//	Queue;
		int m_iProcessingChunk;				//	Number of events to pull at one time

		CManualEvent *m_pRunEvent;			//	When set, thread should run
		CManualEvent *m_pPauseEvent;		//	When set, thread should stop
		CManualEvent *m_pQuitEvent;			//	When set, thread should quit

		CManualEvent m_PausedEvent;			//	If set, then the thread has stopped
	};

class CMsgProcessKeepAlive : public IProgressEvents
	{
	public:
		CMsgProcessKeepAlive (CMsgProcessCtx &Ctx) :
				m_Ctx(Ctx),
				m_dwStart(sysGetTickCount64())
			{ }

		virtual void OnProgress (int iPercent, const CString &sStatus) override;

	private:
		CMsgProcessCtx &m_Ctx;
		DWORDLONG m_dwStart;
	};