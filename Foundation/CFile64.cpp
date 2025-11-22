//	CFile64.cpp
//
//	CFile64 class
//	Copyright (c) 2024 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CANT_OPEN_FILE,				"Unable to open file: %s. [%x]");
DECLARE_CONST_STRING(ERR_READ,							"Error reading file: %s. [%x]");
DECLARE_CONST_STRING(ERR_SEEK,							"Error seeking in file: %s. [%x]");
DECLARE_CONST_STRING(ERR_UNLOCK,						"Error unlocking file: %s. [%x]");
DECLARE_CONST_STRING(ERR_WRITE,							"Error writing file: %s. [%x]");

CFile64::~CFile64 ()

//	CFile64 destructor

	{
	Close();
	}

void CFile64::Close ()

//	Close
//
//	Close the file

	{
	if (m_hFile != INVALID_HANDLE_VALUE)
		{
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		}
	}

bool CFile64::Create (CStringView sFilespec, DWORD dwFlags, CString *retsError)

//	Create
//
//	Create a file stream

	{
	ASSERT(m_hFile == INVALID_HANDLE_VALUE);

	m_sFilespec = sFilespec;

	//	Get flags

	DWORD dwFileFlags = FILE_ATTRIBUTE_NORMAL;
	if (dwFlags & FLAG_WRITE_THROUGH)
		dwFileFlags |= FILE_FLAG_WRITE_THROUGH;

	//	Desired access

	DWORD dwDesiredAccess = 0;
	if (dwFlags & FLAG_OPEN_READ_ONLY)
		dwDesiredAccess |= GENERIC_READ;
	else
		dwDesiredAccess |= GENERIC_READ | GENERIC_WRITE;

	DWORD dwShareMode = FILE_SHARE_READ;
	if ((dwFlags & FLAG_OPEN_READ_ONLY)
			|| (dwFlags & FLAG_ALLOW_WRITE))
		dwShareMode |= FILE_SHARE_WRITE;

	//	NOTE: If the file is opened with FILE_SHARE_DELETE and a second process
	//	tries to open without FILE_SHARE_DELETE, then the second open will fail,
	//	even if other flags match.

	if (dwFlags & FLAG_ALLOW_DELETE)
		dwShareMode |= FILE_SHARE_DELETE;

	DWORD dwCreation = 0;
	if (dwFlags & FLAG_OPEN_READ_ONLY)
		dwCreation = OPEN_EXISTING;
	else if (dwFlags & FLAG_CREATE_NEW)
		dwCreation = CREATE_NEW;
	else if (dwFlags & FLAG_CREATE_ALWAYS)
		dwCreation = CREATE_ALWAYS;
	else if (dwFlags & FLAG_OPEN_ALWAYS)
		dwCreation = OPEN_ALWAYS;
	else
		dwCreation = OPEN_EXISTING;

	//	Open

	m_hFile = ::CreateFile(CString16(m_sFilespec),
			dwDesiredAccess,
			dwShareMode,
			NULL,
			dwCreation,
			dwFileFlags,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		if (retsError)
			*retsError = strPattern(ERR_CANT_OPEN_FILE, m_sFilespec, ::GetLastError());

		return false;
		}

	return true;
	}

bool CFile64::Flush ()

//	Flush
//
//	Flush the file to ensure it is written to disk.

	{
	if (!IsOpen())
		return false;

	if (!::FlushFileBuffers(m_hFile))
		return false;

	return true;
	}

CDateTime CFile64::GetModifiedTime ()

//	GetModifiedTime
//
//	Returns the modified time.

	{
	//	Get modified time

	FILETIME ftWrite;
	if (!::GetFileTime(m_hFile, NULL, NULL, &ftWrite))
		return CDateTime(CDateTime::BeginningOfTime);

	//	Convert

	SYSTEMTIME SystemTime;
	::FileTimeToSystemTime(&ftWrite, &SystemTime);

	//	Done

	return CDateTime(SystemTime);
	}

DWORDLONG CFile64::GetPos ()

