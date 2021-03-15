//	CGridLangAST.cpp
//
//	CGridLangAST Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(KEYWORD_BEGIN,						"begin");
DECLARE_CONST_STRING(KEYWORD_CONST,						"const");
DECLARE_CONST_STRING(KEYWORD_ELSE,						"else");
DECLARE_CONST_STRING(KEYWORD_END,						"end");
DECLARE_CONST_STRING(KEYWORD_FALSE,						"false");
DECLARE_CONST_STRING(KEYWORD_FUNCTION,					"function");
DECLARE_CONST_STRING(KEYWORD_GLOBAL,					"global");
DECLARE_CONST_STRING(KEYWORD_IF,						"if");
DECLARE_CONST_STRING(KEYWORD_METHOD,					"method");
DECLARE_CONST_STRING(KEYWORD_NULL,						"null");
DECLARE_CONST_STRING(KEYWORD_ORDINAL,					"ordinal");
DECLARE_CONST_STRING(KEYWORD_PROPERTY,					"property");
DECLARE_CONST_STRING(KEYWORD_RETURN,					"return");
DECLARE_CONST_STRING(KEYWORD_THEN,						"then");
DECLARE_CONST_STRING(KEYWORD_TRUE,						"true");
DECLARE_CONST_STRING(KEYWORD_TYPE,						"type");
DECLARE_CONST_STRING(KEYWORD_VAR,						"var");

DECLARE_CONST_STRING(ERR_UNEXPECTED_TOKEN,				"Unexpected token: '%s'");
DECLARE_CONST_STRING(ERR_INVALID_FLOAT,					"Invalid float: '%s'");
DECLARE_CONST_STRING(ERR_INVALID_INTEGER,				"Invalid integer: '%s'");

EASTType CGridLangAST::GetOperator (EGridLangToken iToken)

//	GetOperator
//
//	Returns an operator for a token.

	{
	switch (iToken)
		{
		case EGridLangToken::Amp:
			return EASTType::OpArithmeticAnd;

		case EGridLangToken::Bar:
			return EASTType::OpArithmeticOr;

		case EGridLangToken::Slash:
			return EASTType::OpDivide;

		case EGridLangToken::EqualEquals:
			return EASTType::OpEquals;

		case EGridLangToken::Equals:
			return EASTType::OpAssignment;

		case EGridLangToken::OpenParen:
			return EASTType::OpFunctionCall;

		case EGridLangToken::GreaterThan:
			return EASTType::OpGreaterThan;

		case EGridLangToken::GreaterThanEquals:
			return EASTType::OpGreaterThanEquals;

		case EGridLangToken::LessThan:
			return EASTType::OpLessThan;

		case EGridLangToken::LessThanEquals:
			return EASTType::OpLessThanEquals;

		case EGridLangToken::And:
			return EASTType::OpLogicalAnd;

		case EGridLangToken::Or:
			return EASTType::OpLogicalOr;

		case EGridLangToken::Dot:
			return EASTType::OpMemberAccess;

		case EGridLangToken::Minus:
			return EASTType::OpMinus;

		case EGridLangToken::Bang:
			return EASTType::OpNot;

		case EGridLangToken::NotEquals:
			return EASTType::OpNotEquals;

		case EGridLangToken::Plus:
			return EASTType::OpPlus;

		case EGridLangToken::Star:
			return EASTType::OpTimes;

		default:
			return EASTType::Unknown;
		}
	}

int CGridLangAST::GetOperatorPrecedence (EASTType iOperator)

//	GetOperatorPrecedence
//
//	Returns operator precedence. Lower values are tighter precedence.

	{
	switch (iOperator)
		{
		case EASTType::OpFunctionCall:
		case EASTType::OpMemberAccess:
			return 1;

		case EASTType::OpNot:
			return 2;

		case EASTType::OpDivide:
		case EASTType::OpTimes:
			return 3;

		case EASTType::OpMinus:
		case EASTType::OpPlus:
			return 4;

		case EASTType::OpGreaterThan:
		case EASTType::OpGreaterThanEquals:
		case EASTType::OpLessThan:
		case EASTType::OpLessThanEquals:
			return 6;

		case EASTType::OpEquals:
		case EASTType::OpNotEquals:
			return 7;

		case EASTType::OpArithmeticAnd:
			return 8;

		case EASTType::OpArithmeticOr:
			return 10;

		case EASTType::OpLogicalAnd:
			return 11;

		case EASTType::OpLogicalOr:
			return 12;

		case EASTType::OpAssignment:
			return 14;

		default:
			return 100;
		}
	}

