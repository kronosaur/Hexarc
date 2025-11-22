//	CEsperConnectionManager.cpp
//
//	CEsperConnectionManager class
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_TRACE
//#define DEBUG_DELETE
#endif

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null");

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address");
DECLARE_CONST_STRING(FIELD_API,							"api");
DECLARE_CONST_STRING(FIELD_MSG,							"msg");

DECLARE_CONST_STRING(MSG_ESPER_ON_CONNECT,				"Esper.onConnect");
DECLARE_CONST_STRING(MSG_ESPER_ON_DISCONNECT,			"Esper.onDisconnect");
DECLARE_CONST_STRING(MSG_ESPER_ON_READ,					"Esper.onRead");
DECLARE_CONST_STRING(MSG_ESPER_ON_WRITE,				"Esper.onWrite");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(PROTOCOL_AMP1,						"amp1");

DECLARE_CONST_STRING(WORKER_SIGNAL_PAUSE,				"Worker.pause")
DECLARE_CONST_STRING(WORKER_SIGNAL_SHUTDOWN,			"Worker.shutdown");

DECLARE_CONST_STRING(ERR_CANT_CONNECT,					"Unable to connect to: %s.");
DECLARE_CONST_STRING(ERR_CRASH,							"Crash in IO operation.");
DECLARE_CONST_STRING(ERR_INVALID_CONNECTION,			"Invalid connection: %x.");
DECLARE_CONST_STRING(ERR_INVALID_CONNECTION_CTX,		"Invalid connection ID for completion port: %x.");
DECLARE_CONST_STRING(ERR_INVALID_CONNECT_ADDR,			"Invalid connection address.");
DECLARE_CONST_STRING(ERR_INVALID_URL,					"Invalid URL: %s.");
DECLARE_CONST_STRING(ERR_CANT_BEGIN_OPERATION,			"Unable to begin IO operation.");
DECLARE_CONST_STRING(ERR_CREATE_SOCKET_FAILED,			"Unable to create a socket.");
DECLARE_CONST_STRING(ERR_INVALID_PORT,					"Unable to determine port from URL: %s.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND_MESSAGE,		"Unable to send Esper message to engine.");
DECLARE_CONST_STRING(ERR_INVALID_PROPERTY,				"Unknown connection property: %s.");
DECLARE_CONST_STRING(ERR_WS_CONNECTION_BUSY,			"Unable to upgrade to WebSocket: connection busy.");
DECLARE_CONST_STRING(ERR_WS_CONNECTION_CLOSED,			"Unable to upgrade to WebSocket: connection not found.");
DECLARE_CONST_STRING(ERR_WS_CONNECTION_CANT_UPGRADE,	"Unable to upgrade to WebSocket: can't upgrade.");

const DWORDLONG TIMEOUT_CHECK_INTERVAL =				15 * 1000;		//	Check for inactivity every 15 seconds
const DWORDLONG INACTIVITY_TIMEOUT =					60 * 60 * 1000;	//	Timeout inactive sessions after an hour.

const DWORD DEFAULT_AMP1_PORT =							7397;

void CEsperConnectionManager::AddConnection (CEsperConnection *pConnection, CDatum *retdConnection)

//	AddConnection
//
//	Adds a connection to our list

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG_MARK_CRASH
	if (m_bInGC)
		m_pArchon->Log(MSG_LOG_ERROR, "BUG: Adding connection while in GC.");
#endif

	DWORD dwID;
	m_Connections.Insert(pConnection, &dwID);
	pConnection->SetID(dwID);

#ifdef DEBUG_MARK_CRASH
	m_pArchon->Log(MSG_LOG_DEBUG, strPattern("Adding connection %08x%08x ID = %x", HIDWORD((DWORDLONG)pConnection), LODWORD((DWORDLONG)pConnection), dwID));
#endif

	HANDLE hHandle = pConnection->GetCompletionHandle();
	if (hHandle != INVALID_HANDLE_VALUE)
		m_IOCP.AddObject(hHandle, EncodeConnection(pConnection));

	//	Done

	if (retdConnection)
		*retdConnection = CDatum(dwID);
	}

bool CEsperConnectionManager::BeginAMP1Operation (const CString &sHostConnection, const CString &sAddress, DWORD dwPort, CEsperConnection **retpConnection, CString *retsError)

