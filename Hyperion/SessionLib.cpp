//	SessionLib.cpp
//
//	Hyperion Session Library
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(LIBRARY_HYPERION,					"hyperion");
DECLARE_CONST_STRING(LIBRARY_SESSION,					"session");
DECLARE_CONST_STRING(LIBRARY_SESSION_CTX,				"sessionCtx");
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_BODY_BUILDER,	"sessionHTTPBodyBuilder");
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_REQUEST,		"sessionHTTPRequest");

bool httpMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD HTTP_CALL =									0;
DECLARE_CONST_STRING(HTTP_CALL_NAME,					"http");
DECLARE_CONST_STRING(HTTP_CALL_ARGS,					"*");
DECLARE_CONST_STRING(HTTP_CALL_HELP,					"(http method URL headers body [options]) -> result");

const DWORD HTTP_GET_COOKIE =							1;
DECLARE_CONST_STRING(HTTP_GET_COOKIE_NAME,				"httpGetCookie");
DECLARE_CONST_STRING(HTTP_GET_COOKIE_ARGS,				"*");
DECLARE_CONST_STRING(HTTP_GET_COOKIE_HELP,				"(httpGetCookie key) -> value");

const DWORD HTTP_GET_HEADERS =							2;
DECLARE_CONST_STRING(HTTP_GET_HEADERS_NAME,				"httpGetHeaders");
DECLARE_CONST_STRING(HTTP_GET_HEADERS_ARGS,				"*");
DECLARE_CONST_STRING(HTTP_GET_HEADERS_HELP,				"(httpGetHeaders) -> ((header1 value1) (header2 value2)...)");

const DWORD HTTP_GET_RAW_BODY =							3;
DECLARE_CONST_STRING(HTTP_GET_RAW_BODY_NAME,			"httpGetRawBody");
DECLARE_CONST_STRING(HTTP_GET_RAW_BODY_ARGS,			"*");
DECLARE_CONST_STRING(HTTP_GET_RAW_BODY_HELP,			"(httpGetRawBody) -> value");

const DWORD HTTP_GET_REQUEST_URL =						4;
DECLARE_CONST_STRING(HTTP_GET_REQUEST_URL_NAME,			"httpGetRequestURL");
DECLARE_CONST_STRING(HTTP_GET_REQUEST_URL_ARGS,			"*");
DECLARE_CONST_STRING(HTTP_GET_REQUEST_URL_HELP,			"(httpGetRequestURL) -> urlDesc\n\n"

														"urlDesc:\n\n"

														"   'host: The hostname (e.g., www.example.com)\n"
														"   'path: The path, excluding the host (e.g., /foo/test.html)\n"
														"   'protocol: The protocol (e.g., http)\n"
														"   'query: A struct representing the query\n"
														"   'url: The full request URL\n"
														);

const DWORD HTTP_GET_URL_PARAMS =						5;
DECLARE_CONST_STRING(HTTP_GET_URL_PARAMS_NAME,			"httpGetURLParams");
DECLARE_CONST_STRING(HTTP_GET_URL_PARAMS_ARGS,			"*");
DECLARE_CONST_STRING(HTTP_GET_URL_PARAMS_HELP,			"(httpGetURLParams) -> { param1:value1 param2:value2 ... }");

const DWORD HTTP_SET_COOKIE =							6;
DECLARE_CONST_STRING(HTTP_SET_COOKIE_NAME,				"httpSetCookie");
DECLARE_CONST_STRING(HTTP_SET_COOKIE_ARGS,				"*");
DECLARE_CONST_STRING(HTTP_SET_COOKIE_HELP,				"(httpSetCookie cookie) -> True/Nil");

bool serviceMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD SERVICE_FIND_COMMAND =						0;
DECLARE_CONST_STRING(SERVICE_FIND_COMMAND_NAME,			"srvFindCommand");
DECLARE_CONST_STRING(SERVICE_FIND_COMMAND_ARGS,			"*");
DECLARE_CONST_STRING(SERVICE_FIND_COMMAND_HELP,			"(srvFindCommand command [attrib]) -> commandDesc");

