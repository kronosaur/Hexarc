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

int CComplexDatatype::GetID (IDatatype::EImplementation iValue)
	{
	switch (iValue)
		{
		case IDatatype::EImplementation::Any:
			return IMPL_ANY_ID;

		case IDatatype::EImplementation::Array:
			return IMPL_ARRAY_ID;

		case IDatatype::EImplementation::Class:
			return IMPL_CLASS_ID;

		case IDatatype::EImplementation::Enum:
			return IMPL_ENUM_ID;

		case IDatatype::EImplementation::Matrix:
			return IMPL_MAXTRIX_ID;

		case IDatatype::EImplementation::Number:
			return IMPL_NUMBER_ID;

		case IDatatype::EImplementation::Schema:
			return IMPL_SCHEMA_ID;

		case IDatatype::EImplementation::Simple:
			return IMPL_SIMPLE_ID;

		default:
			throw CException(errFail);
		}
	}

IDatatype::EImplementation CComplexDatatype::GetImplementation (int iID)
	{
	switch (iID)
		{
		case IMPL_ANY_ID:
			return IDatatype::EImplementation::Any;

		case IMPL_ARRAY_ID:
			return IDatatype::EImplementation::Array;

		case IMPL_CLASS_ID:
			return IDatatype::EImplementation::Class;

		case IMPL_ENUM_ID:
			return IDatatype::EImplementation::Enum;

		case IMPL_MAXTRIX_ID:
			return IDatatype::EImplementation::Matrix;

		case IMPL_NUMBER_ID:
			return IDatatype::EImplementation::Number;

		case IMPL_SCHEMA_ID:
			return IDatatype::EImplementation::Schema;

		case IMPL_SIMPLE_ID:
			return IDatatype::EImplementation::Simple;

		default:
			return IDatatype::EImplementation::Unknown;
		}
	}

size_t CComplexDatatype::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns a heuristic estimate for serialization size (this helps us to 
//	allocate buffers during serialize.

	{
	return 0;
	}

bool CComplexDatatype::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize from a stream.

	{
	m_pType = std::move(IDatatype::Deserialize(iFormat, Stream));
	if (!m_pType)
		return false;

	return true;
	}
		
void CComplexDatatype::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize to stream.

	{
	m_pType->Serialize(iFormat, Stream);
	}
