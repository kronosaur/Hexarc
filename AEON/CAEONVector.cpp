//	CAEONVector.cpp
//
//	CAEONVector classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_VECTOR_FLOAT_64,			"vector_Float64");
DECLARE_CONST_STRING(TYPENAME_VECTOR_INT_32,			"vector_Int32");
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

