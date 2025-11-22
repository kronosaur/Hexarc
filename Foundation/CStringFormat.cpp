//	CStringFormat.cpp
//
//	CStringFormat class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FORMAT_DEFAULT_LONG_DATE,			"d mmmm yyyy");
DECLARE_CONST_STRING(FORMAT_BASE64,						"base64");
DECLARE_CONST_STRING(FORMAT_BASE64_URL,					"base64Url");
DECLARE_CONST_STRING(FORMAT_HEX,						"hex");
DECLARE_CONST_STRING(FORMAT_R,							"R");
DECLARE_CONST_STRING(FORMAT_RR,							"RR");
DECLARE_CONST_STRING(FORMAT_UTF_8,						"utf-8");

DECLARE_CONST_STRING(STR_BAR,							"|");
DECLARE_CONST_STRING(STR_ERROR,							"error");
DECLARE_CONST_STRING(STR_NAN,							"nan");
DECLARE_CONST_STRING(STR_PERCENT,						"%");
DECLARE_CONST_STRING(STR_ONE_YEAR,						"1 year");
DECLARE_CONST_STRING(STR_ONE_MONTH,						"1 month");
DECLARE_CONST_STRING(STR_ONE_DAY,						"1 day");
DECLARE_CONST_STRING(STR_ONE_HOUR,						"1 hour");
DECLARE_CONST_STRING(STR_ONE_MINUTE,					"1 minute");
DECLARE_CONST_STRING(STR_ONE_SECOND,					"1 second");
DECLARE_CONST_STRING(STR_JUST_NOW,						"just now");

DECLARE_CONST_STRING(ERR_UNKNOWN_FORMAT,				"Unknown format: %s.");

const CStringFormat CStringFormat::Null;

bool CStringFormat::FormatBinary (const BYTE* pData, int iLength, CString& retsResult) const

//	FormatBinary
//
//	Formats binary data as a string.

	{
	if (strEqualsNoCase(m_sFormat, FORMAT_BASE64))
		{
		retsResult = CString::EncodeBase64(pData, iLength);
		return true;
		}

	else if (strEqualsNoCase(m_sFormat, FORMAT_BASE64_URL))
		{
		retsResult = CString::EncodeBase64(pData, iLength, CBase64Encoder::FLAG_BASE64_URL);
		return true;
		}

	else if (strEqualsNoCase(m_sFormat, FORMAT_HEX))
		{
		retsResult = CString::EncodeHex(pData, iLength);
		return true;
		}

	else if (strEqualsNoCase(m_sFormat, FORMAT_UTF_8))
		{
		CStringBuffer Result;
		Result.Write((LPCSTR)pData, iLength);
		retsResult = CString(std::move(Result));
		return true;
		}

	else
		{
		retsResult = strPattern(ERR_UNKNOWN_FORMAT, m_sFormat);
		return false;
		}
	}

CString CStringFormat::FormatDateTime (const CDateTime& Value) const

