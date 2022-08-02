//	CImageDraw.cpp
//
//	CImageDraw class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

void CImageDraw::Blt (CRGBA32Image &Dest, int xDest, int yDest, const CRGBA32Image &Src, int xSrc, int ySrc, int cxSrc, int cySrc, EBlendModes iBlend)

//	Blt
//
//	Blt from one image to another.

	{
	switch (iBlend)
		{
		case blendNormal:
			{
			TSolidImageBlt<CBlendBlend> Algorithm;
			Algorithm.Blt(Dest, xDest, yDest, Src, xSrc, ySrc, cxSrc, cySrc);
			break;
			}

		default:
			ASSERT(false);
			break;
		}
	}

void CImageDraw::BltScaled (CRGBA32Image &Dest, int xDest, int yDest, int cxDest, int cyDest, const CRGBA32Image &Src, int xSrc, int ySrc, int cxSrc, int cySrc, EBlendModes iBlend)

//	BltScaled
//
//	Blt scaled.

	{
	switch (iBlend)
		{
		case blendNormal:
			{
			TSolidImageBlt<CBlendBlend> Algorithm;
			Algorithm.BltScaled(Dest, xDest, yDest, cxDest, cyDest, Src, xSrc, ySrc, cxSrc, cySrc);
			break;
			}

		default:
			ASSERT(false);
			break;
		}
	}

void CImageDraw::Copy (CRGBA32Image &Dest, int xDest, int yDest, const CRGBA32Image &Src, int xSrc, int ySrc, int cxSrc, int cySrc)

//	Copy
//
//	Copy pixels from one image to another (no blending).

	{
	TSolidImageBlt<CBlendCopy> Algorithm;
	Algorithm.Copy(Dest, xDest, yDest, Src, xSrc, ySrc, cxSrc, cySrc);
	}

void CImageDraw::CopyScaled (CRGBA32Image &Dest, int xDest, int yDest, int cxDest, int cyDest, const CRGBA32Image &Src, int xSrc, int ySrc, int cxSrc, int cySrc)

//	CopyScaled
//
//	Copy pixels from one image to another

	{
	TSolidImageBlt<CBlendCopy> Algorithm;
	Algorithm.CopyScaled(Dest, xDest, yDest, cxDest, cyDest, Src, xSrc, ySrc, cxSrc, cySrc);
	}

void CImageDraw::Rectangle (CRGBA32Image &Dest, int xDest, int yDest, int cxDest, int cyDest, CRGBA32 rgbColor, EBlendModes iBlend)

//	Rectangle
//
//	Draw a filled rectangle

	{
	CPainterSolid Fill(rgbColor);

	switch (iBlend)
		{
		case blendNormal:
			{
			TSolidImageDraw<CBlendBlend> Algorithm(Fill);
			Algorithm.Rectangle(Dest, xDest, yDest, cxDest, cyDest);
			break;
			}

		default:
			ASSERT(false);
			break;
		}
	}
