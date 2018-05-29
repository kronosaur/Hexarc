//	FoundationGraphicsPNG.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CPNG
	{
	public:
		static bool Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError = NULL);
	};