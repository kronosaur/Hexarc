//	CLispCompiler.cpp
//
//	CListCompiler class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_NIL,						"nil")
DECLARE_CONST_STRING(STR_TRUE,						"true")
DECLARE_CONST_STRING(STR_QUOTE,						"quote")

DECLARE_CONST_STRING(OPTIONS_EXCLUDE_NIL,			"excludeNil")
DECLARE_CONST_STRING(OPTIONS_ORIGINAL,				"original")

DECLARE_CONST_STRING(PRIMITIVE_INVOKE,				"invoke")

DECLARE_CONST_STRING(SPECIAL_ADD,					"+")
DECLARE_CONST_STRING(SPECIAL_DIVIDE,				"/")
DECLARE_CONST_STRING(SPECIAL_MULTIPLY,				"*")
DECLARE_CONST_STRING(SPECIAL_SUBTRACT,				"-")
DECLARE_CONST_STRING(SPECIAL_IS_EQUAL,				"=")
DECLARE_CONST_STRING(SPECIAL_IS_GREATER,			">")
DECLARE_CONST_STRING(SPECIAL_IS_LESS,				"<")
DECLARE_CONST_STRING(SPECIAL_IS_GREATER_OR_EQUAL,	">=")
DECLARE_CONST_STRING(SPECIAL_IS_LESS_OR_EQUAL,		"<=")
DECLARE_CONST_STRING(SPECIAL_IS_NOT_EQUAL,			"!=")
DECLARE_CONST_STRING(SPECIAL_AND,					"&&")
DECLARE_CONST_STRING(SPECIAL_OR,					"||")
DECLARE_CONST_STRING(SPECIAL_APPLY,					"apply")
DECLARE_CONST_STRING(SPECIAL_BLOCK,					"block")
DECLARE_CONST_STRING(SPECIAL_DEFINE,				"define")
DECLARE_CONST_STRING(SPECIAL_ENUM,					"enum")
DECLARE_CONST_STRING(SPECIAL_ERROR,					"error")
DECLARE_CONST_STRING(SPECIAL_IF,					"if")
DECLARE_CONST_STRING(SPECIAL_INVOKE,				"invoke")
DECLARE_CONST_STRING(SPECIAL_LAMBDA,				"lambda")
DECLARE_CONST_STRING(SPECIAL_LIST,					"list")
DECLARE_CONST_STRING(SPECIAL_MAP,					"map")
DECLARE_CONST_STRING(SPECIAL_NOT,					"not")
DECLARE_CONST_STRING(SPECIAL_QUOTE,					"quote")
DECLARE_CONST_STRING(SPECIAL_SET_AT,				"set@")
DECLARE_CONST_STRING(SPECIAL_SET_BANG,				"set!")
DECLARE_CONST_STRING(SPECIAL_SWITCH,				"switch")
DECLARE_CONST_STRING(SPECIAL_WHILE,					"while")

DECLARE_CONST_STRING(SPECIAL_VAR_I,					"$i")
DECLARE_CONST_STRING(SPECIAL_VAR_LIST,				"$list")
DECLARE_CONST_STRING(SPECIAL_VAR_OPTIONS,			"$options")
DECLARE_CONST_STRING(SPECIAL_VAR_RESULT,			"$result")

DECLARE_CONST_STRING(ERR_INVALID_INVOCATION,		"Cannot invoke: %s.")
DECLARE_CONST_STRING(ERR_CLOSE_PAREN_EXPECTED,		"Expected ')'.")
DECLARE_CONST_STRING(ERR_COLON_EXPECTED,			"Expected ':'.")
DECLARE_CONST_STRING(ERR_ARG_LIST_EXPECTED,			"Expected argument list.")
DECLARE_CONST_STRING(ERR_EXPRESSION_EXPECTED,		"Expression expected.")
DECLARE_CONST_STRING(ERR_IDENTIFIER_EXPECTED,		"Identifier expected: %s.")
DECLARE_CONST_STRING(ERR_LOCAL_VARIABLES_EXPECTED,	"Local variables expected.")
DECLARE_CONST_STRING(ERR_NOT_ENOUGH_ARGS,			"Not enough arguments for function: %s.")
DECLARE_CONST_STRING(ERR_NOT_ENOUGH_ARGS_0,			"Not enough arguments for function.")
DECLARE_CONST_STRING(ERR_TOO_MANY_ARGS,				"Too many arguments for function: %s.")
DECLARE_CONST_STRING(ERR_TOO_MANY_ARGS_0,			"Too many arguments for function.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_CLOSE_PAREN,	"Unexpected ')'.")
DECLARE_CONST_STRING(ERR_EOF,						"Unexpected end of file.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_TOKEN,			"Unexpected token: %s.")
DECLARE_CONST_STRING(ERR_ENUM_USER_VAR,				"enum variable: %s")
DECLARE_CONST_STRING(ERR_MAP_USER_VAR,				"map variable: %s")

bool CLispCompiler::CompileExpression (int iBlock)

//	CompileExpression
//
//	Compiles an expression of the form:
//
//	(function ...)
//	symbol
//	literal string
//	literal integer
//	literal datum
//
//	The result ends at the top of the stack.
//
//	Expects the current token to be the first token of the expression and
//	leaves the token at the last token of the expression.

	{
	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.GetToken(&dToken);

	switch (iToken)
		{
		case CLispParser::tkOpenParen:
			return CompileFunctionCall(iBlock);

		case CLispParser::tkIdentifier:
			return CompileIdentifier(iBlock);

		case CLispParser::tkStringDatum:
			{
			//	Optimize empty strings

			if (((const CString &)dToken).IsEmpty())
				m_pCode->WriteShortOpCode(iBlock, opPushStrNull);

			//	Allocate a literal string in the code block

			else
				{
				int iDataBlock = m_pCode->CreateDatumBlock(dToken);
				m_pCode->WriteShortOpCode(iBlock, opPushStr, iDataBlock);
				}

			return true;
			}

		case CLispParser::tkIntegerDatum:
			{
			DWORD dwValue = (DWORD)(int)dToken;
			DWORD dwHighByte = (dwValue & 0xff000000);

			//	If the value fits in a short opcode, use that

			if (dwHighByte == 0 || dwHighByte == 0xff000000)
				m_pCode->WriteShortOpCode(iBlock, opPushIntShort, dwValue);

			//	Otherwise, long

			else
				m_pCode->WriteLongOpCode(iBlock, opPushInt, dwValue);

			return true;
			}

		case CLispParser::tkIPIntegerDatum:
		case CLispParser::tkFloatDatum:
			{
			int iDataBlock = m_pCode->CreateDatumBlock(dToken);
			m_pCode->WriteShortOpCode(iBlock, opPushDatum, iDataBlock);
			return true;
			}

		case CLispParser::tkOpenBrace:
			return CompileStruct(iBlock);

		case CLispParser::tkQuote:
			{
			m_Parser.ParseToken(&dToken);

			if (!CompileLiteralExpression(iBlock))
				return false;

			return true;
			}

		case CLispParser::tkError:
			m_sError = m_Parser.ComposeError(dToken);
			return false;

		case CLispParser::tkEOF:
			m_sError = m_Parser.ComposeError(ERR_EOF);
			return false;

		default:
			m_sError = m_Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, (const CString &)dToken));
			return false;
		}
	}

