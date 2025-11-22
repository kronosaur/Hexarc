//	Array.cpp
//
//	Array functions and classes
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CArrayBase::CArrayBase (HANDLE hHeap, int iGranularity) : m_pBlock(NULL)

//	CArrayBase constructor

	{
	if (hHeap == NULL)
		hHeap = ::GetProcessHeap();

	//	If we have anything except the default options then we need
	//	to allocate the block

	if (hHeap != ::GetProcessHeap()	|| (iGranularity != DEFAULT_ARRAY_GRANULARITY))
		{
		m_pBlock = (SHeader *)::HeapAlloc(hHeap, 0, sizeof(SHeader));
		m_pBlock->m_hHeap = hHeap;
		m_pBlock->m_iAllocSize = sizeof(SHeader);
		m_pBlock->m_iGranularity = iGranularity;
		m_pBlock->m_iSize = 0;
		}
	}

CArrayBase::~CArrayBase (void)

//	CArrayBase destructor

	{
	if (m_pBlock)
		::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);
	}

void CArrayBase::CopyOptions (const CArrayBase &Src)

//	CopyOptions
//
//	Copies heap an granularity information from source

	{
	//	If we're changing heaps then we need to reallocate

	if (GetHeap() != Src.GetHeap() 
			|| (m_pBlock == NULL && GetGranularity() != Src.GetGranularity()))
		{
		ASSERT(GetSize() == 0);

		if (m_pBlock)
			::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

		m_pBlock = (SHeader *)::HeapAlloc(Src.GetHeap(), 0, sizeof(SHeader));
		m_pBlock->m_hHeap = Src.GetHeap();
		m_pBlock->m_iAllocSize = sizeof(SHeader);
		m_pBlock->m_iGranularity = Src.GetGranularity();
		m_pBlock->m_iSize = 0;
		}

	//	Otherwise we just change the granularity

	else if (GetGranularity() != Src.GetGranularity())
		{
		ASSERT(m_pBlock);
		m_pBlock->m_iGranularity = Src.GetGranularity();
		}
	}

void CArrayBase::DeleteBytes (int iOffset, int iLength)

//	Delete
//
//	Delete iLength bytes in the array at the given offset

	{
	int i;

	if (iLength <= 0)
		return;

	ASSERT(m_pBlock);

	//	Move stuff down

	char *pSource = GetBytes() + iOffset + iLength;
	char *pDest = GetBytes() + iOffset;
	for (i = 0; i < GetSize() - (iOffset + iLength); i++)
		*pDest++ = *pSource++;

	//	Done

	m_pBlock->m_iSize -= iLength;
	}

void CArrayBase::InsertBytes (int iOffset, int iLength, int iAllocQuantum)

//	InsertBytes
//
//	Insert at the offset. NOTE: This function will double the allocation.

	{
	int i;

	if (iLength <= 0)
		return;

	if (iOffset == -1)
		iOffset = GetSize();

	ASSERT(iOffset >= 0 && iOffset <= GetSize());

    //	Reallocate if necessary

	int iNewSize = GetSize() + iLength;
	if (IsReallocNeeded(iNewSize))
		{
		//	Allocate a new block which is at least double the size of the 
		//	current block.

		int iNewAllocSize = Max(AlignUp(iNewSize, iAllocQuantum), 2 * GetSize());
		Realloc(iNewAllocSize, true);
		}

	//	Move the array up
    
	char *pSource = GetBytes() + GetSize()-1;
	char *pDest = pSource + iLength;
    for (i = GetSize()-1; i >= iOffset; i--)
		*pDest-- = *pSource--;

	//	Done
    
	m_pBlock->m_iSize += iLength;
	}

void CArrayBase::InsertBytes (int iOffset, void *pData, int iLength, int iAllocQuantum)

//	Insert
//
//	Insert the given data at the offset

	{
	int i;

	if (iLength <= 0)
		return;

	if (iOffset == -1)
		iOffset = GetSize();

	ASSERT(iOffset >= 0 && iOffset <= GetSize());

    //	Reallocate if necessary

	Resize(GetSize() + iLength, true, iAllocQuantum);
    
	//	Move the array up
    
	char *pSource = GetBytes() + GetSize()-1;
	char *pDest = pSource + iLength;
    for (i = GetSize()-1; i >= iOffset; i--)
		*pDest-- = *pSource--;

	//	Copy the new values

	if (pData)
		{
		pSource = (char *)pData;
		pDest = GetBytes() + iOffset;
		for (i = 0; i < iLength; i++)
			*pDest++ = *pSource++;
		}

	//	Done
    
	m_pBlock->m_iSize += iLength;
	}

void CArrayBase::Realloc (int iNewSize, bool bPreserve)

//	Realloc
//
//	Reallocate the block.

	{
	//	Account for the header

	iNewSize += sizeof(SHeader);

	//	Allocate a new block

	SHeader *pNewBlock = (SHeader *)::HeapAlloc(GetHeap(), 0, iNewSize);
	if (pNewBlock == NULL)
		throw CException(errOutOfMemory);

	pNewBlock->m_hHeap = GetHeap();
	pNewBlock->m_iAllocSize = iNewSize;
	pNewBlock->m_iGranularity = GetGranularity();
	pNewBlock->m_iSize = GetSize();

	//	Transfer the contents, if necessary

	if (m_pBlock && bPreserve)
		{
		char *pSource = GetBytes();
		char *pDest = (char *)(&pNewBlock[1]);
		char *pDestEnd = pDest + GetSize();

		while (pDest < pDestEnd)
			*pDest++ = *pSource++;
		}

	//	Swap blocks

	if (m_pBlock)
		::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

	m_pBlock = pNewBlock;
	}

bool CArrayBase::Resize (int iNewSize, bool bPreserve, int iAllocQuantum)

//	Resize
//
//	Resize the array so that it is at least the given new size. Returns TRUE if 
//	we had to reallocate the array.

	{
	ASSERT(iAllocQuantum > 0);

	//	See if we need to reallocate the block

	if (IsReallocNeeded(iNewSize))
		{
		//	Allocate a new block

		int iNewAllocSize = Max(AlignUp(iNewSize, iAllocQuantum), 2 * GetSize());
		Realloc(iNewAllocSize, bPreserve);

		return true;
		}

	return false;
	}

void CArrayBase::TakeHandoffBase (CArrayBase &Src)

//	TakeHandoffBase
//
//	Takes the allocated array from Src

	{
	if (m_pBlock)
		::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

	m_pBlock = Src.m_pBlock;
	Src.m_pBlock = NULL;
	}
