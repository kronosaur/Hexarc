//	CTimedMessageQueue.cpp
//
//	CTimedMessageQueue class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG_TIMER_QUEUE
static int ADD_COUNT = 0;
static int DEL_COUNT = 0;
#endif

CTimedMessageQueue::CTimedMessageQueue ()

//	CTimedMessageQueue constructor

	{
	m_HasMessages.Create();

	//	Add a dummy entry at the beginning of the queue. This is because
	//	we use 1-based indexing.

	m_Pool.GrowToFit(DEFAULT_QUEUE_SIZE);
	m_Pool.InsertEmpty();
	}

CTimedMessageQueue::SEntry& CTimedMessageQueue::AddMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload)

//	AddMessage
//
//	Adds a new message to the queue

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG_TIMER_QUEUE
	ADD_COUNT++;
	if ((ADD_COUNT % 100) == 0)
		printf("AddMessage: %d; size = %d\n", ADD_COUNT, m_Pool.GetCount());
#endif

	//	Allocate a new entry

	SEntry& Entry = Allocate();
	Entry.dwTime = ::sysGetTickCount64() + dwDelay;

	//	Add the entry

	Entry.sAddr = sAddr;
	Entry.sMsg = sMsg;
	Entry.sReplyAddr = sReplyAddr;
	Entry.dwTicket = dwTicket;
	Entry.dPayload = dPayload;

	//	Add to queue

	InsertToList(Entry);

	//	If we inserted this message at the front of the queue, then the next
	//	wait time has changed.

	if (Entry.dwID == m_dwFirstTimer)
		m_HasMessages.Set();

	//	Done

	return Entry;
	}

void CTimedMessageQueue::AddTimeoutMessage (DWORD dwDelay, const CString &sAddr, const CString &sMsg, DWORD dwTicket, DWORD *retdwID)

//	AddTimeoutMessage
//
//	Adds a new message in which the payload is the ID of the timeout

	{
	CSmartLock Lock(m_cs);

	SEntry& NewEntry = AddMessage(dwDelay, sAddr, sMsg, NULL_STR, dwTicket, CDatum());
	NewEntry.dPayload = CDatum((int)NewEntry.dwID);

	if (retdwID)
		*retdwID = NewEntry.dwID;
	}

CTimedMessageQueue::SEntry& CTimedMessageQueue::Allocate ()

//	Allocate
//
//	Allocate a new entry.

	{
	//	Find a free entry

	if (m_dwFirstFree)
		{
		SEntry& Entry = m_Pool[m_dwFirstFree];
		m_dwFirstFree = Entry.dwNext;
		return Entry;
		}

	//	Allocate a new entry

	else
		{
		SEntry& Entry = *m_Pool.Insert();
		Entry.dwID = m_Pool.GetCount() - 1;
		return Entry;
		}
	}

void CTimedMessageQueue::DeleteMessage (DWORD dwID)

//	Delete
//
//	Delete by ID

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG_TIMER_QUEUE
	DEL_COUNT++;
	if ((DEL_COUNT % 100) == 0)
		printf("DeleteMessage: %d; size = %d\n", DEL_COUNT, m_Pool.GetCount());
#endif

	if (dwID >= (DWORD)m_Pool.GetCount())
		throw CException(errFail);

	if (m_Pool[dwID].dwID != dwID)
		throw CException(errFail);

	SEntry& Entry = m_Pool[dwID];
	if (!Entry.dwTime)
		{
#ifdef DEBUG
		throw CException(errFail);
#endif
		return;
		}

#ifdef DEBUG_TIMER_QUEUE
	printf("[%x:%d] Delete: %s %d\n", (DWORD)(DWORDLONG)this, Entry.dwID, (LPCSTR)Entry.sAddr, Entry.dwTicket);
#endif

	//	If we're deleting the first timer, then we need to recalculate the
	//	next wait time.

	if (dwID == m_dwFirstTimer)
		m_HasMessages.Set();

	//	Remove and free

	RemoveFromList(Entry);
	Free(Entry);
	}

void CTimedMessageQueue::Free (SEntry& Entry)

//	Free
//
//	Add entry back to the free list.

	{
	Entry.dwTime = 0;				//	Free entries have a time of 0
	Entry.dwNext = m_dwFirstFree;
	m_dwFirstFree = Entry.dwID;
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
	
	//	We always reset the event because we're using the event to signal
	//	that the timer has changed.

	m_HasMessages.Reset();

	//	If no timers, then infinite wait.

	if (m_dwFirstTimer == 0)
		{
		m_HasMessages.Reset();
		return INFINITE;
		}

	//	Otherwise, return the time to wait

	DWORDLONG dwNextTime = m_Pool[m_dwFirstTimer].dwTime;
	DWORDLONG dwNow = ::sysGetTickCount64();

	//	If the next time is in the future, then we just subtract

	if (dwNextTime >= dwNow)
		return (DWORD)(dwNextTime - dwNow);

	//	Otherwise, 0

	else
		return 0;
	}

void CTimedMessageQueue::InsertToList (SEntry& NewEntry)

