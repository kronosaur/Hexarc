//	CEsperEngine.cpp
//
//	CEsperEngine class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	IMPLEMENTATION
//
//	1.	A listener gets a connection and returns a socket (in CEsperListenerThread::Run)
//
//	2.	We wrap the socket in a CEsperConnection object and add it to the connection
//		manager (m_Connections)
//
//		The manager adds the socket handle to the IO completion port. This allows
//		us to wait on the IOCP and get a status for the socket (later in step 7).
//
//		We return an ID to our clients (via Esper.onConnect).
//
//	3.	A CEsperConnection object is-a CIOCPSocket, which is-a IIOCPEntry.
//
//	4.	To read, we call the connection manager and ask it to read on the given 
//		CEsperConnection object. This in turn call BeginOperation on the object (with
//		opRead). This just initializes some state in the object (handled by IIOCPEntry).
//
//	5.	Next we call BeginRead on the object. This is again handled by IIOCPEntry.
//		We call OnBeginRead, which is handled by CIOCPSocket, and which in turn calls
//		OnSocketBeginRead (which just stores some context).
//
//	6.	BeginRead continues by calling an overlapped ::ReadFile.
//
//	7.	Our processing thread calls Process on the connection manager. This calls
//		::GetQueuedCompletionStatus, which returns a CEsperConnection object that
//		completed processing.
//
//	8.	We call OperationCompleted on the object, which replies to the original message.

#include "stdafx.h"

const int INITIAL_PROCESSING_THREADS =					6;
const int DEFAULT_BUFFER_SIZE =							16 * 1024;
const int DEFAULT_QUEUE_SIZE =							1000;

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null")

DECLARE_CONST_STRING(VIRTUAL_PORT_ESPER_COMMAND,		"Esper.command")
DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_ESPER,					"Esper")

DECLARE_CONST_STRING(FIELD_CLASS,						"class")
DECLARE_CONST_STRING(FIELD_DATE,						"date")
DECLARE_CONST_STRING(FIELD_PEAK_CONNECTIONS,			"peakConnections")
DECLARE_CONST_STRING(FIELD_PEAK_RECEIVED,				"peakReceived")
DECLARE_CONST_STRING(FIELD_PEAK_SENT,					"peakSent")
DECLARE_CONST_STRING(FIELD_STATUS,						"status")
DECLARE_CONST_STRING(FIELD_TOTAL_CONNECTIONS,			"totalConnections")
DECLARE_CONST_STRING(FIELD_TOTAL_RECEIVED,				"totalReceived")
DECLARE_CONST_STRING(FIELD_TOTAL_SENT,					"totalSent")

DECLARE_CONST_STRING(MSG_ARC_FILE_MSG,					"Arc.fileMsg")
DECLARE_CONST_STRING(MSG_ARC_GET_STATUS,				"Arc.getStatus")
DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")
DECLARE_CONST_STRING(MSG_ARC_SANDBOX_MSG,				"Arc.sandboxMsg")
DECLARE_CONST_STRING(MSG_ERROR_NO_RIGHT,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_ESPER_AMP1,					"Esper.amp1")
DECLARE_CONST_STRING(MSG_ESPER_AMP1_DISCONNECT,			"Esper.amp1Disconnect")
DECLARE_CONST_STRING(MSG_ESPER_DISCONNECT,				"Esper.disconnect")
DECLARE_CONST_STRING(MSG_ESPER_GET_USAGE_HISTORY,		"Esper.getUsageHistory")
DECLARE_CONST_STRING(MSG_ESPER_HTTP,					"Esper.http")
DECLARE_CONST_STRING(MSG_ESPER_ON_CONNECT,				"Esper.onConnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_DISCONNECT,			"Esper.onDisconnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_READ,					"Esper.onRead")
DECLARE_CONST_STRING(MSG_ESPER_ON_WRITE,				"Esper.onWrite")
DECLARE_CONST_STRING(MSG_ESPER_READ,					"Esper.read")
DECLARE_CONST_STRING(MSG_ESPER_SET_CONNECTION_PROPERTY,	"Esper.setConnectionProperty")
DECLARE_CONST_STRING(MSG_ESPER_START_LISTENER,			"Esper.startListener")
DECLARE_CONST_STRING(MSG_ESPER_STOP_LISTENER,			"Esper.stopListener")
DECLARE_CONST_STRING(MSG_ESPER_WRITE,					"Esper.write")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PROTOCOL_AMP1,						"amp1")
DECLARE_CONST_STRING(PROTOCOL_RAW,						"raw")
DECLARE_CONST_STRING(PROTOCOL_TLS,						"tls")

