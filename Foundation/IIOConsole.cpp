//	IIOConsole.cpp
//
//	IIOConsole class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

void IIOConsole::Output (const CString &sText)

//	Output
//
//	Outputs a block of text, parsing for line endings.
//
//	NOTE: If sText is empty we do not output anything.

	{
	enum EStates
		{
		stateStart,
		stateLine,

		stateCR,
		stateLF,

		stateDone,
		};

	EStates iState = stateStart;
	char *pStart;

	char *pPos = sText.GetParsePointer();
	char *pPosEnd = pPos + sText.GetLength();

	while (iState != stateDone)
		{
		switch (iState)
			{
			case stateStart:
				if (pPos == pPosEnd || *pPos == '\0')
					iState = stateDone;
				else if (*pPos == '\r')
					{
					OutputLine(NULL_STR);
					iState = stateCR;
					}
				else if (*pPos == '\n')
					{
					OutputLine(NULL_STR);
					iState = stateLF;
					}
				else
					{
					pStart = pPos;
					iState = stateLine;
					}
				break;

			case stateLine:
				if (pPos == pPosEnd || *pPos == '\0')
					{
					OutputLine(CString(pStart, (pPos - pStart)));
					iState = stateDone;
					}
				else if (*pPos == '\r')
					{
					OutputLine(CString(pStart, (pPos - pStart)));
					iState = stateCR;
					}
				else if (*pPos == '\n')
					{
					OutputLine(CString(pStart, (pPos - pStart)));
					iState = stateLF;
					}
				break;

			case stateCR:
				if (pPos == pPosEnd || *pPos == '\0')
					iState = stateDone;
				else if (*pPos == '\n')
					iState = stateStart;
				else if (*pPos == '\r')
					{
					OutputLine(NULL_STR);
					}
				else
					{
					pStart = pPos;
					iState = stateLine;
					}
				break;

			case stateLF:
				if (pPos == pPosEnd || *pPos == '\0')
					iState = stateDone;
				else if (*pPos == '\r')
					iState = stateStart;
				else if (*pPos == '\n')
					{
					OutputLine(NULL_STR);
					}
				else
					{
					pStart = pPos;
					iState = stateLine;
					}
				break;
			}

		pPos++;
		}
	}
