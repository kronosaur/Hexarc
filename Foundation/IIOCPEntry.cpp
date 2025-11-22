//	IIOCPEntry.cpp
//
//	IIOCPEntry class
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_INVALID_STATE,				"Invalid IO operation state.");
DECLARE_CONST_STRING(ERR_FILE_ERROR,				"IO operation error: %d.");
DECLARE_CONST_STRING(ERR_NOT_SUPPORTED,				"IO operation not supported.");
DECLARE_CONST_STRING(ERR_NO_BYTES,					"IO operation returned 0 bytes.");
DECLARE_CONST_STRING(ERR_INVALID_ADDRESS,			"Invalid address: %s %d.");
DECLARE_CONST_STRING(ERR_CANNOT_BIND,				"Unable to bind socket.");
DECLARE_CONST_STRING(ERR_NOT_ENOUGH_BYTES,			"Wrote %d of %d bytes.");
DECLARE_CONST_STRING(ERR_CANT_CONNECT,				"Operation failed during connect.");
DECLARE_CONST_STRING(ERR_CANT_READ,					"Operation failed during read.");
DECLARE_CONST_STRING(ERR_CANT_WRITE,				"Operation failed during write.");

bool IIOCPEntry::BeginConnection (CStringView sAddress, DWORD dwPort, CString* retsError)

//	BeginConnection
//
//	Overlapped connection to the given address.

	{
	//	Must be in the proper state

	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE
			|| m_bConnected 
			|| m_bConnecting)
		{
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	m_bConnecting = true;

	//	We use the write operation to connect

	m_dwWriteStartTime = sysGetTickCount64();
	utlMemSet(&m_OverlappedWrite, sizeof(m_OverlappedWrite));

	//	Connect

	if (!CreateConnection(sAddress, dwPort, m_OverlappedWrite, retsError))
		{
		m_bConnecting = false;
		return false;
		}

	return true;
	}

bool IIOCPEntry::BeginRead (CString *retsError)

//	BeginRead
//
//	Overlapped read on the given handle. Returns FALSE if we get an error.

	{
	//	Must be in the proper state

	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE
			|| !m_bConnected
			|| m_dwReadStartTime != 0)
		{
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	m_dwReadStartTime = sysGetTickCount64();

	//	If we don't have a buffer, then we're done.

	IMemoryBlock* pBuffer = GetReadBuffer();
	if (pBuffer == NULL)
		{
		m_dwReadStartTime = 0;
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	//	Initialize operation

	utlMemSet(&m_OverlappedRead, sizeof(m_OverlappedRead));

	//	Let our subclasses know

	OnBeginRead();

	//	Read into the buffer

	DWORD lasterror = 0;
	if (!::ReadFile(hHandle,
			pBuffer->GetPointer(),
			pBuffer->GetLength(),
			NULL,
			&m_OverlappedRead))
		lasterror = GetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING 
			|| lasterror == 0)
		return true;

	//	If another error or 0 bytes read, then we fail

	else
		{
		m_dwReadStartTime = 0;

		if (retsError)
			*retsError = strPattern(ERR_FILE_ERROR, lasterror);

		return false;
		}
	}

bool IIOCPEntry::BeginWrite (CStringView sData, CString *retsError)

//	BeginWrite
//
//	Overlapped write on the given handle. Returns FALSE if we get an error.

	{
	//	Must be in the proper state

	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE
			|| !m_bConnected
			|| m_dwWriteStartTime != 0)
		{
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	m_dwWriteStartTime = sysGetTickCount64();
	m_dwWriteOffset = 0;

	//	Let our subclasses know (this also initializes the buffer)

	OnBeginWrite(sData);

	//	Initiate overlapped write

	return WriteBuffer(retsError);
	}

bool IIOCPEntry::OperationComplete (DWORD dwBytesTransferred, OVERLAPPED* pOverlapped)

