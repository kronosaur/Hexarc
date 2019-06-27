//	CMemoryBlockImpl64.cpp
//
//	CMemoryBlockImpl64 class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

char CMemoryBlockImpl64::ReadChar (void)

//	ReadChar
//
//	Reads a character. We return '\0' if we are at the end of the stream.

	{
	DWORDLONG dwLength = GetLength();
	if (m_dwPos >= dwLength)
		return '\0';

	char *pMemory = GetPointer();
	return pMemory[m_dwPos++];
	}

DWORDLONG CMemoryBlockImpl64::ReadTry (void *pData, DWORDLONG dwLength)

//	ReadTry
//
//	Reads data

	{
	DWORDLONG dwLeft = GetLength() - m_dwPos;
	dwLength = Min(dwLeft, dwLength);

	char *pMemory = GetPointer();
	if (pData)
		utlMemCopy(pMemory + m_dwPos, pData, dwLength);

	m_dwPos += dwLength;

	return dwLength;
	}

void CMemoryBlockImpl64::Seek (DWORDLONG dwPos, bool bFromEnd)

//	Seek
//
//	Sets position

	{
	DWORDLONG dwCurrentSize = GetLength();

	if (bFromEnd)
		m_dwPos = Max((DWORDLONG)0, dwCurrentSize - dwPos);
	else
		{
		//	Seeking past the end is OK

		if (dwPos > dwCurrentSize)
			{
			m_dwPos = dwCurrentSize;
			Write(NULL, dwPos - dwCurrentSize);
			dwCurrentSize = GetLength();
			}

		//	Set

		m_dwPos = Max((DWORDLONG)0, Min(dwPos, dwCurrentSize));
		}
	}

void CMemoryBlockImpl64::Write (const void *pData, DWORDLONG dwLength)

//	Write
//
//	Writes data

	{
	DWORDLONG dwCurrentSize = GetLength();

	//	Compute required size (we will throw if we can't set length)

	DWORDLONG dwRequiredSize = m_dwPos + dwLength;
	if (dwRequiredSize > dwCurrentSize)
		SetLength(dwRequiredSize);

	//	Get the pointer (after we grow)

	char *pMemory = GetPointer();

	//	Write

	if (pData)
		utlMemCopy(pData, pMemory + m_dwPos, dwLength);

	m_dwPos += dwLength;
	}
