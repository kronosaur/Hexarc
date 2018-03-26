//	CHTTPProxyService.cpp
//
//	CHTTPProxyService class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DEST_HOST,					"destHost")
DECLARE_CONST_STRING(FIELD_DEST_PORT,					"destPort")
DECLARE_CONST_STRING(FIELD_PROXY,						"proxy")
DECLARE_CONST_STRING(FIELD_RAW,							"raw")

DECLARE_CONST_STRING(HEADER_CONTENT_TYPE,				"content-type")
DECLARE_CONST_STRING(HEADER_HOST,						"host")

DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")

DECLARE_CONST_STRING(MSG_HEXE_HTTP,						"Hexe.http")

DECLARE_CONST_STRING(PORT_HEXE_COMMAND,					"Hexe.command")

DECLARE_CONST_STRING(STR_OK,							"OK")

bool CHTTPProxyService::OnHandleRequest (SHTTPRequestCtx &Ctx)

//	OnHandleRequest
//
//	Handle a request. We return TRUE if Ctx.Reponse is properly initialized.
//	Otherwise, we initialize the RPC request fields in Ctx and return false;

	{
	//	Generate the destination URL

	CString sDestURL;
	if (m_dwDestPort != urlGetDefaultPort(GetProtocol()))
		sDestURL = strPattern("%s://%s:%d%s", GetProtocol(), m_sDestHost, m_dwDestPort, Ctx.Request.GetRequestedURL());
	else
		sDestURL = strPattern("%s://%s%s", GetProtocol(), m_sDestHost, Ctx.Request.GetRequestedURL());

	//	Options

	CComplexStruct *pOptions = new CComplexStruct;
	pOptions->SetElement(FIELD_PROXY, CDatum(CDatum::constTrue));
	pOptions->SetElement(FIELD_RAW, CDatum(CDatum::constTrue));

	//	Get the headers of the request

	CDatum dDestHeaders = CEsperInterface::ConvertHeadersToDatum(Ctx.Request);

	//	Set the host of the new headers

	dDestHeaders.SetElement(HEADER_HOST, m_sDestHost);

	//	Add everything as parameters

	CComplexArray *pParams = new CComplexArray;
	pParams->Append(Ctx.Request.GetMethod());
	pParams->Append(sDestURL);
	pParams->Append(dDestHeaders);
	pParams->Append(Ctx.pBodyBuilder->GetRawBody());
	pParams->Append(CDatum(pOptions));

	//	We let the Hexe engine do a call and wait for the result. We need to do
	//	this so that we can let this thread do other things. This is particularly
	//	important if we're proxying to ourselves; then this thread might be needed
	//	to handle to request.

	Ctx.iStatus = pstatRPCReady;
	Ctx.sRPCAddr = PORT_HEXE_COMMAND;
	Ctx.RPCMsg.sMsg = MSG_HEXE_HTTP;
	Ctx.RPCMsg.dwTicket = 0;
	Ctx.RPCMsg.sReplyAddr = NULL_STR;
	Ctx.RPCMsg.dPayload = CDatum(pParams);

	return false;
	}

bool CHTTPProxyService::OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult)

//	OnHandleRPCResult
//
//	Handle a resulting message

	{
	//	Parse the result

	DWORD dwResultCode = (int)RPCResult.dPayload.GetElement(0);
	const CString &sResultMsg = RPCResult.dPayload.GetElement(1);
	CDatum dHeaders = RPCResult.dPayload.GetElement(2);
	const CString &sBody = RPCResult.dPayload.GetElement(3);

	//	Compose the response

	Ctx.iStatus = pstatResponseReadyProxy;
	Ctx.Response.InitResponse(dwResultCode, sResultMsg);
	CEsperInterface::AddHeadersToMessage(dHeaders, &Ctx.Response);

	if (!sBody.IsEmpty())
		{
		CString sMediaType;
		if (!Ctx.Response.FindHeader(HEADER_CONTENT_TYPE, &sMediaType))
			sMediaType = MEDIA_TYPE_TEXT;

		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
		pBody->DecodeFromBuffer(sMediaType, CStringBuffer(sBody));

		Ctx.Response.SetBody(pBody);
		}

	return true;
	}

bool CHTTPProxyService::OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError)

//	OnHTTPInit
//
//	Initialize from service definition

	{
	//	Load some parameters

	m_sDestHost = dServiceDef.GetElement(FIELD_DEST_HOST);
	m_dwDestPort = (DWORD)(int)dServiceDef.GetElement(FIELD_DEST_PORT);
	if (m_dwDestPort == 0)
		m_dwDestPort = strToInt(GetPort(), 80);

	//	Done

	return true;
	}
