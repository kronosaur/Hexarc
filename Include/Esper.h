//	Esper.h
//
//	Esper Archon Implementation
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#pragma once

#include "OpenSSLUtil.h"

#ifdef DEBUG
//#define DEBUG_ESPER
//#define DEBUG_SOCKET_OPS
//#define DEBUG_AMP1
#endif

class CEsperEngine;

class CEsperConnection : public CIOCPSocket
	{
	public:
		enum ETypes
			{
			typeNone,						//	Unknown or none

			typeAMP1In,						//	Inbound AMP1 connection.
			typeAMP1Out,					//	Outbound AMP1 connection.
			typeHTTPOut,					//	Outbound HTTP connection.
			typeRawIn,						//	Inbound connection, any protocol, client handles everything
			typeTLSIn,						//	Inbound SSL/TLS connection.
			};

		struct SStatus
			{
			SStatus (void) :
					iTotalObjects(0),
					iIdle(0),
					iActive(0),
					iWaitingForRead(0),
					iWaitingForWrite(0)
				{ }

			int iTotalObjects;				//	Total objects associated with port
			int iActive;					//	Objects with recent (<=5 minutes) activity
			int iIdle;						//	Objects with no pending operation
			int iWaitingForRead;			//	Objects waiting for read operation
			int iWaitingForWrite;			//	Objects waiting for write operation
			};

		struct SAMP1Request
			{
			CString sAddress;				//	Address to connect
			DWORD dwPort;					//	Port to connect on
			CString sCommand;				//	Command to send
			CDatum dData;					//	Data

			CString sSenderName;			//	AuthName of sender
			CIPInteger SenderKey;			//	Secret key
			};

		struct SHTTPRequest
			{
			CString sMethod;				//	GET, PUT,etc.
			CString sProtocol;				//	http or https
			CString sHost;					//	www.example.com:8080
			CString sAddress;				//	www.example.com
			CString sPath;					//	/folder/example.html
			DWORD dwPort;					//	e.g., 8080

			CDatum dHeaders;
			CDatum dBody;
			CDatum dOptions;
			};

		CEsperConnection (SOCKET hSocket) : CIOCPSocket(hSocket)
			{ }

		virtual void AccumulateResult (TArray<CString> &Result) { }
		virtual void AccumulateStatus (SStatus *ioStatus) = 0;
		virtual bool BeginAMP1Request (const SArchonMessage &Msg, const SAMP1Request &Request, CString *retsError) { ASSERT(false); return false; }
		virtual bool BeginConnection (const SArchonMessage &Msg, const CString &sAddress, DWORD dwPort, CString *retsError) { ASSERT(false); return false; }
		virtual bool BeginHTTPRequest (const SArchonMessage &Msg, const SHTTPRequest &Request, CString *retsError) { ASSERT(false); return false; }
		virtual bool BeginRead (const SArchonMessage &Msg, CString *retsError) { ASSERT(false); return false; }
		virtual bool BeginWrite (const SArchonMessage &Msg, const CString &sData, CString *retsError) { ASSERT(false); return false; }
		virtual void ClearBusy (void) = 0;
		virtual const CString &GetHostConnection (void) { return NULL_STR; }
		virtual CDatum GetProperty (const CString &sProperty) const { return CDatum(); }
		virtual void OnConnect (void) { }
		virtual bool SetBusy (void) = 0;
		virtual bool SetProperty (const CString &sProperty, CDatum dValue) { return false; }
	};

