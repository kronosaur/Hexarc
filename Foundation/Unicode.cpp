//	Unicode.cpp
//
//	Unicode functions
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include "UnicodeTables.h"

static WORD g_LowercaseTable00[256];
static WORD g_LowercaseTable01[256];
static WORD g_LowercaseTable02[256];
static WORD g_LowercaseTable03[256];
static WORD g_LowercaseTable04[256];
static WORD g_LowercaseTable05[256];
static WORD g_LowercaseTable10[256];
static WORD g_LowercaseTable1E[256];
static WORD g_LowercaseTable1F[256];
static WORD g_LowercaseTable24[256];
static WORD g_LowercaseTableFF[256];

static WORD g_UppercaseTable00[256];
static WORD g_UppercaseTable01[256];
static WORD g_UppercaseTable02[256];
static WORD g_UppercaseTable03[256];
static WORD g_UppercaseTable04[256];
static WORD g_UppercaseTable05[256];
static WORD g_UppercaseTable10[256];
static WORD g_UppercaseTable1E[256];
static WORD g_UppercaseTable1F[256];
static WORD g_UppercaseTable24[256];
static WORD g_UppercaseTableFF[256];
static bool g_bCaseTablesInitialized = false;

CString strEncodeUTF8Char (UTF32 dwCodePoint)

//	strEncodeUTF8Char
//
//	Encodes a Unicode character into a UTF8 string

	{
	BYTE szBuffer[4];

	if (dwCodePoint <= 0x007f)
		{
		szBuffer[0] = (BYTE)dwCodePoint;
		return CString((char *)szBuffer, 1);
		}
	else if (dwCodePoint <= 0x07ff)
		{
		szBuffer[0] = (BYTE)(0xc0 | ((dwCodePoint & 0x0700) >> 6) | ((dwCodePoint & 0x00ff) >> 6));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		return CString((char *)szBuffer, 2);
		}
	else if (dwCodePoint <= 0xffff)
		{
		szBuffer[0] = (BYTE)(0xe0 | ((dwCodePoint & 0xf000) >> 12));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
		szBuffer[2] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		return CString ((char *)szBuffer, 3);
		}
	else
		{
		szBuffer[0] = (BYTE)(0xf0 | ((dwCodePoint & 0x1c0000) >> 18));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x030000) >> 12) | ((dwCodePoint & 0xf000) >> 12));
		szBuffer[2] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
		szBuffer[3] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		return CString ((char *)szBuffer, 4);
		}
	}

void strEncodeUTF8Char (UTF32 dwCodePoint, IByteStream &Stream)

//	strEncodeUTF8Char
//
//	Encodes a Unicode character into a UTF8 stream

	{
	BYTE szBuffer[4];

	if (dwCodePoint <= 0x007f)
		{
		szBuffer[0] = (BYTE)dwCodePoint;
		Stream.Write(szBuffer, 1);
		}
	else if (dwCodePoint <= 0x07ff)
		{
		szBuffer[0] = (BYTE)(0xc0 | ((dwCodePoint & 0x0700) >> 6) | ((dwCodePoint & 0x00ff) >> 6));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		Stream.Write(szBuffer, 2);
		}
	else if (dwCodePoint <= 0xffff)
		{
		szBuffer[0] = (BYTE)(0xe0 | ((dwCodePoint & 0xf000) >> 12));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
		szBuffer[2] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		Stream.Write(szBuffer, 3);
		}
	else
		{
		szBuffer[0] = (BYTE)(0xf0 | ((dwCodePoint & 0x1c0000) >> 18));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x030000) >> 12) | ((dwCodePoint & 0xf000) >> 12));
		szBuffer[2] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
		szBuffer[3] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		Stream.Write(szBuffer, 4);
		}
	}

void strEncodeUTF8Char (UTF32 dwCodePoint, char *pPos, char *pPosEnd)

