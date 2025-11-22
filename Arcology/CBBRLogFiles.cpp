//	CBBRLogFiles.cpp
//
//	CBBRLogFiles class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FILESPEC_DOT_LOG,					"BlackBox*.log")

DECLARE_CONST_STRING(ERR_DIRECTORY_FAILED,				"Unable to find log files at: %s.")
DECLARE_CONST_STRING(ERR_CRASH_GET_LINE,				"Crash in CBBRLogFiles::GetLine on file: %s.")
DECLARE_CONST_STRING(ERR_CRASH_MOVE_BACKWARDS,			"Crash in CBBRLogFiles::MoveBackwards.")

const DWORDLONG MAX_OPEN_TIME =							5 * 60 * 1000;

CBBRLogFiles::CBBRLogFiles (void) :
		m_dwLastAccess(0)

//	CBBRLogFiles constructor

	{
	}

void CBBRLogFiles::CloseFile (int iIndex)

//	CloseFile
//
//	Close the file

	{
	CSmartLock Lock(m_cs);
	SFile &FileData = m_Files[iIndex];

	if (FileData.iStatus == statusReady)
		{
		FileData.File.Close();
		FileData.iStatus = statusClosed;
		}
	}

void CBBRLogFiles::CloseIfUnused (void)

//	CloseIfUnused
//
//	If it's been a while since our last access, close all the files
//	to save memory.

	{
	CSmartLock Lock(m_cs);
	int i;

	if (m_dwLastAccess != 0
			&& m_dwLastAccess + MAX_OPEN_TIME < ::sysGetTickCount64())
		{
		for (i = 0; i < m_Files.GetCount(); i++)
			CloseFile(i);

		//	Since all files are closed, reset the last access to 0 so that we
		//	don't bother trying to close again and again.

		m_dwLastAccess = 0;
		}
	}

char *CBBRLogFiles::FindLineStart (char *pPos, char *pFileStart) const

//	FindLineStart
//
//	Returns the start of the line.

	{
	while (pPos > pFileStart)
		{
		//	Look for an LF

		while (pPos > pFileStart && pPos[-1] != '\n')
			pPos--;

		if (pPos == pFileStart)
			return pPos;

		//	Remember this position because it could be the start of a line.

		char *pLineStart = pPos;

		//	If the previous character is NOT CR, then this is not the beginning
		//	of the line, so we continue.

		pPos--;
		if (pPos == pFileStart)
			return pPos;

		if (pPos[-1] != '\r')
			continue;

		//	Otherwise, check to see if the line starts with a date.

		//	Found it.

		return pLineStart;
		}

	return pPos;
	}

CDatum CBBRLogFiles::GetLine (const SCursor &Cursor) const

//	GetLine
//
//	Get the line at the cursor

	{
	CSmartLock Lock(m_cs);
	ASSERT(Cursor.iFile != -1);
	SFile &FileData = m_Files[Cursor.iFile];

	try
		{
		ASSERT(FileData.iStatus == statusReady);
		char *pPos = FileData.File.GetPointer() + Cursor.dwOffset;

		CDatum dLine(CString(pPos, Cursor.dwLength));
		return dLine;
		}
	catch (...)
		{
		return CDatum(strPattern(ERR_CRASH_GET_LINE, FileData.sFilename));
		}
	}

CDateTime CBBRLogFiles::GetLineDate (const SCursor &Cursor) const

//	GetLineDate
//
//	Returns the datetime stamp of the line.

	{
	CSmartLock Lock(m_cs);
	ASSERT(Cursor.iFile != -1);
	SFile &FileData = m_Files[Cursor.iFile];

	try
		{
		ASSERT(FileData.iStatus == statusReady);
		char *pPos = FileData.File.GetPointer() + Cursor.dwOffset;
		char *pPosEnd = pPos + Cursor.dwLength;

		CDateTime Value;
		if (!CDateTime::Parse(CDateTime::formatAuto, pPos, pPosEnd, &Value))
			return CDateTime();

		return Value;
		}
	catch (...)
		{
		return CDateTime();
		}
	}

bool CBBRLogFiles::Init (const CString &sPath, CString *retsError)

