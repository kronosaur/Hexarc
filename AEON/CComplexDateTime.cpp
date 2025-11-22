//	CComplexDateTime.cpp
//
//	CComplexDateTime class
//	Copyright (c) 2010 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(DATETIME_YEAR,						"year");
DECLARE_CONST_STRING(DATETIME_MONTH,					"month");
DECLARE_CONST_STRING(DATETIME_DAY,						"day");
DECLARE_CONST_STRING(DATETIME_HOUR,						"hour");
DECLARE_CONST_STRING(DATETIME_MINUTE,					"minute");
DECLARE_CONST_STRING(DATETIME_SECOND,					"second");
DECLARE_CONST_STRING(DATETIME_MILLISECOND,				"millisecond");

DECLARE_CONST_STRING(TYPENAME_DATETIME,					"dateTime");

TDatumPropertyHandler<CComplexDateTime> CComplexDateTime::m_Properties = {
	{
		"date",
		"d",
		"Returns the date portion of the timeDate.",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return CDatum(CDateTime(Obj.m_DateTime.Day(), Obj.m_DateTime.Month(), Obj.m_DateTime.Year()));
			},
		NULL,
		},
	{
		"day",
		"I",
		"Returns the day of the month 1-31.",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.Day();
			},
		NULL,
		},
	{
		"dayOfWeek",
		"$DayOfWeek",
		"Returns the day of the the week of this date.",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return CDatum::CreateEnum(Obj.m_DateTime.DayOfWeek(), CAEONTypes::Get(IDatatype::DAY_OF_WEEK_ENUM));
			},
		NULL,
		},
	{
		"days",
		"i",
		"Returns the number of days since 1 AD.",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.DaysSince1AD();
			},
		NULL,
		},
	{
		"hour",
		"I",
		"Returns the hour (0-23).",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.Hour();
			},
		NULL,
		},
	{
		"millisecond",
		"I",
		"Returns the milliseconds (0-999).",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.Millisecond();
			},
		NULL,
		},
	{
		"minute",
		"I",
		"Returns the minute (0-59).",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.Minute();
			},
		NULL,
		},
	{
		"month",
		"I",
		"Returns the month (1-12).",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.Month();
			},
		NULL,
		},
	{
		"second",
		"I",
		"Returns the seconds (0-59).",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.Second();
			},
		NULL,
		},
	{
		"time",
		"m",
		"Returns the TimeSpan since midnight.",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return CDatum(CTimeSpan(Obj.m_DateTime.MillisecondsSinceMidnight()));
			},
		NULL,
		},
	{
		"year",
		"I",
		"Returns the year.",
		[](const CComplexDateTime &Obj, const CString &sProperty)
			{
			return Obj.m_DateTime.Year();
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CComplexDateTime::m_pMethodsExt = NULL;

CString CComplexDateTime::AsString (void) const

//	AsString
//
//	NOTE: We rely on the fact that the returned string is sortable (i.e.,
//	comparable to other strings).

	{
	if (m_DateTime.HasDate() && m_DateTime.HasTime())
		{
		if (m_DateTime.Millisecond() == 0)
			return strPattern("%04d-%02d-%02dT%02d:%02d:%02d",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second());
		else
			return strPattern("%04d-%02d-%02dT%02d:%02d:%02d.%03d",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());
		}
	else if (m_DateTime.HasDate())
		{
		return strPattern("%04d-%02d-%02d",
				m_DateTime.Year(),
				m_DateTime.Month(),
				m_DateTime.Day());
		}
	else if (m_DateTime.HasTime())
		{
		if (m_DateTime.Millisecond() == 0)
			return strPattern("%02d:%02d:%02d",
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second());
		else
			return strPattern("%02d:%02d:%02d.%03d",
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());
		}
	else
		return NULL_STR;
	}

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

		case CDatum::EFormat::GridLang:
			{
			Stream.Write(strPattern("DateTime(\"%d-%02d-%02dT%02d:%02d:%02d.%03d\")",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond()));
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

CDatum CComplexDateTime::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CComplexDateTime* pDatum = new CComplexDateTime;
	CDatum dValue(pDatum);
	Stream.Read(&pDatum->m_DateTime, sizeof(m_DateTime));
	return dValue;
	}

void CComplexDateTime::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_DATE_TIME);

	Stream.Write(&m_DateTime, sizeof(m_DateTime));
	}
