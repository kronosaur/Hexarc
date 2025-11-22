//	CAEONVector.cpp
//
//	CAEONVector classes
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_VECTOR_INT_32,			"vector_Int32");
DECLARE_CONST_STRING(TYPENAME_VECTOR_INT_IP,			"vector_IntIP");
DECLARE_CONST_STRING(TYPENAME_VECTOR_TYPED,				"vector_Typed");

//	CAEONVectorInt32 -----------------------------------------------------------

TDatumPropertyHandler<CAEONVectorInt32> CAEONVectorInt32::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::INT_32);
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.Append(Obj.GetCount());
			return dResult;
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorInt32 &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONVectorInt32::m_pMethodsExt = NULL;

int CAEONVectorInt32::FindMaxElement () const
	{
	int iIndex = -1;
	int iBest;

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (iIndex == -1 || m_Array[i] > iBest)
			{
			iIndex = i;
			iBest = m_Array[i];
			}
		}

	return iIndex;
	}

int CAEONVectorInt32::FindMinElement () const
	{
	int iIndex = -1;
	int iBest;

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (iIndex == -1 || m_Array[i] < iBest)
			{
			iIndex = i;
			iBest = m_Array[i];
			}
		}

	return iIndex;
	}

const CString &CAEONVectorInt32::GetTypename (void) const
	{
	return TYPENAME_VECTOR_INT_32;
	}

void CAEONVectorInt32::InsertEmpty (int iCount)
	{
	int iStart = m_Array.GetCount();
	m_Array.InsertEmpty(iCount);
	for (int i = iStart; i < iStart + iCount; i++)
		m_Array[i] = 0;
	}

CDatum CAEONVectorInt32::MathAbs () const
	{
	CAEONVectorInt32 *pResult = new CAEONVectorInt32;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = abs(m_Array[i]);

	return dResult;
	}

void CAEONVectorInt32::MathAccumulateStats (CDatum::SStatsCtx& Stats) const
	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		double rValue = (double)m_Array[i];
		Stats.iCount++;

		double rDelta = rValue - Stats.rMean;
		Stats.rMean += rDelta / Stats.iCount;
		double rDelta2 = rValue - Stats.rMean;
		Stats.M2 += rDelta * rDelta2;

		if (rValue > Stats.rMax)
			Stats.rMax = rValue;

		if (rValue < Stats.rMin)
			Stats.rMin = rValue;
		}
	}

CDatum CAEONVectorInt32::MathAverage () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	bool bIPInt = false;
	CIPInteger Result;

	LONGLONG iResult = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (bIPInt)
			{
			Result += CIPInteger(m_Array[i]);
			}
		else
			{
			iResult += (LONGLONG)m_Array[i];
			if (iResult < INT_MIN || iResult > INT_MAX)
				{
				Result = CIPInteger(iResult);
				bIPInt = true;
				}
			}
		}

	double rResult;
	if (bIPInt)
		{
		rResult = (Result.AsDouble() / (double)m_Array.GetCount());
		}
	else
		{
		rResult = (double)iResult / (double)m_Array.GetCount();
		}

	//	NOTE: OK to cast to int because average of ints cannot be outside of
	//	int range.

	double intpart;
	if (std::modf(rResult, &intpart) == 0.0)
		return CDatum((int)intpart);
	else
		return CDatum(rResult);
	}

CDatum CAEONVectorInt32::MathMax () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	int iMax = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		if (m_Array[i] > iMax)
			iMax = m_Array[i];

	return CDatum(iMax);
	}

CDatum CAEONVectorInt32::MathMin () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	int iMin = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		if (m_Array[i] < iMin)
			iMin = m_Array[i];

	return CDatum(iMin);
	}

CDatum CAEONVectorInt32::MathSign () const
	{
	CAEONVectorInt32 *pResult = new CAEONVectorInt32;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = Sign(m_Array[i]);

	return dResult;
	}

CDatum CAEONVectorInt32::MathSum () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	bool bIPInt = false;
	CIPInteger Result;

	LONGLONG iResult = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (bIPInt)
			{
			Result += CIPInteger(m_Array[i]);
			}
		else
			{
			iResult += (LONGLONG)m_Array[i];
			if (iResult < INT_MIN || iResult > INT_MAX)
				{
				Result = CIPInteger(iResult);
				bIPInt = true;
				}
			}
		}

	if (bIPInt)
		return CDatum(Result);
	else
		return CDatum((int)iResult);
	}

