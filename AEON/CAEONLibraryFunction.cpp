//	CAEONLibraryFunction.cpp
//
//	CAEONLibraryFunction class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_RESULT,						"result");
DECLARE_CONST_STRING(FIELD_TYPE,						"type")

DECLARE_CONST_STRING(TYPE_DIALOG_SIM,					"dialogSim");
DECLARE_CONST_STRING(TYPE_HEXARC_MSG,					"hexarcMsg");
DECLARE_CONST_STRING(TYPE_INPUT_REQUEST,				"inputRequest");

DECLARE_CONST_STRING(TYPENAME_LIBRARY_FUNCTION,			"libraryFunction");

DECLARE_CONST_STRING(ERR_CRASH,							"Crash in primitive %s.")
DECLARE_CONST_STRING(ERR_NOT_ALLOWED,					"You are not authorized to call %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_TYPE,					"Unknown invoke result type: %s.");

const CString &CAEONLibraryFunction::GetTypename (void) const { return TYPENAME_LIBRARY_FUNCTION; }

CDatum::InvokeResult CAEONLibraryFunction::HandleSpecialResult (SAEONInvokeResult& retResult)

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
		else if (strEquals(sType, TYPE_INPUT_REQUEST))
			{
			retResult.iResult = CDatum::InvokeResult::runInputRequest;
			return retResult.iResult;
			}
		else if (strEquals(sType, TYPE_DIALOG_SIM))
			{
			retResult.iResult = CDatum::InvokeResult::runInputRequestDebugSim;
			retResult.dResult = retResult.dResult.GetElement(FIELD_RESULT);
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

CDatum::InvokeResult CAEONLibraryFunction::Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult)

//	Invoke
//
//	Invoke the library function

	{
	ASSERT(retResult.iResult == CDatum::InvokeResult::unknown);

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
		//	NOTE: When we call m_fnInvoke we expect retResult.iResult to be set
		//	to a valid value if the function wants to return a special result.
		//	For backwards compatibility, and because we're lazy, a value of
		//	unknown means use the old method.

		if (!m_fnInvoke(*pCtx, m_dwData, Stack, CDatum(), CDatum(), retResult))
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

CDatum::InvokeResult CAEONLibraryFunction::InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult)

//	InvokeContinues
//
//	Invoke the library function

	{
	try
		{
		CHexeStackEnv Dummy;
		if (!m_fnInvoke(*pCtx, m_dwData, Dummy, dContext, dResult, retResult))
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

CDatum::InvokeResult CAEONLibraryFunction::InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult)

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
		if (!m_fnInvoke(Ctx, m_dwData, LocalEnv, CDatum(), CDatum(), retResult))
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

void CAEONLibraryFunction::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	We cannot propertly serialize this object because we cannot serialize the 
//	function pointer. But we need to handle this in case the object ever gets
//	printed (or something).

	{
	CDatum dResult(m_sName);
	dResult.Serialize(iFormat, Stream);
	}

void CAEONLibraryFunction::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	We serialize as a string, because we can't serialize the library 
	//	function.
	//
	//	LATER: We could somehow serialize an identifier for the function and
	//	then reload.

	CDatum dResult(m_sName);
	dResult.SerializeAEON(Stream, Serialized);
	}