bool CLispCompiler::CompileFunctionCall (int iBlock)

//	CompileFunctionCall
//
//	Compiles a function call into the given block.
//
//	The current token is the open paren. We leave the token at the last
//	token of the expression (i.e., the close paren).

	{
	//	Skip the open paren and get the next token (the function identifier)

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	//	If we have a close paren then treat this as a literal nil

	if (iToken == CLispParser::tkCloseParen)
		{
		m_pCode->WriteShortOpCode(iBlock, opPushNil);
		return true;
		}

	//	Compile some special forms
	//
	//	(and {expr} ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_AND))
		return CompileSpecialFormAnd(iBlock);

	//	(apply ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_APPLY))
		return CompileSpecialFormApply(iBlock);

	//	(block {locals} ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_BLOCK))
		return CompileSpecialFormBlock(iBlock);

	//	(define {symbol} {value})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_DEFINE))
		return CompileSpecialFormDefine(iBlock);

	//	(enum {list} {var} {expr})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_ENUM))
		return CompileSpecialFormEnum(iBlock);

	//	(error {errorCode} {errorDesc})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_ERROR))
		return CompileSpecialFormError(iBlock);

	//	(if {cond} {then} {else})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_IF))
		return CompileSpecialFormIf(iBlock);

	//	(invoke {msg} ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_INVOKE))
		return CompileSpecialFormInvoke(iBlock);

	//	(lambda {args} {code})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_LAMBDA))
		return CompileSpecialFormLambda(iBlock);

	//	(list ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_LIST))
		return CompileSpecialFormListOp(iBlock, opMakeArray);

	//	(map {list} {options} {var} {expr})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_MAP))
		return CompileSpecialFormMap(iBlock);

	//	(not expr)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_NOT))
		return CompileSpecialFormNot(iBlock);

	//	(or {expr} ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_OR))
		return CompileSpecialFormOr(iBlock);

	//	(quote ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_QUOTE))
		return CompileSpecialFormQuote(iBlock);

	//	(set@ {var} {key} {expr})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_SET_AT))
		return CompileSpecialFormSetAt(iBlock);

	//	(set! {var} {expr})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_SET_BANG))
		return CompileSpecialFormSetBang(iBlock);

	//	(switch {cond} {then} [{cond} {then} ... [{else}]])

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_SWITCH))
		return CompileSpecialFormSwitch(iBlock);

	//	(while {cond} {expr})

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_WHILE))
		return CompileSpecialFormWhile(iBlock);

	//	(+ ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_ADD))
		return CompileSpecialFormListOp(iBlock, opAdd);

	//	(/ ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_DIVIDE))
		return CompileSpecialFormListOp(iBlock, opDivide);

	//	(* ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_MULTIPLY))
		return CompileSpecialFormListOp(iBlock, opMultiply);

	//	(- ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_SUBTRACT))
		return CompileSpecialFormListOp(iBlock, opSubtract);

	//	(= ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_IS_EQUAL))
		return CompileSpecialFormListOp(iBlock, opIsEqual);

	//	(< ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_IS_LESS))
		return CompileSpecialFormListOp(iBlock, opIsLess);

	//	(> ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_IS_GREATER))
		return CompileSpecialFormListOp(iBlock, opIsGreater);

	//	(<= ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_IS_LESS_OR_EQUAL))
		return CompileSpecialFormListOp(iBlock, opIsLessOrEqual);

	//	(>= ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_IS_GREATER_OR_EQUAL))
		return CompileSpecialFormListOp(iBlock, opIsGreaterOrEqual);

	//	(!= ...)

	else if (iToken == CLispParser::tkIdentifier && strEquals(dToken, SPECIAL_IS_NOT_EQUAL))
		return CompileSpecialFormListOp(iBlock, opIsNotEqual);

	//	Otherwise, compile the first element as an expression

	else
		{
		if (!CompileExpression(iBlock))
			return false;
		}

	//	Push the arguments of the function (keeping track of how many we push)

	int iCount = 0;
	iToken = m_Parser.ParseToken(&dToken);
	while (iToken != CLispParser::tkCloseParen)
		{
		iCount++;

		if (!CompileExpression(iBlock))
			return false;

		iToken = m_Parser.ParseToken(&dToken);
		}

	//	Create the environment

	m_pCode->WriteShortOpCode(iBlock, opMakeEnv, iCount);

	//	Call

	m_pCode->WriteShortOpCode(iBlock, opCall);

	//	Done

	return true;
	}

bool CLispCompiler::CompileIdentifier (int iBlock)

//	CompileIdentifier
//
//	Compiles an identifier lookup

	{
	CDatum dToken;
	m_Parser.GetToken(&dToken);

	int iLevel;
	int iIndex;

	//	Check some special identifiers

	if (IsNilSymbol(dToken))
		m_pCode->WriteShortOpCode(iBlock, opPushNil);

	else if (IsTrueSymbol(dToken))
		m_pCode->WriteShortOpCode(iBlock, opPushTrue);

	//	Check primitives

	else if (strEquals(dToken, PRIMITIVE_INVOKE))
		m_pCode->WriteShortOpCode(iBlock, opMakePrimitive, (DWORD)CDatum::ECallType::Invoke);

	//	Look for this argument in the local environment

	else if (m_pCurrentEnv && m_pCurrentEnv->FindArgument(dToken, &iLevel, &iIndex))
		{
		//	Encode the level and index

		DWORD dwArgPos = (((DWORD)iLevel) << 8) | ((DWORD)iIndex);

		//	Add the opcode

		m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);
		}

	//	Otherwise, it must be a global variable

	else
		{
		int iSymbol = m_pCode->CreateDatumBlock(dToken);

		//	Add the lookup opcode

		m_pCode->WriteShortOpCode(iBlock, opPushGlobal, iSymbol);
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileLiteralExpression (int iBlock)

//	CompileLiteralExpression
//
//	Pushes a literal expression

	{
	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.GetToken(&dToken);

	switch (iToken)
		{
		case CLispParser::tkOpenParen:
			{
			//	Make a literal list

			int iCount = 0;
			iToken = m_Parser.ParseToken(&dToken);
			while (iToken != CLispParser::tkCloseParen)
				{
				iCount++;

				if (!CompileLiteralExpression(iBlock))
					return false;

				iToken = m_Parser.ParseToken(&dToken);
				}

			//	Make the list

			m_pCode->WriteShortOpCode(iBlock, opMakeArray, iCount);

			return true;
			}

		case CLispParser::tkIdentifier:
		case CLispParser::tkStringDatum:
			{
			//	Check some special identifiers (which always evaluate to 
			//	themselves)

			if (IsNilSymbol(dToken))
				m_pCode->WriteShortOpCode(iBlock, opPushNil);

			else if (IsTrueSymbol(dToken))
				m_pCode->WriteShortOpCode(iBlock, opPushTrue);

			//	Optimize empty strings

			else if (((const CString &)dToken).IsEmpty())
				m_pCode->WriteShortOpCode(iBlock, opPushStrNull);

			//	Allocate a literal string in the code block

			else
				{
				int iDataBlock = m_pCode->CreateDatumBlock(dToken);
				m_pCode->WriteShortOpCode(iBlock, opPushStr, iDataBlock);
				}

			return true;
			}

		case CLispParser::tkIntegerDatum:
			{
			DWORD dwValue = (DWORD)(int)dToken;
			DWORD dwHighByte = (dwValue & 0xff000000);

			//	If the value fits in a short opcode, use that

			if (dwHighByte == 0 || dwHighByte == 0xff000000)
				m_pCode->WriteShortOpCode(iBlock, opPushIntShort, dwValue);

			//	Otherwise, long

			else
				m_pCode->WriteLongOpCode(iBlock, opPushInt, dwValue);

			return true;
			}

		case CLispParser::tkIPIntegerDatum:
		case CLispParser::tkFloatDatum:
			{
			int iDataBlock = m_pCode->CreateDatumBlock(dToken);
			m_pCode->WriteShortOpCode(iBlock, opPushDatum, iDataBlock);
			return true;
			}

		case CLispParser::tkOpenBrace:
			return CompileLiteralStruct(iBlock);

		case CLispParser::tkQuote:
			{
			//	Make a list of the form "(quote {expr})"
			//	
			//	First push the "quote" identifier

			int iDataBlock = m_pCode->CreateDatumBlock(STR_QUOTE);
			m_pCode->WriteShortOpCode(iBlock, opPushStr, iDataBlock);

			//	Now push the expression

			iToken = m_Parser.ParseToken(&dToken);
			if (!CompileLiteralExpression(iBlock))
				return false;

			//	Make the list

			m_pCode->WriteShortOpCode(iBlock, opMakeArray, 2);

			return true;
			}

		case CLispParser::tkError:
			m_sError = m_Parser.ComposeError(dToken);
			return false;

		case CLispParser::tkEOF:
			m_sError = m_Parser.ComposeError(ERR_EOF);
			return false;

		default:
			m_sError = m_Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, (const CString &)dToken));
			return false;
		}
	}