//	FormatDateTime
//
//	Formats a datetime using the following fields:
//
//	d: Day of the month as digits; no leading zero for single-digit days.
//	dd: Day of the month as digits; leading zero for single-digit days.
//	ddd: Day of the week as three letters.
//	dddd: Full name of the day of the week.
//	m: Month as digits; no leading zero for single-digit months.
//	mm: Month as digits; leading zero for single-digit months.
//	mmm: Month as three letters.
//	mmmm: Full name of the month.
//	yy: Year as last two digits; leading zero for years less than 10.
//	yyyy: Year represented by a four-digit number.
//	h: Hours; no leading zero for single-digit hours (12-hour clock).
//	hh: Hours; leading zero for single-digit hours (12-hour clock).
//	H: Hours; no leading zero for single-digit hours (24-hour clock).
//	HH: Hours; leading zero for single-digit hours (24-hour clock).
//	M: Minutes; no leading zero for single-digit minutes.
//	MM: Minutes; leading zero for single-digit minutes.
//	s: Seconds; no leading zero for single-digit seconds.
//	ss: Seconds; leading zero for single-digit seconds.
//	AM/PM: Use the 12-hour clock and display ‘AM’ or ‘PM’.
//	0: Tenths of a second; 1 digit.
//	00: Hundredths of a second; 2 digits.
//	000: Milliseconds; 3 digits.
//
//	R: Relative time (e.g., "just now", "in 5 minutes", "3 days ago").

	{
	if (strEqualsNoCase(m_sFormat, FORMAT_R))
		{
		CTimeSpan TimePassed = timeSpan(CDateTime(CDateTime::Now), Value);
		if (TimePassed.Days() == 0 && TimePassed.MillisecondsSinceMidnight() < 1000)
			return STR_JUST_NOW;
		else if (TimePassed.Days() > 356 || TimePassed.Days() < -365)
			{
			CStringFormat DateFormat(FORMAT_DEFAULT_LONG_DATE);
			return DateFormat.FormatDateTime(Value);
			}
		else if (TimePassed.IsNegative())
			return strPattern("%s ago", FormatTimeSpan(TimePassed));
		else
			return strPattern("in %s", FormatTimeSpan(TimePassed));
		}

	CStringBuffer Buffer;
	const char* pPos = m_sFormat.GetParsePointer();
	bool bInTime = false;

	while (*pPos != '\0')
		{
		if (*pPos == 'd' || *pPos == 'D')
			{
			if (pPos[1] == 'd' || pPos[1] == 'D')
				{
				if (pPos[2] == 'd' || pPos[2] == 'D')
					{
					//	dddd

					if (pPos[3] == 'd' || pPos[3] == 'D')
						{
						Buffer.Write(Value.DayOfWeekLong());
						pPos += 4;
						}

					//	ddd

					else
						{
						Buffer.Write(Value.DayOfWeekShort());
						pPos += 3;
						}
					}

				//	dd

				else
					{
					Buffer.Write(strPattern("%02d", Value.Day()));
					pPos += 2;
					}
				}

			//	d

			else
				{
				Buffer.Write(strPattern("%d", Value.Day()));
				pPos += 1;
				}

			bInTime = false;
			}
		else if (*pPos == 'm' || *pPos == 'M')
			{
			if (pPos[1] == 'm' || pPos[1] == 'M')
				{
				if (pPos[2] == 'm' || pPos[2] == 'M')
					{
					//	mmmm

					if (pPos[3] == 'm' || pPos[3] == 'M')
						{
						Buffer.Write(Value.MonthLong());
						pPos += 4;
						}

					//	mmm

					else
						{
						Buffer.Write(Value.MonthShort());
						pPos += 3;
						}
					}

				//	mm

				else
					{
					if (bInTime)
						Buffer.Write(strPattern("%02d", Value.Minute()));
					else
						Buffer.Write(strPattern("%02d", Value.Month()));

					pPos += 2;
					}
				}

			//	m

			else
				{
				if (bInTime)
					Buffer.Write(strPattern("%d", Value.Minute()));
				else
					Buffer.Write(strPattern("%d", Value.Month()));

				pPos += 1;
				}
			}
		else if (*pPos == 'y' || *pPos == 'Y')
			{
			if (pPos[1] == 'y' || pPos[1] == 'Y')
				{
				if (pPos[2] == 'y' || pPos[2] == 'Y')
					{
					//	yyyy

					if (pPos[3] == 'y' || pPos[3] == 'Y')
						{
						Buffer.Write(strPattern("%04d", Value.Year()));
						pPos += 4;
						}

					//	yyy

					else
						{
						Buffer.WriteChar(*pPos);
						Buffer.Write(strPattern("%02d", Value.Year() % 100));
						pPos += 3;
						}
					}

				//	yy

				else
					{
					Buffer.Write(strPattern("%02d", Value.Year() % 100));
					pPos += 2;
					}
				}

			//	y

			else
				{
				Buffer.WriteChar(*pPos);
				pPos += 1;
				}

			bInTime = false;
			}
		else if (*pPos == 'h')
			{
			//	hh

			if (pPos[1] == 'h')
				{
				Buffer.Write(strPattern("%02d", Value.Hour12()));
				pPos += 2;
				}

			//	h

			else
				{
				Buffer.Write(strPattern("%d", Value.Hour12()));
				pPos += 1;
				}

			bInTime = true;
			}
		else if (*pPos == 'H')
			{
			//	HH

			if (pPos[1] == 'H')
				{
				Buffer.Write(strPattern("%02d", Value.Hour()));
				pPos += 2;
				}

			//	H

			else
				{
				Buffer.Write(strPattern("%d", Value.Hour()));
				pPos += 1;
				}

			bInTime = true;
			}
		else if (*pPos == 's' || *pPos == 'S')
			{
			if (pPos[1] == 's' || pPos[1] == 'S')
				{
				Buffer.Write(strPattern("%02d", Value.Second()));
				pPos += 2;
				}

			//	s

			else
				{
				Buffer.Write(strPattern("%d", Value.Second()));
				pPos += 1;
				}

			bInTime = true;
			}
		else if (*pPos == '0')
			{
			if (pPos[1] == '0')
				{
				//	000

				if (pPos[2] == '0')
					{
					Buffer.Write(strPattern("%03d", Value.Millisecond()));
					pPos += 3;
					}

				//	00

				else
					{
					Buffer.Write(strPattern("%02d", (int)mathRound(Value.Millisecond() / 10.0)));
					pPos += 2;
					}
				}

			//	0

			else
				{
				Buffer.Write(strPattern("%d", (int)mathRound(Value.Millisecond() / 100.0)));
				pPos += 1;
				}

			bInTime = true;
			}
		else if ((*pPos == 'a' && pPos[1] == 'm')
				|| (*pPos == 'A' && pPos[1] == 'M')
				|| (*pPos == 'p' && pPos[1] == 'm')
				|| (*pPos == 'P' && pPos[1] == 'M'))
			{
			bool bUpperCase = (*pPos == 'A') || (*pPos == 'P');
			pPos++;
			if (*pPos == 'm' || *pPos == 'M')
				{
				pPos++;
				if (*pPos == '/')
					pPos++;

				if (*pPos == 'p' || *pPos == 'P')
					{
					pPos++;
					if (*pPos == 'm' || *pPos == 'M')
						pPos++;
					}
				}

			if (bUpperCase)
				{
				if (Value.Hour() < 12)
					Buffer.Write("AM");
				else
					Buffer.Write("PM");
				}
			else
				{
				if (Value.Hour() < 12)
					Buffer.Write("am");
				else
					Buffer.Write("pm");
				}

			bInTime = true;
			}
		else
			{
			Buffer.WriteChar(*pPos);
			pPos += 1;
			}
		}

	return CString(std::move(Buffer));
	}

