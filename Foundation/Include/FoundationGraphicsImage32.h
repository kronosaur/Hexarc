//	FoundationGraphicsImage32.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

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

		bool operator== (const CRGBA32Image &Src) const;
		bool operator!= (const CRGBA32Image &Src) const { return !(*this == Src); }
		CRGBA32Image &operator= (const CRGBA32Image &Src) { CleanUp(); Copy(Src); return *this; }
		CRGBA32Image &operator= (CRGBA32Image &&Src) noexcept;

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
