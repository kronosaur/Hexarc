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
	m_Array.InsertEmpty(iCount);
	m_iNextArg = iCount;
	}

bool CHexeLocalEnvironment::Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const

//	Contains
//
//	Returns TRUE if we contain the given value.

	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		if (m_Array[i].dValue.Contains(dValue, retChecked))
			return true;

	return false;
	}

bool CHexeLocalEnvironment::FindArgument (const CString &sArg, int *retiLevel, int *retiIndex)

//	FindArgument
//
//	Finds the given argument in the closes environment

	{
	int i;

	//	See if it is in our environment

	for (i = 0; i < m_Array.GetCount(); i++)
		if (strEquals(sArg, m_Array[i].sArg))
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
		return m_Array[iIndex].dValue;
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
	for (int i = 0; i < m_Array.GetCount(); i++)
		if (strEquals(sKey, m_Array[i].sArg))
			return m_Array[i].dValue;

	if (strEquals(sKey, FIELD_LOCAL_ENV_PARENT))
		return m_dParentEnv;

	return CDatum();
	}

void CHexeLocalEnvironment::IncArgumentValue(int iIndex, int iInc)

//	IncArgumentValue
//
//	Increment the given variable,

	{
	m_Array[iIndex].dValue = CHexeProcess::ExecuteIncrement(m_Array[iIndex].dValue, iInc);
	}

bool CHexeLocalEnvironment::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize.

	{
	CDatum dValues = dStruct.GetElement(FIELD_VALUES);

	m_Array.DeleteAll();
	m_Array.GrowToFit(dValues.GetCount() / 2);

	for (int i = 0; i < dValues.GetCount(); i += 2)
		{
		auto pEntry = m_Array.Insert();
		pEntry->sArg = dValues.GetElement(i);
		pEntry->dValue = dValues.GetElement(i + 1);
		}

	m_iNextArg = dStruct.GetElement(FIELD_NEXT_ARG);
	m_dParentEnv = dStruct.GetElement(FIELD_PARENT);

	return true;
	}

void CHexeLocalEnvironment::OnMarked (void)

//	OnMarked
//
//	Marks data in use

	{
	int i;

	for (i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].dValue.Mark();

	m_dParentEnv.Mark();
	}

void CHexeLocalEnvironment::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize all elements for obects that are serialized as structures.

	{
	CDatum dValues(CDatum::typeArray);
	dValues.GrowToFit(2 * m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		dValues.Append(m_Array[i].sArg);
		dValues.Append(m_Array[i].dValue);
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
		if (iIndex >= m_Array.GetCount())
			m_Array.InsertEmpty((iIndex - m_Array.GetCount()) + 1);

		m_Array[iIndex].sArg = sKey;
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
		if (iIndex >= m_Array.GetCount())
			m_Array.InsertEmpty((iIndex - m_Array.GetCount()) + 1);

		m_Array[iIndex].dValue = dValue;
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
	int i;

	for (i = 0; i < m_Array.GetCount(); i++)
		if (strEquals(sKey, m_Array[i].sArg))
			{
			m_Array[i].dValue = dValue;
			return;
			}

	//	If we get here then we need to add the value

	SEntry *pEntry = m_Array.Insert();
	pEntry->sArg = sKey;
	pEntry->dValue = dValue;
	}

