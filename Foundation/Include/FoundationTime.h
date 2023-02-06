//	FoundationTime.h
//
//	Foundation header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

constexpr int SECONDS_PER_DAY =					(60 * 60 * 24);
constexpr int SECONDS_PER_HOUR =				(60 * 60);
constexpr int SECONDS_PER_MINUTE =				60;

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
			formatTime,					//	4:15:00 PM or 16:15:00
			};

		enum DateFormats
			{
			dfNone,
			dfShort,					//	Same as OS short date format
			dfLong,						//	Same as OS long date format
			dfRelative,					//	DateTime relative to now
			};

		enum TimeFormats
			{
			tfNone,
			tfShort,					//	4:15 PM
			tfShort24,					//	16:15
			tfLong,						//	4:15:30 PM
			tfLong24,					//	16:15:30
			};

		static const DWORDLONG SECONDS_PER_MINUTE = 60;
		static const DWORDLONG SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE;
		static const DWORDLONG SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR;

		CDateTime (void);
		CDateTime (Constants Init);
		CDateTime (int iDaysSince1AD, int iMillisecondsSinceMidnight);
		CDateTime (int iDay, int iMonth, int iYear);
		CDateTime (int iDay, int iMonth, int iYear, int iHour, int iMinute, int iSecond, int iMillisecond = 0);
		CDateTime (SYSTEMTIME &SystemTime) { m_Time = SystemTime; }
		CDateTime (StandardFormats iFormat, const CString &sValue);

		static CDateTime FromDOSDateTime (DWORD dwDateTime);
		static CDateTime FromTick (DWORDLONG dwTick);
		static bool Parse (StandardFormats iFormat, char *pPos, char *pPosEnd, CDateTime *retResult);
		static bool Parse (StandardFormats iFormat, const char *pPos, const char *pPosEnd, CDateTime &retResult);
		static bool Parse (StandardFormats iFormat, const CString &sValue, CDateTime *retResult);
		static bool Parse (StandardFormats iFormat, const CString &sValue, CDateTime &retResult);
		static CDateTime ParseIMF (const CString &sValue) { return ParseIMF(sValue.GetParsePointer(), sValue.GetParsePointer() + sValue.GetLength()); }

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

		int Year (void) const { return m_Time.wYear; }
		int Month (void) const { return m_Time.wMonth; }
		int Day (void) const { return m_Time.wDay; }
		int Hour (void) const { return m_Time.wHour; }
		int Minute (void) const { return m_Time.wMinute; }
		int Second (void) const { return m_Time.wSecond; }
		int Millisecond (void) const { return m_Time.wMilliseconds; }

		void SetDate (int iDay, int iMonth, int iYear);
		void SetMillisecond (int iMillisecond) { m_Time.wMilliseconds = iMillisecond; }
		void SetTime (int iHour, int iMinute, int iSecond, int iMillisecond = 0);

		DWORDLONG AsTick () const;
		SYSTEMTIME AsSYSTEMTIME () const { return m_Time; }
		int Age (const CDateTime &Today = CDateTime(Today), int *retiMonths = NULL, int *retiDays = NULL) const;
		CDateTime AsLocalTime (void) const;
		CDateTime AsUTC (void) const;
		int Compare (const CDateTime &Src) const;
		int DayOfWeek (void) const;
		int DaysSince1AD (void) const;
		CString Format (const CString &sFormat) const;
		CString Format (DateFormats iDateFormat, TimeFormats iTimeFormat) const;
		CString Format (StandardFormats iFormat) const;
		CString FormatIMF (void) const;
		bool HasDate (void) const { return m_Time.wDay != 1 || m_Time.wMonth != 1 || m_Time.wYear != 1; }
		bool HasTime (void) const { return m_Time.wHour || m_Time.wMinute || m_Time.wSecond || m_Time.wMilliseconds; }
		bool IsValid (void) const { return HasDate() || HasTime(); }
		int MillisecondsSinceMidnight (void) const;

		static int GetDaysInMonth (int iMonth, int iYear = 0);
		static bool IsValidDate (int iDay, int iMonth, int iYear);
		static bool IsValidTime (int iHour, int iMinute, int iSecond, int iMillisecond = 0);

	private:
		static bool ParseAuto (const char *pPos, const char *pPosEnd, StandardFormats iFormat, CDateTime *retResult);
		static bool ParseDigits (const char *&ioPos, const char *pPosEnd, int *retiValue, int iDefault = -1, int *retiDigitsParsed = NULL);
		static bool ParseFixedDigits (char **iopPos, char *pPosEnd, int iCount, int *retiValue, int iDefault = -1);
		static CDateTime ParseIMF (char *pPos, char *pPosEnd);
		static bool ParseISO8601 (char *pPos, char *pPosEnd, CDateTime *retResult);
		static bool ParseMonthIMF (const CString &sValue, int *retiMonth = NULL, bool bFullNameOK = false);
		static bool ParseTime (const char *pPos, const char *pPosEnd, CDateTime &retResult);
		static CString ParseToken (const char *&pPos);

		SYSTEMTIME m_Time;
	};

