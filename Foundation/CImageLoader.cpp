//	CImageLoader.cpp
//
//	CImageLoader class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(EXTENSION_BMP,						".bmp");
DECLARE_CONST_STRING(EXTENSION_JPEG,					".jpeg");
DECLARE_CONST_STRING(EXTENSION_JPG,						".jpg");
DECLARE_CONST_STRING(EXTENSION_PNG,						".png");

DECLARE_CONST_STRING(ERR_CRASH,							"Crash loading image.");
DECLARE_CONST_STRING(ERR_UNKNOWN_IMAGE_FORMAT,			"Unknown image format.");

CImageLoader::EFormats CImageLoader::GetFormatFromExtension (const CString &sFilespec)

//	GetFormatFromExtension
//
//	Returns the format given the file extension. Returns formatUnknown if we 
//	can't figure out the format.

	{
	//	First get the extension of the file

	CString sExtension = strToLower(fileGetExtension(sFilespec));

	//	Handle it based on the extension

	if (strEquals(sExtension, EXTENSION_BMP))
		return formatBMP;

	else if (strEquals(sExtension, EXTENSION_JPEG) || strEquals(sExtension, EXTENSION_JPG))
		return formatJPEG;

	else if (strEquals(sExtension, EXTENSION_PNG))
		return formatPNG;

	else
		return formatUnknown;
	}

bool CImageLoader::Load (IMemoryBlock &Data, EFormats iFormat, CRGBA32Image &Image, CString *retsError)

//	Load
//
//	Loads an image.

	{
	try
		{
		switch (iFormat)
			{
			case formatJPEG:
				return CJPEG::Load(Data, Image, retsError);

			case formatPNG:
				return CPNG::Load(Data, Image, retsError);

			default:
				if (retsError) *retsError = ERR_UNKNOWN_IMAGE_FORMAT;
				return false;
			}

		return true;
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_CRASH;
		return false;
		}
	}
