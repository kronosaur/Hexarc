//	CDatatypeRange.cpp
//
//	CDatatypeRange class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeRange::OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized)
	{
	DWORD dwFlags = Stream.ReadDWORD();

	m_dBaseType = CDatum::DeserializeAEON(Stream, Serialized);

	return true;
	}

bool CDatatypeRange::OnEquals (const IDatatype &Src) const
	{
	if ((const IDatatype&)m_dBaseType != (const IDatatype&)Src.GetRangeType())
		return false;

	return true;
	}

int CDatatypeRange::OnFindMember (CStringView sName) const
	{
	//	LATER: Return properties.

	return -1;
	}

IDatatype::SMemberDesc CDatatypeRange::OnGetMember (int iIndex) const
	{
	if (iIndex == 0)
		return SMemberDesc({ EMemberType::RangeType, NULL_STR, m_dBaseType });
	else
		//	LATER: Return properties.
		throw CException(errFail);
	}

CDatum CDatatypeRange::OnGetRangeType () const
	{
	return m_dBaseType;
	}

IDatatype::EMemberType CDatatypeRange::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const
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

bool CDatatypeRange::OnIsA (const IDatatype &Type) const
	{
	if (Type.GetClass() != ECategory::Range)
		return false;

	const IDatatype& BaseType = m_dBaseType;
	if (!BaseType.IsA(Type.GetRangeType()))
		return false;

	//	We match.

	return true;
	}

void CDatatypeRange::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	DWORD dwFlags = 0;
	Stream.Write(dwFlags);

	m_dBaseType.SerializeAEON(Stream, Serialized);
	}
