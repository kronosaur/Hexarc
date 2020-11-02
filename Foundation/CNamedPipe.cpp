//	CNamedPipe.cpp
//
//	CNamedPipe class
//	Copyright (c) 2010 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

volatile LONG CNamedPipe::m_dwSN = 0;

CNamedPipe::~CNamedPipe ()

//	CNamedPipe destructor

	{
	if (m_bWaitingForRead)
		CancelIo(m_hRead);

	if (m_hWrite != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hWrite);

	if (m_hRead != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hRead);
	}

bool CNamedPipe::Create (const CString &sName, DWORD dwFlags)

//	Create
//
//	Create a named pipe.

	{
	//	Form the pipe name.

	m_sName = ComposeName(sName);

	//	Create the pipe.

	DWORD dwOpenMode = PIPE_ACCESS_INBOUND;
	if (dwFlags & FLAG_READ_OVERLAPPED)
		dwOpenMode |= FILE_FLAG_OVERLAPPED;

	DWORD dwPipeMode = PIPE_TYPE_BYTE | PIPE_WAIT;

	constexpr DWORD dwRecommendedSize = 4096;
	constexpr DWORD dwTimeOut = 120 * 1000;

	SECURITY_ATTRIBUTES ReadSecurity;
	utlMemSet(&ReadSecurity, sizeof(ReadSecurity));
	ReadSecurity.nLength = sizeof(ReadSecurity);
	if (dwFlags & FLAG_READ_INHERIT)
		ReadSecurity.bInheritHandle = TRUE;

	m_hRead = ::CreateNamedPipeW(CString16(m_sName),
			dwOpenMode,
			dwPipeMode,
			1,
			dwRecommendedSize,
			dwRecommendedSize,
			dwTimeOut,
			&ReadSecurity
			);
	if (m_hRead == INVALID_HANDLE_VALUE)
		return false;

	//	Create the write handle

	DWORD dwCreateFlags = FILE_ATTRIBUTE_NORMAL;
	if (dwFlags & FLAG_WRITE_OVERLAPPED)
		dwCreateFlags |= FILE_FLAG_OVERLAPPED;

	SECURITY_ATTRIBUTES WriteSecurity;
	utlMemSet(&WriteSecurity, sizeof(WriteSecurity));
	WriteSecurity.nLength = sizeof(WriteSecurity);
	if (dwFlags & FLAG_WRITE_INHERIT)
		WriteSecurity.bInheritHandle = TRUE;

	m_hWrite = ::CreateFileW(CString16(m_sName),
			GENERIC_WRITE,
			0,
			&WriteSecurity,
			OPEN_EXISTING,
			dwCreateFlags,
			NULL
			);
	if (m_hWrite == INVALID_HANDLE_VALUE)
		{
		::CloseHandle(m_hRead);
		m_hRead = INVALID_HANDLE_VALUE;
		return false;
		}

	//	Create events, if necessary

	if (dwFlags & FLAG_READ_OVERLAPPED)
		{
		utlMemSet(&m_OverlappedRead, sizeof(m_OverlappedRead));

		m_ReadEvent.Create();
		m_ReadEvent.Reset();
		m_OverlappedRead.hEvent = m_ReadEvent.GetWaitObject();

		m_bAsyncRead = true;
		m_ReadBuffer.SetLength(BUFFER_SIZE);
		AsyncRead();
		}

	if (dwFlags & FLAG_WRITE_OVERLAPPED)
		{
		utlMemSet(&m_OverlappedWrite, sizeof(m_OverlappedWrite));

		m_WriteEvent.Create();
		m_OverlappedWrite.hEvent = m_WriteEvent.GetWaitObject();
		}

	return true;
	}

bool CNamedPipe::AsyncRead ()

//	AsyncRead
//
//	Initiate an asynchronous read.

	{
	if (m_bWaitingForRead || !m_bAsyncRead)
		return false;

	if (!::ReadFile(m_hRead, m_ReadBuffer.GetPointer(), m_ReadBuffer.GetAllocSize(), NULL, &m_OverlappedRead))
		{
		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_IO_PENDING)
			return false;
		}

	m_bWaitingForRead = true;
	return true;
	}

CString CNamedPipe::Read ()

//	Read
//
//	Reads from the pipe and returns data.

	{
	if (m_bWaitingForRead)
		{
		DWORD dwRead;
		if (!::GetOverlappedResult(m_hRead, &m_OverlappedRead, &dwRead, FALSE))
			{
			DWORD dwError = ::GetLastError();
			if (dwError == ERROR_IO_INCOMPLETE)
				return NULL_STR;
			else
				{
				//	LATER: Handle a real error.

				return NULL_STR;
				}
			}

		CString sResult(m_ReadBuffer.GetPointer(), dwRead);

		//	Read some more.

		m_bWaitingForRead = false;
		AsyncRead();

		return sResult;
		}
	else
		{
		//	LATER: Synchronous read
		return NULL_STR;
		}
	}

CString CNamedPipe::ComposeName (const CString &sText)

//	ComposeName
//
//	Compose a name for a named pipe.

	{
	if (sText.IsEmpty())
		return strPattern("\\\\.\\Pipe\\Anonymous.%08x.%08x", GetCurrentProcessId(), InterlockedIncrement(&m_dwSN));
	else
		return strPattern("\\\\.\\Pipe\\%s", sText);
	}

