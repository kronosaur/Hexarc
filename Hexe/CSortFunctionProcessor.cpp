//	CSortFunctionProcessor.cpp
//
//	CSortFunctionProcessor class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ASCENDING,					"ascending")
DECLARE_CONST_STRING(FIELD_DESCENDING,					"descending")
DECLARE_CONST_STRING(FIELD_GET_KEY_VALUE,				"getKeyValue")
DECLARE_CONST_STRING(FIELD_ORDER,						"order")

DECLARE_CONST_STRING(ERR_INVALID_OPTION,				"Invalid sort! option: %s.")
DECLARE_CONST_STRING(ERR_MUST_BE_ARRAY,					"Unable to sort something that is not a list.")

DECLARE_CONST_STRING(TYPENAME_SORT_PROC,				"sortProcessor")
const CString &CSortFunctionProcessor::StaticGetTypename (void) { return TYPENAME_SORT_PROC; }

CSortFunctionProcessor::CSortFunctionProcessor (CDatum dList, CDatum dParams) :
		m_dList(dList),
		m_dParams(dParams)

//	CSortFunctionProcessor constructor

	{
	}

void CSortFunctionProcessor::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	int i;

	m_dList.Mark();
	m_dParams.Mark();

	for (i = 0; i < m_ValueList.GetCount(); i++)
		m_ValueList[i].dKey.Mark();
	}

bool CSortFunctionProcessor::Process (CDatum dSelf, CDatum *retResult)

//	Process
//
//	We return under the following conditions:
//
//	1.	If processing has completed, we return TRUE and retResult has
//		the result (html text).
//
//	2.	If we need to call a lambda function to continue processing, we return
//		FALSE and retResult has the proper calling parameters (ready for
//		Compute.
//
//	3.	If there is an error we return FALSE and retResult is an error string.

	{
	//	Must be a list. If not, we just return the value unsorted

	if (m_dList.GetBasicType() != CDatum::typeArray
			|| m_dList.GetCount() < 2)
		{
		*retResult = m_dList;
		return true;
		}

	//	Ascending or descending?

	CDatum dSort = m_dParams.GetElement(FIELD_ORDER);
	if (dSort.IsNil() || strEquals(dSort, FIELD_ASCENDING))
		m_iSort = AscendingSort;
	else if (strEquals(dSort, FIELD_DESCENDING))
		m_iSort = DescendingSort;
	else
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_OPTION, dSort.AsString()), retResult);
		return false;
		}

	//	If we have a getKeyValue lambda then we need to process that.

	if (!(m_dGetKeyValue = m_dParams.GetElement(FIELD_GET_KEY_VALUE)).IsNil())
		{
		//	Initialize our context

		m_iProcess = processGetKeyValue;
		m_iNext = 0;
		m_ValueList.InsertEmpty(m_dList.GetCount());

		//	Keep processing until we get all the values

		if (!ProcessValueList(dSelf, retResult))
			return false;

		return true;
		}

	//	Otherwise we do a sort on list values.

	else
		{
		m_dList.Sort(m_iSort);
		*retResult = m_dList;
		return true;
		}

	return true;
	}

bool CSortFunctionProcessor::ProcessContinues (CDatum dSelf, CDatum dResult, CDatum *retResult)

//	ProcessContinues
//
//	Process continues. This is called after we made a function call. The result
//	of the function call is in dResult. We return under the same conditions as 
//	Process.

	{
	//	Add the result to the list

	m_ValueList[m_iNext].dKey = dResult;
	m_ValueList[m_iNext].iIndex = m_iNext;
	m_iNext++;

	//	Keep processing until we get all the values

	if (!ProcessValueList(dSelf, retResult))
		return false;

	return true;
	}

bool CSortFunctionProcessor::ProcessValueList (CDatum dSelf, CDatum *retResult)

//	ProcessValueList
//
//	Processes the next item in the value list.

	{
	int i;

	//	First fill the list

	while (m_iNext < m_ValueList.GetCount())
		{
		//	Compute the key value

		if (!m_dGetKeyValue.IsNil())
			{
			CComplexArray *pArgs = new CComplexArray;
			pArgs->Append(m_dList.GetElement(m_iNext));

			CComplexArray *pResult = new CComplexArray;
			pResult->Append(m_dGetKeyValue);
			pResult->Append(CDatum(pArgs));
			pResult->Append(dSelf);

			*retResult = CDatum(pResult);
			return false;
			}

		m_iNext++;
		}

	//	Sort the value list.

	m_ValueList.Sort(NULL, CompareEntry, m_iSort);

	//	Alter the original list with the new order

	for (i = 0; i < m_ValueList.GetCount(); i++)
		m_ValueList[i].dValue = m_dList.GetElement(m_ValueList[i].iIndex);

	for (i = 0; i < m_dList.GetCount(); i++)
		m_dList.SetElement(i, m_ValueList[i].dValue);

	*retResult = m_dList;

	return true;
	}