//	BeginAMP1Operation
//
//	Looks for a (free) connection to the given host. If we don't find it, we create
//	a new one.

	{
	CSmartLock Lock(m_cs);

	//	Look for an available connection to this host. If we can't find one,
	//	then we need to create one.

	CEsperConnection *pConnection = NULL;
	if (!FindOutboundConnection(sHostConnection, &pConnection))
		{
		//	Create a new connection

		pConnection = new CEsperAMP1ConnectionOut(*this, sHostConnection, sAddress, dwPort);
		if (pConnection->GetSocket() == INVALID_SOCKET)
			{
			delete pConnection;
			if (retsError) *retsError = strPattern(ERR_CANT_CONNECT, sAddress);
			return false;
			}

		AddConnection(pConnection);

		//	Add it to our list of outbound connections

		m_Outbound.Insert(pConnection);

		//	Mark it as busy so a different thread doesn't try to take it.

		pConnection->SetBusy(IIOCPEntry::EOperation::connect);
		}

	//	Done

	*retpConnection = pConnection;
	return true;
	}

bool CEsperConnectionManager::BeginHTTPOperation (const CString &sHostConnection, const CString &sAddress, DWORD dwPort, CEsperConnection **retpConnection, CString *retsError)

//	BeginHTTPOperation
//
//	Looks for a (free) connection to the given host. If we don't find it, we create
//	a new one.

	{
	CSmartLock Lock(m_cs);

	//	Look for an available connection to this host. If we can't find one,
	//	then we need to create one.

	CEsperConnection *pConnection = NULL;
	if (!FindOutboundConnection(sHostConnection, &pConnection))
		{
#ifdef DEBUG_HTTP_MESSAGE
		printf("Creating new connection to %s\n", (LPCSTR)sHostConnection);
#endif
		//	Create a new connection

		pConnection = new CEsperHTTPOutConnection(*this, sHostConnection, sAddress, dwPort);
		if (pConnection->GetSocket() == INVALID_SOCKET)
			{
			delete pConnection;
			if (retsError) *retsError = strPattern(ERR_CANT_CONNECT, sAddress);
			return false;
			}

		AddConnection(pConnection);

		//	Add it to our list of outbound connections

		m_Outbound.Insert(pConnection);

		//	Mark it as busy so a different thread doesn't try to take it.

		pConnection->SetBusy(IIOCPEntry::EOperation::connect);
		}
	else
		{
#ifdef DEBUG_HTTP_MESSAGE
		printf("Using existing connection to %s\n", (LPCSTR)sHostConnection);
#endif
		}

	//	Done

	*retpConnection = pConnection;
	return true;
	}

bool CEsperConnectionManager::BeginAMP1Request (const SArchonMessage &Msg, const CString &sFullAddress, const CString &sCommand, CDatum dData, const CString &sAuthName, const CIPInteger &AuthKey, CString *retsError)

//	BeginAMP1Request
//
//	Start an AMP1 request.
//
//	AMP1 is a strict request-response protocol. We send a request and wait for
//	a response. We don't allow multiple requests at the same time.
//
//	In BeginAMP1Operation, we find a free connection to the given host. If we
//	don't find one, we create a new one. We mark the connection as busy so that
//	another thread doesn't try to use it.

	{
	//	Parse the full address into host and protocol

	CString sAddress;
	DWORD dwPort;
	if (!urlParseHostPort(NULL_STR, sFullAddress, &sAddress, &dwPort))
		dwPort = DEFAULT_AMP1_PORT;

	//	Create a canonical host address

	CString sHostConnection = strPattern("%s://%s:%d", PROTOCOL_AMP1, sAddress, dwPort);

	//	Get a connection to this host

	CEsperConnection *pConnection;
	if (!BeginAMP1Operation(sHostConnection, sAddress, dwPort, &pConnection, retsError))
		return false;

	//	Make the request

	CEsperConnection::SAMP1Request Request;
	Request.sAddress = sAddress;
	Request.dwPort = dwPort;
	Request.sCommand = sCommand;
	Request.dData = dData;
	Request.sSenderName = sAuthName;
	Request.SenderKey = AuthKey;

	if (!pConnection->BeginAMP1Request(Msg, Request, retsError))
		return false;

	//	Done

	return true;
	}

bool CEsperConnectionManager::BeginHTTPRequest (const SArchonMessage &Msg, 
												const CString &sMethod, 
												const CString &sURL, 
												CDatum dHeader, 
												CDatum dBody, 
												CDatum dOptions, 
												CString *retsError)

