//	CBMP.cpp
//
//	CBMP class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_IMAGE_TOO_LARGE,				"BMP image is too large.");
DECLARE_CONST_STRING(ERR_INVALID_FORMAT,				"Invalid BMP format.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_FORMAT,			"Unsupported BMP format.");

namespace
	{
	constexpr DWORD BMP_COMPRESSION_RGB = 0;
	constexpr DWORD BMP_COMPRESSION_RLE8 = 1;
	constexpr DWORD BMP_COMPRESSION_RLE4 = 2;
	constexpr DWORD BMP_COMPRESSION_BITFIELDS = 3;
	constexpr DWORD BMP_COMPRESSION_ALPHA_BITFIELDS = 6;

	bool HasData (size_t iOffset, size_t iLength, size_t iTotalLength)
		{
		return (iOffset <= iTotalLength && iLength <= iTotalLength - iOffset);
		}

	bool IsSupportedDIBHeaderSize (DWORD dwHeaderSize)
		{
		return (dwHeaderSize == 12
				|| dwHeaderSize == 40
				|| dwHeaderSize == 52
				|| dwHeaderSize == 56
				|| dwHeaderSize == 108
				|| dwHeaderSize == 124);
		}

	WORD ReadWORD (const BYTE *pData)
		{
		return (WORD)((WORD)pData[0] | ((WORD)pData[1] << 8));
		}

	DWORD ReadDWORD (const BYTE *pData)
		{
		return ((DWORD)pData[0]
				| ((DWORD)pData[1] << 8)
				| ((DWORD)pData[2] << 16)
				| ((DWORD)pData[3] << 24));
		}

	LONG ReadLONG (const BYTE *pData)
		{
		return (LONG)ReadDWORD(pData);
		}

	BYTE DecodeMask (DWORD dwValue, DWORD dwMask)
		{
		if (dwMask == 0)
			return 0;

		while ((dwMask & 1) == 0)
			{
			dwMask >>= 1;
			dwValue >>= 1;
			}

		DWORD dwComponent = dwValue & dwMask;
		return (BYTE)((dwComponent * 255ULL + (dwMask / 2)) / dwMask);
		}

	bool ValidateMasks (DWORD dwRedMask, DWORD dwGreenMask, DWORD dwBlueMask, DWORD dwAlphaMask)
		{
		if (dwRedMask == 0 || dwGreenMask == 0 || dwBlueMask == 0)
			return false;

		return ((dwRedMask & dwGreenMask) == 0
				&& (dwRedMask & dwBlueMask) == 0
				&& (dwGreenMask & dwBlueMask) == 0
				&& (dwAlphaMask == 0
						|| ((dwAlphaMask & dwRedMask) == 0
								&& (dwAlphaMask & dwGreenMask) == 0
								&& (dwAlphaMask & dwBlueMask) == 0)));
		}

	bool SetPalettePixel (CRGBA32Image &Image, int x, int y, BYTE byIndex, const TArray<CRGBA32> &Palette)
		{
		if (x < 0 || x >= Image.GetWidth() || y < 0 || y >= Image.GetHeight() || byIndex >= Palette.GetCount())
			return false;

		*Image.GetPixelPos(x, y) = Palette[byIndex];
		return true;
		}

	bool DecodeRLE (const BYTE *pData, size_t iDataLength, size_t iDataOffset, DWORD dwCompression, const TArray<CRGBA32> &Palette, CRGBA32Image &Image)
		{
		size_t iPos = iDataOffset;
		int x = 0;
		int y = 0;

		while (iPos < iDataLength && y < Image.GetHeight())
			{
			if (!HasData(iPos, 2, iDataLength))
				return false;

			BYTE byCount = pData[iPos++];
			BYTE byValue = pData[iPos++];

			if (byCount != 0)
				{
				for (int i = 0; i < byCount; i++)
					{
					BYTE byIndex = (dwCompression == BMP_COMPRESSION_RLE8
							? byValue
							: (i % 2 == 0 ? (byValue >> 4) : (byValue & 0x0f)));

					if (!SetPalettePixel(Image, x++, Image.GetHeight() - 1 - y, byIndex, Palette))
						return false;
					}
				}
			else if (byValue == 0)
				{
				x = 0;
				y++;
				}
			else if (byValue == 1)
				return true;
			else if (byValue == 2)
				{
				if (!HasData(iPos, 2, iDataLength))
					return false;

				x += pData[iPos++];
				y += pData[iPos++];
				if (x < 0 || x > Image.GetWidth() || y < 0 || y > Image.GetHeight())
					return false;
				}
			else
				{
				int iCount = byValue;
				size_t iEncodedLength = (dwCompression == BMP_COMPRESSION_RLE8 ? iCount : (iCount + 1) / 2);
				if (!HasData(iPos, iEncodedLength, iDataLength))
					return false;

				for (int i = 0; i < iCount; i++)
					{
					BYTE byIndex;
					if (dwCompression == BMP_COMPRESSION_RLE8)
						byIndex = pData[iPos + i];
					else
						{
						BYTE byPair = pData[iPos + (i / 2)];
						byIndex = (i % 2 == 0 ? (byPair >> 4) : (byPair & 0x0f));
						}

					if (!SetPalettePixel(Image, x++, Image.GetHeight() - 1 - y, byIndex, Palette))
						return false;
					}

				iPos += iEncodedLength;
				if ((iEncodedLength & 1) != 0)
					{
					if (!HasData(iPos, 1, iDataLength))
						return false;
					iPos++;
					}
				}
			}

		return (y >= Image.GetHeight());
		}
	}

bool CBMP::IsDIB (const IMemoryBlock64 &Data, bool bPartial)

//	IsDIB
//
//	Returns TRUE if the data has a valid bare Windows DIB header. Ordinary BMP
//	files have a BITMAPFILEHEADER and are detected by their BM signature.

	{
	DWORDLONG dwDataLength64 = Data.GetLength();
	if (dwDataLength64 < 12 || dwDataLength64 > SIZE_MAX)
		return false;

	size_t iDataLength = (size_t)dwDataLength64;
	const BYTE *pData = (const BYTE *)Data.GetPointer();
	DWORD dwHeaderSize = ReadDWORD(pData);
	if (!IsSupportedDIBHeaderSize(dwHeaderSize))
		return false;

	if (!HasData(0, dwHeaderSize, iDataLength))
		return false;

	LONG cxWidth;
	LONG cyHeightSigned;
	WORD wPlanes;
	WORD wBitsPerPixel;
	DWORD dwCompression = BMP_COMPRESSION_RGB;
	DWORD dwColorsUsed = 0;
	bool bCoreHeader = (dwHeaderSize == 12);

	if (bCoreHeader)
		{
		cxWidth = ReadWORD(pData + 4);
		cyHeightSigned = ReadWORD(pData + 6);
		wPlanes = ReadWORD(pData + 8);
		wBitsPerPixel = ReadWORD(pData + 10);
		}
	else
		{
		cxWidth = ReadLONG(pData + 4);
		cyHeightSigned = ReadLONG(pData + 8);
		wPlanes = ReadWORD(pData + 12);
		wBitsPerPixel = ReadWORD(pData + 14);
		dwCompression = ReadDWORD(pData + 16);
		dwColorsUsed = ReadDWORD(pData + 32);
		}

	if (cxWidth <= 0 || cyHeightSigned == 0 || cyHeightSigned == LONG_MIN || wPlanes != 1)
		return false;

	bool bSupported = false;
	if (dwCompression == BMP_COMPRESSION_RGB)
		bSupported = (wBitsPerPixel == 1 || wBitsPerPixel == 4 || wBitsPerPixel == 8 || wBitsPerPixel == 16 || wBitsPerPixel == 24 || wBitsPerPixel == 32);
	else if (dwCompression == BMP_COMPRESSION_RLE8)
		bSupported = (wBitsPerPixel == 8 && cyHeightSigned > 0);
	else if (dwCompression == BMP_COMPRESSION_RLE4)
		bSupported = (wBitsPerPixel == 4 && cyHeightSigned > 0);
	else if (dwCompression == BMP_COMPRESSION_BITFIELDS || dwCompression == BMP_COMPRESSION_ALPHA_BITFIELDS)
		bSupported = ((wBitsPerPixel == 16 || wBitsPerPixel == 32)
				&& (dwCompression != BMP_COMPRESSION_ALPHA_BITFIELDS || dwHeaderSize == 40 || dwHeaderSize >= 56));

	if (!bSupported)
		return false;

	size_t iDataOffset = dwHeaderSize;
	if (dwHeaderSize == 40
			&& (dwCompression == BMP_COMPRESSION_BITFIELDS || dwCompression == BMP_COMPRESSION_ALPHA_BITFIELDS))
		iDataOffset += (dwCompression == BMP_COMPRESSION_ALPHA_BITFIELDS ? 4 : 3) * sizeof(DWORD);

	if (wBitsPerPixel <= 8)
		{
		DWORD dwMaxColors = (1UL << wBitsPerPixel);
		DWORD dwColorCount = (dwColorsUsed == 0 ? dwMaxColors : dwColorsUsed);
		if (dwColorCount == 0 || dwColorCount > dwMaxColors)
			return false;

		iDataOffset += (size_t)dwColorCount * (bCoreHeader ? 3 : 4);
		}

	if (iDataOffset >= iDataLength)
		return false;

	if (bPartial || dwCompression == BMP_COMPRESSION_RLE4 || dwCompression == BMP_COMPRESSION_RLE8)
		return true;

	DWORDLONG cyHeight = (cyHeightSigned < 0 ? -(LONGLONG)cyHeightSigned : cyHeightSigned);
	DWORDLONG dwRowBits = (DWORDLONG)cxWidth * wBitsPerPixel;
	DWORDLONG dwRowSize = ((dwRowBits + 31) / 32) * 4;
	return ((DWORDLONG)iDataOffset + (dwRowSize * cyHeight) <= dwDataLength64);
	}

bool CBMP::Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError)