class CEsperListenerThread : public TThread<CEsperListenerThread>
	{
	public:
		CEsperListenerThread (CEsperEngine *pEngine, CEsperConnection::ETypes iType, const SArchonMessage &Msg);
		~CEsperListenerThread (void);

		const CString &GetName (void) const { return m_sName; }
		void Mark (void) { }
		void Run (void);
		void SignalShutdown (void);
		void WaitForPause (void) { m_PausedEvent.Wait(); }

	private:
		void SendMessageReplyError (const CString &sMsg, const CString &sText);
		void SendMessageReplyText (const CString &sMsg, const CString &sText);

		CCriticalSection m_cs;
		CEsperEngine *m_pEngine;			//	The engine

		CString m_sName;					//	Name of listener (as set by caller)
		CString m_sService;					//	Port number to listen to
		CEsperConnection::ETypes m_iType;	//	Connection type
		SArchonMessage m_OriginalMsg;		//	Message establishing listener

		CString m_sClientAddr;				//	Client address
		CMessagePort *m_pClient;			//	Client port
		DWORD m_dwClientTicket;				//	Ticket to use in replies

		CSocket m_Listener;					//	Socket to use

		bool m_bShutdown;					//	TRUE when we've been asked to shut down
		CManualEvent m_PausedEvent;			//	Set when we've stopped
	};

class CEsperProcessingThread : public TThread<CEsperProcessingThread>
	{
	public:
		CEsperProcessingThread (CEsperEngine *pEngine, bool bLogTrace) : 
				m_pEngine(pEngine), 
				m_pPauseSignal(NULL),
				m_bLogTrace(bLogTrace),
				m_bQuit(false)
			{ m_PausedEvent.Create(); }

		IIOCPEntry *GetPauseSignal (void) const { return m_pPauseSignal; }
		void Mark (void);
		void Run (void);
		void SetQuit (void) { m_bQuit = true; }
		void Stop (void);
		void WaitForPause (void) { m_PausedEvent.Wait(); }

	private:
		CEsperEngine *m_pEngine;			//	The engine
		CManualEvent m_PausedEvent;			//	Set when we've stopped
		IIOCPEntry *m_pPauseSignal;
		bool m_bLogTrace;					//	If TRUE we output tracing info
		bool m_bQuit;						//	If TRUE, we need to quit
	};

class CEsperMsgProcessingThread : public TThread<CEsperMsgProcessingThread>
	{
	public:
		CEsperMsgProcessingThread (CEsperEngine &Engine);

		EProcessingState GetState (DWORD *retdwDuration = NULL);
		void WaitForPause (void) { m_PausedEvent.Wait(); }

		void Run (void);

	private:
		void SetState (EProcessingState iState) { m_iState = iState; m_dwLastStateChange = ::GetTickCount(); }

		CEsperEngine &m_Engine;

		EProcessingState m_iState;			//	Current thread state
		DWORD m_dwLastStateChange;			//	Tick when thread changed state
		CManualEvent m_PausedEvent;			//	If set, then the thread has stopped
	};

class CEsperStats
	{
	public:
		enum EStats
			{
			statNone =						-1,

			statConnectionsIn =				0,
			statBytesRead =					1,
			statBytesWritten =				2,

			statCount =						3,
			};

		struct SAllStats
			{
			CDateTime Time;					//	Date/time of stats
			DWORD dwConnectionsIn;			//	Total connections this hour
			DWORD dwPeakConnectionsIn;		//	Max connections per second
			DWORD dwTotalRead;				//	Total bytes read (100,000 bytes)
			DWORD dwPeakRead;				//	Max bytes per second
			DWORD dwTotalWritten;			//	Total bytes written (100,000 bytes)
			DWORD dwPeakWritten;			//	Max bytes per second
			};

		CEsperStats (void);

		void GetCurrentStat (EStats iStat, TAggregatedSample<DWORDLONG> &Value) const;
		void GetHistory (int iHours, TArray<SAllStats> &History) const;
		void IncStat (EStats iStat, DWORD dwValue = 1);
		void Update (void);

	private:
		enum EConstants
			{
			SAMPLE_TIME =					60000,	//	One sample per minute
			SAMPLE_COUNT =					60,		//	Track the last hour of samples
			};

		DWORD GetTimePoint (void) const { return (DWORD)(::sysGetTickCount64() / (DWORDLONG)SAMPLE_TIME); }
		bool IsHistoryTime (void);

		CCriticalSection m_cs;
		DWORDLONG m_StartTicks;
		CDateTime m_StartTime;
		DWORD m_StartTimePoint;
		TSampleArray<DWORDLONG, SAMPLE_COUNT> m_Stat[statCount];

		CCriticalSection m_csHistory;
		CDateTime m_LastHourlySample;
		TArray<TAggregatedSample<DWORDLONG>> m_HourlyStats[statCount];
	};

