//	CHexeConsole.cpp
//
//	CHexeConsole class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CONSOLE_ID,					"consoleID");
DECLARE_CONST_STRING(FIELD_DONE,						"done");
DECLARE_CONST_STRING(FIELD_RESULT,						"result");
DECLARE_CONST_STRING(FIELD_SEQ,							"seq");

CHexeConsole::CHexeConsole (const CString &sID) :
		m_sID(sID),
		m_dwLastAccess(::sysGetTickCount64())

//	CHexeConsole constructor

	{
	}

void CHexeConsole::AddReaderAccess (const CString &sUsername)

//	AddReaderAccess
//
//	Allows the given user to access the console.
//	NOTE: We assume a canonical username (lowercase)

	{
	CSmartLock Lock(m_cs);

	if (sUsername.IsEmpty())
		return;

	m_Readers.Insert(sUsername);
	}

void CHexeConsole::Close (void)

//	Close
//
//	Close the console so that there is no more output.

	{
	CSmartLock Lock(m_cs);
	m_bClosed = true;
	}

int CHexeConsole::FindFirstLine (DWORD Seq) const

//	FindFirstLine
//
//	Returns the first line that is at or immediately after the given sequence
//	number. If none is found, we return 0.
//
//	NOTE: This must always be called inside a lock.

	{
	for (int i = 0; i < m_Lines.GetCount(); i++)
		{
		if (m_Lines[i].Seq >= Seq)
			return i;
		}

	return -1;
	}

CDatum CHexeConsole::GetConsoleData (DWORD Seq) const

//	GetConsoleData
//
//	Returns console data up from given sequence number.

	{
	CSmartLock Lock(m_cs);

	//	Generate an array of lines

	CDatum dLines;
	int iFirst;
	if ((iFirst = FindFirstLine(Seq)) != -1)
		{
		dLines = CDatum(CDatum::typeArray);

		int iCount = m_Lines.GetCount() - iFirst;
		dLines.GrowToFit(iCount);

		for (int i = iFirst; i < m_Lines.GetCount(); i++)
			dLines.Append(m_Lines[i].dData);
		}

	//	Now compose the result

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_CONSOLE_ID, GetID());
	if (m_bClosed)
		dResult.SetElement(FIELD_DONE, CDatum(CDatum::constTrue));

	dResult.SetElement(FIELD_RESULT, dLines);
	dResult.SetElement(FIELD_SEQ, m_Seq);

	return dResult;
	}

bool CHexeConsole::HasReaderAccess (const CString &sUsername) const

//	HasReaderAccess
//
//	Returns TRUE if the given username can read the console.
//	NOTE: We assume a canonical username (lowercase)

	{
	CSmartLock Lock(m_cs);
	return m_Readers.Find(sUsername);
	}

#if 0
void CHexeConsole::OutputText (const CString &sText)

//	OutputText
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
					OutputData(CDatum());
					iState = stateCR;
					}
				else if (*pPos == '\n')
					{
					OutputData(CDatum());
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
					OutputData(CString(pStart, (pPos - pStart)));
					iState = stateDone;
					}
				else if (*pPos == '\r')
					{
					OutputData(CString(pStart, (pPos - pStart)));
					iState = stateCR;
					}
				else if (*pPos == '\n')
					{
					OutputData(CString(pStart, (pPos - pStart)));
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
					OutputData(CDatum());
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
					OutputData(CDatum());
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
#endif

void CHexeConsole::Mark (void)

//	Mark
//
//	Mark all data in use

	{
	for (int i = 0; i < m_Lines.GetCount(); i++)
		m_Lines[i].dData.Mark();
	}

void CHexeConsole::OutputData (CDatum dData)

//	OutputLine
//
//	Outputs a line to the console.

	{
	CSmartLock Lock(m_cs);

	//	Skip empty data

	if (dData.IsNil())
		return;

	//	If the data is an error we need to convert to a string because our 
	//	recipients don't know how to deserialize an error.

	if (dData.IsError())
		dData = strPattern("ERROR: %s", (LPSTR)dData.AsString());

	//	Insert in log

	SEntry *pEntry = m_Lines.Insert();
	pEntry->dData = dData;
	pEntry->Seq = m_Seq++;
	}
