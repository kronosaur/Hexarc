//	CRGBA32Image.cpp
//
//	CRGBA32Image class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CRGBA32Image CRGBA32Image::m_NullImage;

CRGBA32Image::CRGBA32Image (CRGBA32Image &&Src) noexcept :
		CImagePlane(Src),
		m_pRGBA(Src.m_pRGBA),
		m_bFreeRGBA(Src.m_bFreeRGBA),
		m_bMarked(Src.m_bMarked),
		m_iPitch(Src.m_iPitch),
		m_AlphaType(Src.m_AlphaType)

//	CRGBA32Image move constructor

	{
	Src.m_pRGBA = NULL;
	Src.m_bFreeRGBA = false;
	}

void CRGBA32Image::BltToDC (HDC hDC, int x, int y) const

//	BltToDC
//
//	Blt the image to a DC

	{
	BITMAPINFO *pbmi;
	InitBMI(&pbmi);

	::SetDIBitsToDevice(hDC,
			x,
			y,
			m_cxWidth,
			m_cyHeight,
			0,
			0,
			0,
			m_cyHeight,
			m_pRGBA,
			pbmi,
			DIB_RGB_COLORS);

	delete [] (BYTE *)pbmi;
	}

void CRGBA32Image::CleanUp (void)

//	CleanUp
//
//	Clean up the bitmap

	{
	if (m_pRGBA && m_bFreeRGBA)
		delete [] m_pRGBA;

	m_pRGBA = NULL;
	m_cxWidth = 0;
	m_cyHeight = 0;
	m_iPitch = 0;
	m_AlphaType = alphaNone;
	ResetClipRect();

	if (m_pBMI)
		{
		delete [] (BYTE *)m_pBMI;
		m_pBMI = NULL;
		}
	}

void CRGBA32Image::Copy (const CRGBA32Image &Src)

//	Copy
//
//	Copy from source. We assume that we are already clean.

	{
	if (Src.IsEmpty())
		return;

	//	Copy the buffer

	int iSize = CalcBufferSize(Src.m_iPitch / sizeof(DWORD), Src.m_cyHeight);
	m_pRGBA = new CRGBA32 [iSize];
	m_bFreeRGBA = true;
	m_bMarked = Src.m_bMarked;

	CRGBA32 *pSrc = Src.m_pRGBA;
	CRGBA32 *pSrcEnd = pSrc + iSize;
	CRGBA32 *pDest = m_pRGBA;

	while (pSrc < pSrcEnd)
		*pDest++ = *pSrc++;

	//	Now copy the remaining variable

	m_cxWidth = Src.m_cxWidth;
	m_cyHeight = Src.m_cyHeight;
	m_iPitch = Src.m_iPitch;
	m_AlphaType = Src.m_AlphaType;
	m_rcClip = Src.m_rcClip;
	m_pBMI = NULL;
	}

bool CRGBA32Image::CopyToClipboard (void) const

//	CopyToClipboard
//
//	Copy the image to the clipboard

	{
	//	Create an HBITMAP

	HWND hDesktopWnd = ::GetDesktopWindow();
	HDC hDesktopDC = ::GetDC(hDesktopWnd);
	HDC hDC = ::CreateCompatibleDC(hDesktopDC);
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDesktopDC, m_cxWidth, m_cyHeight);
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hDC, hBitmap);
	BltToDC(hDC, 0, 0);
	::SelectObject(hDC, hOldBitmap);
	::DeleteDC(hDC);
	::ReleaseDC(hDesktopWnd, hDesktopDC);

	//	Copy to the clipboard

	if (!::OpenClipboard(NULL))
		{
		::DeleteObject(hBitmap);
		return false;
		}

	if (!::EmptyClipboard())
		{
		::CloseClipboard();
		::DeleteObject(hBitmap);
		return false;
		}

	if (!::SetClipboardData(CF_BITMAP, hBitmap))
		{
		::CloseClipboard();
		::DeleteObject(hBitmap);
		return false;
		}

	if (!::CloseClipboard())
		return false;

	return true;
	}

bool CRGBA32Image::Create (int cxWidth, int cyHeight, EAlphaTypes AlphaType)

//	Create
//
//	Creates an image; the bits are uninitialized.

	{
	CleanUp();
	if (cxWidth <= 0 || cyHeight <= 0)
		return false;

	//	Allocate a new buffer

	m_iPitch = cxWidth * sizeof(DWORD);
	int iSize = CalcBufferSize(m_iPitch / sizeof(DWORD), cyHeight);
	m_pRGBA = new CRGBA32 [iSize];
	m_bFreeRGBA = true;

	//	Other variables

	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	m_AlphaType = AlphaType;
	ResetClipRect();

	return true;
	}

bool CRGBA32Image::Create (int cxWidth, int cyHeight, EAlphaTypes AlphaType, CRGBA32 InitialValue)

