//	CASTBinaryOp.cpp
//
//	CASTBinaryOp Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CASTBinaryOp::DebugDump (const CString &sIndent) const 
	{
	switch (m_iOp)
		{
		case EASTType::OpArithmeticAnd:
			printf("%soperator &\n", (LPSTR)sIndent);
			break;

		case EASTType::OpArithmeticOr:
			printf("%soperator |\n", (LPSTR)sIndent);
			break;

		case EASTType::OpAssignment:
			printf("%soperator =\n", (LPSTR)sIndent);
			break;

		case EASTType::OpDivide:
			printf("%soperator /\n", (LPSTR)sIndent);
			break;

		case EASTType::OpEquals:
			printf("%soperator ==\n", (LPSTR)sIndent);
			break;

		case EASTType::OpFunctionCall:
			printf("%soperator ()\n", (LPSTR)sIndent);
			break;

		case EASTType::OpGreaterThan:
			printf("%soperator >\n", (LPSTR)sIndent);
			break;

		case EASTType::OpGreaterThanEquals:
			printf("%soperator >=\n", (LPSTR)sIndent);
			break;

		case EASTType::OpLessThan:
			printf("%soperator <\n", (LPSTR)sIndent);
			break;

		case EASTType::OpLessThanEquals:
			printf("%soperator <=\n", (LPSTR)sIndent);
			break;

		case EASTType::OpLogicalAnd:
			printf("%soperator &&\n", (LPSTR)sIndent);
			break;

		case EASTType::OpLogicalOr:
			printf("%soperator ||\n", (LPSTR)sIndent);
			break;

		case EASTType::OpMemberAccess:
			printf("%soperator .\n", (LPSTR)sIndent);
			break;

		case EASTType::OpMinus:
			printf("%soperator -\n", (LPSTR)sIndent);
			break;

		case EASTType::OpNot:
			printf("%soperator !\n", (LPSTR)sIndent);
			break;

		case EASTType::OpNotEquals:
			printf("%soperator !=\n", (LPSTR)sIndent);
			break;

		case EASTType::OpPlus:
			printf("%soperator +\n", (LPSTR)sIndent);
			break;

		case EASTType::OpTimes:
			printf("%soperator *\n", (LPSTR)sIndent);
			break;

		case EASTType::OpReturn:
			printf("%soperator return\n", (LPSTR)sIndent);
			break;

		default:
			printf("%soperator %d\n", (LPSTR)sIndent, (int)m_iOp);
			break;
		}

	m_pLeft->DebugDump(strPattern("%s\t", sIndent));
	m_pRight->DebugDump(strPattern("%s\t", sIndent));
	}
