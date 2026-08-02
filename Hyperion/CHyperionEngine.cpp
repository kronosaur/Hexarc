//	CHyperionEngine.cpp
//
//	CHyperionEngine class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_CHYPERION_ENGINE_QUESTION,	"?");
DECLARE_CONST_STRING(STR_CHYPERION_ENGINE_SLASH,	"/");
DECLARE_CONST_STRING(STR_CHYPERION_ENGINE_HYPERION_SESSION_COUNT,	"Hyperion/sessionCount");
DECLARE_CONST_STRING(STR_CHYPERION_ENGINE_HYPERION_THREADS_PROCESSING,	"Hyperion/threadsProcessing");
DECLARE_CONST_STRING(STR_CHYPERION_ENGINE_HYPERION_THREADS_STUCK,	"Hyperion/threadsStuck");
DECLARE_CONST_STRING(STR_CHYPERION_ENGINE_HYPERION_THREADS_WAITING,	"Hyperion/threadsWaiting");
DECLARE_CONST_STRING(STR_CHYPERION_ENGINE_HYPERION_THREAD_COUNT,	"Hyperion/threadCount");

DECLARE_CONST_STRING(RESID_ARCOLOGY_PACKAGE,			"ArcologyPackage")

DECLARE_CONST_STRING(VIRTUAL_PORT_AEON_NOTIFY,			"Aeon.notify")
DECLARE_CONST_STRING(VIRTUAL_PORT_CRYPTOSAUR_NOTIFY,	"Cryptosaur.notify")
DECLARE_CONST_STRING(VIRTUAL_PORT_HYPERION_COMMAND,		"Hyperion.command")

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null")
DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command")
DECLARE_CONST_STRING(ADDRESS_HYPERION_COMMAND,			"Hyperion.command@~/~")
DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command")

DECLARE_CONST_STRING(ENGINE_NAME_HYPERION,				"Hyperion")

DECLARE_CONST_STRING(FIELD_CREATED_ON,					"createdOn")
DECLARE_CONST_STRING(FIELD_ACTION,						"action")
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath")
DECLARE_CONST_STRING(FIELD_HOSTNAME,					"hostname")
DECLARE_CONST_STRING(FIELD_ID,							"id")
DECLARE_CONST_STRING(FIELD_MODIFIED_BY,					"modifiedBy")
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn")
DECLARE_CONST_STRING(FIELD_MATCH,						"match")
DECLARE_CONST_STRING(FIELD_MATCH_MODE,					"matchMode")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_OPTIONS,						"options")
DECLARE_CONST_STRING(FIELD_PACKAGE,						"package")
DECLARE_CONST_STRING(FIELD_PORT,						"port")
DECLARE_CONST_STRING(FIELD_PRIMARY_KEY,					"primaryKey")
DECLARE_CONST_STRING(FIELD_PRIORITY,					"priority")
DECLARE_CONST_STRING(FIELD_PROTOCOL,					"protocol")
DECLARE_CONST_STRING(FIELD_ROUTE_ID,					"routeID")
DECLARE_CONST_STRING(FIELD_SANDBOX,						"sandbox")
DECLARE_CONST_STRING(FIELD_SERVICE,						"service")
DECLARE_CONST_STRING(FIELD_SOURCE,						"source")
DECLARE_CONST_STRING(FIELD_STATUS,						"status")
DECLARE_CONST_STRING(FIELD_TARGET,						"target")
DECLARE_CONST_STRING(FIELD_UNENCRYPTED_PORT,			"unencryptedPort")
DECLARE_CONST_STRING(FIELD_URL_PATH,					"urlPath")
DECLARE_CONST_STRING(FIELD_VERSION,						"version")

DECLARE_CONST_STRING(ACTION_INTERNAL_REWRITE,			"internalRewrite")
DECLARE_CONST_STRING(ACTION_REDIRECT,					"redirect")
DECLARE_CONST_STRING(ACTION_SERVICE,					"service")

DECLARE_CONST_STRING(MATCH_EXACT,						"exact")
DECLARE_CONST_STRING(MATCH_PREFIX,						"prefix")

DECLARE_CONST_STRING(PROTOCOL_HTTP,						"http")
DECLARE_CONST_STRING(PROTOCOL_TLS,						"tls")

DECLARE_CONST_STRING(PORT_DEFAULT_HTTP,					"80")
DECLARE_CONST_STRING(PORT_DEFAULT_TLS,					"443")