DECLARE_CONST_STRING(STR_ESPER_ENGINE,					"CEsperEngine")

DECLARE_CONST_STRING(WORKER_SIGNAL_SHUTDOWN,			"Worker.shutdown")

DECLARE_CONST_STRING(ERR_DUPLICATE_LISTENER_NAME,		"Duplicate listener name.")
DECLARE_CONST_STRING(ERR_INVALID_LISTENER_NAME,			"Invalid listener name.")
DECLARE_CONST_STRING(ERR_LISTENER_NOT_FOUND,			"Unable to find listener: %s.")
DECLARE_CONST_STRING(ERR_READ_FAILED,					"Read failed: %s")
DECLARE_CONST_STRING(ERR_CONNECT_FAILED,				"Unable to connect: %s")
DECLARE_CONST_STRING(ERR_CREATE_SOCKET_FAILED,			"Unable to create a socket.")
DECLARE_CONST_STRING(ERR_MSG_TIMING,					"Esper: %s took %d ms to process.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND_MESSAGE,		"Unable to send Esper message to engine.")
DECLARE_CONST_STRING(ERR_UNKNOWN_PROTOCOL,				"Unknown protocol: %s.")
DECLARE_CONST_STRING(ERR_WRITE_FAILED,					"Write failed: %s")
DECLARE_CONST_STRING(ERR_CRASH_MSG,						"CRASH: Esper crashed processing %s.")
DECLARE_CONST_STRING(ERR_INTERNAL_SERVER_ERROR,			"Internal server error.")
DECLARE_CONST_STRING(ERR_INVALID_MSG,					"Esper: Unhandled message: %s.")
DECLARE_CONST_STRING(ERR_NOT_ADMIN,						"Service does not have Arc.admin right.")

class CWorkerShutdownEvent : public IIOCPEntry
	{
	public:
		CWorkerShutdownEvent (void) : IIOCPEntry(WORKER_SIGNAL_SHUTDOWN)
			{ }
	};

CEsperEngine::CEsperEngine (void) : 
		m_pProcess(NULL), 
		m_bShutdown(false),
		m_Queue(DEFAULT_QUEUE_SIZE)

//	CSimpleEngine constructor

	{
#ifdef DEBUG_ESPER
	m_bLogTrace = true;
#else

	//	LATER: Allow this to be changed at runtime.
	m_bLogTrace = false;
#endif

	m_pWorkerShutdown.Set(new CWorkerShutdownEvent);
	}

CEsperEngine::~CEsperEngine (void)

//	CSimpleEngine destructor

	{
	int i;

	for (i = 0; i < m_Listeners.GetCount(); i++)
		delete m_Listeners[i];

	for (i = 0; i < m_Workers.GetCount(); i++)
		delete m_Workers[i];

	for (i = 0; i < m_Processors.GetCount(); i++)
		delete m_Processors[i];
	}

CMessagePort *CEsperEngine::Bind (const CString &sAddr)

//	Bind
//
//	Bind an address

	{
	return m_pProcess->Bind(sAddr);
	}

void CEsperEngine::Boot (IArchonProcessCtx *pProcess, DWORD dwID)

//	Boot
//
//	Boot the engine

	{
	CString sError;

	m_pProcess = pProcess;
	m_dwID = dwID;

	//	Initialize SSLCtx.
	//	LATER: This should never fail, but if it does, we should do something.

	m_DefaultSSLCtx.Init();
	m_DefaultSSLCtx.SetSNICallback(OnServerNameIndication, (DWORD_PTR)this);

	//	Initialize connection manager

	m_Connections.SetArchonCtx(pProcess);

	//	Add our ports

	m_pProcess->AddPort(ADDRESS_ESPER_COMMAND, this);
	m_pProcess->AddVirtualPort(VIRTUAL_PORT_ESPER_COMMAND, ADDRESS_ESPER_COMMAND, FLAG_PORT_NEAREST);
	}

void CEsperEngine::DeleteListener (const SArchonMessage &Msg)

//	DeleteListener
//
//	Deletes the listener

	{
	CSmartLock Lock(m_cs);

	//	Get the name

	CString sName = Msg.dPayload.GetElement(0);

	//	Find the listener

	int iIndex;
	if (!FindListener(sName, &iIndex))
		{
		//	If we're in shutdown, then it is OK that we can't find
		//	a listener. Otherwise we return an error

		if (!m_bShutdown)
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_LISTENER_NOT_FOUND, sName), Msg);
		return;
		}

	//	Ask the listener to stop

	m_Listeners[iIndex]->SignalShutdown();

	//	Done (we will reply when the thread shuts down)
	}

