//	CComplexInteger.cpp
//
//	CComplexInteger class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_IP_INTEGER,				"ipInteger")
const CString &CComplexInteger::GetTypename (void) const { return TYPENAME_IP_INTEGER; }

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

CDatum::Types CComplexInteger::GetNumberType (int *retiValue)

//	GetNumberType
//
//	Returns the number type

	{
	//	If we fit in a DWORDLONG, then return that.

	if (m_Value.FitsAsInteger64Unsigned())
		return CDatum::typeInteger64;

	//	Otherwise we need to be an infinite precision integer

	else
		return CDatum::typeIntegerIP;
	}

size_t CComplexInteger::OnCalcSerializeSizeAEONScript (CDatum::ESerializationFormats iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	switch (iFormat)
		{
		case CDatum::formatAEONLocal:
		case CDatum::formatAEONScript:
			return m_Value.AsString().GetLength();

		default:
			ASSERT(false);
			return 0;
		}
	}

void CComplexInteger::Serialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize to the given format.

	{
	switch (iFormat)
		{
		case CDatum::formatAEONScript:
		case CDatum::formatAEONLocal:
			Stream.Write(m_Value.AsString());
			break;

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

