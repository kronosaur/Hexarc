//	CDatatypeQualified.cpp
//
//	CDatatypeQualified class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatatypeQualified::CDatatypeQualified (CDatum dType, DWORD dwQualifiedFlags) :
		IDatatype(false, NULL_STR, 0, false)

//	CDatatypeQualified constructor

	{
	m_dType = dType;
	m_pType = const_cast<IDatatype*>(&(const IDatatype&)dType);
	m_dwQualifiedFlags = dwQualifiedFlags;
	}

bool CDatatypeQualified::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	throw CException(errFail);
	}

bool CDatatypeQualified::OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized)
	{
	throw CException(errFail);
	}

IDatatype::EImplementation CDatatypeQualified::OnGetImplementation () const
	{
	return m_pType->GetImplementation();
	}

void CDatatypeQualified::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	throw CException(errFail);
	}
