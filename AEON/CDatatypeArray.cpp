//	CDatatypeArray.cpp
//
//	CDatatypeArray class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_ARRAY_DIMENSION_MISMATCH,		"Cannot dereference an array with multiple indices.");
DECLARE_CONST_STRING(ERR_DICTIONARY_DIMENSION_MISMATCH,	"Cannot dereference a dictionary with multiple keys.");
DECLARE_CONST_STRING(ERR_INVALID_KEY_COUNT,				"Table has %d keys.");

bool CDatatypeArray::OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType, CString* retsError) const
	{
	if (m_bTable)
		{
		const IDatatype& Schema = GetElementType();
		int iKeyCount = 0;
		for (int i = 0; i < Schema.GetMemberCount(); i++)
			{
			SMemberDesc Member = Schema.GetMember(i);
			if (Member.iType == EMemberType::InstanceKeyVar)
				iKeyCount++;
			}

		if (iKeyCount != ArgTypes.GetCount())
			{
			if (retsError) *retsError = strPattern(ERR_INVALID_KEY_COUNT, iKeyCount);
			return false;
			}
		}
	else if (m_bDictionary)
		{
		if (ArgTypes.GetCount() != 1)
			{
			if (retsError) *retsError = ERR_DICTIONARY_DIMENSION_MISMATCH;
			return false;
			}
		}
	else
		{
		if (ArgTypes.GetCount() != 1)
			{
			if (retsError) *retsError = ERR_ARRAY_DIMENSION_MISMATCH;
			return false;
			}
		}

	//	Get the element type.

	if (retdReturnType)
		{
		*retdReturnType == GetElementType();
		if (retdReturnType->IsNil())
			*retdReturnType = CAEONTypes::Get(IDatatype::ANY);
		}

	return true;
	}

bool CDatatypeArray::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	SetCoreType(Stream.ReadDWORD());

	if (!CComplexDatatype::CreateFromStream(Stream, m_dElementType))
		return false;

	m_bTable = ((const IDatatype&)m_dElementType).GetClass() == IDatatype::ECategory::Schema;
	m_bDictionary = false;

	return true;
	}

bool CDatatypeArray::OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized)
	{
	DWORD dwFlags = 0;
	if (dwVersion >= 2)
		dwFlags = Stream.ReadDWORD();

	if (dwVersion >= 3)
		m_dKeyType = CDatum::DeserializeAEON(Stream, Serialized);

	m_dElementType = CDatum::DeserializeAEON(Stream, Serialized);

	if (dwVersion >= 2)
		{
		m_bTable = (dwFlags & 0x00000001) ? true : false;
		m_bDictionary = (dwFlags & 0x00000002) ? true : false;
		}
	else
		{
		m_bTable = ((const IDatatype&)m_dElementType).GetClass() == IDatatype::ECategory::Schema;
		m_bDictionary = false;
		}

	return true;
	}

bool CDatatypeArray::OnEquals (const IDatatype &Src) const
	{
	CRecursionSmartLock Lock(m_rs);
	if (Lock.InRecursion())
		return true;

	auto &Other = (const CDatatypeArray &)Src;

	if (GetCoreType() && GetCoreType() == Other.GetCoreType())
		return true;

	if (m_bTable != Other.m_bTable)
		return false;

	if (m_bDictionary != Other.m_bDictionary)
		return false;

	if ((const IDatatype &)GetKeyType() != (const IDatatype &)Other.GetKeyType())
		return false;

	if ((const IDatatype &)m_dElementType != (const IDatatype &)Other.m_dElementType)
		return false;

	return true;
	}

int CDatatypeArray::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Returns the index of the given member (by ID) or returns -1.

	{
	if (m_bTable)
		{
		int iIndex = CAEONTable::FindPropertyByKey(sName);
		if (iIndex != -1)
			return iIndex + 1;

		iIndex = CAEONTable::FindMethodByKey(sName);
		if (iIndex != -1)
			return iIndex + 1 + CAEONTable::GetPropertyCount();
		}
	else if (m_bDictionary)
		{
		int iIndex = CAEONDictionary::FindPropertyByKey(sName);
		if (iIndex != -1)
			return iIndex + 2;

		iIndex = CAEONDictionary::FindMethodByKey(sName);
		if (iIndex != -1)
			return iIndex + 2 + CAEONDictionary::GetPropertyCount();
		}
	else
		{
		int iIndex = CComplexArray::FindPropertyByKey(sName);
		if (iIndex != -1)
			return iIndex + 1;

		iIndex = CComplexArray::FindMethodByKey(sName);
		if (iIndex != -1)
			return iIndex + 1 + CComplexArray::GetPropertyCount();
		}

	return -1;
	}

