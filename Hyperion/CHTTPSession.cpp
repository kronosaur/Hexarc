//	CHTTPSession.cpp
//
//	CHTTPSession class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	SEE ALSO
//
//	http://www.w3.org/Protocols/rfc2616/rfc2616.html

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_SESSION_TIMING
#endif

const int DEFAULT_TIMEOUT =								30 * 1000;
const int LONG_POLL_TIMEOUT =							120 * 100;

DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command");

DECLARE_CONST_STRING(EXTENSION_HEXM,					".hexm");

DECLARE_CONST_STRING(FIELD_CLIENT_ADDR,					"clientAddr");
DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc");
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath");
DECLARE_CONST_STRING(FIELD_HTTP_METHOD,					"method");
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn");
DECLARE_CONST_STRING(FIELD_PROTOCOL,					"protocol");
DECLARE_CONST_STRING(FIELD_REQUEST_TIME,				"requestTime");
DECLARE_CONST_STRING(FIELD_SIZE,						"size");
DECLARE_CONST_STRING(FIELD_SOCKET,						"socket");
DECLARE_CONST_STRING(FIELD_STATE,						"state");
DECLARE_CONST_STRING(FIELD_UNMODIFIED,					"unmodified");
DECLARE_CONST_STRING(FIELD_HTTP_URL,					"url");

DECLARE_CONST_STRING(HEADER_CF_CONNECTING_IP,			"CF-Connecting-IP");
DECLARE_CONST_STRING(HEADER_CONNECTION,					"Connection");
DECLARE_CONST_STRING(HEADER_DATE,						"Date");
DECLARE_CONST_STRING(HEADER_IF_MODIFIED_SINCE,			"If-Modified-Since");
DECLARE_CONST_STRING(HEADER_LAST_MODIFIED,				"Last-Modified");
DECLARE_CONST_STRING(HEADER_SERVER,						"Server");

DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD,			"Aeon.fileDownload");
DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD_DESC,		"Aeon.fileDownloadDesc");
DECLARE_CONST_STRING(MSG_ERROR_LONG_POLL,				"Error.longPoll");
DECLARE_CONST_STRING(MSG_ERROR_TIMEOUT,					"Error.timeout");
DECLARE_CONST_STRING(MSG_ESPER_DISCONNECT,				"Esper.disconnect");
DECLARE_CONST_STRING(MSG_ESPER_ON_DISCONNECT,			"Esper.onDisconnect");
DECLARE_CONST_STRING(MSG_ESPER_ON_READ,					"Esper.onRead");
DECLARE_CONST_STRING(MSG_ESPER_ON_WRITE,				"Esper.onWrite");
DECLARE_CONST_STRING(MSG_ESPER_READ,					"Esper.read");
DECLARE_CONST_STRING(MSG_ESPER_WRITE,					"Esper.write");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");
DECLARE_CONST_STRING(MSG_REPLY_LONG_POLL,				"Reply.longPoll");

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command");

DECLARE_CONST_STRING(PROTOCOL_HTTP,						"http");

DECLARE_CONST_STRING(STATE_DISCONNECTED,				"disconnected");
DECLARE_CONST_STRING(STATE_RESPONSE_SENT,				"responseSent");
DECLARE_CONST_STRING(STATE_RESPONSE_SENT_PARTIAL,		"responseSentPartial");
DECLARE_CONST_STRING(STATE_UNKNOWN,						"unknown");
DECLARE_CONST_STRING(STATE_WAITING_FOR_FILE_DATA,		"waitingForFileData");
DECLARE_CONST_STRING(STATE_WAITING_FOR_REQUEST,			"waitingForRequest");
DECLARE_CONST_STRING(STATE_WAITING_FOR_RPC,				"waitingForRPC");
DECLARE_CONST_STRING(STATE_WAITING_FOR_RPC_LONG_POLL,	"waitingForRPCLongPoll");

DECLARE_CONST_STRING(STR_CLOSE,							"Close");
DECLARE_CONST_STRING(STR_OK,							"OK");
DECLARE_CONST_STRING(STR_SERVER_VERSION,				"Hexarc/1.0");
DECLARE_CONST_STRING(STR_KEEP_ALIVE,					"Keep-Alive");

