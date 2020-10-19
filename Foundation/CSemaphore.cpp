//	CSemaphore.cpp
//
//	CSemaphore class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void CSemaphore::Create (const CString &sName, int iMaxCount, bool *retbExists)

//	Create
//
//	Initializes the semaphore at 0 and with the given maximum

	{
	ASSERT(m_hHandle == INVALID_HANDLE_VALUE);
	ASSERT(iMaxCount > 0);

	//	Create the semaphore

	m_iMaxCount = iMaxCount;
	m_hHandle = ::CreateSemaphore(NULL, 
			m_iMaxCount, 
			m_iMaxCount, 
			(sName.IsEmpty() ? NULL : CString16(sName)));
	if (m_hHandle == NULL)
		throw CException(errFail);

	//	See if the semaphore already exists

	if (retbExists)
		*retbExists = (::GetLastError() == ERROR_ALREADY_EXISTS);
	}

void CSemaphore::Decrement (int iCount) const

//	Decrement
//
//	Decrement the semaphore

	{
	ASSERT(m_hHandle != INVALID_HANDLE_VALUE);
	::ReleaseSemaphore(m_hHandle, iCount, NULL);
	}

void CSemaphore::Increment (int iCount) const

//	Increment the semaphore
//
//	Increments up to iMaxCount. Blocks if we are already at iMaxCount

	{
	ASSERT(m_hHandle != INVALID_HANDLE_VALUE);
	ASSERT(iCount <= m_iMaxCount);

	for (int i = 0; i < iCount; i++)
		::WaitForSingleObject(m_hHandle, INFINITE);
	}

bool CSemaphore::TryIncrement (int iCount, DWORD dwTimeout)

//	TryIncrement
//
//	Attempts to increment the semaphore by the given count. If we cannot
//	increment in the time allowed, we decrement and return FALSE.

	{
	int i;

	ASSERT(m_hHandle != INVALID_HANDLE_VALUE);
	ASSERT(iCount <= m_iMaxCount);

	int iIncs = 0;

	for (i = 0; i < iCount; i++)
		{
		DWORD dwResult = ::WaitForSingleObject(m_hHandle, dwTimeout);
		if (dwResult == WAIT_TIMEOUT)
			{
			Decrement(iIncs);
			return false;
			}

		iIncs++;
		}

	return true;
	}