bool CGridLangAST::IsGreaterPrecedence (EASTType iOperator, EASTType iTest)

//	IsGreaterPrecedence
//
//	Returns TRUE if iOperator binds closer than iTest.

	{
	return (GetOperatorPrecedence(iOperator) < GetOperatorPrecedence(iTest));
	}

bool CGridLangAST::IsOperator (EGridLangToken iToken)

//	IsOperator
//
//	Returns TRUE if this is an operator

	{
	return GetOperator(iToken) != EASTType::Unknown;
	}

bool CGridLangAST::Load (IMemoryBlock &Stream, CString *retsError)

//	Load
//
//	Load the AST from source stream.

	{
	CGridLangParser Parser(Stream);
	Parser.NextToken();

	TArray<TSharedPtr<IASTNode>> Program;

	if (!ParseSequence(Parser, NULL, m_pRoot, retsError))
		return false;

	return true;
	}

bool CGridLangAST::ParseArgDef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseArgDef
//
//	The current token is the first token of the argument definition. We end at
//	the first token AFTER the last token of the definition.

	{
	if (Parser.GetToken() != EGridLangToken::Identifier)
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	CString sArg = Parser.GetTokenValue();
	Parser.NextToken();

	//	Expect optional type reference

	TSharedPtr<IASTNode> pType;
	if (Parser.GetToken() == EGridLangToken::Colon)
		{
		Parser.NextToken();
		if (!ParseTypeRef(Parser, pParent, pType, retsError))
			return false;
		}
	else
		{
		pType = CASTTypeRef::Create(pParent, KEYWORD_TYPE);
		}

	//	Done

	retpNode = CASTArgDef::Create(pParent, sArg, pType);

	return true;
	}

bool CGridLangAST::ParseClassDef (CGridLangParser &Parser, IASTNode *pParent, const CString &sBaseType, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseSubClassDef
//
//	The current token is new class type. We end with the token AFTER the last
//	token.

	{
	if (Parser.GetToken() != EGridLangToken::Identifier)
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	CString sID = Parser.GetTokenValue();
	Parser.NextToken();

	//	Expect a BEGIN

	if (Parser.GetToken() != EGridLangToken::Identifier || !strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_BEGIN))
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	Parser.NextToken();

	//	Parse the body

	TSharedPtr<IASTNode> pBody;
	if (!ParseSequence(Parser, pParent, pBody, retsError))
		return false;

	//	Create the definition

	retpNode = CASTClassDef::Create(pParent, sID, sBaseType, pBody);

	//	Skip the END keyword

	Parser.NextToken();
		
	return true;
	}

