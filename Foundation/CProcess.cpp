//	CProcess.cpp
//
//	CProcess class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include "Psapi.h"

void CProcess::Create (const CString sCmdLine)

//	Create
//
//	Create a process with the given command-line.

	{
	Close();

	STARTUPINFO StartupInfo;
	utlMemSet(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);

	PROCESS_INFORMATION ProcessInfo;
	if (!::CreateProcess(NULL,
			CString16(sCmdLine),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&StartupInfo,
			&ProcessInfo))
		throw CException(errOS, ::GetLastError(), sysGetOSErrorText(::GetLastError()));

	//	Remember the handle

	m_hHandle = ProcessInfo.hProcess;

	//	Close the thread handle (since we don't need it)

	::CloseHandle(ProcessInfo.hThread);
	}

CString CProcess::GetExecutableFilespec (void) const

//	GetExecutableFilespec
//
//	Returns full path of the process.

	{
	if (m_hHandle == INVALID_HANDLE_VALUE)
		return NULL_STR;

	TCHAR szBuffer[MAX_FILE_PATH_CHARS];
	int iLen = ::GetModuleFileNameEx(m_hHandle, NULL, szBuffer, MAX_FILE_PATH_CHARS);

	//	Convert to UTF-8

	return CString(szBuffer, iLen);
	}

bool CProcess::GetMemoryInfo (SMemoryInfo *retInfo) const

//	GetMemoryInfo
//
//	Returns memory info for the process.

	{
	PROCESS_MEMORY_COUNTERS_EX Counters;
	if (!::K32GetProcessMemoryInfo(m_hHandle, (PPROCESS_MEMORY_COUNTERS)&Counters, sizeof(Counters)))
		return false;

	retInfo->dwCurrentAlloc = Counters.PrivateUsage;
	retInfo->dwCurrentReserved = Counters.WorkingSetSize;
	retInfo->dwPeakReserved = Counters.PeakWorkingSetSize;

	return true;
	}
