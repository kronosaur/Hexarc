//	CASTFunctionCall.cpp
//
//	CASTFunctionCall Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTFunctionCall::DebugDump (const CString &sIndent) const 
	{
	m_pFunction->DebugDump(sIndent);
	for (int i = 0; i < m_Args.GetCount(); i++)
		m_Args[i]->DebugDump(strPattern("%s\t", (LPSTR)sIndent));
	}
