//	CHexeCodeRPCService.cpp
//
//	CHexeCodeRPCService class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_OK,							"OK")

DECLARE_CONST_STRING(CACHE_NO_CACHE,					"no-cache")

DECLARE_CONST_STRING(FIELD_OUTPUT,						"output")

DECLARE_CONST_STRING(HEADER_CACHE_CONTROL,				"Cache-Control")

DECLARE_CONST_STRING(LIBRARY_HYPERION,					"hyperion")
DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")
DECLARE_CONST_STRING(LIBRARY_SESSION_CTX,				"sessionCtx")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_BODY_BUILDER,	"sessionHTTPBodyBuilder")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_REQUEST,		"sessionHTTPRequest")

DECLARE_CONST_STRING(MEDIA_TYPE_JSON,					"application/json")
DECLARE_CONST_STRING(MEDIA_TYPE_JSON_REQUEST,			"application/jsonrequest")
DECLARE_CONST_STRING(MEDIA_TYPE_MULTIPART_FORM,			"multipart/form-data")
DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")

DECLARE_CONST_STRING(STR_CRASH_PREFIX,					"CRASH: ")
DECLARE_CONST_STRING(STR_DEBUG_PREFIX,					"DEBUG: ")
DECLARE_CONST_STRING(STR_ERROR_PREFIX,					"ERROR:")

DECLARE_CONST_STRING(ERR_DUPLICATE_URL_PATH,			"Duplicate urlPath: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_JSON,			"Error parsing JSON.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_MULTIPART,		"Error parsing MIME multipart/form-data.")
DECLARE_CONST_STRING(ERR_INVALID_URL_PATH,				"Invalid urlPath: %s.")
DECLARE_CONST_STRING(ERR_404_NOT_FOUND,					"Not Found")
DECLARE_CONST_STRING(ERR_UNSUPPORTED_MEDIA_TYPE,		"Unsupported media type: %s.")
DECLARE_CONST_STRING(ERR_JSON_SERIALIZE_TIME_WARNING,	"Serialized JSON response.")

bool CHexeCodeRPCService::ComposeResponse (SHTTPRequestCtx &Ctx, CHexeProcess::ERunCodes iRun, CDatum dResult)

//	ComposeResponse
//
//	Handles a response from the process's computation

	{
	//	If we have an async result, we return the required message

	if (iRun == CHexeProcess::runAsyncRequest)
		{
		Ctx.iStatus = pstatRPCReady;
		Ctx.sRPCAddr = dResult.GetElement(0);
		Ctx.RPCMsg.sMsg = dResult.GetElement(1);
		Ctx.RPCMsg.dwTicket = 0;
		Ctx.RPCMsg.sReplyAddr = NULL_STR;
		Ctx.RPCMsg.dPayload = dResult.GetElement(2);

		return false;
		}

	//	If error, log it

	if (iRun == CHexeProcess::runError)
		{
		const CString &sError = dResult;
		if (strStartsWith(sError, STR_ERROR_PREFIX)
				|| strStartsWith(sError, STR_DEBUG_PREFIX)
				|| strStartsWith(sError, STR_CRASH_PREFIX))
			Ctx.pSession->DebugLog(sError);
		else
			Ctx.pSession->DebugLog(strPattern("USER: %s", sError));
		}

	//	Otherwise we process the result depending on the required format.
	//
	//	JSON

	IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
	if (strEquals(m_sOutputContentType, MEDIA_TYPE_JSON) || strEquals(m_sOutputContentType, MEDIA_TYPE_JSON_REQUEST))
		{
		CStringBuffer Buffer;

		CArchonTimer Timer;

		dResult.Serialize(CDatum::formatJSON, Buffer);
		pBody->DecodeFromBuffer(MEDIA_TYPE_JSON, Buffer);

		Timer.LogTime(Ctx.pSession->GetEngine()->GetProcessCtx(), ERR_JSON_SERIALIZE_TIME_WARNING);
		}
	
	//	HTML

	else if (strEquals(m_sOutputContentType, MEDIA_TYPE_HTML))
		{
		//	If we have a single string, we assume it is well-formed HTML

		if (dResult.GetBasicType() == CDatum::typeString)
			pBody->DecodeFromBuffer(MEDIA_TYPE_HTML, CStringBuffer(dResult.AsString()));
		else
			{
			CString sHTML = strPattern(
					"<!DOCTYPE html>\r\n"
					"<html>\r\n"
					"<body>%s</body>"
					"</html>\r\n",

					(LPSTR)dResult.AsString());

			//	LATER: Need to clean the string to make sure it conforms to HTML.
			pBody->DecodeFromBuffer(MEDIA_TYPE_HTML, CStringBuffer(sHTML));
			}
		}

	//	Unsupported content type

	else
		{
		//	LATER: We probably need some classes for creating HTML.
		CString sHTML = strPattern(
				"<!DOCTYPE html>\r\n"
				"<html>\r\n"
				"<body><h1>Unknown content type: %s</h1></body>"
				"</html>\r\n",

				(LPSTR)m_sOutputContentType);

		pBody->DecodeFromBuffer(MEDIA_TYPE_HTML, CStringBuffer(sHTML));
		}

	//	Compose the response

	Ctx.iStatus = pstatResponseReady;
	Ctx.Response.InitResponse(http_OK, STR_OK);
	Ctx.Response.SetBody(pBody);

	//	Since this is generated by code, we never cache it.

	Ctx.Response.AddHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);

	//	Add any additional headers

	for (int i = 0; i < Ctx.AdditionalHeaders.GetCount(); i++)
		Ctx.Response.AddHeader(Ctx.AdditionalHeaders[i].sField, Ctx.AdditionalHeaders[i].sValue);

	return true;
	}