//	BeginHTTPRequest
//
//	Start an HTTP request.
//
//	HTTP requests are strictly request-response. We send a request and wait for
//	a response. We don't allow multiple requests at the same time. We find a free
//	connection to the given host. If we don't find one, we create a new one. We
//	mark the connection as busy so that another thread doesn't try to use it.
//
//	We return FALSE if we can't start the request.

	{
	//	Prepare the request structure

	CEsperConnection::SHTTPRequest Request;

	//	Parse the URL

	if (!urlParse(sURL.GetParsePointer(), &Request.sProtocol, &Request.sHost, &Request.sPath))
		{
		*retsError = strPattern(ERR_INVALID_URL, sURL);
		return false;
		}

	//	Add the other options

	Request.sMethod = sMethod;
	Request.dHeaders = dHeader;
	Request.dBody = dBody;
	Request.dOptions = dOptions;

	//	Create a canonical host address

	if (!urlParseHostPort(Request.sProtocol, Request.sHost, &Request.sAddress, &Request.dwPort))
		{
		*retsError = strPattern(ERR_INVALID_PORT, sURL);
		return false;
		}

	CString sHostConnection = strPattern("%s://%s:%d", Request.sProtocol, Request.sAddress, Request.dwPort);

	//	Get a connection to this host

	CEsperConnection *pConnection;
	if (!BeginHTTPOperation(sHostConnection, Request.sAddress, Request.dwPort, &pConnection, retsError))
		return false;

	//	Make the request

	if (!pConnection->BeginHTTPRequest(Msg, Request, retsError))
		return false;

	//	Done

	return true;
	}

bool CEsperConnectionManager::BeginOperation (CEsperConnection *pConnection, IIOCPEntry::EOperation iOp)

//	BeginOperation
//
//	Begins an asynchronous operation on a socket. We go through here because
//	we want to insure that two threads cannot begin an operation on the same
//	socket at the same time.

	{
	CSmartLock Lock(m_cs);

	if (!pConnection->SetBusy(iOp))
		return false;

	return true;
	}

bool CEsperConnectionManager::BeginOperation (CDatum dConnection, IIOCPEntry::EOperation iOp, CEsperConnection **retpConnection, CString *retsError)

//	BeginOperation
//
//	Begins the given operation on the connection

	{
	CSmartLock Lock(m_cs);

	//	Get the connection object

	DWORD dwID = (DWORD)dConnection;
	if (!m_Connections.IsValid(dwID))
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_CONNECTION, CEsperInterface::ConnectionToFriendlyID(dConnection));
		return false;
		}

	CEsperConnection *pConnection = m_Connections.GetAt(dwID);

	//	Set the state of the connection to the given operation. We do this 
	//	inside the lock to make sure that a different thread does not start an
	//	operation at the same time.

	if (!pConnection->SetBusy(iOp))
		{
		if (retsError) *retsError = ERR_CANT_BEGIN_OPERATION;
		return false;
		}

	//	Done

	if (retpConnection)
		*retpConnection = pConnection;

	return true;
	}

bool CEsperConnectionManager::BeginRead (const SArchonMessage &Msg, CDatum dConnection, CString *retsError)

//	BeginRead
//
//	Starts a read operation

	{
	//	Start the operation.

	CEsperConnection *pConnection;
	if (!BeginOperation(dConnection, IIOCPEntry::EOperation::read, &pConnection, retsError))
		return false;

	//	Now write
		
	if (!pConnection->BeginRead(Msg, retsError))
		return false;

	//	Done

	return true;
	}

bool CEsperConnectionManager::BeginWrite (const SArchonMessage &Msg, CDatum dConnection, const CString &sData, CString *retsError)

//	BeginWrite
//
//	Starts a write operation

	{
	//	Start the operation.

	CEsperConnection *pConnection;
	if (!BeginOperation(dConnection, IIOCPEntry::EOperation::write, &pConnection, retsError))
		return false;

	//	Now write
		
	if (!pConnection->BeginWrite(Msg, sData, retsError))
		return false;

	//	Done

	return true;
	}

void CEsperConnectionManager::CreateConnection (SConnectionCtx &Ctx,
												CSocket &NewSocket, 
												const CString &sListener, 
												CEsperConnection::ETypes iType)

