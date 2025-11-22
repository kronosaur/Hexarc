//	CEsperWSSConnectionIn.cpp
//
//	CEsperWSSConnectionIn class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address");
DECLARE_CONST_STRING(FIELD_API,							"api");
DECLARE_CONST_STRING(FIELD_MSG,							"msg");

DECLARE_CONST_STRING(HEADER_CONNECTION,					"Connection");
DECLARE_CONST_STRING(HEADER_DATE,						"Date");
DECLARE_CONST_STRING(HEADER_SERVER,						"Server");
DECLARE_CONST_STRING(HEADER_SEC_WEBSOCKET_KEY,			"Sec-WebSocket-Key");
DECLARE_CONST_STRING(HEADER_SEC_WEBSOCKET_ACCEPT,		"Sec-WebSocket-Accept");
DECLARE_CONST_STRING(HEADER_UPGRADE,					"Upgrade");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_ESPER_ON_WS_DISCONNECT,		"Esper.onWSDisconnect");
DECLARE_CONST_STRING(MSG_ESPER_ON_WS_MESSAGE,			"Esper.onWSMessage");

DECLARE_CONST_STRING(STR_SWITCHING_PROTOCOLS,			"Switching Protocols");
DECLARE_CONST_STRING(STR_SERVER_VERSION,				"Hexarc/1.0");
DECLARE_CONST_STRING(STR_UPGRADE,						"Upgrade");
DECLARE_CONST_STRING(STR_WEBSOCKET,						"websocket");
DECLARE_CONST_STRING(STR_TLS_DISCONNECTED,				"Socket connection closed.");
DECLARE_CONST_STRING(STR_WS_CLOSED,						"Received WebSocket close message.");

