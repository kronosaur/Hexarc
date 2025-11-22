//	CAEONExpression.cpp
//
//	CAEONExpression classes
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatum CAEONExpression::Parse (CStringView sExpr)
	{
	//	LATER: Build a full parser.

	CBuffer Stream(sExpr);
	CGridLangParser Parser(NULL_STR, Stream);

	Parser.NextToken();
	if (Parser.GetToken() == EGridLangToken::Identifier || Parser.GetToken() == EGridLangToken::String)
		{
		CString sName = Parser.GetTokenValue();
		return CAEONExpression::CreateColumnRef(sName);
		}
	else if (Parser.GetToken() == EGridLangToken::Dot)
		{
		Parser.NextToken();
		if (Parser.GetToken() == EGridLangToken::Identifier || Parser.GetToken() == EGridLangToken::String)
			{
			CString sName = Parser.GetTokenValue();
			return CAEONExpression::CreateColumnRef(sName);
			}
		else
			return CDatum();
		}
	else
		return CDatum();

	return CDatum();
	}
