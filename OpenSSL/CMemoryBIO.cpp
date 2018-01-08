//	CMemoryBIO.cpp
//
//	CMemoryBIO class
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CMemoryBIO::CMemoryBIO (void) : 
		m_pBIO(NULL)

//	CMemoryBIO constructor

	{
	}

CMemoryBIO::CMemoryBIO (const CString &sData) :
		m_pBIO(NULL)

//	CMemoryBIO constructor

	{
	Write(sData);
	}

CMemoryBIO::CMemoryBIO (const IMemoryBlock &Data) :
		m_pBIO(NULL)

//	CMemoryBIO constructor

	{
	Write(Data);
	}

void CMemoryBIO::CleanUp (void)

//	CleanUp
//
//	Free resources

	{
	if (m_pBIO)
		{
		BIO_free(m_pBIO);
		m_pBIO = NULL;
		}
	}

void CMemoryBIO::Create (void)

//	Create
//
//	Create memory BIO

	{
	CleanUp();
	m_pBIO = BIO_new(BIO_s_mem());
	}

bool CMemoryBIO::Write (const CString &sData)

//	Write
//
//	Write to the BIO

	{
	if (m_pBIO == NULL)
		Create();

	BIO_write(m_pBIO, sData.GetPointer(), sData.GetLength());
	return true;
	}

bool CMemoryBIO::Write (const IMemoryBlock &Data)

//	Write
//
//	Write to the BIO

	{
	if (m_pBIO == NULL)
		Create();

	BIO_write(m_pBIO, Data.GetPointer(), Data.GetLength());
	return true;
	}