//	Init
//
//	Loads the list of log files. Since we never create new log files while the
//	server is running, this only has to be called once at start up.

	{
	int i;

	m_sPath = sPath;

#ifdef DEBUG_FIXUP
	Fixup(fileAppend(sPath, CString("BlackBox_20170124_034229.log")));
#endif

	//	First we make a list of log files at the given path.

	TArray<CString> Files;
	if (!fileGetFileList(sPath, NULL_STR, FILESPEC_DOT_LOG, 0, &Files))
		{
		if (retsError) *retsError = strPattern(ERR_DIRECTORY_FAILED, sPath);
		return false;
		}

	//	We must have at least 1 file

	if (Files.GetCount() == 0)
		{
		if (retsError) *retsError = strPattern(ERR_DIRECTORY_FAILED, sPath);
		return false;
		}

	//	Now sort them in reverse chronological order (we can do this because we
	//	have encoded the date in the name).

	Files.Sort(DescendingSort);

	//	Now create a list of files in the proper order

	m_Files.DeleteAll();
	m_Files.InsertEmpty(Files.GetCount());

	//	Add the other files.

	for (i = 0; i < Files.GetCount(); i++)
		{
		m_Files[i].sFilename = Files[i];
		m_Files[i].dwLength = fileGetSize(Files[i]);
		}

	//	Done

	return true;
	}

bool CBBRLogFiles::MoveBackwards (SCursor &Cursor, CString *retsError)

//	MoveBackwards
//
//	Move the cursor back one line. If there are no more lines, then we return
//	FALSE (and the cursor is set to invalid).
//
//	If we get an error, we return FALSE and retsError is not empty.

	{
	CSmartLock Lock(m_cs);

	try
		{
		//	If we're uninitialized, start at the end of the first file.

		if (Cursor.iFile == -1)
			{
			//	The first file is the latest, so we close it and reopen it so that
			//	we get the latest data.

			if (!RefreshFile(retsError))
				return false;

			Cursor.iFile = 0;
			Cursor.dwOffset = m_Files[0].dwLength;
			}

		//	Otherwise, make sure the file is open

		else
			{
			if (!OpenFile(Cursor.iFile, retsError))
				return false;
			}

		//	If we're at the beginning of the file, then we need to go to the next
		//	file.

		if (Cursor.dwOffset == 0)
			{
			//	If there are no more files, then we've hit the end.

			Cursor.iFile++;
			if (Cursor.iFile >= m_Files.GetCount())
				{
				Cursor.iFile = -1;
				Cursor.dwOffset = 0;
				return false;
				}

			//	Otherwise, start at the end of the file.

			Cursor.dwOffset = m_Files[Cursor.iFile].dwLength;
			return MoveBackwards(Cursor, retsError);
			}

		//	Get a pointer

		char *pStart = m_Files[Cursor.iFile].File.GetPointer();
		char *pPos = pStart + Cursor.dwOffset;

		//	Skip CRLF, if any

		while (pPos > pStart && (pPos[-1] == '\n' || pPos[-1] == '\r'))
			pPos--;

		//	We're now pointing to the end of the previous line (the line we're 
		//	trying to select). We remember it here so that we don't have to
		//	calculate the line length later.

		char *pLineEnd = pPos;

		//	Keep going until we hit the start of the file or until we hit a CRLF.

		pPos = FindLineStart(pPos, pStart);

		//	We're now pointing at the beginning of the line.

		Cursor.dwOffset = (pPos - pStart);
		Cursor.dwLength = (pLineEnd - pPos);

		return true;
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_CRASH_MOVE_BACKWARDS;
		return false;
		}
	}

bool CBBRLogFiles::OpenFile (int iIndex, CString *retsError)

//	OpenFile
//
//	Makes sure the file is open.

	{
	CSmartLock Lock(m_cs);
	SFile &FileData = m_Files[iIndex];

	if (FileData.iStatus == statusClosed)
		{
		if (!FileData.File.OpenReadOnly(FileData.sFilename, retsError))
			return false;

		FileData.iStatus = statusReady;
		}

	return true;
	}

bool CBBRLogFiles::RefreshFile (CString *retsError)

//	RefreshFile
//
//	Reopen the 0th file, which is the tail part of the current log file.

	{
	CSmartLock Lock(m_cs);
	SFile &FileData = m_Files[0];

	//	Close the file if it is open.

	CloseFile(0);

	//	Open the file starting at all the new logs

	if (!FileData.File.OpenReadOnly(FileData.sFilename, retsError))
		return false;

	FileData.iStatus = statusReady;
	FileData.dwLength = FileData.File.GetLength();

	return true;
	}

bool CBBRLogFiles::SearchLine (const SCursor &Cursor, const CString &sSearch, bool bNoCase) const

