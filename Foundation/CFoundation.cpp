//	CFoundation.cpp
//
//	CFoundation class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#if defined(_M_IX86) || defined(_M_X64)
#include <intrin.h>
#endif

DECLARE_CONST_STRING(ERR_CANT_INITIALIZE_COM,			"Unable to initialize COM system.")
DECLARE_CONST_STRING(ERR_CANT_INITIALIZE_WINSOCK,		"Unable to initialize Winsock.")

DECLARE_CONST_STRING(CPU_ARCHITECTURE_ARM,				"arm")
DECLARE_CONST_STRING(CPU_ARCHITECTURE_ARM64,			"arm64")
DECLARE_CONST_STRING(CPU_ARCHITECTURE_IA64,			"ia64")
DECLARE_CONST_STRING(CPU_ARCHITECTURE_UNKNOWN,			"unknown")
DECLARE_CONST_STRING(CPU_ARCHITECTURE_X64,				"x64")
DECLARE_CONST_STRING(CPU_ARCHITECTURE_X86,				"x86")

DECLARE_CONST_STRING(PLATFORM_LINUX,					"linux")
DECLARE_CONST_STRING(PLATFORM_MACOS,					"macos")
DECLARE_CONST_STRING(PLATFORM_UNKNOWN,					"unknown")
DECLARE_CONST_STRING(PLATFORM_WINDOWS,					"windows")

DECLARE_CONST_STRING(REGKEY_CENTRAL_PROCESSOR_0,		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0")
DECLARE_CONST_STRING(REGVALUE_PROCESSOR_NAME,			"ProcessorNameString")

static CFoundation g_Foundation;

static CString GetCPUArchitecture (WORD wArchitecture)
	{
	switch (wArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			return CPU_ARCHITECTURE_X64;

		case PROCESSOR_ARCHITECTURE_ARM:
			return CPU_ARCHITECTURE_ARM;

		case PROCESSOR_ARCHITECTURE_ARM64:
			return CPU_ARCHITECTURE_ARM64;

		case PROCESSOR_ARCHITECTURE_IA64:
			return CPU_ARCHITECTURE_IA64;

		case PROCESSOR_ARCHITECTURE_INTEL:
			return CPU_ARCHITECTURE_X86;

		default:
			return CPU_ARCHITECTURE_UNKNOWN;
		}
	}

static CString TrimWhitespace (const CString& sValue)
	{
	const char* pStart = sValue.GetParsePointer();
	const char* pEnd = pStart + sValue.GetLength();

	while (pStart < pEnd && strIsWhitespace(pStart))
		pStart++;

	while (pEnd > pStart && strIsWhitespace(pEnd - 1))
		pEnd--;

	return CString(pStart, (int)(pEnd - pStart));
	}

static CString GetCPUModelFromRegistry ()
	{
	WCHAR szModel[256];
	DWORD dwType = 0;
	DWORD dwSize = sizeof(szModel);

	LSTATUS iResult = ::RegGetValueW(
			HKEY_LOCAL_MACHINE,
			CString16(REGKEY_CENTRAL_PROCESSOR_0),
			CString16(REGVALUE_PROCESSOR_NAME),
			RRF_RT_REG_SZ,
			&dwType,
			szModel,
			&dwSize);

	if (iResult != ERROR_SUCCESS || dwType != REG_SZ || dwSize == 0)
		return NULL_STR;

	return TrimWhitespace(CString(CString16(szModel)));
	}

static CString GetCPUModelFromCPUID ()
	{
#if defined(_M_IX86) || defined(_M_X64)
	int Info[4] = {};
	__cpuid(Info, 0x80000000);

	if ((DWORD)Info[0] < 0x80000004)
		return NULL_STR;

	char szBrand[0x40] = {};
	__cpuid((int *)(szBrand + 0), 0x80000002);
	__cpuid((int *)(szBrand + 16), 0x80000003);
	__cpuid((int *)(szBrand + 32), 0x80000004);

	return TrimWhitespace(CString(szBrand));
#else
	return NULL_STR;
#endif
	}

static CString GetCPUModel (void)
	{
	CString sModel = GetCPUModelFromRegistry();
	if (!sModel.IsEmpty())
		return sModel;

	return GetCPUModelFromCPUID();
	}

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
	::GetNativeSystemInfo(&SI);

	SCPUInfo Info;
	Info.sArchitecture = GetCPUArchitecture(SI.wProcessorArchitecture);
	Info.sModel = GetCPUModel();
	if (Info.sModel.IsEmpty())
		Info.sModel = Info.sArchitecture;

	Info.iLogicalProcessorCount = SI.dwNumberOfProcessors;
	return Info;
	}

const CString& CFoundation::GetPlatform ()

//	GetPlatform
//
//	Returns the current operating system platform.

	{
#if defined(_WIN32)
	return PLATFORM_WINDOWS;
#elif defined(__APPLE__)
	return PLATFORM_MACOS;
#elif defined(__linux__)
	return PLATFORM_LINUX;
#else
	return PLATFORM_UNKNOWN;
#endif
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
