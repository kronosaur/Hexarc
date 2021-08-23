//	CSimpleEngine.cpp
//
//	CSimpleEngine class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int DEFAULT_PROCESSING_CHUNK =					1;
const int DEFAULT_QUEUE_SIZE =							1000;

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null");

DECLARE_CONST_STRING(FIELD_CLASS,						"class");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");

DECLARE_CONST_STRING(MSG_ARC_FILE_MSG,					"Arc.fileMsg");
DECLARE_CONST_STRING(MSG_ARC_PING,						"Arc.ping");
DECLARE_CONST_STRING(MSG_ARC_SANDBOX_MSG,				"Arc.sandboxMsg");
DECLARE_CONST_STRING(MSG_ERROR_PREFIX,					"Error.");
DECLARE_CONST_STRING(MSG_ERROR_NO_RIGHT,				"Error.notAllowed");
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_EXARCH_ON_LOG,					"Exarch.onLog")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");
DECLARE_CONST_STRING(MSG_OK,							"OK");
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(STR_SIMPLE_ENGINE,					"CSimpleEngine");
DECLARE_CONST_STRING(STR_MESSAGE_DESC,					"%s: %s from %s:%x");

DECLARE_CONST_STRING(ERR_CRASH_ON_MSG,					"CRASH: %s [%s]: Unknown crash.");
DECLARE_CONST_STRING(ERR_EXCEPTION,						"CRASH: %s [%s]: %s");
DECLARE_CONST_STRING(ERR_INTERNAL_SERVER_ERROR,			"Internal server error.");
DECLARE_CONST_STRING(ERR_MSG_TIMING,					"%s: %s took %d ms to process.");
DECLARE_CONST_STRING(ERR_NOT_ADMIN,						"Service does not have Arc.admin right.");
DECLARE_CONST_STRING(ERR_NOT_IN_SANDBOX,				"Service %s is not authorized to perform that operation.");

CSimpleEngine::CSimpleEngine (const CString &sName, int iInitialThreads) : 
		m_sName(sName),
		m_iInitialThreadCount(iInitialThreads),
		m_Queue(DEFAULT_QUEUE_SIZE),
		m_pTimedMessageThread(NULL),
		m_pRunEvent(NULL),
		m_pPauseEvent(NULL),
		m_pQuitEvent(NULL)

//	CSimpleEngine constructor

	{
	m_RefreshEvent.Create();
	}

CSimpleEngine::~CSimpleEngine (void)

//	CSimpleEngine destructor

	{
	for (int i = 0; i < m_Threads.GetCount(); i++)
		delete m_Threads[i];

	if (m_pTimedMessageThread)
		delete m_pTimedMessageThread;
	}

void CSimpleEngine::AddEventRequest (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket)

//	AddEventRequest
//
//	Adds an event to wait on.

	{
	CSmartLock Lock(m_cs);

	SEvent *pNewEvent = m_NewEvents.Insert();
	pNewEvent->sName = sName;
	pNewEvent->hObj = Event.GetWaitObject();
	pNewEvent->pClient = pPort;
	pNewEvent->sMsg = sMsg;
	pNewEvent->dwTicket = dwTicket;

	//	Set the refresh event so that our thread resets

	m_RefreshEvent.Set();
	}

int CSimpleEngine::AddEvents (CWaitArray *retWait)

//	AddEvents
//
//	Adds events to the wait list and returns the index for the
//	first event added. (If there are no events to add then
//	we return -1)

	{
	CSmartLock Lock(m_cs);

	int i;
	int iFirst = -1;

	for (i = 0; i < m_Events.GetCount(); i++)
		{
		int iPos = retWait->Insert(m_Events[i].hObj, strPattern("%s/%s", m_Events[i].sMsg, m_Events[i].sName));
		if (iFirst == -1)
			iFirst = iPos;
		}

	return iFirst;
	}

void CSimpleEngine::Boot (IArchonProcessCtx *pProcess, DWORD dwID)

//	Boot
//
//	Boot the engine

	{
	m_pProcess = pProcess;
	m_dwID = dwID;

	//	Let descendants initializes

	OnBoot();
	}

CDatum CSimpleEngine::GetPortStatus (void) const

//	GetPortStatus
//
//	Returns a struct with well-known fields for status.

	{
	CDatum dStatus = CDatum(CDatum::typeStruct);
	dStatus.SetElement(FIELD_CLASS, STR_SIMPLE_ENGINE);
	dStatus.SetElement(FIELD_STATUS, NULL_STR);
	return dStatus;
	}

void CSimpleEngine::GetThreadStatus (int iThread, SThreadStatus *retStatus)