bool CLispCompiler::CompileLiteralStruct (int iBlock)

//	CompileLiteralStruct
//
//	{ key:value key:value ... }

	{
	//	Keep track of the number of key/value pairs

	int iCount = 0;

	//	Loop over all key/value pairs

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	while (iToken != CLispParser::tkCloseBrace)
		{
		//	We expect this to be the key (an identifier). If not, then error.

		if (iToken != CLispParser::tkIdentifier)
			{
			m_sError = m_Parser.ComposeError(strPattern(ERR_IDENTIFIER_EXPECTED, dToken.AsString()));
			return false;
			}

		//	Push the key as a string

		int iDataBlock = m_pCode->CreateDatumBlock(dToken);
		m_pCode->WriteShortOpCode(iBlock, opPushStr, iDataBlock);

		//	Parse a colon

		iToken = m_Parser.ParseToken(&dToken);
		if (iToken != CLispParser::tkColon)
			{
			m_sError = m_Parser.ComposeError(ERR_COLON_EXPECTED);
			return false;
			}

		//	Parse and push the expression (the value)

		iToken = m_Parser.ParseToken(&dToken);
		if (!CompileLiteralExpression(iBlock))
			return false;

		//	Next

		iCount++;
		iToken = m_Parser.ParseToken(&dToken);
		}

	//	Make the structure

	m_pCode->WriteShortOpCode(iBlock, opMakeStruct, iCount);

	//	Done

	return true;
	}

bool CLispCompiler::CompileLocalDef (int iBlock, int iVarPos, const CString &sVar)

//	CompileLocalDef
//
//	We expect the top of the stack to contain the value to be assigned to this
//	variable.
//
//	We parse an identifier, define it, and assign it.

	{
	//	If we don't have a variable name then we need to parse it.

	CDatum dToken;
	if (sVar.IsEmpty())
		{
		CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
		if (iToken != CLispParser::tkIdentifier)
			{
			m_sError = strPattern(ERR_IDENTIFIER_EXPECTED, dToken.AsString());
			return false;
			}
		}
	else
		dToken = CDatum(sVar);

	//	Add to environment

	m_pCurrentEnv->SetElement((const CString &)dToken, CDatum());

	//	Define the variable

	int iSymbol = m_pCode->CreateDatumBlock(dToken);
	m_pCode->WriteShortOpCode(iBlock, opDefineArg, iSymbol);

	DWORD dwArgPos = (((DWORD)0) << 8) | ((DWORD)iVarPos);
	m_pCode->WriteShortOpCode(iBlock, opPopLocal, dwArgPos);

	//	Done

	return true;
	}

bool CLispCompiler::CompileLocalDefExpression (int iBlock, int iVarPos, const CString &sVar)

