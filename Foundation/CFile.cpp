//	CFile.cpp
//
//	CFile class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CANT_OPEN_FILE,				"Unable to open file: %s. [%x]")
DECLARE_CONST_STRING(ERR_READ,							"Error reading file: %s. [%x]")
DECLARE_CONST_STRING(ERR_SEEK,							"Error seeking in file: %s. [%x]")
DECLARE_CONST_STRING(ERR_UNLOCK,						"Error unlocking file: %s. [%x]")
DECLARE_CONST_STRING(ERR_WRITE,							"Error writing file: %s. [%x]")

CFile::~CFile (void)

//	CFile destructor

	{
	Close();
	}

void CFile::Close (void)

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

bool CFile::Create (const CString &sFilespec, DWORD dwFlags, CString *retsError)

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

bool CFile::Flush (void)

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

CDateTime CFile::GetModifiedTime (void)

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

int CFile::GetPos (void)

//	GetPos
//
//	Get current file position

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	DWORD dwLowPos = ::SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	return dwLowPos;
	}

DWORDLONG CFile::GetSize (void)

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

int CFile::GetStreamLength (void)

//	GetStreamLength
//
//	Get current stream length

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	DWORD dwLowSize = ::GetFileSize(m_hFile, NULL);
	return dwLowSize;
	}

bool CFile::Lock (int iPos, int iLength, int iTimeout)

//	Lock
//
//	Locks a region of the file and returns TRUE if the lock succeeded.

	{
	DWORD dwStartTime = sysGetTickCount();
	while (true)
		{
		//	Try to lock. If we succeed, then we're done

		if (::LockFile(m_hFile, (DWORD)iPos, 0, (DWORD)iLength, 0))
			return true;

		//	If we fail, and we've exceeded the timeout, we exit.

		if (iTimeout == 0 || sysGetTicksElapsed(dwStartTime) > (DWORD)iTimeout)
			return false;

		//	Otherwise, we wait a bit and try again.

		::Sleep(iTimeout / 10);
		}
	}

int CFile::Read (void *pData, int iLength)

//	Read
//
//	Read some data

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (iLength == 0)
		return 0;

	DWORD dwBytesRead;
	if (!::ReadFile(m_hFile, pData, iLength, &dwBytesRead, NULL) || (dwBytesRead != (DWORD)iLength))
		throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_READ, m_sFilespec, ::GetLastError()));

	return dwBytesRead;
	}

void CFile::Seek (int iPos, bool bFromEnd)

//	Seek
//
//	Seek to a given position

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	DWORD dwMethod = (bFromEnd ? FILE_END : FILE_BEGIN);
	if (::SetFilePointer(m_hFile, iPos, NULL, dwMethod) == INVALID_SET_FILE_POINTER)
		throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_SEEK, m_sFilespec, ::GetLastError()));
	}

bool CFile::SetLength (int iLength)

//	SetLength
//
//	Sets the file length

	{
	if (::SetFilePointer(m_hFile, iLength, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;

	if (!::SetEndOfFile(m_hFile))
		return false;

	return true;
	}

bool CFile::SetModifiedTime (const CDateTime &ModifiedOn)

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

CString CFile::TranslateError (DWORD dwError)

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

void CFile::Unlock (int iPos, int iLength)

//	Unlock
//
//	Unlocks a range

	{
	if (!::UnlockFile(m_hFile, (DWORD)iPos, 0, (DWORD)iLength, 0))
		throw CFileException(errFail, m_sFilespec, ::GetLastError(), strPattern(ERR_UNLOCK, m_sFilespec, ::GetLastError()));
	}

int CFile::Write (const void *pData, int iLength)

//	Write
//
//	Write some data

	{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (iLength == 0)
		return 0;

	//	If we have data, write it out

	if (pData)
		{
		DWORD dwBytesWritten;
		if (!::WriteFile(m_hFile, pData, iLength, &dwBytesWritten, NULL) || (dwBytesWritten != (DWORD)iLength))
			throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_WRITE, m_sFilespec, ::GetLastError()));

		return dwBytesWritten;
		}

	//	Otherwise, we seek forward

	else
		{
		if (::SetFilePointer(m_hFile, iLength, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			throw CFileException(errDisk, m_sFilespec, ::GetLastError(), strPattern(ERR_SEEK, m_sFilespec, ::GetLastError()));

		return iLength;
		}
	}