//	GetThreadStatus
//
//	Returns the status of the given thread.

	{
	if (iThread < 0 || iThread >= m_Threads.GetCount())
		{
		retStatus->iState = processingUnknown;
		retStatus->dwDuration = 0;
		return;
		}

	retStatus->iState = m_Threads[iThread]->GetState(&retStatus->dwDuration);
	}

bool CSimpleEngine::IsError (const SArchonMessage &Msg)

//	IsError
//
//	Returns TRUE if this message is an error

	{
	return strStartsWith(Msg.sMsg, MSG_ERROR_PREFIX);
	}

bool CSimpleEngine::IsFileMsg (const SArchonMessage &Msg, CArchonMessageList *retList)

//	IsFileMsg
//
//	If this is an Arc.fileMsg message, then process it.
//
//	NOTE: The semantic of this is that it puts the message into list format (as 
//	the only element) so it can be used by OnProcessMessages

	{
	if (!strEquals(Msg.sMsg, MSG_ARC_FILE_MSG))
		return false;

	retList->DeleteAll();

	SArchonMessage *pMsg = retList->Insert();
	CInterprocessMessageQueue::DecodeFileMsg(Msg, pMsg);
	return true;
	}

bool CSimpleEngine::IsIdle (void)

//	IsIdle
//
//	Returns TRUE if all threads are idle. FALSE otherwise.

	{
	int i;

	try
		{
		for (i = 0; i < m_Threads.GetCount(); i++)
			{
			SThreadStatus Status;
			GetThreadStatus(i, &Status);
			if (Status.iState != processingWaiting)
				return false;
			}
		}
	catch (...)
		{
		return false;
		}

	return true;
	}

bool CSimpleEngine::IsSandboxMsg (const SArchonMessage &Msg, SArchonMessage *retMsg, CHexeSecurityCtx *retSecurityCtx)

//	IsSandboxMsg
//
//	If this is an Arc.sandboxMsg message then we translate it into a message
//	and a security context.

	{
	if (!strEquals(Msg.sMsg, MSG_ARC_SANDBOX_MSG))
		return false;

	retMsg->sMsg = Msg.dPayload.GetElement(0);
	retMsg->sReplyAddr = Msg.sReplyAddr;
	retMsg->dwTicket = Msg.dwTicket;
	retMsg->dPayload = Msg.dPayload.GetElement(1);

	retSecurityCtx->Init(Msg.dPayload.GetElement(2));

	return true;
	}

void CSimpleEngine::LogCrashProcessingMessage (const SArchonMessage &Msg, const CException &e)

//	LogCrashProcessingMessage
//
//	Reports a crash

	{
	if (e.GetCode() == errUnknownError)
		Log(MSG_LOG_ERROR, strPattern(ERR_CRASH_ON_MSG, GetName(), Msg.sMsg));
	else
		Log(MSG_LOG_ERROR, strPattern(ERR_EXCEPTION, GetName(), Msg.sMsg, e.GetErrorString()));

	SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INTERNAL_SERVER_ERROR, Msg);
	}

void CSimpleEngine::LogMessage (const SArchonMessage &Msg)

//	LogMessage
//
//	Logs a message.

	{
	if (strEquals(Msg.sMsg, MSG_EXARCH_ON_LOG))
		return;

	Log(MSG_LOG_INFO, strPattern(STR_MESSAGE_DESC, GetName(), Msg.sMsg, Msg.sReplyAddr, Msg.dwTicket));
	}

void CSimpleEngine::LogMessageTiming (const SArchonMessage &Msg, DWORD dwTime)

//	LogMessageTiming
//
//	Report when a message takes too long.

	{
	//	Don't log error messages. In some cases, like Hyperion start up, we do
	//	some processing in response to an error.

	if (IsError(Msg))
		return;

	//	Log it.

	Log(MSG_LOG_INFO, strPattern(ERR_MSG_TIMING, GetName(), Msg.sMsg, dwTime));
	}

bool CSimpleEngine::ProcessMessageDefault (const SArchonMessage &Msg)

//	ProcessMessageDefault
//
//	Process some default messages.

	{
	if (strEquals(Msg.sMsg, MSG_ARC_PING))
		{
		SendMessageReply(MSG_REPLY_DATA, Msg.dPayload, Msg);
		return true;
		}
	else
		return false;
	}

CDatum CSimpleEngine::MessageToHexeResult (const SArchonMessage &Msg)

//	MessageToHexeResult
//
//	Converts a message to a result for a CHexeProcess

	{
	CDatum dResult;
	if (IsError(Msg))
		CHexeError::Create(Msg.sMsg, Msg.dPayload, &dResult);
	else if (strEquals(Msg.sMsg, MSG_OK) && Msg.dPayload.IsNil())
		dResult = CDatum(CDatum::constTrue);
	else
		dResult = Msg.dPayload;

	return dResult;
	}

