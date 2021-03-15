//	CASTLibraryFunctionDef.cpp
//
//	CASTLibraryFunctionDef Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTLibraryFunctionDef::DebugDump (const CString &sIndent) const 
	{
	printf("%sfunction %s\n", (LPSTR)sIndent, (LPSTR)m_sFunction);
	}
