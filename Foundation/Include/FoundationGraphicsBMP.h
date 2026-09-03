//	FoundationGraphicsBMP.h
//
//	Foundation header file
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CBMP
	{
	public:
		static bool IsDIB (const IMemoryBlock64 &Data, bool bPartial = false);
		static bool Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError = NULL);
		static bool Save (const CRGBA32Image &Image, IByteStream &Output, CString *retsError = NULL);
	};