CString CStringFormat::FormatDouble (double rValue) const

//	FormatDouble
//
//	Formats a double.

	{
	//	Make sure we have parsed the format string

	if (!InitNumericFormats())
		return STR_ERROR;

	SNumberFormat Format = CalcConditionForDouble(rValue);

	//	Handle some special values.

	if (!std::isfinite(rValue))
		return STR_NAN;

	//	Remember if we have a negative value

	bool bNegative = false;
	if (rValue < 0.0)
		{
		rValue = -rValue;
		bNegative = true;
		}

	//	If we're formatting percent, then we need to multiply by 100

	if (Format.bPercent)
		rValue *= 100.0;

	//	Use sprintf to format the number

	CBuffer Buffer = FormatDoubleIntermediate(rValue, Format);
	if (Buffer.GetLength() == 0)
		return STR_ERROR;

	//  Count the number of characters before and after the decimal point

	SDecimalDigits Digits;
	CalcDecimalDigits(Buffer.GetPointer(), bNegative, Digits);

	//  Format

	return FormatDoubleOutput(Digits, Format);
	}

CBuffer CStringFormat::FormatDoubleIntermediate (double rValue, const SNumberFormat& Format)

//	FormatDoubleIntermediate
//
//	Formates the given value using sprintf, but without any other formatting.
//	If we get an error, we return an empty buffer.

	{
	static constexpr int MAX_INTEGER_DIGITS = 310;	//	Since max double is e+308, we need at most 308 digits, plus some extra.

	CString sFormatString;
	int iFormattedLength = MAX_INTEGER_DIGITS;
	if (Format.iMaxSignificantDigits == 0)
		{
		if (Format.iZeroPadding > 0)
			{
			sFormatString = strPattern("%%0%d.0f", Format.iZeroPadding);
			iFormattedLength = Max(Format.iZeroPadding + 1, iFormattedLength);
			}
		else
			{
			sFormatString = CString("%.0f");
			}
		}
	else
		{
		if (Format.iZeroPadding > 0)
			sFormatString = strPattern("%%0%d.%df", Format.iZeroPadding + 1 + Format.iMaxSignificantDigits, Format.iMaxSignificantDigits);
		else
			sFormatString = strPattern("%%0.%df", Format.iMaxSignificantDigits);

		iFormattedLength = Max(Format.iZeroPadding, iFormattedLength) + 1 + Format.iMaxSignificantDigits;
		}

	//	Use sprintf to generate the string
	//
	//	NOTE: We cannot rely on sprintf to round appropriately because it 
	//	doesn't look at all digits. For example:
	//
	//	printf("%.1f", 1.15) -> 1.1
	//
	//	but:
	//
	//	printf("%.1f", 1.16) -> 1.2
	//
	//	Why? Because 1.15 is actually 1.14999999 in IEEE floating point, and
	//	printf only looks at the first digit after the desired precision.
	//
	//	We use SafeRound to round to the appropriate number of digits, but we
	//	only do it for numbers in a certain range because we don't want the
	//	rounding to affect numbers that are too large.

	CBuffer Buffer;
	Buffer.SetLength(iFormattedLength + 1);	//	+1 for NULL terminator
	int err = sprintf_s(Buffer.GetPointer(), Buffer.GetLength(), sFormatString.GetParsePointer(), SafeRound(rValue, Format.iMaxSignificantDigits));
	if (err < 0)
		return CBuffer();

	return Buffer;
	}

CString CStringFormat::FormatDoubleOutput (const SDecimalDigits& Digits, const SNumberFormat& Format)

//	FormatDoubleOutput
//
//	Returns a formatted double.

	{
	//  Figure out the length of the destination string.

	int iOffset;
	int iDestLength = CalcFormatDoubleLength(Digits, Format, iOffset);
	if (iDestLength == 0)
		{
		if (Format.bHasDigits)
			return strPattern("%s0%s", Format.sLeading, Format.sTrailing);
		else
			return strPattern("%s%s", Format.sLeading, Format.sTrailing);
		}

	//  Now we can generate the output.

	CString sResult(iDestLength);
	int iComma = Digits.iLeft % 3;
	if (iComma == 0)
		iComma = 3;

	char *pDest = sResult.GetPointer();
	const char *pDestEnd = pDest + iDestLength;

	//	Write leading text

	const char* pSrc = Format.sLeading.GetParsePointer();
	while (*pSrc != '\0' && pDest < pDestEnd)
		*pDest++ = *pSrc++;

	//	Write the number
	
	if (Format.bHasDigits)
		{
		//	Write the negative sign

		if (Digits.bNegative && !Format.bNoNegativeSign)
			{
			*pDest++ = '-';
			}

		//  Write the left side

		pSrc = Digits.pDigits + iOffset;
		while (*pSrc != '\0' && *pSrc != '.' && pDest < pDestEnd)
			{
			*pDest++ = *pSrc++;
		
			if (Format.bCommaSeparators 
					&& --iComma == 0 
					&& *pSrc != '\0' && *pSrc != '.'
					&& pDest < pDestEnd)
				{
				*pDest++ = ',';
				iComma = 3;
				}
			}

		const char* pDestDigitsEnd = pDestEnd - Format.sTrailing.GetLength();

		//  Write the decimal point

		if (*pSrc == '.' && pDest < pDestDigitsEnd)
			{
			*pDest++ = *pSrc++;
			}

		//  Write the right side

		while (*pSrc != '\0' && pDest < pDestDigitsEnd)
			{
			*pDest++ = *pSrc++;
			}
		}

	//	Write trailing text

	pSrc = Format.sTrailing.GetParsePointer();
	while (*pSrc != '\0' && pDest < pDestEnd)
		*pDest++ = *pSrc++;

	ASSERT(pDest == sResult.GetPointer() + iDestLength);

	//  Done

	*pDest = '\0';
	return sResult;
	}

