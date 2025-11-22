//	CMemoryBlockImpl.cpp
//
//	CMemoryBlockImpl class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CMemoryBlockImpl::GrowToFit (int iLength)
	{
	int iOldPos = GetPos();
	int iOldLength = GetLength();
	Write(NULL, iLength);
	SetLength(iOldLength);
	Seek(iOldPos);
	}

int CMemoryBlockImpl::Read (void *pData, int iLength)

//	Read
//
//	Reads data

	{
	ASSERT(iLength >= 0);
	int iCurrentSize = GetLength();
	char *pMemory = GetPointer();

	iLength = Min(iLength, iCurrentSize - m_iPos);
	if (pData)
		utlMemCopy(pMemory + m_iPos, pData, iLength);
	m_iPos += iLength;

	return iLength;
	}

void CMemoryBlockImpl::Seek (int iPos, bool bFromEnd)

//	Seek
//
//	Sets position

	{
	int iCurrentSize = GetLength();

	if (bFromEnd)
		m_iPos = Max(0, iCurrentSize - iPos);
	else
		{
		//	Seeking past the end is OK

		if (iPos > iCurrentSize)
			{
			m_iPos = iCurrentSize;
			Write(NULL, iPos - iCurrentSize);
			iCurrentSize = GetLength();
			}

		//	Set

		m_iPos = Max(0, Min(iPos, iCurrentSize));
		}
	}

int CMemoryBlockImpl::Write (const void *pData, int iLength)

//	Write
//
//	Writes data

	{
	ASSERT(iLength >= 0);
	int iCurrentSize = GetLength();

	//	Compute required size

	int iRequiredSize = m_iPos + iLength;
	if (iRequiredSize > iCurrentSize)
		{
		SetLength(iRequiredSize);

		//	No guarantee that we were able to grow
		iCurrentSize = GetLength();
		}

	//	Get the pointer (after we grow)

	char *pMemory = GetPointer();

	//	Write

	iLength = Min(iLength, iCurrentSize - m_iPos);
	if (pData)
		utlMemCopy(pData, pMemory + m_iPos, iLength);
	m_iPos += iLength;

	return iLength;
	}

int CMemoryBlockImpl::WriteStream (IByteStream& Stream, int iLength)

//	WriteStream
//
//	Write the stream.

	{
	ASSERT(iLength >= 0);
	int iCurrentSize = GetLength();

	//	Compute required size

	int iRequiredSize = m_iPos + iLength;
	if (iRequiredSize > iCurrentSize)
		{
		SetLength(iRequiredSize);

		//	No guarantee that we were able to grow

		iCurrentSize = GetLength();
		iLength = Min(iLength, iCurrentSize - m_iPos);
		}

	//	Get the pointer (after we grow)

	char *pMemory = GetPointer();

	//	Write to the newly allocated area

	Stream.Read(pMemory + m_iPos, iLength);
	m_iPos += iLength;

	return iLength;
	}
