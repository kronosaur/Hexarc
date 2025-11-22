//	CRGBA32.cpp
//
//	CRGBA32 class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CRGBA32::AlphaArray8 CRGBA32::g_Alpha8 [256];
bool CRGBA32::m_bAlphaInitialized = CRGBA32::InitTables();

TSortMap<CString, CRGBA32> CRGBA32::m_HTMLColors = std::initializer_list<std::pair<CString, CRGBA32>>({
	{ "white",		CRGBA32(0xff, 0xff, 0xff) },
	{ "silver",		CRGBA32(0xc0, 0xc0, 0xc0) },
	{ "gray",		CRGBA32(0x80, 0x80, 0x80) },
	{ "black",		CRGBA32(0x00, 0x00, 0x00) },
	{ "red",		CRGBA32(0xff, 0x00, 0x00) },
	{ "maroon",		CRGBA32(0x80, 0x00, 0x00) },
	{ "yellow",		CRGBA32(0xff, 0xff, 0x00) },
	{ "olive",		CRGBA32(0x80, 0x80, 0x00) },
	{ "lime",		CRGBA32(0x00, 0xff, 0x00) },
	{ "green",		CRGBA32(0x00, 0x80, 0x00) },
	{ "aqua",		CRGBA32(0x00, 0xff, 0xff) },
	{ "teal",		CRGBA32(0x00, 0x80, 0x80) },
	{ "blue",		CRGBA32(0x00, 0x00, 0xff) },
	{ "navy",		CRGBA32(0x00, 0x00, 0x80) },
	{ "fuchsia",	CRGBA32(0xff, 0x00, 0xff) },
	{ "purple",		CRGBA32(0x80, 0x00, 0x80) },
});

CString CRGBA32::AsARGBColor () const

//	AsARGBColor
//
//	Returns AARRGGBB color as hex.

	{
	return strPattern("%02x%02x%02x%02x", GetAlpha(), GetRed(), GetGreen(), GetBlue());
	}

CString CRGBA32::AsHTMLColor () const

//	AsHTMLColor
//
//	Returns an HTML style color.

	{
	if (GetAlpha() == 0xff)
		return strPattern("#%02x%02x%02x", GetRed(), GetGreen(), GetBlue());
	else
		{
		double rAlpha = GetAlpha() / 255.0;
		return strPattern("rgba(%d,%d,%d,%s)", (int)GetRed(), (int)GetGreen(), (int)GetBlue(), strFromDouble(rAlpha, 2));
		}
	}

CRGBA32 CRGBA32::Blend (CRGBA32 rgbDest, CRGBA32 rgbSrc)

//	Blend
//
//	Combines rgbSrc with rgbDest, using rgbSrc as the source opacity.
//	We assume dest has no alpha component and we assume that rgbSrc does.

	{
	BYTE *pAlpha = g_Alpha8[rgbSrc.GetAlpha()];
	BYTE *pAlphaInv = g_Alpha8[rgbSrc.GetAlpha() ^ 0xff];	//	Equivalent to 255 - rgbSrc.GetAlpha()

	BYTE byRedResult = pAlphaInv[rgbDest.GetRed()] + pAlpha[rgbSrc.GetRed()];
	BYTE byGreenResult = pAlphaInv[rgbDest.GetGreen()] + pAlpha[rgbSrc.GetGreen()];
	BYTE byBlueResult = pAlphaInv[rgbDest.GetBlue()] + pAlpha[rgbSrc.GetBlue()];

	return CRGBA32(byRedResult, byGreenResult, byBlueResult);
	}

CRGBA32 CRGBA32::Blend (CRGBA32 rgbDest, CRGBA32 rgbSrc, BYTE bySrcAlpha)

//	Blend
//
//	Combines rgbSrc with rgbDest, using bySrcAlpha as the source opacity.
//	We assume source and dest have no alpha component.

	{
	BYTE *pAlpha = g_Alpha8[bySrcAlpha];
	BYTE *pAlphaInv = g_Alpha8[255 - bySrcAlpha];

	BYTE byRedResult = pAlphaInv[rgbDest.GetRed()] + pAlpha[rgbSrc.GetRed()];
	BYTE byGreenResult = pAlphaInv[rgbDest.GetGreen()] + pAlpha[rgbSrc.GetGreen()];
	BYTE byBlueResult = pAlphaInv[rgbDest.GetBlue()] + pAlpha[rgbSrc.GetBlue()];

	return CRGBA32(byRedResult, byGreenResult, byBlueResult);
	}

