//	CASTVarDef.cpp
//
//	CASTVarDef Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTVarDef::DebugDump (const CString &sIndent) const 
	{
	CString sType;
	if (m_pTypeDef)
		sType = m_pTypeDef->GetName();
	else
		sType = CString("(null)");

	printf("%s%s:%s\n", (LPSTR)sIndent, (LPSTR)m_sName, (LPSTR)sType);
	if (m_pBody)
		m_pBody->DebugDump(strPattern("%s\t", sIndent));
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
			return true;

		default:
			return false;
		}
	}
