//	CTimedMessageQueue.cpp
//
//	CTimedMessageQueue class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const DWORD KEEP_ALIVE_TIME = 1000 * 30;

void CTimedMessageQueue::AddMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload, DWORD *retdwID)

//	AddMessage
//
//	Adds a new message to the queue

	{
	CSmartLock Lock(m_cs);

	//	Compute the smallest time that we have to wait right now

	DWORD dwNow = sysGetTickCount();
	DWORD dwSmallestTimeToWait = GetSmallestTimeToWait(dwNow);

	//	Figure out when this message should be sent

	DWORD dwTime = dwNow + dwDelay;

	//	Add the entry

	SEntry *pEntry = m_Queue.Insert();
	pEntry->dwID = m_dwNextID++;
	pEntry->dwTime = dwTime;
	pEntry->sAddr = sAddr;
	pEntry->sMsg = sMsg;
	pEntry->sReplyAddr = sReplyAddr;
	pEntry->dwTicket = dwTicket;
	pEntry->dPayload = dPayload;

	//	If this message happens before the time we are currently
	//	waiting, then set the event

	if (dwDelay < dwSmallestTimeToWait)
		m_HasMessages.Set();

	//	Done

	if (retdwID)
		*retdwID = pEntry->dwID;
	}

void CTimedMessageQueue::AddTimeoutMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, DWORD dwTicket, DWORD *retdwID)

//	AddTimeoutMessage
//
//	Adds a new message in which the payload is the ID of the timeout

	{
	CSmartLock Lock(m_cs);
	AddMessage(dwDelay, sAddr, sMsg, NULL_STR, dwTicket, CDatum((int)m_dwNextID), retdwID);
	}

void CTimedMessageQueue::DeleteMessage (DWORD dwID)

//	Delete
//
//	Delete by ID

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Queue.GetCount(); i++)
		if (m_Queue[i].dwID == dwID)
			{
			m_Queue.Delete(i);
			break;
			}
	}


DWORD CTimedMessageQueue::GetSmallestTimeToWait (DWORD dwNow)

//	GetSmallestTimeToWait
//
//	Returns the smallest amount of time that we will wait
//	for an event. [Or INIFINITE]
//
//	We assume that m_cs is locked.

	{
	int i;

	DWORD dwSmallestTimeToWait = INFINITE;
	for (i = 0; i < m_Queue.GetCount(); i++)
		{
		DWORD dwTimeToWait = GetTimeToWait(dwNow, m_Queue[i].dwTime);
		if (dwTimeToWait < dwSmallestTimeToWait)
			dwSmallestTimeToWait = dwTimeToWait;
		}

	return dwSmallestTimeToWait;
	}

DWORD CTimedMessageQueue::GetTimeForNextMessage (void)

//	GetTimeForNextMessage
//
//	Returns the number of milliseconds before the next
//	message needs to be processed. This also resets the event so that if any messages
//	are added before this time, the event will be set.
//
//	If there are no message, the function resets the event and returns INFINITE.

	{
	CSmartLock Lock(m_cs);

	//	Loop over all messages and calculate the smallest amount of time
	//	that we need to wait.

	DWORD dwSmallestTimeToWait = GetSmallestTimeToWait(sysGetTickCount());

	//	Reset the event
	//
	//	If any messages are added before the time that we just
	//	returned, we will set the event.

	m_HasMessages.Reset();

	//	Done

	return dwSmallestTimeToWait;
	}

DWORD CTimedMessageQueue::GetTimeToWait (DWORD dwNow, DWORD dwTime)

//	GetTimeToWait
//
//	Returns the amount of time to wait

	{
	//	If time is in the future, then we just subtract

	if (dwTime >= dwNow)
		return dwTime - dwNow;

	else
		{
		//	Otherwise, if we're very far away from Now, then we
		//	must have wrapped around.

		if (dwNow - dwTime > 0x8000000)
			return dwTime - dwNow;

		//	Otherwise, we've passed it

		else
			return 0;
		}
	}

void CTimedMessageQueue::KeepAliveMessage (DWORD dwID)

//	KeepAliveMessage
//
//	Adds 30 seconds to the time to wait.

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Queue.GetCount(); i++)
		if (m_Queue[i].dwID == dwID)
			{
            m_Queue[i].dwTime = ::sysGetTickCount() + KEEP_ALIVE_TIME;
			break;
			}
	}

void CTimedMessageQueue::Mark (void)

//	Mark
//
//	Marks all datums

	{
	int i;

	for (i = 0; i < m_Queue.GetCount(); i++)
		m_Queue[i].dPayload.Mark();
	}

void CTimedMessageQueue::ProcessMessages (IArchonProcessCtx *pProcess)

//	ProcessMessages
//
//	Processes (sends) any messages that need to be sent.

	{
	CSmartLock Lock(m_cs);
	int i;

	DWORD dwNow = sysGetTickCount();
	for (i = 0; i < m_Queue.GetCount(); i++)
		if (GetTimeToWait(dwNow, m_Queue[i].dwTime) == 0)
			{
			pProcess->SendMessageCommand(m_Queue[i].sAddr,
					m_Queue[i].sMsg,
					m_Queue[i].sReplyAddr,
					m_Queue[i].dwTicket,
					m_Queue[i].dPayload);

			m_Queue.Delete(i);
			i--;
			}
	}
