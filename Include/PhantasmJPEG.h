//	PhantasmJPEG.h
//
//	Phantasm header file
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CJPEG
	{
	public:
		static bool Load (IMemoryBlock &Data, CRGBA32Image &Image, CString *retsError = NULL);

	private:
	};