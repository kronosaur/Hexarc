//	CArrayMapProcessor.cpp
//
//	CArrayMapProcessor Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ALLOW_NULL,					"allowNull");

DECLARE_CONST_STRING(TYPENAME_ARRAY_MAP_PROC,			"arrayMapProcessor");
const CString &CArrayMapProcessor::StaticGetTypename (void) { return TYPENAME_ARRAY_MAP_PROC; }

CArrayMapProcessor::CArrayMapProcessor (CDatum dArray, CDatum dOptions, CDatum dMapFunc) :
		m_dArray(dArray),
		m_dOptions(dOptions),
		m_dMapFunc(dMapFunc),
		m_bAllowNull(dOptions.GetElement(FIELD_ALLOW_NULL).AsBool())

//	CArrayMapProcess constructor

	{
	}

void CArrayMapProcessor::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dArray.Mark();
	m_dOptions.Mark();
	m_dMapFunc.Mark();
	m_dResult.Mark();
	}

bool CArrayMapProcessor::Process (CDatum dSelf, SAEONInvokeResult& retResult)

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

	if (!m_dArray.IsContainer())
		{
		retResult.dResult = CDatum();
		return true;
		}
	else if (m_dArray.GetCount() == 0)
		{
		retResult.dResult = CDatum(CDatum::typeArray);
		return true;
		}

	//	Initialize our state.

	m_dResult = CDatum(CDatum::typeArray);
	m_dResult.GrowToFit(m_dArray.GetCount());
	m_iPos = 0;

	//	Run the mapping function

	return CHexe::RunFunction2Args(m_dMapFunc, m_dArray.GetElement(m_iPos), CDatum(m_iPos), dSelf, retResult);
	}

bool CArrayMapProcessor::ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult)

//	ProcessContinues
//
//	Handle the next element.

	{
	//	Add the mapped result.

	if (!dResult.IsIdenticalToNil() || m_bAllowNull)
		m_dResult.Append(dResult);

	//	Are we done?

	m_iPos++;
	if (m_iPos >= m_dArray.GetCount())
		{
		retResult.dResult = m_dResult;
		return true;
		}

	//	Continue mapping

	return CHexe::RunFunction2Args(m_dMapFunc, m_dArray.GetElement(m_iPos), CDatum(m_iPos), dSelf, retResult);
	}