inline int KeyCompare (const CDateTime &Key1, const CDateTime &Key2) { return Key1.Compare(Key2); }

class CTimeSpan
	{
	public:
		CTimeSpan (void);
		CTimeSpan (int iMilliseconds);
		CTimeSpan (DWORDLONG dwMilliseconds, bool bNegative = false);
		CTimeSpan (int iDays, int iMilliseconds, bool bNegative = false);
		CTimeSpan (DWORD dwDays, DWORD dwMilliseconds, bool bNegative = false);
		CTimeSpan (const CTimeSpan &Src, bool bNegative);

		bool operator== (const CTimeSpan &Other) const 
			{ return (m_Days == Other.m_Days) && (m_Milliseconds == Other.m_Milliseconds) && (m_bNegative == Other.m_bNegative); }

		bool operator!= (const CTimeSpan &Other) const 
			{ return (m_Days != Other.m_Days) || (m_Milliseconds != Other.m_Milliseconds) || (m_bNegative == Other.m_bNegative); }

		bool operator> (const CTimeSpan &Other) const { return (Compare(Other) == 1); }
		bool operator< (const CTimeSpan &Other) const { return (Compare(Other) == -1); }
		bool operator>= (const CTimeSpan &Other) const { return (Compare(Other) != -1); }
		bool operator<= (const CTimeSpan &Other) const { return (Compare(Other) != 1); }

		int Compare (const CTimeSpan &Src) const;
		int Days (void) const { return (int)m_Days; }
		bool IsEmpty (void) const { return (m_Days == 0 && m_Milliseconds == 0); }
		bool IsNegative (void) const { return m_bNegative; }
		int Seconds (void) const { return (SECONDS_PER_DAY * m_Days) + (m_Milliseconds / 1000); }
		DWORDLONG Seconds64 (bool *retbNegative = NULL) const { if (retbNegative) *retbNegative = m_bNegative; return (SECONDS_PER_DAY * (DWORDLONG)m_Days) + (DWORDLONG)(m_Milliseconds / 1000); }
		int Milliseconds (void) const { return (SECONDS_PER_DAY * 1000 * m_Days) + m_Milliseconds; }
		DWORDLONG Milliseconds64 (void) const { return ((DWORDLONG)SECONDS_PER_DAY * (DWORDLONG)m_Days * 1000) + (DWORDLONG)m_Milliseconds; }
		int MillisecondsSinceMidnight (void) const { return (int)m_Milliseconds; }
		static const CTimeSpan &Null () { return m_Null; }

		CString Format (const CString &sFormat) const;

		static CTimeSpan Add (const CTimeSpan &A, const CTimeSpan &B);
		static CTimeSpan Subtract (const CTimeSpan &A, const CTimeSpan &B);

	private:
		DWORD m_Days = 0;
		DWORD m_Milliseconds = 0;
		bool m_bNegative = false;

		static const CTimeSpan m_Null;
	};

inline int KeyCompare (const CTimeSpan &Key1, const CTimeSpan &Key2) { return Key1.Compare(Key2); }

CDateTime timeAddTime (const CDateTime &StartTime, const CTimeSpan &Addition);
CString timeGetMonthName (int iMonth);
CTimeSpan timeSpan (const CDateTime &StartTime, const CDateTime &EndTime);
CDateTime timeSubtractTime (const CDateTime &StartTime, const CTimeSpan &Subtraction);
bool timeIsLeapYear (int iYear);

extern CDateTime NULL_DATETIME;