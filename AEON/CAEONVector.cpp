//	CAEONVector.cpp
//
//	CAEONVector classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_VECTOR_FLOAT_64,			"vector_Float64");
DECLARE_CONST_STRING(TYPENAME_VECTOR_INT_32,			"vector_Int32");
DECLARE_CONST_STRING(TYPENAME_VECTOR_INT_IP,			"vector_IntIP");
DECLARE_CONST_STRING(TYPENAME_VECTOR_STRING,			"vector_String");

//	CAEONVectorFloat64 ---------------------------------------------------------

const CString &CAEONVectorFloat64::GetTypename (void) const
	{
	return TYPENAME_VECTOR_FLOAT_64;
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

CDatum CAEONVectorFloat64::MathAverage () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	double rResult = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		rResult += m_Array[i];

	return CDatum(rResult / (double)m_Array.GetCount());
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

CDatum CAEONVectorFloat64::MathSum () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	double rResult = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		rResult += m_Array[i];

	return CDatum(rResult);
	}

//	CAEONVectorInt32 -----------------------------------------------------------

const CString &CAEONVectorInt32::GetTypename (void) const
	{
	return TYPENAME_VECTOR_INT_32;
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

//	CAEONVectorIntIP -----------------------------------------------------------

const CString &CAEONVectorIntIP::GetTypename (void) const
	{
	return TYPENAME_VECTOR_INT_IP;
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

CDatum CAEONVectorIntIP::MathSum () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CIPInteger Result = m_Array[0];
	for (int i = 1; i < m_Array.GetCount(); i++)
		Result += m_Array[i];

	return CDatum(Result);
	}

//	CAEONVectorString ----------------------------------------------------------

const CString &CAEONVectorString::GetTypename (void) const
	{
	return TYPENAME_VECTOR_STRING;
	}

CDatum CAEONVectorString::MathAbs () const
	{
	//	LATER: Convert to appropriate result array

	return CDatum::CreateNaN();
	}

CDatum CAEONVectorString::MathAverage () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CDatum dValue;
	if (!CDatum::CreateFromStringValue(m_Array[0], &dValue)
			|| !dValue.IsNumber())
		return CDatum::CreateNaN();

	CNumberValue Result(dValue);
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (!CDatum::CreateFromStringValue(m_Array[i], &dValue)
				|| !dValue.IsNumber())
			return CDatum::CreateNaN();

		Result.Add(dValue);
		}

	if (!Result.Divide(m_Array.GetCount()))
		return CDatum::CreateNaN();

	return Result.GetDatum();
	}
CDatum CAEONVectorString::MathMax () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CDatum dValue;
	if (!CDatum::CreateFromStringValue(m_Array[0], &dValue)
			|| !dValue.IsNumber())
		return CDatum::CreateNaN();

	CNumberValue Max(dValue);
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (!CDatum::CreateFromStringValue(m_Array[i], &dValue)
				|| !dValue.IsNumber())
			return CDatum::CreateNaN();

		Max.Max(dValue);
		}

	return Max.GetDatum();
	}

CDatum CAEONVectorString::MathMin () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CDatum dValue;
	if (!CDatum::CreateFromStringValue(m_Array[0], &dValue)
			|| !dValue.IsNumber())
		return CDatum::CreateNaN();

	CNumberValue Min(dValue);
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (!CDatum::CreateFromStringValue(m_Array[i], &dValue)
				|| !dValue.IsNumber())
			return CDatum::CreateNaN();

		Min.Min(dValue);
		}

	return Min.GetDatum();
	}

CDatum CAEONVectorString::MathSum () const
	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CDatum dValue;
	if (!CDatum::CreateFromStringValue(m_Array[0], &dValue)
			|| !dValue.IsNumber())
		return CDatum::CreateNaN();

	CNumberValue Result(dValue);
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (!CDatum::CreateFromStringValue(m_Array[i], &dValue)
				|| !dValue.IsNumber())
			return CDatum::CreateNaN();

		Result.Add(dValue);
		}

	return Result.GetDatum();
	}
