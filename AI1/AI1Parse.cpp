//	AI1Parse.cpp
//
//	Parse commands
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

static CString ParseBrackets (char *pPos, char **retpPos);
static CString ParseList (char *pPos, char **retpPos);
static CString ParseQuotedArg (char *pPos, char **retpPos);
static CString ParseStruct (char *pPos, char **retpPos);
static CString ParseUnquotedArg (char *pPos, bool bQuote, char **retpPos);

CString ParseAI1Command (const CString &sInput)
	{
	CStringBuffer Output;

	Output.Write("AI/1.00 ", 8);
	bool bQuoteArg = false;

	char *pPos = sInput.GetParsePointer();
	while (*pPos != '\0')
		{
		while (*pPos == ' ')
			pPos++;

		if (*pPos == '\0')
			break;
		else if (*pPos == '(')
			{
			CString sArg = ParseList(pPos, &pPos);
			Output.Write(sArg);
			Output.Write(" ", 1);
			}
		else if (*pPos == '{')
			{
			CString sArg = ParseStruct(pPos, &pPos);
			Output.Write(sArg);
			Output.Write(" ", 1);
			}
		else if (*pPos == '[')
			{
			CString sArg = ParseBrackets(pPos, &pPos);
			Output.Write(sArg);
			Output.Write(" ", 1);
			}
		else if (*pPos == '"')
			{
			CString sArg = ParseQuotedArg(pPos, &pPos);
			Output.Write(sArg);
			Output.Write(" ", 1);
			}
		else
			{
			CString sArg = ParseUnquotedArg(pPos, bQuoteArg, &pPos);
			Output.Write(sArg);
			Output.Write(" ", 1);

			bQuoteArg = true;
			}
		}

	Output.Write("|", 1);

	return CString::CreateFromHandoff(Output);
	}

CString ParseBrackets (char *pPos, char **retpPos)
	{
	ASSERT(*pPos == '[');

	char *pStart = pPos;
	pPos++;

	int iParenDepth = 1;
	int iQuoteDepth = 0;
	while (*pPos != '\0' && iParenDepth > 0)
		{
		if (iQuoteDepth)
			{
			if (*pPos == '\\')
				{
				pPos++;
				if (*pPos == '\0')
					break;
				}
			else if (*pPos == '"')
				iQuoteDepth--;
			}
		else if (*pPos == '"')
			iQuoteDepth++;
		else if (*pPos == ']')
			iParenDepth--;
		else if (*pPos == '[')
			iParenDepth++;

		pPos++;
		}

	*retpPos = pPos;
	return CString(pStart, pPos - pStart);
	}

CString ParseList (char *pPos, char **retpPos)
	{
	ASSERT(*pPos == '(');

	char *pStart = pPos;
	pPos++;

	int iParenDepth = 1;
	int iQuoteDepth = 0;
	while (*pPos != '\0' && iParenDepth > 0)
		{
		if (iQuoteDepth)
			{
			if (*pPos == '\\')
				{
				pPos++;
				if (*pPos == '\0')
					break;
				}
			else if (*pPos == '"')
				iQuoteDepth--;
			}
		else if (*pPos == '"')
			iQuoteDepth++;
		else if (*pPos == ')')
			iParenDepth--;
		else if (*pPos == '(')
			iParenDepth++;

		pPos++;
		}

	*retpPos = pPos;
	return CString(pStart, pPos - pStart);
	}

CString ParseQuotedArg (char *pPos, char **retpPos)
	{
	ASSERT(*pPos == '"');

	CStringBuffer Output;
	Output.Write(pPos, 1);
	pPos++;

	while (*pPos != '\0' && *pPos != '"')
		{
		if (*pPos == '\\')
			{
			//	Check the next character; if we're escaping a quote, then
			//	pass it through. Otherwise, we escape a backslash.

			pPos++;
			if (*pPos == '\0')
				break;
			else if (*pPos == '"')
				Output.Write("\\\"", 2);
			else
				{
				Output.Write("\\\\", 2);
				continue;
				}
			}
		else
			Output.Write(pPos, 1);

		pPos++;
		}

	Output.Write("\"", 1);
	if (*pPos != '\0')
		pPos++;

	*retpPos = pPos;

	return CString::CreateFromHandoff(Output);
	}

CString ParseStruct (char *pPos, char **retpPos)
	{
	ASSERT(*pPos == '{');

	char *pStart = pPos;
	pPos++;

	int iParenDepth = 1;
	int iQuoteDepth = 0;
	while (*pPos != '\0' && iParenDepth > 0)
		{
		if (iQuoteDepth)
			{
			if (*pPos == '\\')
				{
				pPos++;
				if (*pPos == '\0')
					break;
				}
			else if (*pPos == '"')
				iQuoteDepth--;
			}
		else if (*pPos == '"')
			iQuoteDepth++;
		else if (*pPos == '}')
			iParenDepth--;
		else if (*pPos == '{')
			iParenDepth++;

		pPos++;
		}

	*retpPos = pPos;
	return CString(pStart, pPos - pStart);
	}

CString ParseUnquotedArg (char *pPos, bool bQuote, char **retpPos)
	{
	CStringBuffer Output;

	//	Do not quote digits

	if (strIsDigit(pPos))
		bQuote = false;

	if (bQuote)
		Output.Write("\"", 1);

	char *pStart = pPos;
	while (*pPos != '\0' && *pPos != ' ')
		{
		if (*pPos == '\\')
			{
			Output.Write("\\", 1);
			Output.Write(pPos, 1);
			pPos++;
			}
		else
			{
			Output.Write(pPos, 1);
			pPos++;
			}
		}

	if (bQuote)
		Output.Write("\"", 1);

	*retpPos = pPos;

	CString sArg = CString::CreateFromHandoff(Output);
	CString sArgLower = strToLower(sArg);

	if (strEquals(sArgLower, CString("\"nil\"")))
		return CString("nil");
	else if (strEquals(sArgLower, CString("\"true\"")))
		return CString("true");
	else
		return sArg;
	}

