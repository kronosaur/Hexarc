//	CASTLiteralString.cpp
//
//	CASTLiteralString Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTLiteralString::DebugDump (const CString &sIndent) const 
	{
	printf("%s\"%s\"\n", (LPSTR)sIndent, (LPSTR)m_sValue);
	}