//	strEncodeUTF8Char
//
//	Encodes a Unicode character into a UTF8 buffer

	{
	int iLength = (int)(pPosEnd - pPos);

	if (dwCodePoint <= 0x007f)
		{
		if (iLength >= 1)
			pPos[0] = (BYTE)dwCodePoint;
		}
	else if (dwCodePoint <= 0x07ff)
		{
		if (iLength >= 2)
			{
			pPos[0] = (BYTE)(0xc0 | ((dwCodePoint & 0x0700) >> 6) | ((dwCodePoint & 0x00ff) >> 6));
			pPos[1] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
			}
		}
	else if (dwCodePoint <= 0xffff)
		{
		if (iLength >= 3)
			{
			pPos[0] = (BYTE)(0xe0 | ((dwCodePoint & 0xf000) >> 12));
			pPos[1] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
			pPos[2] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
			}
		}
	else
		{
		if (iLength >= 4)
			{
			pPos[0] = (BYTE)(0xf0 | ((dwCodePoint & 0x1c0000) >> 18));
			pPos[1] = (BYTE)(0x80 | ((dwCodePoint & 0x030000) >> 12) | ((dwCodePoint & 0xf000) >> 12));
			pPos[2] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
			pPos[3] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
			}
		}
	}

void strInitLowercaseTable (UTF32 dwCodePoint, SToLowercase *pLookup, int iCount, WORD *pTable)
	{
	//	First initialize the table with an identity transform

	WORD *pDest = pTable;
	WORD *pDestEnd = pTable + 256;
	DWORD dwChar = dwCodePoint;
	while (pDest < pDestEnd)
		*pDest++ = (WORD)(dwChar++);

	//	Next the lowercase conversions

	SToLowercase *pPos = pLookup;
	SToLowercase *pEnd = pPos + iCount;
	while (pPos < pEnd)
		{
		pTable[pPos->dwUppercase & 0xff] = (pPos->dwLowercase & 0xffff);
		pPos++;
		}
	}

void strInitUppercaseTable (UTF32 dwCodePoint, SToLowercase *pLookup, int iCount, WORD *pTable)
	{
	//	First initialize the table with an identity transform

	WORD *pDest = pTable;
	WORD *pDestEnd = pTable + 256;
	DWORD dwChar = dwCodePoint;
	while (pDest < pDestEnd)
		*pDest++ = (WORD)(dwChar++);

	//	Next the uppercase conversions

	SToLowercase *pPos = pLookup;
	SToLowercase *pEnd = pPos + iCount;
	while (pPos < pEnd)
		{
		pTable[pPos->dwLowercase & 0xff] = (pPos->dwUppercase & 0xffff);
		pPos++;
		}
	}

