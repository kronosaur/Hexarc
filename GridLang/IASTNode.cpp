//	IASTNode.cpp
//
//	IASTNode Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

const IASTNode::STypeDesc IASTNode::m_TypeDesc[(int)EASTType::LastType] = 
	{
		{	EASTType::Unknown,				"unknown"	},

		{	EASTType::ArgDef,				"argument definition"	},
		{	EASTType::ClassDef,				"class definition"	},
		{	EASTType::ConstDef,				"const definition"	},
		{	EASTType::DoIf,					"if statement"	},
		{	EASTType::FunctionCall,			"function call"	},
		{	EASTType::FunctionDef,			"function definition"	},
		{	EASTType::GlobalDef,			"global definition"	},
		{	EASTType::LiteralNull,			"literal null"	},
		{	EASTType::LiteralFloat,			"literal float"	},
		{	EASTType::LiteralInteger,		"literal integer"	},
		{	EASTType::LiteralString,		"literal string"	},
		{	EASTType::LiteralTrue,			"literal true"	},
		{	EASTType::OpArithmeticAnd,		"& operator"	},
		{	EASTType::OpArithmeticOr,		"| operator"	},
		{	EASTType::OpAssignment,			"= operator"	},
		{	EASTType::OpDivide,				"/ operator"	},
		{	EASTType::OpEquals,				"== operator"	},
		{	EASTType::OpFunctionCall,		"function call operator"	},
		{	EASTType::OpGreaterThan,		"> operator"	},
		{	EASTType::OpGreaterThanEquals,	">= operator"	},
		{	EASTType::OpLessThan,			"< operator"	},
		{	EASTType::OpLessThanEquals,		"<= operator"	},
		{	EASTType::OpLogicalAnd,			"&& operator"	},
		{	EASTType::OpLogicalOr,			"|| operator"	},
		{	EASTType::OpMemberAccess,		". operator"	},
		{	EASTType::OpMinus,				"- operator"	},
		{	EASTType::OpNot,				"! operator"	},
		{	EASTType::OpNotEquals,			"!= operator"	},
		{	EASTType::OpPlus,				"+ operator"	},
		{	EASTType::OpTimes,				"* operator"	},
		{	EASTType::OpReturn,				"return operator"	},
		{	EASTType::OrdinalDef,			"ordinal definition"	},
		{	EASTType::PropertyDef,			"property definition"	},
		{	EASTType::Sequence,				"sequence"	},
		{	EASTType::TypeRef,				"type reference"	},
		{	EASTType::VarDef,				"variable definition"	},
		{	EASTType::VarRef,				"variable reference"	},
	};

bool IASTNode::ComposeError (const CString &sError, CString *retsError) const

//	ComposeError
//
//	Composes an error for the given AST.

	{
	if (retsError)
		*retsError = sError;

	return false;
	}

CString IASTNode::GetTypeName (EASTType iType)

//	GetTypeName
//
//	Returns the human-readable name of the type (usually for debugging).

	{
	return m_TypeDesc[(int)iType].sName;
	}
