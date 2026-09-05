//	CHexeCodeRPCService.cpp
//
//	CHexeCodeRPCService class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_CHEXE_CODE_RPCSERVICE_EMPTY_INTERNAL_REDIRECT_URL,	"Empty internal redirect URL.");

DECLARE_CONST_STRING(CACHE_NO_CACHE,					"no-cache");
DECLARE_CONST_STRING(CACHE_NO_STORE,					"no-store");

DECLARE_CONST_STRING(FIELD_CODE,						"code");
DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_ERROR,						"error");
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc");
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath");
DECLARE_CONST_STRING(FIELD_HEADERS,						"headers");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_JSON_RPC,					"jsonrpc");
DECLARE_CONST_STRING(FIELD_JSON_FORMAT,				"jsonFormat");
DECLARE_CONST_STRING(FIELD_MESSAGE,						"message");
DECLARE_CONST_STRING(FIELD_OUTPUT,						"output");
DECLARE_CONST_STRING(FIELD_RPC_MODE,					"rpcMode");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");
DECLARE_CONST_STRING(FIELD_URL,							"url");

DECLARE_CONST_STRING(HEADER_ACCEPT,						"Accept");
DECLARE_CONST_STRING(HEADER_ALLOW,						"Allow");
DECLARE_CONST_STRING(HEADER_CACHE_CONTROL,				"Cache-Control");
DECLARE_CONST_STRING(HEADER_WWW_AUTHENTICATE,			"WWW-Authenticate");

DECLARE_CONST_STRING(LIBRARY_HYPERION,					"hyperion");
DECLARE_CONST_STRING(LIBRARY_SESSION,					"session");
DECLARE_CONST_STRING(LIBRARY_SESSION_CTX,				"sessionCtx");
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_BODY_BUILDER,	"sessionHTTPBodyBuilder");
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_REQUEST,		"sessionHTTPRequest");

DECLARE_CONST_STRING(MEDIA_TYPE_JSON,					"application/json");
DECLARE_CONST_STRING(MEDIA_TYPE_JSON_REQUEST,			"application/jsonrequest");
DECLARE_CONST_STRING(MEDIA_TYPE_CUSTOM,					"custom");
DECLARE_CONST_STRING(MEDIA_TYPE_MULTIPART_FORM,			"multipart/form-data");
DECLARE_CONST_STRING(MEDIA_TYPE_SSE,					"text/event-stream");
DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html");
DECLARE_CONST_STRING(MEDIA_TYPE_INTERNAL_REDIRECT,		"internalRedirect");
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain");

DECLARE_CONST_STRING(METHOD_POST,						"POST");

DECLARE_CONST_STRING(MODE_JSON_RPC,						"jsonrpc");

DECLARE_CONST_STRING(JSON_FORMAT_AEON,					"aeon");
DECLARE_CONST_STRING(JSON_FORMAT_JAVASCRIPT,			"javascript");

DECLARE_CONST_STRING(MSG_ERROR_INVALID_AUTH,			"Error.invalidAuth");
DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed");

DECLARE_CONST_STRING(RESULT_FILE_DATA,					"fileData");
DECLARE_CONST_STRING(RESULT_FILE_PATH,					"filePath");
DECLARE_CONST_STRING(RESULT_INTERNAL_REDIRECT,			"internalRedirect");

DECLARE_CONST_STRING(STR_CRASH_PREFIX,					"CRASH: ");
DECLARE_CONST_STRING(STR_DEBUG_PREFIX,					"DEBUG: ");
DECLARE_CONST_STRING(STR_ERROR_PREFIX,					"ERROR:");
DECLARE_CONST_STRING(STR_FORBIDDEN,						"Forbidden");
DECLARE_CONST_STRING(STR_HEXARC_REALM,					"Basic realm=\"Hexarc\"");
DECLARE_CONST_STRING(STR_INTERNAL_SERVER_ERROR,			"Internal Error");
DECLARE_CONST_STRING(STR_OK,							"OK");
DECLARE_CONST_STRING(STR_TIMEOUT_PREFIX,				"TIMEOUT: ");
DECLARE_CONST_STRING(STR_UNAUTHORIZED,					"Unauthorized");