void strInitCaseTables (void)
	{
	if (!g_bCaseTablesInitialized)
		{
		strInitLowercaseTable(0x0000, g_LowercaseLookup00, sizeof(g_LowercaseLookup00) / sizeof(g_LowercaseLookup00[0]), g_LowercaseTable00);
		strInitLowercaseTable(0x0100, g_LowercaseLookup01, sizeof(g_LowercaseLookup01) / sizeof(g_LowercaseLookup01[0]), g_LowercaseTable01);
		strInitLowercaseTable(0x0200, g_LowercaseLookup02, sizeof(g_LowercaseLookup02) / sizeof(g_LowercaseLookup02[0]), g_LowercaseTable02);
		strInitLowercaseTable(0x0300, g_LowercaseLookup03, sizeof(g_LowercaseLookup03) / sizeof(g_LowercaseLookup03[0]), g_LowercaseTable03);
		strInitLowercaseTable(0x0400, g_LowercaseLookup04, sizeof(g_LowercaseLookup04) / sizeof(g_LowercaseLookup04[0]), g_LowercaseTable04);
		strInitLowercaseTable(0x0500, g_LowercaseLookup05, sizeof(g_LowercaseLookup05) / sizeof(g_LowercaseLookup05[0]), g_LowercaseTable05);
		strInitLowercaseTable(0x1000, g_LowercaseLookup10, sizeof(g_LowercaseLookup10) / sizeof(g_LowercaseLookup10[0]), g_LowercaseTable10);
		strInitLowercaseTable(0x1E00, g_LowercaseLookup1E, sizeof(g_LowercaseLookup1E) / sizeof(g_LowercaseLookup1E[0]), g_LowercaseTable1E);
		strInitLowercaseTable(0x1F00, g_LowercaseLookup1F, sizeof(g_LowercaseLookup1F) / sizeof(g_LowercaseLookup1F[0]), g_LowercaseTable1F);
		strInitLowercaseTable(0x2400, g_LowercaseLookup24, sizeof(g_LowercaseLookup24) / sizeof(g_LowercaseLookup24[0]), g_LowercaseTable24);
		strInitLowercaseTable(0xFF00, g_LowercaseLookupFF, sizeof(g_LowercaseLookupFF) / sizeof(g_LowercaseLookupFF[0]), g_LowercaseTableFF);

		strInitUppercaseTable(0x0000, g_LowercaseLookup00, sizeof(g_LowercaseLookup00) / sizeof(g_LowercaseLookup00[0]), g_UppercaseTable00);
		strInitUppercaseTable(0x0100, g_LowercaseLookup01, sizeof(g_LowercaseLookup01) / sizeof(g_LowercaseLookup01[0]), g_UppercaseTable01);
		strInitUppercaseTable(0x0200, g_LowercaseLookup02, sizeof(g_LowercaseLookup02) / sizeof(g_LowercaseLookup02[0]), g_UppercaseTable02);
		strInitUppercaseTable(0x0300, g_LowercaseLookup03, sizeof(g_LowercaseLookup03) / sizeof(g_LowercaseLookup03[0]), g_UppercaseTable03);
		strInitUppercaseTable(0x0400, g_LowercaseLookup04, sizeof(g_LowercaseLookup04) / sizeof(g_LowercaseLookup04[0]), g_UppercaseTable04);
		strInitUppercaseTable(0x0500, g_LowercaseLookup05, sizeof(g_LowercaseLookup05) / sizeof(g_LowercaseLookup05[0]), g_UppercaseTable05);
		strInitUppercaseTable(0x1000, g_LowercaseLookup10, sizeof(g_LowercaseLookup10) / sizeof(g_LowercaseLookup10[0]), g_UppercaseTable10);
		strInitUppercaseTable(0x1E00, g_LowercaseLookup1E, sizeof(g_LowercaseLookup1E) / sizeof(g_LowercaseLookup1E[0]), g_UppercaseTable1E);
		strInitUppercaseTable(0x1F00, g_LowercaseLookup1F, sizeof(g_LowercaseLookup1F) / sizeof(g_LowercaseLookup1F[0]), g_UppercaseTable1F);
		strInitUppercaseTable(0x2400, g_LowercaseLookup24, sizeof(g_LowercaseLookup24) / sizeof(g_LowercaseLookup24[0]), g_UppercaseTable24);
		strInitUppercaseTable(0xFF00, g_LowercaseLookupFF, sizeof(g_LowercaseLookupFF) / sizeof(g_LowercaseLookupFF[0]), g_UppercaseTableFF);

		g_bCaseTablesInitialized = true;
		}
	}

bool strIsPrintableChar (UTF32 dwCodePoint)

//	strIsPrintableChar
//
//	Returns TRUE if this is a printable character.
//	See: http://www.cs.tut.fi/~jkorpela/chars/spaces.html

	{
	switch (dwCodePoint)
		{
		case 0x0020:
		case 0x00A0:
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
		case 0x2004:
		case 0x2005:
		case 0x2006:
		case 0x2007:
		case 0x2008:
		case 0x2009:
		case 0x200A:
		case 0x200B:
		case 0x202F:
		case 0x205F:
		case 0x3000:
		case 0xFEFF:
			return false;

		default:
			return true;
		}
	}

UTF32 strParseUTF8Char (char **iopPos, char *pEndPos)

