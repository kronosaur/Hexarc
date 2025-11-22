//	CHexeProcess.cpp
//
//	CHexeProcess class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

static constexpr DWORD PROGRAM_LIMITS_VERSION = 1;

DECLARE_CONST_STRING(FIELD_ABORT_TIME,					"abortTime");
DECLARE_CONST_STRING(FIELD_ADD_CONCATENATES_STRINGS,	"addConcatenatesStrings");
DECLARE_CONST_STRING(FIELD_AEON_TYPES,					"AEONTypes");
DECLARE_CONST_STRING(FIELD_LIBRARIES,					"libraries");
DECLARE_CONST_STRING(FIELD_LIMITS,						"limits");
DECLARE_CONST_STRING(FIELD_MAX_EXECUTION_TIME,			"maxExecutionTime");
DECLARE_CONST_STRING(FIELD_STACK_LEVEL,					"stackLevel");
DECLARE_CONST_STRING(FIELD_TYPES,						"types");

DECLARE_CONST_STRING(LIBRARY_CORE,						"core");

DECLARE_CONST_STRING(TYPE_HEXE_LISP,					"$hexeLisp");
DECLARE_CONST_STRING(TYPE_EVENT_HANDLER_CALL,			"eventHandlerCall");

DECLARE_CONST_STRING(ERR_ASYNC_REQUEST_NOT_ALLOWED,		"Async requests are not allowed.");
DECLARE_CONST_STRING(ERR_COMPUTE_CRASH,					"Compute runtime crash.");
DECLARE_CONST_STRING(ERR_LIBRARY_NOT_FOUND,				"Library not found: %s.");
DECLARE_CONST_STRING(ERR_HEXE_CODE_EXPECTED,			"Unable to execute term: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN,						"Unknown error.");
DECLARE_CONST_STRING(ERR_CANT_NEST_EVENT_HANDLER,		"Unable to invoke event handler while inside an event handler.");
DECLARE_CONST_STRING(ERR_INVALID_EVENT_HANDLER,			"Invalid event handler: not a function.");
DECLARE_CONST_STRING(ERR_EXCEEDED_ARRAY_LIMITS,			"Arrays cannot have more than %s elements.");

IHexeVMHost CHexeProcess::DefaultHost;

CHexeProcess::CHexeProcess (IHexeVMHost& Host) :
		m_Host(Host)

//	CHexeProcess constructor

	{
	InitInstructionTable();
	}

CDatum CHexeProcess::AsDatum (const SLimits& Limits)

//	AsDatum
//
//	Persist to datum.

	{
	//	We store compute limits as a single array (for efficiency). The first
	//	element in the array is a version number, so that we can upgrade the
	//	format in the future.

	CDatum dLimits(CDatum::typeArray);
	dLimits.Append(PROGRAM_LIMITS_VERSION);
	dLimits.Append(Limits.iMaxArrayLen);
	dLimits.Append(Limits.iMaxExecutionTimeSec);
	dLimits.Append(Limits.iMaxStackDepth);
	dLimits.Append(Limits.iMaxStringSize);
	dLimits.Append(Limits.iMaxBufferSize);

	return dLimits;
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
		m_Env.GetGlobalEnv().SetAt(sIdentifier, dValue);
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
	m_Env.Init(CDatum(new CHexeGlobalEnvironment));
	m_GlobalEnvCache.DeleteAll();

	m_Libraries.DeleteAll();
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

	DWORD dwID;
	if (!pEnv->FindSymbol(sIdentifier, &dwID))
		return false;

	if (retdValue)
		*retdValue = pEnv->GetAt(dwID);

	return true;
	}

void CHexeProcess::GetCurrentSecurityCtx (CHexeSecurityCtx *retCtx)

//	GetCurrentSecurityCtx
//
//	Gets the current security context based on the global environment and
//	user security.

	{
	m_Env.GetGlobalEnv().GetServiceSecurity(retCtx);
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

CString CHexeProcess::GetErrorMsg (EErrorMsg iError) const

//	GetErrorMsg
//
//	Returns the given error message.

	{
	switch (iError)
		{
		case EErrorMsg::ArrayLimit:
			return strPattern(ERR_EXCEEDED_ARRAY_LIMITS, ::strFormatInteger(GetLimits().iMaxArrayLen, -1, FORMAT_THOUSAND_SEPARATOR));

		default:
			throw CException(errFail);
		}
	}

CDatum CHexeProcess::GetStringFromDataBlock (int iID)

//	GetStringFromDataBlock
//
//	Return string from the data cache.

	{
	return m_pCodeBank->GetDatumFromID(iID);
	}

CDatum CHexeProcess::GetVMInfo () const

//	GetVMInfo
//
//	Returns VM status.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_STACK_LEVEL, m_Stack.GetCount());
	dResult.SetElement(FIELD_AEON_TYPES, CAEONTypes::GetCount());

	return dResult;
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
		CHexeGlobalEnvironment *pGlobalEnv = new CHexeGlobalEnvironment(*pSrcGlobalEnv);
		m_dGlobalEnv = CDatum(pGlobalEnv);
		}

	return true;
	}

