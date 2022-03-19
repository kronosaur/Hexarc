//	CDatatypeArray.cpp
//
//	CDatatypeArray class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeArray::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream)
	{
	Stream.Read(&m_dwCoreType, sizeof(DWORD));

	TUniquePtr<IDatatype> pType = IDatatype::Deserialize(iFormat, Stream);
	if (!pType)
		return false;

	m_dElementType = CDatum(new CComplexDatatype(std::move(pType)));

	return true;
	}

bool CDatatypeArray::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeArray &)Src;

	if (m_dwCoreType != Other.m_dwCoreType)
		return false;

	if ((const IDatatype &)m_dElementType != (const IDatatype &)Other.m_dElementType)
		return false;

	return true;
	}

void CDatatypeArray::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	Stream.Write(&m_dwCoreType, sizeof(DWORD));

	const IDatatype &Type = m_dElementType;
	Type.Serialize(iFormat, Stream);
	}
