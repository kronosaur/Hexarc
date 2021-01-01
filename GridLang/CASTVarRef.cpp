//	CASTVarRef.cpp
//
//	CASTVarRef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTVarRef::DebugDump (const CString &sIndent) const 
	{
	printf("%s%s\n", (LPSTR)sIndent, (LPSTR)m_sVar);
	}