DECLARE_CONST_STRING(STATUS_ACTIVE,						"active")
DECLARE_CONST_STRING(STR_STAR,							"*")
DECLARE_CONST_STRING(SOURCE_SERVICE_DOC,				"serviceDoc")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_AEON_GET_ROWS,					"Aeon.getRows")
DECLARE_CONST_STRING(MSG_AEON_GET_VALUE,				"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate")
DECLARE_CONST_STRING(MSG_ESPER_START_LISTENER,			"Esper.startListener")
DECLARE_CONST_STRING(MSG_ESPER_STOP_LISTENER,			"Esper.stopListener")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_HYPERION_BIND_HOST_REDIRECT,	"Hyperion.bindHostRedirect")
DECLARE_CONST_STRING(MSG_HYPERION_DELETE_ROUTE,			"Hyperion.deleteRoute")
DECLARE_CONST_STRING(MSG_HYPERION_SET_ROUTE,			"Hyperion.setRoute")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(ERR_UNABLE_TO_CONTINUE,			"Hyperion internal error: %s")
DECLARE_CONST_STRING(ERR_ACCESS_DENIED,					"Access denied.")
DECLARE_CONST_STRING(ERR_INVALID_PARAMS,				"Invalid parameters.")
DECLARE_CONST_STRING(ERR_HOST_BINDING_NOT_FOUND,		"Host binding not found: %s %s.")
DECLARE_CONST_STRING(ERR_HOST_BINDING_RELOAD_FAILED,	"Arc.hosts updated, but Hyperion could not reload host bindings: %s")
DECLARE_CONST_STRING(ERR_INVALID_HOST_BINDING,			"Invalid host binding: %s")
DECLARE_CONST_STRING(ERR_INVALID_ROUTE_UPDATE_FIELD,	"Route field cannot be updated: %s.")
DECLARE_CONST_STRING(ERR_ROUTE_NOT_FOUND,				"Route not found: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_SERVICE,				"Unknown HTTP service: %s.")
DECLARE_CONST_STRING(ERR_HOST_PACKAGE_MISMATCH,			"Host binding package %s does not match service package %s.")

const int THREAD_COUNT = 6;
static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;

DECLARE_CONST_STRING(MUTATE_CODE8,						"code8")
DECLARE_CONST_STRING(MUTATE_PRIMARY_KEY,				"primaryKey")
DECLARE_CONST_STRING(OPTION_INCLUDE_KEY,				"includeKey")
DECLARE_CONST_STRING(SOURCE_ADMIN,						"admin")
DECLARE_CONST_STRING(STATUS_DISABLED,					"disabled")
DECLARE_CONST_STRING(TABLE_ARC_HOSTS,					"Arc.hosts")

static CString MakeRoutePathCanonical (const CString &sPath)
	{
	char *pPos = sPath.GetParsePointer();
	char *pPosEnd = pPos + sPath.GetLength();
	if (pPos == pPosEnd)
		return NULL_STR;

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
		*pDest++ = '/';

	utlMemCopy(pPos, pDest, iCopyLen);
	pDest += iCopyLen;

	if (bAddTrailingSlash)
		*pDest++ = '/';

	return sResult;
	}

static CString NormalizeRouteHost (const CString &sHost)
	{
	CString sResult = strToLower(sHost);

	while (!sResult.IsEmpty())
		{
		char *pPos = sResult.GetParsePointer();
		if (pPos[sResult.GetLength() - 1] != '.')
			break;

		sResult = strSubString(sResult, 0, sResult.GetLength() - 1);
		}

	return sResult;
	}

static CString GetRouteMatchMode (CDatum dOptions)
	{
	CString sMatch = dOptions.GetElement(FIELD_MATCH).AsString();
	if (sMatch.IsEmpty())
		sMatch = dOptions.GetElement(FIELD_MATCH_MODE).AsString();
	if (sMatch.IsEmpty())
		sMatch = MATCH_PREFIX;

	return sMatch;
	}

static CString StripQueryString (const CString &sURL)
	{
	int iQuery = strFind(sURL, STR_CHYPERION_ENGINE_QUESTION);
	return (iQuery == -1 ? sURL : strSubString(sURL, 0, iQuery));
	}

static bool RouteMatchesPath (CDatum dOptions, const CString &sRouteURLPath, const CString &sURLToMatch, bool *retbExactPath)
	{
	CString sMatchMode = GetRouteMatchMode(dOptions);
	CString sPathToMatch = StripQueryString(sURLToMatch);

	if (strEquals(sMatchMode, MATCH_EXACT))
		{
		if (retbExactPath)
			*retbExactPath = true;

		return strEquals(MakeRoutePathCanonical(sPathToMatch), sRouteURLPath);
		}
	else
		{
		if (retbExactPath)
			*retbExactPath = false;

		return strStartsWith(sPathToMatch, sRouteURLPath)
				|| strEquals(MakeRoutePathCanonical(sPathToMatch), sRouteURLPath);
		}
	}

static bool RouteMatchesProtocolAndPort (const CString &sRouteProtocol, const CString &sRoutePort, const CString &sUnencryptedPort, const CString &sProtocolToMatch, const CString &sPortToMatch)
	{
	if (strEquals(sRouteProtocol, sProtocolToMatch) && strEquals(sRoutePort, sPortToMatch))
		return true;

	return (!sUnencryptedPort.IsEmpty()
			&& strEquals(sProtocolToMatch, PROTOCOL_HTTP)
			&& strEquals(sUnencryptedPort, sPortToMatch));
	}

static CString GetHostBindingID (CDatum dHost)
	{
	CString sID = dHost.GetElement(FIELD_ID).AsString();
	if (sID.IsEmpty())
		sID = dHost.GetElement(FIELD_PRIMARY_KEY).AsString();

	return sID;
	}

static CString GetHostBindingHostName (CDatum dHost)
	{
	return dHost.GetElement(FIELD_HOSTNAME).AsString();
	}

static CString GetHostBindingServiceName (CDatum dHost)
	{
	CString sService = dHost.GetElement(FIELD_TARGET).AsString();
	if (sService.IsEmpty())
		sService = dHost.GetElement(FIELD_SERVICE).AsString();

	return sService;
	}

static CString GetHostBindingAction (CDatum dHost)
	{
	CString sAction = strClean(dHost.GetElement(FIELD_ACTION).AsString());
	if (sAction.IsEmpty())
		sAction = ACTION_SERVICE;

	return sAction;
	}

static bool HasElement (CDatum dStruct, const CString &sField)
	{
	CDatum dValue;
	return dStruct.FindElement(sField, &dValue);
	}

static bool SetRoutePatchElement (CDatum dPatch, const CString &sField, CDatum *iodHost)
	{
	CDatum dValue;
	if (!dPatch.FindElement(sField, &dValue))
		return false;

	iodHost->SetElement(sField, dValue);
	return true;
	}

static bool ComposeRoutePatch (CDatum dOriginal, CDatum dPatch, CDatum *retdHost, CString *retsError)
	{
	if (!dPatch.IsStruct())
		{
		if (retsError) *retsError = ERR_INVALID_PARAMS;
		return false;
		}

	const CString *DisallowedFields[] =
		{
		&FIELD_ID,
		&FIELD_PRIMARY_KEY,
		&FIELD_ROUTE_ID,
		&FIELD_SERVICE,
		&FIELD_PACKAGE,
		&FIELD_SOURCE,
		&FIELD_CREATED_ON,
		&FIELD_MODIFIED_ON,
		};

	for (int i = 0; i < SIZEOF_STATIC_ARRAY(DisallowedFields); i++)
		if (HasElement(dPatch, *DisallowedFields[i]))
			{
			if (retsError) *retsError = strPattern(ERR_INVALID_ROUTE_UPDATE_FIELD, *DisallowedFields[i]);
			return false;
			}

	CDatum dHost(CDatum::typeStruct);
	dHost.SetElement(FIELD_HOSTNAME, dOriginal.GetElement(FIELD_HOSTNAME));
	dHost.SetElement(FIELD_PROTOCOL, dOriginal.GetElement(FIELD_PROTOCOL));
	dHost.SetElement(FIELD_PORT, dOriginal.GetElement(FIELD_PORT));
	dHost.SetElement(FIELD_UNENCRYPTED_PORT, dOriginal.GetElement(FIELD_UNENCRYPTED_PORT));
	dHost.SetElement(FIELD_URL_PATH, dOriginal.GetElement(FIELD_URL_PATH));
	dHost.SetElement(FIELD_ACTION, dOriginal.GetElement(FIELD_ACTION));
	dHost.SetElement(FIELD_TARGET, dOriginal.GetElement(FIELD_TARGET));
	dHost.SetElement(FIELD_OPTIONS, dOriginal.GetElement(FIELD_OPTIONS));
	dHost.SetElement(FIELD_STATUS, dOriginal.GetElement(FIELD_STATUS));
	dHost.SetElement(FIELD_PRIORITY, dOriginal.GetElement(FIELD_PRIORITY));

	SetRoutePatchElement(dPatch, FIELD_HOSTNAME, &dHost);
	SetRoutePatchElement(dPatch, FIELD_PROTOCOL, &dHost);
	SetRoutePatchElement(dPatch, FIELD_PORT, &dHost);
	SetRoutePatchElement(dPatch, FIELD_UNENCRYPTED_PORT, &dHost);
	SetRoutePatchElement(dPatch, FIELD_URL_PATH, &dHost);
	SetRoutePatchElement(dPatch, FIELD_ACTION, &dHost);
	SetRoutePatchElement(dPatch, FIELD_TARGET, &dHost);
	SetRoutePatchElement(dPatch, FIELD_OPTIONS, &dHost);
	SetRoutePatchElement(dPatch, FIELD_STATUS, &dHost);
	SetRoutePatchElement(dPatch, FIELD_PRIORITY, &dHost);

	*retdHost = dHost;
	return true;
	}

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_AEON_ON_START,					"Aeon.onStart")
DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")
DECLARE_CONST_STRING(MSG_ARC_GET_STATUS,				"Arc.getStatus")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ON_ADMIN_NEEDED,	"Cryptosaur.onAdminNeeded")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ON_START,			"Cryptosaur.onStart")
DECLARE_CONST_STRING(MSG_ESPER_ON_CONNECT,				"Esper.onConnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_DISCONNECT,			"Esper.onDisconnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_LISTENER_STARTED,		"Esper.onListenerStarted")
DECLARE_CONST_STRING(MSG_ESPER_ON_LISTENER_STOPPED,		"Esper.onListenerStopped")
DECLARE_CONST_STRING(MSG_HYPERION_FILE_DOWNLOAD,		"Hyperion.fileDownload")
DECLARE_CONST_STRING(MSG_HYPERION_BIND_HOST,			"Hyperion.bindHost")
DECLARE_CONST_STRING(MSG_HYPERION_GET_OPTIONS,			"Hyperion.getOptions")
DECLARE_CONST_STRING(MSG_HYPERION_GET_PACKAGE_LIST,		"Hyperion.getPackageList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_ROUTE_LIST,		"Hyperion.getRouteList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_SERVICE_LIST,		"Hyperion.getServiceList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_SESSION_LIST,		"Hyperion.getSessionList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_TASK_LIST,		"Hyperion.getTaskList")
DECLARE_CONST_STRING(MSG_HYPERION_REFRESH,				"Hyperion.refresh")
DECLARE_CONST_STRING(MSG_HYPERION_RESIZE_IMAGE,			"Hyperion.resizeImage")
DECLARE_CONST_STRING(MSG_HYPERION_RUN_TASK,				"Hyperion.runTask")
DECLARE_CONST_STRING(MSG_HYPERION_SERVICE_MSG,			"Hyperion.serviceMsg")
DECLARE_CONST_STRING(MSG_HYPERION_SERVICE_MSG_SANDBOXED,"Hyperion.serviceMsgSandboxed")
DECLARE_CONST_STRING(MSG_HYPERION_SET_OPTION,			"Hyperion.setOption")
DECLARE_CONST_STRING(MSG_HYPERION_SET_TASK_RUN_ON,		"Hyperion.setTaskRunOn")
DECLARE_CONST_STRING(MSG_HYPERION_STOP_TASK,			"Hyperion.stopTask")
DECLARE_CONST_STRING(MSG_HYPERION_UNBIND_HOST,			"Hyperion.unbindHost")

