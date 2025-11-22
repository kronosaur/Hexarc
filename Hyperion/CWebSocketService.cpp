//	CWebSocketService.cpp
//
//	CWebSocketService class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command");

DECLARE_CONST_STRING(DEFAULT_API,						"api");

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address");
DECLARE_CONST_STRING(FIELD_API,							"api");
DECLARE_CONST_STRING(FIELD_ARGS,						"args");
DECLARE_CONST_STRING(FIELD_AUTH_TOKEN,					"authToken");
DECLARE_CONST_STRING(FIELD_AUTHORIZATION,				"authorization");
DECLARE_CONST_STRING(FIELD_CONNECT_MSG,					"connectMsg");
DECLARE_CONST_STRING(FIELD_WEBSOCKET_KEY,				"websocketKey");

DECLARE_CONST_STRING(HEADER_AUTHORIZATION,				"Authorization");
DECLARE_CONST_STRING(HEADER_SEC_WEBSOCKET_KEY,			"Sec-WebSocket-Key");

DECLARE_CONST_STRING(MSG_ESPER_START_WS_CONNECTION,		"Esper.startWSConnection");

DECLARE_CONST_STRING(ERR_CONNECT_MSG_CANT_BE_BLANK,		"Connect message cannot be blank.");
DECLARE_CONST_STRING(ERR_INVALID_CONNECT_MSG,			"Invalid connect message: %s.");
DECLARE_CONST_STRING(ERR_400_BAD_REQUEST,				"Bad Request");

CString CWebSocketService::GetAPI () const
	{
	if (m_APIs.GetCount() > 0)
		return m_APIs[0];
	else
		return DEFAULT_API;
	}

bool CWebSocketService::OnHandleRequest (SHTTPRequestCtx &Ctx)

//	OnHandleRequest
//
//	Handle a request. We return TRUE if Ctx.Reponse is properly initialized.
//	Otherwise, we initialize the RPC request fields in Ctx and return false;

	{
	//	The endpoint is the relative path

	CString sPath;
	CDatum dArgs;
	urlParseQuery(Ctx.Request.GetRequestedPath(), &sPath, &dArgs);

	CString sEndpoint = MakeRelativePath(sPath);

	//	Get authorization header, if any

	CString sAuthorization;
	Ctx.Request.FindHeader(HEADER_AUTHORIZATION, &sAuthorization);

	CString sAuthToken = Ctx.Request.GetCookie(FIELD_AUTH_TOKEN);

	//	Figure out where to send the message

	CString sAddress;
	if (!CHexeProcess::IsHexarcMessage(m_sConnectMsg, &sAddress))
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_NOT_IMPLEMENTED, strPattern(ERR_INVALID_CONNECT_MSG, m_sConnectMsg));
		return true;
		}

	//	Compose a payload and send it to the archon handler (the handler who 
	//	will receive all the websocket messages).

	CDatum dOptions(CDatum::typeStruct);
	dOptions.SetElement(FIELD_ARGS, dArgs);
	if (!sAuthToken.IsEmpty())
		dOptions.SetElement(FIELD_AUTH_TOKEN, sAuthToken);

	if (!sAuthorization.IsEmpty())
		dOptions.SetElement(FIELD_AUTHORIZATION, sAuthorization);

	//	Pull the websocket key from the original request because we need to
	//	send it back to Esper to reply.

	CString sKey;
	if (!Ctx.Request.FindHeader(HEADER_SEC_WEBSOCKET_KEY, &sKey))
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_BAD_REQUEST, ERR_400_BAD_REQUEST);
		return true;
		}

	dOptions.SetElement(FIELD_WEBSOCKET_KEY, sKey);

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(GetAPI());
	dPayload.Append(Ctx.pSession->GetSocket());
	dPayload.Append(sEndpoint);
	dPayload.Append(dOptions);

	Ctx.iStatus = pstatRPCReady;
	Ctx.sRPCAddr = sAddress;
	Ctx.RPCMsg.sMsg = m_sConnectMsg;
	Ctx.RPCMsg.dwTicket = 0;
	Ctx.RPCMsg.sReplyAddr = NULL_STR;
	Ctx.RPCMsg.dPayload = dPayload;

	return false;
	}

bool CWebSocketService::OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult)

//	OnHandleRPCResult
//
//	The handler will return information about where to send the websocket message.

	{
	//	If we get back an error, then 404.

	if (ISessionHandler::IsError(RPCResult))
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_NOT_FOUND, RPCResult.sMsg);
		return true;
		}

	//	If we succeed, then we're done

	else
		{
		Ctx.iStatus = pstatUpgradeToWebSocketNoOp;
		return false;
		}

	return false;
	}

bool CWebSocketService::OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError)

//	OnHTTPInit
//
//	Initialize from service definition

	{
	//	Load some parameters

	m_sConnectMsg = dServiceDef.GetElement(FIELD_CONNECT_MSG).AsStringView();
	if (m_sConnectMsg.IsEmpty())
		{
		if (retsError) *retsError = ERR_CONNECT_MSG_CANT_BE_BLANK;
		return false;
		}

	CDatum dAPI = dServiceDef.GetElement(FIELD_API);
	if (dAPI.GetBasicType() == CDatum::typeArray)
		m_APIs = dAPI.AsStringArray();
	else
		{
		CString sAPI = dAPI.AsString();
		if (!sAPI.IsEmpty())
			m_APIs.Insert(sAPI);
		}

	//	Done

	return true;
	}
