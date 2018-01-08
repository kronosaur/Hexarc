//	FoundationTime.h
//
//	Foundation header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

const int SECONDS_PER_DAY =					(60 * 60 * 24);

class CDateTime
	{
	public:
		enum Constants
			{
			Now,						//	Now (GMT)
			Today,
			BeginningOfTime,

			LocalNow,					//	Now in local time
			};

		enum StandardFormats
			{
			formatAuto,					//	Guess format
			formatInterpolateStart,		//	Guess format, allow unknown date/month (and assume start of range)
			formatInterpolateEnd,		//	Guess format, allow unknown date/month (and assume end of range)

			formatIMF,					//	Sun, 06 Nov 1994 08:49:37 GMT
			formatISO8601,				//	1994-11-06T08:49:37.123Z
			};

		enum DateFormats
			{
			dfNone,
			dfShort,					//	Same as OS short date format
			dfLong,						//	Same as OS long date format
			};

		enum TimeFormats
			{
			tfNone,
			tfShort,					//	4:15 PM
			tfShort24,					//	16:15
			tfLong,						//	4:15:30 PM
			tfLong24,					//	16:15:30
			};

		CDateTime (void);
		CDateTime (Constants Init);
		CDateTime (int iDaysSince1AD, int iMillisecondsSinceMidnight);
		CDateTime (int iDay, int iMonth, int iYear);
		CDateTime (int iDay, int iMonth, int iYear, int iHour, int iMinute, int iSecond, int iMillisecond = 0);
		CDateTime (SYSTEMTIME &SystemTime) { m_Time = SystemTime; }

		static bool Parse (StandardFormats iFormat, char *pPos, char *pPosEnd, CDateTime *retResult);
		static bool Parse (StandardFormats iFormat, const CString &sValue, CDateTime *retResult);
		inline static CDateTime ParseIMF (const CString &sValue) { return ParseIMF(sValue.GetParsePointer(), sValue.GetParsePointer() + sValue.GetLength()); }

		bool operator== (const CDateTime &Other) const 
			{ return (m_Time.wYear == Other.m_Time.wYear)
					&& (m_Time.wMonth == Other.m_Time.wMonth)
					&& (m_Time.wDay == Other.m_Time.wDay)
					&& (m_Time.wHour == Other.m_Time.wHour)
					&& (m_Time.wMinute == Other.m_Time.wMinute)
					&& (m_Time.wSecond == Other.m_Time.wSecond)
					&& (m_Time.wMilliseconds == Other.m_Time.wMilliseconds); }

		bool operator!= (const CDateTime &Other) const 
			{ return (m_Time.wYear != Other.m_Time.wYear)
					|| (m_Time.wMonth != Other.m_Time.wMonth)
					|| (m_Time.wDay != Other.m_Time.wDay)
					|| (m_Time.wHour != Other.m_Time.wHour)
					|| (m_Time.wMinute != Other.m_Time.wMinute)
					|| (m_Time.wSecond != Other.m_Time.wSecond)
					|| (m_Time.wMilliseconds != Other.m_Time.wMilliseconds); }

		bool operator> (const CDateTime &Other) const { return (Compare(Other) == 1); }
		bool operator< (const CDateTime &Other) const { return (Compare(Other) == -1); }
		bool operator>= (const CDateTime &Other) const { return (Compare(Other) != -1); }
		bool operator<= (const CDateTime &Other) const { return (Compare(Other) != 1); }

		inline int Year (void) const { return m_Time.wYear; }
		inline int Month (void) const { return m_Time.wMonth; }
		inline int Day (void) const { return m_Time.wDay; }
		inline int Hour (void) const { return m_Time.wHour; }
		inline int Minute (void) const { return m_Time.wMinute; }
		inline int Second (void) const { return m_Time.wSecond; }
		inline int Millisecond (void) const { return m_Time.wMilliseconds; }

		void SetDate (int iDay, int iMonth, int iYear);
		inline void SetMillisecond (int iMillisecond) { m_Time.wMilliseconds = iMillisecond; }
		void SetTime (int iHour, int iMinute, int iSecond, int iMillisecond = 0);

		CDateTime AsLocalTime (void) const;
		int Compare (const CDateTime &Src) const;
		int DayOfWeek (void) const;
		int DaysSince1AD (void) const;
		CString Format (const CString &sFormat) const;
		CString Format (DateFormats iDateFormat, TimeFormats iTimeFormat) const;
		CString FormatIMF (void) const;
		bool IsValid (void) const;
		int MillisecondsSinceMidnight (void) const;

		static int GetDaysInMonth (int iMonth, int iYear = 0);

	private:
		static bool ParseAuto (char *pPos, char *pPosEnd, StandardFormats iFormat, CDateTime *retResult);
		static bool ParseFixedDigits (char **iopPos, char *pPosEnd, int iCount, int *retiValue, int iDefault = -1);
		static CDateTime ParseIMF (char *pPos, char *pPosEnd);
		static bool ParseISO8601 (char *pPos, char *pPosEnd, CDateTime *retResult);
		static bool ParseMonthIMF (const CString &sValue, int *retiMonth = NULL, bool bFullNameOK = false);

		SYSTEMTIME m_Time;
	};

inline int KeyCompare (const CDateTime &Key1, const CDateTime &Key2) { return Key1.Compare(Key2); }

class CTimeSpan
	{
	public:
		CTimeSpan (void);
		CTimeSpan (int iMilliseconds);
		CTimeSpan (DWORDLONG dwMilliseconds);
		CTimeSpan (int iDays, int iMilliseconds);

		inline int Days (void) const { return (int)m_Days; }
		inline int Seconds (void) const { return (SECONDS_PER_DAY * m_Days) + (m_Milliseconds / 1000); }
		inline DWORDLONG Seconds64 (void) const { return (SECONDS_PER_DAY * (DWORDLONG)m_Days) + (DWORDLONG)(m_Milliseconds / 1000); }
		inline int Milliseconds (void) const { return (SECONDS_PER_DAY * 1000 * m_Days) + m_Milliseconds; }
		inline int MillisecondsSinceMidnight (void) const { return (int)m_Milliseconds; }

		CString Format (const CString &sFormat) const;

	private:
		DWORD m_Days;
		DWORD m_Milliseconds;
	};

CDateTime timeAddTime (const CDateTime &StartTime, const CTimeSpan &Addition);
CString timeGetMonthName (int iMonth);
CTimeSpan timeSpan (const CDateTime &StartTime, const CDateTime &EndTime);
CDateTime timeSubtractTime (const CDateTime &StartTime, const CTimeSpan &Subtraction);
bool timeIsLeapYear (int iYear);

extern CDateTime NULL_DATETIME;