CDatum CAEONVectorInt32::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorInt32 *pArray = new CAEONVectorInt32;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		pArray->m_Array.Insert((int)Stream.ReadDWORD());
		}

	return dValue;
	}

void CAEONVectorInt32::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_VECTOR_INT32))
		return;

	//	Otherwise, write out the full array

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		Stream.Write(m_Array[i]);
		}
	}

//	CAEONVectorIntIP -----------------------------------------------------------

TDatumPropertyHandler<CAEONVectorIntIP> CAEONVectorIntIP::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::INT_IP);
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.Append(Obj.GetCount());
			return dResult;
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorIntIP &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONVectorIntIP::m_pMethodsExt = NULL;

int CAEONVectorIntIP::FindMaxElement() const
	{
	int iIndex = -1;
	CIPInteger iBest;

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (iIndex == -1 || m_Array[i] > iBest)
			{
			iIndex = i;
			iBest = m_Array[i];
			}
		}

	return iIndex;
	}

int CAEONVectorIntIP::FindMinElement() const
	{
	int iIndex = -1;
	CIPInteger iBest;

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (iIndex == -1 || m_Array[i] < iBest)
			{
			iIndex = i;
			iBest = m_Array[i];
			}
		}

	return iIndex;
	}

const CString &CAEONVectorIntIP::GetTypename (void) const
	{
	return TYPENAME_VECTOR_INT_IP;
	}

CIPInteger CAEONVectorIntIP::FromDatum (CDatum dValue)
	{
	const CIPInteger& Value = dValue;

	//	We always want a valid value, since we can't even express a null value

	if (Value.IsEmpty())
		return CIPInteger(0);
	else
		return CIPInteger(Value);
	}

void CAEONVectorIntIP::InsertEmpty (int iCount)
	{
	int iStart = m_Array.GetCount();
	m_Array.InsertEmpty(iCount);
	for (int i = iStart; i < iStart + iCount; i++)
		m_Array[i] = CIPInteger(0);
	}

CDatum CAEONVectorIntIP::MathAbs () const
	{
	CAEONVectorIntIP *pResult = new CAEONVectorIntIP;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		if (m_Array[i] < 0)
			pResult->m_Array[i] = -m_Array[i];
		else
			pResult->m_Array[i] = m_Array[i];

	return dResult;
	}

void CAEONVectorIntIP::MathAccumulateStats (CDatum::SStatsCtx& Stats) const
	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		double rValue = m_Array[i].AsDouble();
		Stats.iCount++;

		double rDelta = rValue - Stats.rMean;
		Stats.rMean += rDelta / Stats.iCount;
		double rDelta2 = rValue - Stats.rMean;
		Stats.M2 += rDelta * rDelta2;

		if (rValue > Stats.rMax)
			Stats.rMax = rValue;

		if (rValue < Stats.rMin)
			Stats.rMin = rValue;
		}
	}

CDatum CAEONVectorIntIP::MathAverage () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CIPInteger Result = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		Result += m_Array[i];

	return CNumberValue::Divide(Result, CIPInteger(m_Array.GetCount()));
	}

CDatum CAEONVectorIntIP::MathMax () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CIPInteger Max = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		if (m_Array[i] > Max)
			Max = m_Array[i];

	return CDatum(Max);
	}

CDatum CAEONVectorIntIP::MathMin () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CIPInteger Min = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		if (m_Array[i] < Min)
			Min = m_Array[i];

	return CDatum(Min);
	}

CDatum CAEONVectorIntIP::MathSign () const
	{
	CAEONVectorIntIP *pResult = new CAEONVectorIntIP;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		if (m_Array[i].IsZero())
			pResult->m_Array[i] = CIPInteger(0);
		else if (m_Array[i].IsNegative())
			pResult->m_Array[i] = CIPInteger(-1);
		else
			pResult->m_Array[i] = CIPInteger(1);

	return dResult;
	}

CDatum CAEONVectorIntIP::MathSum () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CIPInteger Result = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		Result += m_Array[i];

	return CDatum(Result);
	}

