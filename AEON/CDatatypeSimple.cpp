//	CDatatypeSimple.cpp
//
//	CDatatypeSimple class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeSimple::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream)
	{
	Stream.Read(&m_dwCoreType, sizeof(DWORD));

	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	DWORD dwFlags;
	Stream.Read(&dwFlags, sizeof(DWORD));
	m_bAbstract = ((dwFlags & 0x00000001) ? true : false);

	return true;
	}

bool CDatatypeSimple::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeSimple &)Src;

	if (m_dwCoreType != Other.m_dwCoreType)
		return false;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_bAbstract != Other.m_bAbstract)
		return false;

	return true;
	}

void CDatatypeSimple::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	Stream.Write(&m_dwCoreType, sizeof(DWORD));

	m_Implements.Serialize(iFormat, Stream);

	DWORD dwFlags = 0;
	dwFlags |= (m_bAbstract ? 0x00000001 : 0);
	Stream.Write(&dwFlags, sizeof(DWORD));
	}
