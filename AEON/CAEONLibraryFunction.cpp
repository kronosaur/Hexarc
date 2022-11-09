//	CAEONLibraryFunction.cpp
//
//	CAEONLibraryFunction class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_TYPE,						"type")

DECLARE_CONST_STRING(TYPE_HEXARC_MSG,					"hexarcMsg");
DECLARE_CONST_STRING(TYPE_INPUT_REQUEST,				"inputRequest");

DECLARE_CONST_STRING(TYPENAME_LIBRARY_FUNCTION,			"libraryFunction");

DECLARE_CONST_STRING(ERR_CRASH,							"Crash in primitive %s.")
DECLARE_CONST_STRING(ERR_NOT_ALLOWED,					"You are not authorized to call %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_TYPE,					"Unknown invoke result type: %s.");

const CString &CAEONLibraryFunction::GetTypename (void) const { return TYPENAME_LIBRARY_FUNCTION; }

CDatum::InvokeResult CAEONLibraryFunction::HandleSpecialResult (CDatum *retdResult)

//	HandleResult
//
//	Converts to appropriate result.

	{
	if (retdResult->IsError())
		return CDatum::InvokeResult::error;

	else if (retdResult->GetBasicType() == CDatum::typeArray)
		return CDatum::InvokeResult::runFunction;

	else if (retdResult->GetBasicType() == CDatum::typeString)
		{
		const CString &sType = *retdResult;
		if (strEquals(sType, TYPE_INPUT_REQUEST))
			{
			return CDatum::InvokeResult::runInputRequest;
			}
		else
			{
			*retdResult = CDatum::CreateError(strPattern(ERR_UNKNOWN_TYPE, sType));
			return CDatum::InvokeResult::error;
			}
		}
	else if (retdResult->GetBasicType() == CDatum::typeStruct)
		{
		const CString &sType = retdResult->GetElement(FIELD_TYPE);
		if (strEquals(sType, TYPE_HEXARC_MSG))
			{
			return CDatum::InvokeResult::runInvoke;
			}
		else if (strEquals(sType, TYPE_INPUT_REQUEST))
			{
			return CDatum::InvokeResult::runInputRequest;
			}
		else
			{
			*retdResult = CDatum::CreateError(strPattern(ERR_UNKNOWN_TYPE, sType));
			return CDatum::InvokeResult::error;
			}
		}
	else
		return CDatum::InvokeResult::error;
	}

CDatum::InvokeResult CAEONLibraryFunction::Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult)

//	Invoke
//
//	Invoke the library function

	{
	//	Make sure we have the execution rights required by this primitive

	if ((m_dwExecutionRights & dwExecutionRights) != m_dwExecutionRights)
		{
		*retdResult = CDatum::CreateError(strPattern(ERR_NOT_ALLOWED, m_sName));
		return CDatum::InvokeResult::error;
		}

	//	Invoke

	try
		{
		if (!m_fnInvoke(*pCtx, m_dwData, dLocalEnv, CDatum(), *retdResult))
			return HandleSpecialResult(retdResult);

		return CDatum::InvokeResult::ok;
		}
	catch (...)
		{
		*retdResult = CDatum::CreateError(strPattern(ERR_CRASH, m_sName));
		return CDatum::InvokeResult::error;
		}
	}

CDatum::InvokeResult CAEONLibraryFunction::InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult)

//	InvokeContinues
//
//	Invoke the library function

	{
	try
		{
		if (!m_fnInvoke(*pCtx, m_dwData, dResult, dContext, *retdResult))
			return HandleSpecialResult(retdResult);

		return CDatum::InvokeResult::ok;
		}
	catch (...)
		{
		*retdResult = CDatum::CreateError(strPattern(ERR_CRASH, m_sName));
		return CDatum::InvokeResult::error;
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
