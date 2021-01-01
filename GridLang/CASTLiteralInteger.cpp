//	CASTLiteralInteger.cpp
//
//	CASTLiteralInteger Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTLiteralInteger::DebugDump (const CString &sIndent) const 
	{
	printf("%s%s\n", (LPSTR)sIndent, (LPSTR)strFromInt(m_iValue));
	}