bool CGridLangAST::ParseExpression (CGridLangParser &Parser, IASTNode *pParent, EASTType iLeftOperator, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseExpression
//
//	The current token is the first token of the expression. We end with the 
//	first token AFTER the expression.

	{
	TSharedPtr<IASTNode> pLeft;

	if (!ParseTerm(Parser, pParent, pLeft, retsError))
		return false;

	while (IsOperator(Parser.GetToken()) 
			&& IsGreaterPrecedence(GetOperator(Parser.GetToken()), iLeftOperator))
		{
		EASTType iOp = GetOperator(Parser.GetToken());

		if (iOp == EASTType::OpFunctionCall)
			{
			TSharedPtr<IASTNode> pFunc;

			if (!ParseFunctionCall(Parser, pParent, pLeft, pFunc, retsError))
				return false;

			pLeft = pFunc;
			}
		else
			{
			Parser.NextToken();

			TSharedPtr<IASTNode> pRight;
			if (!ParseExpression(Parser, pParent, iOp, pRight, retsError))
				return false;

			pLeft = CASTBinaryOp::Create(pParent, iOp, pLeft, pRight);
			}
		}

	retpNode = pLeft;
	return true;
	}

bool CGridLangAST::ParseFunctionCall (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> pFunction, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseFunctionCall
//
//	The current token is an open paren starting the args. We end with the token
//	AFTER the close paren.

	{
	Parser.NextToken();

	//	Parse all arguments

	TArray<TSharedPtr<IASTNode>> Args;
	while (Parser.GetToken() != EGridLangToken::CloseParen)
		{
		TSharedPtr<IASTNode> pArg;
		if (!ParseExpression(Parser, pParent, pArg, retsError))
			return false;

		Args.Insert(pArg);
		if (Parser.GetToken() == EGridLangToken::Comma)
			Parser.NextToken();
		}

	Parser.NextToken();

	retpNode = CASTFunctionCall::Create(pParent, pFunction, std::move(Args));

	return true;
	}

bool CGridLangAST::ParseFunctionDef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseFunctionDef
//
//	The current token is the function name. We end with first token AFTER the
//	definition.

	{
	if (Parser.GetToken() != EGridLangToken::Identifier)
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	CString sFunctionName = Parser.GetTokenValue();
	Parser.NextToken();

	if (Parser.GetToken() != EGridLangToken::OpenParen)
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	Parser.NextToken();

	//	Parse the argument definitions

	TArray<TSharedPtr<IASTNode>> Args;
	while (Parser.GetToken() != EGridLangToken::CloseParen)
		{
		TSharedPtr<IASTNode> pArg;
		if (!ParseArgDef(Parser, pParent, pArg, retsError))
			return false;

		Args.Insert(pArg);
		if (Parser.GetToken() == EGridLangToken::Comma)
			Parser.NextToken();
		}

	//	Parse optional return type

	TSharedPtr<IASTNode> pReturnType;
	if (Parser.NextToken() == EGridLangToken::Colon)
		{
		Parser.NextToken();
		if (!ParseTypeRef(Parser, pParent, pReturnType, retsError))
			return false;
		}
	else
		{
		pReturnType = CASTTypeRef::Create(pParent, KEYWORD_TYPE);
		}

	//	Expect BEGIN keyword

	if (Parser.GetToken() != EGridLangToken::Identifier || !strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_BEGIN))
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	Parser.NextToken();

	//	Parse the body

	TSharedPtr<IASTNode> pBody;
	if (!ParseSequence(Parser, pParent, pBody, retsError))
		return false;

	//	Create the definition

	retpNode = CASTFunctionDef::Create(pParent, sFunctionName, pReturnType, Args, pBody);

	//	Skip the END keyword

	Parser.NextToken();
		
	return true;
	}

bool CGridLangAST::ParseIf (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseIf
//
//	Parses an IF statement. We start with the IF token and end with the token
//	AFTER the end.

	{
	Parser.NextToken();

	TSharedPtr<IASTNode> pCondition;
	if (!ParseExpression(Parser, pParent, pCondition, retsError))
		return false;

	//	Expect THEN or BEGIN

	if (Parser.GetToken() != EGridLangToken::Identifier
			|| (!strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_THEN) && !strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_BEGIN)))
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	Parser.NextToken();

	//	Parse then

	TSharedPtr<IASTNode> pThen;
	if (!ParseSequence(Parser, pParent, pThen, retsError))
		return false;

	//	Parse optional else

	TSharedPtr<IASTNode> pElse;
	if (Parser.GetToken() == EGridLangToken::Identifier 
			&& strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_ELSE))
		{
		Parser.NextToken();

		if (!ParseSequence(Parser, pParent, pElse, retsError))
			return false;
		}

	Parser.NextToken();

	//	Done

	retpNode = CASTIf::Create(pParent, pCondition, pThen, pElse);

	return true;
	}

