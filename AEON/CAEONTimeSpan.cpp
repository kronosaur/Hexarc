//	CAEONTimeSpan.cpp
//
//	CAEONTimeSpan class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_TABLE,					"table");
DECLARE_CONST_STRING(TYPENAME_TIME_SPAN,				"timeSpan");

CString CAEONTimeSpan::AsString (void) const

//	AsString
//
//	Returns the timespan as a string.

	{
	
	}

CDatum CAEONTimeSpan::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element.

	{
	switch (iIndex)
		{
		case PART_DAYS:
			return CDatum(m_TimeSpan.Days());

		case partMonth:
			return CDatum(m_DateTime.Month());

		case partDay:
			return CDatum(m_DateTime.Day());

		case partHour:
			return CDatum(m_DateTime.Hour());

		case partMinute:
			return CDatum(m_DateTime.Minute());

		case partSecond:
			return CDatum(m_DateTime.Second());

		case partMillisecond:
			return CDatum(m_DateTime.Millisecond());

		default:
			return CDatum();
		}
	}

CDatum CAEONTimeSpan::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element.

	{
	}

const CString &CAEONTimeSpan::GetTypename (void) const
	{
	return TYPENAME_TIME_SPAN;
	}

void CAEONTimeSpan::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize the timespan

	{
	}

size_t CAEONTimeSpan::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeAEONScript
//
//	Computes the serialization size.

	{
	}