CHyperionEngine::SMessageHandler CHyperionEngine::m_MsgHandlerList[] =
	{
		{	MSG_AEON_ON_START,					&CHyperionEngine::MsgAeonOnStart },

		{	MSG_ARC_GET_STATUS,					&CHyperionEngine::MsgGetStatus },
		{	MSG_ARC_HOUSEKEEPING,				&CHyperionEngine::MsgHousekeeping },

		{	MSG_CRYPTOSAUR_ON_ADMIN_NEEDED,		&CHyperionEngine::MsgCryptosaurOnAdminNeeded },
		{	MSG_CRYPTOSAUR_ON_START,			&CHyperionEngine::MsgCryptosaurOnStart },
		{	MSG_ESPER_ON_CONNECT,				&CHyperionEngine::MsgEsperOnConnect },
		{	MSG_ESPER_ON_LISTENER_STARTED,		&CHyperionEngine::MsgEsperOnListenerStarted },
		{	MSG_ESPER_ON_LISTENER_STOPPED,		&CHyperionEngine::MsgEsperOnListenerStopped },

		//	Hyperion.fileDownload
		{	MSG_HYPERION_FILE_DOWNLOAD,			&CHyperionEngine::MsgFileDownload },

		//	Hyperion.bindHost {hostname} {service}
		{	MSG_HYPERION_BIND_HOST,				&CHyperionEngine::MsgBindHost },

		//	Hyperion.bindHostRedirect {hostname} {target} [options]
		{	MSG_HYPERION_BIND_HOST_REDIRECT,	&CHyperionEngine::MsgBindHostRedirect },

		//	Hyperion.deleteRoute {routeID}
		{	MSG_HYPERION_DELETE_ROUTE,			&CHyperionEngine::MsgDeleteRoute },

		//	Hyperion.getOptions
		{	MSG_HYPERION_GET_OPTIONS,			&CHyperionEngine::MsgGetOptions },

		//	Hyperion.getPackageList
		{	MSG_HYPERION_GET_PACKAGE_LIST,		&CHyperionEngine::MsgGetPackageList },

		//	Hyperion.getRouteList
		{	MSG_HYPERION_GET_ROUTE_LIST,		&CHyperionEngine::MsgGetRouteList },

		//	Hyperion.getServiceList
		{	MSG_HYPERION_GET_SERVICE_LIST,		&CHyperionEngine::MsgGetServiceList },

		//	Hyperion.getSessionList
		{	MSG_HYPERION_GET_SESSION_LIST,		&CHyperionEngine::MsgGetSessionList },

		//	Hyperion.getTaskList
		{	MSG_HYPERION_GET_TASK_LIST,			&CHyperionEngine::MsgGetTaskList },

		//	Hyperion.refresh
		{	MSG_HYPERION_REFRESH,				&CHyperionEngine::MsgRefresh },

		//	Hyperion.resizeImage filePath newSize [options]
		{	MSG_HYPERION_RESIZE_IMAGE,			&CHyperionEngine::MsgResizeImage },

		//	Hyperion.runTask {taskName}
		{	MSG_HYPERION_RUN_TASK,				&CHyperionEngine::MsgRunTask },

		//	Hyperion.serviceMsg {service} {msg} {payload}
		{	MSG_HYPERION_SERVICE_MSG,			&CHyperionEngine::MsgServiceMsg },

		//	Hyperion.serviceMsgSandboxed {service} {msg} {payload}
		{	MSG_HYPERION_SERVICE_MSG_SANDBOXED,	&CHyperionEngine::MsgServiceMsgSandboxed },

		//	Hyperion.setRoute {routeID} {routePatch}
		{	MSG_HYPERION_SET_ROUTE,				&CHyperionEngine::MsgSetRoute },

		//	Hyperion.setOption {option} {value}
		{	MSG_HYPERION_SET_OPTION,			&CHyperionEngine::MsgSetOption },

		//	Hyperion.setTaskRunOn {taskName} [{dateTime}]
		{	MSG_HYPERION_SET_TASK_RUN_ON,		&CHyperionEngine::MsgSetTaskRunOn },

		//	Hyperion.stopTask {taskName}
		{	MSG_HYPERION_STOP_TASK,				&CHyperionEngine::MsgStopTask },

		//	Hyperion.unbindHost {hostname} {service}
		{	MSG_HYPERION_UNBIND_HOST,			&CHyperionEngine::MsgUnbindHost },
	};

int CHyperionEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CHyperionEngine::m_MsgHandlerList);

CHyperionEngine::CHyperionEngine (void) : TSimpleEngine(ENGINE_NAME_HYPERION, THREAD_COUNT),
		m_bUseHTTPRoutes(false),
		m_bAdminNeeded(false)

//	CHyperionEngine constructor

	{
	}

void CHyperionEngine::ComposeHTTPHostSeeds (TArray<CDatum> *retHosts)

//	ComposeHTTPHostSeeds
//
//	Composes initial Arc.hosts rows from legacy service-document hosts.

	{
	CSmartLock Lock(m_cs);
	int i, j, k, l;

	retHosts->DeleteAll();

	TArray<CHyperionPackageList::SServiceInfo> Services;
	m_Packages.GetServices(&Services);

	CDateTime Now(CDateTime::Now);

	for (i = 0; i < Services.GetCount(); i++)
		{
		CHTTPService *pHTTPService = CHTTPService::AsHTTPService(Services[i].pService);
		if (pHTTPService == NULL)
			continue;

		TArray<CString> Hosts;
		pHTTPService->GetHostsToServe(&Hosts);
		if (Hosts.GetCount() == 0)
			Hosts.Insert(STR_STAR);

		TArray<CString> Paths;
		pHTTPService->GetPathsToServe(&Paths);

		TArray<IHyperionService::SListenerDesc> Listeners;
		pHTTPService->GetListeners(Listeners);

		CString sProtocol;
		CString sPort;
		CString sUnencryptedPort;
		for (l = 0; l < Listeners.GetCount(); l++)
			{
			CString sListenerProtocol = Listeners[l].sProtocol.IsEmpty() ? PROTOCOL_HTTP : strToLower(Listeners[l].sProtocol);
			if (strEquals(sListenerProtocol, PROTOCOL_TLS))
				{
				sProtocol = PROTOCOL_TLS;
				sPort = Listeners[l].sPort;
				}
			else if (strEquals(sListenerProtocol, PROTOCOL_HTTP))
				sUnencryptedPort = Listeners[l].sPort;
			}

		if (sProtocol.IsEmpty())
			{
			sProtocol = PROTOCOL_HTTP;
			sPort = sUnencryptedPort;
			sUnencryptedPort = NULL_STR;
			}

		if (sPort.IsEmpty())
			continue;

		for (j = 0; j < Hosts.GetCount(); j++)
			{
			for (k = 0; k < Paths.GetCount(); k++)
				{
				CString sURLPath = MakeRoutePathCanonical(Paths[k]);
				if (sURLPath.IsEmpty())
					continue;

				CDatum dHost(CDatum::typeStruct);
				dHost.SetElement(FIELD_HOSTNAME, NormalizeRouteHost(Hosts[j]));
				dHost.SetElement(FIELD_PROTOCOL, sProtocol);
				dHost.SetElement(FIELD_PORT, sPort);
				if (!sUnencryptedPort.IsEmpty())
					dHost.SetElement(FIELD_UNENCRYPTED_PORT, sUnencryptedPort);
				dHost.SetElement(FIELD_URL_PATH, sURLPath);
				dHost.SetElement(FIELD_ACTION, ACTION_SERVICE);
				dHost.SetElement(FIELD_TARGET, Services[i].sName);
				dHost.SetElement(FIELD_PACKAGE, pHTTPService->GetPackageName());
				dHost.SetElement(FIELD_SERVICE, Services[i].sName);
				dHost.SetElement(FIELD_STATUS, STATUS_ACTIVE);
				dHost.SetElement(FIELD_SOURCE, SOURCE_SERVICE_DOC);
				dHost.SetElement(FIELD_CREATED_ON, Now);
				dHost.SetElement(FIELD_MODIFIED_ON, Now);

				retHosts->Insert(dHost);
				}
			}
	}
	}

CHyperionEngine::~CHyperionEngine (void)

//	CHyperionEngine destructor

	{
	}

void CHyperionEngine::FatalError (const SArchonMessage &Msg)

//	FatalError
//
//	Received an error from one of our dependencies (e.g., Aeon)

	{
	Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CONTINUE, Msg.dPayload.AsString()));
	}

bool CHyperionEngine::FindAI1Service (const CString &sListener, const CString &sInterface, CAI1Service **retpService)

//	FindAI1Service
//
//	Find the service that will handle the given AI1 interface

	{
	CSmartLock Lock(m_cs);
	int i;

	int iIndex;
	if (!FindListener(sListener, &iIndex))
		return false;

	//	Loop over all services for this listener

	for (i = 0; i < m_Listeners[iIndex].Services.GetCount(); i++)
		{
		CAI1Service *pAI1Service = CAI1Service::AsAI1Service(m_Listeners[iIndex].Services[i]);
		if (pAI1Service == NULL)
			continue;

		if (strEquals(sInterface, pAI1Service->GetInterface()))
			{
			if (retpService)
				*retpService = pAI1Service;
			return true;
			}
		}

	//	Not found

	return false;
	}

bool CHyperionEngine::FindHexarcMsgService (const CString &sService, IHyperionService **retpService)

//	FindHexarcMsgService
//
//	Find the service that will handle the given service message

	{
	CSmartLock Lock(m_cs);

	int iIndex;
	if (!FindHandler(sService, &iIndex))
		return false;

	//	Active?

	if (m_MsgHandlers[iIndex].pService == NULL)
		return false;

	//	Found

	if (retpService)
		*retpService = m_MsgHandlers[iIndex].pService;

	return true;
	}

bool CHyperionEngine::FindHTTPService (const CString &sListener, const CHTTPMessage &Request, CHTTPService **retpService)

//	FindHTTPService
//
//	Find the service that will handle the given request

	{
	SHTTPRouteMatch Match;
	if (!FindHTTPRoute(sListener, Request, &Match) || Match.pService == NULL)
		return false;

	if (retpService)
		*retpService = Match.pService;

	return true;
	}

bool CHyperionEngine::FindHTTPRoute (const CString &sListener, const CHTTPMessage &Request, SHTTPRouteMatch *retRoute)

