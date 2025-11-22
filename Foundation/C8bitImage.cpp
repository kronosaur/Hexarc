//	C8bitImage.cpp
//
//	C8bitImage class
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

C8bitImage::C8bitImage (void) :
		m_pChannel(NULL),
		m_bFree(false),
		m_bMarked(false),
		m_iPitch(0),
		m_pBMI(NULL)

//	C8bitImage constructor

	{
	}

void C8bitImage::CleanUp (void)

//	CleanUp
//
//	Delete

	{
	if (m_pChannel && m_bFree)
		delete [] m_pChannel;

	m_pChannel = NULL;
	m_cxWidth = 0;
	m_cyHeight = 0;
	m_iPitch = 0;
	ResetClipRect();

	if (m_pBMI)
		{
		delete [] (BYTE *)m_pBMI;
		m_pBMI = NULL;
		}
	}

void C8bitImage::Copy (const C8bitImage &Src)

//	Copy
//
//	Copy from source. We assume that we are already clean.

	{
	if (Src.IsEmpty())
		return;

	//	Copy the buffer

	int iSize = CalcBufferSize(Src.m_iPitch, Src.m_cyHeight);
	m_pChannel = new BYTE [iSize];
	m_bFree = true;
	m_bMarked = Src.m_bMarked;

	utlMemCopy(Src.m_pChannel, m_pChannel, iSize);

	//	Now copy the remaining variable

	m_cxWidth = Src.m_cxWidth;
	m_cyHeight = Src.m_cyHeight;
	m_iPitch = Src.m_iPitch;
	m_rcClip = Src.m_rcClip;
	m_pBMI = NULL;
	}

bool C8bitImage::Create (int cxWidth, int cyHeight)

//	Create
//
//	Creates an image; the bits are uninitialized.

	{
	CleanUp();
	if (cxWidth <= 0 || cyHeight <= 0)
		return false;

	//	Allocate a new buffer

	m_iPitch = cxWidth;
	int iSize = CalcBufferSize(m_iPitch, cyHeight);
	m_pChannel = new BYTE [iSize];
	m_bFree = true;

	//	Other variables

	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	ResetClipRect();

	return true;
	}

bool C8bitImage::Create (int cxWidth, int cyHeight, BYTE byInitialValue)

//	Create
//
//	Creates a blank image

	{
	if (!Create(cxWidth, cyHeight))
		return false;

	//	Initialize

	int iSize = CalcBufferSize(m_iPitch, cyHeight);
	utlMemSet(m_pChannel, iSize, byInitialValue);

	return true;
	}

void C8bitImage::InitBMI (BITMAPINFO **retpbi) const

//	InitBMP
//
//	Initializes the m_pBMI structure

	{
	int i;

	BITMAPINFO *pbmi = (BITMAPINFO *)(new BYTE [sizeof(BITMAPINFO) + 255 * sizeof(DWORD)]);
	utlMemSet(pbmi, sizeof(BITMAPINFO), 0);

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = m_cxWidth;
	pbmi->bmiHeader.biHeight = -m_cyHeight;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 8;
	pbmi->bmiHeader.biCompression = BI_RGB;

	for (i = 0; i < 256; i++)
		{
		pbmi->bmiColors[i].rgbRed = i;
		pbmi->bmiColors[i].rgbGreen = i;
		pbmi->bmiColors[i].rgbBlue = i;
		pbmi->bmiColors[i].rgbReserved = 0;
		}

	//	Done

	*retpbi = pbmi;
	}

bool C8bitImage::WriteToWindowsBMP (IByteStream &Stream, CString *retsError) const

//	WriteToWindowsBMP
//
//	Write BMP format to the stream.

	{
	//	Compute the total size of the image data (in bytes)

	int iColorTable = 256 * (sizeof RGBQUAD);
	int iImageDataSize = m_cyHeight * m_cxWidth;

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
		BYTE *pRow = GetPixelPos(0, y);
		Stream.Write(pRow, m_cxWidth);
		}

	delete pbmi;
	return true;
	}
