//	CIOCompletionPort.cpp
//
//	CIOCompletionPort class
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "IOCompletionPortImpl.h"

const int DEFAULT_BUFFER_SIZE =						16 * 1024;
const DWORD TIMEOUT_CHECK_INTERVAL =				15 * 1000;		//	Check for inactivity every 15 seconds
const DWORD INACTIVITY_TIMEOUT =					60 * 60 * 1000;	//	Timeout inactive sessions after an hour.

DECLARE_CONST_STRING(ERR_CANT_BEGIN_OPERATION,		"Unable to begin IO operation.")
DECLARE_CONST_STRING(ERR_CRASH,						"Crash in IO operation.")

CIOCompletionPort::CIOCompletionPort (void) :
		m_iThreads(3)

//	CIOCompletionPort constructor

	{
	m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_iThreads);
	if (m_hIOCP == NULL)
		throw CException(errFail);
	}

CIOCompletionPort::~CIOCompletionPort (void)

//	CIOCompletionPort destructor

	{
	::CloseHandle(m_hIOCP);
	}

void CIOCompletionPort::AddObject (HANDLE hHandle, DWORD_PTR Ctx)

//	AddSocket
//
//	Adds a socket to the completion port.
//
//	NOTE: There is no need to "Delete" or disassociate the given entry from the
//	IO completion port. When the file handle is closed, we automatically
//	dereferrence the IO completion port.

	{
	if (hHandle == INVALID_HANDLE_VALUE)
		throw CException(errFail);

	if (!::CreateIoCompletionPort(hHandle, m_hIOCP, Ctx, 0))
		throw CException(errFail);
	}

void CIOCompletionPort::SignalEvent (DWORD_PTR Ctx)

//	SignalEvent
//
//	Signals a simple event. WaitForCompletion will return with an entry of the
//	given type.
//
//	If iThreadCount is provided, the given number of events are placed in the
//	queue.

	{
	if (!::PostQueuedCompletionStatus(m_hIOCP, 0, Ctx, NULL))
		throw CException(errFail);
	}

bool CIOCompletionPort::ProcessCompletion (SResult& retResult)
	{
	//	Wait for stuff to complete

	bool bSuccess = (::GetQueuedCompletionStatus(m_hIOCP, &retResult.dwBytesTransferred, &retResult.Ctx, &retResult.pOverlapped, INFINITE) == TRUE);

	//	If we failed and pOverlapped is NULL, then this is an error processing
	//	the queue (this should never happen).

	if (!bSuccess && retResult.pOverlapped == NULL)
		throw CException(errFail);

	return bSuccess;
	}
