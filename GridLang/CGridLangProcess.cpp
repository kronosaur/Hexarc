//	CGridLangProcess.cpp
//
//	CGridLangProcess Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(LIBRARY_GRID_LANG_CORE,			"gridLangCore");
DECLARE_CONST_STRING(LIBRARY_GRID_LANG_ENV,				"gridLangEnv");

void CGridLangProcess::Init (const CGridLangProgram &Program)

//	Init
//
//	Initialize.

	{
	if (!m_pDefaultEnv)
		m_pDefaultEnv.Set(new CGLDefaultEnvironment);

	Init(Program, *m_pDefaultEnv);
	}

void CGridLangProcess::Init (const CGridLangProgram &Program, IGridLangEnvironment &Environment)

//	Init
//
//	Initialize.

	{
	m_pProgram = &Program;
	m_iState = EState::None;

	m_pEnvironment = &Environment;
	}

bool CGridLangProcess::Load (CDatum &dResult)

//	Load
//
//	Load the program into the process.

	{
	if (m_iState != EState::None || !m_pProgram)
		throw CException(errFail);

	//	Load the core library

	CGridLangCoreLibrary::Register();

	CString sError;
	if (!m_Hexe.LoadLibrary(LIBRARY_GRID_LANG_CORE, &sError))
		{
		dResult = sError;
		return false;
		}

	if (m_pEnvironment)
		m_Hexe.SetLibraryCtx(LIBRARY_GRID_LANG_ENV, m_pEnvironment);

	//	Add all symbols to the process

	if (!m_Hexe.LoadEntryPoints(m_pProgram->GetCode().GetCodeAsEntryPointTable(), &sError))
		{
		dResult = sError;
		return false;
		}

	//	Done

	m_iState = EState::Loaded;
	return true;
	}

CHexeProcess::ERunCodes CGridLangProcess::Run (CDatum &dResult)

//	Run
//
//	Run the program.

	{
	if (!m_pProgram)
		throw CException(errFail);

	//	Load the Hexe process, if necessary.

	if (m_iState == EState::None)
		{
		if (!Load(dResult))
			return CHexeProcess::runError;
		}

	if (m_iState != EState::Loaded)
		throw CException(errFail);

	//	Run

	CDatum dCode;
	CHexeDocument::CreateFunctionCall(m_pProgram->GetMainFunction(), TArray<CDatum>(), &dCode);

	CHexeProcess::ERunCodes iRun = m_Hexe.Run(dCode, &dResult);

	m_iState = EState::Done;
	return iRun;
	}

CHexeProcess::ERunCodes CGridLangProcess::RunContinues (CDatum dAsyncResult, CDatum &dResult)

//	RunContinues
//
//	Continues after async result.

	{
	if (m_iState != EState::WaitingForAsync)
		throw CException(errFail);

	return m_Hexe.RunContinues(dAsyncResult, &dResult);
	}

void CGridLangProcess::SetExecutionRights (DWORD dwFlags)

//	SetExecutionRights
//
//	Sets rights.

	{
	CHexeSecurityCtx SecurityCtx;
	SecurityCtx.SetExecutionRights(dwFlags);
	m_Hexe.SetSecurityCtx(SecurityCtx);
	}

void CGridLangProcess::SetMaxExecutionTime (DWORD dwMaxTime)

//	SetMaxExecutionTime
//
//	Sets the maximum execution time.

	{
	m_Hexe.SetMaxExecutionTime(dwMaxTime);
	}