//	InsertToList
//
//	Inserts the entry to the linked-list of timer messages in time order.

	{
	if (NewEntry.dwTime == 0 || NewEntry.dwID == 0)
		throw CException(errFail);

	//	If this is the first timer, then we're done

	if (m_dwFirstTimer == 0)
		{
		m_dwFirstTimer = NewEntry.dwID;
		m_dwLastTimer = NewEntry.dwID;
		NewEntry.dwPrev = 0;
		NewEntry.dwNext = 0;
		}

	//	Otherwise we search backwards from the end to find a place to insert.

	else
		{
		SEntry& LastEntry = m_Pool[m_dwLastTimer];

		//	If the new entry is after the last entry, then we're done

		if (NewEntry.dwTime >= LastEntry.dwTime)
			{
			LastEntry.dwNext = NewEntry.dwID;
			NewEntry.dwPrev = m_dwLastTimer;
			NewEntry.dwNext = 0;
			m_dwLastTimer = NewEntry.dwID;
			}

		//	Otherwise, we need to search backwards

		else
			{
			SEntry *pEntry = &LastEntry;
			while (pEntry->dwPrev)
				{
				SEntry& PrevEntry = m_Pool[pEntry->dwPrev];
				if (NewEntry.dwTime >= PrevEntry.dwTime)
					{
					NewEntry.dwNext = PrevEntry.dwNext;
					NewEntry.dwPrev = pEntry->dwPrev;
					PrevEntry.dwNext = NewEntry.dwID;
					pEntry->dwPrev = NewEntry.dwID;
					break;
					}
							
				pEntry = &PrevEntry;
				}
			
			//	If we got to the beginning, then we need to insert at the beginning
			
			if (pEntry->dwPrev == 0)
				{
				NewEntry.dwPrev = 0;
				NewEntry.dwNext = m_dwFirstTimer;
				m_Pool[m_dwFirstTimer].dwPrev = NewEntry.dwID;
				m_dwFirstTimer = NewEntry.dwID;
				}
			}
		}
	}

void CTimedMessageQueue::RemoveFromList (SEntry& Entry)

//	RemoveFromList
//
//	Removes the entry from the linked list.

	{
	if (Entry.dwPrev)
		m_Pool[Entry.dwPrev].dwNext = Entry.dwNext;
	else
		m_dwFirstTimer = Entry.dwNext;

	if (Entry.dwNext)
		m_Pool[Entry.dwNext].dwPrev = Entry.dwPrev;
	else
		m_dwLastTimer = Entry.dwPrev;

	Entry.dwPrev = 0;
	Entry.dwNext = 0;
	}

void CTimedMessageQueue::KeepAliveMessage (DWORD dwID)

//	KeepAliveMessage
//
//	Adds 30 seconds to the time to wait.

	{
	CSmartLock Lock(m_cs);

	if (dwID >= (DWORD)m_Pool.GetCount())
		throw CException(errFail);

	SEntry& Entry = m_Pool[dwID];
	if (Entry.dwID != dwID)
		throw CException(errFail);

	//	Set the event to indicate that the first timer has changed.

	if (m_dwFirstTimer == dwID)
		m_HasMessages.Set();

	//	Remove from the linked list.

	RemoveFromList(Entry);

	//	Set the time.

	Entry.dwTime = ::sysGetTickCount64() + KEEP_ALIVE_TIME;

	//	Insert back into the list.

	InsertToList(Entry);
	}

void CTimedMessageQueue::Mark (void)

//	Mark
//
//	Marks all datums

	{
	DEBUG_TRY

	if (!m_dwFirstTimer)
		return;

	SEntry* pEntry = &m_Pool[m_dwFirstTimer];
	while (pEntry)
		{
		pEntry->dPayload.Mark();

		if (pEntry->dwNext)
			pEntry = &m_Pool[pEntry->dwNext];
		else
			break;
		}

	DEBUG_CATCH
	}

void CTimedMessageQueue::ProcessMessages (IArchonProcessCtx *pProcess)

//	ProcessMessages
//
//	Processes (sends) any messages that need to be sent.

	{
	CSmartLock Lock(m_cs);

	if (!m_dwFirstTimer)
		return;

	DWORDLONG dwNow = sysGetTickCount64();
	SEntry* pEntry = &m_Pool[m_dwFirstTimer];
	while (pEntry && pEntry->dwTime <= dwNow)
		{
#ifdef DEBUG_TIMER_QUEUE
		printf("[%x:%d] Timeout: %s %s %d\n", (DWORD)(DWORDLONG)this, pEntry->dwID, (LPCSTR)pEntry->sAddr, (LPCSTR)pEntry->sMsg, pEntry->dwTicket);
#endif

		pProcess->SendMessageCommand(pEntry->sAddr,
				pEntry->sMsg,
				pEntry->sReplyAddr,
				pEntry->dwTicket,
				pEntry->dPayload);

		//	Remove from the list

		SEntry* pOldEntry = pEntry;
		if (pOldEntry->dwNext)
			pEntry = &m_Pool[pOldEntry->dwNext];
		else
			pEntry = NULL;

		RemoveFromList(*pOldEntry);
		Free(*pOldEntry);
		}
	}