bool CHexeProcess::InitFrom (CDatum dSerialized, CString *retsError)

//	InitFrom
//
//	Initialize from a serialized process.

	{
	DeleteAll();

	if (!m_Types.InitFrom(dSerialized.GetElement(FIELD_TYPES), retsError))
		return false;

	CDatum dLibraries = dSerialized.GetElement(FIELD_LIBRARIES);
	for (int i = 0; i < dLibraries.GetCount(); i++)
		{
		if (!LoadLibrary(dLibraries.GetElement(i).AsStringView(), retsError))
			return false;
		}

	m_Limits = InitLimitsFromDatum(dSerialized.GetElement(FIELD_LIMITS));
	m_bAddConcatenatesStrings = !dSerialized.GetElement(FIELD_ADD_CONCATENATES_STRINGS).IsNil();

	//	If we have maxExecutionTime, then this is a backwards compatible process.

	CDatum dMaxExecutionTime = dSerialized.GetElement(FIELD_MAX_EXECUTION_TIME);
	if (!dMaxExecutionTime.IsNil())
		{
		m_Limits.iMaxExecutionTimeSec = ((int)dMaxExecutionTime) / 1000;
		}

	//	If we have an abort time, then we convert to an absolute time.

	DWORDLONG dwNow = sysGetTickCount64();
	DWORDLONG dwAbortTime = dSerialized.GetElement(FIELD_ABORT_TIME);
	if (dwAbortTime > 0)
		m_dwAbortTime = dwNow + dwAbortTime;

	//	Success!

	return true;
	}

IInvokeCtx::SLimits CHexeProcess::InitLimitsFromDatum (CDatum dData)