//	FindHTTPRoute
//
//	Find the route action that will handle the given request.

	{
	CSmartLock Lock(m_cs);
	int i;

	int iIndex;
	if (!FindListener(sListener, &iIndex))
		return false;

	//	Match on the host and url

	CString sHostToMatch = NormalizeRouteHost(Request.GetRequestedHost());
	CString sURLToMatch = Request.GetRequestedPath();

	//	If Arc.hosts is authoritative, then match against our in-memory route
	//	table instead of service-document hosts.

	if (m_bUseHTTPRoutes)
		{
		bool bBestExactHost = false;
		bool bBestExactPath = false;
		int iBestURLMatch = 0;
		int iBestPriority = 0;
		CString sBestRouteID;
		const SHTTPRoute *pBestRoute = NULL;
		CHTTPService *pBestService = NULL;
		CString sListenerProtocol = m_Listeners[iIndex].sProtocol.IsEmpty() ? PROTOCOL_HTTP : m_Listeners[iIndex].sProtocol;

		for (i = 0; i < m_HTTPRoutes.GetCount(); i++)
			{
			const SHTTPRoute &Route = m_HTTPRoutes[i];
			if (!Route.bActive)
				continue;

			if (!RouteMatchesProtocolAndPort(Route.sProtocol, Route.sPort, Route.sUnencryptedPort, sListenerProtocol, m_Listeners[iIndex].sPort))
				continue;

			bool bExactHost = strEquals(sHostToMatch, Route.sHostName);
			if (!bExactHost && !strEquals(Route.sHostName, STR_STAR))
				continue;

			bool bExactPath = false;
			if (!RouteMatchesPath(Route.dOptions, Route.sURLPath, sURLToMatch, &bExactPath))
				continue;

			CHTTPService *pHTTPService = NULL;
			if (strEquals(Route.sAction, ACTION_SERVICE))
				{
				CString sServiceName = Route.sTarget.IsEmpty() ? Route.sServiceName : Route.sTarget;

				for (int j = 0; j < m_Listeners[iIndex].Services.GetCount(); j++)
					{
					IHyperionService *pService = m_Listeners[iIndex].Services[j];
					if (!strEquals(pService->GetName(), sServiceName))
						continue;

					pHTTPService = CHTTPService::AsHTTPService(pService);
					break;
					}

				if (pHTTPService == NULL)
					continue;
				}

			int iURLMatch = Route.sURLPath.GetLength();
			if (pBestRoute == NULL
					|| (bExactHost && !bBestExactHost)
					|| (bExactHost == bBestExactHost && bExactPath && !bBestExactPath)
					|| (bExactHost == bBestExactHost && bExactPath == bBestExactPath && iURLMatch > iBestURLMatch)
					|| (bExactHost == bBestExactHost && bExactPath == bBestExactPath && iURLMatch == iBestURLMatch && Route.iPriority > iBestPriority)
					|| (bExactHost == bBestExactHost && bExactPath == bBestExactPath && iURLMatch == iBestURLMatch && Route.iPriority == iBestPriority && KeyCompare(Route.sRouteID, sBestRouteID) == -1))
				{
				bBestExactHost = bExactHost;
				bBestExactPath = bExactPath;
				iBestURLMatch = iURLMatch;
				iBestPriority = Route.iPriority;
				sBestRouteID = Route.sRouteID;
				pBestRoute = &Route;
				pBestService = pHTTPService;
				}
			}

		if (pBestRoute == NULL)
			return false;

		if (retRoute)
			{
			retRoute->sAction = pBestRoute->sAction;
			retRoute->sTarget = pBestRoute->sTarget;
			retRoute->sURLPath = pBestRoute->sURLPath;
			retRoute->dOptions = pBestRoute->dOptions;
			retRoute->pService = pBestService;
			}

		return true;
		}

	//	Loop over all services for this listener

	int iBestMatch = 0;
	CHTTPService *pBestService = NULL;
	for (i = 0; i < m_Listeners[iIndex].Services.GetCount(); i++)
		{
		CHTTPService *pHTTPService = CHTTPService::AsHTTPService(m_Listeners[iIndex].Services[i]);
		if (pHTTPService == NULL)
			continue;

		int iMatch = pHTTPService->MatchHostAndURL(sHostToMatch, sURLToMatch);
		if (iMatch > iBestMatch)
			{
			iBestMatch = iMatch;
			pBestService = pHTTPService;
			}
		}

	//	Not found?

	if (pBestService == NULL)
		return false;

	//	Done

	if (retRoute)
		{
		retRoute->sAction = ACTION_SERVICE;
		retRoute->sTarget = pBestService->GetName();
		retRoute->sURLPath = NULL_STR;
		retRoute->dOptions = CDatum();
		retRoute->pService = pBestService;
		}

	return true;
	}

bool CHyperionEngine::FindHTTPRoute (const CString &sProtocol, const CString &sPort, const CHTTPMessage &Request, SHTTPRouteMatch *retRoute)

//	FindHTTPRoute
//
//	Finds a route based on a target protocol and port instead of the current
//	listener.

	{
	CSmartLock Lock(m_cs);
	int i;

	if (!m_bUseHTTPRoutes)
		return false;

	CString sProtocolToMatch = strToLower(sProtocol.IsEmpty() ? PROTOCOL_HTTP : sProtocol);
	CString sHostToMatch = NormalizeRouteHost(Request.GetRequestedHost());
	CString sURLToMatch = Request.GetRequestedPath();

	bool bBestExactHost = false;
	bool bBestExactPath = false;
	int iBestURLMatch = 0;
	int iBestPriority = 0;
	CString sBestRouteID;
	const SHTTPRoute *pBestRoute = NULL;
	CHTTPService *pBestService = NULL;

	for (i = 0; i < m_HTTPRoutes.GetCount(); i++)
		{
		const SHTTPRoute &Route = m_HTTPRoutes[i];
		if (!Route.bActive)
			continue;

		if (!RouteMatchesProtocolAndPort(Route.sProtocol, Route.sPort, Route.sUnencryptedPort, sProtocolToMatch, sPort))
			continue;

		bool bExactHost = strEquals(sHostToMatch, Route.sHostName);
		if (!bExactHost && !strEquals(Route.sHostName, STR_STAR))
			continue;

		bool bExactPath = false;
		if (!RouteMatchesPath(Route.dOptions, Route.sURLPath, sURLToMatch, &bExactPath))
			continue;

		CHTTPService *pHTTPService = NULL;
		if (strEquals(Route.sAction, ACTION_SERVICE))
			{
			CString sServiceName = Route.sTarget.IsEmpty() ? Route.sServiceName : Route.sTarget;

			for (int j = 0; j < m_Listeners.GetCount(); j++)
				{
				CString sListenerProtocol = m_Listeners[j].sProtocol.IsEmpty() ? PROTOCOL_HTTP : m_Listeners[j].sProtocol;
				if (!RouteMatchesProtocolAndPort(Route.sProtocol, Route.sPort, Route.sUnencryptedPort, sListenerProtocol, m_Listeners[j].sPort))
					continue;

				for (int k = 0; k < m_Listeners[j].Services.GetCount(); k++)
					{
					IHyperionService *pService = m_Listeners[j].Services[k];
					if (!strEquals(pService->GetName(), sServiceName))
						continue;

					pHTTPService = CHTTPService::AsHTTPService(pService);
					break;
					}

				if (pHTTPService != NULL)
					break;
				}

			if (pHTTPService == NULL)
				continue;
			}

		int iURLMatch = Route.sURLPath.GetLength();
		if (pBestRoute == NULL
				|| (bExactHost && !bBestExactHost)
				|| (bExactHost == bBestExactHost && bExactPath && !bBestExactPath)
				|| (bExactHost == bBestExactHost && bExactPath == bBestExactPath && iURLMatch > iBestURLMatch)
				|| (bExactHost == bBestExactHost && bExactPath == bBestExactPath && iURLMatch == iBestURLMatch && Route.iPriority > iBestPriority)
				|| (bExactHost == bBestExactHost && bExactPath == bBestExactPath && iURLMatch == iBestURLMatch && Route.iPriority == iBestPriority && KeyCompare(Route.sRouteID, sBestRouteID) == -1))
			{
			bBestExactHost = bExactHost;
			bBestExactPath = bExactPath;
			iBestURLMatch = iURLMatch;
			iBestPriority = Route.iPriority;
			sBestRouteID = Route.sRouteID;
			pBestRoute = &Route;
			pBestService = pHTTPService;
			}
		}

	if (pBestRoute == NULL)
		return false;

	if (retRoute)
		{
		retRoute->sAction = pBestRoute->sAction;
		retRoute->sTarget = pBestRoute->sTarget;
		retRoute->sURLPath = pBestRoute->sURLPath;
		retRoute->dOptions = pBestRoute->dOptions;
		retRoute->pService = pBestService;
		}

	return true;
	}

bool CHyperionEngine::FindHTTPHostByRouteID (CDatum dHostList, const CString &sRouteID, CDatum *retdHost, CString *retsID)

//	FindHTTPHostByRouteID
//
//	Finds the durable Arc.hosts row by public routeID.

	{
	for (int i = 0; i < dHostList.GetCount(); i++)
		{
		CDatum dHost = dHostList.GetElement(i);
		CString sID = GetHostBindingID(dHost);
		if (strEquals(sID, sRouteID))
			{
			if (retdHost)
				*retdHost = dHost;

			if (retsID)
				*retsID = sID;

			return true;
			}
		}

	return false;
	}

