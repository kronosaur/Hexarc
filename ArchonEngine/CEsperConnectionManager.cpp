//	CEsperConnectionManager.cpp
//
//	CEsperConnectionManager class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_TRACE
#endif

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null")

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address")

DECLARE_CONST_STRING(MSG_ESPER_ON_CONNECT,				"Esper.onConnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_DISCONNECT,			"Esper.onDisconnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_READ,					"Esper.onRead")
DECLARE_CONST_STRING(MSG_ESPER_ON_WRITE,				"Esper.onWrite")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PROTOCOL_AMP1,						"amp1")

DECLARE_CONST_STRING(WORKER_SIGNAL_SHUTDOWN,			"Worker.shutdown")

DECLARE_CONST_STRING(ERR_CANT_CONNECT,					"Unable to connect to: %s.")
DECLARE_CONST_STRING(ERR_CRASH,							"Crash in IO operation.")
DECLARE_CONST_STRING(ERR_INVALID_CONNECTION,			"Invalid connection: %x.")
DECLARE_CONST_STRING(ERR_INVALID_CONNECT_ADDR,			"Invalid connection address.")
DECLARE_CONST_STRING(ERR_INVALID_URL,					"Invalid URL: %s.")
DECLARE_CONST_STRING(ERR_CANT_BEGIN_OPERATION,			"Unable to begin IO operation.")
DECLARE_CONST_STRING(ERR_CREATE_SOCKET_FAILED,			"Unable to create a socket.")
DECLARE_CONST_STRING(ERR_INVALID_PORT,					"Unable to determine port from URL: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND_MESSAGE,		"Unable to send Esper message to engine.")
DECLARE_CONST_STRING(ERR_INVALID_PROPERTY,				"Unknown connection property: %s.")

const DWORDLONG TIMEOUT_CHECK_INTERVAL =				15 * 1000;		//	Check for inactivity every 15 seconds
const DWORDLONG INACTIVITY_TIMEOUT =					60 * 60 * 1000;	//	Timeout inactive sessions after an hour.

const DWORD DEFAULT_AMP1_PORT =							7397;

void CEsperConnectionManager::AddConnection (CEsperConnection *pConnection, CDatum *retdConnection)

//	AddConnection
//
//	Adds a connection to our list

	{
	CSmartLock Lock(m_cs);

	DWORD dwID;
	m_Connections.Insert(pConnection, &dwID);
	pConnection->SetID(dwID);
	m_IOCP.AddObject(pConnection);

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
		AddConnection(pConnection);

		//	Add it to our list of outbound connections

		m_Outbound.Insert(pConnection);

		//	Mark it as busy so a different thread doesn't try to take it.

		pConnection->SetBusy();
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

		pConnection->SetBusy();
		}

	//	Done

	*retpConnection = pConnection;
	return true;
	}

bool CEsperConnectionManager::BeginAMP1Request (const SArchonMessage &Msg, const CString &sFullAddress, const CString &sCommand, CDatum dData, const CString &sAuthName, const CIPInteger &AuthKey, CString *retsError)

//	BeginAMP1Request
//
//	Start an AMP1 request.

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

bool CEsperConnectionManager::BeginOperation (CEsperConnection *pEntry, IIOCPEntry::EOperations iOp)

//	BeginOperation
//
//	Begins an asynchronous operation on a socket. We go through here because
//	we want to insure that two threads cannot begin an operation on the same
//	socket at the same time.

	{
	CSmartLock Lock(m_cs);

	if (!pEntry->SetBusy())
		return false;

	return true;
	}

bool CEsperConnectionManager::BeginOperation (CDatum dConnection, IIOCPEntry::EOperations iOp, CEsperConnection **retpConnection, CString *retsError)

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

	if (!pConnection->SetBusy())
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
	if (!BeginOperation(dConnection, IIOCPEntry::opRead, &pConnection, retsError))
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
	if (!BeginOperation(dConnection, IIOCPEntry::opWrite, &pConnection, retsError))
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

			CEsperConnection *pEntry = new CEsperAMP1ConnectionIn(*this, Ctx.Msg.sReplyAddr, NewSocket.Handoff());

			CDatum dConnection;
			AddConnection(pEntry, &dConnection);

			//	Connected

			pEntry->OnConnect();
			break;
			}

		//	This is a raw connection in which the client has to issue read/write
		//	requests.

		case CEsperConnection::typeRawIn:
			{
			//	Create a new connection for this socket. The socket is owned by the
			//	connection now.

			CEsperConnection *pEntry = new CEsperSimpleConnection(*this, NewSocket.Handoff());

			CDatum dConnection;
			AddConnection(pEntry, &dConnection);

			//	Connected

			pEntry->OnConnect();

			//	Send a reply to the client (they are now responsible for the socket)

			SendMessageReplyOnConnect(dConnection, sListener, sAddressName, Ctx.Msg);
			break;
			}

		case CEsperConnection::typeTLSIn:
			{
			//	Create a new connection for this socket. The socket is owned by the
			//	connection now.

			CEsperConnection *pEntry = new CEsperTLSConnectionIn(*this, sListener, sAddressName, Ctx.SSLCtx, Ctx.Msg, NewSocket.Handoff());

			CDatum dConnection;
			AddConnection(pEntry, &dConnection);

			//	Connected. Once we're done with the TLS handshake, etc., the 
			//	connection will reply with an Esper.onConnect message.

			pEntry->OnConnect();
			break;
			}

		default:
			ASSERT(false);
		}

	m_Stats.IncStat(CEsperStats::statConnectionsIn, 1);
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

	if (pConnection->GetCurrentOp() != IIOCPEntry::opNone)
		{
		pConnection->SetDeleteOnCompletion();
		return;
		}

	m_Connections.Delete(pConnection->GetID());
	m_Outbound.DeleteValue(pConnection);
	delete pConnection;
	}

