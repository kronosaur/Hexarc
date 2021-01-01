//	CASTSequence.cpp
//
//	CASTSequence Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTSequence::DebugDump (const CString &sIndent) const 
	{
	for (int i = 0; i < m_Node.GetCount(); i++)
		{
		m_Node[i]->DebugDump(sIndent);
		}
	}