//	CreateConnection
//
//	Creates a new connection from a listener. We take ownership of the socket.

	{
	//	Get information about the connection

	CString sAddressName = NewSocket.GetConnectionAddress();

	//	Deal with the connection based on the listener type

	switch (iType)
		{
		//	This is an AMP1 connection. We send messages to the client when
		//	we get messages.

		case CEsperConnection::typeAMP1In:
			{
			//	Create a new connection for this socket. The socket is owned by the
			//	connection now.

			CEsperConnection *pConnection = new CEsperAMP1ConnectionIn(*this, Ctx.Msg.sReplyAddr, NewSocket.Handoff());

			CDatum dConnection;
			AddConnection(pConnection, &dConnection);

			//	Connected

			pConnection->OnConnect();
			break;
			}

		//	This is a raw connection in which the client has to issue read/write
		//	requests.

		case CEsperConnection::typeRawIn:
			{
			//	Create a new connection for this socket. The socket is owned by the
			//	connection now.

			CEsperConnection *pConnection = new CEsperSimpleConnection(*this, NewSocket.Handoff());

			CDatum dConnection;
			AddConnection(pConnection, &dConnection);

			//	Connected

			pConnection->OnConnect();

			//	Send a reply to the client (they are now responsible for the socket)

			SendMessageReplyOnConnect(dConnection, sListener, sAddressName, Ctx.Msg);
			break;
			}

		case CEsperConnection::typeTLSIn:
			{
			//	Create a new connection for this socket. The socket is owned by the
			//	connection now.

			CEsperConnection *pConnection = new CEsperTLSConnectionIn(*this, sListener, sAddressName, Ctx.SSLCtx, Ctx.Msg, NewSocket.Handoff());

			CDatum dConnection;
			AddConnection(pConnection, &dConnection);

			//	Connected. Once we're done with the TLS handshake, etc., the 
			//	connection will reply with an Esper.onConnect message.

			pConnection->OnConnect();

#ifdef DEBUG_MARK_CRASH
			m_pArchon->Log(MSG_LOG_DEBUG, strPattern("Adding TLS connection %08x%08x ID = %x", HIDWORD((DWORDLONG)pConnection), LODWORD((DWORDLONG)pConnection), pConnection->GetID()));
#endif
			break;
			}

		case CEsperConnection::typeWSIn:
			{
			//	Create a new connection for this socket. The socket is owned by the
			//	connection now.

			CEsperConnection *pConnection = new CEsperWSSConnectionIn(*this, sListener, sAddressName, Ctx.SSLCtx, NewSocket.Handoff());

			CDatum dConnection;
			AddConnection(pConnection, &dConnection);

			//	Connected. Once we're done with the TLS handshake, etc., the 
			//	connection will reply with an Esper.onConnect message.

			pConnection->OnConnect();
			break;
			}

		default:
			throw CException(errFail);
		}

	m_Stats.IncStat(CEsperStats::statConnectionsIn, 1);
	}

IIOCPEntry* CEsperConnectionManager::DecodeConnection (DWORD_PTR Ctx) const

//	DecodeConnection
//
//	Decodes a connection from a DWORD_PTR

	{
	if (Ctx & CONNECTION_FLAG)
		{
		CSmartLock Lock(m_cs);

		DWORD dwID = (DWORD)(Ctx >> 8);
		if (!m_Connections.IsValid(dwID))
			return NULL;

		return m_Connections[dwID];
		}
	else
		return (IIOCPEntry*)Ctx;
	}

void CEsperConnectionManager::DeleteConnection (CDatum dConnection)

//	DeleteConnection
//
//	Deletes the given connection

	{
	CSmartLock Lock(m_cs);

	CEsperConnection *pEntry;
	if (!FindConnection(dConnection, &pEntry))
		return;

	DeleteConnection(pEntry);
	}

void CEsperConnectionManager::DeleteConnection (CEsperConnection *pConnection)

