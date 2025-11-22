//	CHTML.cpp
//
//	CHTML class
//	Copyright (c) 2023 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

TArray<CString> CHTML::ParseCSSProperty (const CString& sValue)

//	ParseCSSProperty
//
//	Parses a CSS property into its components. For example, the following value:
//
//	bold italic 10px/1.2 "Times New Roman", Times, serif
//
//	Will get parsed into the following components:
//
//	bold
//	italic
//	10px/1.2
//	"Times New Roman", Times, serif

	{
	TArray<CString> Result;

	enum class State
		{
		start,
		inToken,
		done
		};

	enum class Quote
		{
		none,
		single,
		dbl,
		};

	const char *pPos = sValue.GetParsePointer();
	const char *pStart = pPos;
	Quote iQuote = Quote::none;

	State iState = State::start;
	while (iState != State::done)
		{
		switch (iState)
			{
			case State::start:
				{
				if (*pPos == '\0')
					iState = State::done;

				else if (*pPos == ' ' || *pPos == '\t')
					pPos++;

				else if (*pPos == '"')
					{
					pStart = pPos++;
					iQuote = Quote::dbl;
					iState = State::inToken;
					}
				else if (*pPos == '\'')
					{
					pStart = pPos++;
					iQuote = Quote::single;
					iState = State::inToken;
					}
				else
					{
					pStart = pPos++;
					iState = State::inToken;
					}

				break;
				}

			case State::inToken:
				{
				if (*pPos == '\0')
					{
					Result.Insert(CString(pStart, pPos - pStart));
					iState = State::done;
					}
				else if (iQuote == Quote::dbl)
					{
					if (*pPos == '"')
						iQuote = Quote::none;

					pPos++;
					}
				else if (iQuote == Quote::single)
					{
					if (*pPos == '\'')
						iQuote = Quote::none;

					pPos++;
					}
				else if (*pPos == '"')
					{
					iQuote = Quote::dbl;
					pPos++;
					}
				else if (*pPos == '\'')
					{
					iQuote = Quote::single;
					pPos++;
					}
				else if (*pPos == ',')
					{
					pPos++;
					while (*pPos == ' ' || *pPos == '\t')
						pPos++;
					}
				else if (*pPos == ' ' || *pPos == '\t')
					{
					//	Peek ahead and see if we have a comma. If so, then this 
					//	is a compound token.

					const char *pPos2 = pPos + 1;
					while (*pPos2 == ' ' || *pPos2 == '\t')
						pPos2++;

					if (*pPos2 == ',')
						{
						pPos2++;
						while (*pPos2 == ' ' || *pPos2 == '\t')
							pPos2++;

						pPos = pPos2;
						}
					else
						{
						Result.Insert(CString(pStart, pPos - pStart));
						pPos++;
						iState = State::start;
						}
					}
				else
					pPos++;

				break;
				}

			default:
				throw CException(errFail);
			}
		}

	return Result;
	}