DECLARE_CONST_STRING(ERR_304_NOT_MODIFIED,				"Not Modified");
DECLARE_CONST_STRING(ERR_400_BAD_REQUEST,				"Bad Request");
DECLARE_CONST_STRING(ERR_RPC_TIMEOUT,					"No response from RPC message.");
DECLARE_CONST_STRING(ERR_404_NOT_FOUND,					"Not Found");
DECLARE_CONST_STRING(ERR_FILE_RECURSION,				"Too many HEXM file loads to handle a request. Possible infinite recursion.");
DECLARE_CONST_STRING(ERR_CANT_SERIALIZE,				"Unable to serialize response.");
DECLARE_CONST_STRING(ERR_UNEXPECTED_MSG,				"[%x] Unexpected msg in state %d: %s.");
DECLARE_CONST_STRING(ERR_CRASH_PROCESS_FILE_HEXM,		"Crash processing HEXM file: %s.");
DECLARE_CONST_STRING(ERR_CRASH_PROCESS_FILE_RAW,		"Crash processing file: %s.");
DECLARE_CONST_STRING(ERR_CRASH_PROCESS_MSG,				"Crash in state %d processing message: %s.");
DECLARE_CONST_STRING(ERR_CRASH_PROCESS_SERVICE,			"Crash processing service status %d. Message: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN_SESSION_STATE,			"Unknown session state in OnProcessMessage.");
DECLARE_CONST_STRING(ERR_HTTP_SESSION_TIMING,			"[%x] %s%s took %d ms to process.");
DECLARE_CONST_STRING(ERR_LONG_POLL_TIMEOUT,				"TIMEOUT: Long-poll expired.");

const DWORD MAX_SINGLE_BODY_SIZE =						100000;

CHTTPSession::CHTTPSession (CHyperionEngine *pEngine, const CString &sListener, const CString &sProtocol, CDatum dSocket, const CString &sNetAddress) : 
		CHyperionSession(pEngine, sListener, sProtocol, dSocket, sNetAddress)

//	CHTTPSession constructor

	{
	}

bool CHTTPSession::CalcRequestIP (const CHTTPMessage &Msg, CString *retsAddress) const

//	CalcRequestIP
//
//	If this is a connection from CloudFlare, get the original IP address.
//	Returns TRUE if the address is different.

	{
	if (Msg.FindHeader(HEADER_CF_CONNECTING_IP, retsAddress))
		return true;

	//	If not CloudFlare, then nothing

	return false;
	}

bool CHTTPSession::Disconnect (const SArchonMessage &Msg)

//	Disconnect
//
//	Disconnect the socket

	{
	//	Initialize context state

	m_iState = State::disconnected;

	//	Compose message

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);

	m_pEngine->LogSessionState(strPattern("[%x:%x] Disconnect.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket()));

	//	Send

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_DISCONNECT,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

	//	Wait for reply

	return true;
	}

CString CHTTPSession::GetDebugInfo (void) const

//	GetDebugInfo
//
//	Info about a session

	{
	return strPattern("[%x] HTTP: state=%d.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_iState);
	}

bool CHTTPSession::GetRequest (const SArchonMessage &Msg, bool bContinued)

//	GetRequest
//
//	Asks to read a request

	{
	//	Initialize context state

	m_iState = State::waitingForRequest;
	m_Ctx.pSession = this;

	if (!bContinued)
		{
		m_Ctx.Request.InitFromPartialBufferReset(IMediaTypeBuilderPtr(m_Ctx.pBodyBuilder->AddRef()));
		m_Ctx.AdditionalHeaders.DeleteAll();
		}

	//	Compose message

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);

	//	Start by reading from the client (we expect a request)

	m_pEngine->LogSessionState(strPattern("[%x:%x] stateWaitingForRequest %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_ESPER_READ));

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_READ,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

	return true;
	}

CString CHTTPSession::GetRequestDescription (void) const

//	GetRequestDescription
//
//	Returns a string of the form: GET http://hostname/url

	{
	return strPattern("%s %s%s", m_Ctx.Request.GetMethod(), m_Ctx.Request.GetRequestedHost(), m_Ctx.Request.GetRequestedPath());
	}

void CHTTPSession::OnEndSession (DWORD dwTicket)

//	OnEndSession
//
//	Session ends

	{
	if (m_Ctx.pService)
		{
		m_Ctx.pService->DeleteSession(this);
		m_Ctx.pService = NULL;
		}
	}

void CHTTPSession::OnGetHyperionStatusReport (CComplexStruct *pStatus) const

//	OnGetHyperionStatusReport
//
//	Fills in status information.

	{
	pStatus->SetElement(FIELD_PROTOCOL, PROTOCOL_HTTP);

	//	We need to protect access to this information because a different thread
	//	might be setting it right now.

	m_pEngine->GetCS().Lock();
	pStatus->SetElement(FIELD_HTTP_METHOD, m_sLastRequestMethod);
	pStatus->SetElement(FIELD_HTTP_URL, m_sLastRequestURL);
	pStatus->SetElement(FIELD_CLIENT_ADDR, m_sLastRequestIP);
	if (m_dwLastRequestTime != 0)
		pStatus->SetElement(FIELD_REQUEST_TIME, ::sysGetSecondsElapsed(m_dwLastRequestTime));

	pStatus->SetElement(FIELD_SOCKET, strPattern("%x", CEsperInterface::ConnectionToFriendlyID(m_dSocket)));

	switch (m_iState)
		{
		case State::unknown:
			pStatus->SetElement(FIELD_STATE, STATE_UNKNOWN);
			break;

		case State::waitingForRequest:
			pStatus->SetElement(FIELD_STATE, STATE_WAITING_FOR_REQUEST);
			break;

		case State::responseSent:
			pStatus->SetElement(FIELD_STATE, STATE_RESPONSE_SENT);
			break;

		case State::responseSentPartial:
			pStatus->SetElement(FIELD_STATE, STATE_RESPONSE_SENT_PARTIAL);
			break;

		case State::waitingForRPCLongPoll:
			pStatus->SetElement(FIELD_STATE, STATE_WAITING_FOR_RPC_LONG_POLL);
			break;

		case State::waitingForRPCResult:
			pStatus->SetElement(FIELD_STATE, STATE_WAITING_FOR_RPC);
			break;

		case State::waitingForFileData:
			pStatus->SetElement(FIELD_STATE, STATE_WAITING_FOR_FILE_DATA);
			break;

		case State::disconnected:
			pStatus->SetElement(FIELD_STATE, STATE_DISCONNECTED);
			break;

		default:
			pStatus->SetElement(FIELD_STATE, strPattern("Unknown: %d", (int)m_iState));
			break;
		}

	m_pEngine->GetCS().Unlock();
	}

void CHTTPSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	m_Ctx.Mark();

	//	Let our ancestor mark

	CHyperionSession::OnMark();
	}

bool CHTTPSession::ProcessStateDisconnected (const SArchonMessage &Msg)

//	ProcessStateDisconnected
//
//	We've disconnected, so nothing left to do.

	{
	if (strEquals(Msg.sMsg, MSG_ESPER_ON_DISCONNECT))
		{
		m_pEngine->LogSessionState(strPattern("[%x:%x] Received: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Msg.sMsg));
		return false;
		}
	else
		{
		m_pEngine->LogSessionState(strPattern("[%x:%x] Message while disconnected: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Msg.sMsg));
		return false;
		}
	}

bool CHTTPSession::ProcessStateResponseSent (const SArchonMessage &Msg)

//	ProcessStateResponseSent
//
//	We've sent a response and are waiting for confirmation that it was sent
//	successfully.

	{
	//	If we have an error then we've dropped the connection
	//	so there is nothing to do

	if (strEquals(Msg.sMsg, MSG_ESPER_ON_DISCONNECT))
		{
#ifdef LOG_HTTP_SESSION
		GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("[%x] Disconnected while waiting for onWrite: %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), Msg.sMsg));
#endif
		return false;
		}

	//	Error

	else if (IsError(Msg))
		{
		//	LATER: Disconnect?

#ifdef LOG_HTTP_SESSION
		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern("[%x] %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), Msg.dPayload.AsString()));
#endif
		return false;
		}

	//	If this is a write confirmation, then we keep reading
	//	from the session

	else if (strEquals(Msg.sMsg, MSG_ESPER_ON_WRITE))
		{
		m_pEngine->LogSessionState(strPattern("[%x:%x] Received %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_ESPER_ON_WRITE));

		//	If this is a 1.0 server or we were asked to close the connection, 
		//	then we're done.

		if (!m_Ctx.Request.IsHTTP11())
			return Disconnect(Msg);

		//	Otherwise, read more.

		else
			return GetRequest(Msg);
		}
	else
		{
		//	If we get this far, then something went wrong

		m_pEngine->LogSessionState(strPattern("[%x:%x] Unexpected message: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Msg.sMsg));

		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_MSG, CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_iState, Msg.sMsg));
		return false;
		}
	}

bool CHTTPSession::ProcessStateResponseSentPartial (const SArchonMessage &Msg)

//	ProcessStateResponseSentPartial
//
//	We've sent a partial response and we're waiting on acknowledgement before
//	continuing to send more.

	{
	//	If we have an error then we've dropped the connection
	//	so there is nothing to do

	if (strEquals(Msg.sMsg, MSG_ESPER_ON_DISCONNECT))
		{
#ifdef LOG_HTTP_SESSION
		GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("[%x] Disconnected while waiting for onWrite: %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), Msg.sMsg));
#endif
		return false;
		}

	//	Error

	else if (IsError(Msg))
		{
		//	LATER: Disconnect?

#ifdef LOG_HTTP_SESSION
		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern("[%x] %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), Msg.dPayload.AsString()));
#endif
		return false;
		}

	//	If this is a write confirmation then send the next bit of data.
	//	(Until we're done with everything.)

	else if (strEquals(Msg.sMsg, MSG_ESPER_ON_WRITE))
		{
		m_pEngine->LogSessionState(strPattern("[%x:%x] Received: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_ESPER_ON_WRITE));

		//	If we're done writing chunks, then continue reading

		if (m_dwPartialSend >= m_Ctx.Response.GetBodySize())
			return GetRequest(Msg);

		//	Otherwise, send the next chunk

		return SendResponseChunk(Msg, m_Ctx.Response);
		}
	else
		{
		//	If we get this far, then something went wrong

		m_pEngine->LogSessionState(strPattern("[%x:%x] Unexpected message: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Msg.sMsg));

		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_MSG, CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_iState, Msg.sMsg));
		return false;
		}
	}

bool CHTTPSession::ProcessStateWaitingForFileData (const SArchonMessage &Msg)

//	ProcessStateWaitingForFileData
//
//	We've requested a file from Aeon and are waiting for it.

	{
	//	If error, then file download failed

	if (!strEquals(Msg.sMsg, MSG_AEON_FILE_DOWNLOAD_DESC))
		{
		m_Ctx.Response.InitResponse(http_NOT_FOUND, ERR_404_NOT_FOUND);
		return SendResponse(m_Ctx, Msg);
		}

	m_pEngine->LogSessionState(strPattern("[%x:%x] Received: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_AEON_FILE_DOWNLOAD_DESC));

#ifdef DEBUG_FILES
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("[%x]: Received Aeon.fileDownloadDesc filePath=%s%s", 
			Msg.dwTicket, 
			(const CString &)Msg.dPayload.GetElement(FIELD_FILE_DESC).GetElement(FIELD_FILE_PATH),
			(Msg.dPayload.GetElement(FIELD_UNMODIFIED).IsNil() ? NULL_STR : CString(" UNMODIFIED"))));
#endif
	//	If the file has not been modified then we can just return

	if (!Msg.dPayload.GetElement(FIELD_UNMODIFIED).IsNil())
		{
		m_Ctx.Response.InitResponse(http_NOT_MODIFIED, ERR_304_NOT_MODIFIED);
		return SendResponse(m_Ctx, Msg);
		}

	//	Parse result

	CDatum dFileDesc = Msg.dPayload.GetElement(FIELD_FILE_DESC);
	CDatum dData = Msg.dPayload.GetElement(FIELD_DATA);

	//	If we have previous data then combine it

	if (!m_Ctx.dFileData.IsNil())
		{
		m_Ctx.dFileData.Append(dData);
		dData = m_Ctx.dFileData;

		m_Ctx.dFileData = CDatum();
		}

	DWORD dwTotalRead = ((const CString &)dData).GetLength();

	//	Figure out how big the entire file is

	DWORD dwTotalSize = (int)dFileDesc.GetElement(FIELD_SIZE);

	//	If we don't have the entire file yet, we save what we have and send
	//	a command to get more.

	if (dwTotalRead < dwTotalSize)
		{
		m_Ctx.dFileData = dData;

		return SendReadFileRequest(m_Ctx, Msg, ((const CString &)m_Ctx.dFileData).GetLength());
		}

	//	Now that we have the file, process it

	return ProcessFileResult(m_Ctx, dFileDesc, dData, Msg);
	}

bool CHTTPSession::ProcessStateWaitingForRequest (const SArchonMessage &Msg)

//	ProcessStateWaitingForRequest
//
//	We are waiting for a request on the socket and received a message (either
//	some data or a disconnect).

	{
	//	If this is a timeout then reset the timeout (since we can wait for a while
	//	for the client. [LATER: Keep track of how long we've kept the connection open
	//	and close it after a period of inactivity.]

	if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
		{
		ResetTimeout(GenerateAddress(PORT_HYPERION_COMMAND), DEFAULT_TIMEOUT);
		return true;
		}

	//	If we have an error then we've dropped the connection
	//	so there is nothing to do

	else if (strEquals(Msg.sMsg, MSG_ESPER_ON_DISCONNECT))
		{
		//	If we've got a request then maybe we have a bug.

		if (m_Ctx.Request.IsMessagePartial())
			GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("[%x] Incomplete message: %s %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_Ctx.Request.GetMethod(), m_Ctx.Request.GetRequestedPath()));
			
		//	Log a disconnect

#ifdef LOG_HTTP_SESSION
		GetProcessCtx()->Log(MSG_LOG_INFO, strPattern("[%x] Disconnected: %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), Msg.sMsg));
#endif
		return false;
		}

	//	Error

	else if (IsError(Msg))
		{
		//	LATER: Disconnect?

#ifdef LOG_HTTP_SESSION
		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern("[%x] %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), Msg.dPayload.AsString()));
#endif
		return false;
		}

	//	If this is an onRead message then we have some data

	else if (strEquals(Msg.sMsg, MSG_ESPER_ON_READ))
		{
		CDatum dData = Msg.dPayload.GetElement(0);

		m_pEngine->LogSessionState(strPattern("[%x:%x] Received: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_ESPER_ON_READ));

		bool bDebugHTTPParsing = m_pEngine->GetOptions().GetOptionBoolean(CHyperionOptions::optionLogHTTPParsing);
		TArray<CString> DebugLog;

		//	Parse the message. If we get an error, then we reply with
		//	a 400 error.

		bool bSuccess = m_Ctx.Request.InitFromPartialBuffer(CStringBuffer((const CString &)dData), false, (bDebugHTTPParsing ? &DebugLog : NULL));

		if (bDebugHTTPParsing)
			{
			for (int i = 0; i < DebugLog.GetCount(); i++)
				GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("[%x] HTTPParsing: %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), DebugLog[i]));
			}

		if (!bSuccess)
			{
#ifdef LOG_HTTP_SESSION
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern("[%x] %s -> 400 Bad Request.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetRequestDescription()));
			GetProcessCtx()->Log(MSG_LOG_INFO, (const CString &)dData);
#endif
			m_Ctx.Response.InitResponse(http_BAD_REQUEST, ERR_400_BAD_REQUEST);
			return SendResponse(m_Ctx, Msg, FLAG_NO_ERROR_LOG);
			}

		//	If we don't yet have a complete message then request more

		if (!m_Ctx.Request.IsMessageComplete())
			return GetRequest(Msg, true);

		//	Time how long it takes us to process a request

		m_dwStartRequest = ::sysGetTickCount64();

		//	See if this is a proxy request (e.g., via CloudFlare)

		CString sOriginalAddr;
		bool bHasProxyIP = CalcRequestIP(m_Ctx.Request, &sOriginalAddr);
		bool bNewProxyIP = (bHasProxyIP && !strEquals(sOriginalAddr, m_sLastRequestIP));
		bool bFirstRequest = m_sLastRequestIP.IsEmpty();

		//	Remember the request. We need to lock a semaphore because a different
		//	thread might be trying to access the data (via OnGetHyperionStatusReport).

		m_pEngine->GetCS().Lock();
		try
			{
			m_sLastRequestMethod = m_Ctx.Request.GetMethod();
			m_sLastRequestURL = m_Ctx.Request.GetRequestedURL();
			m_sLastRequestIP = (bHasProxyIP ? sOriginalAddr : m_sNetAddress);
			m_dwLastRequestTime = ::sysGetTickCount64();
			}
		catch (...)
			{
			}
		m_pEngine->GetCS().Unlock();

#ifdef LOG_HTTP_SESSION
		if (bNewProxyIP)
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern("[%x] Connected %s (CloudFlare %s).", CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_sLastRequestIP, m_sNetAddress));
		else if (bFirstRequest)
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern("[%x] Connected %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_sNetAddress));
			
		GetProcessCtx()->Log(MSG_LOG_INFO, strPattern("[%x] %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetRequestDescription()));
#endif
		//	Ask the engine for a service to handle this request

		CHTTPService *pService;
		if (!m_pEngine->FindHTTPService(m_sListener, m_Ctx.Request, &pService))
			{
			m_Ctx.Response.InitResponse(http_NOT_FOUND, ERR_404_NOT_FOUND);
			return SendResponse(m_Ctx, Msg);
			}

		//	Connect the service to the session and vice versa

		if (pService != m_Ctx.pService)
			{
			if (m_Ctx.pService)
				m_Ctx.pService->DeleteSession(this);

			m_Ctx.pService = pService;
			m_Ctx.pService->InsertSession(this);
			}

		//	Initialize file recursion counter

		m_Ctx.iFileRecursion = 0;

		//	Ask the service to handle the request. If the handler returned
		//	false then it means that it needs to process a message first

		m_Ctx.pService->HandleRequest(m_Ctx);
		return ProcessServiceResult(m_Ctx, Msg);
		}
	else
		{
		//	If we get this far, then something went wrong

		m_pEngine->LogSessionState(strPattern("[%x:%x] Unexpected message: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Msg.sMsg));

		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_MSG, CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_iState, Msg.sMsg));
		return false;
		}
	}

bool CHTTPSession::ProcessStateWaitingForRPCLongPollResult (const SArchonMessage &Msg)

//	ProcessStateWaitingForRPCLongPollResult
//
//	We've sent out an RPC and are waiting for a reply before continuing.

	{
	//	NOTE: If we get a timeout message here, we just return it back as an
	//	special error to the RPC processor. We send it back to the client and
	//	the client is responsible for resubmitting a request.

	if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
		{
		//	Keep waiting.

		ResetTimeout(GenerateAddress(PORT_HYPERION_COMMAND), LONG_POLL_TIMEOUT);
		return true;
		}

	//	Log

	m_pEngine->LogSessionState(strPattern("[%x:%x] Received: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Msg.sMsg));

	//	Return the reply to the handler. They might process some more and
	//	request another RPC or they might return with a response.

	m_Ctx.pService->HandleRPCResult(m_Ctx, Msg);
	return ProcessServiceResult(m_Ctx, Msg);
	}

bool CHTTPSession::ProcessStateWaitingForRPCResult (const SArchonMessage &Msg)

//	ProcessStateWaitingForRPCResult
//
//	We've sent out an RPC and are waiting for a reply before continuing.

	{
	if (strEquals(Msg.sMsg, MSG_REPLY_LONG_POLL))
		{
		m_iState = State::waitingForRPCLongPoll;

		//	Set the timeout again (because it was cleared when we got a reply).
		//	NOTE: We want this to be longer than the RPC handler time out. 
		//	Otherwise, we might time out while the RPC handler still thinks we
		//	need a reply, and then we'll reply to the wrong session.

		ResetTimeout(GenerateAddress(PORT_HYPERION_COMMAND), LONG_POLL_TIMEOUT);
		return true;
		}
	else if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
		{
		m_Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, ERR_RPC_TIMEOUT);
		return SendResponse(m_Ctx, Msg);
		}

	m_pEngine->LogSessionState(strPattern("[%x:%x] Received: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Msg.sMsg));

	//	Return the reply to the handler. They might process some more and
	//	request another RPC or they might return with a response.

	m_Ctx.pService->HandleRPCResult(m_Ctx, Msg);
	return ProcessServiceResult(m_Ctx, Msg);
	}

bool CHTTPSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage

	{
	try 
		{
		switch (m_iState)
			{
			case State::waitingForRequest:
				return ProcessStateWaitingForRequest(Msg);

			case State::responseSent:
				return ProcessStateResponseSent(Msg);

			case State::responseSentPartial:
				return ProcessStateResponseSentPartial(Msg);

			case State::waitingForRPCLongPoll:
				return ProcessStateWaitingForRPCLongPollResult(Msg);

			case State::waitingForRPCResult:
				return ProcessStateWaitingForRPCResult(Msg);

			case State::waitingForFileData:
				return ProcessStateWaitingForFileData(Msg);

			case State::disconnected:
				return ProcessStateDisconnected(Msg);

			default:
				{
				m_Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, ERR_UNKNOWN_SESSION_STATE);
				return SendResponse(m_Ctx, Msg);
				}
			}
		}
	catch (...)
		{
		m_Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, strPattern(ERR_CRASH_PROCESS_MSG, m_iState, Msg.sMsg));
		return SendResponse(m_Ctx, Msg);
		}
	}

bool CHTTPSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession

	{
	//	Read the request

	return GetRequest(Msg);
	}

bool CHTTPSession::ProcessFileResult (SHTTPRequestCtx &Ctx, CDatum dFileDesc, CDatum dFileData, const SArchonMessage &Msg)

//	ProcessFileResult
//
//	Process a file. We return FALSE if we failed and need to close the session.

	{
	int i;

	//	First get the extension of the file

	CString sExtension = strToLower(fileGetExtension(dFileDesc.GetElement(FIELD_FILE_PATH)));

	//	Handle it based on the extension

	if (strEquals(sExtension, EXTENSION_HEXM))
		{
		try
			{
			//	Since a HEXM file can recurse (load another HEXM file) we 
			//	keep track of the number of times that we load a file.

			if (++Ctx.iFileRecursion > 20)
				{
				Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, ERR_FILE_RECURSION);
				return SendResponse(Ctx, Msg);
				}

			//	Load and parse the HEXM file

			Ctx.pService->HandleHexmFile(Ctx, dFileDesc, dFileData);
			return ProcessServiceResult(Ctx, Msg);
			}
		catch (...)
			{
			Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, strPattern(ERR_CRASH_PROCESS_FILE_HEXM, dFileDesc.GetElement(FIELD_FILE_PATH).AsString()));
			return SendResponse(Ctx, Msg);
			}
		}

	//	Otherwise treat as raw

	else
		{
		try
			{
			//	Get the media type from the file extension.
			//	NOTE: If we couldn't figure it out from the extension we return no media type
			//	(and let the client try to figure it out).

			CString sMediaType = IMediaType::MediaTypeFromExtension(sExtension);

			IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
			pBody->DecodeFromBuffer(sMediaType, CStringBuffer(dFileData));

			Ctx.Response.InitResponse(http_OK, STR_OK);
			Ctx.Response.SetBody(pBody);

			//	Set the modification date/time

			Ctx.Response.AddHeader(HEADER_LAST_MODIFIED, (const CDateTime &)dFileDesc.GetElement(FIELD_MODIFIED_ON));

			//	Add any additional headers

			for (i = 0; i < Ctx.AdditionalHeaders.GetCount(); i++)
				Ctx.Response.AddHeader(Ctx.AdditionalHeaders[i].sField, Ctx.AdditionalHeaders[i].sValue);

			return SendResponse(Ctx, Msg);
			}
		catch (...)
			{
			Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, strPattern(ERR_CRASH_PROCESS_FILE_RAW, dFileDesc.GetElement(FIELD_FILE_PATH).AsString()));
			return SendResponse(Ctx, Msg);
			}
		}
	}

bool CHTTPSession::ProcessServiceResult (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg)

//	ProcessServiceResult
//
//	Handle the result from the service. We return FALSE if we failed
//	and need to close the session.

	{
	try
		{
		switch (Ctx.iStatus)
			{
			case pstatResponseReady:
				return SendResponse(m_Ctx, Msg);

			case pstatResponseReadyProxy:
				return SendResponse(m_Ctx, Msg, FLAG_PROXY);

			case pstatRPCReady:
				return SendRPC(m_Ctx);

			case pstatFilePathReady:
				return SendFile(m_Ctx, Msg);

			case pstatFileDataReady:
				return ProcessFileResult(m_Ctx, m_Ctx.dFileDesc, m_Ctx.dFileData, Msg);

			default:
				//	ERROR: This should never happen.
				ASSERT(false);
				return false;
			}
		}
	catch (...)
		{
		Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, strPattern(ERR_CRASH_PROCESS_SERVICE, Ctx.iStatus, Msg.sMsg));
		return SendResponse(Ctx, Msg);
		}
	}

bool CHTTPSession::SendFile (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg)

//	SendFile
//
//	Sends the contents of a file back to the client. We expect Ctx.sFilePath to
//	be initialized.

	{
	//	Clear our some state because it might be stale from a previous request
	//	(since the session may persist across requests).

	Ctx.dFileData = CDatum();

	if (Ctx.pHexeEval)
		{
		delete Ctx.pHexeEval;
		Ctx.pHexeEval = NULL;
		}

	//	Read

	return SendReadFileRequest(Ctx, Msg, 0);
	}

bool CHTTPSession::SendReadFileRequest (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg, int iOffset)

//	SendReadFileRequest
//
//	Sends the contents of a file back to the client. We expect Ctx.sFilePath to
//	be initialized.

	{
	//	See if the request has an "If-Modified-Since" header

	CDateTime IfModifiedSince;
	CString sValue;
	if (Ctx.Request.FindHeader(HEADER_IF_MODIFIED_SINCE, &sValue))
		{
		IfModifiedSince = CDateTime::ParseIMF(sValue);

		//	Since this header has second resolution, we set it to the last
		//	millisecond in that second.

		IfModifiedSince.SetMillisecond(999);
		}

	//	Parse the file path and figure out where to send it to.

	CString sRoot = Ctx.pService->GetFileRoot();
	CString sAddr;
	CString sMsg;
	CDatum dPayload;
	if (!CAeonInterface::ParseFilePath(Ctx.sFilePath, sRoot, iOffset, IfModifiedSince, &sAddr, &sMsg, &dPayload))
		{
		Ctx.Response.InitResponse(http_NOT_FOUND, ERR_404_NOT_FOUND);
		return SendResponse(m_Ctx, Msg);
		}

	//	Set state

	m_iState = State::waitingForFileData;
	Ctx.iStatus = pstatNone;

	//	Send the message

	m_pEngine->LogSessionState(strPattern("[%x:%x] stateWaitingForFileData: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)sMsg));

	ISessionHandler::SendMessageCommand(sAddr,
			sMsg,
			GenerateAddress(PORT_HYPERION_COMMAND),
			dPayload,
			DEFAULT_TIMEOUT);

	return true;
	}

bool CHTTPSession::SendResponse (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg, DWORD dwFlags)

//	SendResponse
//
//	Sends m_Response back to the client.

	{
	bool bProxy = ((dwFlags & FLAG_PROXY) ? true : false);

	//	Compute how long it took us to generate the response (in milliseconds)

	DWORDLONG dwTime = ::sysGetTicksElapsed(m_dwStartRequest);
#ifdef DEBUG_SESSION_TIMING
	GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(ERR_HTTP_SESSION_TIMING, CEsperInterface::ConnectionToFriendlyID(m_dSocket), Ctx.Request.GetRequestedHost(), Ctx.Request.GetRequestedURL(), (DWORD)dwTime));
#else
	if (dwTime >= 1000 && m_iState != State::waitingForRPCLongPoll)
		GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(ERR_HTTP_SESSION_TIMING, CEsperInterface::ConnectionToFriendlyID(m_dSocket), Ctx.Request.GetRequestedHost(), Ctx.Request.GetRequestedPath(), dwTime));
#endif

	//	Add some standard headers

	if (!bProxy)
		{
		Ctx.Response.AddHeader(HEADER_SERVER, STR_SERVER_VERSION);
		Ctx.Response.AddHeader(HEADER_DATE, CDateTime(CDateTime::Now));

		//	If the request allows gzip and the media type also allows it, then
		//	encode as gzip

		EContentEncodingTypes iDefaultEncoding = Ctx.Response.GetDefaultEncoding();
		if (iDefaultEncoding != http_encodingIdentity
				&& Ctx.Request.IsEncodingAccepted(iDefaultEncoding))
			{
			Ctx.Response.Encode(iDefaultEncoding);
			}

		//	For HTTP 1.0 we always close the connection after a response

		if (!Ctx.Request.IsHTTP11())
			Ctx.Response.AddHeader(HEADER_CONNECTION, STR_CLOSE);
		}

	//	If the size of the body is too big then we sent it out in chunks.

	bool bPartial;
	CBuffer ResponseBuff(4096);
	if (Ctx.Response.GetBodySize() > MAX_SINGLE_BODY_SIZE)
		{
		if (!Ctx.Response.WriteHeadersToBuffer(ResponseBuff, CHTTPMessage::FLAG_CHUNKED_ENCODING))
			{
			GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_CANT_SERIALIZE);
			//	LATER: Drop the connection
			return false;
			}

		bPartial = true;
		m_dwPartialSend = 0;
		}

	//	Otherwise we send the whole message in one reply

	else
		{
		//	Stream the message to a buffer.
		//	If we fail sending a response, then we've got serious problems.

		if (!Ctx.Response.WriteToBuffer(ResponseBuff))
			{
			GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_CANT_SERIALIZE);
			//	LATER: Drop the connection
			return false;
			}

		bPartial = false;
		}

	//	Put the data in a binary datum

	ResponseBuff.Seek(0);
	CDatum dData;
	if (!CDatum::CreateBinary(ResponseBuff, -1, &dData))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_CANT_SERIALIZE);
		//	LATER: Drop the connection
		return false;
		}

	//	Log errors

#ifdef LOG_HTTP_SESSION
	bool bNoErrorLog = ((dwFlags & FLAG_NO_ERROR_LOG) ? true : false);
	if (!bNoErrorLog && Ctx.Response.GetStatusCode() != http_OK && Ctx.Response.GetStatusCode() != http_NOT_MODIFIED)
		GetProcessCtx()->Log(MSG_LOG_INFO, strPattern("[%x] %s -> %d %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetRequestDescription(), Ctx.Response.GetStatusCode(), Ctx.Response.GetStatusMsg()));
#endif

	//	We expect an onWrite with some data
	//	NOTE: We always set the state before sending the message because we
	//	might get a response on a different thread.

	m_iState = (bPartial ? State::responseSentPartial : State::responseSent);
	Ctx.iStatus = pstatNone;

	//	Send a write command

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);
	pPayload->Insert(dData);

	if (bPartial)
		m_pEngine->LogSessionState(strPattern("[%x:%x] stateResponseSentPartial %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_ESPER_WRITE));
	else
		m_pEngine->LogSessionState(strPattern("[%x:%x] stateResponseSent %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_ESPER_WRITE));

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_WRITE,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

	return true;
	}

bool CHTTPSession::SendResponseChunk (const SArchonMessage &Msg, CHTTPMessage &Response)

//	SendResponseChunk
//
//	Sends a partial chunk.

	{
	//	Compute the size of the chunk.

	DWORD dwChunkSize = Min(MAX_SINGLE_BODY_SIZE, Response.GetBodySize() - m_dwPartialSend);

	//	Prepare the chunk

	CBuffer ResponseBuff(4096);
	if (dwChunkSize > 0 && !Response.WriteChunkToBuffer(ResponseBuff, m_dwPartialSend, dwChunkSize))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_CANT_SERIALIZE);
		//	LATER: Drop the connection
		return false;
		}

	//	Was that the last chunk?

	m_dwPartialSend += dwChunkSize;
	if (m_dwPartialSend >= Response.GetBodySize())
		{
		//	Write out a terminating chunk

		Response.WriteChunkToBuffer(ResponseBuff, 0, 0);
		}

	//	Put the data in a binary datum

	ResponseBuff.Seek(0);
	CDatum dData;
	if (!CDatum::CreateBinary(ResponseBuff, -1, &dData))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_CANT_SERIALIZE);
		//	LATER: Drop the connection
		return false;
		}

	//	Send a write command

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);
	pPayload->Insert(dData);

	m_pEngine->LogSessionState(strPattern("[%x:%x] SendMessage: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)MSG_ESPER_WRITE));

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_WRITE,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

	return true;
	}

bool CHTTPSession::SendRPC (SHTTPRequestCtx &Ctx)

//	SendRPC
//
//	Sends a message on behalf on the Hexe environment to process something.
//	The session waits until the message comes back and replies to Hexe.

	{
	//	Initialize context state

	m_iState = State::waitingForRPCResult;
	Ctx.iStatus = pstatNone;

	//	Send the message

	m_pEngine->LogSessionState(strPattern("[%x:%x] stateWaitingForRPCResult: %s.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), GetTicket(), (LPSTR)Ctx.RPCMsg.sMsg));

	ISessionHandler::SendMessageCommand(Ctx.sRPCAddr,
			Ctx.RPCMsg.sMsg,
			GenerateAddress(PORT_HYPERION_COMMAND),
			Ctx.RPCMsg.dPayload,
			DEFAULT_TIMEOUT);

	return true;
	}
