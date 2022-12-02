//	Win32Service.cpp
//
//	Implements functions required for a Win32 service
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void PrintHelp (void);

CWin32Service *g_pService = NULL;

int main (int argc, char **argv)

//	main
//
//	Main program entrypoint.

	{
	//	Create the service object.

	g_pService = ::CreateWin32Service();
	if (g_pService == NULL)
		{
		printf("Unable to create service.\n");
		return 1;
		}

	//	Go

	g_pService->Execute(argc, argv);

	//	Done

	g_pService->DestroyWin32Service();
	g_pService = NULL;

	return 0;
	}

void PrintHelp (void)
	{
	CString sFilename = fileGetFilename(fileGetExecutableFilespec());

	printf("%s /install          to install the service\n", (LPSTR)sFilename);
	printf("%s /remove           to remove the service\n", (LPSTR)sFilename);
	printf("%s /debug            to run as a console app\n", (LPSTR)sFilename);
	}

//	CWin32Service --------------------------------------------------------------

CWin32Service::CWin32Service (void) :
		m_bServiceDebugMode(false)

//	CWin32Service constructor

	{
	}

BOOL WINAPI CWin32Service::CtrlHandler (DWORD dwCtrlType)

//	CtrlHandler
//
//	Handle Ctrl+Break in debug console

	{
    switch (dwCtrlType)
	    {
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
        case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
        case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
			printf("Stopping %s.\n", (LPSTR)CString(g_pService->m_sServiceName));
			if (g_pService)
				g_pService->OnStop();

			if (g_pService)
				g_pService->OnWaitForStop();
            return TRUE;
	    }

    return FALSE;
	}

void CWin32Service::DebugRun (void)

//	DebugRun
//
//	Run the service at the console

	{
	m_bServiceDebugMode = true;

	//	Start

	OnStart(m_Params);

	//	Set the Ctrl+C handler

	::SetConsoleCtrlHandler(CtrlHandler, TRUE);

	//	Run

	OnRun();

	//	Done

	m_bServiceDebugMode = false;
	}

void CWin32Service::Execute (int argc, char **argv)

//	Execute
//
//	Execute based on our parameters

	{
	//	Initialize

	m_hStatus = NULL;

	//	Ask our descendant for service info

	SWin32ServiceInfo Info;
	GetInfo(&Info);

	m_sServiceID = Info.sServiceID;
	m_sServiceName = Info.sServiceName;
	m_sServiceDesc = Info.sServiceDesc;

	//	Parse the command line and figure out what our main command is. All
	//	other parameters are kept in m_Params for later.

	ECommandTypes iCmd = ParseCommandLine(argc, argv, &m_Params);

	//	Do the right thing

	switch (iCmd)
		{
		case cmdDebug:
			DebugRun();
			break;

		case cmdInstall:
			Install();
			break;

		case cmdRemove:
			Remove();
			break;

		case cmdRestart:
			Restart();
			break;

		case cmdRun:
			Run();
			break;

		default:
			PrintHelp();
			break;
		}
	}

void CWin32Service::Install (void)

