//	CComplexInteger.cpp
//
//	CComplexInteger class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_IP_INTEGER,				"ipInteger")
const CString &CComplexInteger::GetTypename (void) const { return TYPENAME_IP_INTEGER; }

int CComplexInteger::AsArrayIndex (int iArrayLen, bool* retbFromEnd) const

//	AsArrayIndex
//
//	Converts to 32-bit array index (or -1).

	{
	if (m_Value.FitsAsInteger32Signed())
		return CDatum::CalcArrayIndex(m_Value.AsInteger32Signed(), iArrayLen, retbFromEnd);
	else
		return -1;
	}

DWORDLONG CComplexInteger::CastDWORDLONG (void) const

//	CastDWORDLONG
//
//	Cast to a DWORDLONG. This function will truncate if the integer does not
//	fit.

	{
	return m_Value.AsInteger64Unsigned();
	}

int CComplexInteger::CastInteger32 (void) const

//	CastInteger32
//
//	Cast to 32-bit int. This function will truncate if the integer does not fit.

	{
	return m_Value.AsInteger32Signed();
	}

CDatum CComplexInteger::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CComplexInteger *pValue = new CComplexInteger;
	CDatum dValue(pValue);

	if (!CIPInteger::Deserialize(Stream, &pValue->m_Value))
		return CDatum();

	return dValue;
	}

void CComplexInteger::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	No need for ID because we're a primitive (can't contain other objects).
	Stream.Write(CDatum::SERIALIZE_TYPE_INTIP);
	m_Value.Serialize(Stream);
	}

CDatum::Types CComplexInteger::GetNumberType (int *retiValue)

//	GetNumberType
//
//	Returns the number type

	{
	//	If we fit as a 32-bit integer.

	if (m_Value.FitsAsInteger32Signed())
		{
		if (retiValue)
			*retiValue = m_Value.AsInteger32Signed();

		return CDatum::typeInteger32;
		}

	//	If we fit in a DWORDLONG, then return that.

	else if (m_Value.FitsAsInteger64Unsigned())
		return CDatum::typeInteger64;

	//	Otherwise we need to be an infinite precision integer

	else
		return CDatum::typeIntegerIP;
	}

CDatum CComplexInteger::MathAbs () const

//	MathAbs
//
//	Absolte value.

	{
	if (m_Value.IsNegative())
		return CDatum(-m_Value);
	else
		return CDatum::raw_AsComplex(this);
	}

size_t CComplexInteger::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONLocal:
		case CDatum::EFormat::AEONScript:
			return m_Value.AsString().GetLength();

		default:
			ASSERT(false);
			return 0;
		}
	}

void CComplexInteger::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize to the given format.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
		case CDatum::EFormat::GridLang:
			Stream.Write(m_Value.AsString());
			break;

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