//	DeleteConnection
//
//	Deletes the given connection

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG_MARK_CRASH
	if (m_bInGC)
		m_pArchon->Log(MSG_LOG_ERROR, "BUG: Deleting connection while in GC.");

	if (!m_Connections.IsValid(pConnection->GetID()))
		m_pArchon->Log(MSG_LOG_ERROR, strPattern("BUG: Deleting invalid connection ID: %x", pConnection->GetID()));

	CEsperConnection* pFound = m_Connections[pConnection->GetID()];
	if (pFound != pConnection)
		m_pArchon->Log(MSG_LOG_ERROR, strPattern("BUG: Deleting connection %x, but found %x in table.", pConnection->GetID(), pFound->GetID()));

	m_pArchon->Log(MSG_LOG_DEBUG, strPattern("Removing connection from list %08x%08x ID = %x", HIDWORD((DWORDLONG)pConnection), LODWORD((DWORDLONG)pConnection), pConnection->GetID()));

	if (pConnection->IsDeleted())
		m_pArchon->Log(MSG_LOG_ERROR, strPattern("BUG: Deleting deleted connection %x.", pConnection->GetID()));

	if (pConnection->IsBusy())
		m_pArchon->Log(MSG_LOG_ERROR, strPattern("BUG: Deleting busy connection %x.", pConnection->GetID()));
#endif

	if (pConnection->IsDeleted())
		{
#ifdef DEBUG_DELETE
		printf("ERROR: Deleting deleted connection.\n");
#endif
		return;
		}

	if (pConnection->IsBusy())
		{
#ifdef DEBUG_DELETE
		printf("ERROR: Deleting busy connection: %x.\n", pConnection->GetID());
#endif
		return;
		}

#ifdef DEBUG_DELETE
	printf("Deleting connection: %x.\n", pConnection->GetID());
#endif

	pConnection->SetDeleted();
	m_Connections.Delete(pConnection->GetID());
	m_Outbound.DeleteValue(pConnection);
	m_Deleted.Insert(pConnection);

#ifdef DEBUG_MARK_CRASH
	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		if (m_Connections.GetNext(i) == pConnection)
			{
			m_pArchon->Log(MSG_LOG_ERROR, strPattern("BUG: Connection %x still in table after delete.", pConnection->GetID()));
			}
		}
#endif
	}

void CEsperConnectionManager::DeleteConnectionByAddress (const CString sAddress)

//	DeleteConnectionByAddress
//
//	Deletes any connectio to the given address.

	{
	CSmartLock Lock(m_cs);

	TArray<CEsperConnection *> ToDelete;

	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		CDatum dValue = pConnection->GetProperty(FIELD_ADDRESS);
		if (!dValue.IsNil()
				&& strEqualsNoCase(sAddress, dValue.AsStringView()))
			{
			ToDelete.Insert(pConnection);
			}
		}

	for (int i = 0; i < ToDelete.GetCount(); i++)
		{
		DeleteConnection(ToDelete[i]);
		}
	}

DWORD_PTR CEsperConnectionManager::EncodeConnection (IIOCPEntry* pConnection) const

//	EncodeConnection
//
//	Encodes a connection into a DWORD_PTR

	{
	//	For real connections, we encode the ID because the pointer is not stable
	//	(e.g., during websocket promotion we change objects).

	DWORD dwID = pConnection->GetID();
	if (dwID)
		return ((DWORD_PTR)dwID << 8) | CONNECTION_FLAG;

	//	Otherwise, we store the pointer. We can guarantee that the lower 4 bits
	//	are always zero because of alignment.

	else
		return (DWORD_PTR)pConnection;
	}

bool CEsperConnectionManager::FindConnection (CDatum dConnection, CEsperConnection **retpConnection)

//	FindConnection
//
//	Finds the connection

	{
	CSmartLock Lock(m_cs);

	DWORD dwID = (DWORD)dConnection;
	if (!m_Connections.IsValid(dwID))
		return false;

	if (retpConnection)
		*retpConnection = m_Connections[dwID];

	return true;
	}

bool CEsperConnectionManager::FindOutboundConnection (const CString &sHostConnection, CEsperConnection **retpConnection)

//	FindOutboundConnection
//
//	Returns an outbound connection matching the given host connection string.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Look for an available connection to this host

	for (i = 0; i < m_Outbound.GetCount(); i++)
		if (strEquals(sHostConnection, m_Outbound[i]->GetHostConnection())
				&& m_Outbound[i]->SetBusy(IIOCPEntry::EOperation::connect))
			{
			*retpConnection = m_Outbound[i];
			return true;
			}

	//	Not found

	return false;
	}

void CEsperConnectionManager::FlushConnections ()

