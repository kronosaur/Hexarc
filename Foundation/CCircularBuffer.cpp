//	CCircularBuffer.cpp
//
//	CCircularBuffer class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	m_iReadPos points to the character to read next
//	m_iWritePos points to the location to write next
//
//	When m_iReadPos == m_iWritePos, the buffer is empty.
//	When (m_iWritePos + 1 % m_iAlloc) == m_iReadPos, the buffer is full.
//	Thus the buffer can hold (m_iAlloc - 1) characters.

#include "stdafx.h"

CCircularBuffer::CCircularBuffer (int iSize) : 
		m_iReadPos(0),
		m_iWritePos(0)

//	CCircularBuffer constructor

	{
	m_pBuffer = new char [iSize];
	m_iAlloc = iSize;
	}

int CCircularBuffer::GetStreamLength (void)

//	GetStreamLength
//
//	Returns the remaining length of the stream

	{
	return (m_iWritePos - m_iReadPos + m_iAlloc) % m_iAlloc;
	}

int CCircularBuffer::Read (void *pData, int iLength)

//	Read
//
//	Reads the stream

	{
	if (iLength == -1)
		iLength = GetStreamLength();
	else
		iLength = Min(iLength, GetStreamLength());

	//	If read pos is before the write pos, then we can
	//	do it all in one chunk

	if (pData)
		{
		if (m_iReadPos <= m_iWritePos)
			{
			utlMemCopy(m_pBuffer + m_iReadPos, pData, iLength);
			}

		//	Otherwise we need to read across the buffer boundary

		else
			{
			int iFirstLen = Min(iLength, m_iAlloc - m_iReadPos);
			if (iFirstLen > 0)
				utlMemCopy(m_pBuffer + m_iReadPos, pData, iFirstLen);

			int iSecondLen = Min(iLength - iFirstLen, m_iWritePos);
			if (iSecondLen > 0)
				utlMemCopy(m_pBuffer, (char *)pData + iFirstLen, iSecondLen);
			}
		}

	//	Done

	m_iReadPos = (m_iReadPos + iLength) % m_iAlloc;
	return iLength;
	}

int CCircularBuffer::Scan (const CString &sString) const

//	Scan
//
//	Scans from the read head looking for the given string. Returns the
//	number of characters to read before we reach the beginning of the string
//	(or -1 if not found).

	{
	int iPos = m_iReadPos;
	char *pSrc = sString.GetParsePointer();
	char *pSrcEnd = pSrc + sString.GetLength();

	//	Loop until we find the first character

	while (iPos != m_iWritePos)
		{
		//	If we found the first character, see if we have others

		if (*pSrc == m_pBuffer[iPos])
			{
			int iMatchPos = iPos;

			while (true)
				{
				pSrc++;
				iMatchPos = (iMatchPos + 1) % m_iAlloc;

				if (pSrc == pSrcEnd)
					return ((iPos - m_iReadPos + m_iAlloc) % m_iAlloc);
				else if (iMatchPos == m_iWritePos)
					break;
				else if (*pSrc != m_pBuffer[iMatchPos])
					break;
				}

			pSrc = sString.GetParsePointer();
			}

		//	Next

		iPos = (iPos + 1) % m_iAlloc;
		}

	//	Not found

	return -1;
	}

int CCircularBuffer::Write (void *pData, int iLength)

//	Write
//
//	Writes to the end of the stream

	{
	//	See how much room we have to write

	int iSpaceLeft = ((m_iReadPos - 1) - m_iWritePos + m_iAlloc) % m_iAlloc;

	//	See if we need to resize

	if (iSpaceLeft < iLength)
		{
		int iCurLength = GetStreamLength();
		int iNewAlloc = Max(iCurLength + iLength + 1, Min(m_iAlloc * 2, m_iAlloc + 65536));
		char *pNewBuffer = new char [iNewAlloc];

		Read(pNewBuffer, iCurLength);

		delete [] m_pBuffer;

		m_pBuffer = pNewBuffer;
		m_iAlloc = iNewAlloc;
		m_iReadPos = 0;
		m_iWritePos = iCurLength;
		}

	//	Write

	if (pData)
		{
		if (m_iWritePos < m_iReadPos)
			{
			utlMemCopy(pData, m_pBuffer + m_iWritePos, iLength);
			}
		else
			{
			int iFirstLen = Min(iLength, m_iAlloc - m_iWritePos);
			if (iFirstLen > 0)
				utlMemCopy(pData, m_pBuffer + m_iWritePos, iFirstLen);

			int iSecondLen = iLength - iFirstLen;
			if (iSecondLen > 0)
				utlMemCopy((char *)pData + iFirstLen, m_pBuffer, iSecondLen);
			}
		}

	//	Done

	m_iWritePos = (m_iWritePos + iLength) % m_iAlloc;
	return iLength;
	}