void CSimpleEngine::ProcessEvent (const CString &sEventID)

//	ProcessEvent
//
//	Send a message for this event

	{
	CSmartLock Lock(m_cs);

	//	Look for the event in our list

	int iEvent;
	SEvent *pEvent = NULL;
	for (iEvent = 0; iEvent < m_Events.GetCount(); iEvent++)
		{
		CString sID = strPattern("%s/%s", m_Events[iEvent].sMsg, m_Events[iEvent].sName);
		if (strEquals(sEventID, sID))
			{
			pEvent = &m_Events[iEvent];
			break;
			}
		}

	//	If we can't find it, that's OK, some other thread
	//	probably handled it.

	if (pEvent == NULL)
		return;

	//	Send a message

	SArchonMessage Msg;
	Msg.sMsg = pEvent->sMsg;
	Msg.sReplyAddr = (pEvent->dwTicket == 0 ? ADDR_NULL : NULL_STR);
	Msg.dwTicket = pEvent->dwTicket;

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(CDatum(pEvent->sName));

	Msg.dPayload = CDatum(pPayload);

	pEvent->pClient->SendMessage(Msg);

	//	Done. We delete the event from the list. It is up
	//	to the message handler to reset the event and add it

	m_Events.Delete(iEvent);
	}

void CSimpleEngine::ProcessRefresh (void)

//	ProcessRefresh
//
//	Move events from the new list to the actual list

	{
	CSmartLock Lock(m_cs);

	int i;

	if (m_NewEvents.GetCount() > 0)
		{
		for (i = 0; i < m_NewEvents.GetCount(); i++)
			m_Events.Insert(m_NewEvents[i]);

		m_NewEvents.DeleteAll();
		m_RefreshEvent.Reset();
		}
	}

void CSimpleEngine::StartRunning (CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent)

//	StartRunning
//
//	Start all the threads

	{
	int i;

	m_pRunEvent = &RunEvent;
	m_pPauseEvent = &PauseEvent;
	m_pQuitEvent = &QuitEvent;

	//	Tell our descendants that we're about to start

	OnStartRunning();

	//	Start all threads

	m_pTimedMessageThread = new CSimpleEventThread(this);
	m_pTimedMessageThread->Start();

	for (i = 0; i < m_iInitialThreadCount; i++)
		{
		CSimpleProcessingThread *pThread = new CSimpleProcessingThread(this);
		m_Threads.Insert(pThread);
		pThread->Start();
		}
	}

bool CSimpleEngine::ValidateSandbox (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, const CString &sSandbox)

//	ValidateSandbox
//
//	Validates that we are called by the given sandboxed service OR that the user
//	has admin rights. If not, then we reply with an error.

	{
	//	If no security context then we are being run by a privileged entity.

	if (pSecurityCtx == NULL
			|| pSecurityCtx->HasServiceRightArcAdmin())
		return true;

	//	Check to see if we are in the given sandbox

	if (strEquals(pSecurityCtx->GetSandbox(), sSandbox))
		return true;

	//	Otherwise, error

	SendMessageReplyError(MSG_ERROR_NO_RIGHT, strPattern(ERR_NOT_IN_SANDBOX, pSecurityCtx->GetSandbox()), Msg);
	return false;
	}

bool CSimpleEngine::ValidateSandboxAdmin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	ValidateSandboxAdmin
//
//	Validates that the security context has admin rights. If not, then we reply
//	with an error.

	{
	//	If no security context then we are being run by a privileged entity.

	if (pSecurityCtx == NULL)
		return true;

	if (!pSecurityCtx->HasServiceRightArcAdmin())
		{
		SendMessageReplyError(MSG_ERROR_NO_RIGHT, ERR_NOT_ADMIN, Msg);
		return false;
		}

	//	Done

	return true;
	}

void CSimpleEngine::WaitForShutdown (void)

//	WaitForShutdown
//
//	Wait for threads to exit.

	{
	for (int i = 0; i < m_Threads.GetCount(); i++)
		m_Threads[i]->Wait();

	m_pTimedMessageThread->Wait();

	OnWaitForShutdown();
	}

void CSimpleEngine::WaitForPause (void)

//	WaitForPause
//
//	Wait for all threads to pause for garbage collection

	{
	for (int i = 0; i < m_Threads.GetCount(); i++)
		m_Threads[i]->WaitForPause();

	m_pTimedMessageThread->WaitForPause();

	OnWaitForPause();
	}

//	CSimpleProcessingThread ----------------------------------------------------

CSimpleProcessingThread::CSimpleProcessingThread (CSimpleEngine *pEngine) :
		m_pEngine(pEngine),
		m_iProcessingChunk(DEFAULT_PROCESSING_CHUNK),
		m_iState(processingUnknown),
		m_dwLastStateChange(0)