CDatum CHyperionEngine::ComposeHTTPRouteTable (CDatum dHostList)

//	ComposeHTTPRouteTable
//
//	Composes the public ArcRouteSchema view from Arc.hosts rows.

	{
	CSmartLock Lock(m_cs);

	CDatum dResult = CArcRouteUtil::CreateRouteTable();

	for (int i = 0; i < dHostList.GetCount(); i++)
		{
		CDatum dHost = dHostList.GetElement(i);

		CString sHostID = GetHostBindingID(dHost);
		CString sAction = GetHostBindingAction(dHost);
		CString sTarget = dHost.GetElement(FIELD_TARGET).AsString();
		if (sTarget.IsEmpty() && strEquals(sAction, ACTION_SERVICE))
			sTarget = GetHostBindingServiceName(dHost);

		CString sHostName = NormalizeRouteHost(GetHostBindingHostName(dHost));
		if (sHostID.IsEmpty() || sAction.IsEmpty() || sTarget.IsEmpty() || sHostName.IsEmpty())
			continue;

		CString sProtocol = strToLower(dHost.GetElement(FIELD_PROTOCOL).AsString());
		CString sPort = dHost.GetElement(FIELD_PORT).AsString();
		CString sUnencryptedPort = dHost.GetElement(FIELD_UNENCRYPTED_PORT).AsString();
		CString sURLPath = MakeRoutePathCanonical(dHost.GetElement(FIELD_URL_PATH).AsString());

		if (strEquals(sAction, ACTION_SERVICE))
			{
			CString sServiceName = sTarget;
			IHyperionService *pService = NULL;
			if (!m_Packages.FindServiceByName(sServiceName, &pService) || CHTTPService::AsHTTPService(pService) == NULL)
				continue;

			if (sProtocol.IsEmpty() || sPort.IsEmpty() || sURLPath.IsEmpty())
				continue;

			CDatum dRoute(CDatum::typeStruct);
			dRoute.SetElement(FIELD_ROUTE_ID, sHostID);
			dRoute.SetElement(FIELD_ACTION, ACTION_SERVICE);
			dRoute.SetElement(FIELD_TARGET, sServiceName);
			dRoute.SetElement(FIELD_OPTIONS, dHost.GetElement(FIELD_OPTIONS));
			dRoute.SetElement(FIELD_PACKAGE, pService->GetPackageName());
			dRoute.SetElement(FIELD_SERVICE, sServiceName);
			dRoute.SetElement(FIELD_HOSTNAME, sHostName);
			dRoute.SetElement(FIELD_PROTOCOL, sProtocol);
			dRoute.SetElement(FIELD_PORT, sPort);
			dRoute.SetElement(FIELD_UNENCRYPTED_PORT, sUnencryptedPort);
			dRoute.SetElement(FIELD_URL_PATH, sURLPath);
			dRoute.SetElement(FIELD_PRIORITY, dHost.GetElement(FIELD_PRIORITY).IsNil() ? 0 : (int)dHost.GetElement(FIELD_PRIORITY));
			dRoute.SetElement(FIELD_STATUS, dHost.GetElement(FIELD_STATUS).AsString());
			dRoute.SetElement(FIELD_SOURCE, dHost.GetElement(FIELD_SOURCE));
			dRoute.SetElement(FIELD_CREATED_ON, dHost.GetElement(FIELD_CREATED_ON));
			dRoute.SetElement(FIELD_MODIFIED_ON, dHost.GetElement(FIELD_MODIFIED_ON));

			dResult.Append(dRoute);
			}
		else
			{
			if (sProtocol.IsEmpty() || sPort.IsEmpty() || sURLPath.IsEmpty())
				continue;

			CDatum dRoute(CDatum::typeStruct);
			dRoute.SetElement(FIELD_ROUTE_ID, sHostID);
			dRoute.SetElement(FIELD_ACTION, sAction);
			dRoute.SetElement(FIELD_TARGET, sTarget);
			dRoute.SetElement(FIELD_OPTIONS, dHost.GetElement(FIELD_OPTIONS));
			dRoute.SetElement(FIELD_HOSTNAME, sHostName);
			dRoute.SetElement(FIELD_PROTOCOL, sProtocol);
			dRoute.SetElement(FIELD_PORT, sPort);
			dRoute.SetElement(FIELD_UNENCRYPTED_PORT, sUnencryptedPort);
			dRoute.SetElement(FIELD_URL_PATH, sURLPath);
			dRoute.SetElement(FIELD_PRIORITY, dHost.GetElement(FIELD_PRIORITY).IsNil() ? 0 : (int)dHost.GetElement(FIELD_PRIORITY));
			dRoute.SetElement(FIELD_STATUS, dHost.GetElement(FIELD_STATUS).AsString());
			dRoute.SetElement(FIELD_SOURCE, dHost.GetElement(FIELD_SOURCE));
			dRoute.SetElement(FIELD_CREATED_ON, dHost.GetElement(FIELD_CREATED_ON));
			dRoute.SetElement(FIELD_MODIFIED_ON, dHost.GetElement(FIELD_MODIFIED_ON));

			dResult.Append(dRoute);
			}
		}

	return dResult;
	}

void CHyperionEngine::SetHTTPHosts (CDatum dHostList)

//	SetHTTPHosts
//
//	Sets the in-memory HTTP route list from Arc.hosts rows.

	{
	CDatum dRouteList = ComposeHTTPRouteTable(dHostList);

	CSmartLock Lock(m_cs);
	int i;

	m_bUseHTTPRoutes = true;
	m_HTTPRoutes.DeleteAll();

	for (i = 0; i < dRouteList.GetCount(); i++)
		{
		CDatum dRoute = dRouteList.GetElement(i);

		SHTTPRoute *pRoute = m_HTTPRoutes.Insert();
		pRoute->sRouteID = dRoute.GetElement(FIELD_ROUTE_ID).AsString();
		pRoute->sAction = dRoute.GetElement(FIELD_ACTION).AsString();
		pRoute->sTarget = dRoute.GetElement(FIELD_TARGET).AsString();
		pRoute->dOptions = dRoute.GetElement(FIELD_OPTIONS);
		pRoute->sPackageName = dRoute.GetElement(FIELD_PACKAGE).AsString();
		pRoute->sServiceName = dRoute.GetElement(FIELD_SERVICE).AsString();
		pRoute->sProtocol = strToLower(dRoute.GetElement(FIELD_PROTOCOL).AsString());
		pRoute->sPort = dRoute.GetElement(FIELD_PORT).AsString();
		pRoute->sUnencryptedPort = dRoute.GetElement(FIELD_UNENCRYPTED_PORT).AsString();
		pRoute->sHostName = NormalizeRouteHost(dRoute.GetElement(FIELD_HOSTNAME).AsString());
		pRoute->sURLPath = MakeRoutePathCanonical(dRoute.GetElement(FIELD_URL_PATH).AsString());
		pRoute->iPriority = (int)dRoute.GetElement(FIELD_PRIORITY);

		CString sStatus = dRoute.GetElement(FIELD_STATUS).AsString();
		pRoute->bActive = (sStatus.IsEmpty() || strEqualsNoCase(sStatus, STATUS_ACTIVE));
		}
	}

bool CHyperionEngine::NormalizeHTTPHostDesc (CDatum dHost, CDatum dOriginal, bool bRequireID, CDatum *retdHost, CString *retsError)

