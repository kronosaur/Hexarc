//	CBuffer6464.cpp
//
//	CBuffer6464 class
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CBuffer64::CBuffer64 (int iSize)

//	CBuffer64 constructor

	{
	m_pBuffer = new char [iSize];
	m_dwAllocation = iSize;
	m_dwLength = 0;
	m_bAllocated = true;
	}

CBuffer64::CBuffer64 (const CString &sString, int iPos, int iLength)

//	CBuffer64 constructor

	{
	m_pBuffer = (LPSTR)sString + iPos;
	if (iLength == -1)
		m_dwLength = (DWORDLONG)sString.GetLength() - (DWORDLONG)iPos;
	else
		m_dwLength = iLength;
	m_bAllocated = false;
	m_dwAllocation = 0;
	}

CBuffer64::CBuffer64 (const CBuffer64 &Src)

//	CBuffer64 constructor

	{
	Copy(Src);
	}

CBuffer64::CBuffer64 (CBuffer64 &&Src) noexcept

//	CBuffer64 move constructor

	{
	m_dwLength = Src.m_dwLength;
	m_dwAllocation = Src.m_dwAllocation;
	m_bAllocated = Src.m_bAllocated;

	m_pBuffer = Src.m_pBuffer;

	Src.m_pBuffer = NULL;
	Src.m_bAllocated = false;
	}

CBuffer64::~CBuffer64 (void)

//	CBuffer64 destructor

	{
	if (m_bAllocated)
		delete [] m_pBuffer;
	}

CBuffer64 &CBuffer64::operator= (const CBuffer64 &Src)

//	CBuffer64 operator =

	{
	if (m_bAllocated)
		delete [] m_pBuffer;

	Copy(Src);

	return *this;
	}

CBuffer64 CBuffer64::AsUTF8 (const IMemoryBlock64& Stream)

//	AsUTF8
//
//	If this buffer is a UTF-16 buffer, then we convert to UTF-8. Otherwise, we 
//	return an empty buffer.

	{
	switch (Stream.ReadBOM())
		{
		case IByteStream64::EBOM::UTF16LE:
			{
			//	Skip the BOM

			const char* pStart = Stream.GetPointer() + 2;
			DWORDLONG dwLength = (Stream.GetLength() - 2) / sizeof(WORD);

			CBuffer64 UTF8;

			//	We optimistically assume that the UTF-8 buffer has 1 byte per 
			//	character.

			UTF8.m_dwAllocation = dwLength;
			UTF8.m_dwLength = UTF8.m_dwAllocation;
			UTF8.m_pBuffer = new char [UTF8.m_dwAllocation];
			UTF8.m_bAllocated = true;

			int iResult = ::WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)pStart, (int)dwLength, UTF8.m_pBuffer, (int)UTF8.m_dwAllocation, NULL, NULL);

			//	Deal with failure

			if (iResult == 0)
				{
				if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
					//	Figure out how big the buffer should be and allocate appropriately
					//	And redo the conversion.

					int iNewBufferLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)pStart, (int)dwLength, NULL, 0, NULL, NULL);
					if (iNewBufferLen == 0)
						return CBuffer64();

					delete [] UTF8.m_pBuffer;
					UTF8.m_dwAllocation = iNewBufferLen;
					UTF8.m_dwLength = UTF8.m_dwAllocation;
					UTF8.m_pBuffer = new char [iNewBufferLen];
					iResult = ::WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)pStart, (int)dwLength, UTF8.m_pBuffer, (int)UTF8.m_dwAllocation, NULL, NULL);
					}
				else
					{
					return CBuffer64();
					}
				}

			return UTF8;
			}
		
		default:
			return CBuffer64();
		}
	}

void CBuffer64::Copy (const CBuffer64 &Src)

//	Copy
//
//	Copy the buffer (we assume that we have freed our allocation)

	{
	m_dwLength = Src.m_dwLength;

	if (Src.m_bAllocated)
		{
		m_bAllocated = true;
		m_pBuffer = new char [m_dwLength];
		m_dwAllocation = m_dwLength;
		utlMemCopy(Src.m_pBuffer, m_pBuffer, m_dwLength);
		}
	else
		{
		m_bAllocated = false;
		m_pBuffer = Src.m_pBuffer;
		m_dwAllocation = 0;
		}
	}

LPVOID CBuffer64::GetHandoff (DWORDLONG *retdwAllocation)

