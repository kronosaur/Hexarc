//	CProgramInstance.cpp
//
//	CProgramInstance class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(FIELD_DOLLAR_STATUS,				"$Status");

DECLARE_CONST_STRING(LIBRARY_CODE_SLINGER,				"codeSlinger");
DECLARE_CONST_STRING(LIBRARY_CORE,						"core");

DECLARE_CONST_STRING(LIBRARY_INSTANCE_CTX,				"instanceCtx");

DECLARE_CONST_STRING(PORT_CON,							"CON");
DECLARE_CONST_STRING(PORT_NULL,							"NULL");
DECLARE_CONST_STRING(PORT_RESULT,						"RESULT");

DECLARE_CONST_STRING(PORT_TYPE_CONSOLE,					"console");
DECLARE_CONST_STRING(PORT_TYPE_NULL,					"null");
DECLARE_CONST_STRING(PORT_TYPE_VALUE,					"value");

DECLARE_CONST_STRING(STATUS_CODE_ASYNC_WAIT,			"asyncWait");
DECLARE_CONST_STRING(STATUS_CODE_LOADING,				"loading");
DECLARE_CONST_STRING(STATUS_CODE_NOT_STARTED,			"notStarted");
DECLARE_CONST_STRING(STATUS_CODE_READY,					"ready");
DECLARE_CONST_STRING(STATUS_CODE_RUNNING,				"running");
DECLARE_CONST_STRING(STATUS_CODE_TERMINATED,			"terminated");

