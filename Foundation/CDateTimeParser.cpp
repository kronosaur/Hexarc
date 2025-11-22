//	CDateTimeParser.cpp
//
//	CDateTimeParser class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_UN,							"un");
DECLARE_CONST_STRING(STR_UNK,							"unk");
DECLARE_CONST_STRING(STR_UNKNOWN,						"unknown");
DECLARE_CONST_STRING(STR_00,							"00");
DECLARE_CONST_STRING(STR_000,							"000");

bool CDateTimeParser::Parse (const char *pPos, const char *pPosEnd, const SOptions& Options, CDateTime* retResult)

//	Parse
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
				const char *pStart = pPos;

				//	If we're at the end, then we don't have enough.

				if (pPos == pPosEnd)
					return false;

				//	If this is a letter or 00, then we try to parse it.

				if (strIsASCIIAlpha(pPos) || (*pPos == '0' && pPos[1] == '0'))
					{
					while (strIsASCIIAlpha(pPos) || *pPos == '0')
						pPos++;

					CString sValue = CString(pStart, (int)(pPos - pStart));

					//	If this is the token "un" or "unk" then it means an unknown
					//	component.

					if (strEqualsNoCase(sValue, STR_UN) 
							|| strEqualsNoCase(sValue, STR_UNK)
							|| strEqualsNoCase(sValue, STR_00)
							|| strEqualsNoCase(sValue, STR_000))
						{
						//	If we've already got an unknown date, then this is an unknown
						//	month.
						//
						//	un unk 2016

						if (bUnknownDate)
							{
							if (Options.MonthMissing.iType == EImpute::None)
								return false;

							bUnknownMonth = true;

							//	We interpolate based on whether we prefer start or end.

							iMonth = -1;
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
							if (Options.DayMissing.iType == EImpute::None)
								return false;

							bUnknownDate = true;
							}
						}

					//	Otherwise, if this is a month name, use that

					else if (CDateTime::ParseMonthIMF(sValue, &iMonth, true))
						{
						iDateParts++;
						bUnknownMonth = false;
						}
					else
						{
						//	Skip this token--this could be a day of the week.
						}
					}

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

					else if (iYear != -1 && iMonth == -1 && iValue >= 1 && iValue <= 12)
						{
						iMonth = iValue;
						iDateParts++;
						}

					//	If we already have the day and month, then this is the
					//	year.

					else if (iYear == -1 && iDateParts == 2 && iValue < 100)
						{
						iYear = CalcYear(iValue, Options);
						iDateParts++;
						}

					//	If we already have a month, then this is the day

					else if (iMonth != -1 && iValue >= 1 && iValue <= 31)
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

				//	If we already have enough date parts, then continue to the
				//	time.

				if (iDateParts >= 3)
					iState = stateTime;

				//	If we've got month and year but an unknown date, then that's enough

				else if (iYear != -1 && iMonth != -1 && bUnknownDate)
					{
					switch (Options.DayMissing.iType)
						{
						case EImpute::First:
							iDay = 1;
							break;

						case EImpute::Last:
							iDay = CDateTime::GetDaysInMonth(iMonth, iYear);
							break;

						case EImpute::Default:
							{
							iDay = Options.DayMissing.Default.Day();
							iMonth = Options.DayMissing.Default.Month();
							iYear = Options.DayMissing.Default.Year();
							break;
							}

						case EImpute::Null:
							{
							if (retResult) *retResult = CDateTime();
							return true;
							}

						default:
							return false;
						}

					iState = stateTime;
					}

				//	If we've got a year and an unknown date and month, then we have enough

				else if (iYear != -1 && bUnknownDate)
					{
					switch (Options.MonthMissing.iType)
						{
						case EImpute::First:
							iMonth = 1;
							iDay = 1;
							break;

						case EImpute::Last:
							iMonth = 12;
							iDay = 31;
							break;

						case EImpute::Default:
							{
							iDay = Options.MonthMissing.Default.Day();
							iMonth = Options.MonthMissing.Default.Month();
							iYear = Options.MonthMissing.Default.Year();
							break;
							}

						case EImpute::Null:
							{
							if (retResult) *retResult = CDateTime();
							return true;
							}

						default:
							return false;
						}

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

				//	If this is PM then we're done and we need to adjust the hour.

				else if (*pPos == 'P' || *pPos == 'p' || *pPos == 'A' || *pPos == 'a')
					{
					if (iHour == -1)
						return false;

					if (iMinute == -1)
						iMinute = 0;

					iSecond = 0;
					iMillisecond = 0;
					iState = stateAMPM;
					}

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

				else if (*pPos == 'P' || *pPos == 'p' || *pPos == 'A' || *pPos == 'a')
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

				else if (*pPos == 'P' || *pPos == 'p' || *pPos == 'A' || *pPos == 'a')
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

					const char *pStart = pPos;
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
						{
						//	12 PM is special.

						if (iHour != 12)
							iHour += 12;
						}
					}
				else if (*pPos == 'A' || *pPos == 'a')
					{
					//	12 AM is special.

					if (iHour == 12)
						iHour = 0;
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
		switch (Options.iDateOrder)
			{
			case EDateOrder::DMY:
				iDay = Unknown[0];
				iMonth = Unknown[1];
				iYear = CalcYear(Unknown[2], Options);
				break;

			case EDateOrder::MDY:
				iDay = Unknown[1];
				iMonth = Unknown[0];
				iYear = CalcYear(Unknown[2], Options);
				break;

			case EDateOrder::YMD:
				iYear = CalcYear(Unknown[0], Options);
				iMonth = Unknown[1];
				iDay = Unknown[2];
				break;

			default:
				//	US order
				iYear = CalcYear(Unknown[2], Options);
				iMonth = Unknown[0];
				iDay = Unknown[1];
				break;
			}

		}
	else if (Unknown.GetCount() == 2)
		{
		switch (Options.iDateOrder)
			{
			case EDateOrder::DMY:
				iDay = Unknown[0];
				iMonth = Unknown[1];
				break;

			case EDateOrder::MDY:
			case EDateOrder::YMD:
				iDay = Unknown[1];
				iMonth = Unknown[0];
				break;

			default:
				//	US order
				iMonth = Unknown[0];
				iDay = Unknown[1];
				break;
			}
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
			|| iDay > CDateTime::GetDaysInMonth(iMonth, iYear))
		return false;

	//	Compose

	if (retResult)
		*retResult = CDateTime(iDay, iMonth, iYear, iHour, iMinute, iSecond, iMillisecond);

	return true;
	}

