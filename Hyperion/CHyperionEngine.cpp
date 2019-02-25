//	CHyperionEngine.cpp
//
//	CHyperionEngine class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(RESID_ARCOLOGY_PACKAGE,			"ArcologyPackage")

DECLARE_CONST_STRING(VIRTUAL_PORT_AEON_NOTIFY,			"Aeon.notify")
DECLARE_CONST_STRING(VIRTUAL_PORT_CRYPTOSAUR_NOTIFY,	"Cryptosaur.notify")
DECLARE_CONST_STRING(VIRTUAL_PORT_HYPERION_COMMAND,		"Hyperion.command")

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null")
DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command")
DECLARE_CONST_STRING(ADDRESS_HYPERION_COMMAND,			"Hyperion.command@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_HYPERION,				"Hyperion")

DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath")
DECLARE_CONST_STRING(FIELD_MODIFIED_BY,					"modifiedBy")
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_VERSION,						"version")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_ESPER_START_LISTENER,			"Esper.startListener")
DECLARE_CONST_STRING(MSG_ESPER_STOP_LISTENER,			"Esper.stopListener")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(ERR_UNABLE_TO_CONTINUE,			"Hyperion internal error: %s")

const int THREAD_COUNT = 6;

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
DECLARE_CONST_STRING(MSG_HYPERION_GET_OPTIONS,			"Hyperion.getOptions")
DECLARE_CONST_STRING(MSG_HYPERION_GET_PACKAGE_LIST,		"Hyperion.getPackageList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_SESSION_LIST,		"Hyperion.getSessionList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_TASK_LIST,		"Hyperion.getTaskList")
DECLARE_CONST_STRING(MSG_HYPERION_REFRESH,				"Hyperion.refresh")
DECLARE_CONST_STRING(MSG_HYPERION_RESIZE_IMAGE,			"Hyperion.resizeImage")
DECLARE_CONST_STRING(MSG_HYPERION_RUN_TASK,				"Hyperion.runTask")
DECLARE_CONST_STRING(MSG_HYPERION_SERVICE_MSG,			"Hyperion.serviceMsg")
DECLARE_CONST_STRING(MSG_HYPERION_SET_OPTION,			"Hyperion.setOption")
DECLARE_CONST_STRING(MSG_HYPERION_SET_TASK_RUN_ON,		"Hyperion.setTaskRunOn")
DECLARE_CONST_STRING(MSG_HYPERION_STOP_TASK,			"Hyperion.stopTask")

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

		//	Hyperion.getOptions
		{	MSG_HYPERION_GET_OPTIONS,			&CHyperionEngine::MsgGetOptions },

		//	Hyperion.getPackageList
		{	MSG_HYPERION_GET_PACKAGE_LIST,		&CHyperionEngine::MsgGetPackageList },

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

		//	Hyperion.setOption {option} {value}
		{	MSG_HYPERION_SET_OPTION,			&CHyperionEngine::MsgSetOption },

		//	Hyperion.setTaskRunOn {taskName} [{dateTime}]
		{	MSG_HYPERION_SET_TASK_RUN_ON,		&CHyperionEngine::MsgSetTaskRunOn },

		//	Hyperion.stopTask {taskName}
		{	MSG_HYPERION_STOP_TASK,				&CHyperionEngine::MsgStopTask },
	};

int CHyperionEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CHyperionEngine::m_MsgHandlerList);

CHyperionEngine::CHyperionEngine (void) : TSimpleEngine(ENGINE_NAME_HYPERION, THREAD_COUNT),
		m_bAdminNeeded(false)

//	CHyperionEngine constructor

	{
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
	Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CONTINUE, Msg.dPayload));
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
	CSmartLock Lock(m_cs);
	int i;

	int iIndex;
	if (!FindListener(sListener, &iIndex))
		return false;

	//	Match on the host and url

	CString sHostToMatch = Request.GetRequestedHost();
	CString sURLToMatch = Request.GetRequestedPath();

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

	if (retpService)
		*retpService = pBestService;

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

void CHyperionEngine::MsgGetSessionList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetSessionList
//
//	Hyperion.getSessionList

	{
	int i;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	TArray<ISessionHandler *> Sessions;
	GetSessions(&Sessions);

	CComplexArray *pResult = new CComplexArray;
	pResult->GrowToFit(Sessions.GetCount());

	for (i = 0; i < Sessions.GetCount(); i++)
		pResult->Append(Sessions[i]->GetStatusReport());

	//	Result the list of sessions

	SendMessageReply(MSG_REPLY_DATA, CDatum(pResult), Msg);
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

	pReply->SetElement(CString("Hyperion/sessionCount"), CDatum(GetSessionCount()));
	pReply->SetElement(CString("Hyperion/threadsProcessing"), CDatum(iProcessing));
	pReply->SetElement(CString("Hyperion/threadsStuck"), CDatum(iStuck));
	pReply->SetElement(CString("Hyperion/threadsWaiting"), CDatum(iWaiting));
	pReply->SetElement(CString("Hyperion/threadCount"), CDatum(iCount));

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
	const CString &sOption = Msg.dPayload.GetElement(0);
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
	const CString &sTaskName = Msg.dPayload.GetElement(0);
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
	const CString &sTaskName = Msg.dPayload.GetElement(0);

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
//	Requests the listener to enter the desired state. If sName is NULL_STR then 
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
