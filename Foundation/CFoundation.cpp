//	CFoundation.cpp
//
//	CFoundation class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

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
		if (hr)
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
