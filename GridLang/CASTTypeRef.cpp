//	CASTTypeRef.cpp
//
//	CASTTypeRef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTTypeRef::DebugDump (const CString &sIndent) const 
	{
	printf("%s%s\n", (LPSTR)sIndent, (LPSTR)m_sType);
	}
