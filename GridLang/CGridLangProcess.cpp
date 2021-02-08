//	CGridLangProcess.cpp
//
//	CGridLangProcess Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(LIBRARY_GRID_LANG_CORE,			"gridLangCore");

bool CGridLangProcess::Load (CDatum &dResult)

//	Load
//
//	Load the program into the process.

	{
	if (m_iState != EState::None)
		throw CException(errFail);

	//	Load the core library

	CGridLangCoreLibrary::Register();

	CString sError;
	if (!m_Hexe.LoadLibrary(LIBRARY_GRID_LANG_CORE, &sError))
		{
		dResult = sError;
		return false;
		}

	//	Add all symbols to the process

	if (!m_Hexe.LoadEntryPoints(m_Program.GetCode().GetCodeAsEntryPointTable(), &sError))
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
	CHexeDocument::CreateFunctionCall(m_Program.GetMainFunction(), TArray<CDatum>(), &dCode);

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