int CDateTimeParser::CalcYear (int iValue, const SOptions& Options)

//	CalcYear
//
//	Always returns a 4 digit year and converts 2-digit years to 4-digit years.

	{
	if (iValue >= 100)
		return iValue;

	else if (Options.iCentury)
		return Options.iCentury + iValue;

	else
		{
		//	NOTE: If iBaseYear is ever anything other than 2000, then increment
		//	the following counter by 1 for every century after the 21st.
		//
		//	int georges_legacy_counter = 0;

		int iYear = CDateTime::GetCurrentYear();
		int iYear2D = iYear % 100;
		int iBaseYear = iYear - iYear2D;

		if (iYear2D > iValue)
			{
			if (iYear2D - iValue > 50)
				return iBaseYear - 100 + iValue;
			else
				return iBaseYear + iValue;
			}
		else
			{
			if (iValue - iYear2D > 50)
				return iBaseYear + 100 + iValue;
			else
				return iBaseYear + iValue;
			}
		}
	}

bool CDateTimeParser::Parse (double rValue, const SOptions& Options, CDateTime* retResult)

//	Parse
//
//	Parses from a numeric value.

	{
	switch (Options.iNumericSystem)
		{
		case ENumericSystem::None:
			return false;

		case ENumericSystem::Excel1900:
			if (retResult)
				*retResult = FromExcel(rValue);
			return true;

		case ENumericSystem::Unix:
			{
			LONGLONG lValue = (LONGLONG)rValue;
			if (lValue > MAXINT)
				*retResult = timeAddTime(CDateTime(1, 1, 1970), CTimeSpan((DWORDLONG)lValue));
			else if (lValue >= 0)
				*retResult = timeAddTime(CDateTime(1, 1, 1970), CTimeSpan((DWORDLONG)lValue * 1000));
			else if (lValue >= MININT)
				*retResult = timeSubtractTime(CDateTime(1, 1, 1970), CTimeSpan(((DWORDLONG)-lValue) * 1000));
			else
				*retResult = timeSubtractTime(CDateTime(1, 1, 1970), CTimeSpan((DWORDLONG)-lValue));
			return true;
			}

		case ENumericSystem::UnixMS:
			{
			LONGLONG lValue = (LONGLONG)rValue;
			if (lValue >= 0)
				*retResult = timeAddTime(CDateTime(1, 1, 1970), CTimeSpan((DWORDLONG)lValue));
			else
				*retResult = timeSubtractTime(CDateTime(1, 1, 1970), CTimeSpan((DWORDLONG)-lValue));
			return true;
			}

		case ENumericSystem::UnixSeconds:
			{
			LONGLONG lValue = (LONGLONG)rValue;
			if (lValue >= 0)
				*retResult = timeAddTime(CDateTime(1, 1, 1970), CTimeSpan((DWORDLONG)lValue * 1000));
			else
				*retResult = timeSubtractTime(CDateTime(1, 1, 1970), CTimeSpan(((DWORDLONG)-lValue) * 1000));
			return true;
			}

		default:
			throw CException(errFail);
		}
	}

CDateTime CDateTimeParser::FromExcel (double rValue)

//	FromExcel
//
//	Parses an Excel date.

	{
	CDateTime Start(1, 1, 1900);

	if (rValue >= 1.0)
		{
		int iDays = (int)floor(rValue) - 1;
		int iMilliseconds = (int)mathRound((rValue - floor(rValue)) * SECONDS_PER_DAY * 1000.0);

		//	Adjust for the Excel leap year bug: If the date is >= 60, add 1 day to skip Feb 29, 1900
		if (iDays >= 59)
			iDays--;

		return timeAddTime(Start, CTimeSpan(iDays, iMilliseconds));
		}
	else
		{
		double rDiff = 1.0 - rValue;
		int iDays = (int)floor(rDiff);
		int iMilliseconds = (int)mathRound((rDiff - floor(rDiff)) * SECONDS_PER_DAY * 1000.0);

		return timeSubtractTime(Start, CTimeSpan(iDays, iMilliseconds));
		}
	}

LONGLONG CDateTimeParser::ToUnixTimeMS (const CDateTime& DateTime)
	{
	CTimeSpan Time = timeSpan(CDateTime(1, 1, 1970), DateTime);
	if (Time.IsNegative())
		return -(LONGLONG)Time.Milliseconds64();
	else
		return (LONGLONG)Time.Milliseconds64();
	}