//	strParseUTF8Char
//
//	Returns a single code point and advances the pointer

	{
	BYTE *pPos = (BYTE *)(*iopPos);
	if ((*pPos & 0x80) == 0x00)
		{
		(*iopPos) += 1;
		return *pPos;
		}
	else if ((*pPos & 0xe0) == 0xc0)
		{
		if ((*iopPos) + 2 > pEndPos)
			{
			*iopPos = pEndPos;
			return (UTF32)'?';
			}

		(*iopPos) += 2;
		return (((UTF32)pPos[0] & 0x1f) << 6) | ((UTF32)pPos[1] & 0x3f);
		}
	else if ((*pPos & 0xf0) == 0xe0)
		{
		if ((*iopPos) + 3 > pEndPos)
			{
			*iopPos = pEndPos;
			return (UTF32)'?';
			}

		(*iopPos) += 3;
		return (((UTF32)pPos[0] & 0x0f) << 12) | (((UTF32)pPos[1] & 0x3f) << 6) | ((UTF32)pPos[2] & 0x3f);
		}
	else
		{
		if ((*iopPos) + 4 > pEndPos)
			{
			*iopPos = pEndPos;
			return (UTF32)'?';
			}

		(*iopPos) += 4;
		return (((UTF32)pPos[0] & 0x07) << 18) | (((UTF32)pPos[1] & 0x3f) << 12) | (((UTF32)pPos[2] & 0x3f) << 6) | ((UTF32)pPos[3] & 0x3f);
		}
	}

UTF32 strToLowerChar (UTF32 dwCodePoint)

//	strToLowerChar
//
//	Converts a Unicode character to lowercase

	{
	if (dwCodePoint >= 0x0041 && dwCodePoint <= 0x005a)
		return (dwCodePoint + 0x20);
	else if ((dwCodePoint & 0xffff0000) == 0)
		{
		if (!g_bCaseTablesInitialized)
			strInitCaseTables();

		switch (dwCodePoint & 0x0000ff00)
			{
			case 0x0000:
				return g_LowercaseTable00[dwCodePoint & 0xff];

			case 0x0100:
				return g_LowercaseTable01[dwCodePoint & 0xff];

			case 0x0200:
				return g_LowercaseTable02[dwCodePoint & 0xff];

			case 0x0300:
				return g_LowercaseTable03[dwCodePoint & 0xff];

			case 0x0400:
				return g_LowercaseTable04[dwCodePoint & 0xff];

			case 0x0500:
				return g_LowercaseTable05[dwCodePoint & 0xff];

			case 0x1000:
				return g_LowercaseTable10[dwCodePoint & 0xff];

			case 0x1E00:
				return g_LowercaseTable1E[dwCodePoint & 0xff];

			case 0x1F00:
				return g_LowercaseTable1F[dwCodePoint & 0xff];

			case 0x2400:
				return g_LowercaseTable24[dwCodePoint & 0xff];

			case 0xFF00:
				return g_LowercaseTableFF[dwCodePoint & 0xff];

			default:
				return dwCodePoint;
			}
		}
	else
		return dwCodePoint;
	}

UTF32 strToUpperChar (UTF32 dwCodePoint)

//	strToUpperChar
//
//	Converts a Unicode character to uppercase

	{
	if (dwCodePoint >= 0x0061 && dwCodePoint <= 0x007a)
		return (dwCodePoint - 0x20);
	else if ((dwCodePoint & 0xffff0000) == 0)
		{
		if (!g_bCaseTablesInitialized)
			strInitCaseTables();

		switch (dwCodePoint & 0x0000ff00)
			{
			case 0x0000:
				return g_UppercaseTable00[dwCodePoint & 0xff];

			case 0x0100:
				return g_UppercaseTable01[dwCodePoint & 0xff];

			case 0x0200:
				return g_UppercaseTable02[dwCodePoint & 0xff];

			case 0x0300:
				return g_UppercaseTable03[dwCodePoint & 0xff];

			case 0x0400:
				return g_UppercaseTable04[dwCodePoint & 0xff];

			case 0x0500:
				return g_UppercaseTable05[dwCodePoint & 0xff];

			case 0x1000:
				return g_UppercaseTable10[dwCodePoint & 0xff];

			case 0x1E00:
				return g_UppercaseTable1E[dwCodePoint & 0xff];

			case 0x1F00:
				return g_UppercaseTable1F[dwCodePoint & 0xff];

			case 0x2400:
				return g_UppercaseTable24[dwCodePoint & 0xff];

			case 0xFF00:
				return g_UppercaseTableFF[dwCodePoint & 0xff];

			default:
				return dwCodePoint;
			}
		}
	else
		return dwCodePoint;
	}
