//	CSessionManager.cpp
//
//	CSessionManager class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ARC_KEEP_ALIVE,				"Arc.keepAlive")
DECLARE_CONST_STRING(MSG_ERROR_PREFIX,					"Error.")
DECLARE_CONST_STRING(MSG_SIMPLE_ENGINE_TIMEOUT,			"Error.timeout")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(ERR_CRASH_ON_REPLY,				"%s: CRASH processing reply: %s.")
DECLARE_CONST_STRING(ERR_INVALID_MSG,					"%s: Unhandled message: %s.")
DECLARE_CONST_STRING(ERR_INVALID_REPLY,					"%s: Unhandled reply: %s %s")

CSessionManager::~CSessionManager (void)

//	CSessionManager destructor

	{
	m_Sessions.DeleteAllAndFreeValues();
	m_Deleted.DeleteAllAndFreeValues();
	}

void CSessionManager::Delete (DWORD dwTicket)

//	Delete
//
//	Delete the session

	{
	CSmartLock Lock(m_cs);

	ISessionHandler *pSession = m_Sessions.GetAt(dwTicket);
	if (pSession)
		{
		pSession->EndSession(dwTicket);

		m_Deleted.Insert(pSession);
		m_Sessions.Delete(dwTicket, NULL);
		}
	}

ISessionHandler *CSessionManager::GetAt (DWORD dwTicket)

//	GetAt
//
//	Returns the session for the given ticket

	{
	CSmartLock Lock(m_cs);

	if (!m_Sessions.IsValid(dwTicket))
		return NULL;

	return m_Sessions.GetAt(dwTicket);
	}

void CSessionManager::GetSessions (TArray<ISessionHandler *> *retSessions)

//	GetSessions
//
//	Returns a list of all sessions

	{
	CSmartLock Lock(m_cs);
	SIDTableEnumerator j;

	retSessions->DeleteAll();
	m_Sessions.Reset(j);
	while (m_Sessions.HasMore(j))
		retSessions->Insert(m_Sessions.GetNext(j));
	}

DWORD CSessionManager::Insert (ISessionHandler *pHandler)

//	Insert
//
//	Insert a new session (and return the ticket)

	{
	CSmartLock Lock(m_cs);

	DWORD dwTicket;
	m_Sessions.Insert(pHandler, &dwTicket);
	return dwTicket;
	}

void CSessionManager::Mark (void)

//	Mark
//
//	Mark all handlers

	{
	SIDTableEnumerator j;

	//	NOTE: No need to lock because this is always called while
	//	the world is stopped.

	m_Sessions.Reset(j);
	while (m_Sessions.HasMore(j))
		m_Sessions.GetNext(j)->Mark();

	//	NOTE: No need to mark m_Deleted because they are no longer
	//	valid. In fact, we take this opportunity to purge deleted
	//	stuff.

	m_Deleted.DeleteAllAndFreeValues();
	}

void CSessionManager::ProcessMessage (CSimpleEngine *pEngine, const SArchonMessage &Msg)

//	ProcessMessage
//
//	Processes a session message. This is called from TSimpleEngine. Return FALSE
//	if we got an error processing the message.

	{
	//	Look for the session, but if we don't find it, it must be a bad message

	ISessionHandler *pSession = GetAt(Msg.dwTicket);
	if (pSession == NULL)
		{
		//	If this is an OK message, then skip it.

		if (strEquals(Msg.sMsg, MSG_OK))
			{ }

		//	If this is an error, then it is unhandled.

		else if (strStartsWith(Msg.sMsg, MSG_ERROR_PREFIX))
			{
			pEngine->GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern(ERR_INVALID_REPLY, pEngine->GetName(), Msg.sMsg, Msg.dPayload.AsString()));
			}

		//	If this is a notification, then we don't care.

		else if (CMessagePort::IsNullAddr(Msg.sReplyAddr))
			{
#ifdef DEBUG
			pEngine->GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Ignoring notification: %s", Msg.sMsg));
#endif
			}

		//	Otherwise, we report an unhandled message

		else
			{
			CString sError = strPattern(ERR_INVALID_MSG, pEngine->GetName(), Msg.sMsg);
			pEngine->SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			pEngine->GetProcessCtx()->Log(MSG_LOG_ERROR, sError);
			}

		return;
		}

	//	Let the session process the reply

	try
		{
		bool bContinue;

        //  If this is a keep-alive message, then reset the timeout

        if (strEquals(Msg.sMsg, MSG_ARC_KEEP_ALIVE))
            bContinue = pSession->ProcessKeepAlive(Msg);

		//	If this is a timeout, then deal with it

		else if (strEquals(Msg.sMsg, MSG_SIMPLE_ENGINE_TIMEOUT))
			bContinue = pSession->ProcessTimeout(Msg);

		//	Otherwise process the message

		else
			bContinue = pSession->ProcessMessage(Msg);

		if (!bContinue)
			Delete(Msg.dwTicket);
		}
	catch (...)
		{
		pEngine->GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CRASH_ON_REPLY, pEngine->GetName(), Msg.sMsg));
		}
	}

