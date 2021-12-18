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
	int iTimeMs = m_TimeSpan.MillisecondsSinceMidnight();
	int iMilliseconds = iTimeMs % 1000;
	int iTimeSecs = iTimeMs / 1000;
	int iSeconds = iTimeSecs % 60;
	int iTimeMinutes = iTimeSecs / 60;
	int iMinutes = iTimeMinutes % 60;
	int iHours = iTimeMinutes % 60;

	if (m_TimeSpan.Days() == 0)
		{
		return strPattern("%02d:%02d:%02d.%03d",
				iHours,
				iMinutes,
				iSeconds,
				iMilliseconds
				);
		}
	else
		{
		return strPattern("%d:%02d:%02d:%02d.%03d",
				m_TimeSpan.Days(),
				iHours,
				iMinutes,
				iSeconds,
				iMilliseconds
				);
		}
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

		case PART_MILLISECONDS:
			return CDatum(m_TimeSpan.MillisecondsSinceMidnight());

		case PART_NEGATIVE:
			return CDatum(m_TimeSpan.IsNegative() ? -1 : 1);

		default:
			return CDatum();
		}
	}

CDatum CAEONTimeSpan::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element.

	{
	return CDatum();
	}

const CString &CAEONTimeSpan::GetTypename (void) const
	{
	return TYPENAME_TIME_SPAN;
	}

size_t CAEONTimeSpan::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeAEONScript
//
//	Returns the approximate size of the serialization.

	{
	return 25;
	}

bool CAEONTimeSpan::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Load

	{
	int iLoad;
	Stream.Read(&iLoad, sizeof(iLoad));
	bool bNegative = (iLoad < 0);
	int iDays = Abs(iLoad) - 1;

	int iMilliseconds;
	Stream.Read(&iMilliseconds, sizeof(iMilliseconds));

	m_TimeSpan = CTimeSpan(iDays, iMilliseconds, bNegative);
	return true;
	}

void CAEONTimeSpan::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	//	We encode the sign in the days values, so we need to bias by 1.

	int iSave = (m_TimeSpan.Days() + 1);
	if (m_TimeSpan.IsNegative())
		iSave = -iSave;

	Stream.Write(&iSave, sizeof(iSave));

	//	Write milliseconds

	iSave = m_TimeSpan.MillisecondsSinceMidnight();
	Stream.Write(&iSave, sizeof(iSave));
	}