//	NormalizeHTTPHostDesc
//
//	Validates and normalizes an Arc.hosts row.

	{
	if (!dHost.IsStruct())
		{
		if (retsError) *retsError = ERR_INVALID_PARAMS;
		return false;
		}

	CString sID = GetHostBindingID(dHost);
	if (sID.IsEmpty() && !dOriginal.IsNil())
		sID = GetHostBindingID(dOriginal);

	if (bRequireID && sID.IsEmpty())
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_ID);
		return false;
		}

	CString sAction = strClean(GetHostBindingAction(dHost));
	if (sAction.IsEmpty())
		sAction = ACTION_SERVICE;
	else if (!strEquals(sAction, ACTION_SERVICE) && !strEquals(sAction, ACTION_INTERNAL_REWRITE) && !strEquals(sAction, ACTION_REDIRECT))
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_ACTION);
		return false;
		}

	CString sTarget = strClean(dHost.GetElement(FIELD_TARGET).AsString());
	if (sTarget.IsEmpty())
		sTarget = strClean(GetHostBindingServiceName(dHost));

	if (sTarget.IsEmpty())
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_TARGET);
		return false;
		}

	IHyperionService *pService = NULL;
	CHTTPService *pHTTPService = NULL;
	if (strEquals(sAction, ACTION_SERVICE) && (!m_Packages.FindServiceByName(sTarget, &pService) || (pHTTPService = CHTTPService::AsHTTPService(pService)) == NULL))
		{
		if (retsError) *retsError = strPattern(ERR_UNKNOWN_SERVICE, sTarget);
		return false;
		}

	CString sPackageName = strClean(dHost.GetElement(FIELD_PACKAGE).AsString());
	if (strEquals(sAction, ACTION_SERVICE) && sPackageName.IsEmpty())
		sPackageName = pService->GetPackageName();
	else if (strEquals(sAction, ACTION_SERVICE) && !strEquals(sPackageName, pService->GetPackageName()))
		{
		if (retsError) *retsError = strPattern(ERR_HOST_PACKAGE_MISMATCH, sPackageName, pService->GetPackageName());
		return false;
		}

	CString sHostName = NormalizeRouteHost(strClean(GetHostBindingHostName(dHost)));
	if (sHostName.IsEmpty())
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_HOSTNAME);
		return false;
		}

	CString sProtocol = strToLower(strClean(dHost.GetElement(FIELD_PROTOCOL).AsString()));
	CString sPort = strClean(dHost.GetElement(FIELD_PORT).AsString());
	CString sUnencryptedPort = strClean(dHost.GetElement(FIELD_UNENCRYPTED_PORT).AsString());
	CString sURLPath = MakeRoutePathCanonical(strClean(dHost.GetElement(FIELD_URL_PATH).AsString()));
	if (strEquals(sAction, ACTION_SERVICE))
		{
		if (sURLPath.IsEmpty())
			{
			TArray<CString> Paths;
			pHTTPService->GetPathsToServe(&Paths);

			if (Paths.GetCount() > 0)
				sURLPath = MakeRoutePathCanonical(Paths[0]);
			}

		if (sProtocol.IsEmpty() || sPort.IsEmpty() || (strEquals(sProtocol, PROTOCOL_TLS) && sUnencryptedPort.IsEmpty()))
			{
			TArray<IHyperionService::SListenerDesc> Listeners;
			pHTTPService->GetListeners(Listeners);

			CString sTLSPort;
			CString sHTTPPort;
			for (int i = 0; i < Listeners.GetCount(); i++)
				{
				CString sListenerProtocol = Listeners[i].sProtocol.IsEmpty() ? PROTOCOL_HTTP : strToLower(Listeners[i].sProtocol);
				if (strEquals(sListenerProtocol, PROTOCOL_TLS))
					sTLSPort = Listeners[i].sPort;
				else if (strEquals(sListenerProtocol, PROTOCOL_HTTP))
					sHTTPPort = Listeners[i].sPort;
				}

			if (sProtocol.IsEmpty())
				sProtocol = (!sTLSPort.IsEmpty() ? PROTOCOL_TLS : PROTOCOL_HTTP);

			if (sPort.IsEmpty())
				sPort = (strEquals(sProtocol, PROTOCOL_TLS) ? sTLSPort : sHTTPPort);

			if (sUnencryptedPort.IsEmpty() && strEquals(sProtocol, PROTOCOL_TLS))
				sUnencryptedPort = sHTTPPort;
			}
		}
	else
		{
		if (sProtocol.IsEmpty())
			sProtocol = PROTOCOL_TLS;

		if (sPort.IsEmpty())
			sPort = (strEquals(sProtocol, PROTOCOL_TLS) ? PORT_DEFAULT_TLS : PORT_DEFAULT_HTTP);
		if (sURLPath.IsEmpty())
			sURLPath = STR_CHYPERION_ENGINE_SLASH;
		}

	if (sPort.IsEmpty())
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_PORT);
		return false;
		}

	if (sURLPath.IsEmpty())
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_URL_PATH);
		return false;
		}

	if (!sProtocol.IsEmpty() && !strEquals(sProtocol, PROTOCOL_HTTP) && !strEquals(sProtocol, PROTOCOL_TLS))
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_PROTOCOL);
		return false;
		}

	if (!sUnencryptedPort.IsEmpty() && !strEquals(sProtocol, PROTOCOL_TLS))
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_UNENCRYPTED_PORT);
		return false;
		}

	CString sStatus = strToLower(strClean(dHost.GetElement(FIELD_STATUS).AsString()));
	if (sStatus.IsEmpty())
		sStatus = STATUS_ACTIVE;
	else if (!strEquals(sStatus, STATUS_ACTIVE) && !strEquals(sStatus, STATUS_DISABLED))
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_HOST_BINDING, FIELD_STATUS);
		return false;
		}

	CDatum dPriority = dHost.GetElement(FIELD_PRIORITY);
	if (dPriority.IsNil() && !dOriginal.IsNil())
		dPriority = dOriginal.GetElement(FIELD_PRIORITY);

	CDateTime Now(CDateTime::Now);
	CDatum dCreatedOn = dHost.GetElement(FIELD_CREATED_ON);
	if (dCreatedOn.IsNil() && !dOriginal.IsNil())
		dCreatedOn = dOriginal.GetElement(FIELD_CREATED_ON);
	if (dCreatedOn.IsNil())
		dCreatedOn = Now;

	CDatum dSource = dHost.GetElement(FIELD_SOURCE);
	if (dSource.IsNil() && !dOriginal.IsNil())
		dSource = dOriginal.GetElement(FIELD_SOURCE);
	if (dSource.IsNil())
		dSource = SOURCE_ADMIN;

	CComplexStruct *pHost = new CComplexStruct;
	if (!sID.IsEmpty())
		pHost->SetElement(FIELD_ID, sID);
	pHost->SetElement(FIELD_HOSTNAME, sHostName);
	if (!sProtocol.IsEmpty())
		pHost->SetElement(FIELD_PROTOCOL, sProtocol);
	if (!sPort.IsEmpty())
		pHost->SetElement(FIELD_PORT, sPort);
	if (!sUnencryptedPort.IsEmpty())
		pHost->SetElement(FIELD_UNENCRYPTED_PORT, sUnencryptedPort);
	if (!sURLPath.IsEmpty())
		pHost->SetElement(FIELD_URL_PATH, sURLPath);
	pHost->SetElement(FIELD_ACTION, sAction);
	pHost->SetElement(FIELD_TARGET, sTarget);
	pHost->SetElement(FIELD_OPTIONS, dHost.GetElement(FIELD_OPTIONS));
	if (!dPriority.IsNil())
		pHost->SetElement(FIELD_PRIORITY, (int)dPriority);
	if (strEquals(sAction, ACTION_SERVICE))
		{
		pHost->SetElement(FIELD_SERVICE, sTarget);
		pHost->SetElement(FIELD_PACKAGE, sPackageName);
		}
	pHost->SetElement(FIELD_STATUS, sStatus);
	pHost->SetElement(FIELD_SOURCE, dSource);
	pHost->SetElement(FIELD_CREATED_ON, dCreatedOn);
	pHost->SetElement(FIELD_MODIFIED_ON, Now);

	*retdHost = CDatum(pHost);
	return true;
	}

void CHyperionEngine::LogSessionState (const CString &sLine)

//	LogSessionState
//
//	Logs session state, if enabled.

	{
	if (m_Options.GetOptionBoolean(CHyperionOptions::optionLogSessionState))
		GetProcessCtx()->Log(MSG_LOG_DEBUG, sLine);
	}

void CHyperionEngine::MsgGetOptions (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetOptions
//
//	Hyperion.getOptions

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Return list of options and current settings

	SendMessageReply(MSG_REPLY_DATA, m_Options.GetStatus(), Msg);
	}

void CHyperionEngine::MsgGetPackageList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetPackageList
//
//	Hyperion.getPackageList

	{
	int i;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	TArray<CHyperionPackageList::SPackageInfo> List;
	m_Packages.GetPackageList(&List);
	if (List.GetCount() == 0)
		SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);

	CComplexArray *pReply = new CComplexArray;
	for (i = 0; i < List.GetCount(); i++)
		{
		CComplexStruct *pInfo = new CComplexStruct;
		pInfo->SetElement(FIELD_NAME, List[i].sName);
		pInfo->SetElement(FIELD_FILE_PATH, List[i].sFilePath);
		pInfo->SetElement(FIELD_VERSION, List[i].sVersion);
		pInfo->SetElement(FIELD_MODIFIED_BY, List[i].sModifiedBy);
		pInfo->SetElement(FIELD_MODIFIED_ON, List[i].ModifiedOn);

		pReply->Append(CDatum(pInfo));
		}

	SendMessageReply(MSG_REPLY_DATA, CDatum(pReply), Msg);
	}

void CHyperionEngine::MsgGetRouteList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetRouteList
//
//	Hyperion.getRouteList

	{
	struct SCtx { void Mark () { } };

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	auto pSession = TPromiseSession<SCtx>::Make();
	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			return ReturnEndSession(MSG_REPLY_DATA, ComposeHTTPRouteTable(Msg.dPayload), retReply);
			});

	StartSession(Msg, std::move(pSession));
	}

