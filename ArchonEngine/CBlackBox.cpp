//	CBlackBox.cpp
//
//	CBlackBox class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void CBlackBox::Boot (const CString &sPath)

//	Boot
//
//	Prepare the log file

	{
	ASSERT(!m_File.IsOpen());

	//	Generate a filename using the current date and time

	CDateTime Now(CDateTime::Now);
	CString sFilename = strPattern("BlackBox_%04d%02d%02d_%02d%02d%02d.log",
			Now.Year(),
			Now.Month(),
			Now.Day(),
			Now.Hour(),
			Now.Minute(),
			Now.Second());

	//	Create the file

	CString sError;
	if (!m_File.Create(fileAppend(sPath, sFilename), CFile::FLAG_CREATE_ALWAYS, &sError))
		//	LATER: Handle error somehow.
		NULL;
	}

void CBlackBox::Log (const CString &sLine)

//	Log
//
//	Log a line

	{
	CDateTime Now(CDateTime::Now);
	CString sOutput = strPattern("%s %s\r\n",
			Now.Format(CDateTime::dfShort, CDateTime::tfLong24),
			sLine);

	if (m_File.IsOpen())
		{
		int iWritten = m_File.Write(sOutput);

		//	LATER: Handle out of disk space

		//	In Debug mode, we always output.

#ifdef DEBUG
		printf(sOutput);
#endif
		}
	else if (m_bConsoleOut)
		{
		//	Write out to console
		//	NOTE: We rely on the fact that we called SetConsoleOutputCP(65001),
		//	which is UTF8.

		printf(sOutput);
		}
	}

bool CBlackBox::ReadRecent (const CString &sPath, const CString &sFind, int iLines, TArray<CString> *retLines)

//	ReadRecent
//
//	Returns the most recent set of lines.

	{
	//	First we make a list of log files at the given path.

	TArray<CString> Files;
	if (!fileGetFileList(sPath, NULL_STR, CString("*.log"), 0, &Files))
		return false;

	//	Now sort them in reverse chronological order (we can do this because we
	//	have encoded the date in the name).

	Files.Sort(DescendingSort);

	//	Now loop until we have filled all the lines (or until we run out of log
	//	files).

	int iLogFile = 0;
	int iLinesLeft = iLines;
	while (iLogFile < Files.GetCount() && iLinesLeft > 0)
		{
		//	Open the log file for read-only

		CFileBuffer LogFile;
		if (!LogFile.OpenReadOnly(Files[iLogFile]))
			return false;

		//	Parse backwards until we reach the number of lines that we want
		//	or until we reach the beginning of the file.

		char *pBoF = LogFile.GetPointer();
		char *pEoF = pBoF + LogFile.GetLength();
		char *pStart = pEoF;
		while (pStart > pBoF && iLinesLeft > 0)
			{
			//	Remember the end of the line.

			char *pLineEnd = pStart;

			//	Go backwards until we hit a line ending

			while (pStart > pBoF && pStart[-1] != '\n')
				pStart--;

			//	We're at the beginning of the line so get the whole thing. If
			//	this is a line that we want, then add it to the result.

			CString sLine(pStart, (int)(pLineEnd - pStart));
			if (!sLine.IsEmpty()
					&& (sFind.IsEmpty() || strFind(sLine, sFind) != -1))
				{
				//	We add at the beginning because we are reading backwards

				retLines->Insert(sLine, 0);

				//	We got a line

				iLinesLeft--;
				}

			//	Now move backwards to skip the line ending

			if (pStart > pBoF && pStart[-1] == '\n')
				pStart--;

			if (pStart > pBoF && pStart[-1] == '\r')
				pStart--;
			}

		//	Next file

		LogFile.Close();
		iLogFile++;
		}

	//	Done

	return true;
	}

void CBlackBox::Shutdown (void)

//	Shutdown
//
//	Done

	{
	m_File.Close();
	}
