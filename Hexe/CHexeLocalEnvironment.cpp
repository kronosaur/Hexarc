//	CHexeLocalEnvironment.cpp
//
//	CHexeLocalEnvironment class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LOCAL_ENV_PARENT,			"localEnv_parent");
DECLARE_CONST_STRING(FIELD_NEXT_ARG,					"nextArg");
DECLARE_CONST_STRING(FIELD_PARENT,						"parent");
DECLARE_CONST_STRING(FIELD_VALUES,						"values");

DECLARE_CONST_STRING(TYPENAME_HEXE_LOCAL_ENVIRONMENT,	"hexeLocalEnvironment");
const CString &CHexeLocalEnvironment::StaticGetTypename (void) { return TYPENAME_HEXE_LOCAL_ENVIRONMENT; }

CHexeLocalEnvironment::CHexeLocalEnvironment(int iCount)

//	CHexeLocalEnvironment constructor

	{
	m_iNextArg = iCount;

	if (iCount <= DEFAULT_SIZE)
		{
		m_pArray = m_BaseArray;
		}
	else
		{
		m_DynamicArray.InsertEmpty(iCount);
		m_pArray = &m_DynamicArray[0];
		}
	}

bool CHexeLocalEnvironment::Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const

//	Contains
//
//	Returns TRUE if we contain the given value.

	{
	for (int i = 0; i < GetCount(); i++)
		if (m_pArray[i].dValue.Contains(dValue, retChecked))
			return true;

	return false;
	}

bool CHexeLocalEnvironment::FindArgument (const CString &sArg, int *retiLevel, int *retiIndex)

//	FindArgument
//
//	Finds the given argument in the closes environment

	{
	//	See if it is in our environment

	for (int i = 0; i < GetCount(); i++)
		if (strEquals(sArg, m_pArray[i].sArg))
			{
			*retiLevel = 0;
			*retiIndex = i;
			return true;
			}
	
	//	If not, ask our parent

	CHexeLocalEnvironment *pParent = CHexeLocalEnvironment::Upconvert(m_dParentEnv);
	if (pParent == NULL)
		return false;

	int iLevel;
	if (!pParent->FindArgument(sArg, &iLevel, retiIndex))
		return false;

	*retiLevel = iLevel + 1;
	return true;
	}

CDatum CHexeLocalEnvironment::GetArgument (int iLevel, int iIndex)

//	GetArgument
//
//	Gets the argument

	{
	//	Is the argument at our level?

	if (iLevel == 0)
		{
		return m_pArray[iIndex].dValue;
		}

	//	If not, ask the next level up

	else
		{
		CHexeLocalEnvironment *pParent = CHexeLocalEnvironment::Upconvert(m_dParentEnv);
		if (pParent == NULL)
			throw CException(errFail);
		
		return pParent->GetArgument(iLevel - 1, iIndex);
		}
	}

CDatum CHexeLocalEnvironment::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element

	{
	for (int i = 0; i < GetCount(); i++)
		if (strEquals(sKey, m_pArray[i].sArg))
			return m_pArray[i].dValue;

	if (strEquals(sKey, FIELD_LOCAL_ENV_PARENT))
		return m_dParentEnv;

	return CDatum();
	}

void CHexeLocalEnvironment::GrowArray (int iNewCount)

//	GrowArray
//
//	Grows the array capacity to the given value.

	{
	if (iNewCount > DEFAULT_SIZE)
		{
		int iCurCount = m_DynamicArray.GetCount();
		if (iNewCount <= iCurCount)
			return;

		m_DynamicArray.InsertEmpty(iNewCount - iCurCount);
		m_pArray = &m_DynamicArray[0];

		if (iCurCount == 0 && GetCount() > 0)
			{
			for (int i = 0; i < GetCount(); i++)
				m_pArray[i] = m_BaseArray[i];
			}
		}
	}

void CHexeLocalEnvironment::IncArgumentValue(int iIndex, int iInc)

//	IncArgumentValue
//
//	Increment the given variable,

	{
	m_pArray[iIndex].dValue = CHexeProcess::ExecuteIncrement(m_pArray[iIndex].dValue, iInc);
	}

bool CHexeLocalEnvironment::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize.

	{
	CDatum dValues = dStruct.GetElement(FIELD_VALUES);
	int iCount = dValues.GetCount() / 2;

	m_DynamicArray.DeleteAll();
	m_iNextArg = iCount;
	if (iCount <= DEFAULT_SIZE)
		{
		m_pArray = m_BaseArray;
		}
	else
		{
		m_DynamicArray.InsertEmpty(iCount);
		m_pArray = &m_DynamicArray[0];
		}

	for (int i = 0; i < dValues.GetCount(); i += 2)
		{
		m_pArray[i].sArg = dValues.GetElement(i);
		m_pArray[i].dValue = dValues.GetElement(i + 1);
		}

	m_dParentEnv = dStruct.GetElement(FIELD_PARENT);

	return true;
	}

void CHexeLocalEnvironment::OnMarked (void)

//	OnMarked
//
//	Marks data in use

	{
	for (int i = 0; i < GetCount(); i++)
		m_pArray[i].dValue.Mark();

	m_dParentEnv.Mark();
	}

void CHexeLocalEnvironment::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize all elements for obects that are serialized as structures.

	{
	CDatum dValues(CDatum::typeArray);
	dValues.GrowToFit(2 * GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		dValues.Append(m_pArray[i].sArg);
		dValues.Append(m_pArray[i].dValue);
		}

	pStruct->SetElement(FIELD_VALUES, dValues);

	pStruct->SetElement(FIELD_NEXT_ARG, m_iNextArg);

	//	NOTE: We do not serialize the parent because it is a pointer.
	}

void CHexeLocalEnvironment::SetArgumentKey (int iLevel, int iIndex, const CString &sKey)

//	SetArgumentKey
//
//	Sets the argument key

	{
	if (iLevel == 0)
		{
		if (iIndex >= GetCount())
			{
			GrowArray(iIndex);
			m_iNextArg = iIndex + 1;
			}

		m_pArray[iIndex].sArg = sKey;
		}
	else
		{
		CHexeLocalEnvironment *pParent = CHexeLocalEnvironment::Upconvert(m_dParentEnv);
		if (pParent == NULL)
			return;
		
		return pParent->SetArgumentKey(iLevel - 1, iIndex, sKey);
		}
	}

void CHexeLocalEnvironment::SetArgumentValue (int iLevel, int iIndex, CDatum dValue)

//	SetArgumentValue
//
//	Sets the argument

	{
	if (iLevel == 0)
		{
		if (iIndex >= GetCount())
			{
			GrowArray(iIndex);
			m_iNextArg = iIndex + 1;
			}

		m_pArray[iIndex].dValue = dValue;
		}
	else
		{
		CHexeLocalEnvironment *pParent = CHexeLocalEnvironment::Upconvert(m_dParentEnv);
		if (pParent == NULL)
			return;
		
		return pParent->SetArgumentValue(iLevel - 1, iIndex, dValue);
		}
	}

void CHexeLocalEnvironment::SetElement (const CString &sKey, CDatum dValue)

//	SetElement
//
//	Sets the element

	{
	for (int i = 0; i < GetCount(); i++)
		if (strEquals(sKey, m_pArray[i].sArg))
			{
			m_pArray[i].dValue = dValue;
			return;
			}

	//	If we get here then we need to add the value

	GrowArray(GetCount() + 1);

	SEntry *pEntry = &m_pArray[m_iNextArg];
	pEntry->sArg = sKey;
	pEntry->dValue = dValue;
	m_iNextArg++;
	}

