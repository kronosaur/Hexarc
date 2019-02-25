//	FoundationGraphicsJPEG.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CJPEG
	{
	public:
		static bool Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError = NULL);
		static bool Save (CRGBA32Image &Image, TBuffer<CMallocAllocator> &Data, int iQuality, CString *retsError = NULL);

	private:
	};