class CEsperConnectionManager
	{
	public:
		struct SConnectionCtx
			{
			SConnectionCtx (const SArchonMessage &MsgArg, CSSLCtx &SSLCtxArg) :
					Msg(MsgArg),
					SSLCtx(SSLCtxArg)
				{ }

			const SArchonMessage &Msg;
			CSSLCtx SSLCtx;
			};

		CEsperConnectionManager (void) : 
				m_pArchon(NULL),
				m_dwLastTimeoutCheck(0)
			{ }

		bool AuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) { if (m_pArchon) return m_pArchon->AuthenticateMachine(sMachineName, Key); else return NULL; }
		bool BeginAMP1Request (const SArchonMessage &Msg, const CString &sFullAddress, const CString &sCommand, CDatum dData, const CString &sAuthName, const CIPInteger &AuthKey, CString *retsError);
		bool BeginHTTPRequest (const SArchonMessage &Msg, const CString &sMethod, const CString &sURL, CDatum dHeader, CDatum dBody, CDatum dOptions, CString *retsError);
		bool BeginRead (const SArchonMessage &Msg, CDatum dConnection, CString *retsError);
		bool BeginWrite (const SArchonMessage &Msg, CDatum dConnection, const CString &sData, CString *retsError);
		void CreateConnection (SConnectionCtx &Ctx, CSocket &NewSocket, const CString &sListener, CEsperConnection::ETypes iType);
		void DeleteConnection (CDatum dConnection);
		void DeleteConnectionByAddress (const CString sAddress);
		void GetResults (TArray<CString> &Results);
		void GetStatus (CEsperConnection::SStatus *retStatus);
		CEsperStats &GetStats (void) { return m_Stats; }
		bool IsIdle (void);
		void Log (const CString &sMsg, const CString &sText) { if (m_pArchon) m_pArchon->Log(sMsg, sText); }
		void LogTrace (const CString &sText);
		bool Process (void);
		void ResetConnection (CDatum dConnection);
		void SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload) { if (m_pArchon) m_pArchon->SendMessageCommand(sAddress, sMsg, sReplyAddr, dwTicket, dPayload); }
		void SendMessageReply (const CString &sReplyMsg, CDatum dPayload, const SArchonMessage &OriginalMsg) { if (m_pArchon) m_pArchon->SendMessageReply(sReplyMsg, dPayload, OriginalMsg); }
		void SendMessageReplyData (CDatum dPayload, const SArchonMessage &OriginalMsg);
		void SendMessageReplyDisconnect (const SArchonMessage &OriginalMsg);
		void SendMessageReplyError (const CString &sMsg, const CString &sText, const SArchonMessage &OriginalMsg) { if (m_pArchon) m_pArchon->SendMessageReply(sMsg, CDatum(sText), OriginalMsg); }
		void SendMessageReplyOnConnect (CDatum dConnection, const CString &sListener, const CString &sAddressName, const SArchonMessage &OriginalMsg);
		void SendMessageReplyOnRead (CDatum dConnection, CString &sData, const SArchonMessage &OriginalMsg);
		void SendMessageReplyOnWrite (CDatum dConnection, DWORD dwBytesTransferred, const SArchonMessage &OriginalMsg);
		void SetArchonCtx (IArchonProcessCtx *pArchon) { m_pArchon = pArchon; }
		bool SetProperty (CDatum dConnection, const CString &sProperty, CDatum dValue, CString *retsError);
		void SignalEvent (IIOCPEntry *pObject) { m_IOCP.SignalEvent(pObject); }

	private:
		void AddConnection (CEsperConnection *pConnection, CDatum *retdConnection = NULL);
		bool BeginAMP1Operation (const CString &sHostConnection, CEsperConnection **retpConnection, CString *retsError);
		bool BeginHTTPOperation (const CString &sHostConnection, CEsperConnection **retpConnection, CString *retsError);
		bool BeginOperation (CEsperConnection *pEntry, IIOCPEntry::EOperations iOp);
		bool BeginOperation (CDatum dConnection, IIOCPEntry::EOperations iOp, CEsperConnection **retpConnection, CString *retsError);
		void DeleteConnection (CEsperConnection *pConnection);
		bool DeleteIfMarked (IIOCPEntry *pConnection);
		bool FindConnection (CDatum dConnection, CEsperConnection **retpConnection);
		bool FindOutboundConnection (const CString &sHostConnection, CEsperConnection **retpConnection);
		void TimeoutCheck (void);

		CCriticalSection m_cs;
		IArchonProcessCtx *m_pArchon;
		TIDTable<CEsperConnection *> m_Connections;		//	All connections; owns the connection object
		TArray<CEsperConnection *> m_Outbound;			//	List of outbound connections (cached from m_Connections)
		CIOCompletionPort m_IOCP;
		CEsperStats m_Stats;

		DWORDLONG m_dwLastTimeoutCheck;			//	Tick on which we last checked for timeouts
	};

