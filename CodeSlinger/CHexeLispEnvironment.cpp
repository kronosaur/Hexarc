//	CHexeLispEnvironment.cpp
//
//	CHexeLispEnvironment Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(LIBRARY_CODE_SLINGER,				"codeSlinger");
DECLARE_CONST_STRING(LIBRARY_CORE,						"core");

DECLARE_CONST_STRING(LIBRARY_INSTANCE_CTX,				"instanceCtx");

DECLARE_CONST_STRING(PORT_RESULT,						"RESULT");

DECLARE_CONST_STRING(ERR_INTERNAL_ERROR,				"ERROR: This should never happen.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_CODE,			"Unable to parse input.");

bool CHexeLispEnvironment::OnCreate (CDatum dCode, const SRunOptions &Options, CString *retsError)

//	OnCreate
//
//	Initialiaze.

	{
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

	//	Store the compiled expression in our state

	SetRunState(ProgramState::readyToStart, dExpression);

	return true;
	}

void CHexeLispEnvironment::OnMark ()

//	OnMark
//
//	Mark data in use.

	{
	m_Process.Mark();
	}

SRunResult CHexeLispEnvironment::OnRun ()

//	OnRun
//
//	Run!

	{
	if (GetRunState() == ProgramState::readyToStart)
		{
		SetRunState(ProgramState::running);

		CDatum dResult;
		CHexeProcess::ERunCodes iRun = m_Process.Run(GetRunStateValue(), &dResult);

		return HandleRunResult(iRun, dResult);
		}
	else if (GetRunState() == ProgramState::readyToContinue)
		{
		SetRunState(ProgramState::running);

		CDatum dResult;
		CHexeProcess::ERunCodes iRun = m_Process.RunContinues(GetRunStateValue(), &dResult);

		return HandleRunResult(iRun, dResult);
		}
	else
		return HandleRunResult(CHexeProcess::runError, ERR_INTERNAL_ERROR);
	}

SRunResult CHexeLispEnvironment::HandleRunResult (CHexeProcess::ERunCodes iRunCode, CDatum dResult)

//	HandleRunResult
//
//	Returns a result and completes the run, setting state as appropriate.

	{
	switch (iRunCode)
		{
		case CHexeProcess::runOK:
		case CHexeProcess::runError:
			SetRunState(ProgramState::terminated, dResult);
			Output(PORT_RESULT, dResult);
			break;

		case CHexeProcess::runAsyncRequest:
			SetRunState(ProgramState::waitingForResult, CDatum());
			break;

		default:
			SetRunState(ProgramState::terminated, dResult);
			iRunCode = CHexeProcess::runError;
			break;
		}

	return SRunResult(this, iRunCode, dResult);
	}