//	GetHandoff
//
//	Returns the allocated buffer (or a copy, if not allocated)

	{
	LPVOID pAlloc;

	if (m_bAllocated)
		{
		if (retdwAllocation)
			*retdwAllocation = m_dwAllocation;

		pAlloc = m_pBuffer;
		m_pBuffer = NULL;
		m_bAllocated = false;
		m_dwLength = 0;
		m_dwAllocation = 0;

		Seek(0);
		}
	else
		{
		if (retdwAllocation)
			*retdwAllocation = m_dwLength;

		pAlloc = new char [m_dwLength];
		utlMemCopy(m_pBuffer, pAlloc, m_dwLength);
		}

	return pAlloc;
	}

void CBuffer64::GrowToFit (DWORDLONG dwLength)

//	GrowToFit
//
//	Add enough to append the given length

	{
	//	m_dwAllocation is 0 if we've got a static buffer

	DWORDLONG dwAvail = Max(m_dwAllocation, m_dwLength);
	DWORDLONG dwNewLength = m_dwLength + dwLength;

	//	Grow, if necessary

	if (dwNewLength > dwAvail)
		{
		DWORDLONG dwInc = Max(MIN_ALLOC_INCREMENT, Min(MAX_ALLOC_INCREMENT, dwAvail));
		DWORDLONG dwNewAlloc = AlignUp(dwNewLength, dwInc);

		LPVOID pNewBuffer = new char[dwNewAlloc];
		utlMemCopy(m_pBuffer, pNewBuffer, m_dwLength);

		if (m_bAllocated)
			delete[] m_pBuffer;

		m_pBuffer = (char*)pNewBuffer;
		m_dwAllocation = dwNewAlloc;
		m_bAllocated = true;
		}
	}

void CBuffer64::Init (const void *pBuffer, size_t dwLength, bool bCopy)

//	Init
//
//	Initialize from a buffer. NOTE: We assume all member variables are 
//	uninitialized.

	{
	if (dwLength > INT_MAX)
		throw CException(errOutOfMemory);

	if (bCopy)
		{
		m_bAllocated = true;
		m_dwLength = (int)dwLength;
		m_pBuffer = new char [dwLength];
		m_dwAllocation = (int)dwLength;
		utlMemCopy(pBuffer, m_pBuffer, dwLength);
		}
	else
		{
		m_pBuffer = (char *)pBuffer;
		m_dwAllocation = 0;
		m_dwLength = (int)dwLength;
		m_bAllocated = false;
		}
	}

void CBuffer64::SetLength (DWORDLONG dwLength)

//	SetLength
//
//	Sets the buffer size

	{
	//	m_dwAllocation is 0 if we've got a static buffer

	DWORDLONG dwAvail = Max(m_dwAllocation, m_dwLength);

	//	Grow, if necessary

	if (dwLength > dwAvail)
		{
		DWORDLONG dwInc = Max(MIN_ALLOC_INCREMENT, Min(MAX_ALLOC_INCREMENT, dwAvail));
		DWORDLONG dwNewAlloc = AlignUp(dwLength, dwInc);

		LPVOID pNewBuffer = new char [dwNewAlloc];
		utlMemCopy(m_pBuffer, pNewBuffer, m_dwLength);

		if (m_bAllocated)
			delete [] m_pBuffer;

		m_pBuffer = (char *)pNewBuffer;
		m_dwAllocation = dwNewAlloc;
		m_bAllocated = true;
		}

	//	Set the length

	m_dwLength = dwLength;

	//	Make sure the seek position is correct

	if (GetPos() > m_dwLength)
		Seek(m_dwLength);
	}

void CBuffer64::TakeHandoff (CBuffer64 &Src)

//	TakeHandoff
//
//	Takes ownership of the given buffer's allocation. If the source is not allocated
//	then we make a copy.

	{
	if (m_bAllocated)
		delete [] m_pBuffer;

	m_bAllocated = true;
	m_dwLength = Src.m_dwLength;
	m_pBuffer = (char *)Src.GetHandoff(&m_dwAllocation);

	Seek(0);
	}

void CBuffer64::TakeHandoff (void *pBuffer, DWORDLONG dwAllocLength)

//	TakeHandoff
//
//	Takes ownership of the given allocation.

	{
	if (m_bAllocated)
		delete [] m_pBuffer;

	m_bAllocated = true;
	m_dwLength = dwAllocLength;
	m_dwAllocation = dwAllocLength;
	m_pBuffer = (char *)pBuffer;

	Seek(0);
	}
