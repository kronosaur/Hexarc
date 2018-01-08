//	XML.cpp
//
//	XML functions and classes
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(NOARGS,						"noArgs")
DECLARE_CONST_STRING(QUESTION_MARK_SWITCH,			"?")
DECLARE_CONST_STRING(HELP_SWITCH,					"help")
DECLARE_CONST_STRING(H_SWITCH,						"h")

DECLARE_CONST_STRING(TAG_BR,						"BR")
DECLARE_CONST_STRING(VALUE_TRUE,					"true")

enum ParseCmdState
	{
	stateStart,
	stateSwitch,
	stateParam,
	stateParamStart,
	stateParamQuoted,
	stateArg,
	stateArgQuoted,
	stateDone,
	};

void CreateXMLElementFromCommandLine (int argc, char *argv[], CXMLElement **retpElement)

//	CreateXMLElementFromCommandLine
//
//	Parses the command line into an element of the following form:
//
//	<PROGRAM-NAME
//			SWITCH1="true"
//			SWITCH2="*.*"
//			help="true"
//			noArgs="true"
//			>
//		arg1<br/>
//		arg2<br/>
//		...
//		argn<br/>
//	</PROGRAM-NAME>
//
//	PROGRAM-NAME is the name of the program (argv[0])
//	SWITCH1-N are arguments preceeded by a "/" or "-". The text after the
//		switch character is used after the argument name. If there is a colon
//		at the end of the switch name then we expect a value for the switch
//		(rather than true or false)
//	help is set to true if the command line contains /? or -? or /help or -help
//	noArgs is set to true if there are no switches or arguments in the commandline
//	arg1-argn are the parameters without leading "/" or "-"
//
//	The caller must free the resulting element

	{
	CXMLElement *pCmdLine = new CXMLElement(CString(argv[0]), NULL);
	if (pCmdLine == NULL)
		throw CException(errOutOfMemory);

	//	Parse the command line

	int iArg = 1;
	bool bNoArgs = true;
	while (iArg < argc)
		{
		char *pPos = argv[iArg];
		ParseCmdState iState = stateStart;
		char *pStart;
		CString sToken;

		while (iState != stateDone)
			{
			switch (iState)
				{
				case stateStart:
					{
					if (*pPos == '/' || *pPos == '-')
						{
						iState = stateSwitch;
						pPos++;
						pStart = pPos;
						}
					else if (*pPos == ' ' || *pPos == '\t')
						pPos++;
					else if (*pPos == '\0')
						iState = stateDone;
					else if (*pPos == '\"')
						{
						pPos++;
						iState = stateArgQuoted;
						pStart = pPos;
						}
					else
						{
						iState = stateArg;
						pStart = pPos;
						}
					break;
					}

				case stateSwitch:
					{
					if (*pPos == '/' || *pPos == ' ' || *pPos == '\0')
						{
						sToken = CString(pStart, (pPos - pStart));
						if (strEquals(sToken, QUESTION_MARK_SWITCH)
								|| strEquals(sToken, HELP_SWITCH))
							{
							pCmdLine->AddAttribute(HELP_SWITCH, VALUE_TRUE);
							bNoArgs = false;
							}
						else if (!sToken.IsEmpty())
							{
							pCmdLine->AddAttribute(sToken, VALUE_TRUE);
							bNoArgs = false;
							}
						iState = stateStart;
						}
					else if (*pPos == ':')
						{
						sToken = CString(pStart, (pPos - pStart));
						iState = stateParamStart;
						pPos++;
						}
					else
						pPos++;
					break;
					}

				case stateParam:
					{
					if (*pPos == '\0')
						{
						CString sParam(pStart, (pPos - pStart));
						pCmdLine->AddAttribute(sToken, sParam);
						bNoArgs = false;
						iState = stateStart;
						}
					else
						pPos++;
					break;
					}

				case stateParamQuoted:
					{
					if (*pPos == '\"' || *pPos == '\0')
						{
						CString sParam(pStart, (pPos - pStart));
						pCmdLine->AddAttribute(sToken, sParam);
						bNoArgs = false;
						iState = stateStart;

						if (*pPos == '\"')
							pPos++;
						}
					else
						pPos++;
					break;
					}

				case stateParamStart:
					{
					if (*pPos == '\"')
						{
						iState = stateParamQuoted;
						pPos++;
						pStart = pPos;
						}
					else if (*pPos == ' ')
						pPos++;
					else if (*pPos == '\0')
						{
						iState = stateStart;
						}
					else
						{
						pStart = pPos;
						iState = stateParam;
						}
					break;
					}

				case stateArg:
					{
					if (*pPos == ' ' || *pPos == '\0')
						{
						pCmdLine->AppendContent(CString(pStart, (pPos - pStart)));

						CXMLElement *pBR = new CXMLElement(TAG_BR, NULL);
						pCmdLine->AppendSubElement(pBR);

						bNoArgs = false;
						iState = stateStart;
						}
					else
						pPos++;
					break;
					}

				case stateArgQuoted:
					{
					if (*pPos == '\"' || *pPos == '\0')
						{
						pCmdLine->AppendContent(CString(pStart, (pPos - pStart)));

						CXMLElement *pBR = new CXMLElement(TAG_BR, NULL);
						pCmdLine->AppendSubElement(pBR);

						if (*pPos == '\"')
							pPos++;

						bNoArgs = false;
						iState = stateStart;
						}
					else
						pPos++;
					break;
					}

				default:
					ASSERT(false);
				}
			}

		//	Next
		iArg++;
		}

	//	If no args, add the flag

	if (bNoArgs)
		pCmdLine->AddAttribute(NOARGS, VALUE_TRUE);

	//	Done

	*retpElement = pCmdLine;
	}

