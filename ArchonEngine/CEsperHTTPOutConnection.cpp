//	CEsperHTTPOutConnection.cpp
//
//	CEsperHTTPOutConnection class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_PROXY,						"proxy");
DECLARE_CONST_STRING(FIELD_RAW,							"raw");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");

DECLARE_CONST_STRING(PROTOCOL_HTTPS,					"https");

DECLARE_CONST_STRING(ERR_LOST_CONNECTION,				"HTTP connection lost.");
DECLARE_CONST_STRING(ERR_INVALID_STATE,					"Invalid state for CEsperHTTPOutConnection: %x.");
DECLARE_CONST_STRING(ERR_CANNOT_CONNECT,				"Unable to connect to %s on port %d: %s");

CEsperHTTPOutConnection::CEsperHTTPOutConnection (CEsperConnectionManager &Manager, const CString &sHostConnection, const CString &sAddress, DWORD dwPort) : 
		CEsperConnection(sAddress, dwPort),
		m_Manager(Manager),
		m_sHostConnection(sHostConnection),
		m_pSSL(NULL),
		m_iState(stateDisconnected),
		m_iSSLSavedState(stateNone)

//	CEsperHTTPOutConnection constructor

	{
	}

CEsperHTTPOutConnection::~CEsperHTTPOutConnection (void)

//	CEsperHTTPOutConnection destructor

	{
	if (m_pSSL)
		delete m_pSSL;
	}

void CEsperHTTPOutConnection::AccumulateStatus (SStatus *ioStatus)

//	AccumulateStatus
//
//	Fills in status

	{
	ioStatus->iTotalObjects++;

	switch (m_iState)
		{
		case stateWaitForConnect:
		case stateWaitForRequestAck:
			ioStatus->iWaitingForWrite++;
			break;

		case stateWaitForResponse:
			ioStatus->iWaitingForRead++;
			break;

		default:
			ioStatus->iIdle++;
			break;
		}
	}

bool CEsperHTTPOutConnection::BeginHTTPRequest (const SArchonMessage &Msg, const SHTTPRequest &Request, CString *retsError)

//	BeginHTTPRequest
//
//	Makes a request

	{
	//	Prepare the request.

	m_Msg = Msg;
	m_bProxy = !Request.dOptions.GetElement(FIELD_PROXY).IsNil();
	m_bRaw = !Request.dOptions.GetElement(FIELD_RAW).IsNil();
	CEsperInterface::EncodeHTTPRequest(Request.sMethod, (m_bProxy ? NULL_STR : Request.sHost), Request.sPath, Request.dHeaders, Request.dBody, &m_HTTPMessage);

	//	Allow reconnect in case of error.

	m_bReconnect = true;
	m_bResetBuffer = true;

	//	If we're not yet connected, we need to connect

	if (m_iState == stateDisconnectedBusy)
		{
		//	Get some options

		m_bSSL = strEquals(Request.sProtocol, PROTOCOL_HTTPS);
		m_sAddress = Request.sAddress;
		m_dwPort = Request.dwPort;

		//	Connect. Even if this call fails, it handles replying to the client,
		//	so it is OK to always return TRUE.

		OpConnect();
		}

	//	Otherwise, send

	else if (m_iState == stateConnectedBusy)
		{
		OpSendRequest();
		}

	//	Otherwise, error

	else
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_STATE, (DWORD)m_iState);
		return false;
		}

	//	Success

	return true;
	}

void CEsperHTTPOutConnection::ClearBusy (void)

//	ClearBusy
//
//	Mark this connection as no longer busy

	{
	if (IsDeleted())
		return;

	switch (m_iState)
		{
		case stateConnectedBusy:
			m_iState = stateConnected;
			break;

		case stateDisconnectedBusy:
			m_iState = stateDisconnected;
			break;
		}
	}

void CEsperHTTPOutConnection::OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred)

//	OnSocketOperationComplete
//
//	Operation complete

	{
	switch (m_iState)
		{
		case stateWaitForConnect:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Connected to %s:%d.", CEsperInterface::ConnectionToFriendlyID(GetID()), m_sAddress, m_dwPort));
#endif
			//	Since we succeeded in connecting on this request, we don't 
			//	bother trying to reconnect.

			m_bReconnect = false;

			//	If this is an SSL connection, we need to initialize it.

			if (m_bSSL)
				{
				if (m_pSSL)
					delete m_pSSL;

				m_pSSL = new CSSLAsyncEngine;

				//	Set the hostname so that we can support SNI

				m_pSSL->SetHostname(m_sAddress);

				//	Start handshake

				m_iSSLSavedState = stateSendRequest;
				OpProcessSSL();
				}

			//	Normal request

			else
				OpSendRequest();

			break;
			}

		case stateWaitForRequestAck:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Request sent %d bytes.", CEsperInterface::ConnectionToFriendlyID(GetID()), dwBytesTransferred));
#endif
			//	Start reading

			OpRead();
			break;
			}

		case stateWaitForResponse:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Received %d bytes.", CEsperInterface::ConnectionToFriendlyID(GetID()), GetBuffer()->GetLength()));
