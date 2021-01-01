//	CASTLiteralFloat.cpp
//
//	CASTLiteralFloat Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTLiteralFloat::DebugDump (const CString &sIndent) const 
	{
	printf("%s%s\n", (LPSTR)sIndent, (LPSTR)strFromDouble(m_rValue));
	}