bool CEsperEngine::FindListener (const CString &sName, int *retiPos)

//	FindListener
//
//	Looks for a listener of the given name

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Listeners.GetCount(); i++)
		if (strEquals(sName, m_Listeners[i]->GetName()))
			{
			if (retiPos)
				*retiPos = i;

			return true;
			}

	return false;
	}

const CString &CEsperEngine::GetName (void)

//	GetName
//
//	Returns the engine name.

	{
	return ENGINE_NAME_ESPER;
	}

CDatum CEsperEngine::GetPortStatus (void) const

//	GetPortStatus
//
//	Returns a struct with well-known fields for status.

	{
	CDatum dStatus = CDatum(CDatum::typeStruct);
	dStatus.SetElement(FIELD_CLASS, STR_ESPER_ENGINE);
	dStatus.SetElement(FIELD_STATUS, NULL_STR);

	return dStatus;
	}

void CEsperEngine::GetThreadStatus (int iThread, SThreadStatus *retStatus)

//	GetThreadStatus
//
//	Returns the status of the given thread.

	{
	if (iThread < 0 || iThread >= m_Processors.GetCount())
		{
		retStatus->iState = processingUnknown;
		retStatus->dwDuration = 0;
		return;
		}

	retStatus->iState = m_Processors[iThread]->GetState(&retStatus->dwDuration);
	}

bool CEsperEngine::IsIdle (void)

//	IsIdle
//
//	Returns TRUE if the engine is idle

	{
	int i;

	try
		{
		for (i = 0; i < m_Processors.GetCount(); i++)
			{
			SThreadStatus Status;
			GetThreadStatus(i, &Status);
			if (Status.iState != processingWaiting)
				return false;
			}
		}
	catch (...)
		{
		return false;
		}

	return true;
	}

void CEsperEngine::LogTrace (const CString &sText)

//	LogTrace
//
//	Log trace messages

	{
#ifdef DEBUG_ESPER
	printf("%s\n", (LPSTR)sText);
#endif
	}

void CEsperEngine::Mark (void)

//	Mark
//
//	Mark all AEON data in use

	{
	for (int i = 0; i < m_Listeners.GetCount(); i++)
		m_Listeners[i]->Mark();

	for (int i = 0; i < m_Workers.GetCount(); i++)
		m_Workers[i]->Mark();

	m_Connections.Mark();
	}

void CEsperEngine::MsgEsperAMP1 (const SArchonMessage &Msg)