void CHyperionEngine::MsgBindHost (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgBindHost
//
//	Hyperion.bindHost hostname service

	{
	struct SCtx
		{
		void Mark () { dHost.Mark(); }
		CDatum dHost;
		};

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	SCtx Ctx;
	CDatum dHost(CDatum::typeStruct);
	dHost.SetElement(FIELD_HOSTNAME, Msg.dPayload.GetElement(0));
	dHost.SetElement(FIELD_SERVICE, Msg.dPayload.GetElement(1));

	CString sError;
	if (!NormalizeHTTPHostDesc(dHost, CDatum(), false, &Ctx.dHost, &sError))
		{
		SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	auto pSession = TPromiseSession<SCtx>::Make(std::move(Ctx));
	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			CString sHostName = Ctx.dHost.GetElement(FIELD_HOSTNAME).AsString();
			CString sServiceName = Ctx.dHost.GetElement(FIELD_SERVICE).AsString();
			for (int i = 0; i < Msg.dPayload.GetCount(); i++)
				{
				CDatum dRow = Msg.dPayload.GetElement(i);
				if (strEquals(NormalizeRouteHost(GetHostBindingHostName(dRow)), sHostName)
						&& strEquals(GetHostBindingServiceName(dRow), sServiceName))
					{
					SetHTTPHosts(Msg.dPayload);
					return ReturnEndSession(MSG_REPLY_DATA, true, retReply);
					}
				}

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dMutate(CDatum::typeStruct);
			dMutate.SetElement(FIELD_PRIMARY_KEY, MUTATE_CODE8);
			dMutate.SetElement(FIELD_ID, MUTATE_PRIMARY_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(Ctx.dHost);
			dPayload.Append(dMutate);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_MUTATE, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_HOST_BINDING_RELOAD_FAILED, Msg.dPayload.AsString()), retReply);

			SetHTTPHosts(Msg.dPayload);
			return ReturnEndSession(MSG_REPLY_DATA, true, retReply);
			});

	StartSession(Msg, std::move(pSession));
	}

void CHyperionEngine::MsgBindHostRedirect (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgBindHostRedirect
//
//	Hyperion.bindHostRedirect hostname target [options]

	{
	struct SCtx
		{
		void Mark () { dHost.Mark(); }
		CDatum dHost;
		};

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	CDatum dOptions = Msg.dPayload.GetElement(2);
	if (!dOptions.IsNil() && !dOptions.IsStruct())
		{
		SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS, Msg);
		return;
		}

	CString sAction = dOptions.GetElement(FIELD_ACTION).AsString();
	if (sAction.IsEmpty())
		sAction = ACTION_INTERNAL_REWRITE;

	SCtx Ctx;
	CDatum dHost(CDatum::typeStruct);
	dHost.SetElement(FIELD_HOSTNAME, Msg.dPayload.GetElement(0));
	dHost.SetElement(FIELD_TARGET, Msg.dPayload.GetElement(1));
	dHost.SetElement(FIELD_ACTION, sAction);
	dHost.SetElement(FIELD_PROTOCOL, dOptions.GetElement(FIELD_PROTOCOL));
	dHost.SetElement(FIELD_PORT, dOptions.GetElement(FIELD_PORT));
	dHost.SetElement(FIELD_UNENCRYPTED_PORT, dOptions.GetElement(FIELD_UNENCRYPTED_PORT));
	dHost.SetElement(FIELD_URL_PATH, dOptions.GetElement(FIELD_URL_PATH));
	dHost.SetElement(FIELD_OPTIONS, dOptions);

	CString sError;
	if (!NormalizeHTTPHostDesc(dHost, CDatum(), false, &Ctx.dHost, &sError))
		{
		SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	auto pSession = TPromiseSession<SCtx>::Make(std::move(Ctx));
	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			CString sHostName = Ctx.dHost.GetElement(FIELD_HOSTNAME).AsString();
			CString sAction = Ctx.dHost.GetElement(FIELD_ACTION).AsString();
			CString sTarget = Ctx.dHost.GetElement(FIELD_TARGET).AsString();
			CString sProtocol = Ctx.dHost.GetElement(FIELD_PROTOCOL).AsString();
			CString sPort = Ctx.dHost.GetElement(FIELD_PORT).AsString();
			CString sUnencryptedPort = Ctx.dHost.GetElement(FIELD_UNENCRYPTED_PORT).AsString();
			CString sURLPath = Ctx.dHost.GetElement(FIELD_URL_PATH).AsString();
			for (int i = 0; i < Msg.dPayload.GetCount(); i++)
				{
				CDatum dRow = Msg.dPayload.GetElement(i);
				if (strEquals(NormalizeRouteHost(GetHostBindingHostName(dRow)), sHostName)
						&& strEquals(GetHostBindingAction(dRow), sAction)
						&& strEquals(dRow.GetElement(FIELD_TARGET).AsString(), sTarget)
						&& strEquals(strToLower(dRow.GetElement(FIELD_PROTOCOL).AsString()), sProtocol)
						&& strEquals(dRow.GetElement(FIELD_PORT).AsString(), sPort)
						&& strEquals(dRow.GetElement(FIELD_UNENCRYPTED_PORT).AsString(), sUnencryptedPort)
						&& strEquals(MakeRoutePathCanonical(dRow.GetElement(FIELD_URL_PATH).AsString()), sURLPath))
					{
					SetHTTPHosts(Msg.dPayload);
					return ReturnEndSession(MSG_REPLY_DATA, true, retReply);
					}
				}

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dMutate(CDatum::typeStruct);
			dMutate.SetElement(FIELD_PRIMARY_KEY, MUTATE_CODE8);
			dMutate.SetElement(FIELD_ID, MUTATE_PRIMARY_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(Ctx.dHost);
			dPayload.Append(dMutate);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_MUTATE, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_HOST_BINDING_RELOAD_FAILED, Msg.dPayload.AsString()), retReply);

			SetHTTPHosts(Msg.dPayload);
			return ReturnEndSession(MSG_REPLY_DATA, true, retReply);
			});

	StartSession(Msg, std::move(pSession));
	}

void CHyperionEngine::MsgDeleteRoute (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgDeleteRoute
//
//	Hyperion.deleteRoute routeID

	{
	struct SCtx
		{
		void Mark () { }
		CString sRouteID;
		CString sID;
		};

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	SCtx Ctx;
	Ctx.sRouteID = Msg.dPayload.GetElement(0).AsString();
	if (Ctx.sRouteID.IsEmpty())
		{
		SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS, Msg);
		return;
		}

	auto pSession = TPromiseSession<SCtx>::Make(std::move(Ctx));
	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			if (!FindHTTPHostByRouteID(Msg.dPayload, Ctx.sRouteID, NULL, &Ctx.sID) || Ctx.sID.IsEmpty())
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_ROUTE_NOT_FOUND, Ctx.sRouteID), retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(Ctx.sID);
			dPayload.Append(CDatum());

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_INSERT, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_HOST_BINDING_RELOAD_FAILED, Msg.dPayload.AsString()), retReply);

			SetHTTPHosts(Msg.dPayload);
			return ReturnEndSession(MSG_REPLY_DATA, true, retReply);
			});

	StartSession(Msg, std::move(pSession));
	}

void CHyperionEngine::MsgSetRoute (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSetRoute
//
//	Hyperion.setRoute routeID routePatch

	{
	struct SCtx
		{
		void Mark () { dPatch.Mark(); dHost.Mark(); }
		CString sRouteID;
		CString sID;
		CDatum dPatch;
		CDatum dHost;
		};

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	SCtx Ctx;
	Ctx.sRouteID = Msg.dPayload.GetElement(0).AsString();
	Ctx.dPatch = Msg.dPayload.GetElement(1);
	if (Ctx.sRouteID.IsEmpty() || !Ctx.dPatch.IsStruct())
		{
		SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS, Msg);
		return;
		}

	auto pSession = TPromiseSession<SCtx>::Make(std::move(Ctx));
	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			CDatum dOriginal;
			if (!FindHTTPHostByRouteID(Msg.dPayload, Ctx.sRouteID, &dOriginal, &Ctx.sID) || Ctx.sID.IsEmpty())
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_ROUTE_NOT_FOUND, Ctx.sRouteID), retReply);

			CDatum dPatched;
			CString sError;
			if (!ComposeRoutePatch(dOriginal, Ctx.dPatch, &dPatched, &sError)
					|| !NormalizeHTTPHostDesc(dPatched, dOriginal, true, &Ctx.dHost, &sError))
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, sError, retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(Ctx.sID);
			dPayload.Append(Ctx.dHost);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_INSERT, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_HOST_BINDING_RELOAD_FAILED, Msg.dPayload.AsString()), retReply);

			SetHTTPHosts(Msg.dPayload);
			return ReturnEndSession(MSG_REPLY_DATA, true, retReply);
			});

	StartSession(Msg, std::move(pSession));
	}

