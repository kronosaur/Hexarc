//	FoundationGraphicsImage8.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

//	C8bitImage

class C8bitImage : public CImagePlane
	{
	public:
		C8bitImage (void);
		C8bitImage (const C8bitImage &Src) { Copy(Src); }
		virtual ~C8bitImage (void) { CleanUp(); }

		inline C8bitImage &operator= (const C8bitImage &Src) { CleanUp(); Copy(Src); return *this; }

		inline static C8bitImage &Null (void) { return m_NullImage; }

		//	Basic Interface

		void CleanUp (void);
		bool Create (int cxWidth, int cyHeight);
		bool Create (int cxWidth, int cyHeight, BYTE byInitialValue);
		inline BYTE GetPixel (int x, int y) const { return *GetPixelPos(x, y); }
		inline BYTE *GetPixelPos (int x, int y) const { return (m_pChannel + (y * m_iPitch)) + x; }
		inline BYTE *GetPointer (void) const { return m_pChannel; }
		inline bool IsEmpty (void) const { return (m_pChannel == NULL); }
		inline bool IsMarked (void) const { return m_bMarked; }
		inline BYTE *NextRow (BYTE *pPos) const { return (pPos + m_iPitch); }
		inline void SetMarked (bool bMarked = true) { m_bMarked = bMarked; }
		bool WriteToWindowsBMP (IByteStream &Stream, CString *retsError = NULL) const;

	private:
		static int CalcBufferSize (int cxWidth, int cyHeight) { return (cxWidth * cyHeight); }
		void Copy (const C8bitImage &Src);
		void InitBMI (BITMAPINFO **retpbi) const;

		BYTE *m_pChannel;
		bool m_bFree;						//	If TRUE, we own the memory
		bool m_bMarked;						//	Mark/sweep flag (for use by caller)
		int m_iPitch;						//	Bytes per row

		mutable BITMAPINFO *m_pBMI;			//	Used for blting to a DC

		static C8bitImage m_NullImage;
	};

