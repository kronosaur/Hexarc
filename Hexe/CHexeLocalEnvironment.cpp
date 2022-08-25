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
	ASSERT(GetArgumentCount() <= GetAllocSize());

	for (int i = 0; i < GetArgumentCount(); i++)
		if (m_pArray[i].dValue.Contains(dValue, retChecked))
			return true;

	return false;
	}

bool CHexeLocalEnvironment::FindArgument (const CString &sArg, int *retiLevel, int *retiIndex)

//	FindArgument
//
//	Finds the given argument in the closes environment

	{
	ASSERT(GetArgumentCount() <= GetAllocSize());

	//	See if it is in our environment

	for (int i = 0; i < GetArgumentCount(); i++)
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
		ASSERT(iIndex < GetAllocSize());

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
	ASSERT(GetArgumentCount() <= GetAllocSize());

	for (int i = 0; i < GetArgumentCount(); i++)
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

		if (iCurCount == 0 && GetArgumentCount() > 0)
			{
			for (int i = 0; i < DEFAULT_SIZE; i++)
				m_pArray[i] = m_BaseArray[i];
			}
		}
	}

void CHexeLocalEnvironment::IncArgumentValue(int iIndex, int iInc)

//	IncArgumentValue
//
//	Increment the given variable,

	{
	ASSERT(iIndex < GetAllocSize());

	m_pArray[iIndex].dValue = CHexeProcess::ExecuteIncrement(m_pArray[iIndex].dValue, iInc);
	}

CDatum CHexeLocalEnvironment::MathMax () const

//	MathMax
//
//	Returns the max value.

	{
	int iCount = GetArgumentCount();
	if (iCount == 0)
		return CDatum();

	CNumberValue Result(m_pArray[0].dValue.MathMax());
	for (int i = 1; i < iCount; i++)
		{
		Result.Max(m_pArray[i].dValue.MathMax());
		}

	if (Result.IsValidNumber())
		return Result.GetDatum();
	else
		return CDatum::CreateNaN();
	}

CDatum CHexeLocalEnvironment::MathMin () const

//	MathMin
//
//	Returns the min value.

	{
	int iCount = GetArgumentCount();
	if (iCount == 0)
		return CDatum();

	CNumberValue Result(m_pArray[0].dValue.MathMin());
	for (int i = 1; i < iCount; i++)
		{
		Result.Min(m_pArray[i].dValue.MathMin());
		}

	if (Result.IsValidNumber())
		return Result.GetDatum();
	else
		return CDatum::CreateNaN();
	}

bool CHexeLocalEnvironment::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize.

	{
	CDatum dValues = dStruct.GetElement(FIELD_VALUES);
	int iCount = dValues.GetCount() / 2;

	GrowArray(iCount);
	m_iNextArg = iCount;

	int iArg = 0;
	for (int i = 0; i < dValues.GetCount(); i += 2)
		{
		m_pArray[iArg].sArg = dValues.GetElement(i);
		m_pArray[iArg].dValue = dValues.GetElement(i + 1);

		iArg++;
		}

	m_dParentEnv = dStruct.GetElement(FIELD_PARENT);

	return true;
	}

void CHexeLocalEnvironment::OnMarked (void)

//	OnMarked
//
//	Marks data in use

	{
	for (int i = 0; i < Min(GetArgumentCount(), GetAllocSize()); i++)
		m_pArray[i].dValue.Mark();

	m_dParentEnv.Mark();
	}

void CHexeLocalEnvironment::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize all elements for obects that are serialized as structures.

	{
	ASSERT(GetArgumentCount() <= GetAllocSize());

	CDatum dValues(CDatum::typeArray);
	dValues.GrowToFit(2 * GetArgumentCount());
	for (int i = 0; i < GetArgumentCount(); i++)
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
		if (iIndex >= GetArgumentCount())
			{
			GrowArray(iIndex + 1);
			m_iNextArg = iIndex + 1;
			}

		ASSERT(iIndex < GetAllocSize());

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
		if (iIndex >= GetArgumentCount())
			{
			GrowArray(iIndex + 1);
			m_iNextArg = iIndex + 1;
			}

		ASSERT(iIndex < GetAllocSize());

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
	ASSERT(GetArgumentCount() <= GetAllocSize());

	for (int i = 0; i < GetArgumentCount(); i++)
		if (strEquals(sKey, m_pArray[i].sArg))
			{
			m_pArray[i].dValue = dValue;
			return;
			}

	//	If we get here then we need to add the value

	GrowArray(GetArgumentCount() + 1);

	SEntry *pEntry = &m_pArray[m_iNextArg];
	pEntry->sArg = sKey;
	pEntry->dValue = dValue;
	m_iNextArg++;
	}

