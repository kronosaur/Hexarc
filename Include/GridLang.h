//	GridLang.h
//
//	GridLang Classes
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CGridLangProgram
	{
	public:
		bool Load (IMemoryBlock &Stream, CString *retsError);

	private:
		
	};

class CGridLangProcess
	{
	public:
		bool Load (CGridLangProgram &Program, CString *retsError);
	};