const DWORD SERVICE_GET_INFO =							1;
DECLARE_CONST_STRING(SERVICE_GET_INFO_NAME,				"srvGetInfo");
DECLARE_CONST_STRING(SERVICE_GET_INFO_ARGS,				"*");
DECLARE_CONST_STRING(SERVICE_GET_INFO_HELP,				"(srvGetInfo) -> infoDesc\n\n"

														"infoDesc:\n\n"

														"   'name: Name of service\n"
														"   'packageName: Name of package\n"
														"   'packageVersion: Version string of package\n"
														);

const DWORD SERVICE_PARSE_COMMAND_LINE =				2;
DECLARE_CONST_STRING(SERVICE_PARSE_COMMAND_LINE_NAME,	"srvParseCommandLine");
DECLARE_CONST_STRING(SERVICE_PARSE_COMMAND_LINE_ARGS,	"*");
DECLARE_CONST_STRING(SERVICE_PARSE_COMMAND_LINE_HELP,	"(srvParseCommandLine commandLine) -> (command params)");

bool sessionMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD SESSION_LOG =								0;
DECLARE_CONST_STRING(SESSION_LOG_NAME,					"print");
DECLARE_CONST_STRING(SESSION_LOG_ARGS,					"*");
DECLARE_CONST_STRING(SESSION_LOG_HELP,					"(print ...) -> string");

bool userMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD USER_GET_NAME =								0;
DECLARE_CONST_STRING(USER_GET_NAME_NAME,				"userGetName");
DECLARE_CONST_STRING(USER_GET_NAME_ARGS,				"*");
DECLARE_CONST_STRING(USER_GET_NAME_HELP,				"(userGetName) -> username");

const DWORD USER_GET_RIGHTS =							1;
DECLARE_CONST_STRING(USER_GET_RIGHTS_NAME,				"userGetRights");
DECLARE_CONST_STRING(USER_GET_RIGHTS_ARGS,				"*");
DECLARE_CONST_STRING(USER_GET_RIGHTS_HELP,				"(userGetRights) -> list of rights");

const DWORD USER_HAS_RIGHT =							2;
DECLARE_CONST_STRING(USER_HAS_RIGHT_NAME,				"userHasRight");
DECLARE_CONST_STRING(USER_HAS_RIGHT_ARGS,				"*");
DECLARE_CONST_STRING(USER_HAS_RIGHT_HELP,				"(userHasRight right) -> true/nil");

const DWORD USER_SET =									3;
DECLARE_CONST_STRING(USER_SET_NAME,						"userSet");
DECLARE_CONST_STRING(USER_SET_ARGS,						"*");
DECLARE_CONST_STRING(USER_SET_HELP,						"(userSet username [rights]) -> true/nil");

DECLARE_CONST_STRING(FIELD_ATTRIBUTES,					"attributes");
DECLARE_CONST_STRING(FIELD_CODE,						"code");
DECLARE_CONST_STRING(FIELD_HELP,						"help");
DECLARE_CONST_STRING(FIELD_HOST,						"host");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_PACKAGE_NAME,				"packageName");
DECLARE_CONST_STRING(FIELD_PACKAGE_VERSION,				"packageVersion");
DECLARE_CONST_STRING(FIELD_PARAMETERS,					"parameters");
DECLARE_CONST_STRING(FIELD_PATH,						"path");
DECLARE_CONST_STRING(FIELD_PROTOCOL,					"protocol");
DECLARE_CONST_STRING(FIELD_QUERY,						"query");
DECLARE_CONST_STRING(FIELD_URL,							"url");

DECLARE_CONST_STRING(HEADER_SET_COOKIE,					"set-cookie");

DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");

DECLARE_CONST_STRING(CMD_EVAL,							"eval");

DECLARE_CONST_STRING(OPTION_OPTIONAL,					"optional");

