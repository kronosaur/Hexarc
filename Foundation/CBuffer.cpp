//	CBuffer.cpp
//
//	CBuffer class
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#define MIN_ALLOC_INCREMENT							4096
#define MAX_ALLOC_INCREMENT							65536

CBuffer::CBuffer (int iSize)

//	CBuffer constructor

	{
	m_pBuffer = new char [iSize];
	m_iAllocation = iSize;
	m_iLength = 0;
	m_bAllocated = true;
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

CBuffer::CBuffer (CBuffer &&Src) noexcept

//	CBuffer move constructor

	{
	m_iLength = Src.m_iLength;
	m_iAllocation = Src.m_iAllocation;
	m_bAllocated = Src.m_bAllocated;

	m_pBuffer = Src.m_pBuffer;

	Src.m_pBuffer = NULL;
	Src.m_bAllocated = false;
	Src.m_iAllocation = 0;
	Src.m_iLength = 0;
	Src.Seek(0);
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

CBuffer& CBuffer::operator= (CBuffer &&Src) noexcept

//	CBuffer move operator =

	{
	if (m_bAllocated)
		delete [] m_pBuffer;

	m_iLength = Src.m_iLength;
	m_iAllocation = Src.m_iAllocation;
	m_bAllocated = Src.m_bAllocated;
	m_pBuffer = Src.m_pBuffer;

	Src.m_pBuffer = NULL;
	Src.m_bAllocated = false;
	Src.m_iAllocation = 0;
	Src.m_iLength = 0;
	Src.Seek(0);

	Seek(0);
	return *this;
	}

CBuffer CBuffer::AsUTF8 (const IMemoryBlock& Stream)

//	AsUTF8
//
//	If this buffer is a UTF-16 buffer, then we convert to UTF-8. Otherwise, we 
//	return an empty buffer.

	{
	switch (Stream.ReadBOM())
		{
		case IByteStream::EBOM::UTF16LE:
			{
			//	Skip the BOM

			const char* pStart = Stream.GetPointer() + 2;
			int iLength = (Stream.GetLength() - 2) / sizeof(WORD);

			CBuffer UTF8;

			//	We optimistically assume that the UTF-8 buffer has 1 byte per 
			//	character.

			UTF8.m_iAllocation = iLength;
			UTF8.m_iLength = UTF8.m_iAllocation;
			UTF8.m_pBuffer = new char [UTF8.m_iAllocation];
			UTF8.m_bAllocated = true;

			int iResult = ::WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)pStart, iLength, UTF8.m_pBuffer, UTF8.m_iAllocation, NULL, NULL);

			//	Deal with failure

			if (iResult == 0)
				{
				if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
					//	Figure out how big the buffer should be and allocate appropriately
					//	And redo the conversion.

					int iNewBufferLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)pStart, iLength, NULL, 0, NULL, NULL);
					if (iNewBufferLen == 0)
						return CBuffer();

					delete [] UTF8.m_pBuffer;
					UTF8.m_iAllocation = iNewBufferLen;
					UTF8.m_iLength = UTF8.m_iAllocation;
					UTF8.m_pBuffer = new char [iNewBufferLen];
					iResult = ::WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)pStart, iLength, UTF8.m_pBuffer, UTF8.m_iAllocation, NULL, NULL);
					}
				else
					{
					return CBuffer();
					}
				}

			return UTF8;
			}
		
		default:
			return CBuffer();
		}
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

void CBuffer::Init (const void *pBuffer, size_t dwLength, bool bCopy)

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
		m_iLength = (int)dwLength;
		m_pBuffer = new char [dwLength];
		m_iAllocation = (int)dwLength;
		utlMemCopy(pBuffer, m_pBuffer, dwLength);
		}
	else
		{
		m_pBuffer = (char *)pBuffer;
		m_iAllocation = 0;
		m_iLength = (int)dwLength;
		m_bAllocated = false;
		}
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
