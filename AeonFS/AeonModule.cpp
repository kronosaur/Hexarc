//	AeonFS Arcology Module
//
//	Implements the Aeon File System
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

struct SOptions
	{
	CString sMachineName;
	bool bDebug = false;
	};

DECLARE_CONST_STRING(STR_DEBUG_SWITCH,					"/debug")
DECLARE_CONST_STRING(STR_MACHINE_NAME_SWITCH,			"/machine:")

BOOL WINAPI CtrlHandler (DWORD dwCtrlType);
void ParseCommandLine (int argc, char *argv[], SOptions *retOptions);

static HANDLE g_hMainThread = INVALID_HANDLE_VALUE;
static CArchonProcess *g_pModule = NULL;

int main (int argc, char* argv[])
	{
	CArchonProcess Module;
	g_pModule = &Module;

	//	We expect to be started from Arcology.exe, so if we are run without
	//	parameters then we just show help and exit.

	if (argc == 1)
		{
		printf("AeonModule: This program is a module of Arcology.exe.\n");
		return 1;
		}

	//	Set a close handler

#ifdef DEBUG_CTRL_HANDLER
	g_hMainThread = ::GetCurrentThread();
	::SetConsoleCtrlHandler(CtrlHandler, TRUE);
#endif

	//	See if we have the debug flag

	SOptions Options;
	ParseCommandLine(argc, argv, &Options);

	//	Boot the module

	SProcessDesc Config;
	Config.sMachineName = Options.sMachineName;
	Config.dwFlags = 0;
	Config.dwFlags |= (Options.bDebug ? PROCESS_FLAG_DEBUG : 0);

	//	Add the Aeon engine

	SEngineDesc *pDesc = Config.Engines.Insert();
	CAeonFSEngine *pEngine = new CAeonFSEngine;
	pDesc->pEngine = pEngine;

	//	Boot

	Module.Boot(Config);

	//	Run

	Module.Run();

	return 0;
	}

BOOL WINAPI CtrlHandler (DWORD dwCtrlType)

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
//			g_pModule->SignalShutdown();
			::WaitForSingleObject(g_hMainThread, INFINITE);
            return TRUE;
	    }

    return FALSE;
	}

void ParseCommandLine (int argc, char *argv[], SOptions *retOptions)
	{
	//	Look for switches

	for (int i = 1; i < argc; i++)
		{
		CString sArg(argv[i]);

		if (strEquals(sArg, STR_DEBUG_SWITCH))
			retOptions->bDebug = true;
		else if (strStartsWith(sArg, STR_MACHINE_NAME_SWITCH))
			retOptions->sMachineName = strSubString(sArg, STR_MACHINE_NAME_SWITCH.GetLength());
		}
	}
