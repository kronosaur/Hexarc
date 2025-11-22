//	CAEONVectorNumber.cpp
//
//	CAEONVectorNumber class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_VECTOR_NUMBER,			"vector_Number");

TDatumPropertyHandler<CAEONVectorNumber> CAEONVectorNumber::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::NUMBER);
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
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
		[](const CAEONVectorNumber &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONVectorNumber::m_pMethodsExt = NULL;

CDatum CAEONVectorNumber::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorNumber *pArray = new CAEONVectorNumber;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		pArray->m_Array.Insert(CDatum::DeserializeAEON(Stream, Serialized));
		}

	return dValue;
	}

CDatum CAEONVectorNumber::FromDatum (CDatum dValue)
	{
	switch (dValue.GetBasicType())
		{
		case CDatum::typeNil:
			return CDatum();

		case CDatum::typeInteger32:
		case CDatum::typeInteger64:
		case CDatum::typeIntegerIP:
		case CDatum::typeDouble:
			return dValue;

		default:
			return (double)dValue;
		}
	}

const CString &CAEONVectorNumber::GetTypename (void) const
	{
	return TYPENAME_VECTOR_NUMBER;
	}

void CAEONVectorNumber::InsertEmpty (int iCount)
	{
	int iStart = m_Array.GetCount();
	m_Array.InsertEmpty(iCount);
	}

CDatum CAEONVectorNumber::MathAbs () const
	{
	CAEONVectorNumber *pResult = new CAEONVectorNumber;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathAbs();

	return dResult;
	}

void CAEONVectorNumber::MathAccumulateStats (CDatum::SStatsCtx& Stats) const
	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].MathAccumulateStats(Stats);
	}

CDatum CAEONVectorNumber::MathCeil () const
	{
	CAEONVectorNumber *pResult = new CAEONVectorNumber;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathCeil();

	return dResult;
	}

CDatum CAEONVectorNumber::MathFloor () const
	{
	CAEONVectorNumber *pResult = new CAEONVectorNumber;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathFloor();

	return dResult;
	}

CDatum CAEONVectorNumber::MathRound () const
	{
	CAEONVectorNumber *pResult = new CAEONVectorNumber;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathRound();

	return dResult;
	}

CDatum CAEONVectorNumber::MathSign () const
	{
	CAEONVectorNumber *pResult = new CAEONVectorNumber;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i].MathSign();

	return dResult;
	}

void CAEONVectorNumber::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_VECTOR_NUMBER))
		return;

	//	Otherwise, write out the full array

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		m_Array[i].SerializeAEON(Stream, Serialized);
		}
	}

