//	CComplexDateTime.cpp
//
//	CComplexDateTime class
//	Copyright (c) 2010 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(DATETIME_YEAR,						"year");
DECLARE_CONST_STRING(DATETIME_MONTH,					"month");
DECLARE_CONST_STRING(DATETIME_DAY,						"day");
DECLARE_CONST_STRING(DATETIME_HOUR,						"hour");
DECLARE_CONST_STRING(DATETIME_MINUTE,					"minute");
DECLARE_CONST_STRING(DATETIME_SECOND,					"second");
DECLARE_CONST_STRING(DATETIME_MILLISECOND,				"millisecond");

DECLARE_CONST_STRING(TYPENAME_DATETIME,					"dateTime");

bool CComplexDateTime::CreateFromString (const CString &sString, CDateTime *retDateTime)

//	CreateFromString
//
//	Reverse of AsString

	{
	const char *pPos = sString.GetParsePointer();

	int iYear = strParseInt(pPos, 0, &pPos);
	if (iYear < 1 || iYear > 30827)
		return false;

	if (*pPos++ != '-')
		return false;

	int iMonth = strParseInt(pPos, 0, &pPos);
	if (iMonth < 1 || iMonth > 12)
		return false;

	if (*pPos++ != '-')
		return false;

	int iDay = strParseInt(pPos, 0, &pPos);
	if (iDay < 1 || iDay > CDateTime::GetDaysInMonth(iMonth, iYear))
		return false;

	if (*pPos++ != 'T')
		return false;

	int iHour = strParseInt(pPos, -1, &pPos);
	if (iHour < 0 || iHour > 23)
		return false;

	if (*pPos++ != ':')
		return false;

	int iMinute = strParseInt(pPos, -1, &pPos);
	if (iMinute < 0 || iMinute > 59)
		return false;

	if (*pPos++ != ':')
		return false;

	int iSecond = strParseInt(pPos, -1, &pPos);
	if (iSecond < 0 || iSecond > 59)
		return false;

	if (*pPos++ != '.')
		return false;

	int iMillisecond = strParseInt(pPos, -1, &pPos);
	if (iMillisecond < 0 || iMillisecond > 999)
		return false;

	//	Done

	*retDateTime = CDateTime(iDay, iMonth, iYear, iHour, iMinute, iSecond, iMillisecond);
	return true;
	}

bool CComplexDateTime::CreateFromString (const CString &sString, CDatum *retdDatum)

//	CreateFromString
//
//	Creates a datum from a string

	{
	CDateTime DateTime;
	if (!CreateFromString(sString, &DateTime))
		return false;

	*retdDatum = CDatum(DateTime);
	return true;
	}

CDatum CComplexDateTime::GetElement (int iIndex) const

//	GetElement
//
//	Returns a dateTime component

	{
	switch (iIndex)
		{
		case partYear:
			return CDatum(m_DateTime.Year());

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

CDatum CComplexDateTime::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns a dateTime component

	{
	if (strEquals(sKey, DATETIME_YEAR))
		return CDatum(m_DateTime.Year());
	else if (strEquals(sKey, DATETIME_MONTH))
		return CDatum(m_DateTime.Month());
	else if (strEquals(sKey, DATETIME_DAY))
		return CDatum(m_DateTime.Day());
	else if (strEquals(sKey, DATETIME_HOUR))
		return CDatum(m_DateTime.Hour());
	else if (strEquals(sKey, DATETIME_MINUTE))
		return CDatum(m_DateTime.Minute());
	else if (strEquals(sKey, DATETIME_SECOND))
		return CDatum(m_DateTime.Second());
	else if (strEquals(sKey, DATETIME_MILLISECOND))
		return CDatum(m_DateTime.Millisecond());
	else
		return CDatum();
	}

const CString &CComplexDateTime::GetTypename (void) const { return TYPENAME_DATETIME; }

size_t CComplexDateTime::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return 25;
	}

void CComplexDateTime::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize to the given format.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			CString sDate = strPattern("#%d-%02d-%02dT%02d:%02d:%02d.%03d",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());

			Stream.Write(sDate);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			CString sDate = strPattern("\"%d-%02d-%02dT%02d:%02d:%02d.%03d\"",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());

			Stream.Write(sDate);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}