CString CStringFormat::FormatInteger (int iValue) const

//	FormatInteger
//
//	Formats a 32-bit signed integer.

	{
	//	Make sure we have parsed the format string

	if (!InitNumericFormats())
		return STR_ERROR;

	SNumberFormat Format = CalcConditionForInteger(iValue);

	//	Remember if we have a negative value

	bool bNegative = false;
	if (iValue < 0)
		{
		iValue = -iValue;
		bNegative = true;
		}

	CString sDigits = strFromInt(iValue);
	return FormatIntegerDigits(sDigits, bNegative, Format);
	}

CString CStringFormat::FormatIntegerDigits (CStringView sDigits, bool bNegative, const SNumberFormat& Format)
	{
	//	Figure out how big the result will be.

	int iDestLength = 0;
	int iDigitCount = Max(Format.iZeroPadding, sDigits.GetLength());

	int iDigitOffset = 0;
	if (Format.bHasDigits)
		{
		iDestLength += iDigitCount;

		if (bNegative && !Format.bNoNegativeSign)
			iDestLength++;

		if (Format.bCommaSeparators)
			iDestLength += (iDigitCount - 1) / 3;

		if (Format.iMinSignificantDigits > 0)
			iDestLength += 1 + Format.iMinSignificantDigits;

		//	If we don't have any leading zeros and the buffer starts with 0. then
		//	we need to trim them.

		if (Format.iZeroPadding == 0 && sDigits.GetLength() == 1 && *sDigits.GetParsePointer() == '0' && Format.iMinSignificantDigits > 0)
			{
			iDestLength -= 1;
			iDigitOffset = 1;
			}
		}

	iDestLength += Format.sLeading.GetLength() + Format.sTrailing.GetLength();
	if (iDestLength == 0)
		return NULL_STR;

	//	Now generate the output.

	CString sResult(iDestLength);
	int iComma = iDigitCount % 3;
	if (iComma == 0)
		iComma = 3;

	char* pDest = sResult.GetPointer();
	const char* pDestEnd = pDest + iDestLength;

	//	Write leading text

	const char* pSrc = Format.sLeading.GetParsePointer();
	while (*pSrc != '\0' && pDest < pDestEnd)
		*pDest++ = *pSrc++;

	//	Write the number

	if (Format.bHasDigits)
		{
		//	Write the negative sign

		if (bNegative && !Format.bNoNegativeSign)
			{
			*pDest++ = '-';
			}

		//	Write any leading zeros

		pSrc = sDigits.GetParsePointer() + iDigitOffset;
		if (Format.iZeroPadding > sDigits.GetLength())
			{
			int iLeadingZeros = Format.iZeroPadding - sDigits.GetLength();
			while (iLeadingZeros-- > 0 && pDest < pDestEnd)
				{
				*pDest++ = '0';

				if (Format.bCommaSeparators 
						&& --iComma == 0 
						&& *pSrc != '\0'
						&& pDest < pDestEnd)
					{
					*pDest++ = ',';
					iComma = 3;
					}
				}
			}

		//	Write the number

		while (*pSrc != '\0' && pDest < pDestEnd)
			{
			*pDest++ = *pSrc++;
				
			if (Format.bCommaSeparators 
					&& --iComma == 0 
					&& *pSrc != '\0'
					&& pDest < pDestEnd)
				{
				*pDest++ = ',';
				iComma = 3;
				}
			}

		//	Write any trailing zeros

		if (Format.iMinSignificantDigits > 0 && pDest < pDestEnd)
			{
			*pDest++ = '.';

			int iTrailingZeros = Format.iMinSignificantDigits;
			while (iTrailingZeros-- > 0 && pDest < pDestEnd)
				{
				*pDest++ = '0';
				}
			}
		}

	//	Write any trailing text

	pSrc = Format.sTrailing.GetParsePointer();
	while (*pSrc != '\0' && pDest < pDestEnd)
		*pDest++ = *pSrc++;

	ASSERT(pDest == sResult.GetPointer() + iDestLength);

	//  Done

	*pDest = '\0';
	return sResult;
	}

