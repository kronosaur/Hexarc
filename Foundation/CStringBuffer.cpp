//	CStringBuffer.cpp
//
//	CStringBuffer class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

static constexpr int MIN_ALLOC_INCREMENT =			4096;
static constexpr int MAX_ALLOC_INCREMENT =			100'000'000;

CStringBuffer::CStringBuffer (CString &&Src) noexcept

//	CStringBuffer move constructor

	{
	if (!Src.IsEmpty())
		{
		m_iAlloc = Src.GetLength() + sizeof(int);
		m_pString = Src.Handoff();
		}
	}

CStringBuffer::CStringBuffer (int iSize)

//	CStringBuffer constructor
//
//	NOTE: Unlike CBuffer, we set the size of the string to the given size.

	{
	if (iSize < 0)
		throw CException(errFail);
	else if (iSize == 0)
		{
		m_pString = NULL;
		m_iAlloc = 0;
		}
	else
		{
		char *pNewBuffer = new char [iSize + sizeof(int) + 1];
		char *pNewString = pNewBuffer + sizeof(int);

		m_pString = pNewString;
		m_iAlloc = iSize;

		//	Null-terminate

		m_pString[iSize] = '\0';

		//	Done

		SetLengthParameter(iSize);
		}
	}

void CStringBuffer::CleanUp ()

//	CleanUp
//
//	Free everything

	{
	if (m_iAlloc > 0)
		{
		delete [] GetBuffer();
		m_pString = NULL;
		m_iAlloc = 0;
		Seek(0);
		}
	}

LPSTR CStringBuffer::Handoff (void)

//	Handoff
//
//	Returns the allocated buffer. The caller is responsible for freeing it

	{
	if (GetLength() == 0)
		return NULL;

	else if (m_iAlloc == 0)
		SetLength(GetLength());

	char *pString = m_pString;
	m_pString = NULL;
	m_iAlloc = 0;
	Seek(0);

	return pString;
	}

void CStringBuffer::Move (CStringBuffer &Src)

//	Move
//
//	Moves buffer; we assume that the buffer is pristine.

	{
	m_pString = Src.m_pString;
	m_iAlloc = Src.m_iAlloc;

	Src.m_pString = NULL;
	Src.m_iAlloc = 0;
	Src.Seek(0);
	}

void CStringBuffer::SetLength (int iLength)

//	SetLength
//
//	Sets the length of the string

	{
	//	Edge-condition

	if (iLength == 0)
		{
		if (m_iAlloc > 0)
			delete [] GetBuffer();

		m_pString = NULL;
		m_iAlloc = 0;
		Seek(0);
		return;
		}

	//	Resize

	if (iLength > m_iAlloc)
		{
		//	Allocate in chunks

		int iInc = Max(MIN_ALLOC_INCREMENT, Min(m_iAlloc, MAX_ALLOC_INCREMENT));
		int iNewAlloc = Max(iLength, m_iAlloc + iInc);

		//	Allocate the buffer

		char *pNewBuffer = new char [iNewAlloc + sizeof(int) + 1];
		char *pNewString = pNewBuffer + sizeof(int);

		//	If necessary, copy the old buffer

		if (GetLength() > 0)
			utlMemCopy(GetPointer(), pNewString, Min(GetLength(), iLength));

		//	Free the previous buffer

		if (m_iAlloc > 0)
			delete [] GetBuffer();

		//	Done

		m_pString = pNewString;
		m_iAlloc = iNewAlloc;
		}

	//	Null-terminate

	m_pString[iLength] = '\0';

	//	Done

	SetLengthParameter(iLength);

	//	Make sure the seek position is correct

	if (GetPos() > iLength)
		Seek(iLength);
	}