//	SearchLine
//
//	Searches the line for the given text.
//
//	NOTE: For a case-insensitive search the caller must guarantee that sSearch
//	is lowercase.

	{
	CSmartLock Lock(m_cs);
	ASSERT(Cursor.iFile != -1);
	SFile &FileData = m_Files[Cursor.iFile];

	try
		{
		ASSERT(FileData.iStatus == statusReady);
		const char *pPos = FileData.File.GetPointer() + Cursor.dwOffset;
		const char *pPosEnd = pPos + Cursor.dwLength;

		const char *pSearch = sSearch.GetPointer();
		const char *pSearchEnd = pSearch + sSearch.GetLength();

		const char *pPosLast = pPosEnd - sSearch.GetLength();

		//	If we're doing a case-sensitive search, then it's pretty easy.

		if (!bNoCase)
			{
			while (pPos <= pPosLast)
				{
				if (*pPos == *pSearch)
					{
					//	See how far we match

					const char *pTarget = pPos + 1;
					const char *pToFind = pSearch + 1;
					while (pToFind < pSearchEnd && *pTarget == *pToFind)
						{
						pTarget++;
						pToFind++;
						}

					//	Found it?

					if (pToFind == pSearchEnd)
						return true;
					}

				//	Next

				pPos++;
				}

			//	Not found

			return false;
			}

		//	Else we do a case-insensitive search

		else
			{
			UTF32 dwSearchFirstChar = strParseUTF8Char(&pSearch, pSearchEnd);

			while (pPos <= pPosLast)
				{
				UTF32 dwPos = strParseUTF8Char(&pPos, pPosEnd);

				if (strToLowerChar(dwPos) == dwSearchFirstChar)
					{
					//	See how far we match

					bool bFound = true;
					const char *pTarget = pPos;
					const char *pToFind = pSearch;
					while (pToFind < pSearchEnd)
						{
						UTF32 dwTarget = strParseUTF8Char(&pTarget, pPosEnd);
						UTF32 dwToFind = strParseUTF8Char(&pToFind, pSearchEnd);

						if (strToLowerChar(dwTarget) != dwToFind)
							{
							bFound = false;
							break;
							}
						}

					//	Found it?

					if (bFound)
						return true;
					}
				}

			//	Not found

			return false;
			}
		}
	catch (...)
		{
		return false;
		}
	}

void CBBRLogFiles::Fixup (const CString &sFilespec)
	{
	CString sBadLine("ERROR: Exarch: Unhandled reply: Error.unableToComply Connection lost.");
	char *pBad = sBadLine.GetParsePointer();
	char *pBadEnd = pBad + sBadLine.GetLength();

	CString sError;
	CFileBuffer64 SrcFile;
	if (!SrcFile.OpenReadOnly(sFilespec, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return;
		}

	CString sDestFilespec = fileAppend(m_sPath, CString("Fixup.log"));
	CFile DestFile;
	if (!DestFile.Create(sDestFilespec, CFile::FLAG_CREATE_ALWAYS, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return;
		}

	char *pSrc = SrcFile.GetPointer();
	char *pSrcEnd = pSrc + SrcFile.GetLength();

	int iGoodLines = 0;
	int iBadLines = 0;

	//	Loop over each line in the source, writing out the lines that we want.

	while (pSrc < pSrcEnd)
		{
		char *pLineStart = pSrc;

		//	Find the CRLF

		while (pSrc < pSrcEnd && *pSrc != '\r' && *pSrc != '\n')
			pSrc++;

		while (pSrc < pSrcEnd && (*pSrc == '\r' || *pSrc == '\n'))
			pSrc++;

		//	We've got the end of the line

		char *pLineEnd = pSrc;

		//	Does this line have the text we're looking for?

		bool bFound = false;
		char *pPos1 = pLineStart;
		char *pPos1Last = pLineEnd - sBadLine.GetLength();
		while (pPos1 <= pPos1Last)
			{
			if (*pPos1 == *pBad)
				{
				//	See how far we match

				char *pTarget = pPos1 + 1;
				char *pToFind = pBad + 1;
				while (pToFind < pBadEnd && *pTarget == *pToFind)
					{
					pTarget++;
					pToFind++;
					}

				//	Found it?

				if (pToFind == pBadEnd)
					{
					bFound = true;
					break;
					}
				}

			//	Next

			pPos1++;
			}

		//	If we found the bad line, then skip it.

		if (bFound)
			{
			iBadLines++;

			if ((iBadLines % 1000) == 0)
				printf("Skipped %d bad lines.\n", iBadLines);
			continue;
			}

		//	Otherwise, write the line to the destination

		DestFile.Write(pLineStart, (pLineEnd - pLineStart));
		iGoodLines++;
		}

	//	Done

	printf("Skipped %d bad lines.\nWrote %d good lines.\n", iBadLines, iGoodLines);
	}
