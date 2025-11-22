//	CMessageQueue.cpp
//
//	CMessageQueue class
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CMessageQueue::Dequeue (int iMaxCount, CArchonMessageList *retList)

//	Dequeue
//
//	Adds up to iMaxCount messages to the list (or returns false if there
//	are no messages in the queue).

	{
	CSmartLock Lock(m_cs);
	int iCount = Min(iMaxCount, m_Queue.GetCount());
	if (iCount == 0)
		return false;

	for (int i = 0; i < iCount; i++)
		retList->Insert(m_Queue.GetAt(i));

	m_Queue.Dequeue(iCount);

	if (m_Queue.IsEmpty())
		m_HasMessages.Reset();

	return true;
	}

bool CMessageQueue::Enqueue (const SArchonMessage &Msg)

//	Enqueue
//
//	Enqueues a message to the queue. Returns TRUE if the enqueue succeeded.

	{
	CSmartLock Lock(m_cs);

	bool bWasEmpty = m_Queue.IsEmpty();
	m_Queue.EnqueueAndGrow(Msg);

	if (bWasEmpty && !m_Queue.IsEmpty())
		m_HasMessages.Set();

	return true;
	}

void CMessageQueue::Mark (void)

//	Mark
//
//	Mark all AEON data in use

	{
	DEBUG_TRY

	for (int i = 0; i < m_Queue.GetCount(); i++)
		m_Queue[i].dPayload.Mark();

	DEBUG_CATCH
	}
