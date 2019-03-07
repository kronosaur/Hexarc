//	TArchonConsoleManager.h
//
//	Archon helper classes
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by ArchonEngine.h
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


#pragma once

template <class T>
class TArchonConsoleManager
	{
	public:
		TArchonConsoleManager (void) { }
		TArchonConsoleManager (const TArchonConsoleManager &Src) = delete;
		TArchonConsoleManager (TArchonConsoleManager &&Src) = delete;

		TArchonConsoleManager &operator= (const TArchonConsoleManager &Src) = delete;
		TArchonConsoleManager &operator= (TArchonConsoleManager &&Src) = delete;

		TSharedPtr<T> Alloc (void)
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

			TSharedPtr<T> pConsole(new T(sID));
			m_Consoles.SetAt(sID, pConsole);

			//	Done

			return pConsole;
			}

		void Expire (void)
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

		bool Find (const CString &sID, TSharedPtr<T> *retpConsole = NULL) const
			{
			CSmartLock Lock(m_cs);

			auto pPtr = m_Consoles.GetAt(sID);
			if (pPtr == NULL)
				return false;

			if (retpConsole)
				*retpConsole = *pPtr;

			return true;
			}

		void Mark (void)
			{
			for (int i = 0; i < m_Consoles.GetCount(); i++)
				m_Consoles[i]->Mark();
			}

	private:
		static constexpr DWORD MAX_IDLE_TIME =	15 * 60 * 1000;

		CCriticalSection m_cs;
		TSortMap<CString, TSharedPtr<T>> m_Consoles;
	};

using CArchonConsoleManager = TArchonConsoleManager<CHexeConsole>;
