//	CHexeProcess.cpp
//
//	CHexeProcess class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(LIBRARY_CORE,						"core");

DECLARE_CONST_STRING(TYPE_HEXE_LISP,					"$hexeLisp");

DECLARE_CONST_STRING(ERR_ASYNC_REQUEST_NOT_ALLOWED,		"Async requests are not allowed.");
DECLARE_CONST_STRING(ERR_COMPUTE_CRASH,					"Compute runtime crash.");
DECLARE_CONST_STRING(ERR_LIBRARY_NOT_FOUND,				"Library not found: %s.");
DECLARE_CONST_STRING(ERR_HEXE_CODE_EXPECTED,			"Unable to execute term: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN,						"Unknown error.");

CHexeProcess::CHexeProcess (void)

//	CHexeProcess constructor

	{
	}

void CHexeProcess::CreateFunctionCall (int iArgCount, CDatum *retdExpression)

//	CreateFunctionCall
//
//	Creates a code block that calls a function with the given number
//	of arguments. We expect the stack to contain the function and the parameters.

	{
	CHexeCode::CreateFunctionCall(iArgCount, retdExpression);
	}

void CHexeProcess::CreateInvokeCall (const TArray<CDatum> &Args, CDatum *retdExpression)

//	CreateInvokeCall
//
//	Creates a code block that calls an invoke with the given arguments.

	{
	CHexeCode::CreateInvokeCall(Args, retdExpression);
	}

void CHexeProcess::DefineGlobal (const CString &sIdentifier, CDatum dValue)

//	DefineGlobal
//
//	Defines a global variable

	{
	InitGlobalEnv();

	if (!sIdentifier.IsEmpty())
		m_pCurGlobalEnv->SetAt(sIdentifier, dValue);
	}

void CHexeProcess::DeleteAll (void)

//	DeleteAll
//
//	Deletes everything. Load must be called again.

	{
	m_Stack.DeleteAll();

	m_dExpression = CDatum();
	m_dCodeBank = CDatum();
	m_pCodeBank = NULL;
	m_CallStack.DeleteAll();

	m_pCurGlobalEnv = new CHexeGlobalEnvironment;
	m_dCurGlobalEnv = CDatum(m_pCurGlobalEnv);

	m_dLocalEnv = CDatum();
	m_pLocalEnv = NULL;
	m_LocalEnvStack.DeleteAll();
	}

bool CHexeProcess::FindGlobalDef (const CString &sIdentifier, CDatum *retdValue)

//	FindGlobalDef
//
//	Finds a global value in the process's global environment.
//
//	NOTE: This should only be called outside of Run because it only looks in the
//	process's environment, not in the current function's env.
	
	{
	//	NOTE: We don't call InitGlobalEnv because there is no need to initialize
	//	it. If it's not initialized, then clearly we don't have the definition.

	CHexeGlobalEnvironment *pEnv = CHexeGlobalEnvironment::Upconvert(m_dGlobalEnv);
	if (pEnv == NULL)
		return false;

	return pEnv->Find(sIdentifier, retdValue);
	}

void CHexeProcess::GetCurrentSecurityCtx (CHexeSecurityCtx *retCtx)

//	GetCurrentSecurityCtx
//
//	Gets the current security context based on the global environment and
//	user security.

	{
	m_pCurGlobalEnv->GetServiceSecurity(retCtx);
	m_UserSecurity.GetUserSecurity(retCtx);
	}

CDatum CHexeProcess::GetCurrentSecurityCtx (void)

//	GetCurrentSecurityCtx
//
//	Gets the current security context as a datum.

	{
	CHexeSecurityCtx Ctx;
	GetCurrentSecurityCtx(&Ctx);
	return Ctx.AsDatum();
	}

bool CHexeProcess::InitFrom (const CHexeProcess &Process, CString *retsError)

//	InitFrom
//
//	Initializes the environment from another process. This function copies the
//	global environment, not the run state.
//
//	This method is designed to be called before running.

	{
	DeleteAll();

	CHexeGlobalEnvironment *pSrcGlobalEnv = CHexeGlobalEnvironment::Upconvert(Process.m_dGlobalEnv);
	if (pSrcGlobalEnv)
		{
		CHexeGlobalEnvironment *pGlobalEnv = new CHexeGlobalEnvironment(pSrcGlobalEnv);
		m_dGlobalEnv = CDatum(pGlobalEnv);
		}

	return true;
	}

void CHexeProcess::InitGlobalEnv (CHexeGlobalEnvironment **retpGlobalEnv)

//	InitGlobalEnv
//
//	Initialize global environment if not already initialized.

	{
	if (m_dGlobalEnv.IsNil())
		{
		CHexeGlobalEnvironment *pGlobalEnv = new CHexeGlobalEnvironment;
		m_dGlobalEnv = CDatum(pGlobalEnv);

		m_pCurGlobalEnv = pGlobalEnv;
		m_dCurGlobalEnv = m_dGlobalEnv;

		if (retpGlobalEnv)
			*retpGlobalEnv = pGlobalEnv;
		}
	else if (retpGlobalEnv)
		{
		*retpGlobalEnv = CHexeGlobalEnvironment::Upconvert(m_dGlobalEnv);
		}
	}

bool CHexeProcess::LoadEntryPoints (const CHexeDocument &Program, CString *retsError)

//	LoadEntryPoints
//
//	Loads entry points defined in a document.

	{
	//	Define all the functions in the program. Note that all of the functions
	//	define here will inherit the security context of the document
	//	(i.e., the package) NOT this process. In practice this means that these
	//	functions get their security context from the package, not the service.

	TArray<CHexeDocument::SEntryPoint> EntryPoints;
	Program.GetEntryPoints(&EntryPoints);
	if (!LoadEntryPoints(EntryPoints, retsError))
		return false;

	return true;
	}

bool CHexeProcess::LoadEntryPoints (const TArray<CHexeDocument::SEntryPoint> &EntryPoints, CString *retsError)

//	LoadEntryPoints
//
//	Loads entry points defined in a document.

	{
	int i;

	//	Add all functions to the process global environment

	for (i = 0; i < EntryPoints.GetCount(); i++)
		DefineGlobal(EntryPoints[i].sFunction, EntryPoints[i].dCode);

	return true;
	}

bool CHexeProcess::LoadHexeDefinitions (const CHexeDocument &Program, CString *retsError)

//	LoadHexeDefinitions
//
//	Loads the given definitions. The given datums must be code blocks definiting
//	some global variable.

	{
	//	Run all code fragments in the document (most of the fragments will be
	//	defines for the global environment).
	//
	//	NOTE: Because we are evaluating the defines here the resulting functions
	//	will inherit this process's security context. In practice this means 
	//	that these functions get their security context from the service.

	TArray<CDatum> Definitions;
	Program.GetHexeDefinitions(&Definitions);
	if (!LoadHexeDefinitions(Definitions, retsError))
		return false;

	return true;
	}

bool CHexeProcess::LoadHexeDefinitions (const TArray<CDatum> &Definitions, CString *retsError)

//	LoadHexeDefinitions
//
//	Loads the given definitions. The given datums must be code blocks definiting
//	some global variable.

	{
	int i;

	for (i = 0; i < Definitions.GetCount(); i++)
		{
		CDatum dResult;
		ERun iRun = Run(Definitions[i], &dResult);
		
		//	Process the result

		switch (iRun)
			{
			case ERun::OK:
				break;

			case ERun::Error:
				*retsError = dResult;
				return false;

			case ERun::AsyncRequest:
			case ERun::InputRequest:
				*retsError = ERR_ASYNC_REQUEST_NOT_ALLOWED;
				return false;

			default:
				*retsError = ERR_UNKNOWN;
				return false;
			}
		}

	return true;
	}

bool CHexeProcess::LoadLibrary (const CString &sName, CString *retsError)

//	LoadLibrary
//
//	Loads the definitions in the given library (from the Hexe Librarian)

	{
	int i;
	DWORD dwLibraryID;

	InitGlobalEnv();

	if (!g_HexeLibrarian.FindLibrary(sName, &dwLibraryID))
		{
		if (retsError)
			*retsError = strPattern(ERR_LIBRARY_NOT_FOUND, sName);
		return false;
		}

	for (i = 0; i < g_HexeLibrarian.GetEntryCount(dwLibraryID); i++)
		{
		CDatum dFunction;
		const CString &sFunction = g_HexeLibrarian.GetEntry(dwLibraryID, i, &dFunction);

		m_pCurGlobalEnv->SetAt(sFunction, dFunction);
		}

	return true;
	}

bool CHexeProcess::LoadStandardLibraries (CString *retsError)

//	LoadStandardLibraries
//
//	Loads the core libraries

	{
	//	All processes should have the core library

	if (!LoadLibrary(LIBRARY_CORE, retsError))
		return false;

	//	Done

	return true;
	}

void CHexeProcess::Mark (void)

//	Mark
//
//	Mark all data in use

	{
	m_Stack.Mark();
	m_dExpression.Mark();
	m_dCodeBank.Mark();
	m_CallStack.Mark();

	m_dGlobalEnv.Mark();
	m_dCurGlobalEnv.Mark();

	m_dLocalEnv.Mark();
	m_LocalEnvStack.Mark();

//	m_dSecurityCtx.Mark();
	}

CHexeProcess::ERun CHexeProcess::Run (const CString &sExpression, CDatum *retdResult)

//	Run
//
//	Compiles and runs the given expression

	{
	CString sError;

	CDatum dExpression;
	if (!CHexeDocument::ParseLispExpression(sExpression, &dExpression, &sError))
		{
		*retdResult = CDatum(sError);
		return ERun::Error;
		}

	return Run(dExpression, retdResult);
	}

CHexeProcess::ERun CHexeProcess::Run (CDatum dExpression, CDatum *retResult)
	
//	Run
//
//	Runs the given expression.

	{
	m_Stack.DeleteAll();
	return RunWithStack(dExpression, retResult);
	}

CHexeProcess::ERun CHexeProcess::Run (CDatum dFunc, const TArray<CDatum> &Args, CDatum *retResult)

//	Run
//
//	Runs the given function with the given arguments.

	{
	int i;

	//	First we generate code to call the function

	CDatum dExpression;
	CHexeCode::CreateFunctionCall(Args.GetCount(), &dExpression);

	//	Set up the stack. The function goes first followed by the args

	m_Stack.DeleteAll();
	m_Stack.Push(dFunc);

	for (i = 0; i < Args.GetCount(); i++)
		m_Stack.Push(Args[i]);

	//	Now call the function

	return RunWithStack(dExpression, retResult);
	}

CHexeProcess::ERun CHexeProcess::Run (CDatum dFunc, CDatum dCallExpression, const TArray<CDatum> *pInitialStack, CDatum *retResult)

//	Run
//
//	Adds the given data to a stack (pushed in order) and runs the expression.

	{
	int i;

	//	Set up the stack

	m_Stack.DeleteAll();
	m_Stack.Push(dFunc);

	if (pInitialStack)
		{
		for (i = 0; i < pInitialStack->GetCount(); i++)
			m_Stack.Push(pInitialStack->GetAt(i));
		}

	//	Run

	return RunWithStack(dCallExpression, retResult);
	}

CHexeProcess::ERun CHexeProcess::RunContinues (CDatum dAsyncResult, CDatum *retResult)

//	RunContinues
//
//	Continue running after an interruption

	{
	//	If we got an error, then return it

	if (dAsyncResult.IsError())
		{
		*retResult = dAsyncResult;
		return ERun::Error;
		}

	//	Push the result on the stack

	m_Stack.Push(dAsyncResult);

	//	Continue execution

	ERun iRun;
	try
		{
		iRun = Execute(retResult);
		}
	catch (...)
		{
		*retResult = strPattern(ERR_COMPUTE_CRASH);
		return ERun::Error;
		}

	return iRun;
	}

CHexeProcess::ERun CHexeProcess::RunWithStack (CDatum dExpression, CDatum *retResult)

//	RunWithStack
//
//	Runs the given entrypoint assuming that the stack has been initialized
//	previously.

	{
	//	Set abort time

	if (m_dwMaxExecutionTime)
		m_dwAbortTime = ::sysGetTickCount64() + m_dwMaxExecutionTime;
	else
		m_dwAbortTime = 0;

	//	Prepare the context

	m_dExpression = dExpression;
	m_CallStack.DeleteAll();
	m_LocalEnvStack.DeleteAll();

	//	Initialize the instruction pointer

	CHexeFunction *pExpression = CHexeFunction::Upconvert(dExpression);
	if (pExpression == NULL)
		{
		*retResult = strPattern(ERR_HEXE_CODE_EXPECTED, dExpression.AsString());
		return ERun::Error;
		}

	m_pIP = pExpression->GetCode(&m_dCodeBank);
	if (m_pIP == NULL)
		{
		*retResult = strPattern(ERR_HEXE_CODE_EXPECTED, dExpression.AsString());
		return ERun::Error;
		}

	m_pCodeBank = CHexeCode::Upconvert(m_dCodeBank);
	if (m_pCodeBank == NULL)
		{
		*retResult = strPattern(ERR_HEXE_CODE_EXPECTED, dExpression.AsString());
		return ERun::Error;
		}

	//	Initialize the environment

	InitGlobalEnv();
	m_dCurGlobalEnv = m_dGlobalEnv;
	m_pCurGlobalEnv = CHexeGlobalEnvironment::Upconvert(m_dGlobalEnv);

	m_dLocalEnv = CDatum();
	m_pLocalEnv = NULL;

	//	Run

	ERun iRun;
	try
		{
		iRun = Execute(retResult);
		}
	catch (...)
		{
		*retResult = strPattern(ERR_COMPUTE_CRASH);
		return ERun::Error;
		}

	return iRun;
	}

void *CHexeProcess::SetLibraryCtx (const CString &sLibrary, void *pCtx)

//	SetLibraryCtx
//
//	Sets the library context and returns the previous context.

	{
	void *pOldCtx;

	if (pCtx)
		{
		bool bNew;
		void **ppPos = m_LibraryCtx.SetAt(sLibrary, &bNew);

		pOldCtx = (bNew ? NULL : *ppPos);
		*ppPos = pCtx;
		}

	//	If we're setting the context to NULL then this is the same as deleting
	//	the context.

	else
		{
		int iPos;
		if (m_LibraryCtx.FindPos(sLibrary, &iPos))
			{
			pOldCtx = m_LibraryCtx[iPos];
			m_LibraryCtx.Delete(iPos);
			}
		else
			pOldCtx = NULL;
		}

	return pOldCtx;
	}

void CHexeProcess::SetSecurityCtx (const CHexeSecurityCtx &Ctx)

//	SetSecurityCtx
//
//	Sets the security context for the process.

	{
	//	Service security is stored in the global environment

	CHexeGlobalEnvironment *pEnv;
	InitGlobalEnv(&pEnv);

	pEnv->SetServiceSecurity(Ctx);

	//	User security is stored in the process.

	m_UserSecurity.SetUserSecurity(Ctx);
	}
	
void CHexeProcess::SetUserSecurity (const CString &sUsername, const CAttributeList &Rights)

//	SetUserSecurity
//
//	Sets the username and rights.

	{
	m_UserSecurity.SetUsername(sUsername);
	m_UserSecurity.SetUserRights(Rights);
	}