TArray<CDatum> CDatatypeArray::OnGetDimensionTypes () const
	{
	TArray<CDatum> Dims;
	Dims.Insert(GetKeyType());
	return Dims;
	}

CDatum CDatatypeArray::OnGetFieldsAsTable () const

//	OnGetFieldsAsTable
//
//	Returns a table describing all the members of the schema. The resulting 
//	table has the following fields:
//
//	name (string): The name of the field/column.
//	datatype (datatype): The datatype of the field.
//	label (string): The human-readable name.
//	description (string): A description.

	{
	if (m_bTable)
		{
		CRecursionSmartLock Lock(m_rs);
		if (Lock.InRecursion())
			return CDatum();

		return ((const IDatatype&)m_dElementType).GetFieldsAsTable();
		}
	else
		return CDatum();
	}

IDatatype::SMemberDesc CDatatypeArray::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Get the given member.

	{
	if (iIndex == 0)
		return SMemberDesc({ EMemberType::ArrayElement, NULL_STR, m_dElementType });
	else
		{
		iIndex--;

		if (m_bTable)
			{
			if (iIndex < CAEONTable::GetPropertyCount())
				return SMemberDesc({ EMemberType::InstanceProperty, CAEONTable::GetPropertyKey(iIndex), CAEONTable::GetPropertyType(iIndex) });
			else
				{
				iIndex -= CAEONTable::GetPropertyCount();

				if (iIndex < CAEONTable::GetMethodCount())
					return SMemberDesc({ EMemberType::InstanceMethod, CAEONTable::GetMethodKey(iIndex), CAEONTable::GetMethodType(iIndex) });
				else
					throw CException(errFail);
				}
			}
		else if (m_bDictionary)
			{
			if (iIndex == 0)
				return SMemberDesc({ EMemberType::IndexElement, NULL_STR, m_dKeyType });
				
			iIndex--;

			if (iIndex < CAEONDictionary::GetPropertyCount())
				return SMemberDesc({ EMemberType::InstanceProperty, CAEONDictionary::GetPropertyKey(iIndex), CAEONDictionary::GetPropertyType(iIndex) });
			else
				{
				iIndex -= CAEONDictionary::GetPropertyCount();

				if (iIndex < CAEONDictionary::GetMethodCount())
					return SMemberDesc({ EMemberType::InstanceMethod, CAEONDictionary::GetMethodKey(iIndex), CAEONDictionary::GetMethodType(iIndex) });
				else
					throw CException(errFail);
				}
			}
		else
			{
			if (iIndex < CComplexArray::GetPropertyCount())
				return SMemberDesc({ EMemberType::InstanceProperty, CComplexArray::GetPropertyKey(iIndex), CComplexArray::GetPropertyType(iIndex) });
			else
				{
				iIndex -= CComplexArray::GetPropertyCount();

				if (iIndex < CComplexArray::GetMethodCount())
					return SMemberDesc({ EMemberType::InstanceMethod, CComplexArray::GetMethodKey(iIndex), CComplexArray::GetMethodType(iIndex) });
				else
					throw CException(errFail);
				}
			}
		}
	}

int CDatatypeArray::OnGetMemberCount () const

//	OnGetMemberCount
//
//	Returns the total number of members.

	{
	//	The first entry is the element type. Next we return the set of built-in
	//	properties. And last is the set of built-in methods.

	if (m_bTable)
		return 1 + CAEONTable::GetPropertyCount() + CAEONTable::GetMethodCount();
	else if (m_bDictionary)
		return 2 + CAEONDictionary::GetPropertyCount() + CAEONDictionary::GetMethodCount();
	else
		return 1 + CComplexArray::GetPropertyCount() + CComplexArray::GetMethodCount();
	}

