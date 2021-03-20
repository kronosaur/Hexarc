//	CGridLangVMCompiler.cpp
//
//	CGridLangVMCompiler Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(SYMBOL_MAIN,						"$main")

bool CGridLangVMCompiler::CompileProgram (IASTNode &Root, CGLTypeSystem &retTypes, CGridLangCodeBank &retOutput, CString *retsError)

//	CompileProgram
//
//	Compiles a program from an AST.

	{
	m_Code.Init();
	m_pTypes = &retTypes;

	//	Process the AST and declare all types. The type system is initialized
	//	and we modify the AST to point to appropriate types.

	CGridLangTypeLoader TypeLoader(Root, *m_pTypes);
	if (!TypeLoader.Load(retsError))
		return false;

	//	Now load all library functions into the AST so that they can be looked
	//	up.

	if (!Root.AddChild(CGridLangCoreLibrary::GetDefinitions(), retsError))
		return false;

#if 0
	//	Initialize the type system

	if (!m_Types.InitFromAST(AST, retsError))
		return false;
#endif

	//	Compile all definitions

	for (int i = 0; i < Root.GetChildCount(); i++)
		{
		auto &Def = Root.GetChild(i);

		switch (Def.GetType())
			{
			case EASTType::FunctionDef:
				if (!CompileFunctionDef(Def, retsError))
					{
#ifdef DEBUG
					Root.DebugDump();
					printf("\n");
					m_pTypes->DebugDump();
#endif
					return false;
					}
				break;
				

			case EASTType::ClassDef:
				if (!CompileDefinition(Def, retsError))
					{
#ifdef DEBUG
					Root.DebugDump();
					printf("\n");
					m_pTypes->DebugDump();
#endif
					return false;
					}
				break;
			}
		}

	//	Now compile main statements.

	if (!CompileSequence(Root, retsError))
		{
#ifdef DEBUG
		Root.DebugDump();
		printf("\n");
		m_pTypes->DebugDump();
#endif
		return false;
		}

	//	Terminate

	m_Code.WriteShortOpCode(opHalt);

	//	Define the main function
	//
	//	Type is a function with 0 arguments.
	//	LATER: We need to create the property type.

	const IGLType &MainType = m_pTypes->GetCoreType(GLCoreType::Function);

	//	Add to our code bank

	CDatum dCode = m_Code.CreateOutput();
	retOutput.AddSymbol(SYMBOL_MAIN, MainType, dCode);

	//	Done

	return true;
	}

bool CGridLangVMCompiler::CompileDefinition (const IASTNode &AST, CString *retsError)

//	CompileDefinition
//
//	Compiles all the code for a definition.

	{
	return true;
	}

bool CGridLangVMCompiler::CompileFunctionDef (IASTNode &AST, CString *retsError)

//	CompileFunctionDef
//
//	Compiles a function definition.
//
//	NOTE: Functions are treated as definitions. We compile them and keep track 
//	of their code block in the AST symbol table. When invoking the function, we
//	look it up and get the value.
//
//	This is different from how HexeLisp does it. HexeLisp treats a function 
//	definition as a statement; it pushes the function block ID on the stack and
//	loads it into a global value variable.
//
//	The HexeLisp method means that there is no real difference between function 
//	definitions and anonymous functions. The only difference is whether we load
//	a global variable or not. For GridLang we will need to push the code block 
//	ID on the stack when we have an anonymous function.

	{
	m_Code.EnterCodeBlock();

	//	Now begin writing the new function. We start with an opcode
	//	to enter the environment.

	m_Code.WriteShortOpCode(opEnterEnv);

	//	Add all parameters

	for (int i = 0; i < AST.GetChildCount(); i++)
		{
		IASTNode &Param = AST.GetChild(i);

		//	Add a string block

		int iSymbol = m_Code.CreateDataBlock(Param.GetName());

		//	Add an argument definition opcode

		m_Code.WriteShortOpCode(opDefineArg, iSymbol);
		}

	//	Now compile the body

	if (!CompileSequence(AST.GetRoot(), retsError))
		return false;

	//	Remember the code block in the AST so we can find it later.

	AST.SetCodeID(m_Code.GetCodeBlock());

	//	Done

	m_Code.ExitCodeBlock();
	return true;
	}

bool CGridLangVMCompiler::CompileSequence (const IASTNode &AST, CString *retsError)

