//	CArchonConsoleManager.cpp
//
//	CArchonConsoleManager class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	CArchonConsoleManager is designed for engines to support console IO with
//	users (usually via AI2).
//
//	When a user makes a call that might require prolonged output, the engine
//	should allocate a new console and return console data back to AI2.
//
//	The AI2 program should call back to get console data (passing a consoleID).
//	The engine can then find the console and return data up to the given 
//	sequence number.
//
//	The engine should call Expire periodically (in Housekeeping) to delete 
//	unused consoles.

#include "stdafx.h"

//	If a console has not been accessed for 15 minutes, then we expire it.

static constexpr DWORD MAX_IDLE_TIME =	15 * 60 * 1000;

TSharedPtr<CArchonConsole> CArchonConsoleManager::Alloc (void)

//	Alloc
//
//	Allocates a new console

	{
	CSmartLock Lock(m_cs);

	//	Generate a unique ID

	CString sID;
	do
		{
		sID = cryptoRandomCode(8);
		}
	while (Find(sID));

	//	Create a new console and add it to our list

	TSharedPtr<CArchonConsole> pConsole(new CArchonConsole(sID));
	m_Consoles.SetAt(sID, pConsole);

	//	Done

	return pConsole;
	}

void CArchonConsoleManager::Expire (void)

//	Expire
//
//	Expire any consoles that have not been accessed for a while.

	{
	CSmartLock Lock(m_cs);
	
	auto dwNow = ::sysGetTickCount64();
	for (int i = 0; i < m_Consoles.GetCount(); i++)
		{
		if (m_Consoles[i]->GetLastAccessTime() + MAX_IDLE_TIME < dwNow)
			{
			m_Consoles[i]->Delete();
			m_Consoles.Delete(i);
			i--;
			}
		}
	}

bool CArchonConsoleManager::Find (const CString &sID, TSharedPtr<CArchonConsole> *retpConsole) const

//	Find
//
//	Finds a console by ID. We return FALSE if the console could not be found.

	{
	CSmartLock Lock(m_cs);

	auto pPtr = m_Consoles.GetAt(sID);
	if (pPtr == NULL)
		return false;

	if (retpConsole)
		*retpConsole = *pPtr;

	return true;
	}