void ParseAttributeIntegerList (const CString &sValue, TArray<int> *retList)

//	ParseAttributeIntegerList
//
//	Parses a string into an integer list

	{
	retList->DeleteAll();

	char *pPos = sValue.GetPointer();
	while (*pPos != '\0')
		{
		//	Skip non-numbers

		while (*pPos != '\0' && (*pPos < '0' || *pPos > '9') && *pPos != '-' && *pPos != '+')
			pPos++;

		//	Parse an integer

		if (*pPos != '\0')
			{
			bool bNull;
			int iInt;
			iInt = strParseInt(pPos, 0, &pPos, &bNull);

			//	If we have a valid integer then add it

			if (!bNull)
				retList->Insert(iInt);
			else
				break;
			}
		}
	}

CString strToXMLText (const CString &sText)

//	strToXMLText
//
//	Returns a string that has been stripped of all characters
//	that cannot appear in an XML attribute or body

	{
	CString sResult(sText.GetLength());
	int iExtra = 0;

	//	Optimistically assume that the text has no bad
	//	characters, while also keeping track of the amount of space
	//	that we would need for escape codes.

	char *pDest = sResult.GetParsePointer();
	char *pPos = sText.GetParsePointer();
	char *pPosEnd = pPos + sText.GetLength();
	while (pPos < pPosEnd)
		{
		switch (*pPos)
			{
			case '&':
				iExtra += 4;		//	&amp
				break;

			case '<':
				iExtra += 3;		//	&lt
				break;

			case '>':
				iExtra += 3;		//	&gt
				break;

			case '\"':
				iExtra += 5;		//	&quot
				break;

			case '\'':
				iExtra += 5;		//	apos
				break;

			default:
				if (*pPos < ' ' || *pPos > '~')
					iExtra += 5;
				break;
			}

		*pDest++ = *pPos++;
		}

	//	Done?

	if (iExtra == 0)
		{
		*pDest = '\0';
		return sResult;
		}

	//	Need to escape

	sResult = CString(sText.GetLength() + iExtra);
	pDest = sResult.GetParsePointer();
	pPos = sText.GetParsePointer();
	pPosEnd = pPos + sText.GetLength();
	while (pPos < pPosEnd)
		{
		switch (*pPos)
			{
			case '&':
				*pDest++ = '&';
				*pDest++ = 'a';
				*pDest++ = 'm';
				*pDest++ = 'p';
				*pDest++ = ';';
				break;

			case '<':
				*pDest++ = '&';
				*pDest++ = 'l';
				*pDest++ = 't';
				*pDest++ = ';';
				break;

			case '>':
				*pDest++ = '&';
				*pDest++ = 'g';
				*pDest++ = 't';
				*pDest++ = ';';
				break;

			case '\"':
				*pDest++ = '&';
				*pDest++ = 'q';
				*pDest++ = 'u';
				*pDest++ = 'o';
				*pDest++ = 't';
				*pDest++ = ';';
				break;

			case '\'':
				*pDest++ = '&';
				*pDest++ = 'a';
				*pDest++ = 'p';
				*pDest++ = 'o';
				*pDest++ = 's';
				*pDest++ = ';';
				break;

			default:
				if (*pPos >= ' ' && *pPos <= '~')
					*pDest++ = *pPos;
				else
					{
					CString sEscape = strPattern("%02x", (DWORD)(BYTE)*pPos);
					char *pEscape = sEscape.GetParsePointer();
					*pDest++ = '&';
					*pDest++ = '#';
					*pDest++ = 'x';
					*pDest++ = pEscape[0];
					*pDest++ = pEscape[1];
					*pDest++ = ';';
					}
				break;
			}

		pPos++;
		}

	//	Done!

	*pDest = '\0';
	return sResult;
	}
