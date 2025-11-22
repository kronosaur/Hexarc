//	CHTTPService.cpp
//
//	CHTTPService class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_OK,							"OK")

DECLARE_CONST_STRING(FIELD_FORMAT,						"format")
DECLARE_CONST_STRING(FIELD_HOSTS,						"hosts")
DECLARE_CONST_STRING(FIELD_REDIRECT,					"redirect")
DECLARE_CONST_STRING(FIELD_REDIRECTS,					"redirects")
DECLARE_CONST_STRING(FIELD_SERVICE,						"service")
DECLARE_CONST_STRING(FIELD_TLS,							"tls")
DECLARE_CONST_STRING(FIELD_URL_PATH,					"urlPath")
DECLARE_CONST_STRING(FIELD_URL_PATHS,					"urlPaths")

DECLARE_CONST_STRING(HEADER_LOCATION,					"location")

DECLARE_CONST_STRING(PORT_DEFAULT_TLS,					"443")

DECLARE_CONST_STRING(PROTOCOL_HTTP,						"http")
DECLARE_CONST_STRING(PROTOCOL_HTTPS,					"https")
DECLARE_CONST_STRING(PROTOCOL_TLS,						"tls")

DECLARE_CONST_STRING(SERVICE_HEXCODE,					"hexcode")
DECLARE_CONST_STRING(SERVICE_PROXY,						"proxy")
DECLARE_CONST_STRING(SERVICE_WEB_SOCKET,				"webSocket")
DECLARE_CONST_STRING(SERVICE_WWW,						"www")

DECLARE_CONST_STRING(TLS_NONE,							"none")
DECLARE_CONST_STRING(TLS_OPTIONAL,						"optional")
DECLARE_CONST_STRING(TLS_REQUIRED,						"required")