//	Load
//
//	Loads a Windows BMP into the given image. Returns FALSE if error.

	{
	const BYTE *pData = (const BYTE *)Data.GetPointer();
	size_t iDataLength = Data.GetLength();

	bool bHasFileHeader = (HasData(0, 14, iDataLength) && pData[0] == 'B' && pData[1] == 'M');
	size_t iHeaderOffset = (bHasFileHeader ? 14 : 0);
	if (!HasData(iHeaderOffset, 4, iDataLength))
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	size_t iDataOffset = (bHasFileHeader ? ReadDWORD(pData + 10) : 0);
	DWORD dwHeaderSize = ReadDWORD(pData + iHeaderOffset);
	if (iDataOffset > iDataLength || !HasData(iHeaderOffset, dwHeaderSize, iDataLength))
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	LONG cxWidth;
	LONG cyHeightSigned;
	WORD wPlanes;
	WORD wBitsPerPixel;
	DWORD dwCompression = BMP_COMPRESSION_RGB;
	DWORD dwColorsUsed = 0;
	size_t iColorTableOffset;
	bool bCoreHeader = false;

	if (dwHeaderSize == 12)
		{
		bCoreHeader = true;
		cxWidth = ReadWORD(pData + iHeaderOffset + 4);
		cyHeightSigned = ReadWORD(pData + iHeaderOffset + 6);
		wPlanes = ReadWORD(pData + iHeaderOffset + 8);
		wBitsPerPixel = ReadWORD(pData + iHeaderOffset + 10);
		iColorTableOffset = iHeaderOffset + 12;
		}
	else if (IsSupportedDIBHeaderSize(dwHeaderSize))
		{
		cxWidth = ReadLONG(pData + iHeaderOffset + 4);
		cyHeightSigned = ReadLONG(pData + iHeaderOffset + 8);
		wPlanes = ReadWORD(pData + iHeaderOffset + 12);
		wBitsPerPixel = ReadWORD(pData + iHeaderOffset + 14);
		dwCompression = ReadDWORD(pData + iHeaderOffset + 16);
		dwColorsUsed = ReadDWORD(pData + iHeaderOffset + 32);
		iColorTableOffset = iHeaderOffset + dwHeaderSize;
		}
	else
		{
		if (retsError) *retsError = ERR_UNSUPPORTED_FORMAT;
		return false;
		}

	if (cxWidth <= 0
			|| cyHeightSigned == 0
			|| cyHeightSigned == LONG_MIN
			|| wPlanes != 1)
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	bool bTopDown = (cyHeightSigned < 0);
	int cyHeight = (int)(bTopDown ? -cyHeightSigned : cyHeightSigned);
	if ((DWORDLONG)cxWidth * (DWORDLONG)cyHeight > (DWORDLONG)INT_MAX / sizeof(CRGBA32))
		{
		if (retsError) *retsError = ERR_IMAGE_TOO_LARGE;
		return false;
		}

	if (bTopDown && (dwCompression == BMP_COMPRESSION_RLE4 || dwCompression == BMP_COMPRESSION_RLE8))
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	bool bSupported = false;
	if (dwCompression == BMP_COMPRESSION_RGB)
		bSupported = (wBitsPerPixel == 1 || wBitsPerPixel == 4 || wBitsPerPixel == 8 || wBitsPerPixel == 16 || wBitsPerPixel == 24 || wBitsPerPixel == 32);
	else if (dwCompression == BMP_COMPRESSION_RLE8)
		bSupported = (wBitsPerPixel == 8);
	else if (dwCompression == BMP_COMPRESSION_RLE4)
		bSupported = (wBitsPerPixel == 4);
	else if (dwCompression == BMP_COMPRESSION_BITFIELDS || dwCompression == BMP_COMPRESSION_ALPHA_BITFIELDS)
		bSupported = (wBitsPerPixel == 16 || wBitsPerPixel == 32);

	if (!bSupported)
		{
		if (retsError) *retsError = ERR_UNSUPPORTED_FORMAT;
		return false;
		}

	DWORD dwRedMask = 0;
	DWORD dwGreenMask = 0;
	DWORD dwBlueMask = 0;
	DWORD dwAlphaMask = 0;

	if (dwCompression == BMP_COMPRESSION_BITFIELDS || dwCompression == BMP_COMPRESSION_ALPHA_BITFIELDS)
		{
		if (dwHeaderSize >= 52)
			{
			dwRedMask = ReadDWORD(pData + iHeaderOffset + 40);
			dwGreenMask = ReadDWORD(pData + iHeaderOffset + 44);
			dwBlueMask = ReadDWORD(pData + iHeaderOffset + 48);
			if (dwHeaderSize >= 56)
				dwAlphaMask = ReadDWORD(pData + iHeaderOffset + 52);
			}
		else
			{
			size_t iMaskCount = (dwCompression == BMP_COMPRESSION_ALPHA_BITFIELDS ? 4 : 3);
			if (!HasData(iColorTableOffset, iMaskCount * sizeof(DWORD), iDataLength))
				{
				if (retsError) *retsError = ERR_INVALID_FORMAT;
				return false;
				}

			dwRedMask = ReadDWORD(pData + iColorTableOffset);
			dwGreenMask = ReadDWORD(pData + iColorTableOffset + 4);
			dwBlueMask = ReadDWORD(pData + iColorTableOffset + 8);
			if (iMaskCount == 4)
				dwAlphaMask = ReadDWORD(pData + iColorTableOffset + 12);

			iColorTableOffset += iMaskCount * sizeof(DWORD);
			}
		}
	else if (wBitsPerPixel == 16)
		{
		dwRedMask = 0x00007c00;
		dwGreenMask = 0x000003e0;
		dwBlueMask = 0x0000001f;
		}
	else if (wBitsPerPixel == 32)
		{
		dwRedMask = 0x00ff0000;
		dwGreenMask = 0x0000ff00;
		dwBlueMask = 0x000000ff;
		}

	if ((dwCompression == BMP_COMPRESSION_ALPHA_BITFIELDS && dwAlphaMask == 0)
			|| ((wBitsPerPixel == 16 || wBitsPerPixel == 32)
					&& !ValidateMasks(dwRedMask, dwGreenMask, dwBlueMask, dwAlphaMask)))
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	size_t iMinimumDataOffset = iColorTableOffset;
	TArray<CRGBA32> Palette;
	if (wBitsPerPixel <= 8)
		{
		DWORD dwMaxColors = (1UL << wBitsPerPixel);
		DWORD dwColorCount = (dwColorsUsed == 0 ? dwMaxColors : dwColorsUsed);
		if (dwColorCount == 0 || dwColorCount > dwMaxColors)
			{
			if (retsError) *retsError = ERR_INVALID_FORMAT;
			return false;
			}

		size_t iEntrySize = (bCoreHeader ? 3 : 4);
		size_t iPaletteSize = (size_t)dwColorCount * iEntrySize;
		if (!HasData(iColorTableOffset, iPaletteSize, iDataLength)
				|| (bHasFileHeader && iColorTableOffset + iPaletteSize > iDataOffset))
			{
			if (retsError) *retsError = ERR_INVALID_FORMAT;
			return false;
			}

		Palette.InsertEmpty((int)dwColorCount);
		for (DWORD i = 0; i < dwColorCount; i++)
			{
			const BYTE *pEntry = pData + iColorTableOffset + ((size_t)i * iEntrySize);
			Palette[i] = CRGBA32(pEntry[2], pEntry[1], pEntry[0]);
			}

		iMinimumDataOffset += iPaletteSize;
		}

	if (!bHasFileHeader)
		iDataOffset = iMinimumDataOffset;
	else if (iDataOffset < iMinimumDataOffset)
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	CRGBA32Image::EAlphaTypes iAlphaType = (dwAlphaMask != 0 ? CRGBA32Image::alpha8 : CRGBA32Image::alphaNone);
	if (!Image.Create((int)cxWidth, cyHeight, iAlphaType, CRGBA32(0, 0, 0)))
		{
		if (retsError) *retsError = ERR_IMAGE_TOO_LARGE;
		return false;
		}

	if (dwCompression == BMP_COMPRESSION_RLE4 || dwCompression == BMP_COMPRESSION_RLE8)
		{
		if (!DecodeRLE(pData, iDataLength, iDataOffset, dwCompression, Palette, Image))
			{
			if (retsError) *retsError = ERR_INVALID_FORMAT;
			return false;
			}

		return true;
		}

	DWORDLONG dwRowBits = (DWORDLONG)cxWidth * wBitsPerPixel;
	DWORDLONG dwRowSize = ((dwRowBits + 31) / 32) * 4;
	if (dwRowSize > SIZE_MAX
			|| (DWORDLONG)iDataOffset + (dwRowSize * cyHeight) > iDataLength)
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	for (int y = 0; y < cyHeight; y++)
		{
		int ySrc = (bTopDown ? y : cyHeight - 1 - y);
		const BYTE *pSrc = pData + iDataOffset + ((size_t)ySrc * (size_t)dwRowSize);
		CRGBA32 *pDest = Image.GetPixelPos(0, y);

		for (int x = 0; x < cxWidth; x++)
			{
			switch (wBitsPerPixel)
				{
				case 1:
					{
					BYTE byIndex = (pSrc[x / 8] >> (7 - (x % 8))) & 0x01;
					if (byIndex >= Palette.GetCount())
						{
						if (retsError) *retsError = ERR_INVALID_FORMAT;
						return false;
						}
					pDest[x] = Palette[byIndex];
					break;
					}

				case 4:
					{
					BYTE byPair = pSrc[x / 2];
					BYTE byIndex = (x % 2 == 0 ? (byPair >> 4) : (byPair & 0x0f));
					if (byIndex >= Palette.GetCount())
						{
						if (retsError) *retsError = ERR_INVALID_FORMAT;
						return false;
						}
					pDest[x] = Palette[byIndex];
					break;
					}

				case 8:
					{
					BYTE byIndex = pSrc[x];
					if (byIndex >= Palette.GetCount())
						{
						if (retsError) *retsError = ERR_INVALID_FORMAT;
						return false;
						}
					pDest[x] = Palette[byIndex];
					break;
					}

				case 16:
					{
					DWORD dwPixel = ReadWORD(pSrc + ((size_t)x * 2));
					pDest[x] = CRGBA32(
							DecodeMask(dwPixel, dwRedMask),
							DecodeMask(dwPixel, dwGreenMask),
							DecodeMask(dwPixel, dwBlueMask),
							(dwAlphaMask == 0 ? 0xff : DecodeMask(dwPixel, dwAlphaMask)));
					break;
					}

				case 24:
					pDest[x] = CRGBA32(pSrc[(size_t)x * 3 + 2], pSrc[(size_t)x * 3 + 1], pSrc[(size_t)x * 3]);
					break;

				case 32:
					{
					DWORD dwPixel = ReadDWORD(pSrc + ((size_t)x * 4));
					pDest[x] = CRGBA32(
							DecodeMask(dwPixel, dwRedMask),
							DecodeMask(dwPixel, dwGreenMask),
							DecodeMask(dwPixel, dwBlueMask),
							(dwAlphaMask == 0 ? 0xff : DecodeMask(dwPixel, dwAlphaMask)));
					break;
					}

				default:
					throw CException(errFail);
				}
			}
		}

	return true;
	}

bool CBMP::Save (const CRGBA32Image &Image, IByteStream &Output, CString *retsError)

//	Save
//
//	Saves an image as a Windows BMP. Returns FALSE if error.

	{
	if (Image.GetWidth() <= 0 || Image.GetHeight() <= 0)
		{
		if (retsError) *retsError = ERR_INVALID_FORMAT;
		return false;
		}

	DWORDLONG dwImageSize = (DWORDLONG)Image.GetWidth() * (DWORDLONG)Image.GetHeight() * sizeof(DWORD);
	if (dwImageSize > INT_MAX
			|| dwImageSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) > MAXDWORD)
		{
		if (retsError) *retsError = ERR_IMAGE_TOO_LARGE;
		return false;
		}

	return Image.WriteToWindowsBMP(Output, retsError);
	}
