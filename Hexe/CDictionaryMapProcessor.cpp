//	CDictionaryMapProcessor.cpp
//
//	CDictionaryMapProcessor Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ALLOW_NULL,					"allowNull");

DECLARE_CONST_STRING(TYPENAME_ARRAY_MAP_PROC,			"arrayMapProcessor");
const CString &CDictionaryMapProcessor::StaticGetTypename (void) { return TYPENAME_ARRAY_MAP_PROC; }

CDictionaryMapProcessor::CDictionaryMapProcessor (CDatum dDictionary, CDatum dOptions, CDatum dMapFunc, int iFuncArgs, CDatum dResultType) :
		m_dDictionary(dDictionary),
		m_dOptions(dOptions),
		m_dMapFunc(dMapFunc),
		m_iFuncArgs(iFuncArgs),
		m_bAllowNull(dOptions.GetElement(FIELD_ALLOW_NULL).AsBool()),
		m_dResult(CDatum::CreateDictionary(dResultType))

//	CArrayMapProcess constructor

	{
	}

void CDictionaryMapProcessor::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dDictionary.Mark();
	m_dOptions.Mark();
	m_dMapFunc.Mark();
	m_dResult.Mark();
	}

bool CDictionaryMapProcessor::Process (CDatum dSelf, SAEONInvokeResult &retResult)

//	Process
//
//	We return under the following conditions:
//
//	1.	If processing has completed, we return TRUE and retResult has
//		the result (mapped array).
//
//	2.	If we need to call a lambda function to continue processing, we return
//		FALSE and retResult has the proper calling parameters (ready for
//		Compute.
//
//	3.	If there is an error we return FALSE and retResult is an error string.

	{
	//	Must be a list. If not, we just return nil

	if (!m_dDictionary.IsContainer())
		{
		retResult.dResult = CDatum();
		return true;
		}
	else if (m_dDictionary.GetCount() == 0)
		{
		retResult.dResult = m_dResult;
		return true;
		}

	m_dResult.GrowToFit(m_dDictionary.GetCount());
	m_iPos = 0;

	//	Run the mapping function

	if (m_iFuncArgs == 1)
		return CHexe::RunFunction1Arg(m_dMapFunc, m_dDictionary.GetElement(m_iPos), dSelf, retResult);
	else
		return CHexe::RunFunction2Args(m_dMapFunc, m_dDictionary.GetKeyEx(m_iPos), m_dDictionary.GetElement(m_iPos), dSelf, retResult);
	}

bool CDictionaryMapProcessor::ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult &retResult)

//	ProcessContinues
//
//	Handle the next element.

	{
	//	Add the mapped result.

	if (!dResult.IsIdenticalToNil() || m_bAllowNull)
		m_dResult.SetElementAt(m_dDictionary.GetKeyEx(m_iPos), dResult);

	//	Are we done?

	m_iPos++;
	if (m_iPos >= m_dDictionary.GetCount())
		{
		retResult.dResult = m_dResult;
		return true;
		}

	//	Continue mapping

	if (m_iFuncArgs == 1)
		return CHexe::RunFunction1Arg(m_dMapFunc, m_dDictionary.GetElement(m_iPos), dSelf, retResult);
	else
		return CHexe::RunFunction2Args(m_dMapFunc, m_dDictionary.GetKeyEx(m_iPos), m_dDictionary.GetElement(m_iPos), dSelf, retResult);
	}
