//	CAEONVectorFloat64.cpp
//
//	CAEONVectorFloat64 classes
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_VECTOR_FLOAT_64,			"vector_Float64");

TDatumPropertyHandler<CAEONVectorFloat64> CAEONVectorFloat64::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::FLOAT_64);
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
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
		[](const CAEONVectorFloat64 &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONVectorFloat64::m_pMethodsExt = NULL;

int CAEONVectorFloat64::FindMaxElement () const
	{
	int iIndex = -1;
	double rBest;

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (!std::isfinite(m_Array[i]))
			return -1;
		else if (iIndex == -1 || m_Array[i] > rBest)
			{
			iIndex = i;
			rBest = m_Array[i];
			}
		}

	return iIndex;
	}

int CAEONVectorFloat64::FindMinElement () const
	{
	int iIndex = -1;
	double rBest;

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (!std::isfinite(m_Array[i]))
			return -1;
		else if (iIndex == -1 || m_Array[i] < rBest)
			{
			iIndex = i;
			rBest = m_Array[i];
			}
		}

	return iIndex;
	}

const CString &CAEONVectorFloat64::GetTypename (void) const
	{
	return TYPENAME_VECTOR_FLOAT_64;
	}

void CAEONVectorFloat64::InsertEmpty (int iCount)
	{
	int iStart = m_Array.GetCount();
	m_Array.InsertEmpty(iCount);
	for (int i = iStart; i < iStart + iCount; i++)
		m_Array[i] = 0.0;
	}

bool CAEONVectorFloat64::IsFloat (CDatum dValue, double& retrValue)
	{
	switch (dValue.GetBasicDatatype())
		{
		case IDatatype::FLOAT:
		case IDatatype::FLOAT_64:
			retrValue = dValue.raw_GetDouble();
			return true;

		case IDatatype::INT_8:
		case IDatatype::INT_16:
		case IDatatype::INT_32:
		case IDatatype::INT_64:
		case IDatatype::UINT_8:
		case IDatatype::UINT_16:
		case IDatatype::UINT_32:
		case IDatatype::UINT_64:
			retrValue = (double)dValue;
			return true;

		default:
			return false;
		}
	}

CDatum CAEONVectorFloat64::MathAbs () const
	{
	CAEONVectorFloat64 *pResult = new CAEONVectorFloat64;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = abs(m_Array[i]);

	return dResult;
	}

void CAEONVectorFloat64::MathAccumulateStats (CDatum::SStatsCtx& Stats) const
	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		double rValue = m_Array[i];
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

CDatum CAEONVectorFloat64::MathAverage () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	double rResult = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		rResult += m_Array[i];

	return CDatum(rResult / (double)m_Array.GetCount());
	}

CDatum CAEONVectorFloat64::MathCeil () const
	{
	CAEONVectorFloat64 *pResult = new CAEONVectorFloat64;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = ceil(m_Array[i]);

	return dResult;
	}

CDatum CAEONVectorFloat64::MathFloor () const
	{
	CAEONVectorFloat64 *pResult = new CAEONVectorFloat64;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = floor(m_Array[i]);

	return dResult;
	}

CDatum CAEONVectorFloat64::MathMax () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	double rMax = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		if (m_Array[i] > rMax)
			rMax = m_Array[i];

	return CDatum(rMax);
	}

CDatum CAEONVectorFloat64::MathMin () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	double rMin = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		if (m_Array[i] < rMin)
			rMin = m_Array[i];

	return CDatum(rMin);
	}

CDatum CAEONVectorFloat64::MathRound () const
	{
	CAEONVectorFloat64 *pResult = new CAEONVectorFloat64;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = round(m_Array[i]);

	return dResult;
	}

CDatum CAEONVectorFloat64::MathSign () const
	{
	CAEONVectorFloat64 *pResult = new CAEONVectorFloat64;
	CDatum dResult(pResult);

	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = Sign(m_Array[i]);

	return dResult;
	}

CDatum CAEONVectorFloat64::MathSum () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	double rResult = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		rResult += m_Array[i];

	return CDatum(rResult);
	}

CDatum CAEONVectorFloat64::MathAddToElements (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathAddToElements(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i] + rValue;

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathAddElementsTo (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathAddElementsTo(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = rValue + m_Array[i];

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathDivideElementsBy (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathDivideElementsBy(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());

	if (rValue == 0.0)
		{
		for (int i = 0; i < m_Array.GetCount(); i++)
			pResult->m_Array[i] = CDatum::CreateNaN();
		}
	else
		{
		for (int i = 0; i < m_Array.GetCount(); i++)
			pResult->m_Array[i] = m_Array[i] / rValue;
		}

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathDivideByElements (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathDivideByElements(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = (m_Array[i] == 0.0 ? CDatum::CreateNaN() : CDatum(rValue / m_Array[i]));

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathExpElementsTo (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathExpElementsTo(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = pow(m_Array[i], rValue);

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathExpToElements (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathExpToElements(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = pow(rValue, m_Array[i]);

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathModElementsBy (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathModElementsBy(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());

	if (rValue == 0.0)
		{
		for (int i = 0; i < m_Array.GetCount(); i++)
			pResult->m_Array[i] = CDatum::CreateNaN();
		}
	else
		{
		for (int i = 0; i < m_Array.GetCount(); i++)
			{
			double rResult = std::fmod(m_Array[i], rValue);
			if (rResult < 0.0 && rValue > 0.0)
				pResult->m_Array[i] = CDatum(rResult + rValue);
			else if (rResult > 0.0 && rValue < 0.0)
				pResult->m_Array[i] = CDatum(rResult + rValue);
			else
				pResult->m_Array[i] = CDatum(rResult);
			}
		}

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathModByElements (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathModByElements(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (m_Array[i] == 0.0)
			pResult->m_Array[i] = CDatum::CreateNaN();
		else
			{
			double rResult = std::fmod(rValue, m_Array[i]);
			if (rResult < 0.0 && m_Array[i] > 0.0)
				pResult->m_Array[i] = CDatum(rResult + m_Array[i]);
			else if (rResult > 0.0 && m_Array[i] < 0.0)
				pResult->m_Array[i] = CDatum(rResult + m_Array[i]);
			else
				pResult->m_Array[i] = CDatum(rResult);
			}
		}

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathMultiplyElements (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathMultiplyElements(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i] * rValue;

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathNegateElements () const
	{
	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = -m_Array[i];

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathSubtractFromElements (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathSubtractFromElements(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = m_Array[i] - rValue;

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::MathSubtractElementsFrom (CDatum dValue) const
	{
	double rValue;
	if (!IsFloat(dValue, rValue))
		return IComplexDatum::MathSubtractElementsFrom(dValue);

	CAEONVectorFloat64* pResult = new CAEONVectorFloat64;
	pResult->m_Array.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		pResult->m_Array[i] = rValue - m_Array[i];

	return CDatum(pResult);
	}

CDatum CAEONVectorFloat64::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorFloat64 *pArray = new CAEONVectorFloat64;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		pArray->m_Array.Insert(Stream.ReadDouble());
		}

	return dValue;
	}

void CAEONVectorFloat64::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_VECTOR_FLOAT64))
		return;

	//	Otherwise, write out the full array

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		Stream.Write(m_Array[i]);
		}
	}