//	FlushConnections
//
//	Flush deleted connections. This should be the only place we delete 
//	connections.

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Deleted.GetCount(); i++)
		{
#ifdef DEBUG_MARK_CRASH
		m_pArchon->Log(MSG_LOG_DEBUG, strPattern("Deleting connection %08x%08x ID = %x", HIDWORD((DWORDLONG)m_Deleted[i]), LODWORD((DWORDLONG)m_Deleted[i]), m_Deleted[i]->GetID()));
#endif

		delete m_Deleted[i];
		}

	m_Deleted.DeleteAll();
	}

void CEsperConnectionManager::GetResults (TArray<CString> &Results)

//	GetResults
//
//	Returns an array of results, one for each completed connection.

	{
	CSmartLock Lock(m_cs);

	Results.DeleteAll();

	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		pConnection->AccumulateResult(Results);
		}
	}

void CEsperConnectionManager::GetStatus (CEsperConnection::SStatus *retStatus)

//	GetStatus
//
//	Returns connection status

	{
	CSmartLock Lock(m_cs);

	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		pConnection->AccumulateStatus(retStatus);
		}
	}

bool CEsperConnectionManager::IsIdle (void)

//	IsIdle
//
//	Returns TRUE if all our connections are idle. This is a hack to help us tesSt
//	connections in a single-threaded environment.

	{
	CSmartLock Lock(m_cs);

	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		if (pConnection->IsOperationInProgress())
			return false;
		}

	return true;
	}

void CEsperConnectionManager::LogTrace (const CString &sText)

//	LogTrace
//
//	Log trace messages

	{
#ifdef DEBUG_TRACE
	if (m_pArchon == NULL)
		return;

	m_pArchon->Log(MSG_LOG_DEBUG, sText);
#endif
	}

void CEsperConnectionManager::Mark ()

//	Mark
//
//	Mark data in use

	{
	DEBUG_TRY

#ifdef DEBUG_MARK_CRASH
	m_bInGC = true;
#endif

	int iCrashCount = 0;
	CEsperConnection* pCrashConnection = NULL;

	DWORD dwID;
	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i, &dwID);

		try
			{
			pConnection->Mark();

#ifdef DEBUG_DELETE
			printf("Marking connection: %x\n", pConnection->GetID());
#endif
			}
		catch (...)
			{
			m_pArchon->Log(MSG_LOG_ERROR, strPattern("CRASH: Marking ID = %x", dwID));

			pCrashConnection = pConnection;
			iCrashCount++;
			}
		}

	if (iCrashCount)
		{
		m_pArchon->Log(MSG_LOG_ERROR, strPattern("Crash marking Esper: %d connection%p crashed.", iCrashCount));
		if (!pCrashConnection)
			{
			m_pArchon->Log(MSG_LOG_ERROR, "CRASH: Connection is NULL.");
			}
		else
			{
			try
				{
				m_pArchon->Log(MSG_LOG_ERROR, strPattern("CRASH: Pointer: %08x%08x", HIDWORD((DWORDLONG)pCrashConnection), LODWORD((DWORDLONG)pCrashConnection)));
				m_pArchon->Log(MSG_LOG_ERROR, strPattern("CRASH: Connection ID: %s", pCrashConnection->GetID()));
				if (pCrashConnection->IsDeleted())
					m_pArchon->Log(MSG_LOG_ERROR, "CRASH: Connection is deleted");

				CString sHost = pCrashConnection->GetHostConnection();
				m_pArchon->Log(MSG_LOG_ERROR, strPattern("CRASH: Connection host: %s", sHost));
				}
			catch (...)
				{
				m_pArchon->Log(MSG_LOG_ERROR, "CRASH: Connection is garbage.");
				}
			}
		}

#ifdef DEBUG_MARK_CRASH
	m_bInGC = false;
#endif

	DEBUG_CATCH
	}

bool CEsperConnectionManager::Process (CEsperProcessingThread& Thread)