//	CompileLocalDefExpression
//
//	Compiles an expression and assigns it to the given local variable.

	{
	//	Define the local variable in the environment

	m_pCurrentEnv->SetElement(sVar, CDatum());

	//	Compile the expression

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	if (!CompileExpression(iBlock))
		return false;

	//	Assign the result of the expression to the variable.

	int iSymbol = m_pCode->CreateDatumBlock(sVar);
	m_pCode->WriteShortOpCode(iBlock, opDefineArg, iSymbol);

	DWORD dwArgPos = (((DWORD)0) << 8) | ((DWORD)iVarPos);
	m_pCode->WriteShortOpCode(iBlock, opPopLocal, dwArgPos);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormAnd (int iBlock)

//	CompileSpecialFormAnd
//
//	(and {expr} ...)

	{
	int i;
	TArray<int> Jumps;

	//	Write out all the expressions

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkCloseParen)
		m_pCode->WriteShortOpCode(iBlock, opPushNil);
	else
		{
		while (iToken != CLispParser::tkCloseParen)
			{
			if (!CompileExpression(iBlock))
				return false;

			//	If nil then we short-circuit to the end (keeping track of the
			//	position so we can fix it up later).

			Jumps.Insert(m_pCode->GetCodeBlockPos(iBlock));
			m_pCode->WriteShortOpCode(iBlock, opJumpIfNilNoPop);

			//	Next token. If have more expressions then pop the previous one
			//	off the stack.

			iToken = m_Parser.ParseToken(&dToken);
			if (iToken != CLispParser::tkCloseParen)
				m_pCode->WriteShortOpCode(iBlock, opPop, 1);
			}
		}

	//	At this point we're left with either nil or a true expression on the
	//	stack.

	int iEnd = m_pCode->GetCodeBlockPos(iBlock);

	//	Fix up all the jumps to this point

	for (i = 0; i < Jumps.GetCount(); i++)
		{
		int iJumpOffset = (iEnd - Jumps[i]) / sizeof(DWORD);
		m_pCode->RewriteShortOpCode(iBlock, Jumps[i], opJumpIfNilNoPop, iJumpOffset);
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormApply (int iBlock)

//	CompileSpecialFormApply
//
//	(apply func [exp1 ...] list)

	{
	//	Push the function on the stack

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Push args (including the list at the end)

	int iCount = 0;
	iToken = m_Parser.ParseToken(&dToken);
	while (iToken != CLispParser::tkCloseParen)
		{
		iCount++;

		if (!CompileExpression(iBlock))
			return false;

		iToken = m_Parser.ParseToken(&dToken);
		}

	//	Must have at least the list

	if (iCount < 1)
		{
		m_sError = m_Parser.ComposeError(strPattern(ERR_NOT_ENOUGH_ARGS, SPECIAL_APPLY));
		return false;
		}

	//	Create the environment

	m_pCode->WriteShortOpCode(iBlock, opMakeApplyEnv, iCount);

	//	Call

	m_pCode->WriteShortOpCode(iBlock, opCall);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormBlock (int iBlock)

//	CompileSpecialFormBlock
//
//	Compiles (block (locals) ...)

	{
	//	Create an environment

	EnterLocalEnvironment();

	m_pCode->WriteShortOpCode(iBlock, opMakeBlockEnv);

	//	Push the value of the locals (while separately keeping track of the
	//	variable names).

	int iCount = 0;

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	//	If Nil, then we have no locals

	if (iToken == CLispParser::tkIdentifier && IsNilSymbol(dToken))
		;

	//	Otherwise, if paren, then we have a list of locals

	else if (iToken == CLispParser::tkOpenParen)
		{
		iToken = m_Parser.ParseToken(&dToken);

		while (iToken != CLispParser::tkCloseParen)
			{
			//	If open paren, then this declares a local variable with a value

			if (iToken == CLispParser::tkOpenParen)
				{
				//	First is an identifier

				iToken = m_Parser.ParseToken(&dToken);
				if (iToken != CLispParser::tkIdentifier)
					{
					m_sError = m_Parser.ComposeError(strPattern(ERR_IDENTIFIER_EXPECTED, dToken.AsString()));
					return false;
					}

				//	Add to environment

				m_pCurrentEnv->SetElement((const CString &)dToken, CDatum());

				//	Compile the value of the variable

				iToken = m_Parser.ParseToken(&dToken);
				if (iToken == CLispParser::tkCloseParen)
					{
					m_sError = m_Parser.ComposeError(ERR_EXPRESSION_EXPECTED);
					return false;
					}

				//	Push the expression as a value

				if (!CompileExpression(iBlock))
					return false;

				//	Define it

				int iSymbol = m_pCode->CreateDatumBlock(dToken);
				m_pCode->WriteShortOpCode(iBlock, opDefineArg, iSymbol);

				DWORD dwArgPos = (((DWORD)0) << 8) | ((DWORD)iCount);
				m_pCode->WriteShortOpCode(iBlock, opPopLocal, dwArgPos);

				//	Close paren

				iToken = m_Parser.ParseToken(&dToken);
				if (iToken != CLispParser::tkCloseParen)
					{
					m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
					return false;
					}
				}
			
			//	Otherwise, this better be an identifier

			else if (iToken == CLispParser::tkIdentifier)
				{
				//	Add to environment

				m_pCurrentEnv->SetElement((const CString &)dToken, CDatum());

				//	Push Nil since this variable has no value

				m_pCode->WriteShortOpCode(iBlock, opPushNil);

				//	Define it

				int iSymbol = m_pCode->CreateDatumBlock(dToken);
				m_pCode->WriteShortOpCode(iBlock, opDefineArg, iSymbol);

				DWORD dwArgPos = (((DWORD)0) << 8) | ((DWORD)iCount);
				m_pCode->WriteShortOpCode(iBlock, opPopLocal, dwArgPos);
				}

			//	Else error

			else
				{
				m_sError = m_Parser.ComposeError(strPattern(ERR_IDENTIFIER_EXPECTED, dToken.AsString()));
				return false;
				}

			//	Next

			iCount++;
			iToken = m_Parser.ParseToken(&dToken);
			}
		}

	//	Anything else is an error

	else
		{
		m_sError = m_Parser.ComposeError(ERR_LOCAL_VARIABLES_EXPECTED);
		return false;
		}

	//	Now compile the individual statements

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkCloseParen)
		m_pCode->WriteShortOpCode(iBlock, opPushNil);
	else
		{
		while (iToken != CLispParser::tkCloseParen)
			{
			if (!CompileExpression(iBlock))
				return false;

			iToken = m_Parser.ParseToken(&dToken);

			//	Pop the result off the stack

			if (iToken != CLispParser::tkCloseParen)
				m_pCode->WriteShortOpCode(iBlock, opPop, 1);
			}
		}

	//	Exit the environment

	m_pCode->WriteShortOpCode(iBlock, opExitEnv);

	ExitLocalEnvironment();

	return true;
	}

bool CLispCompiler::CompileSpecialFormDefine (int iBlock)

//	CompileSpecialFormDefine
//
//	Compiles (define ...)

	{
	//	We expect a valid symbol

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkError)
		{
		m_sError = m_Parser.ComposeError(dToken);
		return false;
		}
	else if (iToken != CLispParser::tkIdentifier)
		{
		m_sError = m_Parser.ComposeError(strPattern(ERR_IDENTIFIER_EXPECTED, dToken.AsString()));
		return false;
		}

	//	Write out the symbol

	int iSymbol = m_pCode->CreateDatumBlock(dToken);
	m_Parser.ParseToken();

	//	Compile the expression

	if (!CompileExpression(iBlock))
		return false;

	//	Add a define opcode

	m_pCode->WriteShortOpCode(iBlock, opDefine, iSymbol);

	//	Parse the last closing paren

	iToken = m_Parser.ParseToken();
	if (iToken != CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
		return false;
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormEnum (int iBlock)

//	CompileSpecialFormEnum
//
//	(enum {list} {var} {expr})

	{
	//	Create an environment

	EnterLocalEnvironment();

	m_pCode->WriteShortOpCode(iBlock, opMakeBlockEnv);

	//	We're going to create three local variables:
	//
	//	$list is the value of the list
	//	$i is the current iteration position
	//	{var} is the variable that will get the list element values

	const int LIST_VAR = 0;
	const int USER_VAR = 1;
	const int I_VAR = 2;

	//	We store the list in a local variable named $list.

	if (!CompileLocalDefExpression(iBlock, LIST_VAR, SPECIAL_VAR_LIST))
		return false;

	//	Parse the user variable and assign it to nil

	m_pCode->WriteShortOpCode(iBlock, opPushNil);
	if (!CompileLocalDef(iBlock, USER_VAR))
		{
		m_sError = m_Parser.ComposeError(strPattern(ERR_ENUM_USER_VAR, m_sError));
		return false;
		}

	//	Assign $i to 0

	m_pCode->WriteShortOpCode(iBlock, opPushIntShort, 0);
	if (!CompileLocalDef(iBlock, I_VAR, SPECIAL_VAR_I))
		{
		m_sError = m_Parser.ComposeError(m_sError);
		return false;
		}

	//	Push nil as a result in case of empty loop.

	m_pCode->WriteShortOpCode(iBlock, opPushNil);

	//	This is the top of the loop so we remember its position

	int iBeginLoop = m_pCode->GetCodeBlockPos(iBlock);

	//	Push the index

	DWORD dwArgPos = (((DWORD)0) << 8) | ((DWORD)I_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);

	//	Push the length of the list

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)LIST_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocalLength, dwArgPos);

	//	Less than?

	m_pCode->WriteShortOpCode(iBlock, opIsLess, 2);

	//	If not, then jump to the end
	//	
	//	Write out a jumpIfNil and remember the current position
	//	because we need to go back and set the proper jump distance

	int iJumpIfPos = m_pCode->GetCodeBlockPos(iBlock);
	m_pCode->WriteShortOpCode(iBlock, opJumpIfNil);

	//	Pop the previous result (or the nil that we pushed at the beginning)

	m_pCode->WriteShortOpCode(iBlock, opPop, 1);

	//	Push the index

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)I_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);

	//	Push the ith element of the list

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)LIST_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocalItem, dwArgPos);

	//	Assign it to the user variable

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)USER_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPopLocal, dwArgPos);

	//	Now compile the body of the map.

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Parse the closing paren

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken != CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
		return false;
		}

	//	Increment $i

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)I_VAR);
	m_pCode->WriteShortOpCode(iBlock, opIncLocalInt, dwArgPos);

	//	Loop back to the beginning

	int iJumpOffset = (iBeginLoop - m_pCode->GetCodeBlockPos(iBlock)) / sizeof(DWORD);
	m_pCode->WriteShortOpCode(iBlock, opJump, iJumpOffset);

	//	Now we can fix up the jump at the top of the loop

	iJumpOffset = (m_pCode->GetCodeBlockPos(iBlock) - iJumpIfPos) / sizeof(DWORD);
	m_pCode->RewriteShortOpCode(iBlock, iJumpIfPos, opJumpIfNil, iJumpOffset);

	//	The last body result is on the stack
	//	Exit the environment

	m_pCode->WriteShortOpCode(iBlock, opExitEnv);

	ExitLocalEnvironment();

	return true;
	}

