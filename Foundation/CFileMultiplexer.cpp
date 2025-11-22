//	CFileMultiplexer.cpp
//
//	CFileMultiplexer class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_MIRROR_SAME_AS_PRIMARY,		"Mirror file cannot be same as primary: %s.")
DECLARE_CONST_STRING(ERR_CANT_CREATE_MIRROR,			"Unable to create mirror file: %s.")

void CFileMultiplexer::Close (void)

//	Close
//
//	Close all files

	{
	int i;

	m_Primary.Close();

	for (i = 0; i < m_Mirrors.GetCount(); i++)
		m_Mirrors[i].Close();
	}

bool CFileMultiplexer::Create (const CString &sFilespec, DWORD dwFlags, CString *retsError)

//	Create
//
//	Creates a new primary file.

	{
	ASSERT(!m_Primary.IsOpen());

	//	Clear out all mirrors

	m_Mirrors.DeleteAll();

	//	Create the primary file

	return m_Primary.Create(sFilespec, dwFlags, retsError);
	}

bool CFileMultiplexer::CreateMirror (const CString &sFilespec, CString *retsError)

//	CreateMirror
//
//	Creates a mirror file.

	{
	ASSERT(m_Primary.IsOpen());

	if (strEquals(sFilespec, m_Primary.GetFilespec()))
		{
		if (retsError)
			*retsError = strPattern(ERR_MIRROR_SAME_AS_PRIMARY, sFilespec);
		return false;
		}

	//	Create the mirror

	CFile *pMirror = m_Mirrors.Insert();
	if (!pMirror->Create(sFilespec, CFile::FLAG_OPEN_ALWAYS, retsError))
		{
		m_Mirrors.Delete(m_Mirrors.GetCount() - 1);
		return false;
		}

	//	If the mirror is not the same size as the primary then we copy
	//	the primary over.
	//
	//	NOTE: This is a not-very-accurate heuristic; in the future we should
	//	add a flag to do a checksum comparison.

	DWORD dwPrimarySize = m_Primary.GetStreamLength();
	if ((DWORD)pMirror->GetStreamLength() != dwPrimarySize)
		{
		//	Allocate a buffer to copy

		DWORD dwBufferSize = 1024 * 1024;
		char *pBuffer = new char [dwBufferSize];

		//	Copy

		try
			{
			//	Remember the current seek position of the primary and seek to
			//	the beginning.

			int iOldPos = m_Primary.GetPos();
			m_Primary.Seek(0);

			//	Seek the mirror to the beginning

			pMirror->Seek(0);

			//	Copy

			DWORD dwLeftToCopy = dwPrimarySize;
			while (dwLeftToCopy > 0)
				{
				DWORD dwSize = Min(dwBufferSize, dwLeftToCopy);

				m_Primary.Read(pBuffer, dwSize);
				pMirror->Write(pBuffer, dwSize);

				dwLeftToCopy -= dwSize;
				}

			//	Restore primary position

			m_Primary.Seek(iOldPos);

			//	Set the proper size

			pMirror->SetLength(dwPrimarySize);
			}
		catch (...)
			{
			delete [] pBuffer;

			if (retsError)
				*retsError = strPattern(ERR_CANT_CREATE_MIRROR, sFilespec);
			return false;
			}

		//	Done

		delete [] pBuffer;
		}

	//	Make sure we seek to the right place on the mirror

	try
		{
		pMirror->Seek(m_Primary.GetPos());
		}
	catch (...)
		{
		if (retsError)
			*retsError = strPattern(ERR_CANT_CREATE_MIRROR, sFilespec);
		return false;
		}

	//	Done

	return true;
	}

bool CFileMultiplexer::Delete (void)

//	Delete
//
//	Close and delete all files

	{
	int i;

	//	Delete the primary file

	CString sFilespec = m_Primary.GetFilespec();
	m_Primary.Close();
	bool bSuccess = fileDelete(sFilespec);

	//	Close and delete all mirrors

	for (i = 0; i < m_Mirrors.GetCount(); i++)
		{
		sFilespec = m_Mirrors[i].GetFilespec();
		m_Mirrors[i].Close();
		fileDelete(sFilespec);
		}

	return bSuccess;
	}

bool CFileMultiplexer::Flush (void)

//	Flush
//
//	Flush all files

	{
	int i;
	
	//	Flush the primary file

	bool bSuccess = m_Primary.Flush();

	//	Flush all mirrors

	for (i = 0; i < m_Mirrors.GetCount(); i++)
		m_Mirrors[i].Flush();

	//	Done

	return bSuccess;
	}

int CFileMultiplexer::Read (void *pData, int iLength)

//	Read
//
//	Read

	{
	int i;

	//	Read

	int iResult = m_Primary.Read(pData, iLength);

	//	Seek all mirrors

	for (i = 0; i < m_Mirrors.GetCount(); i++)
		{
		try
			{
			m_Mirrors[i].Seek(m_Mirrors[i].GetPos() + iLength);
			}
		catch (...)
			{
			//	LATER: Figure out what to do with the error.
			}
		}

	//	Done

	return iResult;
	}

void CFileMultiplexer::Seek (int iPos, bool bFromEnd)

//	Seek
//
//	Seek

	{
	int i;

	//	Seek

	m_Primary.Seek(iPos, bFromEnd);

	//	Seek all mirrors

	for (i = 0; i < m_Mirrors.GetCount(); i++)
		{
		try
			{
			m_Mirrors[i].Seek(iPos, bFromEnd);
			}
		catch (...)
			{
			//	LATER: Figure out what to do with the error.
			}
		}
	}

int CFileMultiplexer::Write (const void *pData, int iLength)

//	Write
//
//	Write

	{
	int i;

	//	Read

	int iResult = m_Primary.Write(pData, iLength);

	//	Write to all mirrors

	for (i = 0; i < m_Mirrors.GetCount(); i++)
		{
		try
			{
			m_Mirrors[i].Write(pData, iLength);
			}
		catch (...)
			{
			//	LATER: Figure out what to do with the error.
			}
		}

	//	Done

	return iResult;
	}
