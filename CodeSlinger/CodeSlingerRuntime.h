//	CodeSlingerRuntime.h
//
//	CodeSlinger
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CCodeSlingerEngine;
class CPortSet;
class IProgramInstance;

typedef DWORDLONG SequenceNumber;

class IProgramPort : public TRefCounted<IProgramPort>
	{
	public:
		IProgramPort (CPortSet &PortSet, const CString &sID) :
				m_PortSet(PortSet),
				m_sID(sID)
			{ }

		virtual ~IProgramPort () { }
		const CString &GetID () const { return m_sID; }
		CDatum GetView (SequenceNumber Seq) const { return OnGetView(Seq); }
		void Mark () { OnMark(); }
		void Output (CDatum dValue) { OnOutput(dValue); }

	protected:
		virtual CDatum OnGetView (SequenceNumber Seq) const { return CDatum(); }
		virtual void OnMark () { }
		virtual void OnOutput (CDatum dValue) { }

		CPortSet &m_PortSet;
		CString m_sID;
	};

class CPortSet
	{
	public:
		CPortSet () { }

		bool AddPort (const CString &sID, const CString &sType, CString *retsError = NULL);
		SequenceNumber AllocSeq () { return ++m_Seq; }
		CDatum GetView (SequenceNumber Seq) const;
		void Mark ();
		void Output (const CString &sID, CDatum dValue);

	private:
		CCriticalSection m_cs;
		SequenceNumber m_Seq = 0;
		TSortMap<CString, TSharedPtr<IProgramPort>> m_Ports;
	};

enum class ProgramState
	{
	unknown,

	notStarted,			//	Program has not started
	loading,			//	Loading code or compiling
	readyToStart,		//	Ready to start running
	readyToContinue,	//	Ready to continue with async result
	running,			//	Executing code
	waitingForResult,	//	Async call
	terminated,			//	Program run completed
	};

struct SRunResult
	{
	SRunResult (IProgramInstance *pProgramArg, CHexeProcess::ERunCodes iRunCodeArg, CDatum dResultArg) :
			pProgram(pProgramArg),
			iRunCode(iRunCodeArg),
			dResult(dResultArg)
		{ }

	IProgramInstance *pProgram = NULL;			//	May be NULL if no work was done.
	CHexeProcess::ERunCodes iRunCode = CHexeProcess::runOK;
	CDatum dResult;
	};

struct SRunOptions
	{
	CString sEnvironment;					//	Program Type
	CString sProgramID;						//	Program ID
	CString sRunBy;							//	User running the program
	};

class IProgramInstance : public TRefCounted<IProgramInstance>
	{
	public:
		IProgramInstance (DWORD dwID) :
				m_dwID(dwID)
			{ }

		virtual ~IProgramInstance ()
			{
			DebugBreak();
			}

		bool CanView (const CString &sUsername) const;
		static TSharedPtr<IProgramInstance> Create (DWORD dwID, CDatum dCode, const SRunOptions &Options, CString *retsError = NULL);
		DWORD GetID () const { return m_dwID; }
		CDatum GetView (SequenceNumber Seq) const;
		void Mark ();
		void Output (const CString &sID, CDatum dValue) { m_Ports.Output(sID, dValue); }
		SRunResult Run ();
		bool RunLock ();
		void SetAsyncResult (CDatum dValue);

	protected:
		virtual bool OnCreate (CDatum dCode, const SRunOptions &Options, CString *retsError = NULL) { return true; }
		virtual void OnMark () { }
		virtual SRunResult OnRun () { return SRunResult(NULL, CHexeProcess::runOK, CDatum()); }

		ProgramState GetRunState () const { return m_iState; }
		CDatum GetRunStateValue () const { return m_dValue; }
		void SetRunState (ProgramState iState) { m_iState = iState; }
		void SetRunState (ProgramState iState, CDatum dValue) { m_iState = iState; m_dValue = dValue; }

	private:
		void CreateDefaultPorts ();
		static TSharedPtr<IProgramInstance> CreateFromType (const CString &sType, DWORD dwID);
		CDatum GetStatusCode () const;

		CCriticalSection m_cs;
		DWORD m_dwID = 0;
		CString m_sProgramID;				//	Program ID
		CString m_sRunBy;					//	User running the program

		ProgramState m_iState = ProgramState::notStarted;
		CDatum m_dValue;					//	Depends on state
		bool m_bRunLock = false;			//	If TRUE, a caller is about to call Run.

		CPortSet m_Ports;					//	Ports to communicate with environment
	};

class CProgramSet
	{
	public:
		CProgramSet ();

		bool CreateInstance (CDatum dCode, const SRunOptions &Options, DWORD *retdwID = NULL, CString *retsError = NULL);
		bool GetInstanceView (DWORD dwID, const CString &sUsername, SequenceNumber Seq, CDatum *retdResult) const;
		CManualEvent &GetWorkEvent () { return m_HasWork; }
		void Mark ();
		void OnAsyncResult (const SArchonMessage &Msg);
		SRunResult Run ();
		void SetAsyncTicket (IProgramInstance &Program, DWORD dwTicket);

	private:
		IProgramInstance *LockNextInstance ();

		CCriticalSection m_cs;
		TSortMap<DWORD, TSharedPtr<IProgramInstance>> m_Instances;
		TSortMap<DWORD, IProgramInstance *> m_TicketToInstance;

		DWORD m_dwNextID = 1;
		CManualEvent m_HasWork;
		int m_iNextInstance = 0;
	};

class CProgramExecThread : public TThread<CProgramExecThread>
	{
	public:
		CProgramExecThread (IArchonProcessCtx &ArchonCtx, CCodeSlingerEngine &Engine, CProgramSet &ProgramSet, CManualEvent &SignalPause, CManualEvent &SignalResume, CManualEvent &SignalShutdown) :
				m_ArchonCtx(ArchonCtx),
				m_Engine(Engine),
				m_ProgramSet(ProgramSet),
				m_SignalPause(SignalPause),
				m_SignalResume(SignalResume),
				m_SignalShutdown(SignalShutdown)
			{
			m_Paused.Create();
			}
			
		void Run ();
		void WaitForPause () { m_Paused.Wait(); }

	private:
		IArchonProcessCtx &m_ArchonCtx;
		CCodeSlingerEngine &m_Engine;
		CProgramSet &m_ProgramSet;
		CManualEvent &m_SignalPause;			//	If set, we need to pause thread.
		CManualEvent m_Paused;					//	If set, thread is paused.
		CManualEvent &m_SignalResume;			//	If set, we need to continue thread.
		CManualEvent &m_SignalShutdown;			//	If set, we need to shut down.
	};

class CProgramExecPool
	{
	public:
		CProgramExecPool (CProgramSet &ProgramSet) :
				m_ProgramSet(ProgramSet)
			{ }

		bool Boot (IArchonProcessCtx &ArchonCtx, CCodeSlingerEngine &Engine, int iThreadCount);
		void WaitForPause ();
		void WaitForShutdown ();

	private:
		CProgramSet &m_ProgramSet;
		TArray<TUniquePtr<CProgramExecThread>> m_Pool;
	};