//	OperationComplete
//
//	Operation has completed. Returns TRUE if it succeeded.
	
	{
	CSmartLock Lock(m_cs);

	m_dwLastActivity = sysGetTickCount64();

	if (m_bDeleted)
		{
		//	Nothing else to do; caller will delete this object.
		return true;
		}
	else if (m_bConnecting)
		{
		m_bConnecting = false;
		m_bConnected = true;
		m_dwWriteStartTime = 0;

		OnOperationComplete(EOperation::connect, dwBytesTransferred);
		return true;
		}
	else if (pOverlapped == &m_OverlappedRead && m_dwReadStartTime)
		{
		GetReadBuffer()->SetLength(dwBytesTransferred);
		m_dwReadStartTime = 0;

		if (dwBytesTransferred > 0)
			{
			OnOperationComplete(EOperation::read, dwBytesTransferred);
			return true;
			}
		else
			{
			OnOperationFailed(EOperation::read, ERR_NO_BYTES);
			return false;
			}
		}
	else if (pOverlapped == &m_OverlappedWrite && m_dwWriteStartTime)
		{
		if (dwBytesTransferred > 0)
			{
			m_dwWriteOffset += dwBytesTransferred;

			//	If not done, then we need to write more

			if (m_dwWriteOffset < (DWORD)GetWriteBuffer()->GetLength())
				{
				CString sError;
				if (!WriteBuffer(&sError))
					{
					m_dwWriteStartTime = 0;

					OnOperationFailed(EOperation::write, sError);
					return false;
					}

				return true;
				}
			else
				{
				m_dwWriteStartTime = 0;

				OnOperationComplete(EOperation::write, m_dwWriteOffset);
				return true;
				}
			}
		else
			{
			m_dwWriteStartTime = 0;

			OnOperationFailed(EOperation::write, ERR_NO_BYTES);
			return false;
			}
		}
	else
		{
		//	We should never get here.
		throw CException(errFail);
		}
	}

void IIOCPEntry::OperationFailed (OVERLAPPED* pOverlapped)

//	OperationFailed
//
//	Operation has failed.
	
	{
	CSmartLock Lock(m_cs);

	if (m_bDeleted)
		{
		return;
		}
	else if (m_bConnecting)
		{
		m_bConnecting = false;
		m_bConnected = false;
		m_dwWriteStartTime = 0;

		OnOperationFailed(EOperation::connect, ERR_CANT_CONNECT);
		}
	else if (pOverlapped == &m_OverlappedRead && m_dwReadStartTime)
		{
		m_dwReadStartTime = 0;

		OnOperationFailed(EOperation::read, ERR_CANT_READ);
		}
	else if (pOverlapped == &m_OverlappedWrite && m_dwWriteStartTime)
		{
		m_dwWriteStartTime = 0;

		OnOperationFailed(EOperation::write, ERR_CANT_WRITE);
		}
	else
		{
		//	We should never get here.
		throw CException(errFail);
		}
	}

void IIOCPEntry::Process (void)
	
//	Process
//
//	Process a simple event
	
	{
	CSmartLock Lock(m_cs);

	if (m_bDeleted)
		{
		return;
		}
	
	OnProcess(); 
	}

bool IIOCPEntry::TimeoutCheck (DWORDLONG dwNow, DWORDLONG dwTimeout)

//	TimeoutCheck
//
//	Check to see if we need to cancel an operation in progress.
//	NOTE: We rely on our caller to lock appropriately.
//
//	If we return TRUE, it means the caller should delete us.

	{
	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE)
		return false;

	if (m_dwWriteStartTime > 0)
		{
		DWORDLONG dwElapsed = dwNow - m_dwWriteStartTime;
		if (dwElapsed > dwTimeout)
			{
			::CancelIoEx(hHandle, &m_OverlappedWrite);
			return false;
			}
		}

	if (m_dwReadStartTime > 0)
		{
		DWORDLONG dwElapsed = dwNow - m_dwReadStartTime;
		if (dwElapsed > dwTimeout)
			{
			::CancelIoEx(hHandle, &m_OverlappedRead);
			return false;
			}
		}

	if (m_bMarkedForDelete)
		return true;

	if (m_dwLastActivity > 0)
		{
		DWORDLONG dwElapsed = dwNow - m_dwLastActivity;
		if (dwElapsed <= dwTimeout)
			return false;
		}

	return true;
	}

bool IIOCPEntry::WriteBuffer (CString *retsError)

//	WriteBuffer
//
//	Writes the buffer.

	{
	HANDLE hHandle = GetCompletionHandle();

	//	If we don't have a buffer, then we're done.

	IMemoryBlock *pBuffer = GetWriteBuffer();
	if (pBuffer == NULL)
		{
		m_dwWriteStartTime = 0;
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	//	Initialize operation

	utlMemSet(&m_OverlappedWrite, sizeof(m_OverlappedWrite));

	//	Write into the buffer

#ifdef DEBUG_PERF
	printf("DebugPerf: ::WriteFile %d bytes\n", pBuffer->GetLength());
#endif

	DWORD lasterror = 0;
	if (!::WriteFile(hHandle,
			pBuffer->GetPointer() + m_dwWriteOffset,
			pBuffer->GetLength() - m_dwWriteOffset,
			NULL,
			&m_OverlappedWrite))
		lasterror = GetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING
			|| lasterror == 0)
		return true;

	//	If another error or 0 bytes read, then we fail

	else
		{
		m_dwWriteStartTime = 0;

		if (retsError)
			*retsError = strPattern(ERR_FILE_ERROR, lasterror);

		return false;
		}
	}

