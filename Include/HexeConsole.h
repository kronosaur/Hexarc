//	HexeConsole.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Included by Hexe.h

#pragma once

class CHexeConsole
	{
	public:
		CHexeConsole (const CString &sID = NULL_STR);

		CHexeConsole (const CHexeConsole &Src) = delete;
		CHexeConsole (CHexeConsole &&Src) = delete;
		CHexeConsole &operator= (const CHexeConsole &Src) = delete;
		CHexeConsole &operator= (CHexeConsole &&Src) = delete;

		void AddReaderAccess (const CString &sUsername);
		inline CHexeConsole *AddRef (void) { CSmartLock Lock(m_cs); m_dwRefCount++; return this; }
		void Close (void);
		inline void Delete (void) { CSmartLock Lock(m_cs); if (--m_dwRefCount == 0) delete this; }
		CDatum GetConsoleData (DWORD Seq) const;
		inline const CString &GetID (void) const { return m_sID; }
		inline DWORDLONG GetLastAccessTime (void) const { return m_dwLastAccess; }
		inline DWORD GetLastSeq (void) const { CSmartLock Lock(m_cs); return m_Seq; }
		bool HasReaderAccess (const CString &sUsername) const;
		void Mark (void);
		void OutputData (CDatum dData);
//		void OutputText (const CString &sText);

	private:
		struct SEntry
			{
			DWORD Seq;
			CDatum dData;
			};

		int FindFirstLine (DWORD Seq) const;

		CCriticalSection m_cs;
		CString m_sID;
		DWORD m_dwRefCount = 1;
		DWORD m_Seq = 0;
		TArray<SEntry> m_Lines;

		TArray<CString> m_Readers;
		DWORDLONG m_dwLastAccess = 0;
		bool m_bClosed = false;
	};