bool CGridLangAST::ParseOrdinalDef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseOrdinalDef
//
//	The current token is the name. We end with first token AFTER the definition.

	{
	if (Parser.GetToken() != EGridLangToken::Identifier)
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	CString sName = Parser.GetTokenValue();
	Parser.NextToken();

	//	Expect BEGIN keyword

	if (Parser.GetToken() != EGridLangToken::Identifier || !strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_BEGIN))
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	Parser.NextToken();

	//	Parse the list of ordinals

	TArray<CString> Ordinals;
	while (Parser.GetToken() == EGridLangToken::Identifier
			&& !strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_END))
		{
		Ordinals.Insert(Parser.GetTokenValue());
		Parser.NextToken();

		if (Parser.GetToken() == EGridLangToken::Comma)
			Parser.NextToken();
		}

	//	Expect END

	if (Parser.GetToken() != EGridLangToken::Identifier || !strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_END))
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	Parser.NextToken();

	//	Done

	retpNode = CASTOrdinalDef::Create(pParent, sName, Ordinals);
		
	return true;
	}

bool CGridLangAST::ParseSequence (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseSequence
//
//	Parses a sequence of statements. We start at the first token of the 
//	sequence. We stop when we hit the end of stream or an END keyword. We leave
//	the token at the end token.

	{
	TArray<TSharedPtr<IASTNode>> Program;
	int iVarOrdinal = 0;

	while (true)
		{
		EGridLangToken iToken = Parser.GetToken();
		if (iToken == EGridLangToken::Null)
			{
			//	Done
			break;
			}
		else if (iToken == EGridLangToken::Identifier)
			{
			if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_CONST))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseVarDef(Parser, pParent, EASTType::ConstDef, iVarOrdinal++, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_END) || strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_ELSE))
				{
				//	Done
				break;
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_FUNCTION))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseFunctionDef(Parser, pParent, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_GLOBAL))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseVarDef(Parser, pParent, EASTType::GlobalDef, iVarOrdinal++, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_IF))
				{
				TSharedPtr<IASTNode> pNode;
				if (!ParseIf(Parser, pParent, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_METHOD))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseFunctionDef(Parser, pParent, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_ORDINAL))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseOrdinalDef(Parser, pParent, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_PROPERTY))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseVarDef(Parser, pParent, EASTType::PropertyDef, iVarOrdinal++, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_RETURN))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseExpression(Parser, pParent, pNode, retsError))
					return false;

				Program.Insert(CASTUnaryOp::Create(pParent, EASTType::OpReturn, pNode));
				}
			else if (strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_VAR))
				{
				Parser.NextToken();

				TSharedPtr<IASTNode> pNode;
				if (!ParseVarDef(Parser, pParent, EASTType::VarDef, iVarOrdinal++, pNode, retsError))
					return false;

				Program.Insert(pNode);
				}
			else
				{
				//	If the next token is an identifier, then this is a class 
				//	definition.

				EGridLangToken iNextToken = Parser.PeekToken();
				if (iNextToken == EGridLangToken::Identifier)
					{
					CString sIdentifier = Parser.GetTokenValue();
					Parser.NextToken();

					TSharedPtr<IASTNode> pNode;
					if (!ParseClassDef(Parser, pParent, sIdentifier, pNode, retsError))
						return false;

					Program.Insert(pNode);
					}

				//	Otherwise, we assume it is an expression of some sort.

				else
					{
					TSharedPtr<IASTNode> pNode;
					if (!ParseExpression(Parser, pParent, pNode, retsError))
						return false;

					Program.Insert(pNode);
					}
				}
			}
		else
			{
			if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
			return false;
			}
		}

	retpNode = CASTSequence::Create(pParent, Program, retsError);
	if (!retpNode)
		return false;

	return true;
	}

