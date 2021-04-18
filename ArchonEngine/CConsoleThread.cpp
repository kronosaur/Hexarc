//	CConsoleThread.cpp
//
//	CConsoleThread class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(CMD_QUIT,							"quit")

DECLARE_CONST_STRING(STR_PROMPT,						": ")

DECLARE_CONST_STRING(ERR_UNKNOWN_COMMAND,				"Unknown command.")

CString CConsoleThread::GetInputLine (const CString &sPrompt)
	{
	char szBuffer[1024];
	printf((LPSTR)sPrompt);
	gets_s(szBuffer, sizeof(szBuffer)-1);
	return CString(szBuffer);
	}

void CConsoleThread::PrintUTF8 (const CString sString)
	{
	CString16 sUnicode(strEscapePrintf(sString));
	wprintf((LPTSTR)sUnicode);
	}

bool CConsoleThread::ParseArray (char *pPos, char *pPosEnd, char **retpPos, CDatum *retdArg)

//	ParseArray
//
//	Parse an Array.

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

	if (retpPos)
		*retpPos = pPos;

	CString sArg(pStart, pPos - pStart);
	CStringBuffer Buffer(sArg);
	return CDatum::Deserialize(CDatum::formatAEONScript, Buffer, retdArg);
	}

bool CConsoleThread::ParseInputLine (const CString &sInput, CString *retsCmd, TArray<CDatum> *retArgs)

//	ParseInputLine
//
//	Parses the input line into commands and arguments. Returns FALSE if we failed
//	to parse.

	{
	//	Parse input

	char *pPos = sInput.GetParsePointer();
	char *pPosEnd = pPos + sInput.GetLength();

	//	Parse the command

	char *pStart = pPos;
	while (pPos < pPosEnd && *pPos != ' ')
		pPos++;

	*retsCmd = strToLower(CString(pStart, pPos - pStart));

	//	Parse arguments

	while (pPos < pPosEnd)
		{
		CDatum dArg;

		while (pPos < pPosEnd && strIsWhitespace(pPos))
			pPos++;

		if (*pPos == '(')
			{
			if (!ParseArray(pPos, pPosEnd, &pPos, &dArg))
				return false;

			retArgs->Insert(dArg);
			}
		else if (*pPos == '{')
			{
			if (!ParseStruct(pPos, pPosEnd, &pPos, &dArg))
				return false;

			retArgs->Insert(dArg);
			}
		else if (*pPos == '\"')
			{
			if (!ParseString(pPos, pPosEnd, &pPos, &dArg))
				return false;

			retArgs->Insert(dArg);
			}
		else
			{
			//	Parse a simple primitive

			pStart = pPos;
			while (pPos < pPosEnd && *pPos != ' ')
				pPos++;

			CString sArg(pStart, pPos - pStart);
			CStringBuffer Buffer(sArg);
			if (!CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &dArg))
				return false;

			retArgs->Insert(dArg);
			}
		}

	//	Done

	return true;
	}

bool CConsoleThread::ParseString (char *pPos, char *pPosEnd, char **retpPos, CDatum *retdArg)

//	ParseString
//
//	Parses a quoted string.

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
			else if (*pPos == '\"')
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

	if (retpPos)
		*retpPos = pPos;

	Output.Seek(0);
	return CDatum::Deserialize(CDatum::formatAEONScript, Output, retdArg);
	}

bool CConsoleThread::ParseStruct (char *pPos, char *pPosEnd, char **retpPos, CDatum *retdArg)

//	ParseStruct
//
//	Parses an AEON structure

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

	if (retpPos)
		*retpPos = pPos;

	CString sArg(pStart, pPos - pStart);
	CStringBuffer Buffer(sArg);
	return CDatum::Deserialize(CDatum::formatAEONScript, Buffer, retdArg);
	}

CString CConsoleThread::ProcessCommand (const CString &sCmd, const TArray<CDatum> &Args)

//	ProcessCommand
//
//	Process a given command.

	{
	//	We need exclusive access to the engine. This prevents us from calling
	//	the engine while garbage collection, or something.

	m_Busy.Increment();

	//	Send the command to all engines via message.

	CString sOutput = m_pProcess->ConsoleCommand(sCmd, Args);
	if (sOutput.IsEmpty())
		sOutput = ERR_UNKNOWN_COMMAND;

	//	Done

	m_Busy.Decrement();
	return sOutput;
	}

void CConsoleThread::Run (void)

//	Run
//
//	Runs console loop.

	{
	while (true)
		{
		CString sCmd;
		TArray<CDatum> Args;

		if (!ParseInputLine(GetInputLine(STR_PROMPT), &sCmd, &Args))
			{
			printf("Unable to parse command.\n");
			}

		else if (strEquals(sCmd, CMD_QUIT))
			{
			m_pProcess->SignalShutdown();
			break;
			}

		else if (!sCmd.IsEmpty())
			{
			CString sOutput = ProcessCommand(sCmd, Args);
			PrintUTF8(sOutput);
			printf("\n");
			}
		}
	}
