//	CASTOrdinalDef.cpp
//
//	CASTOrdinalDef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTOrdinalDef::DebugDump (const CString &sIndent) const 
	{
	printf("%sordinal %s\n", (LPSTR)sIndent, (LPSTR)m_sName);
	}
