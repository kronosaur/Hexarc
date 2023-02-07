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

	//	If we're the same, then we're guaranteed to be equal. But sometimes we
	//	can have two different named types that are equal (e.g., table schemas).

	if (strEquals(m_sFullyQualifiedName, Src.m_sFullyQualifiedName))
		return true;

	return OnEquals(Src);
	}

CDatum IDatatype::ApplyKeyToRow (CDatum dKey, CDatum dRow) const

//	ApplyKeyToRow
//
//	Adds the key values to the given row and returns it.

	{
	TArray<int> Keys;
	if (!GetKeyMembers(Keys))
		return dRow;

	CDatum dResult(CDatum::typeStruct);
	dResult.Append(dRow);

	if (Keys.GetCount() == 0)
		return NULL_STR;

	//	If the key is a struct, then we expect it to be a row structure 
	//	(possibly with nothing except key members).

	else if (dKey.GetBasicType() == CDatum::typeStruct)
		{
		for (int i = 0; i < Keys.GetCount(); i++)
			{
			if (Keys[i] >= GetMemberCount())
				continue;

			const CString& sColName = GetMember(Keys[i]).sName;
			dResult.SetElement(sColName, dKey.GetElement(sColName));
			}

		return dResult;
		}

	//	Otherwise, we expect dKey to be an array of key values (or an atomic
	//	value).

	else
		{
		for (int i = 0; i < Keys.GetCount(); i++)
			{
			if (Keys[i] >= GetMemberCount())
				continue;

			const CString& sColName = GetMember(Keys[i]).sName;
			dResult.SetElement(sColName, dKey.GetElement(i));
			}

		return dResult;
		}
	}

CString IDatatype::AsIndexKeyFromValue (CDatum dValue)

//	AsIndexKeyFromValue
//
//	Returns as a key.

	{
	switch (dValue.GetBasicType())
		{
		case CDatum::typeString:
			return (const CString&)dValue;

		case CDatum::typeDateTime:
			return ((const CDateTime&)dValue).Format(CDateTime::formatISO8601);

		default:
			return dValue.AsString();
		}
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

		case EImplementation::Enum:
			pDatatype.Set(new CDatatypeEnum(sFullyQualifiedName));
			break;

		case EImplementation::Matrix:
			pDatatype.Set(new CDatatypeMatrix(sFullyQualifiedName));
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

CString IDatatype::GetKeyFromKeyValue (CDatum dKey) const

//	GetKeyFromKeyValue
//
//	Returns a string encoding the given key value (which is either a single 
//	value or an array of key values).
//
//	If there are no keys, we return NULL_STR.

	{
	TArray<int> Keys;
	if (!GetKeyMembers(Keys))
		return NULL_STR;

	return GetKeyFromKeyValue(Keys, dKey);
	}

CString IDatatype::GetKeyFromKeyValue (const TArray<int>& Keys, CDatum dKey) const

//	GetKeyFromKeyValue
//
//	Returns a string encoding the given key value (which is either a single 
//	value or an array of key values).
//
//	If there are no keys, we return NULL_STR.

	{
	if (Keys.GetCount() == 0)
		return NULL_STR;

	//	If the key is a struct, then we expect it to be a row structure 
	//	(possibly with nothing except key members).

	else if (dKey.GetBasicType() == CDatum::typeStruct)
		{
		CStringBuffer Result;
		for (int i = 0; i < Keys.GetCount(); i++)
			{
			if (Keys[i] >= GetMemberCount())
				return NULL_STR;

			if (i != 0)
				Result.WriteChar('/');

			Result.Write(AsIndexKeyFromValue(dKey.GetElement(GetMember(Keys[i]).sName)));
			}

		return CString(std::move(Result));
		}

	//	If we only have a single key, then we expect dKey to be an atomic value.

	else if (Keys.GetCount() == 1)
		{
		return AsIndexKeyFromValue(dKey);
		}

	//	Otherwise, we expect dKey to be an array of key values.

	else
		{
		CStringBuffer Result;
		Result.Write(AsIndexKeyFromValue(dKey.GetElement(0)));
		for (int i = 1; i < Keys.GetCount(); i++)
			{
			Result.WriteChar('/');
			Result.Write(AsIndexKeyFromValue(dKey.GetElement(i)));
			}

		return CString(std::move(Result));
		}
	}

bool IDatatype::GetKeyMembers (TArray<int>& retKeys) const

//	GetKeyMembers
//
//	If we have key columns, we return TRUE and the indices of the key columns.

	{
	retKeys.DeleteAll();

	for (int i = 0; i < GetMemberCount(); i++)
		{
		if (GetMember(i).iType == EMemberType::InstanceKeyVar)
			retKeys.Insert(i);
		}

	return (retKeys.GetCount() > 0);
	}

CString IDatatype::GetName () const
	{
	return CAEONTypes::ParseNameFromFullyQualifiedName(GetFullyQualifiedName());
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
