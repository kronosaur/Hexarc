//	CAEONStore.cpp
//
//	CAEONStore class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CGCStringAllocator CAEONStore::m_StringAlloc;
CGCComplexAllocator CAEONStore::m_ComplexAlloc;
TGCSlabAllocator<CAEONRecord> CAEONStore::m_RecordAlloc;
TArray<MARKPROC> CAEONStore::m_MarkList;
CAEONTableTable CAEONStore::m_TableTable;

CDatum CAEONStore::CreateRecord (CDatum dType)

//	CreateRecord
//
//	Creates a schema record from a slab-backed GC store.

	{
	CAEONRecord* pRecord = m_RecordAlloc.New(dType);
	return CDatum::raw_AsComplex(pRecord);
	}

CDatum CAEONStore::CreateRecord (CDatum dType, const CDatum* pValues, int iCount)

//	CreateRecord
//
//	Creates a schema record initialized from contiguous slot values.

	{
	CAEONRecord* pRecord = m_RecordAlloc.New(dType, pValues, iCount);
	return CDatum::raw_AsComplex(pRecord);
	}

void CAEONStore::Sweep ()

//	Sweep
//
//	Sweep unmarked objects.

	{
	//	Mark all object that we know about

	for (int i = 0; i < m_MarkList.GetCount(); i++)
		m_MarkList[i]();

#ifdef DEBUG_GC_STATS
	m_ComplexAlloc.DumpStats();
#endif

	//	Sweep

	m_StringAlloc.Sweep();
	m_RecordAlloc.Sweep();
	m_ComplexAlloc.Sweep();
	}
