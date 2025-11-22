//	CHyperionCache.cpp
//
//	CHyperionCache class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CHyperionCache::FindEntry (const CString &sID, CDatum *retdEntry, CDateTime *retModifiedOn) const

//	FindEntry
//
//	Looks for the entry by ID. Return Nil if not found.

	{
	CSmartLock Lock(m_cs);

	const SEntry *pEntry = m_Cache.GetAt(sID);
	if (pEntry == NULL)
		return false;

	//	Return the entry and update access time.

	if (retdEntry)
		{
		*retdEntry = pEntry->dEntry;
		pEntry->dwLastAccess = sysGetTickCount64();
		}

	if (retModifiedOn)
		*retModifiedOn = pEntry->ModifiedOn;

	//	Found!

	return true;
	}

void CHyperionCache::FlushLRU (void)

//	FlushLRU
//
//	Flushes the least recently used entry in our cache

	{
	//	Look for the least recently used entry

	int iLRUEntry = -1;
	DWORDLONG dwLRUTime;
	for (int i = 0; i < m_Cache.GetCount(); i++)
		{
		if (iLRUEntry == -1 || m_Cache[i].dwLastAccess < dwLRUTime)
			{
			iLRUEntry = i;
			dwLRUTime = m_Cache[i].dwLastAccess;
			}
		}

	//	Remove the cache entry, if found.

	if (iLRUEntry != -1)
		{
		DecrementTotalSize(m_Cache[iLRUEntry].dwSize);
		m_Cache.Delete(iLRUEntry);
		}
	}

void CHyperionCache::Mark (void)

//	Mark
//
//	Marks all objects in use.

	{
	int i;

	for (i = 0; i < m_Cache.GetCount(); i++)
		m_Cache[i].dEntry.Mark();
	}

void CHyperionCache::SetEntry (const CString &sID, CDatum dEntry, const CDateTime &ModifiedOn)

//	SetEntry
//
//	Sets the entry

	{
	CSmartLock Lock(m_cs);

	bool bNew;
	SEntry *pEntry = m_Cache.SetAt(sID, &bNew);
	pEntry->dEntry = dEntry;
	pEntry->ModifiedOn = ModifiedOn;
	pEntry->dwLastAccess = sysGetTickCount64();

	//	If we're replacing an entry, subtract the old entry's size from the 
	//	total size of the cache.

	if (!bNew)
		DecrementTotalSize(pEntry->dwSize);

	//	Set the size.

	pEntry->dwSize = dEntry.CalcMemorySize();
	m_dwTotalSize += pEntry->dwSize;

	//	If we've exceeded our allocated size, then we flush something out of
	//	the cache.

	if (m_dwTotalSize > m_dwMaxSize)
		FlushLRU();
	}
