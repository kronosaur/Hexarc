//	IProgramInstance.cpp
//
//	IProgramInstance class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(ENVIRONMENT_HEXE_LISP,				"hexeLisp");
DECLARE_CONST_STRING(ENVIRONMENT_JULIA,					"julia");

DECLARE_CONST_STRING(FIELD_DOLLAR_STATUS,				"$Status");

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

DECLARE_CONST_STRING(ERR_UNKNOWN_PROGRAM_ENV,			"Unknown program environment: %s.");

bool IProgramInstance::CanView (const CString &sUsername) const

//	CanView
//
//	Returns TRUE if the given user can give the running program.

	{
	CSmartLock Lock(m_cs);

	return strEquals(sUsername, m_sRunBy);
	}

TSharedPtr<IProgramInstance> IProgramInstance::Create (DWORD dwID, CDatum dCode, const SRunOptions &Options, CString *retsError)

//	Create
//
//	Initializes the program instance from the given code.

	{
	//	Create the program.

	TSharedPtr<IProgramInstance> pProgram = CreateFromType(Options.sEnvironment, dwID);
	if (!pProgram)
		{
		if (retsError) *retsError = strPattern(ERR_UNKNOWN_PROGRAM_ENV, Options.sEnvironment);
		return pProgram;
		}

	//	Initialize

	pProgram->m_sProgramID = Options.sProgramID;
	pProgram->m_sRunBy = Options.sRunBy;
	pProgram->CreateDefaultPorts();
	pProgram->m_iState = ProgramState::readyToStart;

	//	Create

	if (!pProgram->OnCreate(dCode, Options, retsError))
		return NULL;

	return pProgram;
	}

void IProgramInstance::CreateDefaultPorts ()

//	CreateDefaultPorts
//
//	Creates the default ports.

	{
	m_Ports.AddPort(PORT_NULL, PORT_TYPE_NULL);
	m_Ports.AddPort(PORT_RESULT, PORT_TYPE_VALUE);
	m_Ports.AddPort(PORT_CON, PORT_TYPE_CONSOLE);
	}

TSharedPtr<IProgramInstance> IProgramInstance::CreateFromType (const CString &sType, DWORD dwID)

//	CreateFromType
//
//	Creates the proper class.

	{
	if (sType.IsEmpty() || strEquals(sType, ENVIRONMENT_HEXE_LISP))
		return TSharedPtr<IProgramInstance>(new CHexeLispEnvironment(dwID));
	else
		return NULL;
	}

CDatum IProgramInstance::GetStatusCode () const

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

CDatum IProgramInstance::GetView (SequenceNumber Seq) const

//	GetView
//
//	Returns a client view.

	{
	CDatum dResult = m_Ports.GetView(Seq);

	//	Set status of program

	dResult.SetElement(FIELD_DOLLAR_STATUS, GetStatusCode());
	return dResult;
	}

void IProgramInstance::Mark ()

//	Mark
//
//	Marks data in use.

	{
	m_dValue.Mark();
	m_Ports.Mark();

	OnMark();
	}

SRunResult IProgramInstance::Run ()

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
	SRunResult Result = OnRun();

	m_cs.Lock();
	m_bRunLock = false;
	m_cs.Unlock();

	return Result;
	}

bool IProgramInstance::RunLock ()

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

void IProgramInstance::SetAsyncResult (CDatum dValue)

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