CDatum CAEONVectorIntIP::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorIntIP *pArray = new CAEONVectorIntIP;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		CIPInteger Value;
		CIPInteger::Deserialize(Stream, &Value);
			
		pArray->m_Array.Insert(std::move(Value));
		}

	return dValue;
	}

void CAEONVectorIntIP::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_VECTOR_INTIP))
		return;

	//	Otherwise, write out the full array

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		m_Array[i].Serialize(Stream);
		}
	}

//	CAEONVectorTyped ---------------------------------------------------------

TDatumPropertyHandler<CAEONVectorTyped> CAEONVectorTyped::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			return Obj.m_dElementType;
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			return CAEONTypes::Get(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.Append(Obj.GetCount());
			return dResult;
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorTyped &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONVectorTyped::m_pMethodsExt = NULL;

const CString &CAEONVectorTyped::GetTypename (void) const
	{
	return TYPENAME_VECTOR_TYPED;
	}

size_t CAEONVectorTyped::CalcMemorySize (void) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return 0;

	size_t dwSize = 0;

	for (int i = 0; i < m_Array.GetCount(); i++)
		dwSize += sizeof(m_Array[i]);

	return dwSize;
	}

IComplexDatum *CAEONVectorTyped::Clone (CDatum::EClone iMode) const
	{
	switch (iMode)
		{
		case CDatum::EClone::CopyOnWrite:
			//	Default handler
			return NULL;

		default:
			return new CAEONVectorTyped(m_dDatatype, m_Array);
		}
	}

CDatum CAEONVectorTyped::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorTyped *pArray = new CAEONVectorTyped;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the datatype

	pArray->m_dDatatype = CDatum::DeserializeAEON(Stream, Serialized);
	pArray->m_dElementType = CalcElementType(pArray->m_dDatatype);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		pArray->m_Array.Insert(CDatum::DeserializeAEON(Stream, Serialized));
		}

	return dValue;
	}

bool CAEONVectorTyped::Find (CDatum dValue, int *retiIndex) const
	{
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsEqual(GetElement(i)))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

CDatum CAEONVectorTyped::FindAll (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsEqual(GetElement(i)))
			dResult.Append(i);
		}

	return dResult;
	}

CDatum CAEONVectorTyped::FindAllExact (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsIdentical(GetElement(i)))
			dResult.Append(i);
		}

	return dResult;
	}

bool CAEONVectorTyped::FindExact (CDatum dValue, int *retiIndex) const
	{
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsIdentical(GetElement(i)))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

CString CAEONVectorTyped::Format (const CStringFormat& Format) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return AsAddress();

	return CComplexArray::Format(m_Array, Format);
	}

CDatum CAEONVectorTyped::FromDatum (CDatum dValue) const
	{
	const IDatatype &Type = dValue.GetDatatype();
	if (Type.IsA(m_dElementType))
		return dValue;

	return CDatum::CreateAsType(m_dElementType, dValue);
	}

CDatum CAEONVectorTyped::CalcElementType (CDatum dType)
	{
	return ((const IDatatype&)dType).GetElementType();
	}

CDatum CAEONVectorTyped::GetElementAt (int iIndex) const
	{
	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		return m_Array[iIndex];
	else if (iIndex < 0)
		{
		int iNewIndex = iIndex + m_Array.GetCount();
		if (iNewIndex >= 0 && iNewIndex < m_Array.GetCount())
			return m_Array[iNewIndex];
		else
			return MakeNullElement();
		}
	else
		return MakeNullElement();
	}

#if 0
CDatum CAEONVectorTyped::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const
	{
	int iIndex = dIndex.AsArrayIndex(m_Array.GetCount());
	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		return m_Array[iIndex];
	else if (dIndex.IsContainer())
		{
		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(dIndex.GetCount());

		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			int iIndex = dIndex.GetElement(i).AsArrayIndex(m_Array.GetCount());
			if (iIndex >= 0 && iIndex < m_Array.GetCount())
				dResult.Append(m_Array[iIndex]);
			else
				dResult.Append(MakeNullElement());
			}

		return dResult;
		}
	else
		return MakeNullElement();
	}
#endif

