//	AeonDb Arcology Module
//
//	Implements the Aeon Database
//	Copyright (c) 2011-2012 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

struct SOptions
	{
	CString sMachineName;
	bool bDebug = false;

	bool bConsole = false;					//	Run in console (isolated) mode
	CString sStoragePath;					//	Optional storage path if in console mode
	};

DECLARE_CONST_STRING(PATH_DEFAULT_STORAGE,				"c:\\ArcologyConsole")

DECLARE_CONST_STRING(STR_CONSOLE_SWITCH,				"/console")
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
		printf("AeonModule: This program is a module of Arcology.exe. To run in console mode, try:\n\n");

		printf("aeonDB /console [storage-path]\n");
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
	Config.dwFlags |= (Options.bConsole ? PROCESS_FLAG_CONSOLE_MODE : 0);

	//	Add the Aeon engine

	SEngineDesc *pDesc = Config.Engines.Insert();
	CAeonEngine *pEngine = new CAeonEngine;
	pDesc->pEngine = pEngine;

	//	If we're in console mode, then prepare the engine appropriately.

	if (Options.bConsole)
		{
		CString sStorage = Options.sStoragePath;
		if (sStorage.IsEmpty())
			sStorage = PATH_DEFAULT_STORAGE;

		pEngine->SetConsoleMode(sStorage);
		}

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
	int i;

	//	Look for switches

	for (i = 1; i < argc; i++)
		{
		CString sArg(argv[i]);

		if (strEquals(sArg, STR_CONSOLE_SWITCH))
			retOptions->bConsole = true;
		else if (strEquals(sArg, STR_DEBUG_SWITCH))
			retOptions->bDebug = true;
		else if (strStartsWith(sArg, STR_MACHINE_NAME_SWITCH))
			retOptions->sMachineName = strSubString(sArg, STR_MACHINE_NAME_SWITCH.GetLength());

		//	Otherwise, we assume this is a storage path. This is only valid for
		//	console mode.

		else
			retOptions->sStoragePath = sArg;
		}
	}
