//	FoundationGraphicsImage32.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CRGBA32
	{
	public:
		//	No default initialization for performance.

		CRGBA32 (void) { }

		CRGBA32 (const CRGBA32 &Src) : m_dwPixel(Src.m_dwPixel) { }

		explicit CRGBA32 (DWORD dwPixel) : m_dwPixel(dwPixel)
			{ }

		CRGBA32 (BYTE byRed, BYTE byGreen, BYTE byBlue)
			{ m_dwPixel = 0xff000000 | ((DWORD)byRed << 16) | ((DWORD)byGreen << 8) | (DWORD)byBlue; }

		CRGBA32 (BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
			{ m_dwPixel = ((DWORD)byAlpha << 24) | ((DWORD)byRed << 16) | ((DWORD)byGreen << 8) | (DWORD)byBlue; }

		CRGBA32 (const CRGBA32 &Src, BYTE byNewAlpha)
			{ m_dwPixel = (byNewAlpha << 24) | (Src.m_dwPixel & 0x00ffffff); }

		operator const COLORREF () { return RGB(GetRed(), GetGreen(), GetBlue()); }

		bool operator == (const CRGBA32 &vA) const { return (m_dwPixel == vA.m_dwPixel); }
		bool operator != (const CRGBA32 &vA) const { return (m_dwPixel != vA.m_dwPixel); }

		DWORD AsDWORD (void) const { return m_dwPixel; }
		DWORD AsR5G5B5 (void) const { return (((m_dwPixel & 0x00f80000) >> 9) | ((m_dwPixel & 0x0000f800) >> 6) | ((m_dwPixel & 0x000000f8) >> 3)); }
		DWORD AsR5G6B5 (void) const { return (((m_dwPixel & 0x00f80000) >> 8) | ((m_dwPixel & 0x0000fc00) >> 5) | ((m_dwPixel & 0x000000f8) >> 3)); }
		DWORD AsR8G8B8 (void) const { return m_dwPixel; }
		CString AsHTMLColor () const;
		BYTE GetAlpha (void) const { return (BYTE)((m_dwPixel & 0xff000000) >> 24); }
		BYTE GetBlue (void) const { return (BYTE)(m_dwPixel & 0x000000ff); }
		BYTE GetGreen (void) const { return (BYTE)((m_dwPixel & 0x0000ff00) >> 8); }
		BYTE GetMax (void) const { return Max(Max(GetRed(), GetGreen()), GetBlue()); }
		BYTE GetRed (void) const { return (BYTE)((m_dwPixel & 0x00ff0000) >> 16); }
		bool IsNull (void) const { return (m_dwPixel == 0); }
		void SetAlpha (BYTE byValue) { m_dwPixel = (m_dwPixel & 0x00ffffff) | ((DWORD)byValue << 24); }
		void SetBlue (BYTE byValue) { m_dwPixel = (m_dwPixel & 0xffffff00) | (DWORD)byValue; }
		void SetGreen (BYTE byValue) { m_dwPixel = (m_dwPixel & 0xffff00ff) | ((DWORD)byValue << 8); }
		void SetRed (BYTE byValue) { m_dwPixel = (m_dwPixel & 0xff00ffff) | ((DWORD)byValue << 16); }

		static BYTE *AlphaTable (BYTE byOpacity) { return g_Alpha8[byOpacity]; }

		//  Single channel operations

		static BYTE BlendAlpha (BYTE byDest, BYTE bySrc) { return g_Alpha8[byDest][bySrc]; }
		static BYTE CompositeAlpha (BYTE byDest, BYTE bySrc) { return (BYTE)255 - (BYTE)(((DWORD)(255 - byDest) * (DWORD)(255 - bySrc)) / 255); }

		//  Creates pixels

		static CRGBA32 FromHSB (int iHue, int iSaturation, int iBrightness, BYTE byAlpha = 0xff);
		static CRGBA32 FromHSB (double rHue, double rSaturation, double rBrightness, BYTE byAlpha = 0xff);
		static CRGBA32 FromReal (double rRed, double rGreen, double rBlue, BYTE byAlpha = 0xff)
			{ return CRGBA32((BYTE)(rRed * 255.0), (BYTE)(rGreen * 255.0), (BYTE)(rBlue * 255.0), byAlpha); }
		static CRGBA32 Null (void) { return CRGBA32(0, true); }
		static CRGBA32 Parse (const CString &sValue);

		//  Expects solid pixels, and always returns solid pixels

		static CRGBA32 Blend (CRGBA32 rgbDest, CRGBA32 rgbSrc);
		static CRGBA32 Blend (CRGBA32 rgbDest, CRGBA32 rgbSrc, BYTE bySrcAlpha);

		//  Works on transparent pixels

		static CRGBA32 Composite (CRGBA32 rgbDest, CRGBA32 rgbSrc);

		static bool InitTables (void);

	private:
		CRGBA32 (DWORD dwValue, bool bRaw) : m_dwPixel(dwValue) { }

		DWORD m_dwPixel;

		typedef BYTE AlphaArray8 [256];
		static AlphaArray8 g_Alpha8 [256];
		static bool m_bAlphaInitialized;
	};