bool CLispCompiler::CompileSpecialFormError (int iBlock)

//	CompileSpecialFormError
//
//	(error {errorCode} {errorDesc})

	{
	//	Keep parsing each operand and pushing it on the stack

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	int iCount = 0;
	while (iToken != CLispParser::tkCloseParen)
		{
		iCount++;

		if (!CompileExpression(iBlock))
			return false;

		iToken = m_Parser.ParseToken(&dToken);
		}

	//	Make sure we have the proper number of parameters

	if (iCount > 2)
		{
		m_sError = m_Parser.ComposeError(strPattern(ERR_TOO_MANY_ARGS_0, SPECIAL_ERROR));
		return false;
		}

	//	Generate an instruction (with a count of the number of args)

	m_pCode->WriteShortOpCode(iBlock, opError, iCount);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormIf (int iBlock)

//	CompileSpecialFormIf
//
//	(if {cond} {then} {else})

	{
	//	Push the condition on the stack

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Write out a jumpIfNil and remember the current position
	//	because we need to go back and set the proper jump distance

	int iJumpIfPos = m_pCode->GetCodeBlockPos(iBlock);
	m_pCode->WriteShortOpCode(iBlock, opJumpIfNil);

	//	Now write out the then expression

	iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	We need to write out a jump instruction to skip the else clause
	//	(Note that we need this even if there is no else clause because
	//	we push Nil in that case).

	int iJumpPos = m_pCode->GetCodeBlockPos(iBlock);
	m_pCode->WriteShortOpCode(iBlock, opJump);

	//	Now we know how far the if needs to jump (in DWORDs)

	int iJumpOffset = (m_pCode->GetCodeBlockPos(iBlock) - iJumpIfPos) / sizeof(DWORD);
	m_pCode->RewriteShortOpCode(iBlock, iJumpIfPos, opJumpIfNil, iJumpOffset);

	//	Now write out the else clause. If we have no else clause
	//	then we push nil

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken != CLispParser::tkCloseParen)
		{
		if (!CompileExpression(iBlock))
			return false;

		//	Read the close paren

		iToken = m_Parser.ParseToken();
		if (iToken != CLispParser::tkCloseParen)
			{
			m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
			return false;
			}
		}
	else
		m_pCode->WriteShortOpCode(iBlock, opPushNil);

	//	Now write out the other jump

	iJumpOffset = (m_pCode->GetCodeBlockPos(iBlock) - iJumpPos) / sizeof(DWORD);
	m_pCode->RewriteShortOpCode(iBlock, iJumpPos, opJump, iJumpOffset);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormInvoke (int iBlock)

//	CompileSpecialFormInvoke
//
//	(invoke {msg} ...)

	{
	//	Parse the message and push it on the stack

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Keep parsing each operand and pushing it on the stack
	//	Keeping track of the number of arguments

	iToken = m_Parser.ParseToken(&dToken);

	int iCount = 0;
	while (iToken != CLispParser::tkCloseParen)
		{
		iCount++;

		if (!CompileExpression(iBlock))
			return false;

		iToken = m_Parser.ParseToken(&dToken);
		}

	//	Turn the arguments into a list (the payload)

	m_pCode->WriteShortOpCode(iBlock, opMakeArray, iCount);

	//	Send the message

	m_pCode->WriteShortOpCode(iBlock, opHexarcMsg);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormLambda (int iBlock)

//	CompileSpecialFormLambda
//
//	(lambda {args} {code})

	{
	//	Allocate a new code block

	int iLambdaBlock = m_pCode->CreateCodeBlock();

	//	Write the code to create a function in this block

	m_pCode->WriteShortOpCode(iBlock, opMakeFunc, iLambdaBlock);

	//	Now begin writing the new function. We start with an opcode
	//	to enter the environment.

	m_pCode->WriteShortOpCode(iLambdaBlock, opEnterEnv);

	//	Create an environment

	EnterLocalEnvironment();

	//	Loop over all arg

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkOpenParen)
		{
		iToken = m_Parser.ParseToken(&dToken);
		while (iToken == CLispParser::tkIdentifier)
			{
			//	Add to environment

			m_pCurrentEnv->SetElement((const CString &)dToken, CDatum());

			//	Add a string block

			int iSymbol = m_pCode->CreateDatumBlock(dToken);

			//	Add an argument definition opcode

			m_pCode->WriteShortOpCode(iLambdaBlock, opDefineArg, iSymbol);

			//	Next

			iToken = m_Parser.ParseToken(&dToken);
			}

		//	This better be a close paren

		if (iToken != CLispParser::tkCloseParen)
			{
			m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
			return false;
			}
		}
	else if (iToken == CLispParser::tkIdentifier && IsNilSymbol(dToken))
		{
		//	No args. Skip
		}
	else
		{
		m_sError = m_Parser.ComposeError(ERR_ARG_LIST_EXPECTED);
		return false;
		}

	//	Now compile the body of the function

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkCloseParen)
		m_pCode->WriteShortOpCode(iLambdaBlock, opPushNil);
	else
		{
		while (iToken != CLispParser::tkCloseParen)
			{
			if (!CompileExpression(iLambdaBlock))
				return false;

			iToken = m_Parser.ParseToken(&dToken);

			//	Pop the result off the stack

			if (iToken != CLispParser::tkCloseParen)
				m_pCode->WriteShortOpCode(iLambdaBlock, opPop, 1);
			}
		}

	//	Leave the environment and return

	ExitLocalEnvironment();

	m_pCode->WriteShortOpCode(iLambdaBlock, opExitEnv);
	m_pCode->WriteShortOpCode(iLambdaBlock, opReturn);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormListOp (int iBlock, DWORD dwOpCode, int iMinArgs, int iMaxArgs)

//	CompileSpecialFormListOp
//
//	Compiles:
//
//	(+ ...)
//	(* ...)
//	(- {a} {b})
//	(/ {a} {b})

	{
	//	Keep parsing each operand and pushing it on the stack

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	int iCount = 0;
	while (iToken != CLispParser::tkCloseParen)
		{
		iCount++;

		if (!CompileExpression(iBlock))
			return false;

		iToken = m_Parser.ParseToken(&dToken);
		}

	//	Make sure we have the proper number of parameters

	if (iCount < iMinArgs)
		{
		m_sError = m_Parser.ComposeError(ERR_NOT_ENOUGH_ARGS_0);
		return false;
		}
	else if (iMaxArgs != -1 && iCount > iMaxArgs)
		{
		m_sError = m_Parser.ComposeError(ERR_TOO_MANY_ARGS_0);
		return false;
		}

	//	Generate an instruction (with a count of the number of args)

	m_pCode->WriteShortOpCode(iBlock, dwOpCode, iCount);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormMap (int iBlock)

//	CompileSpecialFormMap
//
//	(map {list} {options} {var} {expr})

	{
	//	Create an environment

	EnterLocalEnvironment();

	m_pCode->WriteShortOpCode(iBlock, opMakeBlockEnv);

	//	We're going to create three local variables:
	//
	//	$list is the value of the list
	//	$i is the current iteration position
	//	$options is a list of option strings
	//	{var} is the variable that will get the list element values
	//	$result is the resulting list (which we build up)

	const int LIST_VAR = 0;
	const int OPTIONS_VAR = 1;
	const int USER_VAR = 2;
	const int I_VAR = 3;
	const int RESULT_VAR = 4;

	//	We store the list in a local variable named $list.

	if (!CompileLocalDefExpression(iBlock, LIST_VAR, SPECIAL_VAR_LIST))
		return false;

	//	Next we push the options expression on the stack

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Push a map of options to flags

	CComplexStruct *pOptionsMap = new CComplexStruct;
	pOptionsMap->SetElement(OPTIONS_EXCLUDE_NIL, (int)CHexeProcess::FLAG_MAP_EXCLUDE_NIL);
	pOptionsMap->SetElement(OPTIONS_ORIGINAL, (int)CHexeProcess::FLAG_MAP_ORIGINAL);

	int iOptionsBlock = m_pCode->CreateDatumBlock(CDatum(pOptionsMap));
	m_pCode->WriteShortOpCode(iBlock, opPushDatum, iOptionsBlock);

	//	Op code to convert the options array to a flags DWORD

	m_pCode->WriteShortOpCode(iBlock, opMakeFlagsFromArray);

	//	Load into $options

	if (!CompileLocalDef(iBlock, OPTIONS_VAR, SPECIAL_VAR_OPTIONS))
		{
		m_sError = m_Parser.ComposeError(m_sError);
		return false;
		}

	//	Parse the user variable and assign it to nil

	m_pCode->WriteShortOpCode(iBlock, opPushNil);
	if (!CompileLocalDef(iBlock, USER_VAR))
		{
		m_sError = m_Parser.ComposeError(strPattern(ERR_MAP_USER_VAR, m_sError));
		return false;
		}

	//	Assign $i to 0

	m_pCode->WriteShortOpCode(iBlock, opPushIntShort, 0);
	if (!CompileLocalDef(iBlock, I_VAR, SPECIAL_VAR_I))
		{
		m_sError = m_Parser.ComposeError(m_sError);
		return false;
		}

	//	Assign $result to a new array

	m_pCode->WriteShortOpCode(iBlock, opPushNil);
	if (!CompileLocalDef(iBlock, RESULT_VAR, SPECIAL_VAR_RESULT))
		{
		m_sError = m_Parser.ComposeError(m_sError);
		return false;
		}

	//	This is the top of the loop so we remember its position

	int iBeginLoop = m_pCode->GetCodeBlockPos(iBlock);

	//	Push the index

	DWORD dwArgPos = (((DWORD)0) << 8) | ((DWORD)I_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);

	//	Push the length of the list

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)LIST_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocalLength, dwArgPos);

	//	Less than?

	m_pCode->WriteShortOpCode(iBlock, opIsLess, 2);

	//	If not, then jump to the end
	//	
	//	Write out a jumpIfNil and remember the current position
	//	because we need to go back and set the proper jump distance

	int iJumpIfPos = m_pCode->GetCodeBlockPos(iBlock);
	m_pCode->WriteShortOpCode(iBlock, opJumpIfNil);

	//	Push the index

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)I_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);

	//	Push the ith element of the list

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)LIST_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocalItem, dwArgPos);

	//	Assign it to the user variable

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)USER_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPopLocal, dwArgPos);

	//	Now compile the body of the map

	iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Parse the closing paren

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken != CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
		return false;
		}

	//	Push the original value, in case we need it

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)USER_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);

	//	Push the options flags so that opMapResult can do the right thing.

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)OPTIONS_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);

	//	Append the output of the expression to the result

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)RESULT_VAR);
	m_pCode->WriteShortOpCode(iBlock, opMapResult, dwArgPos);

	//	Increment $i

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)I_VAR);
	m_pCode->WriteShortOpCode(iBlock, opIncLocalInt, dwArgPos);

	//	Loop back to the beginning

	int iJumpOffset = (iBeginLoop - m_pCode->GetCodeBlockPos(iBlock)) / sizeof(DWORD);
	m_pCode->WriteShortOpCode(iBlock, opJump, iJumpOffset);

	//	Now we can fix up the jump at the top of the loop

	iJumpOffset = (m_pCode->GetCodeBlockPos(iBlock) - iJumpIfPos) / sizeof(DWORD);
	m_pCode->RewriteShortOpCode(iBlock, iJumpIfPos, opJumpIfNil, iJumpOffset);

	//	Push the result

	dwArgPos = (((DWORD)0) << 8) | ((DWORD)RESULT_VAR);
	m_pCode->WriteShortOpCode(iBlock, opPushLocal, dwArgPos);

	//	Exit the environment

	m_pCode->WriteShortOpCode(iBlock, opExitEnv);

	ExitLocalEnvironment();

	return true;
	}