CRGBA32 CRGBA32::Composite (CRGBA32 rgbDest, CRGBA32 rgbSrc)

//	Composite
//
//	Combines two pixels, preserving alpha

	{
	BYTE bySrcAlpha = rgbSrc.GetAlpha();
	BYTE byDestAlpha = rgbDest.GetAlpha();

	if (bySrcAlpha == 0)
		return rgbDest;
	else if (bySrcAlpha == 0xff)
		return rgbSrc;
	else
		{
		BYTE *pAlpha = CRGBA32::AlphaTable(bySrcAlpha);	//	Equivalent to 255 - byAlpha
		BYTE *pAlphaInv = CRGBA32::AlphaTable(bySrcAlpha ^ 0xff);	//	Equivalent to 255 - byAlpha

		BYTE byRedResult = (BYTE)Min((WORD)0xff, (WORD)(pAlphaInv[rgbDest.GetRed()] + (WORD)pAlpha[rgbSrc.GetRed()]));
		BYTE byGreenResult = (BYTE)Min((WORD)0xff, (WORD)(pAlphaInv[rgbDest.GetGreen()] + (WORD)pAlpha[rgbSrc.GetGreen()]));
		BYTE byBlueResult = (BYTE)Min((WORD)0xff, (WORD)(pAlphaInv[rgbDest.GetBlue()] + (WORD)pAlpha[rgbSrc.GetBlue()]));

		return CRGBA32(byRedResult, byGreenResult, byBlueResult, CRGBA32::CompositeAlpha(byDestAlpha, bySrcAlpha));
		}
	}

CRGBA32 CRGBA32::FromHSB (int iHue, int iSaturation, int iBrightness, BYTE byAlpha)

//	FromHSB
//
//	Creates a pixel from Hue, Saturation, Brightness.
//	Hue: 0-359
//	Sat: 0-100
//	Bri: 0-100

	{
	return FromHSB((double)iHue, iSaturation / 100.0, iBrightness / 100.0, byAlpha);
	}

CRGBA32 CRGBA32::FromHSB (double rHue, double rSaturation, double rBrightness, BYTE byAlpha)

//	FromHSB
//
//	Creates a pixel from Hue, Saturation, Brightness.
//	Hue: 0-359
//	Sat: 0-100
//	Bri: 0-100

	{
	if (rSaturation <= 0.0)
		{
		BYTE byValue = (BYTE)round(rBrightness * 255.0);
		return CRGBA32(byValue, byValue, byValue, byAlpha);
		}
	else
		{
		Metric rH = (rHue >= 360.0 ? 0.0 : rHue) / 60.0;
		Metric rI = floor(rH);
		Metric rF = rH - rI;
		Metric rP = rBrightness * (1.0 - rSaturation);
		Metric rQ = rBrightness * (1.0 - rSaturation * rF);
		Metric rT = rBrightness * (1.0 - rSaturation * (1.0 - rF));

		switch ((int)rI)
			{
			case 0:
				return FromReal(rBrightness, rT, rP, byAlpha);

			case 1:
				return FromReal(rQ, rBrightness, rP, byAlpha);

			case 2:
				return FromReal(rP, rBrightness, rT, byAlpha);

			case 3:
				return FromReal(rP, rQ, rBrightness, byAlpha);

			case 4:
				return FromReal(rT, rP, rBrightness, byAlpha);

			case 5:
				return FromReal(rBrightness, rP, rQ, byAlpha);

			default:
				return FromReal(0.0, 0.0, 0.0, byAlpha);
			}
		}
	}

bool CRGBA32::InitTables (void)