DECLARE_CONST_STRING(VERSION_20,						"2.0");

DECLARE_CONST_STRING(ERR_DUPLICATE_URL_PATH,			"Duplicate urlPath: %s.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_MULTIPART,		"Error parsing MIME multipart/form-data.");
DECLARE_CONST_STRING(ERR_INVALID_URL_PATH,				"Invalid urlPath: %s.");
DECLARE_CONST_STRING(ERR_404_NOT_FOUND,					"Not Found");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_MEDIA_TYPE,		"Unsupported media type: %s.");
DECLARE_CONST_STRING(ERR_JSON_SERIALIZE_TIME_WARNING,	"Serialized JSON response.");
DECLARE_CONST_STRING(ERR_INVALID_JSON_FORMAT,			"Invalid jsonFormat: %s");

bool CHexeCodeRPCService::ComposeCustomResponse (SHTTPRequestCtx& Ctx, CHexeProcess::ERun iRun, CDatum dResult)
	{
	CStringView sResultType = dResult.GetElement(FIELD_TYPE);
	if (strEquals(sResultType, RESULT_FILE_DATA))
		{
		Ctx.iStatus = pstatFileDataReady;
		Ctx.dFileDesc = dResult.GetElement(FIELD_FILE_DESC);
		Ctx.dFileData = dResult.GetElement(FIELD_DATA);

		CDatum dHeaders = dResult.GetElement(FIELD_HEADERS);
		if (!dHeaders.IsNil())
			{
			for (int i = 0; i < dHeaders.GetCount(); i++)
				{
				CString sField = dHeaders.GetKey(i);
				CStringView sValue = dHeaders.GetElement(i);
				if (sField.IsEmpty() || sValue.IsEmpty())
					continue;

				Ctx.AdditionalHeaders.Insert(CHTTPMessage::SHeader({ sField, CString(sValue) }));
				}
			}

		//	Return because the caller will set up the response.

		return true;
		}
	else if (strEquals(sResultType, RESULT_FILE_PATH))
		{
		Ctx.iStatus = pstatFilePathReady;
		Ctx.sFilePath = dResult.GetElement(FIELD_FILE_PATH).AsStringView();

		//	Return because the caller will set up the response.

		return true;
		}
	else if (strEquals(sResultType, RESULT_INTERNAL_REDIRECT))
		{
		return ComposeInternalRedirectResponse(Ctx, iRun, dResult.GetElement(FIELD_URL));
		}
	else
		{
		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);

		CString sError;
		if (iRun == CHexeProcess::ERun::Error || iRun == CHexeProcess::ERun::ForcedTerminate)
			sError = dResult.AsStringView();
		else
			sError = strPattern("Unknown result type: %s", sResultType);

		//	LATER: We probably need some classes for creating HTML.
		CString sHTML = strPattern(
				"<!DOCTYPE html>\r\n"
				"<html>\r\n"
				"<body><h1>%s</h1></body>"
				"</html>\r\n",

				htmlWriteText(sError));

		pBody->DecodeFromBuffer(MEDIA_TYPE_HTML, CStringBuffer(sHTML));

		//	Compose the response

		return ComposeOKResponse(Ctx, pBody);
		}
	}

bool CHexeCodeRPCService::ComposeErrorResponse (SHTTPRequestCtx& Ctx, CHexeProcess::ERun iRun, CStringView sErrorMsg)
	{
	IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);

	//	LATER: We probably need some classes for creating HTML.
	CString sHTML = strPattern(
			"<!DOCTYPE html>\r\n"
			"<html>\r\n"
			"<body><h1>%s</h1></body>"
			"</html>\r\n",

			htmlWriteText(sErrorMsg));

	pBody->DecodeFromBuffer(MEDIA_TYPE_HTML, CStringBuffer(sHTML));

	//	Compose the response

	return ComposeOKResponse(Ctx, pBody);
	}

