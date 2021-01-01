//	CASTArgDef.cpp
//
//	CASTArgDef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTArgDef::DebugDump (const CString &sIndent) const 
	{
	printf("%s%s :\n", (LPSTR)sIndent, (LPSTR)m_sArg);
	m_pTypeRef->DebugDump(strPattern("%s\t", sIndent));
	}
