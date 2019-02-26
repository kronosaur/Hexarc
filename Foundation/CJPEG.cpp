//	CJPEG.cpp
//
//	CJPEG class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CRASH,							"Crash parsing JPEG.")
DECLARE_CONST_STRING(ERR_INVALID_FORMAT,				"Invalid JPEG format.")

void jpegOnError (j_common_ptr cinfo);

bool CJPEG::Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError)

//	Load
//
//	Loads a JPEG into the given image. Returns FALSE if error.
//	See: https://gist.github.com/PhirePhly/3080633

	{
	try
		{
		int rc;

		//	Initialize the decompression context structure.

		struct jpeg_decompress_struct cinfo;

		//	The standard error handler will exit the process on any issue, 
		//	so we need to override it with our own that throws exceptions.
		//	See: https://stackoverflow.com/questions/19857766/error-handling-in-libjpeg

		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jerr.error_exit = jpegOnError;

		//	Setup the structure

		jpeg_create_decompress(&cinfo);

		//	Point to the JPEG data

		jpeg_mem_src(&cinfo, (BYTE *)Data.GetPointer(), Data.GetLength());

		//	Read the header

		rc = jpeg_read_header(&cinfo, TRUE);
		if (rc != 1)
			{
			jpeg_destroy_decompress(&cinfo);
			if (retsError) *retsError = ERR_INVALID_FORMAT;
			return false;
			}

		//	We want to return BGRA pixels, so we set the colorspace appropriately.
		//	This is only available in libjpeg-turbo.

		cinfo.out_color_space = JCS_EXT_BGRA;

		//	Start the decompression. This will initialize fields in cinfo about
		//	the image, such as size.

		jpeg_start_decompress(&cinfo);

		//	Create the image

		Image.Create(cinfo.output_width, cinfo.output_height);
		CRGBA32 *pRow = Image.GetPixelPos(0, 0);

		//	Loop over every scanline

		while (cinfo.output_scanline < cinfo.output_height)
			{
			//	Read scanlines expects an array of pointers to scanlines.

			BYTE *scanline[1];
			scanline[0] = (BYTE *)pRow;

			//	Read the scanline

			jpeg_read_scanlines(&cinfo, scanline, 1);

			//	Next row

			pRow = Image.NextRow(pRow);
			}

		//	Done

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		return true;
		}
	catch (CException e)
		{
		if (retsError) *retsError = e.GetErrorString();
		return false;
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_CRASH;
		return false;
		}
	}

bool CJPEG::Save (CRGBA32Image &Image, IByteStream &Output, int iQuality, CString *retsError)

//	Save
//
//	Saves to JPEG

	{
	try
		{
		//	Initialize the compression context structure.

		struct jpeg_compress_struct cinfo;

		//	The standard error handler will exit the process on any issue, 
		//	so we need to override it with our own that throws exceptions.
		//	See: https://stackoverflow.com/questions/19857766/error-handling-in-libjpeg

		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jerr.error_exit = jpegOnError;

		//	Initialize JPEG compression object

		jpeg_create_compress(&cinfo);

		//	Send the output to a memory block that the library will allocate for
		//	us and that we later take ownership of.

		BYTE *pOutput = NULL;
		DWORD dwOutputSize = 0;
		jpeg_mem_dest(&cinfo, &pOutput, &dwOutputSize);

		//	Describe the input image

		cinfo.image_height = Image.GetHeight();
		cinfo.image_width = Image.GetWidth();
		cinfo.input_components = 4;
		cinfo.in_color_space = JCS_EXT_BGRA;

		//	Set other defaults

		jpeg_set_defaults(&cinfo);

		//	Set quality

		jpeg_set_quality(&cinfo, iQuality, true);

		//	Start compressor

		jpeg_start_compress(&cinfo, true);

		//	Loop over every row

		CRGBA32 *pRow = Image.GetPixelPos(0, 0);
		while (cinfo.next_scanline < cinfo.image_height)
			{
			JSAMPROW row_pointer[1];
			row_pointer[0] = (BYTE *)pRow;

			jpeg_write_scanlines(&cinfo, row_pointer, 1);

			pRow = Image.NextRow(pRow);
			}

		//	Done

		jpeg_finish_compress(&cinfo);

		//	Write out to the output stream

		Output.Write(pOutput, (int)dwOutputSize);

		//	Free the buffer

		free(pOutput);

		//	Done

		jpeg_destroy_compress(&cinfo);

		return true;
		}
	catch (CException e)
		{
		if (retsError) *retsError = e.GetErrorString();
		return false;
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_CRASH;
		return false;
		}
	}

void jpegOnError (j_common_ptr cinfo)
	{
	char jpegLastErrorMsg[JMSG_LENGTH_MAX];

	//	Format the message into the buffer

	( *( cinfo->err->format_message ) ) ( cinfo, jpegLastErrorMsg );

	//	Throw it

	throw CException(errFail, CString(jpegLastErrorMsg));
	}