//	Process
//
//	Process completions. We return FALSE if we want the thread to quit.

	{
#ifdef DEBUG_MARK_CRASH
	if (m_bInGC)
		m_pArchon->Log(MSG_LOG_ERROR, "BUG: Calling Process while inside GC.");
#endif

	//	Before we start, we take a chance to see if any connections have timed out.

	TimeoutCheck();
	FlushConnections();

	//	Wait until an object completes.

	CIOCompletionPort::SResult Result;
	bool bSuccess = m_IOCP.ProcessCompletion(Result);

	IIOCPEntry* pEntry = DecodeConnection(Result.Ctx);
	if (!pEntry)
		{
		m_pArchon->Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_CONNECTION_CTX, (DWORD)Result.Ctx));
		return true;
		}

	//	If we have a completion handle, then we need to succeed or fail the 
	//	operation.

	if (pEntry->GetCompletionHandle() != INVALID_HANDLE_VALUE)
		{

		if (bSuccess)
			pEntry->OperationComplete(Result.dwBytesTransferred, Result.pOverlapped);
		else
			pEntry->OperationFailed(Result.pOverlapped);
		}

	//	For events with no completion handle, we just process them

	else
		{
		//	If this is a shutdown event, then tell the thread.

		if (strEquals(pEntry->GetType(), WORKER_SIGNAL_SHUTDOWN))
			return false;

		//	Otherwise, if this is the pause event, then we need to pause the
		//	thread until we're asked to resume.

		else if (strEquals(pEntry->GetType(), WORKER_SIGNAL_PAUSE))
			{
			Thread.Stop();
			}

		//	Otherwise, process

		else
			{
			pEntry->Process();
			}
		}

	//	Thread should continue

	return true;
	}

void CEsperConnectionManager::ResetConnection (CDatum dConnection)

//	ResetConnection
//
//	Resets the connection.

	{
	CSmartLock Lock(m_cs);

	CEsperConnection *pEntry;
	if (!FindConnection(dConnection, &pEntry))
		return;

	//	Close the old socket and create a new one.

	pEntry->ResetSocket();

	//	Add the new socket to the IOCP.

	HANDLE hHandle = pEntry->GetCompletionHandle();
	if (hHandle != INVALID_HANDLE_VALUE)
		m_IOCP.AddObject(hHandle, EncodeConnection(pEntry));
	}

void CEsperConnectionManager::SendMessageReplyData (CDatum dPayload, const SArchonMessage &OriginalMsg) 

//	SendMessageReplyData
//
//	Replies with data
	
	{
	if (m_pArchon)
		m_pArchon->SendMessageReply(MSG_REPLY_DATA, dPayload, OriginalMsg);
	}

void CEsperConnectionManager::SendMessageReplyDisconnect (const SArchonMessage &OriginalMsg)

//	SendMessageReplyDisconnect
//
//	Reply with Esper.onDisconnect

	{
	if (m_pArchon)
		m_pArchon->SendMessageReply(MSG_ESPER_ON_DISCONNECT, CDatum(), OriginalMsg);
	}

void CEsperConnectionManager::SendMessageReplyOnConnect (CDatum dConnection, const CString &sListener, const CString &sAddressName, const SArchonMessage &OriginalMsg)

//	SendMessageReplyOnConnect
//
//	Reply with Esper.onConnect

	{
	if (m_pArchon == NULL)
		return;

	//	Send a reply to the client (they are now responsible for the socket)

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(dConnection);
	dPayload.Append(sListener);
	dPayload.Append(sAddressName);

	m_pArchon->SendMessageReply(MSG_ESPER_ON_CONNECT, dPayload, OriginalMsg);
	}

void CEsperConnectionManager::SendMessageReplyOnRead (CDatum dConnection, CString &sData, const SArchonMessage &OriginalMsg)

//	SendMessageReplyOnRead
//
//	Reply with Esper.onRead

	{
	if (m_pArchon == NULL)
		return;

	int iDataLen = sData.GetLength();
	CDatum dData;
	CDatum::CreateStringFromHandoff(sData, &dData);

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dData);
	pPayload->Insert((int)iDataLen);
	pPayload->Insert(dConnection);

	m_pArchon->SendMessageReply(MSG_ESPER_ON_READ, CDatum(pPayload), OriginalMsg);
	}

void CEsperConnectionManager::SendMessageReplyOnWrite (CDatum dConnection, DWORD dwBytesTransferred, const SArchonMessage &OriginalMsg)

//	SendMessageReplyOnWrite
//
//	Reply with Esper.onWrite

	{
	if (m_pArchon == NULL)
		return;

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dwBytesTransferred);
	pPayload->Insert(dConnection);

	m_pArchon->SendMessageReply(MSG_ESPER_ON_WRITE, CDatum(pPayload), OriginalMsg);
	}

