//	CHexeLibraryFunction.cpp
//
//	CHexeLibraryFunction class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_TYPE,						"type")

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
	pFunc->m_dwExecutionRights = Def.dwExecutionRights;

	*retdFunc = CDatum(pFunc);
	}

CDatum::InvokeResult CHexeLibraryFunction::HandleSpecialResult (CDatum *retdResult)

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
			CHexeError::Create(NULL_STR, strPattern(ERR_UNKNOWN_TYPE, sType), retdResult);
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
			CHexeError::Create(NULL_STR, strPattern(ERR_UNKNOWN_TYPE, sType), retdResult);
			return CDatum::InvokeResult::error;
			}
		}
	else
		return CDatum::InvokeResult::error;
	}

CDatum::InvokeResult CHexeLibraryFunction::Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult)

//	Invoke
//
//	Invoke the library function

	{
	//	Make sure we have the execution rights required by this primitive

	if ((m_dwExecutionRights & dwExecutionRights) != m_dwExecutionRights)
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_NOT_ALLOWED, m_sName), retdResult);
		return CDatum::InvokeResult::error;
		}

	//	Invoke

	try
		{
		if (!m_pfFunc(pCtx, m_dwData, dLocalEnv, CDatum(), retdResult))
			return HandleSpecialResult(retdResult);

		return CDatum::InvokeResult::ok;
		}
	catch (...)
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_CRASH, m_sName), retdResult);
		return CDatum::InvokeResult::error;
		}
	}

CDatum::InvokeResult CHexeLibraryFunction::InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult)

//	InvokeContinues
//
//	Invoke the library function

	{
	try
		{
		if (!m_pfFunc(pCtx, m_dwData, dResult, dContext, retdResult))
			return HandleSpecialResult(retdResult);

		return CDatum::InvokeResult::ok;
		}
	catch (...)
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_CRASH, m_sName), retdResult);
		return CDatum::InvokeResult::error;
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