CString CStringFormat::FormatIPInteger (const CIPInteger& Value) const

//	FormatIPInteger
//
//	Formats an IP Integer

	{
	//	Make sure we have parsed the format string

	if (!InitNumericFormats())
		return STR_ERROR;

	const SNumberFormat Format = CalcConditionForIPInteger(Value);

	//	First format the integer as a string.

	bool bNegative = Value.IsNegative();
	CString sValue = (Format.bHasDigits ? Value.AsString(true) : NULL_STR);	//	TRUE means omit sign (we add it later)

	//	Format

	return FormatIntegerDigits(sValue, bNegative, Format);
	}

CString CStringFormat::FormatNull () const

//	FormatNull
//
//	Formats a null value.

	{
	//	Make sure we have parsed the format string

	if (!InitNumericFormats())
		return STR_ERROR;

	for (int i = 0; i < m_Clauses.GetCount(); i++)
		{
		//	If we have an explicit null clause, then use it.

		if (m_Clauses[i].iCondition == ECondition::Null)
			return m_Clauses[i].Format.sLeading + m_Clauses[i].Format.sTrailing;
		}

	//	Else, we always return an empty string.

	return NULL_STR;
	}

CString CStringFormat::FormatTimeSpan (const CTimeSpan& Value) const

//	FormatTimeSpan
//
//	Formats a time span using the following fields:
//
//	R: Adjustable time span (e.g., 1 day, 2 hours, 3 minutes, 4 seconds).

	{
	if (strEqualsNoCase(m_sFormat, FORMAT_R))
		{
		return FormatTimeSpanHumanized(Value, 1);
		}
	else if (strEqualsNoCase(m_sFormat, FORMAT_RR))
		{
		return FormatTimeSpanHumanized(Value, 2);
		}
	else
		{
		//	We don't understand the format
		return STR_ERROR;
		}
	}

CString CStringFormat::FormatTimeSpanHumanized (const CTimeSpan& Value, int iParts) const

//	FormatTimeSpanHumanized
//
//	Formats as:
//
//		1 year
//		1 year, 2 months
//		1 year, 2 months, 3 days

	{
	int iTotalDays = Value.Days();
	int iTotalSeconds = Value.MillisecondsSinceMidnight() / 1000;

	int iYears = iTotalDays / 365;
	int iDays = iTotalDays % 365;
	int iMonths = iDays / 30;
	iDays = iDays % 30;

	int iHours = iTotalSeconds / (60 * 60);
	int iMinutes = (iTotalSeconds % (60 * 60)) / 60;
	int iSeconds = iTotalSeconds % 60;

	CString sYears = (iYears == 1 ? STR_ONE_YEAR : strPattern("%d years", iYears));
	CString sMonths = (iMonths == 1 ? STR_ONE_MONTH : strPattern("%d months", iMonths));
	CString sDays = (iDays == 1 ? STR_ONE_DAY : strPattern("%d days", iDays));
	CString sHours = (iHours == 1 ? STR_ONE_HOUR : strPattern("%d hours", iHours));
	CString sMinutes = (iMinutes == 1 ? STR_ONE_MINUTE : strPattern("%d minutes", iMinutes));
	CString sSeconds = (iSeconds == 1 ? STR_ONE_SECOND : strPattern("%d seconds", iSeconds));

	if (iYears > 0)
		{
		if (iParts == 1)
			return sYears;
		else if (iParts == 2)
			{
			if (iMonths == 0)
				return sYears;
			else
				return strPattern("%s and %s", sYears, sMonths);
			}
		else
			{
			if (iMonths == 0 && iDays == 0)
				return sYears;
			else if (iMonths == 0)
				return strPattern("%s and %s", sYears, sDays);
			else if (iDays == 0)
				return strPattern("%s and %s", sYears, sMonths);
			else
				return strPattern("%s, %s, and %s", sYears, sMonths, sDays);
			}
		}
	else if (iMonths > 0)
		{
		if (iParts == 1)
			return sMonths;
		else if (iParts == 2)
			{
			if (iDays == 0)
				return sMonths;
			else
				return strPattern("%s and %s", sMonths, sDays);
			}
		else
			{
			if (iDays == 0)
				return sMonths;
			else
				return strPattern("%s and %s", sMonths, sDays);
			}
		}
	else if (iDays > 0)
		{
		if (iParts == 1)
			return sDays;
		else if (iParts == 2)
			{
			if (iHours == 0)
				return sDays;
			else
				return strPattern("%s and %s", sDays, sHours);
			}
		else
			{
			if (iHours == 0)
				return sDays;
			else
				return strPattern("%s, %s, and %s", sDays, sHours, sMinutes);
			}
		}
	else if (iHours > 0)
		{
		if (iParts == 1)
			return sHours;
		else if (iParts == 2)
			{
			if (iMinutes == 0)
				return sHours;
			else
				return strPattern("%s and %s", sHours, sMinutes);
			}
		else
			{
			if (iMinutes == 0)
				return sHours;
			else
				return strPattern("%s, %s, and %s", sHours, sMinutes, sSeconds);
			}
		}
	else if (iMinutes > 0)
		{
		if (iParts == 1)
			return sMinutes;
		else
			return strPattern("%s and %s", sMinutes, sSeconds);
		}
	else
		return sSeconds;
	}