//	Install
//
//	Installs the service to the registry

	{
	int i;

    SC_HANDLE hService;
    SC_HANDLE hSCManager;

	//	Generate the executable path, adding parameters as appropriate

	CString sFilespec = strPattern("%s /run", (LPSTR)::fileGetExecutableFilespec());
	for (i = 0; i < m_Params.GetCount(); i++)
		sFilespec += strPattern(" %s", m_Params[i]);

	//	Open the Service Control Manager

	hSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
		{
		printf("Unable to install service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	//	Create the service

	hService = ::CreateService(hSCManager,
			m_sServiceID,
			m_sServiceName,
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_NORMAL,
			CString16(sFilespec),
			NULL,
			NULL,
			_T(""),		//	Dependencies
			NULL,
			NULL);
	if (!hService)
		{
		::CloseServiceHandle(hSCManager);
		printf("Unable to install service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	//	Set the service description

	SERVICE_DESCRIPTION ServiceDesc;
	ServiceDesc.lpDescription = m_sServiceDesc;
	::ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &ServiceDesc);

	//	Done

	printf("%s installed.\n", (LPSTR)CString(m_sServiceName));
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCManager);
	}

CWin32Service::ECommandTypes CWin32Service::ParseCommandLine (int argc, char **argv, TArray<CString> *retParams)
	{
	int i;
	ECommandTypes iCmd = cmdHelp;

	//	Get all parameters and keep track of commands we recognize.

	for (i = 1; i < argc; i++)
		{
		CString sParam = CString(argv[i]);

		char *pPos = sParam.GetParsePointer();
		if (*pPos == '-' || *pPos == '/')
			{
			if ( _stricmp( "install", pPos+1 ) == 0 )
				iCmd = cmdInstall;
			else if ( _stricmp( "remove", pPos+1 ) == 0 )
				iCmd = cmdRemove;
			else if ( _stricmp( "restart", pPos+1 ) == 0 )
				iCmd = cmdRestart;
			else if ( _stricmp( "debug", pPos+1 ) == 0 )
				iCmd = cmdDebug;
			else if ( _stricmp( "run", pPos+1 ) == 0 )
				iCmd = cmdRun;

			//	If not a known command, then add it to our parameter list

			else
				retParams->Insert(sParam);
			}
		else
			retParams->Insert(sParam);
		}

	return iCmd;
	}

void CWin32Service::Remove (void)

//	Remove
//
//	Remove the service from the registry

	{
    SC_HANDLE hService;
    SC_HANDLE hSCManager;

	//	Open the Service Control Manager

	hSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
		{
		printf("Unable to uninstall service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	//	Open the service

	hService = ::OpenService(hSCManager, m_sServiceID, SERVICE_ALL_ACCESS);
	if (!hService)
		{
		::CloseServiceHandle(hSCManager);
		printf("Unable to uninstall service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	//	Try to stop the service

	SERVICE_STATUS Status;
	if (::ControlService(hService, SERVICE_CONTROL_STOP, &Status))
		{
		//	Wait until service has stopped

		printf("Stopping %s.", (LPSTR)CString(m_sServiceName));
		::Sleep(1000);
		while (::QueryServiceStatus(hService, &Status))
			{
			if (Status.dwCurrentState == SERVICE_STOP_PENDING)
				{
				printf(".");
				::Sleep(1000);
				}
			else
				break;
			}

		if (Status.dwCurrentState != SERVICE_STOPPED)
			{
			::CloseServiceHandle(hService);
			::CloseServiceHandle(hSCManager);
			printf("\nUnable to stop service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
			return;
			}

		printf("\nService stopped.\n");
		}

	//	Remove the service

	if (!::DeleteService(hService))
		{
		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCManager);
		printf("Unable to uninstall service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	//	Done

	printf("%s uninstalled.\n", (LPSTR)CString(m_sServiceName));
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCManager);
	}

void CWin32Service::ReportErrorEvent (LPCSTR pMsg, DWORD dwError)

//	ReportErrorEvent
//
//	Reports an error to the OS event log

	{
	constexpr int BUFFER_SIZE = 1024;

	if (dwError == 0)
		dwError = ::GetLastError();

	//	Register an event source

	HANDLE hEventSource = ::RegisterEventSource(NULL, m_sServiceID);
	if (hEventSource == NULL)
		return;

	//	Compose some error message strings

	TCHAR szMsg1[BUFFER_SIZE];
	swprintf_s(szMsg1, BUFFER_SIZE, TEXT("%s error: %d"), (LPTSTR)m_sServiceName, dwError);

	CString16 sMsg2(pMsg);

	LPCTSTR pStrings[2];
	pStrings[0] = szMsg1;
	pStrings[1] = sMsg2;

	//	Report

	::ReportEvent(hEventSource,
			EVENTLOG_ERROR_TYPE,
			0,
			0,
			NULL,
			2,
			0,
			pStrings,
			NULL);

	//	Done

	::DeregisterEventSource(hEventSource);
	}

bool CWin32Service::ReportStatus (DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)

//	ReportStatus
//
//	Reports service status to controller

	{
	static DWORD dwCheckPoint = 1;

	//	Initialize status

	if (dwCurrentState == SERVICE_START_PENDING)
		m_Status.dwControlsAccepted = 0;
	else
		m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	m_Status.dwCurrentState = dwCurrentState;
	m_Status.dwWin32ExitCode = dwWin32ExitCode;
	m_Status.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
		m_Status.dwCheckPoint = 0;
	else
		m_Status.dwCheckPoint = dwCheckPoint++;

	//	Report

	if (!::SetServiceStatus(m_hStatus, &m_Status))
		{
		ReportErrorEvent("SetServiceStatus failed");
		return false;
		}

	return true;
	}

void CWin32Service::Restart (void)

//	Restart
//
//	Restarts the service

	{
    SC_HANDLE hService;
    SC_HANDLE hSCManager;

	//	Open the Service Control Manager

	hSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
		{
		printf("Unable to open service manager: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	//	Open the service

	hService = ::OpenService(hSCManager, m_sServiceID, SERVICE_ALL_ACCESS);
	if (!hService)
		{
		::CloseServiceHandle(hSCManager);
		printf("Unable to open service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	//	Try to stop the service

	SERVICE_STATUS Status;
	if (::ControlService(hService, SERVICE_CONTROL_STOP, &Status))
		{
		//	Wait until service has stopped

		printf("Stopping %s.", (LPSTR)CString(m_sServiceName));
		::Sleep(1000);
		while (::QueryServiceStatus(hService, &Status))
			{
			if (Status.dwCurrentState == SERVICE_STOP_PENDING)
				{
				printf(".");
				::Sleep(1000);
				}
			else
				break;
			}

		if (Status.dwCurrentState != SERVICE_STOPPED)
			{
			::CloseServiceHandle(hService);
			::CloseServiceHandle(hSCManager);
			printf("\nUnable to stop service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
			return;
			}

		printf("\nService stopped.\n");
		}

	//	Try to restart the service

	if (!::StartService(hService, 0, NULL))
		{
		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCManager);
		printf("Unable to start service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}

	printf("Service start pending.\n");

	// Check the status until the service is no longer start pending. 
 
	SERVICE_STATUS_PROCESS StatusProcess;
	DWORD dwBytesNeeded;
	if (!::QueryServiceStatusEx(hService,
            SC_STATUS_PROCESS_INFO,
            (BYTE *)&StatusProcess,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded))
		{
		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCManager);
		printf("Unable to start service: %s\n", (LPSTR)sysGetOSErrorText(GetLastError()));
		return;
		}
 
    //	Save the tick count and initial checkpoint.

    DWORD dwStartTickCount = GetTickCount();
    DWORD dwOldCheckPoint = StatusProcess.dwCheckPoint;

    while (StatusProcess.dwCurrentState == SERVICE_START_PENDING) 
		{ 
        // Do not wait longer than the wait hint. A good interval is 
        // one tenth the wait hint, but no less than 1 second and no 
        // more than 10 seconds. 
 
        DWORD dwWaitTime = StatusProcess.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

		::Sleep(dwWaitTime);

        // Check the status again. 
 
		if (!::QueryServiceStatusEx(hService,
				SC_STATUS_PROCESS_INFO,
				(BYTE *)&StatusProcess,
				sizeof(SERVICE_STATUS_PROCESS),
				&dwBytesNeeded))
			break;
	 
		if (StatusProcess.dwCheckPoint > dwOldCheckPoint)
			{
			// The service is making progress.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = StatusProcess.dwCheckPoint;
			}
		else
			{
			if (GetTickCount() - dwStartTickCount > StatusProcess.dwWaitHint)
				{
				// No progress made within the wait hint
				break;
				}
			}
		}

	::CloseServiceHandle(hService); 
	::CloseServiceHandle(hSCManager);

    if (StatusProcess.dwCurrentState != SERVICE_RUNNING) 
		{ 
        printf("Service not started.\n");
        printf("  Current State: %d\n", StatusProcess.dwCurrentState); 
        printf("  Exit Code: %d\n", StatusProcess.dwWin32ExitCode); 
        printf("  Service Specific Exit Code: %d\n", 
            StatusProcess.dwServiceSpecificExitCode); 
        printf("  Check Point: %d\n", StatusProcess.dwCheckPoint); 
        printf("  Wait Hint: %d\n", StatusProcess.dwWaitHint); 
        return;
	    }

	printf("Service started.\n"); 
	}

void CWin32Service::Run ()

//	Run
//
//	Run the service through service manager

	{
	//	Initialize service info

    SERVICE_TABLE_ENTRY dispatchTable[2];
	dispatchTable[0].lpServiceName = m_sServiceID;
	dispatchTable[0].lpServiceProc = ServiceMain;
	dispatchTable[1].lpServiceName = NULL;
	dispatchTable[1].lpServiceProc = NULL;

	//	Start the service

	if (!::StartServiceCtrlDispatcher(dispatchTable))
		ReportErrorEvent("StartServiceCtrlDispatcher failed.");
	}

void WINAPI CWin32Service::ServiceControl (DWORD dwCtrlCode)

//	ServiceControl
//
//	Handle requested control code

	{
	switch (dwCtrlCode)
		{
		case SERVICE_CONTROL_STOP:
			g_pService->ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
			g_pService->OnStop();
			break;

		case SERVICE_CONTROL_INTERROGATE:
			g_pService->ReportStatus(g_pService->m_Status.dwCurrentState, NO_ERROR, 0);
			break;
		}
	}

void WINAPI CWin32Service::ServiceMain (DWORD dwArgc, LPTSTR *pArgv)
	{
	//	Register service control handler

	g_pService->m_hStatus = ::RegisterServiceCtrlHandler(g_pService->m_sServiceID, ServiceControl);
	if (!g_pService->m_hStatus)
		{
		g_pService->ReportErrorEvent("Unable to register service control handler.");
		return;
		}

	//	Initialize status

	g_pService->m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_pService->m_Status.dwServiceSpecificExitCode = 0;

	//	Report status

	if (!g_pService->ReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000))
		return;

	//	Start

	g_pService->OnStart(g_pService->m_Params);

	//	We're running

	if (!g_pService->ReportStatus(SERVICE_RUNNING, NO_ERROR, 0))
		return;

	//	Run

	g_pService->OnRun();

	//	When OnStop is called, OnRun will exit. Now we clean up

	g_pService->ReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