bool CEsperConnectionManager::SendWSMessage (CDatum dConnection, CDatum dMessage, CString* retsError)

//	SendWSMessage
//
//	Sends a message on the given WebSocket connection.

	{
	CSmartLock Lock(m_cs);

	CEsperConnection *pEntry;
	if (!FindConnection(dConnection, &pEntry))
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_CONNECTION, CEsperInterface::ConnectionToFriendlyID(dConnection));
		return false;
		}

	return pEntry->SendWSMessage(dMessage, retsError);
	}

bool CEsperConnectionManager::SetProperty (CDatum dConnection, const CString &sProperty, CDatum dValue, CString *retsError)

//	SetProperty
//
//	Sets a property on a connection

	{
	CSmartLock Lock(m_cs);

	CEsperConnection *pEntry;
	if (!FindConnection(dConnection, &pEntry))
		{
		*retsError = strPattern(ERR_INVALID_CONNECTION, CEsperInterface::ConnectionToFriendlyID(dConnection));
		return false;
		}

	//	Set the property

	if (!pEntry->SetProperty(sProperty, dValue))
		{
		*retsError = strPattern(ERR_INVALID_PROPERTY, sProperty);
		return false;
		}

	//	Success!

	return true;
	}

void CEsperConnectionManager::TimeoutCheck (void)

//	TimeoutCheck
//
//	Check to see if any connections have timed out

	{
	CSmartLock Lock(m_cs);

	DWORDLONG dwNow;
	if (sysGetTicksElapsed(m_dwLastTimeoutCheck, &dwNow) <= TIMEOUT_CHECK_INTERVAL)
		return;

	m_dwLastTimeoutCheck = dwNow;
	int iConnectionCount = 0;

	TArray<CEsperConnection *> ToDelete;

	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		if (pConnection->TimeoutCheck(dwNow, INACTIVITY_TIMEOUT))
			ToDelete.Insert(pConnection);
		else
			iConnectionCount++;
		}

	//	Delete 

	for (int j = 0; j < ToDelete.GetCount(); j++)
		{
		DeleteConnection(ToDelete[j]);
		}

	if (iConnectionCount > 1000)
		{
		m_pArchon->Log(MSG_LOG_DEBUG, strPattern("DEBUG: %d active connections", iConnectionCount));
		}
	}

bool CEsperConnectionManager::UpgradeToWebSocket (CDatum dConnection, CDatum dConnectInfo, CStringView sKey, CString* retsError)

//	UpgradeToWebSocket
//
//	Upgrades the connection to a WebSocket connection.

	{
	CSmartLock Lock(m_cs);

	DWORD dwID = (DWORD)dConnection;
	if (!m_Connections.IsValid(dwID))
		{
		//	Connection was probably closed.
		if (retsError) *retsError = ERR_WS_CONNECTION_CLOSED;
		return false;
		}

	CEsperConnection* pOld = m_Connections[dwID];

	//	Operation shouldn't be in progress, otherwise we can't switch overlapped 
	//	structures.

	if (pOld->IsOperationInProgress())
		{
		if (retsError) *retsError = ERR_WS_CONNECTION_BUSY;
		return false;
		}

	//	Upgrade the connection. This will transfer resources to the new 
	//	connection.

	CEsperConnection* pNew = pOld->UpgradeWebSocket();
	if (!pNew)
		{
		if (retsError) *retsError = ERR_WS_CONNECTION_CANT_UPGRADE;
		return false;
		}

#ifdef DEBUG_MARK_CRASH
	if (m_bInGC)
		m_pArchon->Log(MSG_LOG_ERROR, "BUG: Upgrading connection while in GC.");

	m_pArchon->Log(MSG_LOG_DEBUG, strPattern("Upgrading connection %08x%08x ID = %x", HIDWORD((DWORDLONG)pOld), LODWORD((DWORDLONG)pOld), dwID));
	m_pArchon->Log(MSG_LOG_DEBUG, strPattern("New connection %08x%08x ID = %x", HIDWORD((DWORDLONG)pNew), LODWORD((DWORDLONG)pNew), dwID));
#endif

	//	Replace the connection

	m_Connections[dwID] = pNew;
	pNew->SetID(dwID);
	m_Deleted.Insert(pOld);

	//	Let the connection handle the rest.

	pNew->OnUpgradedToWebSocket(dConnectInfo, sKey);

	return true;
	}