bool CGridLangAST::ParseTerm (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseTerm
//
//	The current token is the first token of the term. We end with the first 
//	token AFTER the term.

	{
	if (Parser.GetToken() == EGridLangToken::OpenParen)
		{
		Parser.NextToken();

		if (!ParseExpression(Parser, pParent, retpNode, retsError))
			return false;

		if (Parser.GetToken() != EGridLangToken::CloseParen)
			{
			if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
			return false;
			}

		Parser.NextToken();
		}
	else if (Parser.GetToken() == EGridLangToken::Plus)
		{
		Parser.NextToken();

		if (!ParseTerm(Parser, pParent, retpNode, retsError))
			return false;
		}
	else if (Parser.GetToken() == EGridLangToken::Minus)
		{
		Parser.NextToken();

		TSharedPtr<IASTNode> pNode;
		if (!ParseTerm(Parser, pParent, pNode, retsError))
			return false;

		retpNode = CASTUnaryOp::Create(pParent, EASTType::OpMinus, pNode);
		}
	else if (Parser.GetToken() == EGridLangToken::Identifier)
		{
		CString sIdentifier = Parser.GetTokenValue();
		Parser.NextToken();

		if (strEqualsNoCase(sIdentifier, KEYWORD_FALSE))
			{
			retpNode = CASTLiteralIdentifier::Create(pParent, EASTType::LiteralNull);
			}
		else if (strEqualsNoCase(sIdentifier, KEYWORD_NULL))
			{
			retpNode = CASTLiteralIdentifier::Create(pParent, EASTType::LiteralNull);
			}
		else if (strEqualsNoCase(sIdentifier, KEYWORD_TRUE))
			{
			retpNode = CASTLiteralIdentifier::Create(pParent, EASTType::LiteralTrue);
			}
		else
			{
			retpNode = CASTVarRef::Create(pParent, sIdentifier);
			}
		}
	else if (Parser.GetToken() == EGridLangToken::Integer || Parser.GetToken() == EGridLangToken::HexInteger)
		{
		bool bFailed;
		int iValue = strToInt(Parser.GetTokenValue(), 0, &bFailed);
		if (bFailed)
			{
			if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_INVALID_INTEGER, Parser.GetTokenValue()));
			return false;
			}

		retpNode = CASTLiteralInteger::Create(pParent, iValue);

		Parser.NextToken();
		}
	else if (Parser.GetToken() == EGridLangToken::Real)
		{
		double rValue = strToDouble(Parser.GetTokenValue());

		retpNode = CASTLiteralFloat::Create(pParent, rValue);

		Parser.NextToken();
		}
	else if (Parser.GetToken() == EGridLangToken::String)
		{
		retpNode = CASTLiteralString::Create(pParent, Parser.GetTokenValue());

		Parser.NextToken();
		}
	else
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	return true;
	}

bool CGridLangAST::ParseTypeRef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseTypeRef
//
//	The current token is the first token of the type reference. We end with the
//	First token AFTER the last token.

	{
	if (Parser.GetToken() != EGridLangToken::Identifier)
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	CString sType = Parser.GetTokenValue();
	Parser.NextToken();

	retpNode = CASTTypeRef::Create(pParent, sType);

	return true;
	}

bool CGridLangAST::ParseVarDef (CGridLangParser &Parser, IASTNode *pParent, EASTType iVarType, int iOrdinal, TSharedPtr<IASTNode> &retpNode, CString *retsError)

//	ParseVarDef
//
//	The current token is the first token after the variable type. We end with 
//	the first token AFTER the last token of the definition.

	{
	if (Parser.GetToken() != EGridLangToken::Identifier)
		{
		if (retsError) *retsError = Parser.ComposeError(strPattern(ERR_UNEXPECTED_TOKEN, Parser.GetTokenValue()));
		return false;
		}

	CString sName = Parser.GetTokenValue();
	Parser.NextToken();

	//	Parse optional type

	TSharedPtr<IASTNode> pType;
	if (Parser.GetToken() == EGridLangToken::Colon)
		{
		Parser.NextToken();
		if (!ParseTypeRef(Parser, pParent, pType, retsError))
			return false;
		}
	else
		{
		pType = CASTTypeRef::Create(pParent, KEYWORD_TYPE);
		}

	//	Parse either a value or body

	TSharedPtr<IASTNode> pBody;
	if (Parser.GetToken() == EGridLangToken::Equals)
		{
		Parser.NextToken();

		if (!ParseExpression(Parser, pParent, pBody, retsError))
			return false;
		}
	else if (Parser.GetToken() == EGridLangToken::Identifier && strEqualsNoCase(Parser.GetTokenValue(), KEYWORD_BEGIN))
		{
		Parser.NextToken();

		if (!ParseSequence(Parser, pParent, pBody, retsError))
			return false;

		Parser.NextToken();
		}
	else
		{
		pBody = CASTLiteralIdentifier::Create(pParent, EASTType::LiteralNull);
		}

	//	Done

	retpNode = CASTVarDef::Create(pParent, iVarType, iOrdinal, sName, pType, pBody);

	return true;
	}
