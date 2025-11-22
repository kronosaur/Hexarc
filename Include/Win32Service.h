//	Win32Service.h
//
//	Functions and classes for a Win32 Service
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Win32Service provides classes and entrypoints for implementing
//	a Win32 service.
//
//	1. Include Foundation.h
//	2. Include Win32Service.h
//	3. Define and implement class that derives from CWin32Service.
//		3a.	DestroyWin32Service should undo CreateWin32Service's allocation.
//		3b.	GetInfo must return a valid service name and description.
//		3c. OnStart should optionally execute code before steady-state run.
//		3d.	OnRun should run until OnStop is called.
//		3e.	OnStop will be called on a different thread. It should signal OnRun to stop.
//	4. Implement CreateWin32Service to return a new instance of your class.
//	5. Link with Win32Service.lib

#pragma once

struct SWin32ServiceInfo
	{
	CString sServiceID;
	CString sServiceName;
	CString sServiceDesc;
	};

class CWin32Service
	{
	public:
		CWin32Service (void);

		//	Virtual functions
		//	Descendant must override these functions
		virtual ~CWin32Service (void) { }
		virtual void DestroyWin32Service (void) { delete this; }

		virtual void GetInfo (SWin32ServiceInfo *retInfo) { }
		virtual void OnCrash (CStringView sError) { }
		virtual void OnRun (void) { }
		virtual void OnStart (const TArray<CString> &Params) { }
		virtual void OnStop (void) { }
		virtual void OnWaitForStop (void) { }

		//	Internal functions
		//	Descendants must not call these functions
		//	(They cannot be private because they are called from the
		//	main application function).

		void Execute (int argc, char **argv);

		static void BootHandlers ();

	protected:
		//	Helper functions
		//	Descendants may call these functions

		inline bool InServiceDebugMode (void) const { return m_bServiceDebugMode; }
		void ReportErrorEvent (LPCSTR pMsg, DWORD dwError = 0);

	private:
		enum ECommandTypes
			{
			cmdDebug,
			cmdHelp,
			cmdInstall,
			cmdRemove,
			cmdRun,
			cmdRestart,
			};

		void DebugRun (void);
		void Install (void);
		ECommandTypes ParseCommandLine (int argc, char **argv, TArray<CString> *retParams);
		void Remove (void);
		void Run (void);
		bool ReportStatus (DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
		void Restart (void);

		static void __cdecl AbortHandler (int sig);
		static BOOL WINAPI CtrlHandler (DWORD dwCtrlType);
		static LONG WINAPI ExceptionHandler (PEXCEPTION_POINTERS ep);
		static void WINAPI ServiceControl (DWORD dwCtrlCode);
		static void WINAPI ServiceMain (DWORD dwArgc, LPTSTR *pArgv);

		CString16 m_sServiceID;
		CString16 m_sServiceName;
		CString16 m_sServiceDesc;

		SERVICE_STATUS_HANDLE m_hStatus;
		SERVICE_STATUS m_Status;

		TArray<CString> m_Params;			//	Parameters we were called with
		bool m_bServiceDebugMode;			//	TRUE if we're running with /debug
	};

CWin32Service *CreateWin32Service (void);