class CEsperCertificateCache
	{
	public:
		CEsperCertificateCache (void) { }

		bool AddCertificate (const CString &sDomain, CDatum dCert, CString *retsError = NULL);
		bool AddCertificate (const CString &sDomain, CSSLCert &Cert, CSSLEnvelopeKey &Key, CSSLCtx *retCtx = NULL, CString *retsError = NULL);
		bool GetSSLCtx (const CString &sDomain, CSSLCtx *retCtx) const;

	private:
		CCriticalSection m_cs;
		TSortMap<CString, CSSLCtx> m_Certs;
	};

class CEsperEngine : public IArchonEngine, public IArchonMessagePort
	{
	public:
		CEsperEngine (void);
		virtual ~CEsperEngine (void);

		//	IArchonMessagePort
		virtual CDatum GetPortStatus (void) const override;
		virtual bool SendMessage (const SArchonMessage &Msg);

		//	IArchonEngine
		virtual void Boot (IArchonProcessCtx *pProcess, DWORD dwID) override;
		virtual const CString &GetName (void) override;
		virtual bool IsIdle (void) override;
		virtual void Mark (void) override;
		virtual void StartRunning (CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent) override;
		virtual void SignalShutdown (void) override;
		virtual void SignalPause (void) override;
		virtual void WaitForShutdown (void) override;
		virtual void WaitForPause (void) override;

		//	Functions used by CEsperListenerThread
		bool AuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) { return m_pProcess->AuthenticateMachine(sMachineName, Key); }
		CMessagePort *Bind (const CString &sAddr);
		void DeleteConnection (CDatum dConnection) { m_Connections.DeleteConnection(dConnection); }
		bool Dequeue (int iMaxCount, CArchonMessageList *retList) { return m_Queue.Dequeue(iMaxCount, retList); }
		CManualEvent &GetQueueEvent (void) { return m_Queue.GetEvent(); }
		CManualEvent &GetQuitEvent (void) { return *m_pQuitEvent; }
		CManualEvent &GetRunEvent (void) { return *m_pRunEvent; }
		CManualEvent &GetPauseEvent (void) { return *m_pPauseEvent; }
		IArchonProcessCtx *GetProcessCtx (void) { return m_pProcess; }
		void Log (const CString &sMsg, const CString &sText) { m_pProcess->Log(sMsg, sText); }
		void LogTrace (const CString &sText);
		void OnConnect (const SArchonMessage &Msg, CSocket &NewSocket, const CString &sListener, CEsperConnection::ETypes iType);
		bool ProcessConnections (void) { return m_Connections.Process(); }
		void ProcessMessage (const SArchonMessage &Msg, CHexeSecurityCtx *pSecurityCtx = NULL);
		void ReportShutdown (const CString &sName) { RemoveListener(sName); }
		void ResetConnection (CDatum dConnection) { m_Connections.ResetConnection(dConnection); }

		//	Helpers
		void SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload) { m_pProcess->SendMessageCommand(sAddress, sMsg, sReplyAddr, dwTicket, dPayload); }
		void SendMessageReply (const CString &sReplyMsg, CDatum dPayload, const SArchonMessage &OriginalMsg) { m_pProcess->SendMessageReply(sReplyMsg, dPayload, OriginalMsg); }
		void SendMessageReplyDisconnect (const SArchonMessage &OriginalMsg);
		void SendMessageReplyData (CDatum dPayload, const SArchonMessage &OriginalMsg);
		void SendMessageReplyError (const CString &sMsg, const CString &sText, const SArchonMessage &OriginalMsg) { m_pProcess->SendMessageReply(sMsg, CDatum(sText), OriginalMsg); }
		void SendMessageReplyOnRead (CDatum dConnection, CString &sData, const SArchonMessage &OriginalMsg);
		void SendMessageReplyOnWrite (CDatum dConnection, DWORD dwBytesTransferred, const SArchonMessage &OriginalMsg);

	private:
		struct SThreadStatus
			{
			EProcessingState iState;
			DWORD dwDuration;
			};

		void DeleteListener (const SArchonMessage &Msg);
		bool FindListener (const CString &sName, int *retiPos = NULL);
		void GetThreadStatus (int iThread, SThreadStatus *retStatus);
		void RemoveListener (const CString &sName);
		bool ValidateSandboxAdmin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		void MsgEsperAMP1 (const SArchonMessage &Msg);
		void MsgEsperAMP1Disconnect (const SArchonMessage &Msg);
		void MsgEsperDisconnect (const SArchonMessage &Msg);
		void MsgEsperGetUsageHistory (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgEsperHTTP (const SArchonMessage &Msg);
		void MsgEsperRead (const SArchonMessage &Msg);
		void MsgEsperSetConnectionProperty (const SArchonMessage &Msg);
		void MsgEsperStartListener (const SArchonMessage &Msg);
		void MsgEsperWrite (const SArchonMessage &Msg);
		void MsgGetStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		static bool OnServerNameIndication (DWORD_PTR dwData, const CString &sServerName, CSSLCtx *retNewCtx);

		IArchonProcessCtx *m_pProcess;		//	Parent process
		DWORD m_dwID;						//	Our ID
		bool m_bShutdown;					//	TRUE if shutdown signalled
		bool m_bLogTrace;					//	If TRUE we output tracing info

		CEsperConnectionManager m_Connections;	//	List of connections

		CEsperCertificateCache m_Certificates;
		CSSLCtx m_DefaultSSLCtx;			//	Default context

		CCriticalSection m_cs;				//	Protects adding/removing listeners
		TArray<CEsperListenerThread *> m_Listeners;		//	List of listener threads
		TArray<CEsperProcessingThread *> m_Workers;		//	List of worker threads

		TArray<CEsperMsgProcessingThread *> m_Processors;	//	List of message processors
		CMessageQueue m_Queue;				//	When set, there are messages in the queue

		CManualEvent *m_pRunEvent;
		CManualEvent *m_pPauseEvent;
		CManualEvent *m_pQuitEvent;

		TUniquePtr<IIOCPEntry> m_pWorkerShutdown;
	};