//	MsgEsperAMP1
//
//	Esper.amp1 {machine-address} {command} {data}

	{
	CString sFullAddress = Msg.dPayload.GetElement(0);
	CString sCommand = Msg.dPayload.GetElement(1);
	CDatum dData = Msg.dPayload.GetElement(2);
	CString sAuthName = Msg.dPayload.GetElement(3);
	CIPInteger AuthKey = Msg.dPayload.GetElement(4);

	//	Don't log, because we might recurse when sending a log message

#if 0
#ifdef DEBUG
	Log(MSG_LOG_DEBUG, strPattern("Send AMP1 %s to %s.", sCommand, sFullAddress));
#endif
#endif

	CString sError;
	if (!m_Connections.BeginAMP1Request(Msg, sFullAddress, sCommand, dData, sAuthName, AuthKey, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	We reply when the async operation completes (or we get an error).
	}

void CEsperEngine::MsgEsperAMP1Disconnect (const SArchonMessage &Msg)

//	MsgEsperAMP1Disconnect
//
//	Esper.amp1Disconnect {machine-address}

	{
	m_Connections.DeleteConnectionByAddress(Msg.dPayload.GetElement(0));
	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CEsperEngine::MsgEsperDisconnect (const SArchonMessage &Msg)

//	MsgEsperDisconnect
//
//	Disconnects

	{
	CDatum dConnection = Msg.dPayload.GetElement(0);

#ifdef DEBUG_SOCKET_OPS
	if (m_bLogTrace)
		LogTrace(strPattern("[%x] Request disconnect", CEsperInterface::ConnectionToFriendlyID(dConnection)));
#endif

	m_pProcess->SendMessageReply(MSG_ESPER_ON_DISCONNECT, CDatum(), Msg);
	m_Connections.DeleteConnection(dConnection);
	}

void CEsperEngine::MsgEsperGetUsageHistory (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgEsperGetUsageHistory
//
//	Returns an array of struct, each struct has the following fields:
//
//	date: The datetime of the sample
//	totalConnections: Total connections this hour
//	peakConnections: Max connections per second
//	totalReceived: Total bytes received this hour (100,000 bytes)
//	peakReceived: Max bytes per second
//	totalSent: Total bytes sent this hour (100,000 bytes)
//	peakSent: Max bytes per second

	{
	int i;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Figure out how many hours of history the caller wants

	int iHours;
	if (Msg.dPayload.GetElement(0).IsNil())
		iHours = 24;
	else
		iHours = Max(0, (int)Msg.dPayload.GetElement(0));

	//	Get the data

	TArray<CEsperStats::SAllStats> Results;
	m_Connections.GetStats().GetHistory(iHours, Results);
	if (Results.GetCount() == 0)
		{
		SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);
		return;
		}

	//	Convert to a datum

	CDatum dResults(CDatum::typeArray);
	dResults.GrowToFit(Results.GetCount());
	for (i = 0; i < Results.GetCount(); i++)
		{
		CDatum dRecord(CDatum::typeStruct);
		dRecord.SetElement(FIELD_DATE, Results[i].Time);
		dRecord.SetElement(FIELD_TOTAL_CONNECTIONS, Results[i].dwConnectionsIn);
		dRecord.SetElement(FIELD_PEAK_CONNECTIONS, Results[i].dwPeakConnectionsIn);
		dRecord.SetElement(FIELD_TOTAL_RECEIVED, Results[i].dwTotalRead);
		dRecord.SetElement(FIELD_PEAK_RECEIVED, Results[i].dwPeakRead);
		dRecord.SetElement(FIELD_TOTAL_SENT, Results[i].dwTotalWritten);
		dRecord.SetElement(FIELD_PEAK_SENT, Results[i].dwPeakWritten);

		dResults.Append(dRecord);
		}

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dResults, Msg);
	}

void CEsperEngine::MsgEsperHTTP (const SArchonMessage &Msg)

//	MsgEsperHTTP
//
//	Issues an HTTP request

	{
	CString sRequest = Msg.dPayload.GetElement(0);
	CString sURL = Msg.dPayload.GetElement(1);
	CDatum dHeaders = Msg.dPayload.GetElement(2);
	CDatum dBody = Msg.dPayload.GetElement(3);
	CDatum dOptions = Msg.dPayload.GetElement(4);

	//	Begin request

	CString sError;
	if (!m_Connections.BeginHTTPRequest(Msg, sRequest, sURL, dHeaders, dBody, dOptions, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	We reply when the async operation completes (or we get an error).
	}

void CEsperEngine::MsgEsperRead (const SArchonMessage &Msg)

//	MsgEsperRead
//
//	Requests a read on the given connection. If we fail, we return an error
//	to the caller.

	{
	CDatum dConnection = Msg.dPayload.GetElement(0);

	CString sError;
	if (!m_Connections.BeginRead(Msg, dConnection, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_READ_FAILED, sError), Msg);
		if (m_bLogTrace)
			LogTrace(strPattern("[%x] Read failed: %s", CEsperInterface::ConnectionToFriendlyID(dConnection), sError));

		return;
		}

#ifdef DEBUG_SOCKET_OPS
	if (m_bLogTrace)
		LogTrace(strPattern("[%x] Read", CEsperInterface::ConnectionToFriendlyID(dConnection)));
#endif

	//	We reply when the async operation completes (or we get an error).
	}

void CEsperEngine::MsgEsperSetConnectionProperty (const SArchonMessage &Msg)

//	MsgEsperSetConnectionProperty
//
//	Esper.setConnectionProperty {socket} {property} {value}

	{
	CDatum dConnection = Msg.dPayload.GetElement(0);
	CString sProperty = Msg.dPayload.GetElement(1);
	CDatum dValue = Msg.dPayload.GetElement(2);

	CString sError;
	if (!m_Connections.SetProperty(dConnection, sProperty, dValue, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CEsperEngine::MsgEsperStartListener (const SArchonMessage &Msg)

//	MsgEsperStartListener
//
//	Esper.startListener {name} {port} [{protocol}]

	{
	CSmartLock Lock(m_cs);

	//	If we're in shutdown, then ignore this message

	if (m_bShutdown)
		return;

	//	Get some parameters

	CString sName = Msg.dPayload.GetElement(0);
	CString sProtocol = Msg.dPayload.GetElement(2);

	//	Get the type from the protocol

	CEsperConnection::ETypes iType;
	if (sProtocol.IsEmpty() || strEquals(sProtocol, PROTOCOL_RAW))
		iType = CEsperConnection::typeRawIn;
	else if (strEquals(sProtocol, PROTOCOL_AMP1))
		iType = CEsperConnection::typeAMP1In;
	else if (strEquals(sProtocol, PROTOCOL_TLS))
		iType = CEsperConnection::typeTLSIn;
	else
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNKNOWN_PROTOCOL, sProtocol), Msg);
		return;
		}

	//	Make sure that we don't already have a listener with this name

	if (sName.IsEmpty() || FindListener(sName))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, 
				(sName.IsEmpty() ? ERR_INVALID_LISTENER_NAME : ERR_DUPLICATE_LISTENER_NAME), 
				Msg);
		return;
		}

	//	Add a new thread (array takes ownership of the context)

	CEsperListenerThread *pThread = new CEsperListenerThread(this, iType, Msg);
	m_Listeners.Insert(pThread);
	pThread->Start();
	}

void CEsperEngine::MsgEsperWrite (const SArchonMessage &Msg)

//	MsgEsperWrite
//
//	Requests a write on the given connection.

	{
	CDatum dConnection = Msg.dPayload.GetElement(0);
	const CString &sData = Msg.dPayload.GetElement(1);

	CString sError;
	if (!m_Connections.BeginWrite(Msg, dConnection, sData, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_WRITE_FAILED, sError), Msg);
		if (m_bLogTrace)
			LogTrace(strPattern("[%x] Write failed (%d bytes): %s", CEsperInterface::ConnectionToFriendlyID(dConnection), sData.GetLength(), sError));

		return;
		}

#ifdef DEBUG_SOCKET_OPS
	if (m_bLogTrace)
		LogTrace(strPattern("[%x] Write %d bytes", CEsperInterface::ConnectionToFriendlyID(dConnection), sData.GetLength()));
#endif

	//	We reply when the async operation completes (or we get an error).
	}

