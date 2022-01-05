//	CHexeLocalEnvironment.cpp
//
//	CHexeLocalEnvironment class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_NEXT_ARG,					"nextArg");
DECLARE_CONST_STRING(FIELD_PARENT,						"parent");
DECLARE_CONST_STRING(FIELD_VALUES,						"values");

DECLARE_CONST_STRING(TYPENAME_HEXE_LOCAL_ENVIRONMENT,	"hexeLocalEnvironment");
const CString &CHexeLocalEnvironment::StaticGetTypename (void) { return TYPENAME_HEXE_LOCAL_ENVIRONMENT; }

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
		if (iIndex < 0 || iIndex >= m_Array.GetCount())
			return CDatum();

		return m_Array[iIndex].dValue;
		}

	//	If not, ask the next level up

	else
		{
		CHexeLocalEnvironment *pParent = CHexeLocalEnvironment::Upconvert(m_dParentEnv);
		if (pParent == NULL)
			return CDatum();
		
		return pParent->GetArgument(iLevel - 1, iIndex);
		}
	}

CDatum CHexeLocalEnvironment::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element

	{
	int i;

	for (i = 0; i < m_Array.GetCount(); i++)
		if (strEquals(sKey, m_Array[i].sArg))
			return m_Array[i].dValue;

	return CDatum();
	}

bool CHexeLocalEnvironment::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize.

	{
	CDatum dValues = dStruct.GetElement(FIELD_VALUES);

	m_Array.DeleteAll();
	m_Array.GrowToFit(dValues.GetCount());

	for (int i = 0; i < dValues.GetCount(); i++)
		{
		auto pEntry = m_Array.Insert();
		pEntry->sArg = dValues.GetKey(i);
		pEntry->dValue = dValues.GetElement(i);
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
	CDatum dValues(CDatum::typeStruct);
	dValues.GrowToFit(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		dValues.SetElement(m_Array[i].sArg, m_Array[i].dValue);

	pStruct->SetElement(FIELD_VALUES, dValues);

	pStruct->SetElement(FIELD_NEXT_ARG, m_iNextArg);
//	pStruct->SetElement(FIELD_PARENT, m_dParentEnv);
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