bool CHexeCodeRPCService::ComposeHTMLResponse (SHTTPRequestCtx& Ctx, CHexeProcess::ERun iRun, CDatum dResult)
	{
	IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);

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

				htmlWriteText(dResult.AsString()));

		pBody->DecodeFromBuffer(MEDIA_TYPE_HTML, CStringBuffer(sHTML));
		}

	//	Compose the response

	return ComposeOKResponse(Ctx, pBody);
	}

bool CHexeCodeRPCService::ComposeInternalRedirectResponse (SHTTPRequestCtx& Ctx, CHexeProcess::ERun iRun, CDatum dResult)
	{
	if (iRun == CHexeProcess::ERun::Error || iRun == CHexeProcess::ERun::ForcedTerminate)
		return ComposeErrorResponse(Ctx, iRun, dResult.AsStringView());

	CStringView sURL = dResult.AsStringView();
	if (sURL.IsEmpty())
		return ComposeErrorResponse(Ctx, iRun, STR_CHEXE_CODE_RPCSERVICE_EMPTY_INTERNAL_REDIRECT_URL);

	Ctx.iStatus = pstatInternalRedirect;
	Ctx.sInternalRedirectURL = sURL;

	return false;
	}

bool CHexeCodeRPCService::ComposeJSONResponse (SHTTPRequestCtx& Ctx, CHexeProcess::ERun iRun, CDatum dResult)
	{
	//	If the response is null, then return 202 Accepted. We need this for
	//	MCP and other protocols.

	if (dResult.IsIdenticalToNil())
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_ACCEPTED, STR_OK);
		return true;
		}

	//	If this is an error and we're in JSON-RPC mode, then we need to return an error object.

	else if (iRun == CHexeProcess::ERun::Error
			&& m_iRPCMode == ERPCMode::JSONRPC20)
		{
		int RPCErrorCode = -1;
		CString sRPCErrorMsg;

		CString sErrorCode;
		if (dResult.IsError(&sErrorCode))
			{
			if (strEquals(sErrorCode, MSG_ERROR_INVALID_AUTH))
				{
				Ctx.Response.InitResponse(http_UNAUTHORIZED, STR_UNAUTHORIZED);
				Ctx.Response.AddHeader(HEADER_WWW_AUTHENTICATE, STR_HEXARC_REALM);
				RPCErrorCode = -32001;
				sRPCErrorMsg = dResult.AsStringView();
				}
			else if (strEquals(sErrorCode, MSG_ERROR_NOT_ALLOWED))
				{
				Ctx.Response.InitResponse(http_FORBIDDEN, STR_FORBIDDEN);
				RPCErrorCode = -32003;
				sRPCErrorMsg = dResult.AsStringView();
				}
			else
				{
				Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, STR_INTERNAL_SERVER_ERROR);
				RPCErrorCode = -32603;
				sRPCErrorMsg = dResult.AsStringView();
				}
			}
		else
			{
			Ctx.Response.InitResponse(http_INTERNAL_SERVER_ERROR, STR_INTERNAL_SERVER_ERROR);
			RPCErrorCode = -32603;
			sRPCErrorMsg = dResult.AsString();
			}

		CDatum dRPCResult(CDatum::typeStruct);
		dRPCResult.SetElement(FIELD_JSON_RPC, VERSION_20);
		dRPCResult.SetElement(FIELD_ID, Ctx.dBody.GetElement(FIELD_ID));

		CDatum dError(CDatum::typeStruct);
		dError.SetElement(FIELD_CODE, RPCErrorCode);
		dError.SetElement(FIELD_MESSAGE, sRPCErrorMsg);
		dRPCResult.SetElement(FIELD_ERROR, dError);

		//	Serialize as JSON

		CStringBuffer Buffer;
		dRPCResult.Serialize(GetJSONSerializationFormat(), Buffer);

		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
		pBody->DecodeFromBuffer(MEDIA_TYPE_JSON, Buffer);
		Ctx.Response.SetBody(pBody);

		//	For auth errors we want no-store.

		Ctx.Response.AddHeader(HEADER_CACHE_CONTROL, CACHE_NO_STORE);

		//	Done

		Ctx.iStatus = pstatResponseReady;
		return true;
		}

	//	Otherwise, normal JSON

	else
		{
		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);

		//	Otherwise, we serialize the result as JSON and return it.

		CArchonTimer Timer;
		CStringBuffer Buffer;
		dResult.Serialize(GetJSONSerializationFormat(), Buffer);