void CEsperEngine::MsgGetStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetStatus
//
//	Arc.getStatus

	{
	//	Ask the connections for a status

	CEsperConnection::SStatus Status;
	m_Connections.GetStatus(&Status);

	//	Compose into a struct

	CComplexStruct *pResult = new CComplexStruct;
	pResult->SetElement(CString("Esper/connectionActive"), CDatum(Status.iActive));
	pResult->SetElement(CString("Esper/connectionCount"), CDatum(Status.iTotalObjects));
	pResult->SetElement(CString("Esper/connectionsIdle"), CDatum(Status.iIdle));
	pResult->SetElement(CString("Esper/connectionsReading"), CDatum(Status.iWaitingForRead));
	pResult->SetElement(CString("Esper/connectionsWriting"), CDatum(Status.iWaitingForWrite));

	//	Done

	m_pProcess->SendMessageReply(MSG_REPLY_DATA, CDatum(pResult), Msg);
	}

void CEsperEngine::MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgHousekeeping

	{
	CEsperStats &Stats = m_Connections.GetStats();

	//	Update stats so we collect hourly data

	Stats.Update();

#ifdef DEBUG_NET_USAGE
	TAggregatedSample<DWORDLONG> Reads;
	Stats.GetCurrentStat(CEsperStats::statBytesRead, Reads);
	printf("Read %d; Average: %d; Peak: %d\n", (DWORD)Reads.GetTotal(), (DWORD)Reads.GetAverage(), (DWORD)Reads.GetPeak());

	TAggregatedSample<DWORDLONG> Writes;
	Stats.GetCurrentStat(CEsperStats::statBytesWritten, Writes);
	printf("Write %d; Average: %d; Peak: %d\n", (DWORD)Writes.GetTotal(), (DWORD)Writes.GetAverage(), (DWORD)Writes.GetPeak());
#endif
	}