//	GetPos
//
//	Get current file position

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	LARGE_INTEGER liPos;
	LARGE_INTEGER liMove;
	liMove.QuadPart = 0;

	if (!::SetFilePointerEx(m_hFile, liMove, &liPos, FILE_CURRENT))
		throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_SEEK, m_sFilespec, ::GetLastError()));

	return liPos.QuadPart;
	}

DWORDLONG CFile64::GetSize (void)

//	GetSize
//
//	Gets the size of the file.

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	LARGE_INTEGER liSize;
	if (!::GetFileSizeEx(m_hFile, &liSize))
		return 0;

	return liSize.QuadPart;
	}

DWORDLONG CFile64::GetStreamLength (void)

//	GetStreamLength
//
//	Get current stream length

	{
	return GetSize();
	}

bool CFile64::Lock (DWORDLONG dwPos, DWORDLONG dwLength, int iTimeout)

//	Lock
//
//	Locks a region of the file and returns TRUE if the lock succeeded.

	{
	DWORD dwStartTime = sysGetTickCount();
	OVERLAPPED overlapped = { 0 };
	overlapped.Offset = static_cast<DWORD>(dwPos & 0xFFFFFFFF);          // Lower 32-bits of the offset
	overlapped.OffsetHigh = static_cast<DWORD>((dwPos >> 32) & 0xFFFFFFFF); // Higher 32-bits of the offset

	DWORD lockLow = static_cast<DWORD>(dwLength & 0xFFFFFFFF);           // Lower 32-bits of the length
	DWORD lockHigh = static_cast<DWORD>((dwLength >> 32) & 0xFFFFFFFF);  // Higher 32-bits of the length

	while (true)
		{
		//	Try to lock. If we succeed, then we're done

		if (::LockFileEx(m_hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, lockLow, lockHigh, &overlapped))
			return true;

		//	If we fail, and we've exceeded the timeout, we exit.

		if (iTimeout == 0 || sysGetTicksElapsed(dwStartTime) > (DWORD)iTimeout)
			return false;

		//	Otherwise, we wait a bit and try again.

		::Sleep(iTimeout / 10);
		}
	}

DWORDLONG CFile64::ReadTry (void *pData, DWORDLONG dwLength)

//	Read
//
//	Read some data

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (dwLength == 0)
		return 0;

	if (pData)
		{
		if (dwLength > 0x8000000)
			{
			DWORDLONG dwLeftToRead = dwLength;
			BYTE* pDest = (BYTE*)pData;

			while (dwLeftToRead > 0)
				{
				DWORDLONG dwChunkToRead = Min(dwLeftToRead, (DWORDLONG)0x80000000);
				DWORD dwBytesRead;
				if (!::ReadFile(m_hFile, pDest, (DWORD)dwChunkToRead, &dwBytesRead, NULL) || (dwBytesRead != (DWORD)dwChunkToRead))
					throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_READ, m_sFilespec, ::GetLastError()));

				dwLeftToRead -= dwChunkToRead;
				pDest += dwChunkToRead;
				}

			return dwLength;
			}
		else
			{
			DWORD dwBytesRead;
			if (!::ReadFile(m_hFile, pData, (DWORD)dwLength, &dwBytesRead, NULL) || (dwBytesRead != (DWORD)dwLength))
				throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_READ, m_sFilespec, ::GetLastError()));

			return dwBytesRead;
			}
		}

	//	If we don't want to read, we just advance.

	else
		{
		LARGE_INTEGER liMove;
		liMove.QuadPart = dwLength;

		if (::SetFilePointerEx(m_hFile, liMove, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_SEEK, m_sFilespec, ::GetLastError()));

		return dwLength;
		}
	}

void CFile64::Seek (DWORDLONG dwPos, bool bFromEnd)

//	Seek
//
//	Seek to a given position

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	LARGE_INTEGER liMove;
	liMove.QuadPart = dwPos;

	DWORD dwMethod = (bFromEnd ? FILE_END : FILE_BEGIN);
	if (::SetFilePointerEx(m_hFile, liMove, NULL, dwMethod) == INVALID_SET_FILE_POINTER)
		throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_SEEK, m_sFilespec, ::GetLastError()));
	}

