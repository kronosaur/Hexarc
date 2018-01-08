//	CIOCompletionPort.cpp
//
//	CIOCompletionPort class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

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

void CIOCompletionPort::AddObject (IIOCPEntry *pEntry)

//	AddSocket
//
//	Adds a socket to the completion port.
//
//	NOTE: There is no need to "Delete" or disassociate the given entry from the
//	IO completion port. When the file handle is closed, we automatically
//	dereferrence the IO completion port.

	{
	HANDLE hNewHandle = pEntry->GetCompletionHandle();
	if (hNewHandle != INVALID_HANDLE_VALUE)
		{
		if (!::CreateIoCompletionPort(hNewHandle, m_hIOCP, (DWORD_PTR)pEntry, 0))
			throw CException(errFail);
		}
	}

void CIOCompletionPort::SignalEvent (IIOCPEntry *pObject)

//	SignalEvent
//
//	Signals a simple event. WaitForCompletion will return with an entry of the
//	given type.
//
//	If iThreadCount is provided, the given number of events are placed in the
//	queue.

	{
	if (pObject == NULL)
		return;

	if (!::PostQueuedCompletionStatus(m_hIOCP, 0, (DWORD_PTR)pObject, NULL))
		throw CException(errFail);
	}

bool CIOCompletionPort::ProcessCompletion (IIOCPEntry **retpObject, DWORD *retdwBytesTransferred)
	{
	//	Wait for stuff to complete

	DWORD dwBytesTransferred;
	IIOCPEntry *pEntry;
	OVERLAPPED *pOverlapped;

	bool bSuccess = (::GetQueuedCompletionStatus(m_hIOCP, &dwBytesTransferred, (DWORD_PTR *)&pEntry, &pOverlapped, INFINITE) == TRUE);

	//	If we failed and pOverlapped is NULL, then this is an error processing
	//	the queue (this should never happen).

	if (!bSuccess && pOverlapped == NULL)
		throw CException(errFail);

	//	We always return the object

	*retpObject = pEntry;
	*retdwBytesTransferred = dwBytesTransferred;

	//	If we don't have a completion handle, then we always succeed
	//	(This is an event notification.)

	if (pEntry->GetCompletionHandle() == INVALID_HANDLE_VALUE)
		return true;

	//	Let this entry handle the completion

	else if (bSuccess)
		{
		switch (pEntry->GetCurrentOp())
			{
			//	Connections work even if 0 bytes transferred

			case IIOCPEntry::opConnect:
				return true;

			//	Writes fail unless all bytes are sent

			case IIOCPEntry::opWrite:
				return (dwBytesTransferred == pEntry->GetBuffer()->GetLength());

			//	Else, success if more than 0 bytes

			default:
				return (dwBytesTransferred > 0);
			}
		}
	else
		{
		//	NOTE: We connecting (with ConnectEx) if the connection fails, we timeout
		//	from internal TCP timeouts, regardless of the fact that we pass INFINITE to
		//	GetQueuedCompletionStatus. This is OK, but we could check the error result
		//	here and return it.

		return false;
		}
	}