//	CSimpleProcessingThread constructor

	{
	m_PausedEvent.Create();
	}

EProcessingState CSimpleProcessingThread::GetState (DWORD *retdwDuration)

//	GetState
//
//	Returns the current state of the engine and how long it has been in that
//	state.
//
//	NOTE: For efficiency we don't lock anything when returning this, so there
//	is a small chance that the state will not match the duration.

	{
	if (retdwDuration)
		*retdwDuration = ::GetTickCount() - m_dwLastStateChange;

	return m_iState;
	}

void CSimpleProcessingThread::Run (void)

//	Run
//
//	Processing thread

	{
	try
		{
		//	Keep looping until we're asked to quit

		while (true)
			{
			SetState(processingWaiting);

			//	Wait for the quit event and the work event

			CWaitArray Wait;
			int STOP_EVENT = Wait.Insert(m_pEngine->GetPauseEvent());
			int QUIT_EVENT = Wait.Insert(m_pEngine->GetQuitEvent());
			int WORK_EVENT = Wait.Insert(m_pEngine->GetQueueEvent());
			int REFRESH_EVENT = Wait.Insert(m_pEngine->GetRefreshEvent());
			int FIRST_EVENT = m_pEngine->AddEvents(&Wait);

			int iEvent = Wait.WaitForAny();
			if (iEvent == QUIT_EVENT)
				{
				SetState(processingExit);
				return;
				}
			else if (iEvent == STOP_EVENT)
				{
				SetState(processingStopped);
				m_PausedEvent.Set();

				CWaitArray Wait;
				int QUIT_EVENT = Wait.Insert(m_pEngine->GetQuitEvent());
				int RUN_EVENT = Wait.Insert(m_pEngine->GetRunEvent());

				int iEvent = Wait.WaitForAny();
				if (iEvent == STOP_EVENT)
					{
					SetState(processingExit);
					return;
					}
				else
					m_PausedEvent.Reset();
				}
			else if (iEvent == WORK_EVENT)
				{
				SetState(processingMessages);

				CArchonMessageList MsgList;
				if (m_pEngine->Dequeue(m_iProcessingChunk, &MsgList))
					m_pEngine->ProcessMessages(MsgList);
				}
			else if (iEvent == REFRESH_EVENT)
				{
				SetState(processingRefresh);
				m_pEngine->ProcessRefresh();
				}
			else if (FIRST_EVENT != -1 && iEvent >= FIRST_EVENT)
				{
				SetState(processingEvent);
				m_pEngine->ProcessEvent(Wait.GetData(iEvent));
				}
			}
		}
	catch (...)
		{
		m_pEngine->GetProcessCtx()->LogBlackBox(strPattern("CRASH: CSimpleEngine: %s.", m_pEngine->GetName()));
		throw;
		}
	}

//	CSimpleEventThread ---------------------------------------------------------

CSimpleEventThread::CSimpleEventThread (CSimpleEngine *pEngine) :
		m_pEngine(pEngine)

//	CSimpleEventThread constructor

	{
	m_PausedEvent.Create();
	}

void CSimpleEventThread::Run (void)

//	Run
//
//	Processing thread

	{
	try
		{
		//	Keep looping until we're asked to quit

		while (true)
			{
			//	Wait for the quit event and the work event

			CWaitArray Wait;
			int STOP_EVENT = Wait.Insert(m_pEngine->GetPauseEvent());
			int QUIT_EVENT = Wait.Insert(m_pEngine->GetQuitEvent());
			int NEW_MESSAGES_EVENT = Wait.Insert(m_pEngine->GetTimedMessageEvent());

			//	Figure out how long to wait

			DWORD dwWaitTime = m_pEngine->GetTimedMessageWait();

			int iEvent = Wait.WaitForAny(dwWaitTime);
			if (iEvent == QUIT_EVENT)
				return;
			else if (iEvent == STOP_EVENT)
				{
				m_PausedEvent.Set();

				CWaitArray Wait;
				int QUIT_EVENT = Wait.Insert(m_pEngine->GetQuitEvent());
				int RUN_EVENT = Wait.Insert(m_pEngine->GetRunEvent());

				int iEvent = Wait.WaitForAny();
				if (iEvent == STOP_EVENT)
					return;
				else
					m_PausedEvent.Reset();
				}
			else if (iEvent == NEW_MESSAGES_EVENT)
				;
			else if (iEvent == OS_WAIT_TIMEOUT)
				{
				m_pEngine->ProcessTimedMessages();
				}
			}
		}
	catch (...)
		{
		m_pEngine->GetProcessCtx()->LogBlackBox(strPattern("CRASH: CSimpleEventThread: %s.", m_pEngine->GetName()));
		throw;
		}
	}

