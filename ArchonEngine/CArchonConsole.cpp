//	CArchonConsole.cpp
//
//	CArchonConsole class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CONSOLE_ID,					"consoleID");
DECLARE_CONST_STRING(FIELD_DONE,						"done");
DECLARE_CONST_STRING(FIELD_RESULT,						"result");
DECLARE_CONST_STRING(FIELD_SEQ,							"seq");

CArchonConsole::CArchonConsole (const CString &sID) :
		m_sID(sID),
		m_dwLastAccess(::sysGetTickCount64())

//	CArchonConsole constructor

	{
	}

void CArchonConsole::AddReaderAccess (const CString &sUsername)

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

void CArchonConsole::Close (void)

//	Close
//
//	Close the console so that there is no more output.

	{
	CSmartLock Lock(m_cs);
	m_bClosed = true;
	}

int CArchonConsole::FindFirstLine (DWORD Seq) const

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

CDatum CArchonConsole::GetConsoleData (DWORD Seq) const

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
		CStringBuffer Lines;

		for (int i = iFirst; i < m_Lines.GetCount(); i++)
			{
			if (i != iFirst)
				Lines.Write("\n", 1);

			Lines.Write(m_Lines[i].sLine);
			}

		CDatum::CreateStringFromHandoff(Lines, &dLines);
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

bool CArchonConsole::HasReaderAccess (const CString &sUsername) const

//	HasReaderAccess
//
//	Returns TRUE if the given username can read the console.
//	NOTE: We assume a canonical username (lowercase)

	{
	CSmartLock Lock(m_cs);
	return m_Readers.Find(sUsername);
	}

void CArchonConsole::OnOutputLine (const CString &sLine)

//	OnOutputLine
//
//	Outputs a line to the console.

	{
	CSmartLock Lock(m_cs);

	SEntry *pEntry = m_Lines.Insert();
	pEntry->sLine = sLine;
	pEntry->Seq = m_Seq++;
	}
