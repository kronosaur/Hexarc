//	CComplexDatatype.cpp
//
//	CComplexDatatype class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_DATATYPE,				"datatype")

TDatumPropertyHandler<CComplexDatatype> CComplexDatatype::m_Properties = {
	{
		"name",
		"Fully-qualified name of the datatype.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_pType->GetFullyQualifiedName());
			},
		NULL,
		},
	};

const CString &CComplexDatatype::GetTypename (void) const 
	{
	return TYPENAME_DATATYPE;
	}