//	Create
//
//	Creates a blank image

	{
	if (!Create(cxWidth, cyHeight, AlphaType))
		return false;

	//	Initialize

	int iSize = CalcBufferSize(m_iPitch / sizeof(DWORD), cyHeight);
	CRGBA32 *pDest = m_pRGBA;
	CRGBA32 *pDestEnd = pDest + iSize;

	while (pDest < pDestEnd)
		*pDest++ = InitialValue;

	return true;
	}

C8bitImage CRGBA32Image::GetChannel (EChannels iChannel) const

//	GetChannel
//
//	Creates an 8-bit image based on the channel.

	{
	C8bitImage Channel;

	if (IsEmpty())
		return Channel;

	if (!Channel.Create(GetWidth(), GetHeight()))
		return Channel;

	CRGBA32 *pSrcRow = GetPixelPos(0, 0);
	CRGBA32 *pSrcRowEnd = GetPixelPos(0, GetHeight());
	BYTE *pDestRow = Channel.GetPixelPos(0, 0);

	switch (iChannel)
		{
		case channelAlpha:
			{
			while (pSrcRow < pSrcRowEnd)
				{
				CRGBA32 *pSrc = pSrcRow;
				CRGBA32 *pSrcEnd = pSrcRow + GetWidth();
				BYTE *pDest = pDestRow;

				while (pSrc < pSrcEnd)
					{
					*pDest++ = pSrc->GetAlpha();
					pSrc++;
					}

				pSrcRow = NextRow(pSrcRow);
				pDestRow = Channel.NextRow(pDestRow);
				}

			break;
			}

		case channelRed:
			{
			while (pSrcRow < pSrcRowEnd)
				{
				CRGBA32 *pSrc = pSrcRow;
				CRGBA32 *pSrcEnd = pSrcRow + GetWidth();
				BYTE *pDest = pDestRow;

				while (pSrc < pSrcEnd)
					{
					*pDest++ = pSrc->GetRed();
					pSrc++;
					}

				pSrcRow = NextRow(pSrcRow);
				pDestRow = Channel.NextRow(pDestRow);
				}

			break;
			}

		case channelGreen:
			{
			while (pSrcRow < pSrcRowEnd)
				{
				CRGBA32 *pSrc = pSrcRow;
				CRGBA32 *pSrcEnd = pSrcRow + GetWidth();
				BYTE *pDest = pDestRow;

				while (pSrc < pSrcEnd)
					{
					*pDest++ = pSrc->GetGreen();
					pSrc++;
					}

				pSrcRow = NextRow(pSrcRow);
				pDestRow = Channel.NextRow(pDestRow);
				}

			break;
			}

		case channelBlue:
			{
			while (pSrcRow < pSrcRowEnd)
				{
				CRGBA32 *pSrc = pSrcRow;
				CRGBA32 *pSrcEnd = pSrcRow + GetWidth();
				BYTE *pDest = pDestRow;

				while (pSrc < pSrcEnd)
					{
					*pDest++ = pSrc->GetBlue();
					pSrc++;
					}

				pSrcRow = NextRow(pSrcRow);
				pDestRow = Channel.NextRow(pDestRow);
				}

			break;
			}

		default:
			ASSERT(false);
			break;
		}

	return Channel;
	}

void CRGBA32Image::InitBMI (BITMAPINFO **retpbi) const

//	InitBMP
//
//	Initializes the m_pBMI structure

	{
	BITMAPINFO *pbmi = (BITMAPINFO *)(new BYTE [sizeof(BITMAPINFO) + 2 * sizeof(DWORD)]);
	utlMemSet(pbmi, sizeof(BITMAPINFO), 0);

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = m_cxWidth;
	pbmi->bmiHeader.biHeight = -m_cyHeight;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 32;
	pbmi->bmiHeader.biCompression = BI_RGB;

	//	Done

	*retpbi = pbmi;
	}

bool CRGBA32Image::WriteToWindowsBMP (IByteStream &Stream, CString *retsError) const

//	WriteToWindowsBMP
//
//	Write BMP format to the stream.

	{
	//	Compute the total size of the image data (in bytes)

	int iColorTable = 0;
	int iImageDataSize = m_cyHeight * m_cxWidth * sizeof(DWORD);

	BITMAPFILEHEADER header;
	header.bfType = 'MB';
	header.bfOffBits = sizeof(header) + sizeof(BITMAPINFOHEADER) + iColorTable;
	header.bfSize = header.bfOffBits + iImageDataSize;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	Stream.Write(&header, sizeof(header));

	BITMAPINFO *pbmi;
	InitBMI(&pbmi);

	//	Bottom up
	pbmi->bmiHeader.biHeight = -pbmi->bmiHeader.biHeight;

	Stream.Write(pbmi, sizeof(BITMAPINFOHEADER) + iColorTable);

	//	Write the bits bottom-up

	for (int y = m_cyHeight - 1; y >= 0; y--)
		{
		DWORD *pRow = (DWORD *)GetPixelPos(0, y);
		Stream.Write(pRow, m_cxWidth * sizeof(DWORD));
		}

	delete [] (BYTE *)pbmi;
	return true;
	}