DECLARE_CONST_STRING(ERR_CANT_GET_RIGHTS,				"Cannot obtain user rights.");
DECLARE_CONST_STRING(ERR_INVALID_COOKIE,				"Invalid cookie.");
DECLARE_CONST_STRING(ERR_NO_SESSION_CTX,				"Unable to find session context.");
DECLARE_CONST_STRING(ERR_BAD_COMMAND_LINE,				"Unable to parse command line.");
DECLARE_CONST_STRING(ERR_INVALID_REQUEST_URL,			"Unable to parse request URL: %s.");

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_SessionLibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(HTTP_CALL, httpMisc, IInvokeCtx::EXEC_RIGHT_SIDE_EFFECTS),
	DECLARE_DEF_LIBRARY_FUNC(HTTP_GET_COOKIE, httpMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(HTTP_GET_HEADERS, httpMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(HTTP_GET_RAW_BODY, httpMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(HTTP_GET_REQUEST_URL, httpMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(HTTP_GET_URL_PARAMS, httpMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(HTTP_SET_COOKIE, httpMisc, 0),

	DECLARE_DEF_LIBRARY_FUNC(SERVICE_FIND_COMMAND, serviceMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(SERVICE_GET_INFO, serviceMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(SERVICE_PARSE_COMMAND_LINE, serviceMisc, 0),

	DECLARE_DEF_LIBRARY_FUNC(SESSION_LOG, sessionMisc, IInvokeCtx::EXEC_RIGHT_SIDE_EFFECTS),

	DECLARE_DEF_LIBRARY_FUNC(USER_GET_NAME, userMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(USER_GET_RIGHTS, userMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(USER_HAS_RIGHT, userMisc, 0),
	DECLARE_DEF_LIBRARY_FUNC(USER_SET, userMisc, IInvokeCtx::EXEC_RIGHT_SIDE_EFFECTS),
	};

const int g_iSessionLibraryDefCount = SIZEOF_STATIC_ARRAY(g_SessionLibraryDef);
bool g_bRegistered = false;

void RegisterSessionLibrary (void)
	{
	if (!g_bRegistered)
		{
		g_bRegistered = true;
		g_HexeLibrarian.RegisterLibrary(LIBRARY_SESSION, g_iSessionLibraryDefCount, g_SessionLibraryDef);
		}
	}

//	Implementations ------------------------------------------------------------

bool httpMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	int i;

	switch (dwData)
		{
		case HTTP_CALL:
			{
			if (!CEsperInterface::HTTP(LocalEnv.GetArgument(0).AsStringView(),
					LocalEnv.GetArgument(1).AsStringView(),
					LocalEnv.GetArgument(2),
					LocalEnv.GetArgument(3),
					LocalEnv.GetArgument(4),
					&retResult.dResult))
				{
				CHexeError::Create(NULL_STR, retResult.dResult.AsString(), &retResult.dResult);
				return false;
				}

			return true;
			}

		case HTTP_GET_COOKIE:
			{
			CHTTPMessage *pRequest = (CHTTPMessage *)pCtx->GetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST);
			if (pRequest == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CStringView sKey = LocalEnv.GetArgument(0);
			retResult.dResult = pRequest->GetCookie(sKey);
			return true;
			}

		case HTTP_GET_HEADERS:
			{
			CHTTPMessage *pRequest = (CHTTPMessage *)pCtx->GetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST);
			if (pRequest == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CComplexArray *pArray = new CComplexArray;
			for (i = 0; i < pRequest->GetHeaderCount(); i++)
				{
				CString sHeader;
				CString sValue;
				pRequest->GetHeader(i, &sHeader, &sValue);

				CComplexArray *pHeader = new CComplexArray;
				pHeader->Insert(sHeader);
				pHeader->Insert(sValue);

				pArray->Insert(CDatum(pHeader));
				}

			retResult.dResult = CDatum(pArray);
			return true;
			}

		case HTTP_GET_RAW_BODY:
			{
			CEsperBodyBuilder *pBodyBuilder = (CEsperBodyBuilder *)pCtx->GetLibraryCtx(LIBRARY_SESSION_HTTP_BODY_BUILDER);
			if (pBodyBuilder == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = pBodyBuilder->GetRawBody();
			return true;
			}

		case HTTP_GET_REQUEST_URL:
			{
			CHTTPMessage *pRequest = (CHTTPMessage *)pCtx->GetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST);
			if (pRequest == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			//	Parse the URL components

			CString sURL = pRequest->GetRequestedURL();

			CString sProtocol;
			CString sHost;
			CString sPathAndQuery;
			if (!urlParse(sURL, &sProtocol, &sHost, &sPathAndQuery))
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_REQUEST_URL, sURL), &retResult.dResult);
				return false;
				}

			CString sPath;
			CDatum dQuery;
			if (!urlParseQuery(sPathAndQuery, &sPath, &dQuery))
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_REQUEST_URL, sURL), &retResult.dResult);
				return false;
				}

			//	Create a result

			CComplexStruct *pResult = new CComplexStruct;
			pResult->SetElement(FIELD_HOST, sHost);
			pResult->SetElement(FIELD_PATH, sPath);
			pResult->SetElement(FIELD_PROTOCOL, sProtocol);
			pResult->SetElement(FIELD_URL, sURL);
			pResult->SetElement(FIELD_QUERY, dQuery);

			//	Done

			retResult.dResult = CDatum(pResult);
			return true;
			}

		case HTTP_GET_URL_PARAMS:
			{
			CHTTPMessage *pRequest = (CHTTPMessage *)pCtx->GetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST);
			if (pRequest == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			if (!urlParseQuery(pRequest->GetRequestedPath(), NULL, &retResult.dResult))
				{
				retResult.dResult = CDatum();
				return false;
				}

			return true;
			}

		case HTTP_SET_COOKIE:
			{
			SHTTPRequestCtx *pSessionCtx = (SHTTPRequestCtx *)pCtx->GetLibraryCtx(LIBRARY_SESSION_CTX);
			if (pSessionCtx == NULL)
				{
				CHexeError::Create(NULL_STR, ERR_NO_SESSION_CTX, &retResult.dResult);
				return false;
				}

			CDatum dCookie = LocalEnv.GetArgument(0);
			if (dCookie.GetBasicType() == CDatum::typeString)
				{
				CHTTPMessage::SHeader *pHeader = pSessionCtx->AdditionalHeaders.Insert();
				pHeader->sField = HEADER_SET_COOKIE;
				pHeader->sValue = dCookie.AsStringView();
				}
			else
				{
				CHexeError::Create(NULL_STR, ERR_INVALID_COOKIE, &retResult.dResult);
				return false;
				}

			retResult.dResult = CDatum(CDatum::typeTrue);
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool serviceMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	switch (dwData)
		{
		case SERVICE_FIND_COMMAND:
			{
			int i;
			CHyperionEngine *pEngine = (CHyperionEngine *)pCtx->GetLibraryCtx(LIBRARY_HYPERION);
			if (pEngine == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CStringView sCommandName = LocalEnv.GetArgument(0);
			CStringView sAttrib = LocalEnv.GetArgument(1);

			//	If we don't have a command name then we return a list of all 
			//	commands with the attribute.

			if (sCommandName.IsEmpty() && !sAttrib.IsEmpty())
				{
				TArray<CHyperionCommandSet::SCommandInfo> List;
				pEngine->GetCommands(sAttrib, &List);

				if (List.GetCount() == 0)
					{
					retResult.dResult = CDatum();
					return true;
					}

				CComplexArray *pList = new CComplexArray;
				for (i = 0; i < List.GetCount(); i++)
					{
					//	Skip private commands

					if (!List[i].bPublic)
						continue;

					//	Compose a structure with the command info

					CComplexStruct *pDesc = new CComplexStruct;
					pDesc->SetElement(FIELD_NAME, List[i].sName);
					pDesc->SetElement(FIELD_HELP, List[i].dHelp);
					pDesc->SetElement(FIELD_CODE, List[i].dCode);

					CDatum dAttribs;
					CDatum::CreateFromAttributeList(List[i].Attribs, &dAttribs);
					pDesc->SetElement(FIELD_ATTRIBUTES, dAttribs);

					//	Add to list

					pList->Insert(CDatum(pDesc));
					}

				//	Done

				retResult.dResult = CDatum(pList);
				return true;
				}

			//	Look for the command

			else
				{
				CHyperionCommandSet::SCommandInfo CommandInfo;
				if (!pEngine->FindServiceCommand(sAttrib, sCommandName, &CommandInfo))
					{
					//	We return Nil instead of an error because we allow the caller to report
					//	the error in their own way.
					retResult.dResult = CDatum();
					return true;
					}

				//	If the command is not public, then we can't access it.

				if (!CommandInfo.bPublic)
					{
					retResult.dResult = CDatum();
					return true;
					}

				//	Compose a structure with the command info

				CComplexStruct *pDesc = new CComplexStruct;
				pDesc->SetElement(FIELD_NAME, CommandInfo.sName);
				pDesc->SetElement(FIELD_HELP, CommandInfo.dHelp);
				pDesc->SetElement(FIELD_CODE, CommandInfo.dCode);

				CDatum dAttribs;
				CDatum::CreateFromAttributeList(CommandInfo.Attribs, &dAttribs);
				pDesc->SetElement(FIELD_ATTRIBUTES, dAttribs);

				//	Done

				retResult.dResult = CDatum(pDesc);
				return true;
				}
			}

		case SERVICE_GET_INFO:
			{
			CHyperionEngine *pEngine = (CHyperionEngine *)pCtx->GetLibraryCtx(LIBRARY_HYPERION);
			SHTTPRequestCtx *pSessionCtx = (SHTTPRequestCtx *)pCtx->GetLibraryCtx(LIBRARY_SESSION_CTX);
			if (pSessionCtx == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CHyperionPackageList::SPackageInfo PackageInfo;
			if (pEngine)
				pEngine->GetPackageInfo(pSessionCtx->pService->GetPackageName(), PackageInfo);

			CDatum dResult(new CComplexStruct);
			dResult.SetElement(FIELD_NAME, pSessionCtx->pService->GetName());
			dResult.SetElement(FIELD_PACKAGE_NAME, pSessionCtx->pService->GetPackageName());
			dResult.SetElement(FIELD_PACKAGE_VERSION, PackageInfo.sVersion);

			//	Done

			retResult.dResult = dResult;
			return true;
			}

		case SERVICE_PARSE_COMMAND_LINE:
			{
			CStringView sCmdLine = LocalEnv.GetArgument(0);

			//	Parse

			const char *pPos = sCmdLine.GetParsePointer();
			const char *pPosEnd = pPos + sCmdLine.GetLength();

			switch (*pPos)
				{
				//	Handle some special one-character commands

				case '=':
					{
					//	Command is the character

					CDatum dResult(CDatum::typeArray);
					dResult.Append(CMD_EVAL);

					CDatum dParams(CDatum::typeArray);
					dResult.Append(dParams);

					//	The remaining characters are the argument.

					pPos++;
					if (pPos < pPosEnd)
						dParams.Append(CString(pPos, (pPosEnd - pPos)));

					//	Done

					retResult.dResult = dResult;
					return true;
					}

				//	Otherwise, parse commands by whitespace

				default:
					{
					//	First we parse the command, which is delimited by whitespace

					while (pPos < pPosEnd && strIsWhitespace(pPos))
						pPos++;

					const char *pStart = pPos;
					while (pPos < pPosEnd && !strIsWhitespace(pPos))
						pPos++;

					CString sCmd(pStart, pPos - pStart);

					//	Now parse the parameters into an array

					CDatum dParams(CDatum::typeArray);
					while (pPos < pPosEnd)
						{
						//	Skip leading whitespace

						while (pPos < pPosEnd && strIsWhitespace(pPos))
							pPos++;

						//	If we hit the end, then we're done

						if (pPos == pPosEnd)
							break;

						//	Parse the param

						CBuffer Buffer(pPos, (int)(pPosEnd - pPos), false);
						CDatum dParam;
						if (!CDatum::Deserialize(CDatum::EFormat::AEONScript, Buffer, &dParam))
							{
							CHexeError::Create(NULL_STR, ERR_BAD_COMMAND_LINE, &retResult.dResult);
							return false;
							}

						dParams.Append(dParam);

						//	Adjust pPos

						pPos += Buffer.GetPos();
						}

					//	Return the command and the parameters in an array

					CDatum dResult(CDatum::typeArray);
					dResult.Append(sCmd);
					dResult.Append(dParams);

					retResult.dResult = dResult;
					return true;
					}
				}
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool sessionMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	switch (dwData)
		{
		case SESSION_LOG:
			{
			int i;

			//	Concatenate the args

			CStringBuffer Buffer;

			for (i = 0; i < LocalEnv.GetCount(); i++)
				{
				CString sString = LocalEnv.GetArgument(i).AsString();
				Buffer.Write((LPSTR)sString, sString.GetLength());
				}

			CDatum::CreateStringFromHandoff(Buffer, &retResult.dResult);

			//	Get a session context. If we find one, then log through that (so that
			//	we can get the session ID).

			CHyperionSession *pSession = (CHyperionSession *)pCtx->GetLibraryCtx(LIBRARY_SESSION);
			if (pSession)
				{
				pSession->DebugLog(retResult.dResult.AsString());
				return true;
				}

			//	Otherwise, get an engine context.

			CHyperionEngine *pEngine = (CHyperionEngine *)pCtx->GetLibraryCtx(LIBRARY_HYPERION);
			if (pEngine)
				{
				pEngine->GetProcessCtx()->Log(MSG_LOG_INFO, retResult.dResult.AsString());
				return true;
				}

			//	Otherwise, nothing

			retResult.dResult = CDatum();
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool userMisc (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	switch (dwData)
		{
		case USER_GET_NAME:
			{
			CHyperionSession *pSession = (CHyperionSession *)pCtx->GetLibraryCtx(LIBRARY_SESSION);
			if (pSession == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = pSession->GetSecurityCtx().GetUsername();
			return true;
			}

		case USER_GET_RIGHTS:
			{
			CHyperionSession *pSession = (CHyperionSession *)pCtx->GetLibraryCtx(LIBRARY_SESSION);
			if (pSession == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			if (!CDatum::CreateFromAttributeList(pSession->GetSecurityCtx().GetUserRights(), &retResult.dResult))
				{
				retResult.dResult = ERR_CANT_GET_RIGHTS;
				return false;
				}

			return true;
			}

		case USER_HAS_RIGHT:
			{
			CHyperionSession *pSession = (CHyperionSession *)pCtx->GetLibraryCtx(LIBRARY_SESSION);
			if (pSession == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			if (pSession->GetSecurityCtx().HasUserRight(LocalEnv.GetArgument(0).AsStringView()))
				retResult.dResult = CDatum(true);
			else
				retResult.dResult = CDatum();
			return true;
			}

		case USER_SET:
			{
			CHyperionSession *pSession = (CHyperionSession *)pCtx->GetLibraryCtx(LIBRARY_SESSION);
			if (pSession == NULL)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CHexeSecurityCtx &SecurityCtx = pSession->GetSecurityCtx();

			CDatum dUsername = LocalEnv.GetArgument(0);
			CDatum dRights = LocalEnv.GetArgument(1);

			if (!dUsername.IsNil())
				{
				SecurityCtx.SetUsername(dUsername.AsString());
				SecurityCtx.SetUserRights(dRights);
				}
			else
				SecurityCtx.SetAnonymous();

			//	Make sure we update the process also

			pCtx->SetUserSecurity(SecurityCtx.GetUsername(), SecurityCtx.GetUserRights());

			//	Done

			retResult.dResult = CDatum(true);
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}