//	InitFromDatum
//
//	Intialize limits structure from datum.

	{
	IInvokeCtx::SLimits Limits;

	if (!dData.IsNil())
		{
		Limits.iMaxArrayLen = dData.GetElement(1);
		Limits.iMaxExecutionTimeSec = dData.GetElement(2);
		Limits.iMaxStackDepth = dData.GetElement(3);
		Limits.iMaxStringSize = dData.GetElement(4);
		Limits.iMaxBufferSize = dData.GetElement(5);
		if (Limits.iMaxBufferSize == 0)
			Limits.iMaxBufferSize = 100'000'000;	//	Default for backwards compatibility
		}

	return Limits;
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

		m_Env.SetGlobalEnv(m_dGlobalEnv, pGlobalEnv);
		m_GlobalEnvCache.DeleteAll();

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
			case ERun::ForcedTerminate:
				*retsError = dResult.AsStringView();
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
	InitGlobalEnv();

	m_Libraries.Insert(sName);

	DWORD dwLibraryID;
	if (!g_HexeLibrarian.FindLibrary(sName, &dwLibraryID))
		{
		if (retsError)
			*retsError = strPattern(ERR_LIBRARY_NOT_FOUND, sName);
		return false;
		}

	for (int i = 0; i < g_HexeLibrarian.GetEntryCount(dwLibraryID); i++)
		{
		CDatum dFunction;
		const CString &sFunction = g_HexeLibrarian.GetEntry(dwLibraryID, i, &dFunction);

		m_Env.GetGlobalEnv().SetAt(sFunction, dFunction);
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
	m_Types.Mark();

	m_dGlobalEnv.Mark();
	m_Env.Mark();
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
	//	First we generate code to call the function

	CDatum dExpression;
	CHexeCode::CreateFunctionCall(Args.GetCount(), &dExpression);

	//	Set up the stack. The function goes first followed by the args

	m_Stack.DeleteAll();
	m_Stack.Push(dFunc);

	for (int i = 0; i < Args.GetCount(); i++)
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

	//	If in an event handler, then we're done.

	if (m_iEventHandlerLevel == -1)
		{
		*retResult = dAsyncResult;
		m_iEventHandlerLevel = 0;
		return ERun::EventHandlerDone;
		}

	//	Push the result on the stack

	m_Stack.Push(dAsyncResult);

	//	Progress

	if (m_pComputeProgress)
		m_pComputeProgress->OnStart();

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

	if (m_pComputeProgress)
		m_pComputeProgress->OnStop();

	return iRun;
	}

CHexeProcess::ERun CHexeProcess::RunContinuesFromStopCheck (CDatum& retResult)

//	RunContinuesFromStopCheck
//
//	Continue running after an interruption

	{
	//	Progress

	if (m_pComputeProgress)
		m_pComputeProgress->OnStart();

	//	Continue execution

	ERun iRun;
	try
		{
		iRun = Execute(&retResult);
		}
	catch (...)
		{
		retResult = strPattern(ERR_COMPUTE_CRASH);
		return ERun::Error;
		}

	if (m_pComputeProgress)
		m_pComputeProgress->OnStop();

	return iRun;
	}

CHexeProcess::ERun CHexeProcess::RunEventHandler (CDatum dFunc, const TArray<CDatum> &Args, CDatum &retResult)

//	RunEventHandler
//
//	Saves the current execution context and sets up execution to call the given
//	function and arguments.
//
//	When the function returns Run will exit with InputRequest.

	{
	//	Validate function

	CDatum dNewCodeBank;
	DWORD *pNewIP;
	CDatum::ECallType iFuncType = dFunc.GetCallInfo(&dNewCodeBank, &pNewIP);
	switch (iFuncType)
		{
		case CDatum::ECallType::Call:
		case CDatum::ECallType::CachedCall:
			{
			//	This in the call stack so that we know what to do when we return.

			m_CallStack.PushSysCall(m_dExpression, m_dCodeBank, m_pIP, m_dExpression, TYPE_EVENT_HANDLER_CALL, 0);

			//	Set up environment

			m_Env.PushNewFrame();

			for (int i = 0; i < Args.GetCount(); i++)
				m_Env.GetLocalEnv().SetArgumentValue(0, i, Args[i]);

			//	Make the call

			m_dExpression = dFunc;
			SetCodeBank(dNewCodeBank);

			m_pIP = pNewIP;

			//	Remember that we're in an event handler.

			m_iEventHandlerLevel++;

			//	Progress

			if (m_pComputeProgress)
				m_pComputeProgress->OnStart();

			//	Run

			ERun iRun;
			try
				{
				iRun = Execute(&retResult);
				}
			catch (...)
				{
				retResult = strPattern(ERR_COMPUTE_CRASH);
				return ERun::Error;
				}

			if (m_pComputeProgress)
				m_pComputeProgress->OnStop();

			return iRun;
			}

		case CDatum::ECallType::Library:
			{
			//	Set up environment

			m_Env.PushNewFrame();

			for (int i = 0; i < Args.GetCount(); i++)
				m_Env.GetLocalEnv().SetArgumentValue(0, i, Args[i]);

			//	Make the call

			DWORDLONG dwStart = ::sysGetTickCount64();

			SAEONInvokeResult Result;
			CDatum::InvokeResult iResult = dFunc.Invoke(this, m_Env.GetLocalEnv(), m_UserSecurity.GetExecutionRights(), Result);

			m_dwLibraryTime += ::sysGetTickCount64() - dwStart;

			//	Remember that we're in an event handler.

			m_iEventHandlerLevel = -1;	//	-1 means a library call

			if (iResult != CDatum::InvokeResult::ok)
				return ExecuteHandleInvokeResult(iResult, dFunc, Result, &retResult, FLAG_NO_ADVANCE);

			//	Restore the environment

			m_Env.PopFrame();

			//	Done

			m_iEventHandlerLevel = 0;
			retResult = Result.dResult;
			return ERun::EventHandlerDone;
			}

		default:
			retResult = CDatum::CreateError(ERR_INVALID_EVENT_HANDLER);
			return ERun::Error;
		}
	}

CHexeProcess::ERun CHexeProcess::RunEntryPoint (CStringView sEntryPoint, const TArray<CDatum>& Args, CDatum& retResult)

//	RunEntryPoint
//
//	Runs the given function by name.

	{
	//	First we generate code to call the function

	CDatum dExpression;
	CHexeCode::CreateFunctionCall(sEntryPoint, Args.GetCount(), dExpression);

	//	Set up the stack. The function goes first followed by the args

	m_Stack.DeleteAll();

	for (int i = 0; i < Args.GetCount(); i++)
		m_Stack.Push(Args[i]);

	//	Now call the function

	return RunWithStack(dExpression, &retResult);
	}

CHexeProcess::ERun CHexeProcess::RunWithStack (CDatum dExpression, CDatum *retResult)

//	RunWithStack
//
//	Runs the given entrypoint assuming that the stack has been initialized
//	previously.

	{
	//	Prepare the context

	m_dExpression = dExpression;
	m_CallStack.DeleteAll();
	m_Env.DeleteAll();

	//	Initialize the instruction pointer

	CHexeFunction *pExpression = CHexeFunction::Upconvert(dExpression);
	if (pExpression == NULL)
		{
		*retResult = strPattern(ERR_HEXE_CODE_EXPECTED, dExpression.AsString());
		return ERun::Error;
		}

	CDatum dCodeBank;
	m_pIP = pExpression->GetCode(&dCodeBank);
	if (m_pIP == NULL)
		{
		*retResult = strPattern(ERR_HEXE_CODE_EXPECTED, dExpression.AsString());
		return ERun::Error;
		}

	if (!SetCodeBank(dCodeBank))
		{
		*retResult = strPattern(ERR_HEXE_CODE_EXPECTED, dExpression.AsString());
		return ERun::Error;
		}

	//	Initialize the environment

	InitGlobalEnv();
	m_Env.Init(m_dGlobalEnv);

	//	Progress

	if (m_pComputeProgress)
		m_pComputeProgress->OnStart();

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

	if (m_pComputeProgress)
		m_pComputeProgress->OnStop();

	return iRun;
	}

CDatum CHexeProcess::Serialize () const

//	Serialize
//
//	Serialize into a struct.

	{
	CDatum dResult(CDatum::typeStruct);

	dResult.SetElement(FIELD_TYPES, m_Types.Serialize());

	CDatum dLibraries(CDatum::typeArray);
	for (int i = 0; i < m_Libraries.GetCount(); i++)
		dLibraries.Append(m_Libraries[i]);

	dResult.SetElement(FIELD_LIBRARIES, dLibraries);

	dResult.SetElement(FIELD_LIMITS, AsDatum(m_Limits));
	if (m_bAddConcatenatesStrings)
		dResult.SetElement(FIELD_ADD_CONCATENATES_STRINGS, CDatum(true));

	//	We serialize abort time as the number of milliseconds left in our 
	//	execution quota.

	if (m_dwAbortTime > 0)
		{
		//	NOTE: If for some reason we're past the abort time, then we just
		//	serialize 1 second left.

		DWORDLONG dwNow = ::sysGetTickCount64();
		DWORDLONG dwTimeLeft = (m_dwAbortTime > dwNow ? m_dwAbortTime - dwNow : 1000);
		dResult.SetElement(FIELD_ABORT_TIME, dwTimeLeft);
		}

	//	Done

	return dResult;
	}

bool CHexeProcess::SetCodeBank (CDatum dCodeBank)

//	SetCodeBank
//
//	Sets the new codebank

	{
	auto pCodeBank = CHexeCode::Upconvert(dCodeBank);
	if (!pCodeBank)
		return false;

	if (pCodeBank != m_pCodeBank)
		m_GlobalEnvCache.DeleteAll();

	m_dCodeBank = dCodeBank;
	m_pCodeBank = pCodeBank;

	return true;
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
