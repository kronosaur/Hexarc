//	CASTIf.cpp
//
//	CASTIf Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTIf::DebugDump (const CString &sIndent) const 
	{
	printf("%sif\n", (LPSTR)sIndent);
	m_pCondition->DebugDump(strPattern("%s\t", sIndent));
	printf("%sthen\n", (LPSTR)sIndent);
	m_pThen->DebugDump(strPattern("%s\t", sIndent));
	if (m_pElse)
		{
		printf("%selse\n", (LPSTR)sIndent);
		m_pElse->DebugDump(strPattern("%s\t", sIndent));
		}
	}
