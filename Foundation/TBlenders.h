//	TBlenders.h
//
//	Templates for various pixel blending algorithms.
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.

#pragma once

template <class BLENDER> class TBlendImpl
	{
	public:
		__forceinline static CRGBA32 BlendAlpha (CRGBA32 rgbDest, CRGBA32 rgbSource, BYTE byAlpha) { return BLENDER::Blend(rgbDest, CRGBA32(rgbSource, CRGBA32::BlendAlpha(byAlpha, rgbSource.GetAlpha()))); }

		__forceinline static void SetBlend (CRGBA32 *pDest, CRGBA32 rgbSource) { *pDest = BLENDER::Blend(*pDest, rgbSource); }
		__forceinline static void SetBlendAlpha (CRGBA32 *pDest, CRGBA32 rgbSource, BYTE byAlpha) { *pDest = BLENDER::BlendAlpha(*pDest, rgbSource, byAlpha); }
		__forceinline static void SetBlendPreMult (CRGBA32 *pDest, CRGBA32 rgbSource) { *pDest = BLENDER::BlendPreMult(*pDest, rgbSource); }
		__forceinline static void SetCopy (CRGBA32 *pDest, CRGBA32 rgbSource) { *pDest = BLENDER::Copy(*pDest, rgbSource); }
	};

//	CBlendBlend
//
//	Use this blending mode when the source pixel could have transparency which
//	we need to honor.

class CBlendBlend : public TBlendImpl<CBlendBlend>
	{
	public:
		inline static CRGBA32 Blend (CRGBA32 rgbDest, CRGBA32 rgbSource) { return CRGBA32::Blend(rgbDest, rgbSource); }

		inline static CRGBA32 BlendPreMult (CRGBA32 rgbDest, CRGBA32 rgbSource) 
			{
			BYTE *pAlphaInv = CRGBA32::AlphaTable(rgbSource.GetAlpha() ^ 0xff);	//	Equivalent to 255 - rgbSrc.GetAlpha()

			BYTE byRedResult = pAlphaInv[rgbDest.GetRed()] + rgbSource.GetRed();
			BYTE byGreenResult = pAlphaInv[rgbDest.GetGreen()] + rgbSource.GetGreen();
			BYTE byBlueResult = pAlphaInv[rgbDest.GetBlue()] + rgbSource.GetBlue();

			return CRGBA32(byRedResult, byGreenResult, byBlueResult);
			}

		inline static CRGBA32 Copy (CRGBA32 rgbDest, CRGBA32 rgbSource) { return rgbSource; }
	};

//	CBlendCopy
//
//	Use this blending mode when we want to treat the source pixel as a solid 
//	color (i.e., ignore any alpha channel in the source).

class CBlendCopy : public TBlendImpl<CBlendCopy>
	{
	public:
		inline static CRGBA32 Blend (CRGBA32 rgbDest, CRGBA32 rgbSource) { return rgbSource; }
		inline static CRGBA32 BlendAlpha (CRGBA32 rgbDest, CRGBA32 rgbSource, BYTE byAlpha) { return CRGBA32::Blend(rgbDest, CRGBA32(rgbSource, byAlpha)); }
		inline static CRGBA32 BlendPreMult (CRGBA32 rgbDest, CRGBA32 rgbSource) { return rgbSource; }
		inline static CRGBA32 Copy (CRGBA32 rgbDest, CRGBA32 rgbSource) { return rgbSource; }
	};