void CAEONVectorTyped::InsertElementAt (CDatum dIndex, CDatum dDatum)
	{
	bool bFromEnd;
	int iIndex = dIndex.AsArrayIndex(m_Array.GetCount(), &bFromEnd);

	//	If from the end, we insert AFTER the specified position (because
	//	we want -1 to mean AFTER the last entry).

	if (bFromEnd && iIndex >= 0)
		iIndex++;

	//	Handle different cases

	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		{
		m_Array.Insert(dDatum, iIndex);
		}
	else if (iIndex >= 0)
		{
		CDatum dNullValue = MakeNullElement();
		int iNew = iIndex - GetCount();
		GrowToFit(iNew + 1);
		for (int i = 0; i < iNew; i++)
			Append(dNullValue);

		Append(dDatum);
		}
	else if (dIndex.IsContainer())
		{
		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			InsertElementAt(dIndex.GetElement(i), dDatum);
			}
		}
	}

CDatum CAEONVectorTyped::MakeNullElement () const

//	MakeNullElement
//
//	Returns an appropriate value for out of range dereferences.

	{
	const IDatatype& ElementType = m_dElementType;
	if (ElementType.CanBeNull())
		return CDatum();
	else
		return CDatum::CreateAsType(m_dElementType, CDatum());
	}

CDatum CAEONVectorTyped::MathAbs () const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum();

	CAEONVectorTyped *pResult = new CAEONVectorTyped(m_dDatatype);
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathAbs();

	return dResult;
	}

void CAEONVectorTyped::MathAccumulateStats (CDatum::SStatsCtx& Stats) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].MathAccumulateStats(Stats);
	}

CDatum CAEONVectorTyped::MathCeil () const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum();

	CAEONVectorTyped *pResult = new CAEONVectorTyped(m_dDatatype);
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathCeil();

	return dResult;
	}

CDatum CAEONVectorTyped::MathFloor () const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum();

	CAEONVectorTyped *pResult = new CAEONVectorTyped(m_dDatatype);
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathFloor();

	return dResult;
	}

CDatum CAEONVectorTyped::MathRound () const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum();

	CAEONVectorTyped *pResult = new CAEONVectorTyped(m_dDatatype);
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathRound();

	return dResult;
	}

CDatum CAEONVectorTyped::MathSign () const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum();

	CAEONVectorTyped *pResult = new CAEONVectorTyped(m_dDatatype);
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathSign();

	return dResult;
	}

size_t CAEONVectorTyped::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	size_t TotalSize = 2 + m_Array.GetCount();

	for (int i = 0; i < m_Array.GetCount(); i++)
		TotalSize += m_Array[i].CalcSerializeSize(iFormat);

	return TotalSize;
	}

void CAEONVectorTyped::OnMarked (void)
	{
	m_dDatatype.Mark();
	m_dElementType.Mark();

	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].Mark();
	}

bool CAEONVectorTyped::RemoveAll ()
	{
	m_Array.DeleteAll();
	return true;
	}

bool CAEONVectorTyped::RemoveElementAt (CDatum dIndex)

//	RemoveElementAt
//
//	Removes the element by index.

	{
	int iIndex = dIndex.AsArrayIndex(m_Array.GetCount());

	//	Handle different cases

	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		{
		m_Array.Delete(iIndex);
		return true;
		}
	else if (dIndex.IsContainer())
		{
		int iOldSize = m_Array.GetCount();

		TSortMap<int, bool> Indices;
		for (int i = 0; i < dIndex.GetCount(); i++)
			Indices.SetAt(dIndex.GetElement(i).AsArrayIndex(m_Array.GetCount()), true);

		m_Array.Delete(Indices);
		return (iOldSize != m_Array.GetCount());
		}
	else
		return false;
	}

void CAEONVectorTyped::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResolveDatatypes
//
//	Resolve datatypes.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	m_dDatatype = TypeSystem.ResolveType(m_dDatatype);
	m_dElementType = CalcElementType(m_dDatatype);

	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].ResolveDatatypes(TypeSystem);
	}

void CAEONVectorTyped::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		Stream.Write("null", 4);

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("(", 1);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write(")", 1);
			break;
			}

		case CDatum::EFormat::GridLang:
			{
			Stream.Write("[ ", 2);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write(" ]", 2);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("[", 1);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write("]", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONVectorTyped::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_VECTOR_TYPED))
		return;

	//	Serialize the datatype

	m_dDatatype.SerializeAEON(Stream, Serialized);

	//	Write out the elements.

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		m_Array[i].SerializeAEON(Stream, Serialized);
		}
	}
