//	BlackBoxReader.h
//
//	DrHouse Engine Implementation
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#pragma once

class CBlackBoxProcessor;

class CBBRLogFiles
	{
	public:
		struct SCursor
			{
			SCursor (void) : 
					iFile(-1),
					dwOffset(0),
					dwLength(0)
				{ }

			int iFile;						//	Index into m_Files (-1 = invalid)
			DWORDLONG dwOffset;				//	Byte offset into file (always at start of a line).
			DWORDLONG dwLength;				//	Length of the current line
			};

		CBBRLogFiles (void);

		void CloseIfUnused (void);
		CDatum GetLine (const SCursor &Cursor) const;
		CDateTime GetLineDate (const SCursor &Cursor) const;
		bool Init (const CString &sPath, CString *retsError);
		bool MoveBackwards (SCursor &Cursor, CString *retsError = NULL);
		bool SearchLine (const SCursor &Cursor, const CString &sSearch, bool bNoCase = false) const;
		inline void SetAccessTime (void) { m_dwLastAccess = ::sysGetTickCount64(); }

	private:
		enum EStatus
			{
			statusClosed,					//	File is closed
			statusReady,					//	File is open and available
			statusError,					//	Error opening the file
			};

		struct SFile
			{
			SFile (void) :
					iStatus(statusClosed),
					dwLength(0)
				{ }

			EStatus iStatus;
			CString sFilename;
			CFileBuffer64 File;				//	Mapped file
			DWORDLONG dwLength;				//	If 0, we've never opened the file.
			};

		void CloseFile (int iIndex);
		char *FindLineStart (char *pPos, char *pFileStart) const;
		bool OpenFile (int iIndex, CString *retsError = NULL);
		bool RefreshFile (CString *retsError = NULL);

		void Fixup (const CString &sFilespec);

		CCriticalSection m_cs;
		CString m_sPath;					//	Path for BlackBox files
		TArray<SFile> m_Files;				//	All log files
		DWORDLONG m_dwLastAccess;			//	Accessed on this tick
	};

class CBlackBoxProcessor
	{
	public:
		enum EResults
			{
			resultDone,						//	No more processing needed
			resultContinue,					//	Need more processing
			};

		struct SOptions
			{
			SOptions (void) :
					iMaxLines(10),
					After(timeSubtractTime(CDateTime(CDateTime::Now), CTimeSpan(90, 0))),
					bNoCase(true)
				{ }

			int iMaxLines;
			CDateTime After;
			CDateTime Before;
			bool bNoCase;
			};

		CBlackBoxProcessor (void);
		~CBlackBoxProcessor (void);

		bool CreateSession (const CString &sSearch, const SOptions &Options, DWORD *retdwID, CString *retsError);
		bool GetResults (DWORD dwID, DWORD *retdwLinesSearched, CDatum *retdResult, CString *retsError);
		bool Init (const CString &sPath, CString *retsError);
		void Mark (void);
		EResults Process (void);

	private:
		enum EStatus
			{
			statusNone,						//	Session is new
			statusRunning,					//	Currently being processed
			statusComplete,					//	Complete
			statusDeleted,					//	Deleted
			};

		struct SSession
			{
			SSession (void) :
					dwID(0),
					iStatus(statusNone),
					iMaxLines(0),
					bNoCase(true),
					dwLinesChecked(0),
					Expires(0)
				{ }

			DWORD dwID;						//	Session ID
			EStatus iStatus;				//	Session status

			CString sSearch;				//	String to search for
			int iMaxLines;					//	Max lines to return
			CDateTime After;				//	On or after this date
			CDateTime Before;				//	On or before this date
			bool bNoCase;					//	Case-insensitive

			CDatum dResult;					//	Complete result set
			DWORD dwLinesChecked;			//	How many lines did we search
			DWORDLONG Expires;				//	Delete after this tick
			};

		bool CanDelete (SSession &Session, DWORDLONG dwNow) const;
		SSession *GetNextSession (void);
		void SetSessionResult (SSession *pSession, DWORD dwLinesChecked, CDatum dResult = CDatum());

		CCriticalSection m_cs;
		CBBRLogFiles m_LogFiles;			//	Set of log files

		DWORD m_dwNextSessionID;
		TSortMap<DWORD, SSession *> m_Sessions;
	};