#ifdef DEBUG_PERF
		printf("DebugPerf: JSON serialize %d bytes.\n", Buffer.GetLength());
#endif

		pBody->DecodeFromBuffer(MEDIA_TYPE_JSON, Buffer);

		Timer.LogTime(Ctx.pSession->GetEngine()->GetProcessCtx(), ERR_JSON_SERIALIZE_TIME_WARNING);

		//	Compose the response

		return ComposeOKResponse(Ctx, pBody);
		}
	}

bool CHexeCodeRPCService::ComposeOKResponse (SHTTPRequestCtx& Ctx, IMediaTypePtr pBody)
	{
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

bool CHexeCodeRPCService::ComposeResponse (SHTTPRequestCtx &Ctx, CHexeProcess::ERun iRun, CDatum dResult)

//	ComposeResponse
//
//	Handles a response from the process's computation

	{
	//	If error, log it

	if (iRun == CHexeProcess::ERun::Error || iRun == CHexeProcess::ERun::ForcedTerminate)
		{
		CStringView sError = dResult;
		if (strStartsWith(sError, STR_TIMEOUT_PREFIX))
			{
			//	No need to report because this is an expected error
			}
		else if (strStartsWith(sError, STR_ERROR_PREFIX)
				|| strStartsWith(sError, STR_DEBUG_PREFIX)
				|| strStartsWith(sError, STR_CRASH_PREFIX))
			Ctx.pSession->DebugLog(sError);
		else
			Ctx.pSession->DebugLog(strPattern("USER: %s", sError));
		}

	//	If we have an async result, we return the required message

	if (iRun == CHexeProcess::ERun::AsyncRequest)
		{
		Ctx.iStatus = pstatRPCReady;
		Ctx.sRPCAddr = dResult.GetElement(0).AsStringView();
		Ctx.RPCMsg.sMsg = dResult.GetElement(1).AsStringView();
		Ctx.RPCMsg.dwTicket = 0;
		Ctx.RPCMsg.sReplyAddr = NULL_STR;
		Ctx.RPCMsg.dPayload = dResult.GetElement(2);

		return false;
		}

	//	Otherwise we process the result depending on the required format.
	//
	//	JSON

	if (strEquals(m_sOutputContentType, MEDIA_TYPE_JSON) || strEquals(m_sOutputContentType, MEDIA_TYPE_JSON_REQUEST))
		{
		return ComposeJSONResponse(Ctx, iRun, dResult);
		}
	
	//	HTML

	else if (strEquals(m_sOutputContentType, MEDIA_TYPE_HTML))
		{
		return ComposeHTMLResponse(Ctx, iRun, dResult);
		}

	//	Internal redirect

	else if (strEquals(m_sOutputContentType, MEDIA_TYPE_INTERNAL_REDIRECT))
		{
		return ComposeInternalRedirectResponse(Ctx, iRun, dResult);
		}

	//	Custom handler

	else if (strEquals(m_sOutputContentType, MEDIA_TYPE_CUSTOM))
		{
		return ComposeCustomResponse(Ctx, iRun, dResult);
		}

	//	Unsupported content type

	else
		{
		CString sErrorMsg = strPattern("Unknown content type: %s", m_sOutputContentType);
		return ComposeErrorResponse(Ctx, iRun, sErrorMsg);
		}
	}

bool CHexeCodeRPCService::OnHandleRequest (SHTTPRequestCtx &Ctx)

//	OnHandleRequest
//
//	Handle a request. We return TRUE if Ctx.Reponse is properly initialized.
//	Otherwise, we initialize the RPC request fields in Ctx and return false;

	{
	CString sError;

#ifdef DEBUG_REQUEST
	printf("URL: %s\n", (LPCSTR)Ctx.Request.GetRequestedURL());
	for (int i = 0; i < Ctx.Request.GetHeaderCount(); i++)
		{
		CString sHeader;
		CString sValue;

		Ctx.Request.GetHeader(i, &sHeader, &sValue);
		printf("Request header: %s: %s\n", (LPCSTR)sHeader, (LPCSTR)sValue);
		}
#endif

	//	If output content type is application/json, and the Accept header 
	//	includes text/event-stream but not application/json, then we need to 
	//	return an error.

	if (strEquals(m_sOutputContentType, MEDIA_TYPE_JSON))
		{
		CString sAccept;
		if (Ctx.Request.FindHeader(HEADER_ACCEPT, &sAccept)
				&& strFind(sAccept, MEDIA_TYPE_SSE) != -1
				&& strFind(sAccept, MEDIA_TYPE_JSON) == -1)
			{
#ifdef DEBUG_REQUEST
			printf("SSE not supported.\n");
#endif
			Ctx.iStatus = pstatResponseReady;
			Ctx.Response.InitResponse(http_NOT_ALLOWED, strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, MEDIA_TYPE_SSE));
			Ctx.Response.AddHeader(HEADER_ALLOW, METHOD_POST);
			return true;
			}
		}

	//	Get the request parameters

	CDatum dQuery;
	CString sPath;
	urlParseQuery(Ctx.Request.GetRequestedPath(), &sPath, &dQuery);
	const CString &sMethod = Ctx.Request.GetMethod();

	CString sRelativePath = MakeRelativePath(sPath);

	//	Compose the request body into a datum

	if (!Ctx.pBodyBuilder->GetBody(&Ctx.dBody))
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_UNSUPPORTED_MEDIA_TYPE, Ctx.dBody.AsStringView());
		return true;
		}

