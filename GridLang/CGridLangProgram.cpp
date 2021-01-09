//	CGridLangProgram.cpp
//
//	CGridLangProgram Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

bool CGridLangProgram::Load (IMemoryBlock &Stream, CString *retsError)

//	Load
//
//	Load a program from a stream.

	{
	//	Load the source into an AST

	CGridLangAST AST;
	if (!AST.Load(Stream, retsError))
		return false;

	//	Initialize the type system

	if (!m_Types.InitFromAST(AST, retsError))
		return false;

	//	Done

	return true;
	}