void CEsperEngine::OnConnect (const SArchonMessage &Msg, CSocket &NewSocket, const CString &sListener, CEsperConnection::ETypes iType)

//	OnConnect
//
//	A listener thread has received a connection.

	{
	CEsperConnectionManager::SConnectionCtx Ctx(Msg, m_DefaultSSLCtx);
	m_Connections.CreateConnection(Ctx, NewSocket, sListener, iType);
	}

bool CEsperEngine::OnServerNameIndication (DWORD_PTR dwData, const CString &sServerName, CSSLCtx *retNewCtx)

//	OnServerNameIndication
//
//	This is called on an SSL connection when we've received a server name from 
//	the client. We need to switch context to one with the appropriate certificates.

	{
	CEsperEngine *pEsper = (CEsperEngine *)dwData;

	//	Find the appropriate context and return it.

	if (!pEsper->m_Certificates.GetSSLCtx(sServerName, retNewCtx))
		{
		//	If we could not find the certificate, we create a self-signed one.

		CSSLEnvelopeKey Key;
		if (!Key.Init())
			return false;

		CSSLCert::SDesc CertDesc;
		CertDesc.Subject = CDistinguishedName(strPattern("CN=%s,O=Kronosaur Productions,C=US", sServerName));
		CertDesc.Issuer = CertDesc.Subject;
		CertDesc.ValidTime = CTimeSpan(90, 0);
		CertDesc.Key = Key;
		CSSLCert Cert;
		if (!Cert.InitSelfSigned(CertDesc))
			return false;

		//	Add to the cache and get an SSL Ctx.

		if (!pEsper->m_Certificates.AddCertificate(sServerName, Cert, Key, retNewCtx))
			return false;

		return true;
		}

	return true;
	}

void CEsperEngine::ProcessMessage (const SArchonMessage &Msg, CHexeSecurityCtx *pSecurityCtx)

//	ProcessMessage
//
//	Processes messages

	{
	try
		{
		DWORD dwStartTime = sysGetTickCount();

		//	Arc.fileMsg

		if (strEquals(Msg.sMsg, MSG_ARC_FILE_MSG))
			{
			SArchonMessage NewMsg;
			CInterprocessMessageQueue::DecodeFileMsg(Msg, &NewMsg);
			SendMessage(NewMsg);
			}

		//	Arc.getStatus

		else if (strEquals(Msg.sMsg, MSG_ARC_GET_STATUS))
			MsgGetStatus(Msg, pSecurityCtx);

		//	Arc.housekeeping

		else if (strEquals(Msg.sMsg, MSG_ARC_HOUSEKEEPING))
			MsgHousekeeping(Msg, pSecurityCtx);

		//	Arc.sandboxMsg

		else if (strEquals(Msg.sMsg, MSG_ARC_SANDBOX_MSG))
			{
			SArchonMessage SandboxMsg;
			CHexeSecurityCtx SecurityCtx;

			SandboxMsg.sMsg = Msg.dPayload.GetElement(0);
			SandboxMsg.sReplyAddr = Msg.sReplyAddr;
			SandboxMsg.dwTicket = Msg.dwTicket;
			SandboxMsg.dPayload = Msg.dPayload.GetElement(1);

			SecurityCtx.Init(Msg.dPayload.GetElement(2));

			return ProcessMessage(SandboxMsg, &SecurityCtx);
			}

		//	Esper messages

		else if (strEquals(Msg.sMsg, MSG_ESPER_HTTP))
			MsgEsperHTTP(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_GET_USAGE_HISTORY))
			MsgEsperGetUsageHistory(Msg, pSecurityCtx);

		//	Else, unhandled message

		else
			{
			CString sError = strPattern(ERR_INVALID_MSG, Msg.sMsg);
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			m_pProcess->Log(MSG_LOG_ERROR, sError);
			}

		DWORD dwTime = sysGetTicksElapsed(dwStartTime);
		if (dwTime >= 1000)
			Log(MSG_LOG_INFO, strPattern(ERR_MSG_TIMING, Msg.sMsg, dwTime));
		}
	catch (...)
		{
		m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CRASH_MSG, Msg.sMsg));
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INTERNAL_SERVER_ERROR, Msg);
		}
	}

