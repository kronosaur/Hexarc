//	GridLang.h
//
//	GridLang Classes
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "GridLangAST.h"
#include "GridLangTypes.h"

class CGridLangProgram
	{
	public:
		const CGLTypeNamespace &GetGlobals () const { return m_Types.GetGlobals(); }
		bool Load (IMemoryBlock &Stream, CString *retsError);

	private:
		CGLTypeSystem m_Types;
	};

class CGridLangProcess
	{
	public:
		bool Load (CGridLangProgram &Program, CString *retsError);
	};
