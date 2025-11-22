//	CHexeLocalEnvironment.cpp
//
//	CHexeLocalEnvironment class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LOCAL_ENV_PARENT,			"localEnv_parent");
DECLARE_CONST_STRING(FIELD_NEXT_ARG,					"nextArg");
DECLARE_CONST_STRING(FIELD_PARENT,						"parent");
DECLARE_CONST_STRING(FIELD_VALUES,						"values");

DECLARE_CONST_STRING(TYPENAME_HEXE_LOCAL_ENVIRONMENT,	"hexeLocalEnvironment");
const CString &CHexeLocalEnvironment::StaticGetTypename (void) { return TYPENAME_HEXE_LOCAL_ENVIRONMENT; }

CHexeLocalEnvironment::CHexeLocalEnvironment (int iCount)

//	CHexeLocalEnvironment constructor

	{
	m_iArgCount = iCount;
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

void CHexeLocalEnvironment::AppendArgumentValue (CDatum dValue)

//	AppendArgumentValue
//
//	Appends an argument value from left to right.

	{
	//	Handle the spread operator.

	if (dValue.GetAnnotation().fSpread)
		{
		CDatum dArray = dValue.GetElement(0);
		GrowArray(m_iArgCount + dArray.GetCount());
		for (int i = 0; i < dArray.GetCount(); i++)
			m_pArray[m_iArgCount + i].dValue = dArray.GetElement(i);

		m_iArgCount += dArray.GetCount();
		}

	//	Normal argument.

	else
		{
		GrowArray(m_iArgCount + 1);
		m_pArray[m_iArgCount++].dValue = dValue;
		}

	m_iNextArg = m_iArgCount;
	}

bool CHexeLocalEnvironment::Contains (CDatum dValue) const

//	Contains
//
//	Returns TRUE if we contain the given value.

	{
	ASSERT(GetArgumentCount() <= GetAllocSize());

	for (int i = 0; i < GetArgumentCount(); i++)
		if (m_pArray[i].dValue.Contains(dValue))
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

	CHexeLocalEnvironment *pParent = GetParentEnv();
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
#ifdef DEBUG_PROC_ENV
		if (iIndex >= GetArgumentCount())
			throw CException(errFail);
#endif

		return (iIndex < GetArgumentCount() ? m_pArray[iIndex].dValue : CDatum());
		}

	//	If not, ask the next level up

	else
		{
		CHexeLocalEnvironment *pParent = GetParentEnv();
		if (pParent == NULL)
			throw CException(errFail);
		
		return pParent->GetArgument(iLevel - 1, iIndex);
		}
	}

CDatum CHexeLocalEnvironment::GetArgumentsAsCacheKey () const