CString CDatatypeArray::OnGetName () const
	{
	if (IsAnonymous())
		{
		if (m_bTable)
			return strPattern("table of %s", (LPCSTR)((const IDatatype&)m_dElementType).GetName());
		else if (m_bDictionary)
			{
			const IDatatype& KeyType = GetKeyType();
			const IDatatype& ElementType = GetElementType();

			if (KeyType.IsAny() && ElementType.IsAny())
				return strPattern("Dictionary");
			else if (KeyType.IsAny())
				return strPattern("dictionary of %s", (LPCSTR)ElementType.GetName());
			else
				return strPattern("dictionary[%s] of %s", (LPCSTR)KeyType.GetName(), (LPCSTR)ElementType.GetName());
			}
		else
			{
			const IDatatype& ElementType = GetElementType();
			if (ElementType.IsAny())
				return strPattern("Array");
			else
				return strPattern("array of %s", (LPCSTR)ElementType.GetName());
			}
		}
	else
		return DefaultGetName();
	}

IDatatype::EMemberType CDatatypeArray::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	OnHasMember
//
//	Looks for the given member and returns the type.

	{
	int iIndex = FindMember(sName);
	if (iIndex == -1)
		return EMemberType::None;

	SMemberDesc Member = GetMember(iIndex);
	if (retdType)
		*retdType = Member.dType;

	if (retiOrdinal)
		*retiOrdinal = Member.iOrdinal;

	return Member.iType;
	}

bool CDatatypeArray::OnIsA (const IDatatype &Type) const
	{
	//	We implement the following abstract types

	switch (Type.GetCoreType())
		{
		case IDatatype::ARRAY:
			return !m_bTable && !m_bDictionary;

		case IDatatype::INDEXED:
		case IDatatype::MUTABLE_INDEXED:
			return true;

		case IDatatype::TABLE:
			return m_bTable;

		case IDatatype::ABSTRACT_DICTIONARY:
		case IDatatype::ABSTRACT_MUTABLE_DICTIONARY:
			return m_bTable || m_bDictionary;

		case IDatatype::DICTIONARY:
			return m_bDictionary;
		}

	//	Otherwise, see if we're an array of the same type or subtype.

	switch (Type.GetClass())
		{
		case IDatatype::ECategory::Array:
		case IDatatype::ECategory::Dictionary:
		case IDatatype::ECategory::Table:
			{
			CRecursionSmartLock Lock(m_rs);
			if (Lock.InRecursion())
				return true;

			if (GetClass() != Type.GetClass())
				return false;

			const IDatatype& OurElementType = m_dElementType;
			CDatum dElementType = Type.GetElementType();
			if (!OurElementType.IsA(dElementType))
				return false;

			CDatum dKeyType = Type.GetKeyType();
			if (!((const IDatatype&)GetKeyType()).IsA(dKeyType))
				return false;

			//	Array of Int32 and IntIP are not the same type as array of 
			//	Number (even though Int32 and IntIP are subtypes of Number)
			//	because we can't mutate them in the same way (they are thus
			//	not covariant).
			//
			//	Same for array of float

			switch (OurElementType.GetCoreType())
				{
				case IDatatype::INT_32:
				case IDatatype::INT_IP:
				case IDatatype::FLOAT:
				case IDatatype::FLOAT_64:
				case IDatatype::INTEGER:
				case IDatatype::REAL:
					return false;

				default:
					break;
				}

			return true;
			}
		}

	return false;
	}

CDatum CDatatypeArray::OnGetKeyType () const

//	OnGetKeyType
//
//	Returns the key type.

	{
	//	For tables, it depends on whether the schema has keys or not.

	if (m_bTable)
		{
		const IDatatype& ElementType = GetElementType();

		TArray<int> Keys;
		if (!ElementType.GetKeyMembers(Keys))
			return CAEONTypes::Get(IDatatype::INTEGER);
		else
			{
			//	If we have a single key, then that is the key type.

			if (Keys.GetCount() == 1)
				{
				SMemberDesc MemberInfo = ElementType.GetMember(Keys[0]);
				return MemberInfo.dType;
				}

			//	Otherwise, it is Any

			else
				return CAEONTypes::Get(IDatatype::ANY);
			}
		}

	//	Otherwise, if we have an explicit key type, then use that.

	else if (!m_dKeyType.IsNil())
		return m_dKeyType;

	//	Otherwise, it is an integer

	else
		return CAEONTypes::Get(IDatatype::INTEGER);
	}

void CDatatypeArray::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	DWORD dwFlags = 0;
	dwFlags |= (m_bTable ?		0x00000001 : 0);
	dwFlags |= (m_bDictionary ? 0x00000002 : 0);

	Stream.Write(dwFlags);

	m_dKeyType.SerializeAEON(Stream, Serialized);
	m_dElementType.SerializeAEON(Stream, Serialized);
	}