void CEsperConnectionManager::DeleteConnectionByAddress (const CString sAddress)

//	DeleteConnectionByAddress
//
//	Deletes any connectio to the given address.

	{
	CSmartLock Lock(m_cs);

	TArray<CEsperConnection *> ToDelete;
	for (int i = 0; i < m_Connections.GetCount(); i++)
		{
		CDatum dValue = m_Connections[i]->GetProperty(FIELD_ADDRESS);
		if (!dValue.IsNil()
				&& strEqualsNoCase(sAddress, dValue))
			{
			ToDelete.Insert(m_Connections[i]);
			}
		}

	for (int i = 0; i < ToDelete.GetCount(); i++)
		DeleteConnection(ToDelete[i]);
	}

bool CEsperConnectionManager::DeleteIfMarked (IIOCPEntry *pConnection)

//	DeleteIfMarked
//
//	Deletes the connection if marked. Returns TRUE if we deleted the connection.

	{
	CSmartLock Lock(m_cs);

	if (pConnection->IsDeleted())
		{
		m_Connections.Delete(pConnection->GetID());
		//	OK to promote since we only compare pointers
		m_Outbound.DeleteValue((CEsperConnection *)pConnection);
		delete pConnection;
		return true;
		}

	return false;
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
				&& m_Outbound[i]->SetBusy())
			{
			*retpConnection = m_Outbound[i];
			return true;
			}

	//	Not found

	return false;
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
//	Returns TRUE if all our connections are idle. This is a hack to help us test
//	connections in a single-threaded environment.

	{
	CSmartLock Lock(m_cs);

	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		if (!pConnection->SetBusy())
			return false;

		pConnection->ClearBusy();
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
	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		pConnection->Mark();
		}
	}

bool CEsperConnectionManager::Process (void)

//	Process
//
//	Process completions. We return FALSE if we want the thread to quit.

	{
	//	Before we start, we take a chance to see if any connections have timed out.

	TimeoutCheck();

	//	Wait until an object completes.
	//
	//	NOTE: We can't guarantee that the returned object is a CEsperConnection.
	//	It could be an event object of some kind.

	DWORD dwBytesTransferred;
	IIOCPEntry *pEntry;
	bool bSuccess = m_IOCP.ProcessCompletion(&pEntry, &dwBytesTransferred);

	//	If the returned entry has been deleted, then we delete it now

	if (DeleteIfMarked(pEntry))
		{ }

	//	If we failed, then pass it on

	else if (!bSuccess)
		pEntry->OperationFailed();

	//	For events with no completion handle, we just do them.

	else if (pEntry->GetCompletionHandle() == INVALID_HANDLE_VALUE)
		{
		//	If this is a shutdown event, then tell the thread.

		if (strEquals(pEntry->GetType(), WORKER_SIGNAL_SHUTDOWN))
			return false;

		//	Otherwise, process

		pEntry->Process();
		}

	//	Otherwise, operation completes

	else
		{
		pEntry->GetBuffer()->SetLength(dwBytesTransferred);
		pEntry->OperationComplete(dwBytesTransferred);
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

	m_IOCP.AddObject(pEntry);
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

	TArray<CEsperConnection *> ToDelete;

	SIDTableEnumerator i;
	m_Connections.Reset(i);
	while (m_Connections.HasMore(i))
		{
		CEsperConnection *pConnection = m_Connections.GetNext(i);
		if (pConnection->TimeoutCheck(dwNow, INACTIVITY_TIMEOUT))
			ToDelete.Insert(pConnection);
		}

	//	Delete 

	int j;
	for (j = 0; j < ToDelete.GetCount(); j++)
		{
		DeleteConnection(ToDelete[j]);
		}
	}
