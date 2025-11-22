//	FoundationGraphicsJPEG.h
//
//	Foundation header file
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CJPEG
	{
	public:
		static constexpr int QUALITY_DEFAULT = 80;

		static bool Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError = NULL);
		static bool Save (const CRGBA32Image &Image, IByteStream &Output, int iQuality, CString *retsError = NULL);

	private:
	};
