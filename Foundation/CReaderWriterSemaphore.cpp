//	CReaderWriterSemaphore.cpp
//
//	CReaderWriterSemaphore class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

CReaderWriterSemaphore::CReaderWriterSemaphore (int iMaxReaders) :
		m_iMaxReaders(iMaxReaders)

//	CReaderWriterSemaphore constructor

	{
	ASSERT(m_iMaxReaders > 0);

	m_hSem = ::CreateSemaphore(NULL, m_iMaxReaders, m_iMaxReaders, NULL);
	if (m_hSem == NULL)
		throw CException(errFail);
	}

void CReaderWriterSemaphore::LockReader (void)

//	LockReader
//
//	Lock a reader

	{
	::WaitForSingleObject(m_hSem, INFINITE);
	}

void CReaderWriterSemaphore::LockWriter (void)

//	LockWriter
//
//	Lock a writer

	{
	//	Lock out all readers

	for (int i = 0; i < m_iMaxReaders; i++)
		::WaitForSingleObject(m_hSem, INFINITE);
	}

void CReaderWriterSemaphore::UnlockReader (void)

//	UnlockReader
//
//	Unlock a reader

	{
	::ReleaseSemaphore(m_hSem, 1, NULL);
	}

void CReaderWriterSemaphore::UnlockWriter (void)

//	UnlockWriter
//
//	Unlock a writer

	{
	::ReleaseSemaphore(m_hSem, m_iMaxReaders, NULL);
	}
