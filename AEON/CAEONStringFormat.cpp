//	CAEONStringFormat.cpp
//
//	CAEONStringFormat class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_STRING_FORMAT,		"stringFormat");

TDatumPropertyHandler<CAEONStringFormat> CAEONStringFormat::m_Properties = {
	};

TDatumMethodHandler<CAEONStringFormat> CAEONStringFormat::m_Methods = {
	};

const CString& CAEONStringFormat::StaticGetTypename (void)
	{
	return TYPENAME_STRING_FORMAT;
	}

size_t CAEONStringFormat::CalcMemorySize () const
	{
	return CString(m_Format.GetFormatString()).CalcSerializeSize();
	}

CDatum CAEONStringFormat::GetDatatype () const
	{
	return CAEONTypes::Get(IDatatype::STRING_FORMAT_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONStringFormat::GetMembers (void)
	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

int CAEONStringFormat::OpCompare (CDatum::Types iValueType, CDatum dValue) const
	{
	return ::KeyCompare(m_Format.GetFormatString(), dValue.AsStringView());
	}

bool CAEONStringFormat::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const
	{
	return strEquals(m_Format.GetFormatString(), dValue.AsStringView());
	}

bool CAEONStringFormat::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const
	{
	return strEquals(m_Format.GetFormatString(), dValue.AsStringView());
	}

bool CAEONStringFormat::OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream)
	{
	CString sFormat = CString::Deserialize(Stream);
	m_Format = CStringFormat(sFormat);
	return true;
	}

void CAEONStringFormat::OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const
	{
	m_Format.GetFormatString().Serialize(Stream);
	}

void CAEONStringFormat::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized)
	{
	CString sFormat = CString::Deserialize(Stream);
	m_Format = CStringFormat(sFormat);
	}

void CAEONStringFormat::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_Format.GetFormatString().Serialize(Stream);
	}
