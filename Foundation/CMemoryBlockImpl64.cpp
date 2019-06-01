//	CMemoryBlockImpl64.cpp
//
//	CMemoryBlockImpl64 class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

void CMemoryBlockImpl64::Read (void *pData, DWORDLONG dwLength)

//	Read
//
//	Reads data

	{
	DWORDLONG dwCurrentSize = GetLength();
	if (dwLength < dwCurrentSize - m_dwPos)
		throw CException(errFail);

	char *pMemory = GetPointer();

	if (pData)
		utlMemCopy(pMemory + m_dwPos, pData, dwLength);

	m_dwPos += dwLength;
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
