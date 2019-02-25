//	CBuffer.cpp
//
//	CBuffer class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define MIN_ALLOC_INCREMENT							4096
#define MAX_ALLOC_INCREMENT							65536

CBuffer::CBuffer (void) :
		m_pBuffer(NULL),
		m_iLength(0),
		m_iAllocation(0),
		m_bAllocated(false)

//	CBuffer constructor

	{
	}

CBuffer::CBuffer (int iSize)

//	CBuffer constructor

	{
	m_pBuffer = new char [iSize];
	m_iAllocation = iSize;
	m_iLength = 0;
	m_bAllocated = true;
	}

CBuffer::CBuffer (LPVOID pBuffer, int iLength, bool bCopy)

//	CBuffer constructor

	{
	if (bCopy)
		{
		m_bAllocated = true;
		m_iLength = iLength;
		m_pBuffer = new char [iLength];
		m_iAllocation = iLength;
		utlMemCopy(pBuffer, m_pBuffer, iLength);
		}
	else
		{
		m_pBuffer = (char *)pBuffer;
		m_iAllocation = 0;
		m_iLength = iLength;
		m_bAllocated = false;
		}
	}

CBuffer::CBuffer (const CString &sString, int iPos, int iLength)

//	CBuffer constructor

	{
	m_pBuffer = (LPSTR)sString + iPos;
	if (iLength == -1)
		m_iLength = sString.GetLength() - iPos;
	else
		m_iLength = iLength;
	m_bAllocated = false;
	m_iAllocation = 0;
	}

CBuffer::CBuffer (const CBuffer &Src)

//	CBuffer constructor

	{
	Copy(Src);
	}

CBuffer::~CBuffer (void)

//	CBuffer destructor

	{
	if (m_bAllocated)
		delete [] m_pBuffer;
	}

CBuffer &CBuffer::operator= (const CBuffer &Src)

//	CBuffer operator =

	{
	if (m_bAllocated)
		delete [] m_pBuffer;

	Copy(Src);

	return *this;
	}

void CBuffer::Copy (const CBuffer &Src)

//	Copy
//
//	Copy the buffer (we assume that we have freed our allocation)

	{
	m_iLength = Src.m_iLength;

	if (Src.m_bAllocated)
		{
		m_bAllocated = true;
		m_pBuffer = new char [m_iLength];
		m_iAllocation = m_iLength;
		utlMemCopy(Src.m_pBuffer, m_pBuffer, m_iLength);
		}
	else
		{
		m_bAllocated = false;
		m_pBuffer = Src.m_pBuffer;
		m_iAllocation = 0;
		}
	}

LPVOID CBuffer::GetHandoff (int *retiAllocation)

//	GetHandoff
//
//	Returns the allocated buffer (or a copy, if not allocated)

	{
	LPVOID pAlloc;

	if (m_bAllocated)
		{
		if (retiAllocation)
			*retiAllocation = m_iAllocation;

		pAlloc = m_pBuffer;
		m_pBuffer = NULL;
		m_bAllocated = false;
		m_iLength = 0;
		m_iAllocation = 0;

		Seek(0);
		}
	else
		{
		if (retiAllocation)
			*retiAllocation = m_iLength;

		pAlloc = new char [m_iLength];
		utlMemCopy(m_pBuffer, pAlloc, m_iLength);
		}

	return pAlloc;
	}

void CBuffer::SetLength (int iLength)

//	SetLength
//
//	Sets the buffer size

	{
	//	m_iAllocation is 0 if we've got a static buffer

	int iAvail = Max(m_iAllocation, m_iLength);

	//	Grow, if necessary

	if (iLength > iAvail)
		{
		int iInc = Max(MIN_ALLOC_INCREMENT, min(MAX_ALLOC_INCREMENT, iAvail));
		int iNewAlloc = AlignUp(iLength, iInc);

		LPVOID pNewBuffer = new char [iNewAlloc];
		utlMemCopy(m_pBuffer, pNewBuffer, m_iLength);

		if (m_bAllocated)
			delete [] m_pBuffer;

		m_pBuffer = (char *)pNewBuffer;
		m_iAllocation = iNewAlloc;
		m_bAllocated = true;
		}

	//	Set the length

	m_iLength = iLength;

	//	Make sure the seek position is correct

	if (GetPos() > m_iLength)
		Seek(m_iLength);
	}

void CBuffer::TakeHandoff (CBuffer &Src)

//	TakeHandoff
//
//	Takes ownership of the given buffer's allocation. If the source is not allocated
//	then we make a copy.

	{
	if (m_bAllocated)
		delete [] m_pBuffer;

	m_bAllocated = true;
	m_iLength = Src.m_iLength;
	m_pBuffer = (char *)Src.GetHandoff(&m_iAllocation);

	Seek(0);
	}

void CBuffer::TakeHandoff (void *pBuffer, int iAllocLength)

//	TakeHandoff
//
//	Takes ownership of the given allocation.

	{
	if (m_bAllocated)
		delete [] m_pBuffer;

	m_bAllocated = true;
	m_iLength = iAllocLength;
	m_iAllocation = iAllocLength;
	m_pBuffer = (char *)pBuffer;

	Seek(0);
	}