DECLARE_CONST_STRING(WEB_SOCKET_GUID,					"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

DECLARE_CONST_STRING(ERR_400_BAD_REQUEST,				"Bad Request");
DECLARE_CONST_STRING(ERR_READ_FAILED,					"Read failed: %s");
DECLARE_CONST_STRING(ERR_INVALID_CLIENT_ADDRESS,		"Invalid client address");
DECLARE_CONST_STRING(ERR_ALREADY_READING,				"Unable to read; already reading.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_WRITE,				"Unable to write to socket.");

CEsperWSSConnectionIn::CEsperWSSConnectionIn (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLCtx& SSLCtx, SOCKET hSocket) 
		: CEsperTLSConnectionImpl(Manager, sListener, sNetworkAddress, SSLCtx, hSocket)

//	CEsperWSSConnectionIn constructor

	{
	}

CEsperWSSConnectionIn::CEsperWSSConnectionIn (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLAsyncEngine* pSSL, SOCKET hSocket)
		: CEsperTLSConnectionImpl(Manager, sListener, sNetworkAddress, pSSL, hSocket)
	{
	}

void CEsperWSSConnectionIn::OnTLSConnect ()
	{
	}

void CEsperWSSConnectionIn::OnTLSDisconnect ()
	{
	if (!m_bDisconnected)
		{
		m_Manager.GetHost().LogWebSocket(GetID(), STR_TLS_DISCONNECTED);
		SendWSOnDisconnect();
		m_bDisconnected = true;
		}
	}

void CEsperWSSConnectionIn::OnTLSRead (CString&& sData)
	{
	m_Protocol.OnRead(CBuffer(sData));

	while (m_Protocol.HasMore())
		{
		CString sFrameData;
		CWebSocketProtocol::EOpCode iOpCode = m_Protocol.GetNextFrame(sFrameData);
		switch (iOpCode)
			{
			case CWebSocketProtocol::EOpCode::Close:
				{
				//	We're done

				m_Manager.GetHost().LogWebSocket(GetID(), STR_WS_CLOSED);
				DeleteConnection();
				if (!m_bDisconnected)
					{
					SendWSOnDisconnect();
					m_bDisconnected = true;
					}
				break;
				}

			case CWebSocketProtocol::EOpCode::Ping:
				SendWSFrame(CWebSocketProtocol::EOpCode::Pong, sFrameData);
				break;

			case CWebSocketProtocol::EOpCode::Pong:
				//	Ignore
				break;

			case CWebSocketProtocol::EOpCode::Binary:
				SendWSOnMessage(CDatum::CreateBinary(std::move(sFrameData)));
				break;

			case CWebSocketProtocol::EOpCode::Text:
#ifdef DEBUG_WEBSOCKET
				printf("OnTLSRead: %s\n", (LPCSTR)sFrameData);
#endif
				SendWSOnMessage(CDatum(std::move(sFrameData)));
				break;
			}
		}

	//	Keep reading

	ReadData();
	}

void CEsperWSSConnectionIn::OnTLSWriteComplete (DWORD dwBytesWritten)
	{
#ifdef DEBUG_WEBSOCKET
	printf("[%x]: OnTLSWriteComplete: %d bytes written\n", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), dwBytesWritten);
#endif

	//	NOTE: The connection's critical section is locked when we get here.

	//	If we have more data to write, then write it now.

	if (m_Output.GetCount() > 0)
		{
		CString sData = std::move(m_Output[0]);
		m_Output.Delete(0);

		//	This should always succeed because we just finished writing.

		WriteData(sData);
		}
	}

void CEsperWSSConnectionIn::DisconnectWithError (CStringView sError)
	{
	if (!m_bDisconnected)
		{
		m_Manager.GetHost().LogWebSocket(GetID(), sError);
		SendWSOnDisconnect();
		DeleteConnection();
		m_bDisconnected = true;
		}
	}

void CEsperWSSConnectionIn::OnUpgradedToWebSocket (CDatum dConnectInfo, CStringView sKey)
	{
	//	Get the info about how to send messages to client.

	m_sClientAddress = dConnectInfo.GetElement(FIELD_ADDRESS).AsString();
	if (m_sClientAddress.IsEmpty())
		{
		CHTTPMessage Response;
		Response.InitResponse(http_INTERNAL_SERVER_ERROR, ERR_INVALID_CLIENT_ADDRESS);
		SendHTTPResponse(Response);
		return;
		}

	m_sClientMsg = dConnectInfo.GetElement(FIELD_MSG).AsString();
	if (m_sClientMsg.IsEmpty())
		m_sClientMsg = MSG_ESPER_ON_WS_MESSAGE;

	m_sAPI = dConnectInfo.GetElement(FIELD_API).AsString();

	//	Compute the WebSocket upgrade response

	CHTTPMessage Response = CWebSocketProtocol::ComposeAcceptResponse(sKey, STR_SERVER_VERSION);
	SendHTTPResponse(Response);

	//	Begin read. We will get called at OnTLSRead when we have data.

	ReadData();
	}

void CEsperWSSConnectionIn::ReadData ()
	{
	CSmartLock Lock(m_cs);

	//	NOTE: We read directly instead of going through m_Manager because we 
	//	don't want need to lock the manager.

	if (!SetBusy(EOperation::read))
		return DisconnectWithError(ERR_ALREADY_READING);

	CString sError;
	if (!BeginRead(SArchonMessage(), &sError))
		return DisconnectWithError(strPattern(ERR_READ_FAILED, sError));
	}

void CEsperWSSConnectionIn::WriteData (CStringView sData)
	{
	CSmartLock Lock(m_cs);

	//	If we're already in the middle of writing, then we need to buffer this
	//	data and write it later.

	if (!SetBusy(EOperation::write))
		{
#ifdef DEBUG_WEBSOCKET
		printf("ERROR: Can't write now; buffering\n");
#endif
		m_Output.Insert(sData);
		return;
		}

	//	Otherwise, write now.

	CString sError;
	if (!BeginWrite(SArchonMessage(), sData, &sError))
		{
#ifdef DEBUG_WEBSOCKET
		printf("ERROR: Unable to write: %s\n", (LPCSTR)sError);
#endif
		//	If we fail to write, then add to our output buffer and try again later.
		//	(This should never happen because we've already checked above.)

		m_Output.Insert(sData);
		}
	}

void CEsperWSSConnectionIn::SendHTTPBadRequest ()
	{
	CHTTPMessage Response;
	Response.InitResponse(http_BAD_REQUEST, ERR_400_BAD_REQUEST);
	SendHTTPResponse(Response);
	}

void CEsperWSSConnectionIn::SendHTTPResponse (CHTTPMessage& Response)
	{
	Response.AddHeader(HEADER_SERVER, STR_SERVER_VERSION);
	Response.AddHeader(HEADER_DATE, CDateTime(CDateTime::Now));

	//	Stream the message to a buffer.
	//	If we fail sending a response, then we've got serious problems.

	CStringBuffer ResponseBuff;
	if (!Response.WriteToBuffer(ResponseBuff))
		{
		m_Manager.GetHost().LogWebSocket(GetID(), ERR_UNABLE_TO_WRITE);
		m_bDisconnected = true;
		return;
		}

	WriteData(CString(std::move(ResponseBuff)));
	}

bool CEsperWSSConnectionIn::SendWSMessage (CDatum dMessage, CString* retsError)
	{
#ifdef DEBUG_WEBSOCKET
	printf("SendWSMessage: %s\n", (LPCSTR)dMessage.AsString());
#endif

	if (dMessage.GetBasicType() == CDatum::typeBinary)
		{
		SendWSFrame(CWebSocketProtocol::EOpCode::Binary, dMessage.AsStringView());
		return true;
		}
	else if (dMessage.GetBasicType() == CDatum::typeString)
		{
		SendWSFrame(CWebSocketProtocol::EOpCode::Text, dMessage.AsStringView());
		return true;
		}
	else
		{
		SendWSFrame(CWebSocketProtocol::EOpCode::Text, dMessage.SerializeToString(CDatum::EFormat::JSON));
		return true;
		}
	}

void CEsperWSSConnectionIn::SendWSOnMessage (CDatum dMessage)

//	SendWSOnMessage
//
//	Sends a message to the client.

	{
	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(CDatum(GetID()));
	dPayload.Append(dMessage);

	m_Manager.SendMessageCommand(m_sClientAddress, MSG_ESPER_ON_WS_MESSAGE, NULL_STR, 0, dPayload);
	}

void CEsperWSSConnectionIn::SendWSOnDisconnect ()

//	SendWSOnDisconnect
//
//	Sends an disconnect message to client.

	{
	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(CDatum(GetID()));

	m_Manager.SendMessageCommand(m_sClientAddress, MSG_ESPER_ON_WS_DISCONNECT, NULL_STR, 0, dPayload);
	}

void CEsperWSSConnectionIn::SendWSFrame (CWebSocketProtocol::EOpCode iOpCode, CStringView sData)

//	SendWSFrame
//
//	Sends a WebSocket frame to the client.

	{
	//	Compose the frame
	//	LATER: If data is above a certain size, breakup into multiple frames.

	CString sFrame = CWebSocketProtocol::EncodeFrame(true, iOpCode, sData);

	//	Send the frame

	WriteData(sFrame);
	}