void CHyperionEngine::MsgUnbindHost (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgUnbindHost
//
//	Hyperion.unbindHost hostname service

	{
	struct SCtx
		{
		void Mark () { dHost.Mark(); }
		CDatum dHost;
		CString sID;
		};

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	SCtx Ctx;
	CDatum dHost(CDatum::typeStruct);
	dHost.SetElement(FIELD_HOSTNAME, Msg.dPayload.GetElement(0));
	dHost.SetElement(FIELD_SERVICE, Msg.dPayload.GetElement(1));

	CString sError;
	if (!NormalizeHTTPHostDesc(dHost, CDatum(), false, &Ctx.dHost, &sError))
		{
		SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	auto pSession = TPromiseSession<SCtx>::Make(std::move(Ctx));
	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			CString sHostName = Ctx.dHost.GetElement(FIELD_HOSTNAME).AsString();
			CString sServiceName = Ctx.dHost.GetElement(FIELD_SERVICE).AsString();
			for (int i = 0; i < Msg.dPayload.GetCount(); i++)
				{
				CDatum dRow = Msg.dPayload.GetElement(i);
				if (strEquals(NormalizeRouteHost(GetHostBindingHostName(dRow)), sHostName)
						&& strEquals(GetHostBindingServiceName(dRow), sServiceName))
					{
					Ctx.sID = GetHostBindingID(dRow);
					break;
					}
				}

			if (Ctx.sID.IsEmpty())
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_HOST_BINDING_NOT_FOUND, sHostName, sServiceName), retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(Ctx.sID);
			dPayload.Append(CDatum());

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_INSERT, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			return EPromiseResult::OK;
			});

	pSession->Then(
		[](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			CDatum dOptions(CDatum::typeArray);
			dOptions.Append(OPTION_INCLUDE_KEY);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(TABLE_ARC_HOSTS);
			dPayload.Append(CDatum());
			dPayload.Append(0);
			dPayload.Append(dOptions);

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, Session.GenerateAddress(VIRTUAL_PORT_HYPERION_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},

		[this](auto &Session, auto &Ctx, const auto &Msg, auto &retReply)
			{
			if (IsError(Msg))
				return ReturnError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_HOST_BINDING_RELOAD_FAILED, Msg.dPayload.AsString()), retReply);

			SetHTTPHosts(Msg.dPayload);
			return ReturnEndSession(MSG_REPLY_DATA, true, retReply);
			});

	StartSession(Msg, std::move(pSession));
	}

void CHyperionEngine::MsgGetServiceList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetServiceList
//
//	Hyperion.getServiceList

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	TArray<CHyperionPackageList::SServiceInfo> List;
	m_Packages.GetServices(&List);
	if (List.GetCount() == 0)
		SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);

	CDatum dReply(CDatum::typeArray);
	for (int i = 0; i < List.GetCount(); i++)
		{
		const IHyperionService *pService = List[i].pService;

		CDatum dEntry(CDatum::typeStruct);
		dEntry.SetElement(FIELD_NAME, List[i].sName);
		dEntry.SetElement(FIELD_PACKAGE, pService->GetPackageName());
		dEntry.SetElement(FIELD_PROTOCOL, pService->GetProtocol());

		if (pService->IsSandboxed())
			dEntry.SetElement(FIELD_SANDBOX, pService->GetSecurityCtx().GetSandboxName());

		dReply.Append(dEntry);
		}

	SendMessageReply(MSG_REPLY_DATA, dReply, Msg);
	}

void CHyperionEngine::MsgGetSessionList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetSessionList
//
//	Hyperion.getSessionList

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	TArray<ISessionHandler *> Sessions;
	GetSessions(&Sessions);

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(Sessions.GetCount());

	for (int i = 0; i < Sessions.GetCount(); i++)
		dResult.Append(Sessions[i]->GetStatusReport());

	//	Result the list of sessions

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CHyperionEngine::MsgGetTaskList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetTaskList
//
//	Hyperion.getTaskList

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	SendMessageReply(MSG_REPLY_DATA, m_Scheduler.GetTaskList(), Msg);
	}

void CHyperionEngine::MsgGetStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetStatus
//
//	Hyperion.getStatus

	{
	int i;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	CComplexStruct *pReply = new CComplexStruct;

	//	Get the status of all threads

	int iCount = GetThreadCount();
	int iStuck = 0;
	int iWaiting = 0;
	int iProcessing = 0;
	int iOther = 0;

	for (i = 0; i < iCount; i++)
		{
		SThreadStatus Status;
		GetThreadStatus(i, &Status);

		switch (Status.iState)
			{
			case processingWaiting:
				iWaiting++;
				break;

			case processingMessages:
				{
				if (Status.dwDuration > 30 * 1000)
					iStuck++;
				else
					iProcessing++;
				break;
				}

			default:
				iOther++;
				break;
			}
		}

	//	Report on thread status

	pReply->SetElement(STR_CHYPERION_ENGINE_HYPERION_SESSION_COUNT, CDatum(GetSessionCount()));
	pReply->SetElement(STR_CHYPERION_ENGINE_HYPERION_THREADS_PROCESSING, CDatum(iProcessing));
	pReply->SetElement(STR_CHYPERION_ENGINE_HYPERION_THREADS_STUCK, CDatum(iStuck));
	pReply->SetElement(STR_CHYPERION_ENGINE_HYPERION_THREADS_WAITING, CDatum(iWaiting));
	pReply->SetElement(STR_CHYPERION_ENGINE_HYPERION_THREAD_COUNT, CDatum(iCount));

#ifdef DEBUG
	TArray<ISessionHandler *> Sessions;
	GetSessions(&Sessions);

	for (i = 0; i < Sessions.GetCount(); i++)
		{
		CHTTPSession *pSession = (CHTTPSession *)Sessions[i];

		Log(MSG_LOG_DEBUG, pSession->GetDebugInfo());
		}
#endif

	SendMessageReply(MSG_REPLY_DATA, CDatum(pReply), Msg);
	}

void CHyperionEngine::MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgHousekeeping
//
//	Arc.housekeeping

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Get a list of tasks to run and send a message for each one.

	TArray<CString> Tasks;
	m_Scheduler.GetTasksToRun(&Tasks);

	for (i = 0; i < Tasks.GetCount(); i++)
		{
		SArchonMessage Msg;
		Msg.sMsg = MSG_HYPERION_RUN_TASK;
		Msg.sReplyAddr = ADDR_NULL;
		Msg.dwTicket = 0;
		Msg.dPayload = CDatum(Tasks[i]);

		SendMessage(Msg);
		}
	}

void CHyperionEngine::MsgSetOption (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSetOption
//
//	Sets an option for the engine.

	{
	CStringView sOption = Msg.dPayload.GetElement(0);
	CDatum dValue = Msg.dPayload.GetElement(1);

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Set the option.

	CString sError;
	if (!m_Options.SetOption(sOption, dValue, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CHyperionEngine::MsgSetTaskRunOn (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSetTaskRunOn
//
//	Hyperion.setTaskRunOn {taskName} [{dateTime}]

	{
	CStringView sTaskName = Msg.dPayload.GetElement(0);
	CDateTime RunOn = Msg.dPayload.GetElement(1).AsDateTime();
	if (!RunOn.IsValid())
		RunOn = CDateTime(CDateTime::Now);

	CString sError;
	if (!m_Scheduler.SetTaskRunOn(sTaskName, RunOn, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CHyperionEngine::MsgStopTask (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgStopTask
//
//	Hyperion.stopTask {taskName}

	{
	CStringView sTaskName = Msg.dPayload.GetElement(0);

	CString sError;
	if (!m_Scheduler.SetSignalStop(sTaskName, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CHyperionEngine::OnBoot (void)

//	OnBoot
//
//	Boot up the engine

	{
	//	Need to register Luminous objects so that we can serve them.

	CAEONLuminous::Boot();
	CArcRouteUtil::Boot();

	//	Register our command port

	AddPort(ADDRESS_HYPERION_COMMAND);
	AddVirtualPort(VIRTUAL_PORT_HYPERION_COMMAND, ADDRESS_HYPERION_COMMAND, FLAG_PORT_NEAREST);

	//	Subscribe to notifications

	AddVirtualPort(VIRTUAL_PORT_AEON_NOTIFY, ADDRESS_HYPERION_COMMAND, FLAG_PORT_ALWAYS);
	AddVirtualPort(VIRTUAL_PORT_CRYPTOSAUR_NOTIFY, ADDRESS_HYPERION_COMMAND, FLAG_PORT_ALWAYS);

	//	Register some Hexe libraries

	RegisterSessionLibrary();
	}

void CHyperionEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark data in use (and garbage collect)

	{
	m_Packages.Mark();
	m_Scheduler.Mark();
	m_Cache.Mark();

	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_HTTPRoutes.GetCount(); i++)
		m_HTTPRoutes[i].dOptions.Mark();
	}

void CHyperionEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Engine is running

	{
	//	Add the built-in services from the executable
	//
	//	NOTE: This adds the package to the list of packages, but we don't
	//	start listening until LoadServices is called.

	AddServicePackage(RESID_ARCOLOGY_PACKAGE);
	}

void CHyperionEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Engine has stopped

	{
	}

void CHyperionEngine::SignalListener (const CString &sName, EListenerStatus iDesiredStatus)

//	SignalListener
//
//	Requests the listener to enter the desired state. If sID is NULL_STR then 
//	all listeners are affected.

	{
	CSmartLock Lock(m_cs);
	int i;

	if (sName.IsEmpty())
		{
		for (i = 0; i < m_Listeners.GetCount(); i++)
			m_Listeners[i].iDesiredStatus = iDesiredStatus;
		}
	else
		{
		int iIndex;
		if (!FindListener(sName, &iIndex))
			return;

		m_Listeners[iIndex].iDesiredStatus = iDesiredStatus;
		}
	}