#endif
			//	Process message

			OpProcessReceivedData(*GetBuffer());
			break;
			}

		case stateWaitToReceiveSSLData:
			{
			m_pSSL->ProcessReceiveData(*GetBuffer());
			OpProcessSSL();
			break;
			}

		case stateWaitToSendSSLData:
			{
			OpProcessSSL();
			break;
			}

		case stateWaitToSendSSLDataThenReceive:
			{
			OpRead(stateWaitToReceiveSSLData);
			break;
			}

		case stateWaitToSendSSLDataThenReady:
			{
			OnSSLOperationComplete();
			break;
			}
		}
	}

void CEsperHTTPOutConnection::OnSocketOperationFailed (EOperations iOp)

//	OnSocketOperationFailed
//
//	Failed

	{
	switch (m_iState)
		{
		case stateWaitForRequestAck:
		case stateWaitForResponse:
		case stateWaitToReceiveSSLData:
		case stateWaitToSendSSLData:
		case stateWaitToSendSSLDataThenReceive:
		case stateWaitToSendSSLDataThenReady:
			OpTransmissionFailed();
			break;

		default:
			m_Manager.LogTrace(strPattern("[%x] Disconnect on failure", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			m_Manager.SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_LOST_CONNECTION, m_Msg);
			m_Manager.DeleteConnection(CDatum(GetID()));
			break;
		}
	}

void CEsperHTTPOutConnection::OnSSLOperationComplete (void)

//	OnSSLOperationComplete
//
//	SSL operation finished.

	{
	switch (m_iSSLSavedState)
		{
		case stateSendRequest:
			{
#ifdef DEBUG_SOCKET_OPS
			CSSLAsyncEngine::SConnectionStatus Status;
			m_pSSL->GetConnectionStatus(&Status);
			m_Manager.LogTrace(strPattern("[%x] SSL handshake complete: %s %s", CEsperInterface::ConnectionToFriendlyID(GetID()), Status.sProtocol, Status.sCipherName));
#endif
			OpSendRequest();
			break;
			}

		case stateWaitForRequestAck:
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Request sent.", CEsperInterface::ConnectionToFriendlyID(GetID())));
#endif
			OpReceiveResponse();
			break;

		case stateWaitForResponse:
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Received response (%d bytes).", CEsperInterface::ConnectionToFriendlyID(GetID()), m_pSSL->GetBuffer().GetLength()));
#endif
			OpProcessReceivedData(m_pSSL->GetBuffer());
			break;

		default:
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Unhandled state.", CEsperInterface::ConnectionToFriendlyID(GetID())));
#endif
			break;
		}
	}

bool CEsperHTTPOutConnection::OpConnect (bool bReconnect)

//	OpConnect
//
//	Initializes a connection and switches our state appropriately. If we get an 
//	error, we reply to the original client.
//
//	We return TRUE if the connection request succeeded (but we don't know if
//	the connection completed until we get called back).
//
//	If bReconnect is TRUE, we create a new socket and attempt a reconnect.

	{
	CString sError;

#ifdef DEBUG_SOCKET_OPS
	if (bReconnect)
		m_Manager.LogTrace(strPattern("[%x] Reconnecting to %s:%d.", CEsperInterface::ConnectionToFriendlyID(GetID()), m_sAddress, m_dwPort));
	else
		m_Manager.LogTrace(strPattern("[%x] Connecting to %s:%d.", CEsperInterface::ConnectionToFriendlyID(GetID()), m_sAddress, m_dwPort));
#endif

	//	Reset the socket, if necessary

	if (bReconnect)
		{
		m_bReconnect = false;
		m_Manager.ResetConnection(CDatum(GetID()));
		}

	//	Begin a connection

	m_iState = stateWaitForConnect;
	if (!IIOCPEntry::BeginConnection(m_sAddress, m_dwPort, &sError))
		{
		//	Reset the connection for next time.

		m_iState = stateDisconnected;
		m_Manager.ResetConnection(CDatum(GetID()));

#ifdef DEBUG_SOCKET_OPS
		m_Manager.LogTrace(strPattern("[%x] Connect failed.", CEsperInterface::ConnectionToFriendlyID(GetID())));
#endif
		m_Manager.SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANNOT_CONNECT, m_sAddress, m_dwPort, sError), m_Msg);
		return false;
		}

	//	Done

	return true;
	}

bool CEsperHTTPOutConnection::OpMessageComplete (void)

//	OpMessageComplete
//
//	We've received a complete response from the server.

	{
#ifdef DEBUG_SOCKET_OPS
	m_Manager.LogTrace(strPattern("[%x] HTTP response complete.", CEsperInterface::ConnectionToFriendlyID(GetID())));
#endif

	m_iState = stateConnected;
	CDatum dPayload = CEsperInterface::DecodeHTTPResponse(m_HTTPMessage, m_bRaw);
	m_Manager.SendMessageReplyData(dPayload, m_Msg);

	//	Free up some memory

	m_HTTPMessage.InitFromPartialBufferReset();

	//	Done

	return true;
	}

