//	CASTUnaryOp.cpp
//
//	CASTUnaryOp Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTUnaryOp::DebugDump (const CString &sIndent) const 
	{
	switch (m_iOp)
		{
		case EASTType::OpNot:
			printf("%soperator !\n", (LPSTR)sIndent);
			break;

		case EASTType::OpReturn:
			printf("%soperator return\n", (LPSTR)sIndent);
			break;

		default:
			printf("%soperator %d\n", (LPSTR)sIndent, (int)m_iOp);
			break;
		}

	m_pOperand->DebugDump(strPattern("%s\t", sIndent));
	}

bool CASTUnaryOp::IsStatement () const
	{
	switch (m_iOp)
		{
		case EASTType::OpReturn:
			return true;

		default:
			return false;
		}
	}