void CEsperEngine::RemoveListener (const CString &sName)

//	RemoveListener
//
//	Removes the given listener. Socket must be closed.

	{
	int i;

	m_cs.Lock();
	CEsperListenerThread *pThread = NULL;
	for (i = 0; i < m_Listeners.GetCount(); i++)
		if (strEquals(sName, m_Listeners[i]->GetName()))
			{
			pThread = m_Listeners[i];
			m_Listeners.Delete(i);
			break;
			}

	if (pThread == NULL)
		{
		ASSERT(false);
		return;
		}

	//	Remember if we're the last thread

	bool bLastThread = (m_Listeners.GetCount() == 0);

	//	Unlock so that we don't crash when the semaphore
	//	is destroyed.

	m_cs.Unlock();

	//	Delete the thread context

	delete pThread;
	}

bool CEsperEngine::SendMessage (const SArchonMessage &Msg)

//	SendMessage
//
//	Handle messages sent to us

	{
	try
		{
		//	Stop processing messages once we shutdown

		if (m_bShutdown)
			return false;

		//	We handle our messages synchronously since they only post stuff on
		//	queues handled by other threads.

		if (strEquals(Msg.sMsg, MSG_ESPER_AMP1))
			MsgEsperAMP1(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_AMP1_DISCONNECT))
			MsgEsperAMP1Disconnect(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_DISCONNECT))
			MsgEsperDisconnect(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_READ))
			MsgEsperRead(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_SET_CONNECTION_PROPERTY))
			MsgEsperSetConnectionProperty(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_START_LISTENER))
			MsgEsperStartListener(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_STOP_LISTENER))
			DeleteListener(Msg);

		else if (strEquals(Msg.sMsg, MSG_ESPER_WRITE))
			MsgEsperWrite(Msg);

		//	Otherwise, post to our message queue.

		else
			return m_Queue.Enqueue(Msg);

		//	Message is always posted

		return true;
		}
	catch (...)
		{
		m_pProcess->Log(MSG_LOG_ERROR, CString("CRASH: CEsperEngine::SendMessage."));
		return false;
		}
	}

void CEsperEngine::SendMessageReplyData (CDatum dPayload, const SArchonMessage &OriginalMsg) 

//	SendMessageReplyData
//
//	Replies with data
	
	{
	m_pProcess->SendMessageReply(MSG_REPLY_DATA, dPayload, OriginalMsg);
	}

void CEsperEngine::SendMessageReplyDisconnect (const SArchonMessage &OriginalMsg)

//	SendMessageReplyDisconnect
//
//	Reply with Esper.onDisconnect

	{
	m_pProcess->SendMessageReply(MSG_ESPER_ON_DISCONNECT, CDatum(), OriginalMsg);
	}

void CEsperEngine::SendMessageReplyOnRead (CDatum dConnection, CString &sData, const SArchonMessage &OriginalMsg)

