//	CGridLangVMCompiler.cpp
//
//	CGridLangVMCompiler Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

bool CGridLangVMCompiler::CompileDefinition (const IASTNode &AST, CString *retsError)

//	CompileDefinition
//
//	Compiles all the code for a definition.

	{
	return true;
	}

bool CGridLangVMCompiler::CompileSequence (const IASTNode &AST, const CString &sSymbol, CString *retsError)

//	CompileSequence
//
//	Compiles all statements in the given sequence and writes out to the given
//	symbol.

	{
	SCompilerCtx Ctx(m_Types);

	//	If no statements in the sequence, then we just push null.

	if (AST.GetStatementCount() == 0)
		{
		Ctx.WriteShortOpCode(opPushNil);
		}

	//	Otherwise, add all statements to the current block.

	else
		{
		for (int i = 0; i < AST.GetStatementCount(); i++)
			{
			if (!CompileStatement(Ctx, AST.GetStatement(i), retsError))
				return false;
			}
		}

	//	Terminate

	Ctx.WriteShortOpCode(opHalt);

	//	Type is a function with 0 arguments.
	//	LATER: We need to create the property type.

	const IGLType &MainType = m_Types.GetCoreType(GLCoreType::Function);

	//	Add to our code bank

	CDatum dCode = Ctx.Code.CreateOutput(Ctx.iBlock);
	m_Output.AddSymbol(sSymbol, MainType, dCode);

	return true;
	}

//	Main Compile Functions -----------------------------------------------------
//
//	The functions in this section compile specific ASTs given a compiler context
//	block.

bool CGridLangVMCompiler::CompileExpression (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError)

//	CompileExpression
//
//	Compiles an expression.

	{
	switch (AST.GetType())
		{
		case EASTType::LiteralString:
			{
			int iDataBlock = Ctx.Code.CreateDatumBlock(AST.GetValue());
			Ctx.WriteShortOpCode(opPushStr, iDataBlock);
			break;
			}

		case EASTType::VarRef:
			{
			if (!CompileVariableReference(Ctx, AST, retsError))
				return false;
			break;
			}

		default:
			return ComposeError(Ctx, strPattern("Not an expression: %s", AST.GetTypeName()), retsError);
		}

	return true;
	}

bool CGridLangVMCompiler::CompileFunctionCall (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError)

//	CompileFunctionCall
//
//	Compiles a function call.

	{
	//	Compile the function name as an expression. I.e., we support function
	//	indirection, etc. here.

	if (!CompileExpression(Ctx, AST.GetRoot(), retsError))
		return false;

	//	Now push the arguments of the function.

	for (int i = 0; i < AST.GetChildCount(); i++)
		{
		if (!CompileExpression(Ctx, AST.GetChild(i), retsError))
			return false;
		}

	//	Create the environment and call the function

	Ctx.WriteShortOpCode(opMakeEnv, AST.GetChildCount());
	Ctx.WriteShortOpCode(opCall);

	return true;
	}

bool CGridLangVMCompiler::CompileStatement (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError)

//	CompileStatement
//
//	Compiles a statement to the current block.

	{
	switch (AST.GetType())
		{
		case EASTType::FunctionCall:
			if (!CompileFunctionCall(Ctx, AST, retsError))
				return false;
			break;

		default:
			return ComposeError(Ctx, strPattern("Not a statement: %s", AST.GetTypeName()), retsError);
		}

	return true;
	}

bool CGridLangVMCompiler::CompileVariableReference (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError)

//	CompileVariableReference
//
//	Compiles a variable reference.

	{
	//	Lookup the variable in our scope.

	const IGLType *pVarDef = Ctx.Scope.Find(AST.GetName());
	if (!pVarDef)
		return ComposeError(Ctx, strPattern("Unknown identifier: %s", AST.GetName()), retsError);

	//	Compile the appropriate reference

	switch (pVarDef->GetClass())
		{
		case GLTypeClass::Function:
			{
			//	Lookup the function

			int iDataBlock = Ctx.Code.CreateDatumBlock(pVarDef->GetName());
			Ctx.WriteShortOpCode(opPushGlobal, iDataBlock);

			break;
			}

		default:
			return ComposeError(Ctx, strPattern("Unknown definition: %s", pVarDef->GetName()), retsError);
		}

	return true;
	}

//	Utility Functions ----------------------------------------------------------

bool CGridLangVMCompiler::ComposeError (SCompilerCtx &Ctx, const CString &sError, CString *retsError) const

//	ComposeError
//
//	Compose and error.

	{
	if (retsError)
		*retsError = sError;

	return false;
	}