CStringFormat::SNumberFormat CStringFormat::CalcConditionForDouble (double rValue) const
	{
	for (int i = 0; i < m_Clauses.GetCount(); i++)
		{
		switch (m_Clauses[i].iCondition)
			{
			case ECondition::Zero:
				if (rValue == 0.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::One:
				if (rValue == 1.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::NegativeOne:
				if (rValue == -1.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::Positive:
				if (rValue > 0.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::Negative:
				if (rValue < 0.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::NotZero:
				if (rValue != 0.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::NotOne:
				if (rValue != 1.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::Null:
				if (!std::isfinite(rValue))
					return m_Clauses[i].Format;
				break;

			case ECondition::GreaterThanOne:
				if (rValue > 1.0)
					return m_Clauses[i].Format;
				break;

			case ECondition::LessThanNegativeOne:
				if (rValue < -1.0)
					return m_Clauses[i].Format;
				break;

			default:
				return m_Clauses[i].Format;
			}
		}

	return SNumberFormat();
	}

CStringFormat::SNumberFormat CStringFormat::CalcConditionForInteger (int iValue) const
	{
	for (int i = 0; i < m_Clauses.GetCount(); i++)
		{
		switch (m_Clauses[i].iCondition)
			{
			case ECondition::Zero:
				if (iValue == 0)
					return m_Clauses[i].Format;
				break;

			case ECondition::One:
				if (iValue == 1)
					return m_Clauses[i].Format;
				break;

			case ECondition::NegativeOne:
				if (iValue == -1)
					return m_Clauses[i].Format;
				break;

			case ECondition::Positive:
				if (iValue > 0)
					return m_Clauses[i].Format;
				break;

			case ECondition::Negative:
				if (iValue < 0)
					return m_Clauses[i].Format;
				break;

			case ECondition::NotZero:
				if (iValue != 0)
					return m_Clauses[i].Format;
				break;

			case ECondition::NotOne:
				if (iValue != 1)
					return m_Clauses[i].Format;
				break;

			case ECondition::Null:
				break;

			case ECondition::GreaterThanOne:
				if (iValue > 1)
					return m_Clauses[i].Format;
				break;

			case ECondition::LessThanNegativeOne:
				if (iValue < -1)
					return m_Clauses[i].Format;
				break;

			default:
				return m_Clauses[i].Format;
			}
		}

	return SNumberFormat();
	}

CStringFormat::SNumberFormat CStringFormat::CalcConditionForIPInteger (const CIPInteger& Value) const
	{
	for (int i = 0; i < m_Clauses.GetCount(); i++)
		{
		switch (m_Clauses[i].iCondition)
			{
			case ECondition::Zero:
				if (Value.IsZero())
					return m_Clauses[i].Format;
				break;

			case ECondition::One:
				if (Value == CIPInteger(1))
					return m_Clauses[i].Format;
				break;

			case ECondition::NegativeOne:
				if (Value == CIPInteger(-1))
					return m_Clauses[i].Format;
				break;

			case ECondition::Positive:
				if (Value > CIPInteger(0))
					return m_Clauses[i].Format;
				break;

			case ECondition::Negative:
				if (Value < CIPInteger(-1))
					return m_Clauses[i].Format;
				break;

			case ECondition::NotZero:
				if (!Value.IsZero())
					return m_Clauses[i].Format;
				break;

			case ECondition::NotOne:
				if (Value != CIPInteger(1))
					return m_Clauses[i].Format;
				break;

			case ECondition::Null:
				if (Value.IsEmpty())
					return m_Clauses[i].Format;
				break;

			case ECondition::GreaterThanOne:
				if (Value > CIPInteger(1))
					return m_Clauses[i].Format;
				break;

			case ECondition::LessThanNegativeOne:
				if (Value < CIPInteger(-1))
					return m_Clauses[i].Format;
				break;

			default:
				return m_Clauses[i].Format;
			}
		}

	return SNumberFormat();
	}

void CStringFormat::CalcDecimalDigits (const char* pSrc, bool bNegative, SDecimalDigits& retDigits)

//	CalcDecimalDigits
//
//	Count digits in a decimal representation.

	{
	retDigits.pDigits = pSrc;
	retDigits.bNegative = bNegative;

	while (*pSrc != '\0')
		{
		if (*pSrc == '.')
			{
			retDigits.bDecimal = true;
			}
		else if (retDigits.bDecimal)
			{
			retDigits.iRight++;
			}
		else
			{
			retDigits.iLeft++;
			}
		
		pSrc++;
		}
	}

int CStringFormat::CalcFormatDoubleLength (const SDecimalDigits& Digits, const SNumberFormat& Format, int& retiOffset)

//	CalcFormatDoubleLength
//
//	Returns the length of the destination based on format.

	{
	int iDestLength = 0;

	if (Format.bHasDigits)
		{
		iDestLength += Digits.iLeft;
		if (Digits.bDecimal)
			iDestLength++;
		if (Format.bCommaSeparators)
			iDestLength += (Digits.iLeft - 1) / 3;

		int iMinDestLength = iDestLength + Format.iMinSignificantDigits;
		iDestLength += Format.iMaxSignificantDigits;

		//	If we don't have any leading zeros and the buffer starts with 0. then
		//	we need to trim them.

		retiOffset = 0;
		if (Format.iZeroPadding == 0 && Digits.iLeft > 0 && Digits.bDecimal && Digits.pDigits[0] == '0' && Digits.pDigits[1] == '.')
			{
			iDestLength -= 1;
			iMinDestLength -= 1;
			retiOffset = 1;
			}

		//	If we have more trailing zeros than the minimum length, then scan the
		//	buffer backwards to see if we need to truncate.

		if (Digits.iRight > Format.iMinSignificantDigits)
			{
			const char* pSrc = Digits.pDigits + Digits.iRight + Digits.iLeft + (Digits.bDecimal ? 1 : 0) - 1;
			while (pSrc > Digits.pDigits && *pSrc == '0' && iDestLength > iMinDestLength)
				{
				iDestLength--;
				pSrc--;
				}

			//	If we're at the decimal point, then we can remove it too.

			if (pSrc > Digits.pDigits && *pSrc == '.')
				{
				iDestLength--;
				pSrc--;
				}
			}

		//	If we deleted everything, then we at least output 0

		if (iDestLength == 0)
			return 0;

		//	If we have a negative value, then we need to add a character

		if (Digits.bNegative && !Format.bNoNegativeSign)
			iDestLength++;
		}

	//	Add leading and trailing values

	iDestLength += Format.sLeading.GetLength() + Format.sTrailing.GetLength();

	return iDestLength;
	}

bool CStringFormat::IsPluralizationFormat (const TArray<CString>& Clauses)
	{
	//	Must have exactly two clauses.

	if (Clauses.GetCount() != 2)
		return false;

	//	Neither clause has a condition.

	for (int i = 0; i < Clauses.GetCount(); i++)
		{
		const char* pPos = Clauses[i].GetParsePointer();
		if (*pPos == '=' || *pPos == '<' || *pPos == '!' || *pPos == '>')
			return false;
		}

	return true;
	}

bool CStringFormat::IsSpecialPluralizationFormat (const TArray<CString>& Clauses)
	{
	//	Must have exactly two clauses.

	if (Clauses.GetCount() != 2)
		return false;

	//	The first clause must have a numeric format.

	bool bFirstHasNumeric = false;
	const char* pPos = Clauses[0].GetParsePointer();
	while (*pPos != ' ' && *pPos != '\0')
		{
		if (*pPos == '#' || *pPos == '0')
			{
			bFirstHasNumeric = true;
			break;
			}
		pPos++;
		}

	if (!bFirstHasNumeric)
		return false;

	//	The second clause must not have a numeric format.

	pPos = Clauses[1].GetParsePointer();
	while (*pPos != ' ' && *pPos != '\0')
		{
		if (*pPos == '#' || *pPos == '0')
			return false;
		pPos++;
		}

	return true;
	}

void CStringFormat::ParseClause (CStringView sFormat, SConditionalClause& retClause)
	{
	bool bHasCondition = false;
	const char* pPos = sFormat.GetParsePointer();
	if (*pPos == '=')
		{
		pPos++;
		if (*pPos == '0')
			retClause.iCondition = ECondition::Zero;
		else if (*pPos == '1')
			retClause.iCondition = ECondition::One;
		else if (*pPos == '-' && pPos[1] == '1')
			retClause.iCondition = ECondition::NegativeOne;
		else
			retClause.iCondition = ECondition::Default;

		bHasCondition = true;
		}
	else if (*pPos == '>')
		{
		pPos++;
		if (*pPos == '0')
			retClause.iCondition = ECondition::Positive;
		else if (*pPos == '1')
			retClause.iCondition = ECondition::GreaterThanOne;
		else
			retClause.iCondition = ECondition::Default;

		bHasCondition = true;
		}
	else if (*pPos == '<')
		{
		pPos++;
		if (*pPos == '0')
			retClause.iCondition = ECondition::Negative;
		else if (*pPos == '-' && pPos[1] == '1')
			retClause.iCondition = ECondition::LessThanNegativeOne;
		else
			retClause.iCondition = ECondition::Default;

		bHasCondition = true;
		}
	else if (*pPos == '!')
		{
		pPos++;
		if (*pPos == '0')
			retClause.iCondition = ECondition::NotZero;
		else if (*pPos == '1')
			retClause.iCondition = ECondition::NotOne;
		else if (*pPos == ':')
			retClause.iCondition = ECondition::Null;
		else
			retClause.iCondition = ECondition::Default;

		bHasCondition = true;
		}
	else
		{
		retClause.iCondition = ECondition::Default;
		}

	if (bHasCondition)
		{
		while (*pPos != ':' && *pPos != '\0')
			pPos++;

		if (*pPos == ':')
			pPos++;
		}

	retClause.Format = ParseNumberFormat(CString(pPos));

	//	If we have a special condition for negative numbers, then we don't need
	//	the negative sign in the format.

	if (retClause.iCondition == ECondition::Negative || retClause.iCondition == ECondition::NegativeOne)
		retClause.Format.bNoNegativeSign = true;
	}

TArray<CStringFormat::SConditionalClause> CStringFormat::Parse (CStringView sFormat, bool* retbFormatError)

//	ParseFormat
//
//	Parses a multi-clause format.

	{
	//	Short-circuit if the format is ridiculously long.

	if (sFormat.GetLength() > MAX_FORMAT_STRING)
		{
		if (retbFormatError)
			*retbFormatError = true;

		return TArray<SConditionalClause>();
		}

	TArray<SConditionalClause> Result;

	//	Split by the vertical bar separator.

	TArray<CString> Clauses;
	strSplit(sFormat, STR_BAR, &Clauses, -1, 0);

	//	There is a special case where we have two clauses and only the first 
	//	clause has a numeric format. In that case, we assume that the second
	//	clause has the same numeric format and represents the plural version.

	if (IsSpecialPluralizationFormat(Clauses))
		{
		Result.InsertEmpty(2);
		Result[0].iCondition = ECondition::One;
		Result[0].Format = ParseNumberFormat(Clauses[0]);

		const char* pStart = Clauses[0].GetParsePointer();
		const char* pPos = pStart;
		while (*pPos != ' ' && *pPos != '\0')
			pPos++;

		if (*pPos == ' ')
			pPos++;

		Result[1].iCondition = ECondition::Default;
		Result[1].Format = ParseNumberFormat(CString(pStart, pPos - pStart) + Clauses[1]);
		}

	//	If there are exactly two clauses and neither has a conditions, then
	//	we assume that the first clause is the singular and the second is the
	//	plural.

	else if (IsPluralizationFormat(Clauses))
		{
		Result.InsertEmpty(2);
		Result[0].iCondition = ECondition::One;
		Result[0].Format = ParseNumberFormat(Clauses[0]);

		Result[1].iCondition = ECondition::Default;
		Result[1].Format = ParseNumberFormat(Clauses[1]);
		}

	//	Otherwise, we expect a condition for each clause (or we assume default).

	else
		{
		Result.InsertEmpty(Clauses.GetCount());
		for (int i = 0; i < Clauses.GetCount(); i++)
			{
			ParseClause(Clauses[i], Result[i]);
			}
		}

	if (retbFormatError)
		*retbFormatError = false;

	return Result;
	}

CStringFormat::SNumberFormat CStringFormat::ParseNumberFormat (CStringView sFormat)

//  ParseNumberFormat
//
//  Parses a number format.

	{
	SNumberFormat Result;

	//	Handle some special values.

	if (strEquals(sFormat, STR_PERCENT))
		{
		Result.bPercent = true;
		Result.bHasDigits = true;
		Result.sTrailing = STR_PERCENT;
		return Result;
		}

	//	Parse

	const char* pPos = sFormat.GetParsePointer();

	//	Get any leading string.

	const char* pStart = pPos;
	while (*pPos != '\0' && *pPos != '#' && *pPos != '0' && *pPos != ',' && *pPos != '.')
		{
		if (*pPos == '%')
			{
			Result.bPercent = true;
			Result.bHasDigits = true;
			}
		pPos++;
		}

	if (pPos != pStart)
		Result.sLeading = CString(pStart, pPos - pStart);

	//  Skip 0 padding

	while (*pPos == '0' || *pPos == ',')
		{
		if (*pPos == '0')
			{
			Result.iZeroPadding++;
			Result.bHasDigits = true;
			}
		else if (*pPos == ',')
			Result.bCommaSeparators = true;

		pPos++;
		}

	//	One or more numbers or comma.

	while (*pPos == ',' || *pPos == '#' || *pPos == '0')
		{
		if (*pPos == '#')
			{
			if (Result.iZeroPadding)
				Result.iZeroPadding++;
			Result.bHasDigits = true;
			}
		else if (*pPos == '0')
			{
			Result.iZeroPadding++;
			Result.bHasDigits = true;
			}
		else if (*pPos == ',')
			{
			Result.bCommaSeparators = true;
			}

		pPos++;
		}

	//	Decimal

	if (*pPos == '.')
		{
		pPos++;

		while (*pPos == '#' || *pPos == '0')
			{
			if (*pPos == '#')
				{
				Result.iMaxSignificantDigits++;
				Result.bHasDigits = true;
				}
			else if (*pPos == '0')
				{
				Result.iMaxSignificantDigits++;
				Result.iMinSignificantDigits = Result.iMaxSignificantDigits;
				Result.bHasDigits = true;
				}
			pPos++;
			}
		}

	//	Trailing

	pStart = pPos;
	while (*pPos != '\0')
		{
		if (*pPos == '%')
			{
			Result.bPercent = true;
			Result.bHasDigits = true;
			}
		pPos++;
		}

	if (pStart != pPos)
		Result.sTrailing = CString(pStart, pPos - pStart);

	return Result;
	}

double CStringFormat::SafeRound (double rValue, int iDigits)
	{
	// Check magnitude of rValue

	double absValue = abs(rValue);
	if (absValue < 1e15 && iDigits < 15) 
		{
		double scale = pow(10, iDigits);
		return round(rValue * scale) / scale;
		}

	// If it's not safe to round, return the original value	

	return rValue;
	}