//	GetArgumentsAsCacheKey
//
//	Returns all arguments as a cache key

	{
	if (GetArgumentCount() == 0)
		return CDatum();
	else if (GetArgumentCount() == 1)
		return GetArgument(0);
	else
		{
		CDatum dResult(CDatum::typeArray);
		for (int i = 0; i < GetArgumentCount(); i++)
			dResult.Append(GetArgument(i));

		return dResult;
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
		return m_ParentEnv.GetClosure();

	return CDatum();
	}

CHexeLocalEnvironment& CHexeLocalEnvironment::GetEnvAtLevel (int iLevel)

//	GetEnvAtLevel
//
//	Return the local environment at the given level.

	{
	CHexeLocalEnvironment *pEnv = const_cast<CHexeLocalEnvironment*>(this);
	while (iLevel--)
		{
		pEnv = pEnv->GetParentEnv();
		if (!pEnv)
			throw CException(errFail);
		}

	return *pEnv;
	}

CHexeLocalEnvironment* CHexeLocalEnvironment::GetParentEnv ()

//	GetParentEnv
//
//	Returns the parent environment. NOTE: This is a pointer to the parent
//	environment, so it should not be kept by the caller.

	{
	return m_ParentEnv.GetEnv();
	}

CDatum CHexeLocalEnvironment::GetParentEnvClosure()

//	GetParentEnvClosure
//
//	Returns the parent environment as a datum (which can be kept by the caller).

	{
	return m_ParentEnv.GetClosure();
	}

CHexeLocalEnvironment* CHexeLocalEnvironment::GetParentEnvHandoff ()

//	GetParentEnv
//
//	Returns the parent environment and clears it from us.

	{
	return m_ParentEnv.GetHandoff();
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

void CHexeLocalEnvironment::IncArgumentValue (int iIndex, int iInc)

//	IncArgumentValue
//
//	Increment the given variable,

	{
	ASSERT(iIndex < GetAllocSize());

	m_pArray[iIndex].dValue.MutateAdd(iInc);
	}

CDatum CHexeLocalEnvironment::IncArgumentValueInt32 (int iIndex, int iInc)

//	IncArgumentValueInt32
//
//	Increment the given variable,

	{
	ASSERT(iIndex < GetAllocSize());
	
	m_pArray[iIndex].dValue.MutateAddInt32(iInc);
	return m_pArray[iIndex].dValue;
	}

void CHexeLocalEnvironment::Init (int iCount)

//	Init
//
//	Initialize the environment

	{
	if (iCount > GetAllocSize())
		GrowArray(iCount);

	m_ParentEnv = CHexeLocalEnvPointer();
	m_iArgCount = iCount;
	m_iNextArg = iCount;

	for (int i = 0; i < iCount; i++)
		{
		m_pArray[i].sArg = NULL_STR;
		m_pArray[i].dValue = CDatum();
		}
	}

CString CHexeLocalEnvironment::MakeCacheKey () const

//	MakeCacheKey
//
//	Returns a key to use for the argument cache

	{
	CStringBuffer Buffer;

	for (int i = 0; i < GetArgumentCount(); i++)
		{
		DWORDLONG dwValue = GetArgument(i).raw_AsEncoded();
		Buffer.Write((char*)&dwValue, sizeof(dwValue));
		}

	return CString(std::move(Buffer));
	}

CDatum CHexeLocalEnvironment::MathMax () const

//	MathMax
//
//	Returns the max value.

	{
	if (GetArgumentCount() == 0)
		return CDatum();

	bool bFound = false;
	CDatum dMax;
	for (int i = 0; i < GetArgumentCount(); i++)
		{
		CDatum dValue = m_pArray[i].dValue.MathMax();
		if (dValue.IsNil())
			continue;

		if (dValue.IsIdenticalToNaN())
			return dValue;

		if (!bFound || dValue.OpCompare(dMax) > 0)
			{
			dMax = dValue;
			bFound = true;
			}
		}

	return dMax;
	}

CDatum CHexeLocalEnvironment::MathMin () const

//	MathMin
//
//	Returns the min value.

	{
	if (GetArgumentCount() == 0)
		return CDatum();

	bool bFound = false;
	CDatum dMin;
	for (int i = 0; i < GetArgumentCount(); i++)
		{
		CDatum dValue = m_pArray[i].dValue.MathMin();
		if (dValue.IsNil())
			continue;

		if (dValue.IsIdenticalToNaN())
			return dValue;

		if (!bFound || dValue.OpCompare(dMin) < 0)
			{
			dMin = dValue;
			bFound = true;
			}
		}

	return dMin;
	}

bool CHexeLocalEnvironment::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize.

	{
	CDatum dValues = dStruct.GetElement(FIELD_VALUES);
	int iCount = dValues.GetCount() / 2;

	GrowArray(iCount);
	m_iArgCount = iCount;
	m_iNextArg = iCount;

	int iArg = 0;
	for (int i = 0; i < dValues.GetCount(); i += 2)
		{
		m_pArray[iArg].sArg = dValues.GetElement(i).AsStringView();
		m_pArray[iArg].dValue = dValues.GetElement(i + 1);

		iArg++;
		}

	m_ParentEnv = CHexeLocalEnvPointer(dStruct.GetElement(FIELD_PARENT));

	return true;
	}

void CHexeLocalEnvironment::OnMarked (void)

//	OnMarked
//
//	Marks data in use

	{
#ifdef DEBUG_PROC_ENV
	if (GetAllocSize() < GetArgumentCount())
		throw CException(errFail);
#endif

	for (int i = 0; i < GetArgumentCount(); i++)
		m_pArray[i].dValue.Mark();

	m_ParentEnv.Mark();
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

void CHexeLocalEnvironment::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_iArgCount = (int)Stream.ReadDWORD();
	m_iNextArg = m_iArgCount;

	int iCount = (int)Stream.ReadDWORD();
	GrowArray(iCount);

	for (int i = 0; i < iCount; i++)
		{
		m_pArray[i].sArg = CString::Deserialize(Stream);
		m_pArray[i].dValue = CDatum::DeserializeAEON(Stream, Serialized);
		}
	}

void CHexeLocalEnvironment::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	Stream.Write(m_iArgCount);

	Stream.Write(GetArgumentCount());
	for (int i = 0; i < GetArgumentCount(); i++)
		{
		m_pArray[i].sArg.Serialize(Stream);
		m_pArray[i].dValue.SerializeAEON(Stream, Serialized);
		}
	}

void CHexeLocalEnvironment::SetArgumentKey (int iLevel, int iIndex, CStringView sKey)

//	SetArgumentKey
//
//	Sets the argument key

	{
	if (iLevel == 0)
		{
		if (iIndex >= GetArgumentCount())
			{
			GrowArray(iIndex + 1);

			//	NOTE: If we have to grow the array then it means that we passed
			//	in fewer arguments than the function expects. We need to set the
			//	value to null in case an old value is left over.

			m_pArray[iIndex].dValue = CDatum();

			m_iArgCount = iIndex + 1;
			m_iNextArg = iIndex + 1;
			}

		ASSERT(iIndex < GetAllocSize());

		m_pArray[iIndex].sArg = sKey;
		}
	else
		{
		CHexeLocalEnvironment *pParent = GetParentEnv();
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
			m_iArgCount = iIndex + 1;
			m_iNextArg = m_iArgCount;
			}

		ASSERT(iIndex < GetAllocSize());

		m_pArray[iIndex].dValue = dValue;
		}
	else
		{
		CHexeLocalEnvironment *pParent = GetParentEnv();
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

	SEntry *pEntry = &m_pArray[m_iArgCount];
	pEntry->sArg = sKey;
	pEntry->dValue = dValue;
	m_iArgCount++;
	m_iNextArg = m_iArgCount;
	}

void CHexeLocalEnvironment::SetParentEnv (CDatum dParentEnv)

//	SetParentEnv
//
//	Sets the parent environment.
	
	{
	m_ParentEnv = CHexeLocalEnvPointer(dParentEnv);
	}

void CHexeLocalEnvironment::SetParentEnv (CHexeLocalEnvPointer&& ParentEnv)

//	SetParentEnv
//
//	Sets the parent environment.
	
	{
	m_ParentEnv = std::move(ParentEnv);
	}
