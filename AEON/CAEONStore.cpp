//	CAEONStore.cpp
//
//	CAEONStore class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CGCStringAllocator CAEONStore::m_StringAlloc;
CGCComplexAllocator CAEONStore::m_ComplexAlloc;
TArray<MARKPROC> CAEONStore::m_MarkList;
CAEONTableTable CAEONStore::m_TableTable;

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
	m_ComplexAlloc.Sweep();
	}
