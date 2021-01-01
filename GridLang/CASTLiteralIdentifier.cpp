//	CASTLiteralIdentifier.cpp
//
//	CASTLiteralIdentifier Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTLiteralIdentifier::DebugDump (const CString &sIndent) const 
	{
	switch (m_iType)
		{
		case EASTType::LiteralNull:
			printf("%snull\n", (LPSTR)sIndent);
			break;
			
		case EASTType::LiteralTrue:
			printf("%strue\n", (LPSTR)sIndent);
			break;
			
		default:
			printf("%sunknown %d\n", (LPSTR)sIndent, (int)m_iType);
			break;
		}
	}
