//	CCortex.cpp
//
//	CCortex class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#if 0
const int SEGMENT_SIZE =					32767;

CCortex::CCortex (void) : m_pFirstFree(NULL)

//	CCortex constructor

	{
	}

CCortex::~CCortex (void)

//	CCortex destructor

	{
	//	Sweep to free all allocated objects

	Sweep();

	//	Delete all segments

	int i;
	for (i = 0; i < m_Backbone.GetCount(); i++)
		delete m_Backbone[i];
	}

CDatumBlock *CCortex::Alloc (void)

//	Alloc
//
//	Allocate a new datum

	{
	//	If none free, allocate a new segment
	//	(AllocSeg will throw if it runs out of memory)

	if (m_pFirstFree == NULL)
		AllocSeg();

	//	Pull free datum off

	CDatumBlock *pNew = m_pFirstFree;
	m_pFirstFree = pNew->GetNext();
	pNew->SetNext(NULL);

	return pNew;
	}

void CCortex::AllocSeg (void)

//	AllocSeg
//
//	Allocate a new segment

	{
	CDatumBlock *pNewSeg = new CDatumBlock [SEGMENT_SIZE];

	//	Initilize them all (they are initialized as free
	//	and we link them all together)

	CDatumBlock *pNew = pNewSeg;
	CDatumBlock *pEnd = pNewSeg + (SEGMENT_SIZE - 1);
	while (pNew < pEnd)
		{
		pNew->SetNext(pNew + 1);
		pNew++;
		}

	//	Add to backbone

	m_Backbone.Insert(pNewSeg);

	//	Add to free list

	ASSERT(m_pFirstFree == NULL);
	m_pFirstFree = pNewSeg;
	}

void CCortex::ClearMarks (void)

//	ClearMarks
//
//	Clear all marks on all datums

	{
	}

void CCortex::Sweep (void)

//	Sweep
//
//	Sweep away any unmarked (unreferenced) datums.
//	The user must manually mark all datums that are in use before
//	calling this function (otherwise, all data will be freed)
//
//	After this call, all datums will be unmarked.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	First we loop over every unmarked datum and
	//	mark it for delete. We need to do this to deal
	//	with multiple datums pointing to a single underlying object

	for (i = 0; i < m_Backbone.GetCount(); i++)
		{
		CDatumBlock *pDatum = m_Backbone[i];
		CDatumBlock *pEnd = pDatum + SEGMENT_SIZE;
		while (pDatum < pEnd)
			{
			if (!pDatum->IsFree() && !pDatum->IsMarked())
				pDatum->MarkForDelete();

			pDatum++;
			}
		}

	//	Now we delete the marked datums and any associated
	//	objects. (We also clear the marks from all other datums)

	for (i = 0; i < m_Backbone.GetCount(); i++)
		{
		CDatumBlock *pDatum = m_Backbone[i];
		CDatumBlock *pEnd = pDatum + SEGMENT_SIZE;
		while (pDatum < pEnd)
			{
			if (pDatum->IsMarked())
				pDatum->ClearMark();
			else if (!pDatum->IsFree())
				{
				pDatum->SetFree();
				pDatum->SetNext(m_pFirstFree);
				m_pFirstFree = pDatum;
				}

			pDatum++;
			}
		}
	}

#endif