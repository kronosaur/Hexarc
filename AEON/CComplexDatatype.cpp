//	CComplexDatatype.cpp
//
//	CComplexDatatype class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_DATATYPE,				"datatype")

TDatumPropertyHandler<CComplexDatatype> CComplexDatatype::m_Properties = {
	{
		"fields",
		"A table representing the fields of the type.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return Obj.m_pType->GetMembersAsTable();
			},
		NULL,
		},
	{
		"name",
		"Name of the datatype.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_pType->GetName());
			},
		NULL,
		},
	{
		"symbol",
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
