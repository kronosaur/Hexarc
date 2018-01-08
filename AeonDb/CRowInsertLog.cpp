//	CRowInsertLog.cpp
//
//	CRowInsertLog class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_NO_TERMINATOR,					"Invalid serialization terminator in recovery file")
DECLARE_CONST_STRING(ERR_CANT_INSERT,					"Unable to insert row from recovery file")
DECLARE_CONST_STRING(ERR_CANT_OPEN_RECOVERY,			"Unable to open recovery file: %s.")
DECLARE_CONST_STRING(ERR_CANT_PARSE,					"Unable to parse data in recovery file")
DECLARE_CONST_STRING(ERR_CRASH_LOADING,					"Unable to read recovery file")
DECLARE_CONST_STRING(ERR_CRASH,							"Crash while inserting")

const DWORD SIGNATURE =									'ROEA';		//	'AEOR' backwards because of little-endianness
const DWORD CURRENT_VERSION =							1;

bool CRowInsertLog::Create (const CString &sFilename)

//	Create
//
//	Create a new log file

	{
	if (!m_File.Create(sFilename, CFile::FLAG_CREATE_ALWAYS | CFile::FLAG_WRITE_THROUGH))
		return false;

	m_sFilename = sFilename;
	m_dwVersion = CURRENT_VERSION;

	//	Output header

	SHeader Header;
	Header.dwSignature = SIGNATURE;
	Header.dwVersion = CURRENT_VERSION;

	m_File.Write(&Header, sizeof(Header));

	return true;
	}

bool CRowInsertLog::Insert (const CRowKey &Key, CDatum dData, SEQUENCENUMBER RowID, CString *retsError)

//	Insert
//
//	Insert a row

	{
	ASSERT(m_File.IsOpen());

	//	Write out to a memory buffer first

	CBuffer Output;

	//	Save the key

	Key.AsEncodedString().Serialize(Output);

	//	Save the data

	dData.Serialize(CDatum::formatAEONScript, Output);

	//	We need a terminator for the datum serialization

	if (m_dwVersion > 0)
		Output.Write(" ", 1);

	//	Save the rowID

	Output.Write(&RowID, sizeof(RowID));

	//	Write to the file

	try
		{
		m_File.Write(Output.GetPointer(), Output.GetLength());
		}
	catch (CException e)
		{
		if (retsError)
			*retsError = e.GetErrorString();
		return false;
		}
	catch (...)
		{
		if (retsError)
			*retsError = ERR_CRASH;
		return false;
		}

	//	Success

	return true;
	}

bool CRowInsertLog::Open (const CString &sFilename, CAeonRowArray *pRows, int *retiRowCount, CString *retsError)

//	Open
//
//	Open a log file.

	{
	if (!m_File.IsOpen())
		{
		ASSERT(!sFilename.IsEmpty());

		if (!m_File.Create(sFilename, CFile::FLAG_OPEN_ALWAYS))
			{
			*retsError = strPattern(ERR_CANT_OPEN_RECOVERY, sFilename);
			return false;
			}

		//	Open successful

		m_sFilename = sFilename;

		//	See what version we're at. Version 0 did not have a header, so we
		//	need to account for that.

		DWORD dwFileSize = m_File.GetStreamLength();
		if (dwFileSize >= sizeof(SHeader))
			{
			SHeader Header;

			m_File.Read(&Header, sizeof(SHeader));
			if (Header.dwSignature == SIGNATURE)
				m_dwVersion = Header.dwVersion;

			//	Version 0 recovery file.

			else
				m_dwVersion = 0;
			}

		//	If we've created a new recovery file then initialize the header

		else if (dwFileSize == 0)
			{
			SHeader Header;
			Header.dwSignature = SIGNATURE;
			Header.dwVersion = CURRENT_VERSION;

			m_File.Write(&Header, sizeof(SHeader));

			m_dwVersion = CURRENT_VERSION;
			}

		//	Otherwise we've got a version 0 recovery file.

		else
			m_dwVersion = 0;
		}

	//	See if there is anything in the file that we need to recover
	//	NOTE: We leave the file pointer at the end.

	if (!Recover(pRows, retiRowCount, retsError))
		{
		*retsError = strPattern("%s: %s", *retsError, sFilename);
		return false;
		}

	//	Done

	return true;
	}

bool CRowInsertLog::Recover (CAeonRowArray *pRows, int *retiRowCount, CString *retsError)

//	Recover
//
//	Recover a row array from a log file

	{
	int iRowCount = 0;
	int iTotalSize = m_File.GetStreamLength();
	if (iTotalSize == 0)
		{
		if (retiRowCount)
			*retiRowCount = 0;
		return true;
		}

	//	Start at the beginning

	m_File.Seek(m_dwVersion == 0 ? 0 : sizeof(SHeader));

	//	Read it

	while (m_File.GetPos() < iTotalSize)
		{
		try
			{
			//	Key

			CString sKey = CString::Deserialize(m_File);
			CRowKey Key;
			CRowKey::CreateFromEncodedKey(pRows->GetDimensions(), sKey, &Key);

			//	Data

			CDatum dData;
			if (!CDatum::Deserialize(CDatum::formatAEONScript, m_File, &dData))
				{
				*retsError = ERR_CANT_PARSE;
				return false;
				}

			//	Skip terminator, if necessary

			if (m_dwVersion > 0)
				{
				char chTerm;
				m_File.Read(&chTerm, 1);
				if (chTerm != ' ')
					{
					*retsError = ERR_NO_TERMINATOR;
					return false;
					}
				}

			//	RowID

			SEQUENCENUMBER RowID;
			m_File.Read(&RowID, sizeof(RowID));

			//	Add to row

			if (!pRows->Insert(Key, dData, RowID))
				{
				*retsError = ERR_CANT_INSERT;
				return false;
				}

			iRowCount++;
			}
		catch (...)
			{
			*retsError = ERR_CRASH_LOADING;
			return false;
			}
		}

	//	Done

	if (retiRowCount)
		*retiRowCount = iRowCount;

	return true;
	}

bool CRowInsertLog::Reset (void)

//	Reset
//
//	Delete the file

	{
	ASSERT(m_File.IsOpen());

	//	Re-write to latest version

	if (m_dwVersion != CURRENT_VERSION)
		{
		SHeader Header;
		Header.dwSignature = SIGNATURE;
		Header.dwVersion = CURRENT_VERSION;

		m_File.Seek(0);
		m_File.Write(&Header, sizeof(SHeader));

		m_dwVersion = CURRENT_VERSION;
		}

	return m_File.SetLength(sizeof(SHeader));
	}
