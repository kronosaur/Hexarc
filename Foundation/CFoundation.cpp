//	CFoundation.cpp
//
//	CFoundation class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CANT_INITIALIZE_COM,			"Unable to initialize COM system.")
DECLARE_CONST_STRING(ERR_CANT_INITIALIZE_WINSOCK,		"Unable to initialize Winsock.")

static CFoundation g_Foundation;

CFoundation::CFoundation (void) :
		m_bInitialized(false)

//	CFoundation constructor

	{
	}

CFoundation::~CFoundation (void)

//	CFoundation destructor

	{
	Shutdown();
	}

bool CFoundation::Boot (DWORD dwFlags, CString *retsError)

//	Boot
//
//	Must be called first. Returns FALSE if we could not initialized.

	{
	return g_Foundation.Startup(dwFlags, retsError);
	}

CFoundation::SCPUInfo CFoundation::GetCPUInfo ()

//	GetCPUInfo
//
//	Returns CPU info.

	{
	SYSTEM_INFO SI;
	::GetSystemInfo(&SI);

	SCPUInfo Info;
	Info.iLogicalProcessorCount = SI.dwNumberOfProcessors;
	return Info;
	}

void CFoundation::Shutdown (void)

//	Shutdown
//
//	Clean up

	{
	if (!m_bInitialized)
		return;

	WSACleanup();

	if (m_bCOMInitialized)
		::CoUninitialize();

	m_bInitialized = false;
	}

bool CFoundation::Startup (DWORD dwFlags, CString *retsError)

//	Startup
//
//	Initialize

	{
	if (m_bInitialized)
		return true;

	//	Initialize COM

	if (dwFlags & BOOT_FLAG_COM)
		{
		HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (hr != S_OK)
			{
			if (retsError) *retsError = ERR_CANT_INITIALIZE_COM;
			return false;
			}

		m_bCOMInitialized = true;
		}

	//	Initialize winsock

	WORD wVersionRequested = 0x0202;		//	Version 2.2
	WSADATA wsaData;
	if (WSAStartup(wVersionRequested, &wsaData))
		{
		if (retsError) *retsError = ERR_CANT_INITIALIZE_WINSOCK;
		return false;
		}

	//	Success!

	m_bInitialized = true;
	return true;
	}

void CFoundation::DebugTest_TIDTable ()
	{
	int iMaxCount = 0;
	TIDTable<DWORDLONG> IDTable;
	TSortMap<DWORD, bool> IDsInUse;
	for (int i = 0; i < 1000000; i++)
		{
		if (IDsInUse.GetCount() == 0 || mathRandom(1, 100) <= 50)
			{
			DWORD dwID;
			DWORDLONG* pPayload = IDTable.Insert(&dwID);
			*pPayload = dwID;

			bool bNew;
			IDsInUse.SetAt(dwID, true, &bNew);
			if (!bNew)
				{
				printf("ERROR: Duplicate ID.\n");
				throw CException(errFail);
				}

			if (IDsInUse.GetCount() > iMaxCount)
				iMaxCount = IDsInUse.GetCount();
			}
		else
			{
			DWORD dwIDToDelete = IDsInUse.GetKey(mathRandom(0, IDsInUse.GetCount() - 1));
			DWORDLONG dwPayload = IDTable.GetAt(dwIDToDelete);
			if ((DWORD)dwPayload != dwIDToDelete)
				throw CException(errFail);

			IDTable.Delete(dwIDToDelete);
			IDsInUse.DeleteAt(dwIDToDelete);
			}

		if (((i + 1) % 1000) == 0)
			printf("%d\n", i + 1);
		}

	int iCount = 0;
	SIDTableEnumerator i;
	IDTable.Reset(i);
	while (IDTable.HasMore(i))
		{
		DWORDLONG dwPayload = IDTable.GetNext(i);
		if (!IDsInUse.GetAt((DWORD)dwPayload))
			{
			throw CException(errFail);
			}

		iCount++;
		}

	if (iCount != IDsInUse.GetCount())
		throw CException(errFail);

	printf("Done. Max count = %d\n", iMaxCount);
	}
