//	CProcess.cpp
//
//	CProcess class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "Psapi.h"

void CProcess::Create (const CString sCmdLine, const SOptions &Options)

//	Create
//
//	Create a process with the given command-line.

	{
	Close();

	STARTUPINFO StartupInfo;
	utlMemSet(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.hStdInput = Options.hStdIn;
	StartupInfo.hStdOutput = Options.hStdOut;
	StartupInfo.hStdError = Options.hStdError;

	if (Options.hStdError != INVALID_HANDLE_VALUE || Options.hStdIn != INVALID_HANDLE_VALUE || Options.hStdOut != INVALID_HANDLE_VALUE)
		StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

	LPVOID pEnvironment = NULL;
	CBuffer EnvironmentBlock;
	if (Options.EnvironmentVars.GetCount())
		{
		EnvironmentBlock = CreateEnvironmentBlock(Options.EnvironmentVars);
		pEnvironment = (LPVOID)EnvironmentBlock.GetPointer();
		}

	PROCESS_INFORMATION ProcessInfo;
	if (!::CreateProcess(NULL,
			CString16(sCmdLine),
			NULL,
			NULL,
			TRUE,
			0,
			pEnvironment,
			NULL,
			&StartupInfo,
			&ProcessInfo))
		throw CException(errOS, ::GetLastError(), sysGetOSErrorText(::GetLastError()));

	//	Remember the handle

	m_hHandle = ProcessInfo.hProcess;

	//	Close the thread handle (since we don't need it)

	::CloseHandle(ProcessInfo.hThread);
	}

CBuffer CProcess::CreateEnvironmentBlock (const TSortMap<CString, CString> &EnvironmentVars)

//	CreateEnvironmentBlock
//
//	Returns a buffer structured as an environment block.

	{
	constexpr int DEFAULT_SIZE = 4096;
	CBuffer Block(DEFAULT_SIZE);
	for (int i = 0; i < EnvironmentVars.GetCount(); i++)
		{
		const CString &sVar = EnvironmentVars.GetKey(i);
		const CString &sValue = EnvironmentVars[i];

		Block.Write(sVar);
		Block.WriteChar('=');
		Block.Write(sValue);
		Block.WriteChar('\0');
		}

	Block.WriteChar('\0');

	return Block;
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