DECLARE_CONST_STRING(ERR_INVALID_TLS,					"Invalid TLS parameter: %s.")
DECLARE_CONST_STRING(ERR_INVALID_REDIRECT_PATH,			"Invalid redirect urlPath: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_SERVICE,				"Unknown http service: %s.")

DECLARE_CONST_STRING(STR_MOVED_PERMANENTLY,				"Moved Permanently")

CHTTPService *CHTTPService::AsHTTPService (IHyperionService *pService)

//	AsHTTPService
//
//	Casts a service to a CHTTPService (based on the protocol)

	{
	if (pService == NULL)
		return NULL;

	const CString &sProtocol = pService->GetProtocol();
	if (strEquals(sProtocol, PROTOCOL_HTTP))
		return (CHTTPService *)pService;
	else
		return NULL;
	}

bool CHTTPService::CreateServiceClass (CDatum dServiceDef, const CHexeDocument &Package, IHyperionService **retpService, CString *retsError)

//	CreateServiceClass
//
//	Creates a specific http service class

	{
	//	Figure out which service class to create

	CStringView sService = dServiceDef.GetElement(FIELD_SERVICE);
	IHyperionService *pService;
	if (strEquals(sService, SERVICE_HEXCODE))
		pService = new CHexeCodeRPCService;
	else if (strEquals(sService, SERVICE_PROXY))
		pService = new CHTTPProxyService;
	else if (strEquals(sService, SERVICE_WEB_SOCKET))
		pService = new CWebSocketService;
	else if (strEquals(sService, SERVICE_WWW))
		pService = new CWWWService;
	else
		{
		if (retsError)
			*retsError = strPattern(ERR_UNKNOWN_SERVICE, sService);
		return false;
		}

	//	Done

	*retpService = pService;
	return true;
	}

bool CHTTPService::HandleRequest (SHTTPRequestCtx &Ctx)

//	HandleRequest
//
//	Handle a request
	
	{
	int i;

	//	Parse URL

	CString sURLHost;
	CString sURLPath;
	Ctx.Request.ParseRequestedURL(NULL, &sURLHost, &sURLPath);

	//	If we require TLS, then redirect HTTP to HTTPS

	if (m_iTLS == tlsRequired 
			&& !strEquals(Ctx.pSession->GetProtocol(), PROTOCOL_TLS))
		{
		Ctx.iStatus = pstatResponseReady;
		Ctx.Response.InitResponse(http_MOVED_PERMANENTLY, STR_MOVED_PERMANENTLY);

		CString sRedirect = ::strPattern("https://%s%s", sURLHost, sURLPath);
		Ctx.Response.AddHeader(HEADER_LOCATION, sRedirect);
		return true;
		}

	//	Convert the URL to a relative path before checking for redirects.

	sURLPath = MakeRelativePath(sURLPath);

	//	See if we need to redirect.

	for (i = 0; i < m_Redirects.GetCount(); i++)
		{
		const SRedirectDesc &Redirect = m_Redirects[i];

		//	If we match a redirect line, then redirect

		CString sRedirect;
		if (MatchRedirect(Redirect, sURLHost, sURLPath, &sRedirect))
			{
			Ctx.iStatus = pstatResponseReady;
			Ctx.Response.InitResponse(http_MOVED_PERMANENTLY, STR_MOVED_PERMANENTLY);
			Ctx.Response.AddHeader(HEADER_LOCATION, sRedirect);
			return true;
			}
		}

	//	Let our subclasses deal with it.

	return OnHandleRequest(Ctx); 
	}

CString CHTTPService::MakePathCanonical (const CString &sPath)

//	MakePathCanonical
//
//	Make sure that the path begins and ends with a slash

	{
	char *pPos = sPath.GetParsePointer();
	char *pPosEnd = pPos + sPath.GetLength();
	if (pPos == pPosEnd)
		return NULL_STR;

	//	If we end with a star, then remove it

	bool bRemoveStar = false;
	if (pPosEnd[-1] == '*')
		{
		pPosEnd--;
		bRemoveStar = true;
		}

	bool bAddForwardSlash = (*pPos != '/');
	bool bAddTrailingSlash = (pPosEnd[-1] != '/');
	if (!bAddForwardSlash && !bAddTrailingSlash && !bRemoveStar)
		return sPath;

	int iCopyLen = (int)(pPosEnd - pPos);

	int iResultLen = iCopyLen
			+ (bAddForwardSlash ? 1 : 0)
			+ (bAddTrailingSlash ? 1 : 0);

	CString sResult(iResultLen);
	char *pDest = sResult.GetParsePointer();
	
	if (bAddForwardSlash)
		{
		*pDest = '/';
		pDest++;
		}

	utlMemCopy(pPos, pDest, iCopyLen);

	if (bAddTrailingSlash)
		{
		*pDest = '/';
		pDest++;
		}

	return sResult;
	}

CString CHTTPService::MakeRelativePath(const CString &sPath)

//	MakeRelativePath
//
//	Converts an absolute path (excluding domain) to a path that is relative to
//	one of the URL paths that this service hosts.
//
//	The result does not have a leading / (or a trailing /)
//
//	EXAMPLE:
//
//	m_PathsToServe				sPath						Result
//	----------------------------------------------------------------------------
//	/transcendence/				/transcendence/test			test
//	/transcendence/				/transcendence/test			test
//	/transcendence/				/transcendence/test/sub1	test/sub1
//	/transcendence/				/foo/test					NULL_STR
//
//	NOTE: We rely on the fact the m_PathsToServe paths are canonical
//	(see MakePathCanonical).

	{
	int i;

	if (sPath.IsEmpty())
		return NULL_STR;

	for (i = 0; i < m_PathsToServe.GetCount(); i++)
		{
		if (strStartsWith(sPath, m_PathsToServe[i]))
			{
			char *pPos = sPath.GetParsePointer();
			char *pPosEnd = pPos + sPath.GetLength();

			pPos += m_PathsToServe[i].GetLength();

			//	If the same path, then the relative path is NULL_STR.
			//	(We are at the root on one of the paths).

			if (pPos == pPosEnd)
				return NULL_STR;

			//	Is there a trailing slash? If so, delete it.

			if (pPosEnd[-1] == '/')
				pPosEnd--;

			//	Returns result.

			CString sResult((int)(pPosEnd - pPos));
			char *pDest = sResult.GetParsePointer();
			
			while (pPos < pPosEnd)
				*pDest++ = *pPos++;

			return sResult;
			}
		}

	//	If we get this far then no path matched, which is an error because some
	//	other part should have caught it.

	ASSERT(false);
	return NULL_STR;
	}

bool CHTTPService::MatchRedirect (const SRedirectDesc &Desc, const CString &sURLHost, const CString &sURLPath, CString *retsRedirect) const

//	MatchRedirect
//
//	Returns TRUE if we match the given redirect descriptor

	{
	//	Match host?

	if (Desc.fMatchHost 
			&& !strEquals(sURLHost, Desc.sHost))
		return false;

	//	If we're matching all paths, then we always continue

	if (Desc.fMatchAllPaths)
		{ }

	//	If we need to match an exact path and it doesn't match exactly, then
	//	we're done.

	else if (Desc.fMatchExactPath
			&& !strEquals(sURLPath, Desc.sPath))
		return false;

	//	If the URL doesn't even start with the path we want, then wildcards
	//	won't match either, so skip it.

	else if (!strStartsWith(sURLPath, Desc.sPath))
		return false;

	//	If we're matching a directory, then we either need an exact match, or
	//	we need to match with a slash.
	//
	//	NOTE: We guarantee that sURLPath doesn't have a trailing / because
	//	it is stripped out by MakeRelativePath.

	else if (Desc.fMatchDir
			&& !strEquals(sURLPath, Desc.sPath)
			&& !strStartsWith(sURLPath, Desc.sPathWithSlash))
		return false;

	//	If we get this far, we match. Figure out what the redirect URL is.

	if (Desc.fReplaceWildcard)
		{
		if (Desc.fMatchAllPaths || !Desc.fMatchExactPath)
			{
			if (Desc.fMatchDir && sURLPath.GetLength() > Desc.sPathWithSlash.GetLength())
				*retsRedirect = Desc.sRedirect + strSubString(sURLPath, Desc.sPathWithSlash.GetLength());
			else
				*retsRedirect = Desc.sRedirect + strSubString(sURLPath, Desc.sPath.GetLength());
			return true;
			}
		else
			{
			*retsRedirect = Desc.sRedirect;
			return true;
			}
		}
	else
		{
		*retsRedirect = Desc.sRedirect;
		return true;
		}
	}

int CHTTPService::MatchHostAndURL (const CString &sHost, const CString &sURL)

//	MatchHostAndURL
//
//	Returns a value indicating the degree to which this service would like
//	to handle a request for the given host and URL.
//
//	A value of 0 means that we do not handle the URL at all.

	{
	int i;

	//	If we have no hosts in the list, then we handle all hosts.
	//	Otherwise, check to see if we handle this host.

	if (m_HostsToServe.GetCount() > 0)
		{
		bool bFound = false;
		for (i = 0; i < m_HostsToServe.GetCount(); i++)
			if (strEquals(sHost, m_HostsToServe[i]))
				{
				bFound = true;
				break;
				}

		if (!bFound)
			return 0;
		}

	//	Find the longest URL that we match

	int iBestMatch = 0;
	for (i = 0; i < m_PathsToServe.GetCount(); i++)
		if (strStartsWith(sURL, m_PathsToServe[i]))
			{
			//	NOTE: We rely on the fact that m_PathsToServe uses canonical
			//	paths, which always end in a slash.

			//	Longer matches are better (more specific).

			int iMatch = m_PathsToServe[i].GetLength();

			if (iMatch > iBestMatch)
				iBestMatch = iMatch;
			}

	//	Done

	return iBestMatch;
	}

void CHTTPService::OnGetListeners (TArray<SListenerDesc> &Listeners) const

//	OnGetPortList
//
//	Returns a list of ports to listen on.

	{
	Listeners.DeleteAll();

	if (!GetPort().IsEmpty())
		{
		SListenerDesc *pDesc = Listeners.Insert();
		pDesc->sProtocol = NULL_STR;	//	raw protocol is the default
		pDesc->sPort = GetPort();

		//	If we support TLS, then we also want to listen on 443.

		if (m_iTLS != tlsNone)
			{
			pDesc = Listeners.Insert();
			pDesc->sProtocol = PROTOCOL_TLS;
			pDesc->sPort = PORT_DEFAULT_TLS;
			}
		}
	}

bool CHTTPService::OnInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError)

