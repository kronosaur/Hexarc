//	IDatatype.cpp
//
//	IDatatype class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_FORMAT,						"format");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_KEY,							"key");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_MEMBER_TYPE,					"memberType");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ORDINAL,						"ordinal");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");
DECLARE_CONST_STRING(FIELD_UI,							"ui");

DECLARE_CONST_STRING(DISPLAY_DEFAULT,					"default");
DECLARE_CONST_STRING(DISPLAY_EDITABLE,					"editable");
DECLARE_CONST_STRING(DISPLAY_HIDDEN,					"hidden");
DECLARE_CONST_STRING(DISPLAY_READ_ONLY,					"readOnly");

IDatatype::IDatatype (const CString &sFullyQualifiedName, DWORD dwCoreType, bool bForceAnonymous) :
		m_dwCoreType(dwCoreType)

//	IDatatype constructor

	{
	if (!sFullyQualifiedName.IsEmpty())
		{
		m_sFullyQualifiedName = sFullyQualifiedName;
		m_fAnonymous = bForceAnonymous;
		}
	else
		{
		m_sFullyQualifiedName = CAEONTypes::MakeAnonymousName(NULL_STR);
		m_fAnonymous = true;
		}
	}

bool IDatatype::operator == (const IDatatype &Src) const

//	operator ==

	{
	if (GetImplementation() != Src.GetImplementation())
		return false;

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

	else if (dKey.IsStruct())
		{
		for (int i = 0; i < Keys.GetCount(); i++)
			{
			if (Keys[i] >= GetMemberCount())
				continue;

			CString sColName = GetMember(Keys[i]).sID;
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

			CString sColName = GetMember(Keys[i]).sID;
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
			return dValue.AsString();

		case CDatum::typeDateTime:
			return ((const CDateTime&)dValue).Format(CDateTime::formatISO8601);

		default:
			return dValue.AsString();
		}
	}

CString IDatatype::DefaultGetName () const
	{
	return CAEONTypes::ParseNameFromFullyQualifiedName(GetFullyQualifiedName(), true);
	}

TUniquePtr<IDatatype> IDatatype::Deserialize (CDatum::EFormat iFormat, DWORD dwType, IByteStream &Stream)

//	Deserialize
//
//	Deserialize the datatype.

	{
	DWORD dwVersion;
	EImplementation iType = CComplexDatatype::GetImplementation(dwType, dwVersion);
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

		case EImplementation::Function:
			pDatatype.Set(new CDatatypeFunction(sFullyQualifiedName));
			break;

		case EImplementation::Tensor:
			pDatatype.Set(new CDatatypeTensor(sFullyQualifiedName));
			break;

		case EImplementation::Nullable:
			pDatatype.Set(new CDatatypeNullable(sFullyQualifiedName));
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

	//	Load the rest

	if (!pDatatype->OnDeserialize(iFormat, Stream, dwVersion))
		return NULL;

	return pDatatype;
	}

TUniquePtr<IDatatype> IDatatype::DeserializeAEON (IByteStream& Stream, DWORD dwType, CAEONSerializedMap &Serialized)
	{
	//	The new version has a flag on the type.

	bool bReadFlags = false;
	if (dwType & NEW_VERSION_FLAG)
		{
		dwType &= ~NEW_VERSION_FLAG;
		bReadFlags = true;
		}

	//	NOTE: The type is passed in because callers need to interpret the
	//	type before they call us.

	DWORD dwVersion;
	EImplementation iType = CComplexDatatype::GetImplementation(dwType, dwVersion);
	if (iType == EImplementation::Unknown)
		return NULL;

	//	Read the name

	CString sFullyQualifiedName = CString::Deserialize(Stream);

	//	Create

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

		case EImplementation::Function:
			pDatatype.Set(new CDatatypeFunction(sFullyQualifiedName));
			break;

		case EImplementation::Tensor:
			pDatatype.Set(new CDatatypeTensor(sFullyQualifiedName));
			break;

		case EImplementation::Nullable:
			pDatatype.Set(new CDatatypeNullable(sFullyQualifiedName));
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

	//	Read flags

	DWORD dwFlags = 0;
	if (bReadFlags)
		dwFlags = Stream.ReadDWORD();

	pDatatype->m_fAnonymous = ((dwFlags & 0x00000001) ? true : false);

	//	Load the rest

	if (!pDatatype->OnDeserializeAEON(Stream, dwVersion, Serialized))
		return NULL;

	return pDatatype;
	}

bool IDatatype::FindMember (const TArray<SMemberDesc>& Members, CStringView sName, EMemberType iType, int* retiPos)

//	FindMember
//
//	Returns TRUE if the given member is found in the table.

	{
	for (int i = 0; i < Members.GetCount(); i++)
		{
		if (strEquals(Members[i].sID, sName))
			{
			if (iType != EMemberType::None && Members[i].iType != iType)
				continue;

			if (retiPos)
				*retiPos = i;

			return true;
			}
		}

	return false;
	}

CString IDatatype::GetID (EDisplay iDisplay)

//	GetID
//
//	Returns a string ID.

	{
	switch (iDisplay)
		{
		case EDisplay::Default:
			return DISPLAY_DEFAULT;

		case EDisplay::Editable:
			return DISPLAY_EDITABLE;

		case EDisplay::Hidden:
			return DISPLAY_HIDDEN;

		case EDisplay::ReadOnly:
			return DISPLAY_READ_ONLY;

		default:
			throw CException(errFail);
		}
	}

bool IDatatype::GetDisplayMembers (TArray<int>& retMembers) const

//	GetDisplayMembers
//
//	Returns the set of members that should be displayed (either on a form or as
//	table columns).

	{
	retMembers.DeleteAll();

	for (int i = 0; i < GetMemberCount(); i++)
		{
		SMemberDesc Member = GetMember(i);

		//	We only display instance variables

		switch (Member.iType)
			{
			case EMemberType::EnumValue:
			case EMemberType::InstanceKeyVar:
			case EMemberType::InstanceVar:
				break;

			default:
				continue;
			}

		//	Skip hidden members

		if (Member.iDisplay == EDisplay::Hidden)
			continue;

		//	Add it

		retMembers.Insert(i);
		}

	return retMembers.GetCount() > 0;
	}

CDatum IDatatype::GetElementType () const

//	GetElementType
//
//	For Arrays and tables, we return the element type. For all others, we return
//	null.

	{
	if (GetMemberCount() == 0)
		return CDatum();

	CDatum dElementType;
	if (HasMember(EMemberType::ArrayElement, &dElementType))
		return dElementType;

	return CDatum();
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

	else if (dKey.IsStruct())
		{
		CStringBuffer Result;
		for (int i = 0; i < Keys.GetCount(); i++)
			{
			if (Keys[i] >= GetMemberCount())
				return NULL_STR;

			if (i != 0)
				Result.WriteChar('/');

			Result.Write(AsIndexKeyFromValue(dKey.GetElement(GetMember(Keys[i]).sID)));
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

CDatum IDatatype::GetKeyFromRow (CDatum dRow) const

//	GetKeyFromRow
//
//	Returns the key value from the given row.

	{
	TArray<int> Keys;
	if (!GetKeyMembers(Keys))
		return CDatum();

	if (Keys.GetCount() == 1)
		return dRow.GetElement(GetMember(Keys[0]).sID);
	else
		{
		CDatum dResult(CDatum::typeArray);
		for (int i = 0; i < Keys.GetCount(); i++)
			dResult.Append(dRow.GetElement(GetMember(Keys[i]).sID));

		return dResult;
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

bool IDatatype::GetKeyMembers (TArray<CString>& retKeys) const

//	GetKeyMembers
//
//	If we have key columns, we return TRUE and the indices of the key columns.

	{
	retKeys.DeleteAll();

	for (int i = 0; i < GetMemberCount(); i++)
		{
		auto Member = GetMember(i);
		if (Member.iType == EMemberType::InstanceKeyVar)
			retKeys.Insert(Member.sID);
		}

	return (retKeys.GetCount() > 0);
	}

CDatum IDatatype::GetMembersAsTable () const

//	GetMembersAsTable
//
//	Returns a table of all members.

	{
	CDatum dMemberTypeEnum = CAEONTypes::Get(IDatatype::MEMBER_TYPE_ENUM);

	CDatum dResult = CDatum::CreateTable(CAEONTypes::Get(IDatatype::MEMBER_TABLE));
	for (int i = 0; i < GetMemberCount(); i++)
		{
		SMemberDesc Member = GetMember(i);

		CDatum dMemberType;
		switch (Member.iType)
			{
			case EMemberType::InstanceKeyVar:
			case EMemberType::InstanceVar:
				dMemberType = CDatum::CreateEnum(ORDINAL_MEMBER_TYPE_VAR, dMemberTypeEnum);
				break;

			case EMemberType::InstanceMethod:
				dMemberType = CDatum::CreateEnum(ORDINAL_MEMBER_TYPE_FUNCTION, dMemberTypeEnum);
				break;

			case EMemberType::InstanceProperty:
				dMemberType = CDatum::CreateEnum(ORDINAL_MEMBER_TYPE_PROPERTY, dMemberTypeEnum);
				break;

			case EMemberType::InstanceValue:
			case EMemberType::InstanceReadOnlyProperty:
				dMemberType = CDatum::CreateEnum(ORDINAL_MEMBER_TYPE_DEF, dMemberTypeEnum);
				break;

			default:
				//	Not a member type
				continue;
			}

		CDatum dRow(CDatum::typeStruct);
		dRow.SetElement(FIELD_ID, Member.sID);
		dRow.SetElement(FIELD_MEMBER_TYPE, dMemberType);
		dRow.SetElement(FIELD_TYPE, Member.dType);
		dRow.SetElement(FIELD_LABEL, Member.sLabel);
		dRow.SetElement(FIELD_DESCRIPTION, CDatum());

		dResult.Append(dRow);
		}

	return dResult;
	}

bool IDatatype::HasMember (EMemberType iType, CDatum* retdType, int* retiOrdinal) const

//	HasMember
//
//	Returns TRUE if we have at least one member of the given type.

	{
	if (IsAny())
		{
		if (retdType)
			*retdType = CAEONTypeSystem::GetCoreType(IDatatype::ANY);

		if (retiOrdinal)
			*retiOrdinal = -1;

		return true;
		}

	for (int i = 0; i < GetMemberCount(); i++)
		{
		SMemberDesc Member = GetMember(i);
		if (Member.iType == iType)
			{
			if (retdType)
				*retdType = Member.dType;

			if (retiOrdinal)
				*retiOrdinal = Member.iOrdinal;

			return true;
			}
		}

	return false;
	}

bool IDatatype::IsA (const IDatatype &Type) const
	
//	IsA
//
//	Returns TRUE if we are the given type or a subtype of the given type.

	{
	//	We are always a subtype of ANY

	if (Type.IsAny() || Type == *this)
		return true;

	//	If the Type is a nullable version of us, then we are a subtype.

	if (Type.GetClass() == ECategory::Nullable && IsA(Type.GetVariantType()))
		return true;

	return OnIsA(Type);
	}

bool IDatatype::IsAEx (const IDatatype& Type) const

//	IsAEx
//
//	Returns TRUE if we are the given type of a subtype, but ignores fully qualified name.

	{
	//	We are always a subtype of ANY

	if (Type.IsAny()
			|| (GetCoreType() && GetCoreType() == Type.GetCoreType())
			|| IsEqualEx(Type))
		return true;

	//	If the Type is a nullable version of us, then we are a subtype.

	if (Type.GetClass() == ECategory::Nullable && IsA(Type.GetVariantType()))
		return true;

	return OnIsA(Type);
	}

bool IDatatype::IsA (DWORD dwType) const

//	IsA
//
//	Returns TRUE if we are the given type or a subtype of the given type.

	{
	return IsA(CAEONTypeSystem::GetCoreType(dwType));
	}

bool IDatatype::IsEqualEx(const IDatatype& Src) const

//	IsEqualEx
//
//	Returns TRUE if this datatype is equal to the given datatype, ignoring the
//	fully qualified name. This is useful when we are importing a datatype from
//	another source.

	{
	if (GetImplementation() != Src.GetImplementation())
		return false;

	return OnEquals(Src);
	}

CDatum IDatatype::OnGetFieldsAsTable () const

//	OnGetFieldsAsTable
//
//	The default implementation return a table of all fields (not properties or
//	methods).

	{
	CDatum dSchema = CAEONTypeSystem::GetCoreType(IDatatype::SCHEMA_TABLE);
	CDatum dResult = CDatum::CreateTable(dSchema);

	for (int i = 0; i < GetMemberCount(); i++)
		{
		SMemberDesc Member = GetMember(i);
		switch (Member.iType)
			{
			case EMemberType::EnumValue:
			case EMemberType::InstanceKeyVar:
			case EMemberType::InstanceVar:
				{
				CDatum dRow(CDatum::typeStruct);
				dRow.SetElement(FIELD_ID, Member.sID);
				dRow.SetElement(FIELD_TYPE, Member.dType);
				dRow.SetElement(FIELD_LABEL, Member.sLabel);
				dRow.SetElement(FIELD_FORMAT, Member.sFormat);
				dRow.SetElement(FIELD_DESCRIPTION, CDatum());
				dRow.SetElement(FIELD_KEY, Member.iType == EMemberType::InstanceKeyVar);
				dRow.SetElement(FIELD_ORDINAL, Member.iOrdinal);

				switch (Member.iDisplay)
					{
					case EDisplay::Default:
						dRow.SetElement(FIELD_UI, NULL_STR);
						break;
					
					case EDisplay::Editable:
						dRow.SetElement(FIELD_UI, DISPLAY_EDITABLE);
						break;
					
					case EDisplay::Hidden:
						dRow.SetElement(FIELD_UI, DISPLAY_HIDDEN);
						break;
					
					case EDisplay::ReadOnly:
						dRow.SetElement(FIELD_UI, DISPLAY_READ_ONLY);
						break;

					default:
						throw CException(errFail);
					}

				dResult.Append(dRow);
				break;
				}
			}
		}

	return dResult;
	}

CDatum IDatatype::OnGetKeyType () const

//	OnGetKeyType
//
//	Default implementation.

	{
	return CAEONTypes::Get(IDatatype::ANY);
	}

CDatum IDatatype::OnGetRangeType () const

//	OnGetRangeType
//
//	Default implementation.

	{
	return CAEONTypes::Get(IDatatype::ANY);
	}

IDatatype::EDisplay IDatatype::ParseDisplay (CDatum dValue)

//	ParseDisplay
//
//	Parse a display option.

	{
	if (dValue.IsNil())
		return EDisplay::Default;
	else if (dValue.GetBasicType() == CDatum::typeString)
		{
		CStringView sValue = dValue;
		if (strEqualsNoCase(sValue, DISPLAY_DEFAULT))
			return EDisplay::Default;
		else if (strEqualsNoCase(sValue, DISPLAY_EDITABLE))
			return EDisplay::Editable;
		else if (strEqualsNoCase(sValue, DISPLAY_HIDDEN))
			return EDisplay::Hidden;
		else if (strEqualsNoCase(sValue, DISPLAY_READ_ONLY))
			return EDisplay::ReadOnly;
		else
			return EDisplay::Default;
		}
	else
		return EDisplay::Default;
	}

void IDatatype::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
#ifdef DEBUG
	if (strEquals(m_sFullyQualifiedName, CString("$GridACLType")))
		{
		int a = 0;
		}
#endif

	//	If this is a core type, then we only need to save the fully qualified
	//	name because it will be unique.

	if (IsCoreType() || GetImplementation() == EImplementation::Unknown)
		{
		Stream.Write(CComplexDatatype::GetImplCoreTypeID());
		m_sFullyQualifiedName.Serialize(Stream);
		}

	//	Otherwise, we need to save the entire type.

	else
		{
		//	Save the type.

		DWORD dwType = CComplexDatatype::GetID(GetImplementation());
		dwType |= NEW_VERSION_FLAG;
		Stream.Write(dwType);

		//	Save the name.

		m_sFullyQualifiedName.Serialize(Stream);

		//	Save flags

		DWORD dwFlags = 0;
		dwFlags |= (m_fAnonymous ? 0x00000001 : 0);
		Stream.Write(dwFlags);

		//	Serialize the rest.

		OnSerializeAEON(Stream, Serialized);
		}
	}