bool CLispCompiler::CompileSpecialFormNot (int iBlock)

//	CompileSpecialFormNot
//
//	(not {expr})

	{
	//	Keep parsing each operand and pushing it on the stack

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	if (!CompileExpression(iBlock))
		return false;

	//	Parse closing paren

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken != CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
		return false;
		}

	//	Generate not instruction

	m_pCode->WriteShortOpCode(iBlock, opNot);

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormOr (int iBlock)

//	CompileSpecialFormOr
//
//	(|| {expr} ...)

	{
	int i;
	TArray<int> Jumps;

	//	Write out all the expressions

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkCloseParen)
		m_pCode->WriteShortOpCode(iBlock, opPushNil);
	else
		{
		while (iToken != CLispParser::tkCloseParen)
			{
			if (!CompileExpression(iBlock))
				return false;

			//	If nil then we short-circuit to the end (keeping track of the
			//	position so we can fix it up later).

			Jumps.Insert(m_pCode->GetCodeBlockPos(iBlock));
			m_pCode->WriteShortOpCode(iBlock, opJumpIfNotNilNoPop);

			//	Next token. If have more expressions then pop the previous one
			//	off the stack.

			iToken = m_Parser.ParseToken(&dToken);
			if (iToken != CLispParser::tkCloseParen)
				m_pCode->WriteShortOpCode(iBlock, opPop, 1);
			}
		}

	//	At this point we're left with either nil or a true expression on the
	//	stack.

	int iEnd = m_pCode->GetCodeBlockPos(iBlock);

	//	Fix up all the jumps to this point

	for (i = 0; i < Jumps.GetCount(); i++)
		{
		int iJumpOffset = (iEnd - Jumps[i]) / sizeof(DWORD);
		m_pCode->RewriteShortOpCode(iBlock, Jumps[i], opJumpIfNotNilNoPop, iJumpOffset);
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormQuote (int iBlock)

//	CompileSpecialFormQuote
//
//	(quote {expr})

	{
	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	if (!CompileLiteralExpression(iBlock))
		return false;

	//	Parse closing paren

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken != CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
		return false;
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormSetAt (int iBlock)

//	CompileSpecialFormSetAt
//
//	(set@ {var} {key} {expr})

	{
	//	Get the variable

	CDatum dVariable;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dVariable);
	if (iToken != CLispParser::tkIdentifier)
		{
		m_sError = m_Parser.ComposeError(strPattern(ERR_IDENTIFIER_EXPECTED, dVariable.AsString()));
		return false;
		}

	//	Parse the key and push it on the stack

	CDatum dToken;
	iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Now parse the expression (the value)

	iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Close paren

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken != CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
		return false;
		}

	//	Look for this argument in the local environment

	int iLevel;
	int iIndex;
	if (m_pCurrentEnv && m_pCurrentEnv->FindArgument(dVariable, &iLevel, &iIndex))
		{
		//	Encode the level and index

		DWORD dwArgPos = (((DWORD)iLevel) << 8) | ((DWORD)iIndex);

		//	Add the opcode

		m_pCode->WriteShortOpCode(iBlock, opSetLocalItem, dwArgPos);
		}

	//	Otherwise, it must be a global variable

	else
		{
		int iSymbol = m_pCode->CreateDatumBlock(dVariable);

		//	Add the lookup opcode

		m_pCode->WriteShortOpCode(iBlock, opSetGlobalItem, iSymbol);
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormSetBang (int iBlock)

//	CompileSpecialForm
//
//	(set! {var} {expr})

	{
	//	Get the variable

	CDatum dVariable;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dVariable);
	if (iToken != CLispParser::tkIdentifier)
		{
		m_sError = m_Parser.ComposeError(strPattern(ERR_IDENTIFIER_EXPECTED, dVariable.AsString()));
		return false;
		}

	CDatum dToken;
	iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_EXPRESSION_EXPECTED);
		return false;
		}

	//	Compile the new value of the variable and push it on the stack.

	if (!CompileExpression(iBlock))
		return false;

	//	Close paren

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken != CLispParser::tkCloseParen)
		{
		m_sError = m_Parser.ComposeError(ERR_CLOSE_PAREN_EXPECTED);
		return false;
		}

	//	Look for this argument in the local environment

	int iLevel;
	int iIndex;
	if (m_pCurrentEnv && m_pCurrentEnv->FindArgument(dVariable, &iLevel, &iIndex))
		{
		//	Encode the level and index

		DWORD dwArgPos = (((DWORD)iLevel) << 8) | ((DWORD)iIndex);

		//	Add the opcode

		m_pCode->WriteShortOpCode(iBlock, opSetLocal, dwArgPos);
		}

	//	Otherwise, it must be a global variable

	else
		{
		int iSymbol = m_pCode->CreateDatumBlock(dVariable);

		//	Add the lookup opcode

		m_pCode->WriteShortOpCode(iBlock, opSetGlobal, iSymbol);
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormSwitch (int iBlock)

//	CompileSpecialFormSwitch
//
//	(switch {cond} {then} ... [{else}])

	{
	int i;
	TArray<int> JumpToEnd;
	bool bHasElse = false;

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);

	while (iToken != CLispParser::tkCloseParen)
		{
		//	Push the condition on the stack

		if (!CompileExpression(iBlock))
			return false;

		//	Get the next token. If we have a close paren then we just compiled
		//	the else clause (instead of a new condition).

		iToken = m_Parser.ParseToken(&dToken);
		if (iToken == CLispParser::tkCloseParen)
			{
			bHasElse = true;
			break;
			}

		//	Write out a jumpIfNil and remember the current position
		//	because we need to go back and set the proper jump distance

		int iJumpIfPos = m_pCode->GetCodeBlockPos(iBlock);
		m_pCode->WriteShortOpCode(iBlock, opJumpIfNil);

		//	Now write out the then expression

		if (!CompileExpression(iBlock))
			return false;

		//	We need to write out a jump instruction to skip the else clause
		//	(Note that we need this even if there is no else clause because
		//	we push Nil in that case).

		JumpToEnd.Insert(m_pCode->GetCodeBlockPos(iBlock));
		m_pCode->WriteShortOpCode(iBlock, opJump);

		//	Now we know how far we need to jump to get to the next condition
		//	(or the else).

		int iJumpOffset = (m_pCode->GetCodeBlockPos(iBlock) - iJumpIfPos) / sizeof(DWORD);
		m_pCode->RewriteShortOpCode(iBlock, iJumpIfPos, opJumpIfNil, iJumpOffset);

		//	Now get the next token.

		iToken = m_Parser.ParseToken(&dToken);
		}

	//	If we have no else then we need to add one

	if (!bHasElse)
		m_pCode->WriteShortOpCode(iBlock, opPushNil);

	//	Write out all jumps

	for (i = 0; i < JumpToEnd.GetCount(); i++)
		{
		int iJumpOffset = (m_pCode->GetCodeBlockPos(iBlock) - JumpToEnd[i]) / sizeof(DWORD);
		m_pCode->RewriteShortOpCode(iBlock, JumpToEnd[i], opJump, iJumpOffset);
		}

	//	Done

	return true;
	}