bool CHexeCodeRPCService::OnHandleRequest (SHTTPRequestCtx &Ctx)

//	OnHandleRequest
//
//	Handle a request. We return TRUE if Ctx.Reponse is properly initialized.
//	Otherwise, we initialize the RPC request fields in Ctx and return false;

	{
	CString sError;

	//	Get the request parameters

	CDatum dQuery;
	CString sPath;
	urlParseQuery(Ctx.Request.GetRequestedPath(), &sPath, &dQuery);
	const CString &sMethod = Ctx.Request.GetMethod();

	CString sRelativePath = MakeRelativePath(sPath);

	//	Compose the request body into a datum

	CDatum dBody;
	if (!Ctx.pBodyBuilder->GetBody(&dBody))
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_UNSUPPORTED_MEDIA_TYPE, dBody);
		return true;
		}

	//	We can reset the body, since we don't need it anymore (and we don't
	//	want BodyBuilder to keep it in memory).

	Ctx.pBodyBuilder->ResetBody();

	//	If a different service initialized the process, then clean up

	if (Ctx.pProcess && Ctx.pProcessService != this)
		{
		delete Ctx.pProcess;
		Ctx.pProcess = NULL;
		}

	//	If necessary, initialize the process
	
	if (Ctx.pProcess == NULL)
		{
		//	We take this opportunity to set the security context for the
		//	session. This only initializes service security (user context is
		//	left alone).

		Ctx.pSession->SetServiceSecurity(GetSecurityCtx());

		//	Create a new process

		Ctx.pProcessService = this;
		Ctx.pProcess = new CHexeProcess;

		//	Clone from the template

		Ctx.pProcess->InitFrom(m_ProcessTemplate);

		//	Set some context

		Ctx.pProcess->SetLibraryCtx(LIBRARY_HYPERION, Ctx.pSession->GetEngine());
		Ctx.pProcess->SetLibraryCtx(LIBRARY_SESSION, Ctx.pSession);
		Ctx.pProcess->SetLibraryCtx(LIBRARY_SESSION_CTX, &Ctx);
		Ctx.pProcess->SetLibraryCtx(LIBRARY_SESSION_HTTP_BODY_BUILDER, Ctx.pBodyBuilder);
		Ctx.pProcess->SetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST, &Ctx.Request);
		Ctx.pProcess->SetSecurityCtx(Ctx.pSession->GetSecurityCtx());
		}

	//	Generate an array of arguments to the function call

	TArray<CDatum> Args;
	Args.Insert(sPath);		//	Full path
	Args.Insert(sMethod);
	Args.Insert(CEsperInterface::ConvertHeadersToDatum(Ctx.Request));	//	Headers
	if (!dBody.IsNil())
		Args.Insert(dBody);
	else
		Args.Insert(dQuery);

	//	Generate a call to the appropriate handler for this entry point

	CDatum dCode;
	if (sRelativePath.IsEmpty() && Ctx.pProcess->FindGlobalDef(strPattern("%s+/", GetName()), &dCode))
		;

	//	Otherwise, see if we have the generic handler.

	else if (Ctx.pProcess->FindGlobalDef(strPattern("%s+%s", GetName(), sRelativePath), &dCode))
		;

	//	Otherwise, see if we have the generic handler.

	else if (Ctx.pProcess->FindGlobalDef(strPattern("%s+*", GetName()), &dCode))
		;

	//	Otherwise we have an error

	else
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_NOT_FOUND, ERR_404_NOT_FOUND);
		return true;
		}

	//	Run

	CDatum dResult;
	CHexeProcess::ERunCodes iRun = Ctx.pProcess->Run(dCode, Args, &dResult);

	//	Handle it

	return ComposeResponse(Ctx, iRun, dResult);
	}

bool CHexeCodeRPCService::OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult)

//	OnHandleRPCResult
//
//	Handle a resulting message

	{
	CDatum dResult;
	CHexeProcess::ERunCodes iRun = Ctx.pProcess->RunContinues(CSimpleEngine::MessageToHexeResult(RPCResult), &dResult);

	if (iRun == CHexeProcess::runAsyncRequest)
		{
		Ctx.iStatus = pstatRPCReady;
		Ctx.sRPCAddr = dResult.GetElement(0);
		Ctx.RPCMsg.sMsg = dResult.GetElement(1);
		Ctx.RPCMsg.dPayload = dResult.GetElement(2);
		Ctx.RPCMsg.dwTicket = 0;
		Ctx.RPCMsg.sReplyAddr = NULL_STR;

		return false;
		}

	return ComposeResponse(Ctx, iRun, dResult);
	}

bool CHexeCodeRPCService::OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError)

//	OnHTTPInit
//
//	Initialize from service definition

	{
	//	Load some parameters

	m_sOutputContentType = dServiceDef.GetElement(FIELD_OUTPUT);
	if (m_sOutputContentType.IsEmpty())
		m_sOutputContentType = MEDIA_TYPE_HTML;

	//	Initialize the process template

	m_ProcessTemplate.SetSecurityCtx(GetSecurityCtx());

	if (!m_ProcessTemplate.LoadStandardLibraries(retsError))
		return false;

	if (!m_ProcessTemplate.LoadLibrary(LIBRARY_SESSION, retsError))
		return false;

	if (!m_ProcessTemplate.LoadEntryPoints(Package, retsError))
		return false;

	if (!m_ProcessTemplate.LoadHexeDefinitions(Package, retsError))
		return false;

	//	Done

	return true;
	}

void CHexeCodeRPCService::OnHTTPMark (void)

//	OnHTTPMark
//
//	Mark data in use

	{
	m_ProcessTemplate.Mark();
	}