bool CFile64::SetLength (DWORDLONG dwLength)

//	SetLength
//
//	Sets the file length

	{
	LARGE_INTEGER liMove;
	liMove.QuadPart = dwLength;

	if (::SetFilePointerEx(m_hFile, liMove, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;

	if (!::SetEndOfFile(m_hFile))
		return false;

	return true;
	}

bool CFile64::SetModifiedTime (const CDateTime &ModifiedOn)

//	SetModifiedTime
//
//	Sets the modified time.

	{
	SYSTEMTIME SystemTime = ModifiedOn.AsSYSTEMTIME();
	FILETIME ftWrite;
	::SystemTimeToFileTime(&SystemTime, &ftWrite);

	if (!::SetFileTime(m_hFile, NULL, NULL, &ftWrite))
		return false;

	return true;
	}

CString CFile64::TranslateError (DWORD dwError)

//	TranslateError
//
//	Translates to a human-readable message.

	{
	switch (dwError)
		{
		case ERROR_ACCESS_DENIED:
			return strPattern("0x%08d Access denied.", dwError);

		case ERROR_FILE_NOT_FOUND:
			return strPattern("0x%08d File not found.", dwError);

		default:
			return strPattern("0x%08d", dwError);
		}
	}

void CFile64::Unlock (DWORDLONG dwPos, DWORDLONG dwLength)

//	Unlock
//
//	Unlocks a range

	{
	OVERLAPPED overlapped = { 0 };
	overlapped.Offset = static_cast<DWORD>(dwPos & 0xFFFFFFFF);          // Lower 32-bits of the offset
	overlapped.OffsetHigh = static_cast<DWORD>((dwPos >> 32) & 0xFFFFFFFF); // Higher 32-bits of the offset

	DWORD lockLow = static_cast<DWORD>(dwLength & 0xFFFFFFFF);           // Lower 32-bits of the length
	DWORD lockHigh = static_cast<DWORD>((dwLength >> 32) & 0xFFFFFFFF);  // Higher 32-bits of the length

	if (!::UnlockFileEx(m_hFile, 0, lockLow, lockHigh, &overlapped))
		throw CFileException(errFail, m_sFilespec, ::GetLastError(), strPattern(ERR_UNLOCK, m_sFilespec, ::GetLastError()));
	}

void CFile64::Write (const void *pData, DWORDLONG dwLength)

//	Write
//
//	Write some data

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (dwLength == 0)
		return;

	//	If we have data, write it out

	if (pData)
		{
		if (dwLength > 0x80000000)
			{
			DWORDLONG dwLeftToWrite = dwLength;
			BYTE* pSrc = (BYTE*)pData;

			while (dwLeftToWrite > 0)
				{
				DWORDLONG dwChunkToWrite = Min(dwLeftToWrite, (DWORDLONG)0x80000000);
				DWORD dwBytesWritten;
				if (!::WriteFile(m_hFile, pSrc, (DWORD)dwChunkToWrite, &dwBytesWritten, NULL) || (dwBytesWritten != (DWORD)dwChunkToWrite))
					throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_WRITE, m_sFilespec, ::GetLastError()));

				dwLeftToWrite -= dwChunkToWrite;
				pSrc += dwChunkToWrite;
				}
			}
		else
			{
			DWORD dwBytesWritten;
			if (!::WriteFile(m_hFile, pData, (DWORD)dwLength, &dwBytesWritten, NULL) || (dwBytesWritten != (DWORD)dwLength))
				throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_WRITE, m_sFilespec, ::GetLastError()));
			}
		}

	//	Otherwise, we seek forward

	else
		{
		LARGE_INTEGER liMove;
		liMove.QuadPart = dwLength;

		if (::SetFilePointerEx(m_hFile, liMove, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_SEEK, m_sFilespec, ::GetLastError()));
		}
	}
