//	CHexeLibraryFunction.cpp
//
//	CHexeLibraryFunction class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_RESULT,						"result");
DECLARE_CONST_STRING(FIELD_TYPE,						"type")

DECLARE_CONST_STRING(TYPE_DIALOG_SIM,					"dialogSim");
DECLARE_CONST_STRING(TYPE_HEXARC_MSG,					"hexarcMsg")
DECLARE_CONST_STRING(TYPE_INPUT_REQUEST,				"inputRequest")

DECLARE_CONST_STRING(ERR_CRASH,							"Crash in primitive %s.")
DECLARE_CONST_STRING(ERR_NOT_ALLOWED,					"You are not authorized to call %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_TYPE,					"Unknown invoke result type: %s.")

DECLARE_CONST_STRING(TYPENAME_HEXE_LIBRARY_FUNCTION,	"hexeLibraryFunction")
const CString &CHexeLibraryFunction::StaticGetTypename (void) { return TYPENAME_HEXE_LIBRARY_FUNCTION; }

void CHexeLibraryFunction::Create (const SLibraryFuncDef &Def, CDatum *retdFunc)

//	Create
//
//	Creates a new library function

	{
	CHexeLibraryFunction *pFunc = new CHexeLibraryFunction;
	pFunc->m_sName = Def.sName;
	pFunc->m_pfFunc = Def.pfFunc;
	pFunc->m_dwData = Def.dwData;
	pFunc->m_sArgList = Def.sArgList;
	pFunc->m_sHelpLine = Def.sHelpLine;
	pFunc->m_dwExecFlags = Def.dwExecFlags;

	pFunc->m_dDatatype = CAEONTypes::CreateFunctionType(Def.sArgList);

	*retdFunc = CDatum(pFunc);
	}

CDatum::InvokeResult CHexeLibraryFunction::HandleSpecialResult (SAEONInvokeResult& retResult)

//	HandleResult
//
//	Converts to appropriate result.

	{
	//	If the result is already set, then return it.
	//	LATER: Eventually we should migrate all library functions
	//	to the new system.

	if (retResult.iResult != CDatum::InvokeResult::unknown)
		return retResult.iResult;

	else if (retResult.dResult.IsError())
		{
		retResult.iResult = CDatum::InvokeResult::error;
		return retResult.iResult;
		}

	else if (retResult.dResult.GetBasicType() == CDatum::typeArray)
		{
		retResult.iResult = CDatum::InvokeResult::runFunction;
		return retResult.iResult;
		}

	else if (retResult.dResult.GetBasicType() == CDatum::typeString)
		{
		CStringView sType = retResult.dResult;
		if (strEquals(sType, TYPE_INPUT_REQUEST))
			{
			retResult.iResult = CDatum::InvokeResult::runInputRequest;
			return retResult.iResult;
			}
		else
			{
			retResult.iResult = CDatum::InvokeResult::error;
			retResult.dResult = CDatum::CreateError(strPattern(ERR_UNKNOWN_TYPE, sType));
			return retResult.iResult;
			}
		}
	else if (retResult.dResult.GetBasicType() == CDatum::typeStruct)
		{
		CStringView sType = retResult.dResult.GetElement(FIELD_TYPE);
		if (strEquals(sType, TYPE_HEXARC_MSG))
			{
			retResult.iResult = CDatum::InvokeResult::runInvoke;
			return retResult.iResult;
			}
		else if (strEquals(sType, TYPE_DIALOG_SIM))
			{
			retResult.iResult = CDatum::InvokeResult::runInputRequestDebugSim;
			retResult.dResult = retResult.dResult.GetElement(FIELD_RESULT);
			return retResult.iResult;
			}
		else if (strEquals(sType, TYPE_INPUT_REQUEST))
			{
			retResult.iResult = CDatum::InvokeResult::runInputRequest;
			return retResult.iResult;
			}
		else
			{
			retResult.iResult = CDatum::InvokeResult::error;
			retResult.dResult = CDatum::CreateError(strPattern(ERR_UNKNOWN_TYPE, sType));
			return retResult.iResult;
			}
		}
	else
		{
		retResult.iResult = CDatum::InvokeResult::error;
		return retResult.iResult;
		}
	}

CDatum::InvokeResult CHexeLibraryFunction::Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult)

//	Invoke
//
//	Invoke the library function

	{
	TArray<CDatum> Args;
	for (int i = 0; i < LocalEnv.GetCount(); i++)
		Args.Insert(LocalEnv.GetArgument(i));

	CHexeStackEnv Stack(std::move(Args));

	//	Make sure we have the execution rights required by this primitive

	if (!IInvokeCtx::CanExecute(m_dwExecFlags, dwExecutionRights))
		{
		retResult.iResult = CDatum::InvokeResult::error;
		retResult.dResult = CDatum::CreateError(strPattern(ERR_NOT_ALLOWED, m_sName));
		return retResult.iResult;
		}

	//	Invoke

	try
		{
		if (!m_pfFunc(pCtx, m_dwData, Stack, CDatum(), CDatum(), retResult))
			return HandleSpecialResult(retResult);

		retResult.iResult = CDatum::InvokeResult::ok;
		return retResult.iResult;
		}
	catch (...)
		{
		retResult.iResult = CDatum::InvokeResult::error;
		retResult.dResult = CDatum::CreateError(strPattern(ERR_CRASH, m_sName));
		return retResult.iResult;
		}
	}

CDatum::InvokeResult CHexeLibraryFunction::InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult)

//	InvokeContinues
//
//	Invoke the library function

	{
	try
		{
		CHexeStackEnv Dummy;
		if (!m_pfFunc(pCtx, m_dwData, Dummy, dContext, dResult, retResult))
			return HandleSpecialResult(retResult);

		retResult.iResult = CDatum::InvokeResult::ok;
		return retResult.iResult;
		}
	catch (...)
		{
		retResult.iResult = CDatum::InvokeResult::error;
		retResult.dResult = CDatum::CreateError(strPattern(ERR_CRASH, m_sName));
		return retResult.iResult;
		}
	}

CDatum::InvokeResult CHexeLibraryFunction::InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult)

//	Invoke
//
//	Invoke the library function

	{
	//	Make sure we have the execution rights required by this primitive

	if (!IInvokeCtx::CanExecute(m_dwExecFlags, dwExecutionRights))
		{
		retResult.iResult = CDatum::InvokeResult::error;
		retResult.dResult = CDatum::CreateError(strPattern(ERR_NOT_ALLOWED, m_sName));
		return retResult.iResult;
		}

	//	Invoke

	try
		{
		if (!m_pfFunc(&Ctx, m_dwData, LocalEnv, CDatum(), CDatum(), retResult))
			return HandleSpecialResult(retResult);

		retResult.iResult = CDatum::InvokeResult::ok;
		return retResult.iResult;
		}
	catch (...)
		{
		retResult.iResult = CDatum::InvokeResult::error;
		retResult.dResult = CDatum::CreateError(strPattern(ERR_CRASH, m_sName));
		return retResult.iResult;
		}
	}

void CHexeLibraryFunction::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	We cannot propertly serialize this object because we cannot serialize the 
//	function pointer. But we need to handle this in case the object ever gets
//	printed (or something).

	{
	CDatum dResult(m_sName);
	dResult.Serialize(iFormat, Stream);
	}
