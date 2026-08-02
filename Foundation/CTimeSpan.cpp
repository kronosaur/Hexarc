//	CTimeSpan.cpp
//
//	CTimeSpan class
//	Copyright (c) 2026 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_COLON,							":");

bool CTimeSpan::Parse (CStringView sValue, CTimeSpan& retResult)

//	Parse
//
//	Parses a string into a CTimeSpan. Supported formats:
//
//		"123"        -> seconds
//		"123.456"    -> seconds with milliseconds
//		"3:45"       -> minutes:seconds
//		"2:3:45"     -> hours:minutes:seconds
//		"1:2:3:45"   -> days:hours:minutes:seconds
//
//	Returns an false on parse error.

	{
	retResult = CTimeSpan();
	if (sValue.IsEmpty())
		return false;

	//	Negative?

	bool bNegative = false;
	const char* pPos = sValue.GetPointer();
	while (strIsWhitespace(*pPos))
		pPos++;

	CString sTrimmed;
	if (*pPos == '-')
		{
		bNegative = true;
		sTrimmed = CString(pPos + 1);
		}
	else
		sTrimmed = CString(pPos);

	//	Split on colons

	TArray<CString> Parts;
	strSplit(sTrimmed, STR_COLON, &Parts);

	if (Parts.GetCount() == 0 || Parts.GetCount() > 4)
		return false;

	int iDays = 0;
	int iHours = 0;
	int iMinutes = 0;
	double rSeconds = 0.0;

	//	Parse based on number of parts
	//	NOTE: strToInt and strToDouble skip leading whitespace automatically.

	switch (Parts.GetCount())
		{
		case 1:
			//	Just seconds (possibly with decimal)
			rSeconds = strToDouble(Parts[0]);
			break;

		case 2:
			//	minutes:seconds
			{
			bool bFailed;
			iMinutes = strToInt(Parts[0], 0, &bFailed);
			if (bFailed)
				return false;

			rSeconds = strToDouble(Parts[1]);
			break;
			}

		case 3:
			//	hours:minutes:seconds
			{
			bool bFailed;
			iHours = strToInt(Parts[0], 0, &bFailed);
			if (bFailed)
				return false;

			iMinutes = strToInt(Parts[1], 0, &bFailed);
			if (bFailed)
				return false;

			rSeconds = strToDouble(Parts[2]);
			break;
			}

		case 4:
			//	days:hours:minutes:seconds
			{
			bool bFailed;
			iDays = strToInt(Parts[0], 0, &bFailed);
			if (bFailed)
				return false;

			iHours = strToInt(Parts[1], 0, &bFailed);
			if (bFailed)
				return false;

			iMinutes = strToInt(Parts[2], 0, &bFailed);
			if (bFailed)
				return false;

			rSeconds = strToDouble(Parts[3]);
			break;
			}
		}

	//	Convert to total milliseconds. We compute in DWORDLONG to avoid overflow.

	int iWholeSeconds = (int)rSeconds;
	int iMilliseconds = (int)((rSeconds - iWholeSeconds) * 1000.0 + 0.5);

	DWORDLONG dwTotalMilliseconds =
		(DWORDLONG)iDays * 24 * 60 * 60 * 1000 +
		(DWORDLONG)iHours * 60 * 60 * 1000 +
		(DWORDLONG)iMinutes * 60 * 1000 +
		(DWORDLONG)iWholeSeconds * 1000 +
		(DWORDLONG)iMilliseconds;

	retResult = CTimeSpan(dwTotalMilliseconds, bNegative);
	return true;
	}
