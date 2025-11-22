//	CSegmentBlockCache.cpp
//
//	CSegmentBlockCache class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CSegmentBlockCache::~CSegmentBlockCache (void)

//	CSegmentBlockCache destructor

	{
	Term();
	}

void CSegmentBlockCache::FreeUnloadedBlocks (void)

//	FreeUnloadedBlocks
//
//	Frees as many blocks as possible to get 1 below the cache size

	{
	int i;

	while (m_Cache.GetCount() > m_iCacheSize - 1)
		{
		//	Find the least recently accessed unloaded block

		int iBestBlock = -1;
		DWORD dwBestAccess;
		for (i = 0; i < m_Cache.GetCount(); i++)
			{
			if (m_Cache[i].dwRefCount == 0)
				{
				if (iBestBlock == -1 || m_Cache[i].dwLastAccess < dwBestAccess)
					{
					dwBestAccess = m_Cache[i].dwLastAccess;
					iBestBlock = i;
					}
				}
			}

		//	If we could not find an approprate block, then we're done

		if (iBestBlock == -1)
			return;

		//	Otherwise, free the block

		delete m_Cache[iBestBlock].pBlock;
		m_Cache.Delete(iBestBlock);
		}
	}

bool CSegmentBlockCache::Init (const CString &sFilespec, int iCacheSize)

//	Init
//
//	Initialize the block cache

	{
	//	Open the segment

	if (!m_File.Create(sFilespec, CFile::FLAG_OPEN_READ_ONLY))
		return false;

	//	Set some variables

	m_iCacheSize = iCacheSize;

	//	Done

	return true;
	}

void CSegmentBlockCache::LoadBlock (DWORD dwOffset, DWORD dwBlockSize, void **retpBlock)

//	LoadBlock
//
//	Loads a block. Caller must call UnloadBlock when done.

	{
	ASSERT(m_iCacheSize != -1);
	ASSERT(dwBlockSize > 0);

	//	Look for the block in the cache. If it is not there, then we
	//	need to load it from disk.

	SEntry *pEntry = m_Cache.GetAt(dwOffset);
	if (pEntry == NULL)
		{
		//	Free a block from the cache, if necessary

		FreeUnloadedBlocks();

		//	Allocate a block of sufficient size

		void *pBlock = new char [dwBlockSize];

		//	Load from disk

		try
			{
			m_File.Seek(dwOffset);
			DWORD dwBytesRead = m_File.Read(pBlock, dwBlockSize);
			}
		catch (...)
			{
			delete pBlock;
			throw;
			}

		//	Insert the entry

		pEntry = m_Cache.Insert(dwOffset);
		pEntry->dwRefCount = 0;
		pEntry->pBlock = pBlock;
		}

	//	Return the entry

	pEntry->dwRefCount++;
	pEntry->dwLastAccess = sysGetTickCount();
	if (retpBlock)
		*retpBlock = pEntry->pBlock;
	}

void CSegmentBlockCache::Term (void)

//	Term
//
//	Free the cache and closes the file

	{
	int i;

	//	Close the file

	m_File.Close();

	//	Delete cache

	for (i = 0; i < m_Cache.GetCount(); i++)
		{
		ASSERT(m_Cache[i].dwRefCount == 0);
		delete m_Cache[i].pBlock;
		}

	m_Cache.DeleteAll();
	}

void CSegmentBlockCache::UnloadBlock (DWORD dwOffset)

//	UnloadBlock
//
//	Unloads the block

	{
	//	Get the block from the cache

	SEntry *pEntry = m_Cache.GetAt(dwOffset);
	if (pEntry)
		{
		ASSERT(pEntry->dwRefCount > 0);
		if (pEntry->dwRefCount > 0)
			pEntry->dwRefCount--;
		}
	}
