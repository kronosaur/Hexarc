//	CHexeLibraryFunction.cpp
//
//	CHexeLibraryFunction class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CRASH,							"Crash in primitive %s.")
DECLARE_CONST_STRING(ERR_NOT_ALLOWED,					"You are not authorized to call %s.")

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

bool CHexeLibraryFunction::Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult)

//	Invoke
//
//	Invoke the library function

	{
	//	Make sure we have the execution rights required by this primitive

	if ((m_dwExecutionRights & dwExecutionRights) != m_dwExecutionRights)
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_NOT_ALLOWED, m_sName), retdResult);
		return false;
		}

	//	Invoke

	try
		{
		return m_pfFunc(pCtx, m_dwData, dLocalEnv, CDatum(), retdResult);
		}
	catch (...)
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_CRASH, m_sName), retdResult);
		return false;
		}
	}

bool CHexeLibraryFunction::InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult)

//	InvokeContinues
//
//	Invoke the library function

	{
	try
		{
		return m_pfFunc(pCtx, m_dwData, dResult, dContext, retdResult);
		}
	catch (...)
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_CRASH, m_sName), retdResult);
		return false;
		}
	}
