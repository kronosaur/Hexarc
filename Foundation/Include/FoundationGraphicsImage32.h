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

		static CRGBA32 Null (void) { return CRGBA32(0, true); }

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

//	CRGBA32Image
//
//	This class manages a 32-bit image. Each pixel has the following bit-pattern:
//
//	33222222 22221111 11111100 00000000
//	10987654 32109876 54321098 76543210
//
//	AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB

class CRGBA32Image : public CImagePlane
	{
	public:
		enum EAlphaTypes
			{
			alphaNone,						//	Ignore alpha byte
			alpha1,							//	1-bit alpha (non-zero = opaque)
			alpha8,							//	8-bit alpha
			};

		enum EFlags
			{
			//	CreateFromBitmap

			FLAG_PRE_MULT_ALPHA =			0x00000001,
			};

		CRGBA32Image (void) { }
		CRGBA32Image (const CRGBA32Image &Src) { Copy(Src); }
		CRGBA32Image (CRGBA32Image &&Src) noexcept;
		virtual ~CRGBA32Image (void) { CleanUp(); }

		CRGBA32Image &operator= (const CRGBA32Image &Src) { CleanUp(); Copy(Src); return *this; }

		static CRGBA32Image &Null (void) { return m_NullImage; }

		//	Basic Interface

		void CleanUp (void);
		bool Create (int cxWidth, int cyHeight, EAlphaTypes AlphaType = alphaNone);
		bool Create (int cxWidth, int cyHeight, EAlphaTypes AlphaType, CRGBA32 InitialValue);
		EAlphaTypes GetAlphaType (void) const { return m_AlphaType; }
		C8bitImage GetChannel (EChannels iChannel) const;
		CRGBA32 GetPixel (int x, int y) const { return *GetPixelPos(x, y); }
		CRGBA32 *GetPixelPos (int x, int y) const { return (CRGBA32 *)((BYTE *)m_pRGBA + (y * m_iPitch)) + x; }
		CRGBA32 *GetPointer (void) const { return m_pRGBA; }
		bool IsEmpty (void) const { return (m_pRGBA == NULL); }
		bool IsMarked (void) const { return m_bMarked; }
		CRGBA32 *NextRow (CRGBA32 *pPos) const { return (CRGBA32 *)((BYTE *)pPos + m_iPitch); }
		void SetMarked (bool bMarked = true) { m_bMarked = bMarked; }

		//	Windows

		void BltToDC (HDC hDC, int x, int y) const;
		bool CopyToClipboard (void) const;
		bool WriteToWindowsBMP (IByteStream &Stream, CString *retsError = NULL) const;

	private:
		static int CalcBufferSize (int cxWidth, int cyHeight) { return (cxWidth * cyHeight); }
		void Copy (const CRGBA32Image &Src);
		void InitBMI (BITMAPINFO **retpbi) const;

		CRGBA32 *m_pRGBA = NULL;
		bool m_bFreeRGBA = false;				//	If TRUE, we own the memory
		bool m_bMarked = false;					//	Mark/sweep flag (for use by caller)
		int m_iPitch = 0;						//	Bytes per row
		EAlphaTypes m_AlphaType = alphaNone;

		mutable BITMAPINFO *m_pBMI = NULL;		//	Used for blting to a DC

		static CRGBA32Image m_NullImage;
	};