bool CLispCompiler::CompileSpecialFormWhile (int iBlock)

//	CompileSpecialFormWhile
//
//	(while {cond} {expr})

	{
	//	Push Nil on the stack; If this is an empty loop then this will be the
	//	result. Otherwise, we pop this off before we evaluate the body of the
	//	loop.

	m_pCode->WriteShortOpCode(iBlock, opPushNil);

	//	This is the top of the loop so we remember its position

	int iBeginLoop = m_pCode->GetCodeBlockPos(iBlock);

	//	Evaluate the condition (result is on the stack)

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	if (!CompileExpression(iBlock))
		return false;

	//	Write out a jumpIfNil and remember the current position
	//	because we need to go back and set the proper jump distance

	int iJumpIfPos = m_pCode->GetCodeBlockPos(iBlock);
	m_pCode->WriteShortOpCode(iBlock, opJumpIfNil);

	//	Pop the previous result

	m_pCode->WriteShortOpCode(iBlock, opPop, 1);

	//	Now compile the body of the loop

	iToken = m_Parser.ParseToken(&dToken);
	if (iToken == CLispParser::tkCloseParen)
		m_pCode->WriteShortOpCode(iBlock, opPushNil);
	else
		{
		while (iToken != CLispParser::tkCloseParen)
			{
			if (!CompileExpression(iBlock))
				return false;

			iToken = m_Parser.ParseToken(&dToken);

			//	Pop the result off the stack

			if (iToken != CLispParser::tkCloseParen)
				m_pCode->WriteShortOpCode(iBlock, opPop, 1);
			}
		}

	//	Jump back to the top of the loop

	int iJumpOffset = (iBeginLoop - m_pCode->GetCodeBlockPos(iBlock)) / (int)sizeof(DWORD);
	m_pCode->WriteShortOpCode(iBlock, opJump, iJumpOffset);

	//	Now we know how far the if needs to jump (in DWORDs)

	iJumpOffset = (m_pCode->GetCodeBlockPos(iBlock) - iJumpIfPos) / sizeof(DWORD);
	m_pCode->RewriteShortOpCode(iBlock, iJumpIfPos, opJumpIfNil, iJumpOffset);

	//	Done

	return true;
	}

