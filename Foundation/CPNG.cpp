//	CPNG.cpp
//
//	CPNG class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "lodepng\lodepng.h"

bool CPNG::Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError)

//	Load
//
//	Loads a PNG into the given image. Returns FALSE if error.

	{
	unsigned char *pOutput;
	unsigned int dwWidth;
	unsigned int dwHeight;

	unsigned error = lodepng_decode32(&pOutput, &dwWidth, &dwHeight, (unsigned char *)Data.GetPointer(), Data.GetLength());
	if (error)
		{
		if (retsError) *retsError = CString(lodepng_error_text(error));
		return false;
		}

	//	Copy the buffer to the new image now that we have the bits and the size.
	//	We also need to flip from RGBA to BGRA

	Image.Create(dwWidth, dwHeight);
	CRGBA32 *pDestRow = Image.GetPixelPos(0, 0);
	CRGBA32 *pDestRowEnd = pDestRow + (dwWidth * dwHeight);

	DWORD *pSrcRow = (DWORD *)pOutput;

	while (pDestRow < pDestRowEnd)
		{
		CRGBA32 *pDest = pDestRow;
		CRGBA32 *pDestEnd = pDestRow + dwWidth;
		DWORD *pSrc = pSrcRow;

		while (pDest < pDestEnd)
			{
			BYTE *pPixel = (BYTE *)pSrc;
			*pDest++ = CRGBA32(pPixel[0], pPixel[1], pPixel[2], pPixel[3]);
			pSrc++;
			}

		pDestRow = Image.NextRow(pDestRow);
		pSrcRow += dwWidth;
		}

	free(pOutput);

	return true;
	}