//	CompileSequence
//
//	Compiles all statements in the given sequence and writes out to the given
//	symbol.

	{
	//	If no statements in the sequence, then we just push null.

	if (AST.GetStatementCount() == 0)
		{
		m_Code.WriteShortOpCode(opPushNil);
		}

	//	Otherwise, add all statements to the current block.

	else
		{
		//	If we have local variables, then we create a local environment.

		if (AST.GetVarDefCount() > 0)
			{
			m_Code.WriteShortOpCode(opMakeBlockEnv);
			}

		//	Compile statements

		for (int i = 0; i < AST.GetStatementCount(); i++)
			{
			if (!CompileStatement(AST.GetStatement(i), retsError))
				return false;
			}

		if (AST.GetVarDefCount() > 0)
			{
			m_Code.WriteShortOpCode(opExitEnv);
			}
		}

	return true;
	}

//	Main Compile Functions -----------------------------------------------------
//
//	The functions in this section compile specific ASTs given a compiler context
//	block.

bool CGridLangVMCompiler::CompileExpression (const IASTNode &AST, CString *retsError)

//	CompileExpression
//
//	Compiles an expression.

	{
	switch (AST.GetType())
		{
		case EASTType::LiteralString:
			{
			int iDataBlock = m_Code.CreateDataBlock(AST.GetValue());
			m_Code.WriteShortOpCode(opPushStr, iDataBlock);
			break;
			}

		case EASTType::FunctionCall:
			{
			if (!CompileFunctionCall(AST, retsError))
				return false;
			break;
			}

		case EASTType::VarRef:
			{
			if (!CompileVariableReference(AST, retsError))
				return false;
			break;
			}

		default:
			return ComposeError(strPattern("Not an expression: %s", AST.GetTypeName()), retsError);
		}

	return true;
	}

bool CGridLangVMCompiler::CompileFunctionCall (const IASTNode &AST, CString *retsError)

//	CompileFunctionCall
//
//	Compiles a function call.

	{
	//	Compile the function name as an expression. I.e., we support function
	//	indirection, etc. here.

	if (!CompileExpression(AST.GetRoot(), retsError))
		return false;

	//	Now push the arguments of the function.

	for (int i = 0; i < AST.GetChildCount(); i++)
		{
		if (!CompileExpression(AST.GetChild(i), retsError))
			return false;
		}

	//	Create the environment and call the function

	m_Code.WriteShortOpCode(opMakeEnv, AST.GetChildCount());
	m_Code.WriteShortOpCode(opCall);

	return true;
	}

bool CGridLangVMCompiler::CompileStatement (const IASTNode &AST, CString *retsError)

//	CompileStatement
//
//	Compiles a statement to the current block.

	{
	switch (AST.GetType())
		{
		case EASTType::FunctionCall:
			if (!CompileFunctionCall(AST, retsError))
				return false;
			break;

		case EASTType::VarDef:
			if (!CompileVariableDefinition(AST, retsError))
				return false;
			break;

		default:
			return ComposeError(strPattern("Not a statement: %s", AST.GetTypeName()), retsError);
		}

	return true;
	}

bool CGridLangVMCompiler::CompileVariableDefinition (const IASTNode &AST, CString *retsError)

//	CompileVariableDefinition
//
//	Compiles a variable definition and assignment.

	{
	//	If we have a value to assign it to, compile it now.

	if (AST.GetChildCount() > 0)
		{
		if (!CompileExpression(AST.GetChild(0), retsError))
			return false;
		}

	//	Otherwise, push nil

	else
		{
		m_Code.WriteShortOpCode(opPushNil);
		}

	//	Define it

	int iSymbol = m_Code.CreateDataBlock(AST.GetName());
	m_Code.WriteShortOpCode(opDefineArg, iSymbol);

	DWORD dwArgPos = (((DWORD)0) << 8) | ((DWORD)AST.GetCodeID());
	m_Code.WriteShortOpCode(opPopLocal, dwArgPos);

	return true;
	}

bool CGridLangVMCompiler::CompileVariableReference (const IASTNode &AST, CString *retsError)

//	CompileVariableReference
//
//	Compiles a variable reference.

	{
#if 0
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
#endif

	return true;
	}

//	Utility Functions ----------------------------------------------------------

bool CGridLangVMCompiler::ComposeError (const CString &sError, CString *retsError) const

//	ComposeError
//
//	Compose and error.

	{
	if (retsError)
		*retsError = sError;

	return false;
	}
