//	CASTVarDef.cpp
//
//	CASTVarDef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTVarDef::DebugDump (const CString &sIndent) const 
	{
	printf("%s%s\n", (LPSTR)sIndent, (LPSTR)m_sName);
	}

bool CASTVarDef::IsStatement () const

//	IsStatement
//
//	Returns TRUE if this needs to be executed in order.

	{
	switch (m_iVarType)
		{
		case EASTType::ConstDef:
		case EASTType::GlobalDef:
		case EASTType::VarDef:
			return (m_pBody && m_pBody->GetType() != EASTType::LiteralNull);

		default:
			return false;
		}
	}
