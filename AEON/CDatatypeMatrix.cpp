//	CDatatypeMatrix.cpp
//
//	CDatatypeMatrix class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeMatrix::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream)
	{
	Stream.Read(&m_dwCoreType, sizeof(DWORD));

	TUniquePtr<IDatatype> pType = IDatatype::Deserialize(iFormat, Stream);
	if (!pType)
		return false;

	m_dElementType = CDatum(new CComplexDatatype(std::move(pType)));

	Stream.Read(&m_iRows, sizeof(DWORD));
	Stream.Read(&m_iCols, sizeof(DWORD));

	return true;
	}

bool CDatatypeMatrix::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeMatrix &)Src;

	if (m_dwCoreType != Other.m_dwCoreType)
		return false;

	if ((const IDatatype &)m_dElementType != (const IDatatype &)Other.m_dElementType)
		return false;

	if (m_iRows != Other.m_iRows)
		return false;

	if (m_iCols != Other.m_iCols)
		return false;

	return true;
	}

void CDatatypeMatrix::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	Stream.Write(&m_dwCoreType, sizeof(DWORD));

	const IDatatype &Type = m_dElementType;
	Type.Serialize(iFormat, Stream);

	Stream.Write(&m_iRows, sizeof(DWORD));
	Stream.Write(&m_iCols, sizeof(DWORD));
	}