//	OnInit
//
//	Initialize the service from the service definition

	{
	int i;

	//	Get some basic info

	m_sService = dServiceDef.GetElement(FIELD_SERVICE).AsStringView();
	m_sFormat = dServiceDef.GetElement(FIELD_FORMAT).AsStringView();

	//	TLS

	CStringView sTLS = dServiceDef.GetElement(FIELD_TLS);
	if (sTLS.IsEmpty() || strEqualsNoCase(sTLS, TLS_NONE))
		m_iTLS = tlsNone;
	else if (strEqualsNoCase(sTLS, TLS_OPTIONAL))
		m_iTLS = tlsOptional;
	else if (strEqualsNoCase(sTLS, TLS_REQUIRED))
		m_iTLS = tlsRequired;
	else
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_TLS, sTLS);
		return false;
		}

	//	Get the list of hosts that we serve

	CDatum dHosts = dServiceDef.GetElement(FIELD_HOSTS);
	for (i = 0; i < dHosts.GetCount(); i++)
		m_HostsToServe.Insert(dHosts.GetElement(i).AsStringView());

	//	Get the paths that we serve

	CDatum dPaths = dServiceDef.GetElement(FIELD_URL_PATHS);
	for (i = 0; i < dPaths.GetCount(); i++)
		{
		CString sPath = MakePathCanonical(dPaths.GetElement(i).AsStringView());
		if (!sPath.IsEmpty())
			m_PathsToServe.Insert(sPath);

		//	LATER: Make sure paths are proper URL paths
		}

	//	Redirects

	CDatum dRedirects = dServiceDef.GetElement(FIELD_REDIRECTS);
	for (i = 0; i < dRedirects.GetCount(); i++)
		{
		CDatum dDesc = dRedirects.GetElement(i);

		CStringView sPattern = dDesc.GetElement(FIELD_URL_PATH);
		if (sPattern.IsEmpty())
			continue;

		SRedirectDesc *pRedirect = m_Redirects.Insert();

		//	Parse the path URL

		CString sHost;
		CString sPath;
		if (!urlParse(sPattern, NULL, &sHost, &sPath))
			{
			if (retsError) *retsError = strPattern(ERR_INVALID_REDIRECT_PATH, sPattern);
			return false;
			}

		//	If we have a host, then we need to match it.

		if (!sHost.IsEmpty())
			{
			pRedirect->sHost = sHost;
			pRedirect->fMatchHost = true;
			}

		//	Parse the path

		int iPathStart = 0;
		int iPathLength = sPath.GetLength();

		//	If the path has a leading '/' remove it. [We only accept paths
		//	relative to the root.]

		const char *pPos = sPath.GetParsePointer();
		if (*pPos == '/')
			{
			iPathStart++;
			iPathLength--;
			}

		//	If we end with a '*' then remove it and set a flag

		if (iPathLength > 0 && pPos[iPathStart + iPathLength - 1] == '*')
			{
			iPathLength--;

			//	If we have no path left then it means we had "/*" for a path
			//	which means match everything.

			if (iPathLength <= 0)
				pRedirect->fMatchAllPaths = true;
			else
				pRedirect->fMatchExactPath = false;
			}

		//	If we end with '/' then remove it because we always remove trailing
		//	slashes when matching.

		if (iPathLength > 0 && pPos[iPathStart + iPathLength - 1] == '/')
			{
			pRedirect->sPathWithSlash = strSubString(sPath, iPathStart, iPathLength);

			iPathLength--;
			pRedirect->fMatchDir = true;
			}

		//	Add the path

		pRedirect->sPath = strSubString(sPath, iPathStart, iPathLength);

		//	If the redirect ends in '*' then remove it and set a flag

		CStringView sRedirectPath = dDesc.GetElement(FIELD_REDIRECT);

		pPos = sRedirectPath.GetParsePointer();
		if (pPos[sRedirectPath.GetLength() - 1] == '*')
			{
			pRedirect->sRedirect = strSubString(sRedirectPath, 0, sRedirectPath.GetLength() - 1);
			pRedirect->fReplaceWildcard = true;
			}
		else
			pRedirect->sRedirect = sRedirectPath;
		}

	//	Let derrived classes initialize

	return OnHTTPInit(dServiceDef, Package, retsError);
	}
