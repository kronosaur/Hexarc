//	IDatatype.cpp
//
//	IDatatype class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool IDatatype::operator == (const IDatatype &Src) const

//	operator ==

	{
	if (GetImplementation() != Src.GetImplementation())
		return false;

	if (!strEquals(m_sFullyQualifiedName, Src.m_sFullyQualifiedName))
		return false;

	return OnEquals(Src);
	}

TUniquePtr<IDatatype> IDatatype::Deserialize (CDatum::EFormat iFormat, IByteStream &Stream)

//	Deserialize
//
//	Deserialize the datatype.

	{
	//	Load the type

	DWORD dwType;
	Stream.Read(&dwType, sizeof(DWORD));
	EImplementation iType = CComplexDatatype::GetImplementation(dwType);
	if (iType == EImplementation::Unknown)
		return NULL;

	//	Read the name

	CString sFullyQualifiedName = CString::Deserialize(Stream);

	TUniquePtr<IDatatype> pDatatype;
	switch (iType)
		{
		case EImplementation::Any:
			pDatatype.Set(new CDatatypeAny(sFullyQualifiedName));
			break;

		case EImplementation::Array:
			pDatatype.Set(new CDatatypeArray(sFullyQualifiedName));
			break;

		case EImplementation::Class:
			pDatatype.Set(new CDatatypeClass(sFullyQualifiedName));
			break;

		case EImplementation::Number:
			pDatatype.Set(new CDatatypeNumber(sFullyQualifiedName));
			break;

		case EImplementation::Schema:
			pDatatype.Set(new CDatatypeSchema(sFullyQualifiedName));
			break;

		case EImplementation::Simple:
			pDatatype.Set(new CDatatypeSimple(sFullyQualifiedName));
			break;

		default:
			return NULL;
		}

	if (!pDatatype->OnDeserialize(iFormat, Stream))
		return NULL;

	return pDatatype;
	}

CString IDatatype::GetName () const
	{
	return CAEONTypeSystem::ParseNameFromFullyQualifiedName(GetFullyQualifiedName());
	}

bool IDatatype::IsACoreType (DWORD dwType) const
	{
	CDatum dType = CAEONTypeSystem::GetCoreType(dwType);
	const IDatatype &Type = dType;
	return IsA(Type);
	}

void IDatatype::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serializes the datatype.

	{
	//	Save the type.

	DWORD dwType = CComplexDatatype::GetID(GetImplementation());
	Stream.Write(&dwType, sizeof(DWORD));

	//	Save the name.

	m_sFullyQualifiedName.Serialize(Stream);

	//	Serialize the rest.

	OnSerialize(iFormat, Stream);
	}
