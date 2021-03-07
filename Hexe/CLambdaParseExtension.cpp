//	CLambdaParseExtension.cpp
//
//	CLambdaParseExtension class
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_LAMBDA_PREFIX,					"(lambda")

DECLARE_CONST_STRING(ERR_NO_ASYNC_CALLS,				"Asynchronous invoke calls not supported in this context.")
DECLARE_CONST_STRING(ERR_UNKNOWN_RUN_RESULT,			"Unexpected run result: %d.")

bool CLambdaParseExtension::ParseAEONArray (CCharStream &Stream, CDatum *retDatum)

//	ParseAEONArray
//
//	Parses an array. If we see a lambda expression we return a CHexeFunction
//	instead.

	{
	//	Backup one because we want to read the '('

	Stream.UnreadChar();

	//	Look ahead to see if we have a lambda expression.

	char *pSrc = STR_LAMBDA_PREFIX.GetParsePointer();
	char *pSrcEnd = pSrc + STR_LAMBDA_PREFIX.GetLength();
	while (pSrc < pSrcEnd && Stream.ReadChar() == *pSrc++)
		;

	//	Unread everything (either way we need to start parsing again from the
	//	start of the array).

	Stream.UnreadChar((int)(pSrc - STR_LAMBDA_PREFIX.GetParsePointer()));

	//	Read the paren

	Stream.ReadChar();

	//	If this was not a lambda expression then let the normal parser handle
	//	it.

	if (pSrc < pSrcEnd)
		return false;

	//	Otherwise, we parse a Hexe expression

	CString sError;
	CDatum dExpression;
	if (!ParseHexeLispDef(&Stream, &dExpression, &sError))
		{
		CHexeError::Create(NULL_STR, sError, retDatum);
		return true;
		}

	//	Compile the expression so that we end up with a function object
	//	NOTE: These functions inherit the global environment of the
	//	process.

	CHexeProcess::ERun iRun = m_Process.Run(dExpression, retDatum);
	if (iRun == CHexeProcess::ERun::AsyncRequest)
		{
		CHexeError::Create(NULL_STR, ERR_NO_ASYNC_CALLS, retDatum);
		return true;
		}
	else if (iRun != CHexeProcess::ERun::OK && iRun != CHexeProcess::ERun::Error)
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_UNKNOWN_RUN_RESULT, iRun), retDatum);
		return true;
		}

	return true;
	}

bool CLambdaParseExtension::ParseHexeLispDef (CCharStream *pStream, CDatum *retdDatum, CString *retsError)

//	ParseHexeLisp
//
//	Parses a HexeLisp expression.

	{
	CHexeCodeIntermediate CodeBlocks;
	int iBlock;

	CLispCompiler Compiler;
	if (!Compiler.CompileTerm(pStream, CodeBlocks, &iBlock, retsError))
		return false;

	//	Create a function call entry point

	CHexeCode::Create(CodeBlocks, iBlock, retdDatum);

	//	Done

	return true;
	}