//	SendMessageReplyOnRead
//
//	Reply with Esper.onRead

	{
	int iDataLen = sData.GetLength();
	CDatum dData;
	CDatum::CreateStringFromHandoff(sData, &dData);

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dData);
	pPayload->Insert((int)iDataLen);
	pPayload->Insert(dConnection);

	m_pProcess->SendMessageReply(MSG_ESPER_ON_READ, CDatum(pPayload), OriginalMsg);
	}

void CEsperEngine::SendMessageReplyOnWrite (CDatum dConnection, DWORD dwBytesTransferred, const SArchonMessage &OriginalMsg)

//	SendMessageReplyOnWrite
//
//	Reply with Esper.onWrite

	{
	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dwBytesTransferred);
	pPayload->Insert(dConnection);

	m_pProcess->SendMessageReply(MSG_ESPER_ON_WRITE, CDatum(pPayload), OriginalMsg);
	}

void CEsperEngine::SignalShutdown (void)

//	SignalShutdown
//
//	Tell engine to shut down

	{
	int i;

	printf("EsperEngine: SignalShutdown.\n");

	//	Stop all worker threads

	for (i = 0; i < m_Workers.GetCount(); i++)
		m_Connections.SignalEvent(m_pWorkerShutdown);

	//	Close all the sockets so that the listener threads terminate

	m_cs.Lock();
	m_bShutdown = true;
	for (i = 0; i < m_Listeners.GetCount(); i++)
		m_Listeners[i]->SignalShutdown();
	m_cs.Unlock();
	}

void CEsperEngine::SignalPause (void)

//	SignalPause
//
//	Tell engine to pause its threads

	{
	int i;

	//	Stop all worker threads

	for (i = 0; i < m_Workers.GetCount(); i++)
		m_Connections.SignalEvent(m_Workers[i]->GetPauseSignal());
	}

void CEsperEngine::StartRunning (CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent)

//	StartRunning
//
//	Start running

	{
	int i;

	m_pRunEvent = &RunEvent;
	m_pPauseEvent = &PauseEvent;
	m_pQuitEvent = &QuitEvent;

	//	Create some worker threads

	for (i = 0; i < INITIAL_PROCESSING_THREADS; i++)
		{
		CEsperProcessingThread *pThread = new CEsperProcessingThread(this, m_bLogTrace);
		m_Workers.Insert(pThread);
		pThread->Start();
		}

	for (i = 0; i < INITIAL_PROCESSING_THREADS; i++)
		{
		CEsperMsgProcessingThread *pThread = new CEsperMsgProcessingThread(*this);
		m_Processors.Insert(pThread);
		pThread->Start();
		}
	}

bool CEsperEngine::ValidateSandboxAdmin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	ValidateSandboxAdmin
//
//	Validates that the security context has admin rights. If not, then we reply
//	with an error.

	{
	//	If no security context then we are being run by a privileged entity.

	if (pSecurityCtx == NULL)
		return true;

	if (!pSecurityCtx->HasServiceRightArcAdmin())
		{
		SendMessageReplyError(MSG_ERROR_NO_RIGHT, ERR_NOT_ADMIN, Msg);
		return false;
		}

	//	Done

	return true;
	}

void CEsperEngine::WaitForShutdown (void)

//	WaitForShutdown
//
//	Waits until all threads have shut down

	{
	int i;

	//	Wait for all the reader threads to terminate

	for (i = 0; i < m_Workers.GetCount(); i++)
		m_Workers[i]->Wait();

	for (i = 0; i < m_Processors.GetCount(); i++)
		m_Processors[i]->Wait();

	//	If we get here then SignalShutdown has already been called, which
	//	means that we can no longer add listener threads.

	CWaitArray Wait;
	for (i = 0; i < m_Listeners.GetCount(); i++)
		Wait.Insert(*m_Listeners[i]);

	Wait.WaitForAll();
	}

void CEsperEngine::WaitForPause (void)

//	WaitForPause
//
//	Wait for all threads to pause for garbage collection

	{
	int i;

	for (i = 0; i < m_Workers.GetCount(); i++)
		m_Workers[i]->WaitForPause();

	for (i = 0; i < m_Processors.GetCount(); i++)
		m_Processors[i]->WaitForPause();

	for (i = 0; i < m_Listeners.GetCount(); i++)
		m_Listeners[i]->WaitForPause();
	}
