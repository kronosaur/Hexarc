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

//	CAEONVectorInt32 -----------------------------------------------------------

const CString &CAEONVectorInt32::GetTypename (void) const
	{
	return TYPENAME_VECTOR_INT_32;
	}

//	CAEONVectorString ----------------------------------------------------------

const CString &CAEONVectorString::GetTypename (void) const
	{
	return TYPENAME_VECTOR_STRING;
	}