bool CEsperHTTPOutConnection::OpProcessReceivedData (const IMemoryBlock &Buffer)

//	OpProcessReceivedData
//
//	Process data we've read from the server.

	{
	//	If this is the first bit of data, reset the buffer

	if (m_bResetBuffer)
		{
		m_HTTPMessage.InitFromPartialBufferReset();
		m_bResetBuffer = false;

		//	After this we cannot reconnect, because we've blown away the
		//	request message.

		m_bReconnect = false;
		}

	//	Parse the message. If the message is complete, then reply with 
	//	completed message.

	m_HTTPMessage.InitFromPartialBuffer(Buffer);
	if (m_HTTPMessage.IsMessageComplete())
		return OpMessageComplete();

	//	Otherwise, we need to continue reading

	else
		return OpReceiveResponse();
	}

bool CEsperHTTPOutConnection::OpProcessSSL (void)

//	OpProcessSSL
//
//	Process SSL until we are ready.

	{
	CString sError;
	switch (m_pSSL->Process(&sError))
		{
		case CSSLAsyncEngine::resReady:
			{
			OnSSLOperationComplete();
			return true;
			}

		case CSSLAsyncEngine::resReceiveData:
			{
			if (m_pSSL->ProcessHasDataToSend())
				{
				CStringBuffer Buffer;
				m_pSSL->ProcessSendData(Buffer);
				OpWrite(Buffer, stateWaitToSendSSLDataThenReceive);
				}
			else
				OpRead(stateWaitToReceiveSSLData);

			return true;
			}

		case CSSLAsyncEngine::resSendData:
			{
			CStringBuffer Buffer;
			m_pSSL->ProcessSendData(Buffer);
			OpWrite(Buffer, stateWaitToSendSSLData);
			return true;
			}

		case CSSLAsyncEngine::resError:
			m_iState = stateConnected;

#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] SSL error: %s", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), sError));
#endif
			m_Manager.SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_LOST_CONNECTION, m_Msg);
			return false;

		default:
			ASSERT(false);
			return false;
		}
	}

bool CEsperHTTPOutConnection::OpRead (EStates iNewState)

//	OpRead
//
//	Initiates a read operation.

	{
	CString sError;

	m_iState = iNewState;
	if (!IIOCPEntry::BeginRead(&sError))
		return OpTransmissionFailed();

	return true;
	}

bool CEsperHTTPOutConnection::OpReceiveResponse (void)

//	OpReceiveResponse
//
//	Reads a response from the server

	{
	//	SSL

	if (m_pSSL)
		{
		m_pSSL->Receive();
		m_iSSLSavedState = stateWaitForResponse;
		return OpProcessSSL();
		}

	//	Normal read

	else
		return OpRead(stateWaitForResponse);

	//	Done

	return true;
	}

bool CEsperHTTPOutConnection::OpSendRequest (void)

//	OpSendRequest
//
//	Makes an HTTP request.

	{
	CString sError;

	//	Serialize the request

	CStringBuffer RequestBuffer;
	m_HTTPMessage.WriteToBuffer(RequestBuffer);

	//	Send the request. If we're using SSL, go through that.

	if (m_pSSL)
		{
		m_pSSL->Send(RequestBuffer);
		m_iSSLSavedState = stateWaitForRequestAck;
		return OpProcessSSL();
		}

	//	Normal send

	else
		OpWrite(RequestBuffer, stateWaitForRequestAck);

	//	Done

	return true;
	}

bool CEsperHTTPOutConnection::OpTransmissionFailed (void)

//	OpTransmissionFailed
//
//	This is called when a read or write operation failed. We try to reconnect
//	(if possible). If we cannot continue, we reply to the client with the
//	proper message.

	{
	//	If we failed, try to reconnect.

	if (m_bReconnect)
		return OpConnect(true);

	//	Otherwise, we reply with an error

	else
		{
		//	We stay in the connected state, even thought the socket is probably
		//	screwed up. On the next request, we will get an error and try to
		//	reconnect (and reset the socket).
		//
		//	Alternatively, we could reset the socket here.

		m_iState = stateConnected;

#ifdef DEBUG_SOCKET_OPS
		m_Manager.LogTrace(strPattern("[%x] Unable to communicate on socket.", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
#endif
		m_Manager.SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_LOST_CONNECTION, m_Msg);
		return false;
		}
	}

bool CEsperHTTPOutConnection::OpWrite (const CString &sData, EStates iNewState)

//	OpWrite
//
//	Initiates a write operation

	{
	CString sError;

	m_iState = iNewState;
	if (!IIOCPEntry::BeginWrite(sData, &sError))
		return OpTransmissionFailed();

	return true;
	}

bool CEsperHTTPOutConnection::SetBusy (void)

//	SetBusy
//
//	Mark this connection as busy.

	{
	if (IsDeleted())
		return false;

	switch (m_iState)
		{
		case stateConnected:
			m_iState = stateConnectedBusy;
			return true;

		case stateDisconnected:
			m_iState = stateDisconnectedBusy;
			return true;

		default:
			return false;
		}
	}