bool CLispCompiler::CompileStruct (int iBlock)

//	CompileStruct
//
//	{ key:value key:value ... }

	{
	//	Keep track of the number of key/value pairs

	int iCount = 0;

	//	Loop over all key/value pairs

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	while (iToken != CLispParser::tkCloseBrace)
		{
		//	We expect this to be the key (an identifier). If not, then error.

		if (iToken != CLispParser::tkIdentifier)
			{
			m_sError = m_Parser.ComposeError(strPattern(ERR_IDENTIFIER_EXPECTED, dToken.AsString()));
			return false;
			}

		//	Push the key as a string

		int iDataBlock = m_pCode->CreateDatumBlock(dToken);
		m_pCode->WriteShortOpCode(iBlock, opPushStr, iDataBlock);

		//	Parse a colon

		iToken = m_Parser.ParseToken(&dToken);
		if (iToken != CLispParser::tkColon)
			{
			m_sError = m_Parser.ComposeError(ERR_COLON_EXPECTED);
			return false;
			}

		//	Parse and push the expression (the value)

		iToken = m_Parser.ParseToken(&dToken);
		if (!CompileExpression(iBlock))
			return false;

		//	Next

		iCount++;
		iToken = m_Parser.ParseToken(&dToken);
		}

	//	Make the structure

	m_pCode->WriteShortOpCode(iBlock, opMakeStruct, iCount);

	//	Done

	return true;
	}

bool CLispCompiler::CompileTerm (CCharStream *pStream, CHexeCodeIntermediate &Code, int *retiBlock, CString *retsError)

//	Compile
//
//	Compiles a single term from the given stream into a new block in retCode. Returns TRUE if successful.
//	We leave the stream at the first character after the end of the term.

	{
	//	Initialize the parser

	m_Parser.Init(pStream);
	m_pCode = &Code;
	m_dCurrentEnv = CDatum();
	m_pCurrentEnv = NULL;
	int iBlock = -1;

	//	Parse a token

	CDatum dToken;
	CLispParser::ETokens iToken = m_Parser.ParseToken(&dToken);
	switch (iToken)
		{
		case CLispParser::tkOpenParen:
		case CLispParser::tkIdentifier:
		case CLispParser::tkStringDatum:
		case CLispParser::tkIntegerDatum:
		case CLispParser::tkIPIntegerDatum:
		case CLispParser::tkFloatDatum:
		case CLispParser::tkOpenBrace:
		case CLispParser::tkQuote:
			{
			iBlock = m_pCode->CreateCodeBlock();
			if (!CompileExpression(iBlock))
				{
				*retsError = m_sError;
				return false;
				}

			break;
			}

		case CLispParser::tkError:
			*retsError = dToken;
			return false;

		case CLispParser::tkEOF:
			*retsError = m_Parser.ComposeError(ERR_EOF);
			return false;

		case CLispParser::tkCloseParen:
			*retsError = m_Parser.ComposeError(ERR_UNEXPECTED_CLOSE_PAREN);
			return false;

		default:
			*retsError = strPattern(m_Parser.ComposeError(ERR_UNEXPECTED_TOKEN), (const CString &)dToken);
			return false;
		}

	ASSERT(iBlock != -1);

	//	Output a halt instruction at the end of the block

	m_pCode->WriteShortOpCode(iBlock, opHalt);

	//	We don't need to keep any state

	m_pCode = NULL;
	m_dCurrentEnv = CDatum();
	m_pCurrentEnv = NULL;

	//	Done

	if (retiBlock)
		*retiBlock = iBlock;

	return true;
	}

void CLispCompiler::EnterLocalEnvironment (void)

//	EnterLocalEnvironment
//
//	Enter a new environment frame

	{
	//	Create an environment

	CHexeLocalEnvironment *pLocalEnv = new CHexeLocalEnvironment;

	//	Link it to the current lexical environment

	pLocalEnv->SetParentEnv(m_dCurrentEnv);
	m_dCurrentEnv = CDatum(pLocalEnv);
	m_pCurrentEnv = pLocalEnv;
	}

void CLispCompiler::ExitLocalEnvironment (void)

//	ExitLocalEnvironment
//
//	Exit the current environment

	{
	//	Restore the environment

	m_dCurrentEnv = m_pCurrentEnv->GetParentEnv();
	m_pCurrentEnv = CHexeLocalEnvironment::Upconvert(m_dCurrentEnv);
	}

bool CLispCompiler::IsNilSymbol (CDatum dDatum)

//	IsNilSymbol
//
//	Is this nil?

	{
	//	Because we were too sloppy on Transcendence we do a
	//	case-insenstive check

	return strEquals(strToLower(dDatum), STR_NIL);
	}

bool CLispCompiler::IsTrueSymbol (CDatum dDatum)

//	IsTrueSymbol
//
//	Is this true?

	{
	//	Because we were too sloppy on Transcendence we do a
	//	case-insenstive check

	return strEquals(strToLower(dDatum), STR_TRUE);
	}
