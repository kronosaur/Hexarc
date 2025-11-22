//	CWWWService.cpp
//
//	CWWWService class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_OK,							"OK");
DECLARE_CONST_STRING(STR_STAR,							"*");

DECLARE_CONST_STRING(FIELD_DEFAULT_FILE,				"defaultFile");
DECLARE_CONST_STRING(FIELD_FILE_PATHS,					"filePaths");
DECLARE_CONST_STRING(FIELD_URL_PATHS,					"urlPaths");

DECLARE_CONST_STRING(ERR_404_NOT_FOUND,					"Not Found");

DECLARE_CONST_STRING(LIBRARY_SESSION,					"session");

DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html");

DECLARE_CONST_STRING(ERR_MUST_HAVE_DEFAULT_FILE,		"WWW service must have default file when using * path pattern.");

CString CWWWService::GetFilePath (const CString &sPath)

//	GetFilePath
//
//	Converts from a web path to a file path

	{
	if (!strStartsWith(sPath, m_sWebPath))
		return NULL_STR;

	//	The file path looks like:
	//
	//	/Arc.services/{package}{filePathRoot}{pathRemainder}

	return strPattern("/Arc.services/%s%s%s",
			GetPackageName(),
			m_sFilePath,
			strSubString(sPath, m_sWebPath.GetLength()));
	}

CString CWWWService::OnGetFileRoot (void) const

//	OnGetFileRoot
//
//	Returns the root filePath of the package files for this service.

	{
	return strPattern("/Arc.services/%s%s", GetPackageName(), m_sFilePath);
	}

bool CWWWService::OnHandleHexmFile (SHTTPRequestCtx &Ctx, CDatum dFileDesc, CDatum dData)

//	OnHandleHexmFile
//
//	Loads and evaluates a hexm file.

	{
	//	Set the context for the session

	Ctx.pSession->SetServiceSecurity(GetSecurityCtx());

	//	Allocate a new markup evaluator object and let it process.
	//	It is responsible for composing the response (or making an RPC
	//	call if necessary).

	ASSERT(Ctx.pHexeEval == NULL);
	Ctx.pHexeEval = new CHexeMarkupEvaluator;
	return Ctx.pHexeEval->EvalInit(Ctx, m_ProcessTemplate, Ctx.pSession->GetSecurityCtx(), m_HexeDefinitions, dFileDesc, dData);
	}

bool CWWWService::OnHandleRequest (SHTTPRequestCtx &Ctx)

//	OnHandleRequest
//
//	Handle a request. We return TRUE if Ctx.Reponse is properly initialized.
//	Otherwise, we initialize the RPC request fields in Ctx and return false;

	{
	//	Get the request parameters

	CDatum dQuery;
	CString sPath;
	urlParseQuery(Ctx.Request.GetRequestedPath(), &sPath, &dQuery);
	const CString &sMethod = Ctx.Request.GetMethod();

	//	If this is a single file, then we always return the default file.

	if (m_bSingleFile)
		sPath = urlAppend(m_sWebPath, m_sDefaultFile);

	//	If this is the root then we supply the default file.

	else if (strEquals(sPath, m_sWebPath) && !m_sDefaultFile.IsEmpty())
		sPath = urlAppend(sPath, m_sDefaultFile);

	//	Translate the path to a filePath

	CString sFilePath = GetFilePath(sPath);
	if (sFilePath.IsEmpty())
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_NOT_FOUND, ERR_404_NOT_FOUND);
		return true;
		}

	//	Ask the session to load the file.

	Ctx.iStatus = pstatFilePathReady;
	Ctx.sFilePath = sFilePath;
	return false;
	}

bool CWWWService::OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult)

//	OnHandleRPCResult
//
//	Handle a resulting message. We can get here in one of two ways:
//
//	1.	The RPC call to get the file from Aeon has returned (Ctx.pHexeEval == NULL)
//	2.	We're in the middle of evaluating a page and an RPC call has returned.
//		(Ctx.pHexeEval != NULL)

	{
	//	Pass the RPC to the hexm evaluator

	if (Ctx.pHexeEval)
		return Ctx.pHexeEval->EvalContinues(Ctx, RPCResult);

	//	This should never happen

	else
		{
		ASSERT(false);
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_NOT_FOUND, ERR_404_NOT_FOUND);
		return true;
		}
	}

bool CWWWService::OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError)

//	OnHTTPInit
//
//	Initialize from service definition

	{
	//	Get some basic info

	m_sWebPath = dServiceDef.GetElement(FIELD_URL_PATHS).GetElement(0).AsStringView();
	m_sFilePath = dServiceDef.GetElement(FIELD_FILE_PATHS).GetElement(0).AsStringView();
	m_sDefaultFile = dServiceDef.GetElement(FIELD_DEFAULT_FILE).AsStringView();

	//	If the web path ends in * then it means that all requests starting with 
	//	that path go to the default file (which can then parse the rest of the
	//	path).

	if (strEndsWith(m_sWebPath, STR_STAR))
		{
		m_sWebPath = strSubString(m_sWebPath, 0, m_sWebPath.GetLength() - 1);
		m_bSingleFile = true;

		if (m_sDefaultFile.IsEmpty())
			{
			if (retsError) *retsError = ERR_MUST_HAVE_DEFAULT_FILE;
			return false;
			}
		}
	else
		m_bSingleFile = false;

	//	Initialize the process template

	m_ProcessTemplate.SetSecurityCtx(GetSecurityCtx());

	if (!m_ProcessTemplate.LoadStandardLibraries(retsError))
		return false;

	if (!m_ProcessTemplate.LoadLibrary(LIBRARY_SESSION, retsError))
		return false;

	if (!m_ProcessTemplate.LoadEntryPoints(Package, retsError))
		return false;

	//	Load HexeDefinitions (we apply these later since they have to be tied
	//	to the execution process [so that global variables work out.])

	Package.GetHexeDefinitions(&m_HexeDefinitions);

	//	Done

	return true;
	}

void CWWWService::OnHTTPMark (void)

//	OnHTTPMark
//
//	Mark data in use

	{
	int i;

	m_ProcessTemplate.Mark();

	for (i = 0; i < m_HexeDefinitions.GetCount(); i++)
		m_HexeDefinitions[i].Mark();
	}
