//	CHyperionCache.cpp
//
//	CHyperionCache class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool CHyperionCache::FindEntry (const CString &sID, CDatum *retdEntry) const

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

	//	Found!

	return true;
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

void CHyperionCache::SetEntry (const CString &sID, CDatum dEntry)

//	SetEntry
//
//	Sets the entry

	{
	CSmartLock Lock(m_cs);

	SEntry *pEntry = m_Cache.SetAt(sID);
	pEntry->dEntry = dEntry;
	pEntry->dwLastAccess = sysGetTickCount64();
	}
