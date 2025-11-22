//	CDatatypeSimple.cpp
//
//	CDatatypeSimple class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_EXPECT_INDEX,					"Expected index.");


bool CDatatypeSimple::OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType, CString* retsError) const
	{
	if (IsA(IDatatype::INDEXED))
		{
		if (ArgTypes.GetCount() < 1)
			{
			if (retsError) *retsError = ERR_EXPECT_INDEX;
			return false;
			}

		if (retdReturnType)
			*retdReturnType = CAEONTypes::Get(IDatatype::ANY);

		return true;
		}
	else
		return false;
	}

bool CDatatypeSimple::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	SetCoreType(Stream.ReadDWORD());

	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	DWORD dwFlags;
	Stream.Read(&dwFlags, sizeof(DWORD));
	m_bAbstract = ((dwFlags & 0x00000001) ? true : false);
	m_bCanBeNull = ((dwFlags & 0x00000002) ? true : false);
	m_bNoMembers = ((dwFlags & 0x00000004) ? true : false);

	return true;
	}

bool CDatatypeSimple::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeSimple &)Src;

	if (GetCoreType() != Other.GetCoreType())
		return false;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_bAbstract != Other.m_bAbstract)
		return false;

	return true;
	}

bool CDatatypeSimple::OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized)
	{
	if (!CDatatypeList::DeserializeAEON(Stream, Serialized, m_Implements))
		return false;

	DWORD dwFlags = Stream.ReadDWORD();
	m_bAbstract = ((dwFlags & 0x00000001) ? true : false);
	m_bCanBeNull = ((dwFlags & 0x00000002) ? true : false);
	m_bNoMembers = ((dwFlags & 0x00000004) ? true : false);

	return true;
	}

void CDatatypeSimple::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_Implements.SerializeAEON(Stream, Serialized);

	DWORD dwFlags = 0;
	dwFlags |= (m_bAbstract ? 0x00000001 : 0);
	dwFlags |= (m_bCanBeNull ? 0x00000002 : 0);
	dwFlags |= (m_bNoMembers ? 0x00000004 : 0);
	Stream.Write(dwFlags);
	}