DECLARE_CONST_STRING(ERR_INTERNAL_ERROR,				"ERROR: This should never happen.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_CODE,			"Unable to parse input.");

bool CProgramInstance::CanView (const CString &sUsername) const

//	CanView
//
//	Returns TRUE if the given user can give the running program.

	{
	CSmartLock Lock(m_cs);

	return strEquals(sUsername, m_sRunBy);
	}

bool CProgramInstance::Create (CDatum dCode, const SRunOptions &Options, CString *retsError)

//	Create
//
//	Initializes the program instance from the given code.

	{
	//	Initialize

	m_sProgramID = Options.sProgramID;
	m_sRunBy = Options.sRunBy;

	CreateDefaultPorts();

	//	Initialize the process

	m_Process.LoadLibrary(LIBRARY_CORE);
	m_Process.LoadLibrary(LIBRARY_CODE_SLINGER);

	m_Process.SetLibraryCtx(LIBRARY_INSTANCE_CTX, this);

	//	We run with minimal rights
	//
	//	LATER: If we want more rights, we need to pass in the rights that we 
	//	want and we need to verify that the service and user have appropriate
	//	rights.

	CHexeSecurityCtx SecurityCtx;
	SecurityCtx.SetExecutionRights(CHexeSecurityCtx::EXEC_RIGHTS_MINIMAL);
	m_Process.SetSecurityCtx(SecurityCtx);

	//	Abort if we don't complete this in a certain amount of time

	m_Process.SetMaxExecutionTime(EXECUTION_QUANTUM);

	//	Parse into an expression (depending on the type of input)

	CDatum dExpression;
	if (dCode.GetBasicType() == CDatum::typeString)
		{
		CString sError;
		if (!CHexeDocument::ParseLispExpression(dCode, &dExpression, retsError))
			return false;
		}

	//	Otherwise we don't know how to parse the input

	else
		{
		if (retsError) *retsError = ERR_UNABLE_TO_PARSE_CODE;
		return false;
		}

	//	Set our state

	m_iState = ProgramState::readyToStart;
	m_dValue = dExpression;

	return true;
	}

void CProgramInstance::CreateDefaultPorts ()

//	CreateDefaultPorts
//
//	Creates the default ports.

	{
	m_Ports.AddPort(PORT_NULL, PORT_TYPE_NULL);
	m_Ports.AddPort(PORT_RESULT, PORT_TYPE_VALUE);
	m_Ports.AddPort(PORT_CON, PORT_TYPE_CONSOLE);
	}

CDatum CProgramInstance::GetStatusCode () const

//	GetStatusCode
//
//	Returns the current program status.

	{
	CSmartLock Lock(m_cs);

	switch (m_iState)
		{
		case ProgramState::loading:
			return STATUS_CODE_LOADING;

		case ProgramState::notStarted:
			return STATUS_CODE_NOT_STARTED;

		case ProgramState::readyToContinue:
		case ProgramState::readyToStart:
			return STATUS_CODE_READY;

		case ProgramState::running:
			return STATUS_CODE_RUNNING;

		case ProgramState::terminated:
			return STATUS_CODE_TERMINATED;

		case ProgramState::waitingForResult:
			return STATUS_CODE_ASYNC_WAIT;

		default:
			return strPattern("UNKNOWN%d", (int)m_iState);
		}
	}

CDatum CProgramInstance::GetView (SequenceNumber Seq) const

//	GetView
//
//	Returns a client view.

	{
	CDatum dResult = m_Ports.GetView(Seq);

	//	Set status of program

	dResult.SetElement(FIELD_DOLLAR_STATUS, GetStatusCode());
	return dResult;
	}

SRunResult CProgramInstance::HandleRunResult (CHexeProcess::ERunCodes iRunCode, CDatum dResult)

//	HandleRunResult
//
//	Returns a result and completes the run, setting state as appropriate.

	{
	switch (iRunCode)
		{
		case CHexeProcess::runOK:
		case CHexeProcess::runError:
			m_iState = ProgramState::terminated;
			m_dValue = dResult;
			m_Ports.Output(PORT_RESULT, dResult);
			break;

		case CHexeProcess::runAsyncRequest:
			m_iState = ProgramState::waitingForResult;
			m_dValue = CDatum();
			break;

		default:
			m_iState = ProgramState::terminated;
			m_dValue = dResult;
			iRunCode = CHexeProcess::runError;
			break;
		}

	m_cs.Lock();
	m_bRunLock = false;
	m_cs.Unlock();

	return SRunResult(this, iRunCode, dResult);
	}

void CProgramInstance::Mark ()

//	Mark
//
//	Marks data in use.

	{
	m_dValue.Mark();
	m_Process.Mark();
	m_Ports.Mark();
	}

SRunResult CProgramInstance::Run ()

//	Run
//
//	Runs the program until:
//
//	1. The program terminates.
//	2. The program needs an async result.
//	3. The program exceeds its time quantum.
//
//	NOTE: Do not call Run unless RunLock returns TRUE.

	{
	if (m_iState == ProgramState::readyToStart)
		{
		m_iState = ProgramState::running;

		CDatum dResult;
		CHexeProcess::ERunCodes iRun = m_Process.Run(m_dValue, &dResult);

		return HandleRunResult(iRun, dResult);
		}
	else if (m_iState == ProgramState::readyToContinue)
		{
		m_iState = ProgramState::running;

		CDatum dResult;
		CHexeProcess::ERunCodes iRun = m_Process.RunContinues(m_dValue, &dResult);

		return HandleRunResult(iRun, dResult);
		}
	else
		return HandleRunResult(CHexeProcess::runError, ERR_INTERNAL_ERROR);
	}

bool CProgramInstance::RunLock ()

//	RunLock
//
//	Attempts to lock the program for running. Returns TRUE if successful (and
//	the lock will be released at the end of Run.

	{
	CSmartLock Lock(m_cs);

	if (!m_bRunLock
			&& (m_iState == ProgramState::readyToStart || m_iState == ProgramState::readyToContinue))
		{
		m_bRunLock = true;
		return true;
		}
	else
		return false;
	}

void CProgramInstance::SetAsyncResult (CDatum dValue)

//	SetAsyncResult
//
//	Sets an async result for the program.

	{
	CSmartLock Lock(m_cs);

	if (!m_bRunLock && m_iState == ProgramState::waitingForResult)
		{
		m_iState = ProgramState::readyToContinue;
		m_dValue = dValue;
		}
	}