//	InitTables
//
//	Initializes alpha tables for blending.

	{
	DWORD i, j;

	//	Compute alpha table

	for (i = 0; i < 256; i++)
		for (j = 0; j < 256; j++)
			g_Alpha8[j][i] = (BYTE)((DWORD)((i * (j / 255.0f)) + 0.5));

	//	Compute screen table

#if 0
	for (i = 0; i < 256; i++)
		for (j = 0; j < 256; j++)
			g_Screen8[j][i] = (BYTE)(0xff - g_Alpha8[0xff - i][0xff - j]);
#endif

	return true;
	}

CRGBA32 CRGBA32::Parse (const CString &sValue, bool *retbFail)

//	Parse
//
//	Parse from a string. We support the following formats:
//
//	#rrggbb
//	rgb(r, g, b)
//	rgb(r, g, b, a)

	{
	const char *pPos = sValue.GetParsePointer();
	const char *pPosEnd = pPos + sValue.GetLength();

	if (*pPos == '#')
		{
		pPos++;

		DWORD dwHex = strParseIntOfBase(pPos, 16, 0);
		if (retbFail) *retbFail = false;

		return CRGBA32((dwHex >> 16) & 0xff, (dwHex >> 8) & 0xff, dwHex & 0xff);
		}
	else if (*pPos == 'r' || *pPos == 'R')
		{
		pPos++;
		if (*pPos != 'g' && *pPos != 'G')
			{
			if (retbFail) *retbFail = true;
			return CRGBA32(0, 0, 0);
			}

		pPos++;
		if (*pPos != 'b' && *pPos != 'B')
			{
			if (retbFail) *retbFail = true;
			return CRGBA32(0, 0, 0);
			}

		pPos++;
		if (*pPos == 'a' || *pPos == 'A')
			pPos++;

		if (*pPos != '(')
			{
			if (retbFail) *retbFail = true;
			return CRGBA32(0, 0, 0);
			}

		pPos++;
		int iRed = Max(0, Min(strParseInt(pPos, 0, &pPos), 255));
		while (*pPos == ' ') pPos++;
		if (*pPos == ',')
			pPos++;

		int iGreen = Max(0, Min(strParseInt(pPos, 0, &pPos), 255));
		while (*pPos == ' ') pPos++;
		if (*pPos == ',')
			pPos++;

		int iBlue = Max(0, Min(strParseInt(pPos, 0, &pPos), 255));
		while (*pPos == ' ') pPos++;

		int iAlpha = 255;
		if (*pPos == ',')
			{
			pPos++;

			double rAlpha = strParseDouble(pPos, 1.0, &pPos);
			if (rAlpha > 1.0)
				iAlpha = Min((int)rAlpha, 255);
			else
				iAlpha = (int)(Max(0.0, rAlpha) * 255.0);
			}

		if (retbFail) *retbFail = false;

		return CRGBA32(iRed, iGreen, iBlue, iAlpha);
		}
	else if (strIsHexNumber(pPos, (int)(pPosEnd - pPos)))
		{
		if (pPosEnd - pPos >= 8)
			{
			DWORD dwHex = strParseIntOfBase(pPos, 16, 0);
			if (retbFail) *retbFail = false;

			return CRGBA32((dwHex >> 16) & 0xff, (dwHex >> 8) & 0xff, dwHex & 0xff, (dwHex >> 24) & 0xff);
			}
		else if (pPosEnd - pPos >= 6)
			{
			DWORD dwHex = strParseIntOfBase(pPos, 16, 0);
			if (retbFail) *retbFail = false;

			return CRGBA32((dwHex >> 16) & 0xff, (dwHex >> 8) & 0xff, dwHex & 0xff);
			}
		else
			{
			if (retbFail) *retbFail = true;
			return CRGBA32(0, 0, 0);
			}
		}
	else
		{
		if (retbFail) *retbFail = true;
		return CRGBA32(0, 0, 0);
		}
	}

CRGBA32 CRGBA32::ParseHTMLColor (const CString& sValue, bool* retbFail)
	{
	if (auto *pColor = m_HTMLColors.GetAt(strToLower(sValue)))
		{
		if (retbFail) *retbFail = false;
		return *pColor;
		}
	else
		{
		if (retbFail) *retbFail = true;
		return CRGBA32(0, 0, 0);
		}
	}
