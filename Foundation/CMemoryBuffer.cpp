//	CMemoryBuffer.cpp
//
//	CMemoryBuffer class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int ALLOC_SIZE =						4096;
const int DEFAULT_MAX_SIZE =				1024 * 1024;

CMemoryBuffer::CMemoryBuffer (int iMaxSize) :
		m_iMaxSize(iMaxSize > 0 ? AlignUp(iMaxSize, ALLOC_SIZE) : DEFAULT_MAX_SIZE),
		m_iCommittedSize(0),
		m_iCurrentSize(0),
		m_pBlock(NULL)

//	CMemoryBuffer constructor

	{
	//	Reserve a block of memory equal to the maximum size requested

	m_pBlock = (char *)::VirtualAlloc(NULL, m_iMaxSize, MEM_RESERVE, PAGE_NOACCESS);
	if (m_pBlock == NULL)
		throw CException(errOutOfMemory);
	}

CMemoryBuffer::CMemoryBuffer (void *pSource, int iLength) :
		m_iMaxSize(-1),
		m_iCommittedSize(iLength),
		m_iCurrentSize(iLength),
		m_pBlock((char *)pSource)

//	CMemoryBuffer constructor

	{
	}

CMemoryBuffer::~CMemoryBuffer (void)

//	CMemoryBuffer destructor

	{
	if (m_pBlock && !IsConstant())
		{
		::VirtualFree(m_pBlock, m_iCommittedSize, MEM_DECOMMIT);
		::VirtualFree(m_pBlock, 0, MEM_RELEASE);
		}
	}

void CMemoryBuffer::SetLength (int iLength)

//	SetLength
//
//	Sets the length

	{
	if (iLength > m_iCommittedSize)
		{
		int iAdditionalSize;

		if (IsConstant())
			{
			ASSERT(false);
			throw CException(errOutOfMemory);
			}

		//	Figure out how much to add

		iAdditionalSize = AlignUp(iLength, ALLOC_SIZE) - m_iCommittedSize;

		//	Figure out if we're over the limit. We cannot rely on VirtualAlloc
		//	to keep track of our maximum reservation

		if (m_iCommittedSize + iAdditionalSize > m_iMaxSize)
			{
			//	Allocate a new, bigger virtual block

			int iNewMaxSize = (m_iMaxSize < 0x3fff0000 ? Max(m_iMaxSize * 2, m_iCommittedSize + iAdditionalSize) : 0x7fff0000);
			char *pNewBlock = (char *)::VirtualAlloc(NULL, iNewMaxSize, MEM_RESERVE, PAGE_NOACCESS);
			if (pNewBlock == NULL)
				throw CException(errOutOfMemory);

			//	Commit and copy the new block

			if (m_iCommittedSize > 0)
				{
				if (::VirtualAlloc(pNewBlock, m_iCommittedSize, MEM_COMMIT, PAGE_READWRITE) == NULL)
					throw CException(errOutOfMemory);

				//	Copy over to the new block

				utlMemCopy(m_pBlock, pNewBlock, m_iCommittedSize);

				//	Free the old block

				::VirtualFree(m_pBlock, m_iCommittedSize, MEM_DECOMMIT);
				}

			//	Free original

			::VirtualFree(m_pBlock, 0, MEM_RELEASE);

			//	Flip over

			m_pBlock = pNewBlock;
			m_iMaxSize = iNewMaxSize;
			}

		//	Commit

		if (::VirtualAlloc(m_pBlock + m_iCommittedSize,
				iAdditionalSize,
				MEM_COMMIT,
				PAGE_READWRITE) == NULL)
			{
			DWORD dwError = ::GetLastError();
			throw CException(errOutOfMemory);
			}

		m_iCommittedSize += iAdditionalSize;
		}

	//	Set the length

	m_iCurrentSize = iLength;

	//	Make sure the seek position is correct

	if (GetPos() > m_iCurrentSize)
		Seek(m_iCurrentSize);
	}

