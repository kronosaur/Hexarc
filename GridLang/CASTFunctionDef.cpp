//	CASTFunctionDef.cpp
//
//	CASTFunctionDef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTFunctionDef::DebugDump (const CString &sIndent) const 
	{
	printf("%sfunction %s (\n", (LPSTR)sIndent, (LPSTR)m_sFunction);
	for (int i = 0; i < m_ArgDefs.GetCount(); i++)
		{
		m_ArgDefs[i]->DebugDump(strPattern("%s\t", (LPSTR)sIndent));
		}
	printf("%s)\n", (LPSTR)sIndent);

	m_pBody->DebugDump(strPattern("%s\t", sIndent));
	}
