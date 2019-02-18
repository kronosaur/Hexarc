//	CDateTime.cpp
//
//	CDateTime class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_UN,							"un")
DECLARE_CONST_STRING(STR_UNK,							"unk")
DECLARE_CONST_STRING(STR_UNKNOWN,						"unknown")

int g_DaysInMonth[] =
	//	J   F   M   A   M   J   J   A   S   O   N   D
	{	31,	28,	31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int g_DaysOfYearByMonth[] =
	//		J   F   M   A   M   J   J   A   S   O   N   D
	{	0,  31, 59, 90, 120,151,181,212,243,273,304,334,365 };

int g_DaysOfLeapYearByMonth[] =
	//		J   F   M   A   M   J   J   A   S   O   N   D
	{	0,  31, 60, 91, 121,152,182,213,244,274,305,335,366 };

char *g_szMonthName[] =
	{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
	};

char *g_szMonthNameIMF[] =
	{
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
	};

char *g_szDayNameIMF[] =
	{
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	};

CDateTime NULL_DATETIME;

//	CDateTime -----------------------------------------------------------------

CDateTime::CDateTime (void)

//	CDateTime constructor

	{
	m_Time.wYear = 1;
	m_Time.wMonth = 1;
	m_Time.wDay = 1;
	m_Time.wHour = 0;
	m_Time.wMinute = 0;
	m_Time.wSecond = 0;
	m_Time.wMilliseconds = 0;
	}

CDateTime::CDateTime (Constants Init)

//	CDateTime constructor

	{
	switch (Init)
		{
		case Now:
			::GetSystemTime(&m_Time);
			break;

		//	WARNING: Not sure if this works for timezones other than GMT

		case Today:
			::GetSystemTime(&m_Time);

			m_Time.wHour = 0;
			m_Time.wMinute = 0;
			m_Time.wSecond = 0;
			m_Time.wMilliseconds = 0;
			break;

		case BeginningOfTime:
			m_Time.wYear = 1;
			m_Time.wMonth = 1;
			m_Time.wDay = 1;
			m_Time.wHour = 0;
			m_Time.wMinute = 0;
			m_Time.wSecond = 0;
			m_Time.wMilliseconds = 0;
			break;

		case LocalNow:
			::GetSystemTime(&m_Time);
			::SystemTimeToTzSpecificLocalTime(NULL, &m_Time, &m_Time);
			break;

		default:
			ASSERT(false);
		};
	}

CDateTime::CDateTime (int iDaysSince1AD, int iMillisecondsSinceMidnight)

//	CDateTime constructor

	{
	//	Take a stab at figuring out the year

	int iY1 = (400 * iDaysSince1AD - 146000) / 146097;
	
	int iDays1 = 0;
	if (iY1 > 0)
		{
		iDays1 += 365 * iY1;
		iDays1 += iY1 / 4;
		iDays1 -= iY1 / 100;
		iDays1 += iY1 / 400;
		}

	iDays1 = iDaysSince1AD - iDays1;

	//	Adjust if we're over

	int iDaysInY1 = (timeIsLeapYear(iY1+1) ? 366 : 365);
	while (iDays1 >= iDaysInY1)
		{
		iY1++;
		iDays1 -= iDaysInY1;

		iDaysInY1 = (timeIsLeapYear(iY1+1) ? 366 : 365);
		}

	m_Time.wYear = (iY1 + 1);

	//	Calculate month and day

	m_Time.wMonth = 1;
	if (timeIsLeapYear(m_Time.wYear))
		{
		while (iDays1 + 1 > g_DaysOfLeapYearByMonth[m_Time.wMonth])
			m_Time.wMonth++;

		m_Time.wDay = (iDays1 - g_DaysOfLeapYearByMonth[m_Time.wMonth-1]) + 1;
		}
	else
		{
		while (iDays1 + 1 > g_DaysOfYearByMonth[m_Time.wMonth])
			m_Time.wMonth++;

		m_Time.wDay = (iDays1 - g_DaysOfYearByMonth[m_Time.wMonth-1]) + 1;
		}

	//	Calculate time

	m_Time.wHour = iMillisecondsSinceMidnight / (60 * 60 * 1000);
	m_Time.wMinute = (iMillisecondsSinceMidnight % (60 * 60 * 1000)) / (60 * 1000);
	m_Time.wSecond = (iMillisecondsSinceMidnight % (60 * 1000)) / 1000;
	m_Time.wMilliseconds = iMillisecondsSinceMidnight % 1000;

	//	Don't yet calculate day of week

	m_Time.wDayOfWeek = 0xffff;
	}

CDateTime::CDateTime (int iDay, int iMonth, int iYear)

//	CDateTime constructor

	{
	m_Time.wYear = iYear;
	m_Time.wMonth = iMonth;
	m_Time.wDay = iDay;

	m_Time.wHour = 0;
	m_Time.wMinute = 0;
	m_Time.wSecond = 0;
	m_Time.wMilliseconds = 0;

	m_Time.wDayOfWeek = 0xffff;
	}

CDateTime::CDateTime (int iDay, int iMonth, int iYear, int iHour, int iMinute, int iSecond, int iMillisecond)

//	CDateTime constructor

	{
	m_Time.wYear = iYear;
	m_Time.wMonth = iMonth;
	m_Time.wDay = iDay;

	m_Time.wHour = iHour;
	m_Time.wMinute = iMinute;
	m_Time.wSecond = iSecond;
	m_Time.wMilliseconds = iMillisecond;

	m_Time.wDayOfWeek = 0xffff;
	}

CDateTime CDateTime::AsLocalTime (void) const

//	AsLocalTime
//
//	Converts the given time to local time (assumes time is UTC)

	{
	CDateTime Local;
	Local.m_Time = m_Time;
	::SystemTimeToTzSpecificLocalTime(NULL, &Local.m_Time, &Local.m_Time);
	return Local;
	}

int CDateTime::Compare (const CDateTime &Src) const

//	Compare
//
//	If this > Src,		1
//	If this == Src,		0
//	If this < Src,		-1

	{
	if (m_Time.wYear > Src.m_Time.wYear)
		return 1;
	else if (m_Time.wYear < Src.m_Time.wYear)
		return -1;
	else if (m_Time.wMonth > Src.m_Time.wMonth)
		return 1;
	else if (m_Time.wMonth < Src.m_Time.wMonth)
		return -1;
	else if (m_Time.wDay > Src.m_Time.wDay)
		return 1;
	else if (m_Time.wDay < Src.m_Time.wDay)
		return -1;
	else if (m_Time.wHour > Src.m_Time.wHour)
		return 1;
	else if (m_Time.wHour < Src.m_Time.wHour)
		return -1;
	else if (m_Time.wMinute > Src.m_Time.wMinute)
		return 1;
	else if (m_Time.wMinute < Src.m_Time.wMinute)
		return -1;
	else if (m_Time.wSecond > Src.m_Time.wSecond)
		return 1;
	else if (m_Time.wSecond < Src.m_Time.wSecond)
		return -1;
	else if (m_Time.wMilliseconds > Src.m_Time.wMilliseconds)
		return 1;
	else if (m_Time.wMilliseconds < Src.m_Time.wMilliseconds)
		return -1;
	else
		return 0;
	}

int CDateTime::DayOfWeek (void) const

//	DayOfWeek
//
//	Returns the day of week of the current date. 0 = Sunday.
//	See: http://www.faqs.org/faqs/calendars/faq/part1/index.html

	{
	int a = (14 - m_Time.wMonth) / 12;
	int y = m_Time.wYear - a;
	int m = m_Time.wMonth + 12 * a - 2;

	//	LATER: We assume Gregorian calendar

	return (m_Time.wDay + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12) % 7;
	}

int CDateTime::DaysSince1AD (void) const

//	DaysSince1AD
//
//	Returns the number of days elapsed since Jan 1, 1 AD.

	{
	int iDays = 0;

	//	Add all years before this one

	iDays += 365 * (Year() - 1);
	iDays += (Year() - 1) / 4;
	iDays -= (Year() - 1) / 100;
	iDays += (Year() - 1) / 400;

	//	Now add the day months for this year

	if (timeIsLeapYear(Year()))
		iDays += g_DaysOfLeapYearByMonth[Month()-1];
	else
		iDays += g_DaysOfYearByMonth[Month()-1];

	iDays += Day()-1;

	return iDays;
	}

CString CDateTime::Format (const CString &sFormat) const

//	Format
//
//	Formats a datetime
//
//    %a    abbreviated weekday name (Sun)
//    %A    full weekday name (Sunday)
//    %b    abbreviated month name (Dec)
//    %B    full month name (December)
//    %c    date and time (Dec  2 06:55:15 1979)
//    %d    day of the month (02)
//    %H    hour of the 24-hour day (06)
//    %I    hour of the 12-hour day (06)
//    %j    day of the year, from 001 (335)
//    %m    month of the year, from 01 (12)
//    %M    minutes after the hour (55)
//    %p    AM/PM indicator (AM)
//    %S    seconds after the minute (15)
//    %U    Sunday week of the year, from 00 (48)
//    %w    day of the week, from 0 for Sunday (6)
//    %W    Monday week of the year, from 00 (47)
//    %x    date (Dec  2 1979)
//    %X    time (06:55:15)
//    %y    year of the century, from 00 (79)
//    %Y    year (1979)
//    %Z    time zone name, if any (EST)
//    %%    percent character %

	{
	return CString("not yet implemented");
	}

CString CDateTime::Format (DateFormats iDateFormat, TimeFormats iTimeFormat) const

//	Format
//
//	Formats using OS functions

	{
	//	Relative dates

	if (iDateFormat == dfRelative)
		{
		CTimeSpan Span = timeSpan(*this, CDateTime(Now));
		DWORDLONG dwSeconds = Span.Seconds64();
		if (dwSeconds < SECONDS_PER_MINUTE)
			return CString("just now");
		else if (dwSeconds < SECONDS_PER_MINUTE + (SECONDS_PER_MINUTE / 2))
			return CString("1 minute ago");
		else if (dwSeconds < SECONDS_PER_HOUR)
			return strPattern("%d minutes ago", (int)(dwSeconds / SECONDS_PER_MINUTE));
		else if (dwSeconds < SECONDS_PER_HOUR + (SECONDS_PER_HOUR / 2))
			return CString("1 hour ago");
		else if (dwSeconds < 2 * SECONDS_PER_DAY)
			return strPattern("%d hours ago", (int)(dwSeconds / SECONDS_PER_HOUR));
		else if (dwSeconds < 30 * SECONDS_PER_DAY)
			return strPattern("%d days ago", (int)(dwSeconds / SECONDS_PER_DAY));
		else
			return strPattern("%d %s %d", Day(), CString(g_szMonthNameIMF[Month() - 1], 3), Year());
		}

	//	Formats based on system calls

	else
		{
		TCHAR szBuffer[1024];
		TCHAR *pPos = szBuffer;
		TCHAR *pPosEnd = pPos + (sizeof(szBuffer) / sizeof(szBuffer[0]));

		//	Compose flags for the calls

		bool bDate = true;
		DWORD dwDateFlags;
		switch (iDateFormat)
			{
			case dfNone:
				bDate = false;
				break;

			case dfShort:
				dwDateFlags = DATE_SHORTDATE;
				break;

			case dfLong:
				dwDateFlags = DATE_LONGDATE;
				break;

			default:
				ASSERT(false);
			}

		bool bTime = true;
		DWORD dwTimeFlags;
		switch (iTimeFormat)
			{
			case tfNone:
				bTime = false;
				break;

			case tfShort:
				dwTimeFlags = TIME_NOSECONDS;
				break;

			case tfShort24:
				dwTimeFlags = TIME_NOSECONDS | TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER;
				break;

			case tfLong:
				dwTimeFlags = 0;
				break;

			case tfLong24:
				dwTimeFlags = TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER;
				break;

			default:
				ASSERT(false);
			}

		//	Format

		int iLen = 0;
		if (bDate)
			{
			iLen = ::GetDateFormat(LOCALE_USER_DEFAULT,
					dwDateFlags,
					&m_Time,
					NULL,
					pPos,
					(int)(pPosEnd - pPos));
			pPos += iLen;

			//	If we have a time then we don't need the terminating NULL.

			if (bTime)
				pPos[-1] = _T(' ');
			}

		//	Format the time

		if (bTime)
			{
			iLen = ::GetTimeFormat(LOCALE_USER_DEFAULT,
					dwTimeFlags,
					&m_Time,
					NULL,
					pPos,
					(int)(pPosEnd - pPos));
			pPos += iLen;
			}

		//	Done

		*pPos = _T('\0');
		return CString(szBuffer, (int)(pPos - szBuffer) - 1);
		}
	}

CString CDateTime::FormatIMF (void) const

//	FormatIMF
//
//	Returns the current date time in Internet Message Format (RFC 2822)

	{
	return strPattern("%s, %02d %s %d %02d:%02d:%02d GMT",
			CString(g_szDayNameIMF[DayOfWeek()], 3),
			Day(),
			CString(g_szMonthNameIMF[Month() - 1], 3),
			Year(),
			Hour(),
			Minute(),
			Second());
	}

int CDateTime::GetDaysInMonth (int iMonth, int iYear)

//	GetDaysInMonth
//
//	Returns the number of days in a month

	{
	if (iMonth < 1 || iMonth > 12)
		return 0;

	if (iYear == 0 || !timeIsLeapYear(iYear))
		return g_DaysInMonth[iMonth - 1];
	else
		{
		if (iMonth == 2)
			return 29;
		else
			return g_DaysInMonth[iMonth - 1];
		}
	}

bool CDateTime::IsValid (void) const

//	IsValid
//
//	Returns FALSE if this is set to BeginningOfTime

	{
	return (m_Time.wYear != 1
			|| m_Time.wMonth != 1
			|| m_Time.wDay != 1
			|| m_Time.wHour != 0
			|| m_Time.wMinute != 0
			|| m_Time.wSecond != 0
			|| m_Time.wMilliseconds != 0);
	}

int CDateTime::MillisecondsSinceMidnight (void) const

//	MillisecondsSinceMidnight
//
//	Returns the number of milliseconds since midnight

	{
	return Millisecond()
			+ (Second() * 1000)
			+ (Minute() * 60 * 1000)
			+ (Hour() * 60 * 60 * 1000);
	}

bool CDateTime::Parse (StandardFormats iFormat, char *pPos, char *pPosEnd, CDateTime *retResult)

//	Parse
//
//	Parse standard formats. Returns TRUE if successful.

	{
	switch (iFormat)
		{
		case formatAuto:
		case formatInterpolateStart:
		case formatInterpolateEnd:
			return ParseAuto(pPos, pPosEnd, iFormat, retResult);

		case formatIMF:
			*retResult = ParseIMF(pPos, pPosEnd);
			return retResult->IsValid();

		case formatISO8601:
			return ParseISO8601(pPos, pPosEnd, retResult);

		default:
			ASSERT(false);
			return false;
		}
	}

bool CDateTime::Parse (StandardFormats iFormat, const CString &sValue, CDateTime *retResult)

//	Parse
//
//	Parse standard formats. Returns TRUE if successful.

	{
	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();

	return Parse(iFormat, pPos, pPosEnd, retResult);
	}

bool CDateTime::ParseAuto (char *pPos, char *pPosEnd, StandardFormats iFormat, CDateTime *retResult)

//	ParseAuto
//
//	Attempts to figure out the format. We parse the following formats:
//
//	11-23-2015
//	11-23-2015 1:30:00 PM
//	2015-11-23
//	2015-11-23T01:30:00Z
//	2015-11-23T01:30:00.000Z
//	23NOV2015
//	23 November 2015
//	November 23, 2015

	{
	enum EStates
		{
		stateStart,
		stateTime,
		stateTimeSeconds,
		stateTimeMilliseconds,
		stateAMPM,
		stateDone,
		};

	//	Keep looping until we're done, filling in each part that we find.

	int iYear = -1;
	int iMonth = -1;
	int iDay = -1;
	int iHour = -1;
	int iMinute = -1;
	int iSecond = -1;
	int iMillisecond = -1;
	int iDateParts = 0;
	bool bUnknownDate = false;
	bool bUnknownMonth = false;
	TArray<int> Unknown;

	EStates iState = stateStart;

	while (iState != stateDone)
		{
		//	Skip any whitespace or delimeters

		while (pPos < pPosEnd && !strIsASCIIAlphaNumeric(pPos))
			pPos++;

		//	State machine

		switch (iState)
			{
			case stateStart:
				{
				char *pStart = pPos;

				//	If we're at the end, then we don't have enough.

				if (pPos == pPosEnd)
					return false;

				//	Parse a token, either a number or a string

				else if (strIsDigit(pPos))
					{
					while (strIsDigit(pPos))
						pPos++;

					int iValue = strToInt(CString(pStart, (int)(pPos - pStart)), 0);

					//	If this is greater than 1900, then this is clearly a year

					if (iValue > 1900 && iYear == -1)
						{
						iYear = iValue;
						iDateParts++;
						}

					//	If we already have the year, but not the month, then we 
					//	assume that this is the month (since year-first formats 
					//	are not ambiguous).

					else if (iYear != -1 && iMonth == -1 && iValue <= 12)
						{
						iMonth = iValue;
						iDateParts++;
						}

					//	If we already have a month, then this is the day

					else if (iMonth != -1 && iValue <= 31)
						{
						iDay = iValue;
						iDateParts++;
						}

					//	Otherwise, this could be either a month or day

					else
						{
						Unknown.Insert(iValue);
						iDateParts++;
						}

					}
				else
					{
					while (strIsASCIIAlpha(pPos))
						pPos++;

					CString sValue = CString(pStart, (int)(pPos - pStart));

					//	If this is the token "un" or "unk" then it means an unknown
					//	component.

					if (strEqualsNoCase(sValue, STR_UN) || strEqualsNoCase(sValue, STR_UNK))
						{
						//	This is invalid unless we're interpolating

						if (iFormat == formatAuto)
							return false;

						//	If we've already got an unknown date, then this is an unknown
						//	month.
						//
						//	un unk 2016

						if (bUnknownDate)
							{
							bUnknownMonth = true;

							//	We interpolate based on whether we prefer start or end.

							iMonth = (iFormat == formatInterpolateEnd ? 12 : 1);
							iDateParts++;
							}

						//	Otherwise, this is either an unknown date or an unknown date
						//	AND and unknown month.
						//
						//	un Jan 2016
						//	un unk 2016
						//	un 2016

						else
							{
							bUnknownDate = true;
							}
						}

					//	Otherwise, if this is a month name, use that

					else if (ParseMonthIMF(sValue, &iMonth, true))
						{
						iDateParts++;
						bUnknownMonth = false;
						}
					else
						{
						//	Skip this token--this could be a day of the week.
						}
					}

				//	If we already have enough date parts, then continue to the
				//	time.

				if (iDateParts >= 3)
					iState = stateTime;

				//	If we've got month and year but an unknown date, then that's enough

				else if (iYear != -1 && iMonth != -1 && bUnknownDate)
					{
					iDay = (iFormat == formatInterpolateEnd ? GetDaysInMonth(iMonth, iYear) : 1);
					iState = stateTime;
					}

				//	If we've got a year and an unknown date, then we have enough

				else if (iYear != -1 && bUnknownDate)
					{
					iMonth = (iFormat == formatInterpolateEnd ? 12 : 1);
					iDay = (iFormat == formatInterpolateEnd ? 31 : 1);
					iState = stateTime;
					}

				break;
				}

			case stateTime:
				{
				//	If we're at the end then that's OK--we don't have a time 
				//	value.

				if (pPos == pPosEnd)
					{
					iHour = 0;
					iMinute = 0;
					iSecond = 0;
					iMillisecond = 0;
					iState = stateDone;
					}

				//	If we have a 'T', then this is the time separator

				else if (*pPos == 'T' || *pPos == 't')
					pPos++;

				//	We better have a number

				else if (!strIsDigit(pPos))
					{
					iHour = 0;
					iMinute = 0;
					iSecond = 0;
					iMillisecond = 0;
					iState = stateDone;
					}

				//	Parse a number

				else
					{
					int iValue = strParseInt(pPos, -1, &pPos);

					//	Add as appropriate

					if (iHour == -1)
						{
						if (iValue < 0 || iValue > 23)
							return false;

						iHour = iValue;
						}

					else if (iMinute == -1)
						{
						if (iValue < 0 || iValue > 59)
							return false;

						iMinute = iValue;
						}

					else
						return false;

					//	If we've got a valid minute, look for (optional) seconds

					if (iMinute != -1)
						iState = stateTimeSeconds;
					}

				break;
				}

			case stateTimeSeconds:
				{
				//	If we're at the end then that's OK--we don't have seconds.

				if (pPos == pPosEnd)
					{
					iSecond = 0;
					iMillisecond = 0;
					iState = stateDone;
					}

				//	If this is PM then we're done and we need to adjust the hour.

				else if (*pPos == 'P' || *pPos == 'p')
					{
					iSecond = 0;
					iMillisecond = 0;
					iState = stateAMPM;
					}

				//	If not a digit, then we're at the end

				else if (!strIsDigit(pPos))
					{
					iSecond = 0;
					iMillisecond = 0;
					iState = stateDone;
					}

				//	Parse a number

				else
					{
					int iValue = strParseInt(pPos, -1, &pPos);
					if (iValue < 0 || iValue > 60)
						return false;

					iSecond = iValue;
					iState = stateTimeMilliseconds;
					}

				break;
				}

			case stateTimeMilliseconds:
				{
				//	If we're at the end then that's OK--we don't have milliseconds.

				if (pPos == pPosEnd)
					{
					iMillisecond = 0;
					iState = stateDone;
					}

				//	If this is PM then we're done and we need to adjust the hour.

				else if (*pPos == 'P' || *pPos == 'p')
					{
					iMillisecond = 0;
					iState = stateAMPM;
					}

				//	If not a digit, then we're at the end

				else if (!strIsDigit(pPos))
					{
					iMillisecond = 0;
					iState = stateDone;
					}

				//	Parse a number

				else
					{
					//	Get the string

					char *pStart = pPos;
					while (pPos < pPosEnd && strIsDigit(pPos))
						pPos++;

					int iDigits = (int)(pPos - pStart);
					CString sValue = CString(pStart, iDigits);

					//	Convert to milliseconds based on the number of digits

					if (iDigits == 0)
						{
						iMillisecond = 0;
						iState = stateDone;
						}
					else if (iDigits == 1)
						{
						iMillisecond = strToInt(sValue, 0) * 100;
						iState = stateDone;
						}
					else if (iDigits == 2)
						{
						iMillisecond = strToInt(sValue, 0) * 10;
						iState = stateDone;
						}
					else if (iDigits == 3)
						{
						iMillisecond = strToInt(sValue, 0);
						iState = stateDone;
						}
					else
						{
						iMillisecond = strToInt(strSubString(sValue, 0, 3), 0);
						iState = stateDone;
						}
					}

				break;
				}

			case stateAMPM:
				{
				if (*pPos == 'P' || *pPos == 'p')
					{
					pPos++;
					if (pPos == pPosEnd 
							|| (*pPos != 'M' && *pPos != 'm')
							|| iHour > 12)
						{
						//	Technically an error (but here we just ignore it).
						}
					else
						iHour += 12;
					}

				iState = stateDone;
				break;
				}
			}
		}

	//	If we have three unknown values, then we assume the first is the month
	//	and the second is the date, and the third is the year

	if (Unknown.GetCount() >= 3)
		{
		iYear = Unknown[2];
		if (iYear > 1900)
			;
		else if (iYear >= 50)
			iYear += 1900;
		else
			iYear += 2000;

		iMonth = Unknown[0];
		iDay = Unknown[1];
		}
	else if (Unknown.GetCount() == 2)
		{
		iMonth = Unknown[0];
		iDay = Unknown[1];
		}
	
	//	If we have a single unknown value, then its either the day or the month,
	//	depending on what we need.

	else if (Unknown.GetCount() == 1)
		{
		if (iDay == -1)
			iDay = Unknown[0];
		else if (iMonth == -1)
			iMonth = Unknown[0];
		else
			return false;
		}

	//	Make sure the date is valid

	if (iYear < 1900
			|| iMonth < 1
			|| iMonth > 12
			|| iDay < 1
			|| iDay > GetDaysInMonth(iMonth, iYear))
		return false;

	//	Compose

	if (retResult)
		*retResult = CDateTime(iDay, iMonth, iYear, iHour, iMinute, iSecond, iMillisecond);

	return true;
	}

bool CDateTime::ParseFixedDigits (char **iopPos, char *pPosEnd, int iCount, int *retiValue, int iDefault)

//	ParseFixedDigits
//
//	Parses the given number of digits and returns the value. Returns FALSE if
//	we don't have the required number.

	{
	//	If we're already at the end, then we return TRUE to support optional
	//	values.

	if (iDefault != -1 && (*iopPos) == pPosEnd)
		{
		if (retiValue)
			*retiValue = iDefault;
		return true;
		}

	//	Parse

	int iValue = 0;
	char *pEnd = (*iopPos) + iCount;
	if (pEnd > pPosEnd)
		return false;

	while ((*iopPos) < pEnd)
		{
		if (**iopPos < '0' || **iopPos > '9')
			return false;

		iValue = (10 * iValue) + ((**iopPos) - '0');
		(*iopPos)++;
		}

	if (retiValue)
		*retiValue = iValue;

	return true;
	}

CDateTime CDateTime::ParseIMF (char *pPos, char *pPosEnd)

//	ParseIMF
//
//	Parse an Internet Message Format string (RFC 2822)

	{
	//	The first token is either a day-of-week or a day

	char *pStart = pPos;
	while (*pPos != ',' && *pPos != ' ' && pPos < pPosEnd)
		pPos++;

	if (pPos == pPosEnd)
		return CDateTime();

	//	If it's a day-of-week, skip it.

	else if (*pPos == ',')
		{
		pPos++;
		while (*pPos == ' ' && pPos < pPosEnd)
			pPos++;

		if (pPos == pPosEnd)
			return CDateTime();

		pStart = pPos;
		while (*pPos != ' ' && pPos < pPosEnd)
			pPos++;

		if (pPos == pPosEnd)
			return CDateTime();
		}

	//	This should be a day.

	int iDay = strParseInt(pStart, -1);
	if (iDay == -1)
		return CDateTime();

	//	Get the month

	pPos++;
	pStart = pPos;
	while (*pPos != ' ' && pPos < pPosEnd)
		pPos++;

	if (pPos == pPosEnd)
		return CDateTime();

	int iMonth;
	if (!ParseMonthIMF(CString(pStart, (int)(pPos - pStart)), &iMonth))
		return CDateTime();

	//	Get the year

	pPos++;
	pStart = pPos;
	while (*pPos != ' ' && pPos < pPosEnd)
		pPos++;

	if (pPos == pPosEnd)
		return CDateTime();

	int iYear = strParseInt(pStart, -1);
	if (iYear == -1)
		return CDateTime();

	//	Make sure the day is correct

	if (iYear <= 1 || GetDaysInMonth(iMonth, iYear) < iDay || iDay < 1)
		return CDateTime();

	//	Get the hour

	pPos++;
	pStart = pPos;
	while (*pPos != ':' && pPos < pPosEnd)
		pPos++;

	if (pPos == pPosEnd)
		return CDateTime();

	int iHour = strParseInt(pStart, -1);
	if (iHour < 0 || iHour > 23)
		return CDateTime();

	//	Get the minute

	pPos++;
	pStart = pPos;
	while (*pPos != ':' && pPos < pPosEnd)
		pPos++;

	if (pPos == pPosEnd)
		return CDateTime();

	int iMinute = strParseInt(pStart, -1);
	if (iMinute < 0 || iMinute > 59)
		return CDateTime();

	//	Get the second

	pPos++;
	pStart = pPos;
	while (*pPos != ' ' && pPos < pPosEnd)
		pPos++;

	if (pPos == pPosEnd)
		return CDateTime();

	int iSecond = strParseInt(pStart, -1);
	if (iSecond < 0 || iSecond > 59)
		return CDateTime();

	//	LATER: Parse timezone

	//	Compose

	return CDateTime(iDay, iMonth, iYear, iHour, iMinute, iSecond, 0);
	}

bool CDateTime::ParseISO8601 (char *pPos, char *pPosEnd, CDateTime *retResult)

//	ParseISO8601
//
//	Parses ISO 8601

	{
	//	Parse the year (exactly 4 digits)

	int iYear;
	if (!ParseFixedDigits(&pPos, pPosEnd, 4, &iYear))
		return false;

	//	Optional '-'

	if (*pPos == '-')
		pPos++;

	//	Parse the month (OK if missing--we default to 1)

	int iMonth;
	if (!ParseFixedDigits(&pPos, pPosEnd, 2, &iMonth, 1)
			|| iMonth < 1 || iMonth > 12)
		return false;

	//	Optional '-'

	if (*pPos == '-')
		pPos++;

	//	Parse the day

	int iDay;
	if (!ParseFixedDigits(&pPos, pPosEnd, 2, &iDay, 1)
			|| iDay < 1 || iDay > GetDaysInMonth(iMonth, iYear))
		return false;

	//	Optional 'T' for time

	if (*pPos == 'T')
		pPos++;

	//	Parse the hour

	int iHour;
	if (!ParseFixedDigits(&pPos, pPosEnd, 2, &iHour, 0)
			|| iHour < 0 || iHour > 23)
		return false;

	//	Optional ':'

	if (*pPos == ':')
		pPos++;

	//	Parse the minute

	int iMinute;
	if (!ParseFixedDigits(&pPos, pPosEnd, 2, &iMinute, 0)
			|| iMinute < 0 || iMinute > 59)
		return false;

	//	Optional ':'

	if (*pPos == ':')
		pPos++;

	//	Parse the second

	int iSecond;
	if (!ParseFixedDigits(&pPos, pPosEnd, 2, &iSecond, 0)
			|| iSecond < 0 || iSecond > 59)
		return false;

	//	Parse optional milliseconds

	int iMilliseconds = 0;
	if (*pPos == '.')
		{
		pPos++;

		//	Parse up to 3 digits

		for (int i = 0; i < 3; i++)
			{
			if (*pPos >= '0' && *pPos <= '9')
				{
				iMilliseconds = (10 * iMilliseconds) + (*pPos - '0');
				pPos++;
				}
			else
				iMilliseconds = (10 * iMilliseconds);
			}
		}

	//	LATER: Deal with timezone. For now we don't worry since we can't 
	//	represent timezones in CDateTime. We let our caller deal with that.

	//	Done

	if (retResult)
		*retResult = CDateTime(iDay, iMonth, iYear, iHour, iMinute, iSecond, iMilliseconds);

	return true;
	}

bool CDateTime::ParseMonthIMF (const CString &sValue, int *retiMonth, bool bFullNameOK)

//	ParseMonthIMF
//
//	Parses a month

	{
	int iMonth;

	if (!bFullNameOK && sValue.GetLength() != 3)
		return false;

	CString sLower = strToLower(sValue);
	char *pPos = sLower.GetParsePointer();
	if (*pPos == 'j')
		{
		if (pPos[1] == 'a')
			{
			if (pPos[2] == 'n')
				iMonth = 1;
			else
				return false;
			}
		else if (pPos[1] == 'u')
			{
			if (pPos[2] == 'n')
				iMonth = 6;
			else if (pPos[2] == 'l')
				iMonth = 7;
			else
				return false;
			}
		else
			return false;
		}
	else if (*pPos == 'f')
		{
		if (pPos[1] == 'e' && pPos[2] == 'b')
			iMonth = 2;
		else
			return false;
		}
	else if (*pPos == 'm')
		{
		if (pPos[1] == 'a')
			{
			if (pPos[2] == 'r')
				iMonth = 3;
			else if (pPos[2] == 'y')
				iMonth = 5;
			else
				return false;
			}
		else
			return false;
		}
	else if (*pPos == 'a')
		{
		if (pPos[1] == 'p' && pPos[2] == 'r')
			iMonth = 4;
		else if (pPos[1] == 'u' && pPos[2] == 'g')
			iMonth = 8;
		else
			return false;
		}
	else if (*pPos == 's')
		{
		if (pPos[1] == 'e' && pPos[2] == 'p')
			iMonth = 9;
		else
			return false;
		}
	else if (*pPos == 'o')
		{
		if (pPos[1] == 'c' && pPos[2] == 't')
			iMonth = 10;
		else
			return false;
		}
	else if (*pPos == 'n')
		{
		if (pPos[1] == 'o' && pPos[2] == 'v')
			iMonth = 11;
		else
			return false;
		}
	else if (*pPos == 'd')
		{
		if (pPos[1] == 'e' && pPos[2] == 'c')
			iMonth = 12;
		else
			return false;
		}
	else
		return false;

	if (retiMonth)
		*retiMonth = iMonth;

	return true;
	}

void CDateTime::SetDate (int iDay, int iMonth, int iYear)

//	SetDate
//
//	Sets the date
//
//	iDay is the date (1st = 1)
//	iMonth is the month (Jan = 1)
//	iYear is the year (e.g., 2003)

	{
	m_Time.wDay = (short)iDay;
	m_Time.wDayOfWeek = 0xffff;
	m_Time.wMonth = (short)iMonth;
	m_Time.wYear = (short)iYear;
	}

void CDateTime::SetTime (int iHour, int iMinute, int iSecond, int iMillisecond)

//	SetTime
//
//	Sets the time

	{
	m_Time.wHour = (short)iHour;
	m_Time.wMinute = (short)iMinute;
	m_Time.wSecond = (short)iSecond;
	m_Time.wMilliseconds = (short)iMillisecond;
	}

//	CTimeSpan -----------------------------------------------------------------

CTimeSpan::CTimeSpan (void) : m_Days(0), m_Milliseconds(0)

//	CTimeSpan constructor

	{
	}

CTimeSpan::CTimeSpan (int iMilliseconds)

//	CTimeSpan constructor

	{
	m_Days = iMilliseconds / (SECONDS_PER_DAY * 1000);
	m_Milliseconds = iMilliseconds % (SECONDS_PER_DAY * 1000);
	}

CTimeSpan::CTimeSpan (DWORDLONG dwMilliseconds)

//	CTimeSpan constructor

	{
	m_Days = (DWORD)(dwMilliseconds / (DWORDLONG)(SECONDS_PER_DAY * 1000));
	m_Milliseconds = dwMilliseconds % (DWORDLONG)(SECONDS_PER_DAY * 1000);
	}

CTimeSpan::CTimeSpan (int iDays, int iMilliseconds) : m_Days(iDays), m_Milliseconds(iMilliseconds)

//	CTimeSpan constructor

	{
	}

CString FormatTwoUnits (const CString &sMajor, const CString &sMinor, int iMinor)
	{
	if (iMinor == 0)
		return sMajor;
	else
		return strPattern("%s and %s", sMajor, sMinor);
	}

CString CTimeSpan::Format (const CString &sFormat) const

//	Format
//
//	Formats the time span
//
//	-			1 hour and 5 minutes
//	hh:mm:ss	04:01:10.4

	{
	int iYears = m_Days / 365;
	int iDays = m_Days % 365;
	int iHours = m_Milliseconds / (60 * 60 * 1000);
	int iMinutes = (m_Milliseconds % (60 * 60 * 1000)) / (60 * 1000);
	int iSeconds = (m_Milliseconds % (60 * 1000)) / 1000;
	int iMilliseconds = (m_Milliseconds % 1000);

	CString sYears;
	if (iYears == 1)
		sYears = CString("1 year");
	else if (iYears > 1)
		sYears = strPattern("%d years", iYears);

	CString sDays;
	if (iDays == 1)
		sDays = CString("1 day");
	else if (iDays > 1)
		sDays = strPattern("%d days", iDays);

	CString sHours;
	if (iHours == 1)
		sHours = CString("1 hour");
	else if (iHours > 1)
		sHours = strPattern("%d hours", iHours);

	CString sMinutes;
	if (iMinutes == 1)
		sMinutes = CString("1 minute");
	else if (iMinutes > 1)
		sMinutes = strPattern("%d minutes", iMinutes);

	CString sSeconds;
	if (iMilliseconds / 10 > 0)
		sSeconds = strPattern("%d.%02d seconds", iSeconds, iMilliseconds / 10);
	else if (iSeconds == 1)
		sSeconds = CString("1 second");
	else
		sSeconds = strPattern("%d seconds", iSeconds);

	//	Format

	CString sResult;
	if (iYears > 0)
		return FormatTwoUnits(sYears, sDays, iDays);
	else if (iDays > 0)
		return FormatTwoUnits(sDays, sHours, iHours);
	else if (iHours > 0)
		return FormatTwoUnits(sHours, sMinutes, iMinutes);
	else if (iMinutes > 0)
		return FormatTwoUnits(sMinutes, sSeconds, iSeconds + iMilliseconds / 10);
	else
		return sSeconds;
	}

//	Functions -----------------------------------------------------------------

CDateTime timeAddTime (const CDateTime &StartTime, const CTimeSpan &Addition)

//	timeAddTime
//
//	Adds a timespan to a datetime

	{
	int iDaysSince1AD = StartTime.DaysSince1AD();
	int iMillisecondsSinceMidnight = StartTime.MillisecondsSinceMidnight();

	//	Add

	iDaysSince1AD += Addition.Days();
	iMillisecondsSinceMidnight += Addition.MillisecondsSinceMidnight();
	if (iMillisecondsSinceMidnight >= SECONDS_PER_DAY * 1000)
		{
		iDaysSince1AD++;
		iMillisecondsSinceMidnight -= SECONDS_PER_DAY * 1000;
		}

	return CDateTime(iDaysSince1AD, iMillisecondsSinceMidnight);
	}

CString timeGetMonthName (int iMonth)
	{
	if (iMonth < 1 || iMonth > 12)
		return NULL_STR;

	return CString(g_szMonthName[iMonth - 1]);
	}

bool timeIsLeapYear (int iYear)

//	timeIsLeapYear
//
//	Returns TRUE if given year is a leap year

	{
	return ((iYear % 4) == 0)
			&& (((iYear % 100) != 0) || ((iYear % 400) == 0));
	}

CTimeSpan timeSpan (const CDateTime &StartTime, const CDateTime &EndTime)

//	timeSpan
//
//	Returns the difference between the two times

	{
	int iDays = EndTime.DaysSince1AD() - StartTime.DaysSince1AD();
	if (iDays < 0)
		return timeSpan(EndTime, StartTime);

	int iStartTime = StartTime.MillisecondsSinceMidnight();
	int iEndTime = EndTime.MillisecondsSinceMidnight();

	int iMilliseconds = 0;
	if (iEndTime > iStartTime)
		{
		iMilliseconds = iEndTime - iStartTime;
		}
	else
		{
		if (iDays > 0)
			{
			iDays--;
			iMilliseconds = (iEndTime + SECONDS_PER_DAY * 1000) - iStartTime;
			}
		else
			iMilliseconds = iStartTime - iEndTime;
		}

	return CTimeSpan(iDays, iMilliseconds);
	}

CDateTime timeSubtractTime (const CDateTime &StartTime, const CTimeSpan &Subtraction)

//	timeSubtractTime
//
//	Subtracts time from datetime

	{
	int iDaysSince1AD = StartTime.DaysSince1AD();
	int iMillisecondsSinceMidnight = StartTime.MillisecondsSinceMidnight();

	//	Add

	iDaysSince1AD -= Subtraction.Days();
	if (Subtraction.MillisecondsSinceMidnight() > iMillisecondsSinceMidnight)
		{
		iMillisecondsSinceMidnight += SECONDS_PER_DAY * 1000;
		iDaysSince1AD--;
		}
	
	iMillisecondsSinceMidnight -= Subtraction.MillisecondsSinceMidnight();

	return CDateTime(iDaysSince1AD, iMillisecondsSinceMidnight);
	}
