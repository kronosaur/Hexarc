//	CDatatypeNullable.cpp
//
//	CDatatypeNullable class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeNullable::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	SetCoreType(Stream.ReadDWORD());

	if (!CComplexDatatype::CreateFromStream(Stream, m_dVariantType))
		return false;

	return true;
	}

bool CDatatypeNullable::OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized)
	{
	DWORD dwFlags = Stream.ReadDWORD();
	m_dVariantType = CDatum::DeserializeAEON(Stream, Serialized);
	return true;
	}

bool CDatatypeNullable::OnEquals (const IDatatype &Src) const
	{
	CRecursionSmartLock Lock(m_rs);
	if (Lock.InRecursion())
		return true;

	auto &Other = (const CDatatypeNullable &)Src;

	if (GetCoreType() && GetCoreType() == Other.GetCoreType())
		return true;

	if ((const IDatatype &)m_dVariantType != (const IDatatype &)Other.m_dVariantType)
		return false;

	return true;
	}

int CDatatypeNullable::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Returns the index of the given member (by ID) or returns -1.

	{
	return ((const IDatatype&)m_dVariantType).FindMember(sName);
	}

IDatatype::SMemberDesc CDatatypeNullable::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Get the given member.

	{
	return ((const IDatatype&)m_dVariantType).GetMember(iIndex);
	}

int CDatatypeNullable::OnGetMemberCount () const

//	OnGetMemberCount
//
//	Returns the total number of members.

	{
	return ((const IDatatype&)m_dVariantType).GetMemberCount();
	}

CString CDatatypeNullable::OnGetName () const

//	OnGetName
//
//	Returns a human-readable name.

	{
	return strPattern("%s?", ((const IDatatype&)m_dVariantType).GetName());
	}

IDatatype::EMemberType CDatatypeNullable::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	OnHasMember
//
//	Looks for the given member and returns the type.

	{
	return ((const IDatatype&)m_dVariantType).HasMember(sName, retdType, retiOrdinal);
	}

bool CDatatypeNullable::OnIsA (const IDatatype &Type) const
	{
	//	If the type can be null and our variant is-type, then we are a type.

	if (Type.CanBeNull() && ((const IDatatype&)m_dVariantType).IsA(Type))
		return true;

	return false;
	}

void CDatatypeNullable::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	DWORD dwFlags = 0;
	Stream.Write(dwFlags);

	m_dVariantType.SerializeAEON(Stream, Serialized);
	}
