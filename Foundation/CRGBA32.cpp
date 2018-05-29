//	CRGBA32.cpp
//
//	CRGBA32 class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CRGBA32::AlphaArray8 CRGBA32::g_Alpha8 [256];
bool CRGBA32::m_bAlphaInitialized = CRGBA32::InitTables();

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
