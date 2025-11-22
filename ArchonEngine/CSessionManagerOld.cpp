//	CSessionManagerOld.cpp
//
//	CSessionManagerOld class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CSessionManagerOld::~CSessionManagerOld (void)

//	CSessionManagerOld destructor

	{
	PurgeDeleted();

	for (int i = 0; i < m_Sessions.GetCount(); i++)
		delete m_Sessions[i];
	}

void CSessionManagerOld::Delete (SESSIONID dwSessionID)

//	Delete
//
//	Delete the given session

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Sessions.GetCount(); i++)
		if (GetSessionID(m_Sessions[i]) == dwSessionID)
			{
			m_Deleted.Insert(m_Sessions[i]);
			m_Sessions.Delete(i);
			break;
			}
	}

void CSessionManagerOld::DeleteBySocket (SOCKET hSocket)

//	DeleteBySocket
//
//	Delete the given session

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Sessions.GetCount(); i++)
		if (m_Sessions[i]->GetSocket() == hSocket)
			{
			m_Deleted.Insert(m_Sessions[i]);
			m_Sessions.Delete(i);
			break;
			}
	}

ISessionCtx *CSessionManagerOld::FindBySocket (SOCKET hSocket)

//	FindBySocket
//
//	Find the session with the given socket
//	(Or NULL if not found).

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Sessions.GetCount(); i++)
		if (m_Sessions[i]->GetSocket() == hSocket)
			return m_Sessions[i];

	return NULL;
	}

ISessionCtx *CSessionManagerOld::Insert (ISessionCtx *pSession)

//	Insert
//
//	Inserts a session. If a session for the socket already exists
//	we return that session instead (and dispose of the
//	given session)

	{
	CSmartLock Lock(m_cs);

	ISessionCtx *pExisting = FindBySocket(pSession->GetSocket());
	if (pExisting)
		{
		delete pSession;
		return pExisting;
		}

	m_Sessions.Insert(pSession);

	return pSession;
	}

void CSessionManagerOld::PurgeDeleted (void)

//	PurgeDeleted
//
//	Purge deleted sessions

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Deleted.GetCount(); i++)
		delete m_Deleted[i];

	m_Deleted.DeleteAll();
	}
