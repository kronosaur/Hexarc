//	CThread.cpp
//
//	CThread class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include <process.h>

CThread::~CThread (void)

//	CThread destructor

	{
	if (m_hThread != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hThread);
	}

void CThread::Start (LPTHREAD_START_ROUTINE pfStart, LPVOID pData)

//	Start
//
//	Start a new thread

	{
	ASSERT(m_hThread == INVALID_HANDLE_VALUE);

	m_hThread = (HANDLE)_beginthreadex(NULL,
			0,
			(unsigned int (__stdcall *)(void *))pfStart,
			pData,
			0,
			(unsigned int *)&m_dwThreadID);
	if (m_hThread == (HANDLE)-1)
		{
		m_hThread = INVALID_HANDLE_VALUE;
		throw CException(errFail);
		}
	}
