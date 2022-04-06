//	CAEONStore.cpp
//
//	CAEONStore class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

TAllocatorGC<double> CAEONStore::m_DoubleAlloc;
CGCStringAllocator CAEONStore::m_StringAlloc;
CGCComplexAllocator CAEONStore::m_ComplexAlloc;
TArray<MARKPROC> CAEONStore::m_MarkList;

void CAEONStore::Sweep ()

//	Sweep
//
//	Sweep unmarked objects.

	{
	//	Mark all object that we know about

	for (int i = 0; i < m_MarkList.GetCount(); i++)
		m_MarkList[i]();

	//	Sweep

	m_DoubleAlloc.Sweep();
	m_StringAlloc.Sweep();
	m_ComplexAlloc.Sweep();
	}
