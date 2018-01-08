//	CFileBuffer64.cpp
//
//	CFileBuffer64 class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CANT_OPEN_FILE,				"Unable to open file: %s. [%x]")

CFileBuffer64::CFileBuffer64 (void) : m_hFile(INVALID_HANDLE_VALUE)

//	CFileBuffer64 constructor

	{
	}

CFileBuffer64::~CFileBuffer64 (void)

//	CFileBuffer destructor

	{
	Close();
	}

void CFileBuffer64::Close (void)

//	Close
//
//	Close the file

	{
	if (m_hFile != INVALID_HANDLE_VALUE)
		{
		::UnmapViewOfFile(m_pBlock);
		::CloseHandle(m_hFileMap);
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		}
	}

CDateTime CFileBuffer64::GetModifiedTime (void)

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

bool CFileBuffer64::Open (const CString &sFilespec, DWORDLONG dwStart, DWORDLONG dwLength, DWORD dwFlags, CString *retsError)

//	Open
//
//	Opens the file

	{
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_sFilespec = sFilespec;

		DWORD dwShareFlags = 0;

		m_hFile = ::CreateFile(CString16(m_sFilespec),
				GENERIC_READ,
				((dwFlags & FILE_SHARE_WRITE) ? (FILE_SHARE_WRITE | FILE_SHARE_READ) : FILE_SHARE_READ),
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			{
			if (retsError)
				*retsError = strPattern(ERR_CANT_OPEN_FILE, sFilespec, ::GetLastError());

			return false;
			}

		//	Open a file mapping

		m_hFileMap = ::CreateFileMapping(m_hFile,
				NULL,
				PAGE_READONLY,
				0,
				0,
				NULL);
		if (m_hFileMap == INVALID_HANDLE_VALUE)
			{
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;

			if (retsError)
				*retsError = strPattern(ERR_CANT_OPEN_FILE, sFilespec, ::GetLastError());

			return false;
			}

		//	Map a view of the file

		m_pBlock = (char *)::MapViewOfFile(m_hFileMap,
				FILE_MAP_READ,
				HIDWORD(dwStart),
				LODWORD(dwStart),
				dwLength);
		if (m_pBlock == NULL)
			{
			::CloseHandle(m_hFileMap);
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;

			if (retsError)
				*retsError = strPattern(ERR_CANT_OPEN_FILE, sFilespec, ::GetLastError());

			return false;
			}

		//	Figure out the size of the file

		DWORD dwHigh;
		DWORD dwLow = ::GetFileSize(m_hFile, &dwHigh);
		m_dwMaxSize = (dwLength > 0 ? dwLength : MAKEDWORDLONG(dwLow, dwHigh) - dwStart);

		m_dwCommittedSize = m_dwMaxSize;
		m_dwCurrentSize = m_dwMaxSize;
		}

	//	Done

	return true;
	}
