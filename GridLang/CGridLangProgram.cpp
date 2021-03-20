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

	//	Compile

	CGridLangVMCompiler Compiler;
	if (!Compiler.CompileProgram(AST.GetRoot(), m_Types, m_Code, retsError))
		return false;

	//	Done

	return true;
	}
