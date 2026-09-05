//	CBlackBox.cpp
//
//	CBlackBox class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_CBLACK_BOX_LOG,	"*.log");

DECLARE_CONST_STRING(DEFAULT_PREFIX,					"BlackBox");

DECLARE_CONST_STRING(LINE_CONTINUATION_PREFIX,		"\r\n  | ")
DECLARE_CONST_STRING(LINE_END,						"\r\n")
DECLARE_CONST_STRING(SPACE,							" ")

DECLARE_CONST_STRING(ERR_UNABLE_TO_GET_VERSION,			"Unable to obtain executable version information.");

static bool ParseInteger (const char *&ioPos, const char *pPosEnd, int iMinDigits, int iMaxDigits, int *retiValue)
	{
	const char *pStart = ioPos;
	int iValue = 0;

	while (ioPos < pPosEnd
			&& *ioPos >= '0'
			&& *ioPos <= '9'
			&& (ioPos - pStart) < iMaxDigits)
		{
		iValue = (10 * iValue) + (*ioPos - '0');
		ioPos++;
		}

	int iDigits = (int)(ioPos - pStart);
	if (iDigits < iMinDigits
			|| (ioPos < pPosEnd && *ioPos >= '0' && *ioPos <= '9'))
		return false;

	if (retiValue)
		*retiValue = iValue;

	return true;
	}

void CBlackBox::Boot (const CString &sPath, const CString &sLogPrefix, bool bSetConsoleOutput)

//	Boot
//
//	Prepare the log file

	{
	ASSERT(!m_File.IsOpen());

	CString sPrefix = sLogPrefix;
	if (sPrefix.IsEmpty())
		sPrefix = DEFAULT_PREFIX;

	//	Generate a filename using the current date and time

	CDateTime Now(CDateTime::Now);
	CString sFilename = strPattern("%s_%04d%02d%02d_%02d%02d%02d.log",
			sPrefix,
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
		return;

	if (bSetConsoleOutput)
		SetConsoleOutput(true);
	}

void CBlackBox::Log (const CString &sLine)

//	Log
//
//	Log a line

	{
	try 
		{
		bool bConsoleOut = m_bConsoleOut;

		CDateTime Now(CDateTime::Now);

		//	We use a fixed timestamp format so that readers can reliably identify
		//	the start of an entry regardless of the machine's locale.

		CStringBuffer Output;
		Output.WriteIntString(Now.Month(), "%d");
		Output.WriteChar('/');
		Output.WriteIntString(Now.Day(), "%d");
		Output.WriteChar('/');
		Output.WriteIntString(Now.Year(), "%04d");
		Output.WriteChar(' ');
		Output.WriteIntString(Now.Hour(), "%d");
		Output.WriteChar(':');
		Output.WriteIntString(Now.Minute(), "%02d");
		Output.WriteChar(':');
		Output.WriteIntString(Now.Second(), "%02d");
		Output.WriteChar(' ');

		//	Normalize any embedded line endings and mark each continuation line.
		//	Aside from making multiline entries clear to a reader, the marker makes
		//	sure that continuation text can never look like a new log entry.

		const char *pPos = sLine.GetPointer();
		const char *pPosEnd = pPos + sLine.GetLength();
		const char *pLineStart = pPos;
		while (pPos < pPosEnd)
			{
			if (*pPos != '\r' && *pPos != '\n')
				{
				pPos++;
				continue;
				}

			Output.Write(pLineStart, (int)(pPos - pLineStart));
			Output.Write(LINE_CONTINUATION_PREFIX);

			if (*pPos == '\r' && pPos + 1 < pPosEnd && pPos[1] == '\n')
				pPos += 2;
			else
				pPos++;

			pLineStart = pPos;
			}

		Output.Write(pLineStart, (int)(pPosEnd - pLineStart));
		Output.Write(LINE_END);

		if (m_File.IsOpen())
			{
			int iWritten = m_File.Write(Output.GetPointer(), Output.GetLength());

			//	LATER: Handle out of disk space

			//	In Debug mode, we always output.

#ifdef DEBUG
			bConsoleOut = true;
#endif
			}

		if (bConsoleOut)
			{
			//	Write out to console
			//	NOTE: We rely on the fact that we called SetConsoleOutputCP(65001),
			//	which is UTF8.

			fwrite(Output.GetPointer(), 1, Output.GetLength(), stdout);
			}
		}
	catch (...)
		{
		}
	}

bool CBlackBox::ParseLogLineDate (const char *pPos, const char *pPosEnd, CDateTime *retDate)

//	ParseLogLineDate
//
//	Parses the fixed timestamp at the start of a BlackBox entry. We deliberately
//	do not use formatAuto because log text can contain arbitrary numbers that
//	happen to form a valid date.

	{
	int iMonth;
	if (!ParseInteger(pPos, pPosEnd, 1, 2, &iMonth)
			|| pPos == pPosEnd
			|| *pPos++ != '/')
		return false;

	int iDay;
	if (!ParseInteger(pPos, pPosEnd, 1, 2, &iDay)
			|| pPos == pPosEnd
			|| *pPos++ != '/')
		return false;

	int iYear;
	if (!ParseInteger(pPos, pPosEnd, 4, 4, &iYear)
			|| pPos == pPosEnd
			|| *pPos++ != ' ')
		return false;

	int iHour;
	if (!ParseInteger(pPos, pPosEnd, 1, 2, &iHour)
			|| pPos == pPosEnd
			|| *pPos++ != ':')
		return false;

	int iMinute;
	if (!ParseInteger(pPos, pPosEnd, 2, 2, &iMinute)
			|| pPos == pPosEnd
			|| *pPos++ != ':')
		return false;

	int iSecond;
	if (!ParseInteger(pPos, pPosEnd, 2, 2, &iSecond)
			|| (pPos < pPosEnd && *pPos != ' ' && *pPos != '\r' && *pPos != '\n'))
		return false;

	if (iYear < 1900
			|| !CDateTime::IsValidDate(iDay, iMonth, iYear)
			|| !CDateTime::IsValidTime(iHour, iMinute, iSecond))
		return false;

	if (retDate)
		*retDate = CDateTime(iDay, iMonth, iYear, iHour, iMinute, iSecond);

	return true;
	}

void CBlackBox::LogExecVersion (void)

//	LogExecVersion
//
//	Logs the executable file version.

	{
	SFileVersionInfo Version;
	if (!::fileGetVersionInfo(NULL_STR, &Version))
		{
		Log(ERR_UNABLE_TO_GET_VERSION);
		return;
		}

	Log(strPattern("%s %s", Version.sProductName, Version.sProductVersion));
	Log(Version.sCopyright);
	}

bool CBlackBox::ReadRecent (const CString &sPath, const CString &sFind, int iLines, TArray<CString> *retLines)

//	ReadRecent
//
//	Returns the most recent set of lines.

	{
	//	First we make a list of log files at the given path.

	TArray<CString> Files;
	if (!fileGetFileList(sPath, NULL_STR, STR_CBLACK_BOX_LOG, 0, &Files))
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