#ifdef DEBUG_REQUEST
	printf("%s\n", (LPCSTR)dBody.AsString());
#endif

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
	if (!Ctx.dBody.IsNil())
		Args.Insert(Ctx.dBody);
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
	CHexeProcess::ERun iRun = Ctx.pProcess->Run(dCode, Args, &dResult);

	//	Handle it

	return ComposeResponse(Ctx, iRun, dResult);
	}

bool CHexeCodeRPCService::OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult)

//	OnHandleRPCResult
//
//	Handle a resulting message

	{
	CDatum dResult;
	CHexeProcess::ERun iRun = Ctx.pProcess->RunContinues(CSimpleEngine::MessageToHexeResult(RPCResult), &dResult);

	if (iRun == CHexeProcess::ERun::AsyncRequest)
		{
		Ctx.iStatus = pstatRPCReady;
		Ctx.sRPCAddr = dResult.GetElement(0).AsStringView();
		Ctx.RPCMsg.sMsg = dResult.GetElement(1).AsStringView();
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

	m_sOutputContentType = dServiceDef.GetElement(FIELD_OUTPUT).AsStringView();
	if (m_sOutputContentType.IsEmpty())
		m_sOutputContentType = MEDIA_TYPE_HTML;

	//	Parse JSON format

	CStringView sJSONFormat = dServiceDef.GetElement(FIELD_JSON_FORMAT).AsStringView();
	if (sJSONFormat.IsEmpty() || strEquals(sJSONFormat, JSON_FORMAT_AEON))
		m_iJSONFormat = EJSONFormat::AEON;
	else if (strEquals(sJSONFormat, JSON_FORMAT_JAVASCRIPT))
		m_iJSONFormat = EJSONFormat::JavaScript;
	else
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_JSON_FORMAT, sJSONFormat);
		return false;
		}

	//	Parse RPC Mode

	CStringView sRPCMode = dServiceDef.GetElement(FIELD_RPC_MODE).AsStringView();
	if (sRPCMode.IsEmpty())
		m_iRPCMode = ERPCMode::None;
	else if (strEquals(sRPCMode, MODE_JSON_RPC))
		m_iRPCMode = ERPCMode::JSONRPC20;
	else
		{
		if (retsError) *retsError = strPattern("Invalid rpcMode: %s", sRPCMode);
		return false;
		}

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

