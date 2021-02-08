//	CGridLangProgram.cpp
//
//	CGridLangProgram Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(SYMBOL_MAIN,						"$main")

const CString &CGridLangProgram::GetMainFunction () const

//	GetMainFunction
//
//	Returns the main function.

	{
	return SYMBOL_MAIN;
	}

bool CGridLangProgram::Load (IMemoryBlock &Stream, CString *retsError)

//	Load
//
//	Load a program from a stream.

	{
	//	Load the source into an AST

	CGridLangAST AST;
	if (!AST.Load(Stream, retsError))
		return false;

	if (AST.IsEmpty())
		return true;

	const IASTNode &Root = AST.GetRoot();

	//	Initialize the type system

	if (!m_Types.InitFromAST(AST, retsError))
		return false;

	//	Compile all definitions

	CGridLangVMCompiler Compiler(m_Types, m_Code);
	for (int i = 0; i < Root.GetDefinitionCount(); i++)
		{
		auto &Def = Root.GetDefinition(i);

		if (!Compiler.CompileDefinition(Def, retsError))
			return false;
		}

	//	Now compile main statements.

	if (!Compiler.CompileSequence(Root, SYMBOL_MAIN, retsError))
		return false;

	//	Done

	return true;
	}
