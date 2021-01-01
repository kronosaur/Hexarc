//	CASTClassDef.cpp
//
//	CASTClassDef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTClassDef::DebugDump (const CString &sIndent) const 
	{
	printf("%s%s %s\n", (LPSTR)sIndent, (LPSTR)m_sBaseID, (LPSTR)m_sID);
	m_pBody->DebugDump(strPattern("%s\t", sIndent));
	}
