//	CDatatypeNumber.cpp
//
//	CDatatypeNumber class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeNumber::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream)
	{
	Stream.Read(&m_dwCoreType, sizeof(DWORD));

	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	Stream.Read(&m_iBits, sizeof(DWORD));

	DWORD dwFlags;
	Stream.Read(&dwFlags, sizeof(DWORD));
	m_bFloat =		((dwFlags & 0x00000001) ? true : false);
	m_bUnsigned =	((dwFlags & 0x00000002) ? true : false);

	return true;
	}

bool CDatatypeNumber::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeNumber &)Src;

	if (m_dwCoreType != Other.m_dwCoreType)
		return false;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_iBits != Other.m_iBits)
		return false;

	if (m_bFloat != Other.m_bFloat)
		return false;

	if (m_bUnsigned != Other.m_bUnsigned)
		return false;

	return true;
	}

void CDatatypeNumber::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	Stream.Write(&m_dwCoreType, sizeof(DWORD));

	m_Implements.Serialize(iFormat, Stream);

	Stream.Write(&m_iBits, sizeof(DWORD));

	DWORD dwFlags = 0;
	dwFlags |= (m_bFloat ?		0x00000001 : 0);
	dwFlags |= (m_bUnsigned ?	0x00000002 : 0);
	Stream.Write(&dwFlags, sizeof(DWORD));
	}
