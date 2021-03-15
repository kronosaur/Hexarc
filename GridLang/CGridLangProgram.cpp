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

	//	Process the AST and declare all types. The type system is initialized
	//	and we modify the AST to point to appropriate types.

	CGridLangTypeLoader TypeLoader(AST, m_Types);
	if (!TypeLoader.Load(retsError))
		return false;

	//	Now load all library functions into the AST so that they can be looked
	//	up.

	if (!AST.GetRoot().AddChild(CGridLangCoreLibrary::GetDefinitions(), retsError))
		return false;

#if 0
	//	Initialize the type system

	if (!m_Types.InitFromAST(AST, retsError))
		return false;
#endif

	//	Compile all definitions

	const IASTNode &Root = AST.GetRoot();

	CGridLangVMCompiler Compiler(m_Types, m_Code);
	for (int i = 0; i < Root.GetDefinitionCount(); i++)
		{
		auto &Def = Root.GetDefinition(i);

		if (!Compiler.CompileDefinition(Def, retsError))
			{
#ifdef DEBUG
			AST.DebugDump();
			printf("\n");
			m_Types.DebugDump();
#endif
			return false;
			}
		}

	//	Now compile main statements.

	if (!Compiler.CompileSequence(Root, SYMBOL_MAIN, retsError))
		{
#ifdef DEBUG
		AST.DebugDump();
		printf("\n");
		m_Types.DebugDump();
#endif
		return false;
		}

	//	Done

	return true;
	}
