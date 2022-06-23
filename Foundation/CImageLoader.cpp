//	CImageLoader.cpp
//
//	CImageLoader class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(EXTENSION_BMP,						".bmp");
DECLARE_CONST_STRING(EXTENSION_JPEG,					".jpeg");
DECLARE_CONST_STRING(EXTENSION_JPG,						".jpg");
DECLARE_CONST_STRING(EXTENSION_PNG,						".png");

DECLARE_CONST_STRING(MEDIATYPE_GIF,						"image/gif");
DECLARE_CONST_STRING(MEDIATYPE_JPEG,					"image/jpeg");
DECLARE_CONST_STRING(MEDIATYPE_PNG,						"image/png");

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

CImageLoader::EFormats CImageLoader::GetFormatFromString (const CString& sValue)

//	GetFormatFromString
//
//	Returns the given format from a string. The string can be either a media 
//	type, or a file name extension.
//
//	image/jpeg
//	image.jpeg
//	image.jpg
//	.jpg
//	jpeg

	{
	if (strEqualsNoCase(sValue, MEDIATYPE_GIF))
		//	Not yet supported
		return formatUnknown;
	else if (strEqualsNoCase(sValue, MEDIATYPE_JPEG))
		return formatJPEG;
	else if (strEqualsNoCase(sValue, MEDIATYPE_PNG))
		return formatPNG;

	//	Otherwise, expect a file name or extension

	else
		{
		//	Parse the extension. We expect sValue to be *.ext.

		CString sExtension = strToLower(fileGetExtension(sValue));

		//	If blank, then it means that there was no . in the string. In that
		//	case, we assume that the input is just the extension, so we convert
		//	it to a dotted value.

		if (sExtension.IsEmpty())
			sExtension = strPattern(".%s", sValue);

		//	Map

		if (strEqualsNoCase(sExtension, EXTENSION_BMP))
			return formatBMP;

		else if (strEqualsNoCase(sExtension, EXTENSION_JPEG) || strEqualsNoCase(sExtension, EXTENSION_JPG))
			return formatJPEG;

		else if (strEqualsNoCase(sExtension, EXTENSION_PNG))
			return formatPNG;

		else
			return formatUnknown;
		}
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
