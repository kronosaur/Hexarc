//	FoundationGraphicsDraw.h
//
//	Foundation header file
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CImageDraw
	{
	public:
		static void Blt (CRGBA32Image &Dest, int xDest, int yDest, const CRGBA32Image &Src, int xSrc = 0, int ySrc = 0, int cxSrc = -1, int cySrc = -1, EBlendModes iBlend = blendNormal);
		static void BltScaled (CRGBA32Image &Dest, int xDest, int yDest, int cxDest, int cyDest, const CRGBA32Image &Src, int xSrc = 0, int ySrc = 0, int cxSrc = -1, int cySrc = -1, EBlendModes iBlend = blendNormal);
		static void Copy (CRGBA32Image &Dest, int xDest, int yDest, const CRGBA32Image &Src, int xSrc = 0, int ySrc = 0, int cxSrc = -1, int cySrc = -1);
		static void CopyScaled (CRGBA32Image &Dest, int xDest, int yDest, int cxDest, int cyDest, const CRGBA32Image &Src, int xSrc = 0, int ySrc = 0, int cxSrc = -1, int cySrc = -1);
		static void Rectangle (CRGBA32Image &Dest, int xDest, int yDest, int cxDest, int cyDest, CRGBA32 rgbColor, EBlendModes iBlend = blendNormal);
	};
