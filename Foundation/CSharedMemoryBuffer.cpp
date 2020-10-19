//	CMemoryBuffer.cpp
//
//	CMemoryBuffer class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

constexpr int DEFAULT_MAX_SIZE =				1024 * 1024;

CSharedMemoryBuffer::CSharedMemoryBuffer (CSharedMemoryBuffer &&Src) noexcept

//	CSharedMemoryBuffer move constructor

	{
	m_hMapFile = Src.m_hMapFile;
	m_iMaxSize = Src.m_iMaxSize;
	m_pBlock = Src.m_pBlock;

	Src.m_hMapFile = INVALID_HANDLE_VALUE;
	Src.m_iMaxSize = 0;
	Src.m_pBlock = NULL;
	}

CSharedMemoryBuffer::~CSharedMemoryBuffer (void)

//	CSharedMemoryBuffer destructor

	{
	Close();
	}

CSharedMemoryBuffer &CSharedMemoryBuffer::operator= (CSharedMemoryBuffer &&Src) noexcept

//	CSharedMemoryBuffer move operator

	{
	//	If assigning to itself, then no change.

	if (m_pBlock == Src.m_pBlock)
		return *this;
		
	Close();

	m_hMapFile = Src.m_hMapFile;
	m_iMaxSize = Src.m_iMaxSize;
	m_pBlock = Src.m_pBlock;

	Src.m_hMapFile = INVALID_HANDLE_VALUE;
	Src.m_iMaxSize = 0;
	Src.m_pBlock = NULL;

	return *this;
	}

void CSharedMemoryBuffer::Close (void)

//	Close
//
//	Explicitly close the buffer

	{
	if (m_pBlock)
		{
		::UnmapViewOfFile(m_pBlock);
		m_pBlock = NULL;
		}

	if (m_hMapFile != INVALID_HANDLE_VALUE)
		{
		::CloseHandle(m_hMapFile);
		m_hMapFile = INVALID_HANDLE_VALUE;
		}
	}

void CSharedMemoryBuffer::Create (LPSTR sName, int iMaxSize, bool *retbExists)

//	Create
//
//	Create the shared memory object

	{
	ASSERT(m_hMapFile == INVALID_HANDLE_VALUE);
	ASSERT(iMaxSize >= 0);

	//	Create a mapped file

	m_iMaxSize = (iMaxSize == 0 ? DEFAULT_MAX_SIZE : iMaxSize);
	m_hMapFile = ::CreateFileMapping(
			INVALID_HANDLE_VALUE,				//	Use paging file
			NULL,								//	Default security
			PAGE_READWRITE,						//	Read-write access
			0,									//	Max size (high DWORD)
			m_iMaxSize,							//	Max size (low DWORD)
			(sName ? CString16(sName) : NULL));	//	Name
	if (m_hMapFile == NULL)
		throw CException(errFail);

	//	See if the shared object already exists

	if (retbExists)
		*retbExists = (::GetLastError() == ERROR_ALREADY_EXISTS);

	//	Map a view of it

	m_pBlock = (char *)::MapViewOfFile(m_hMapFile,
			FILE_MAP_ALL_ACCESS,				//	Read-write access
			0,									//	Offset (high DWORD)
			0,									//	Offset (low DWORD)
			m_iMaxSize);						//	Bytes to map
	if (m_pBlock == NULL)
		{
		::CloseHandle(m_hMapFile);
		m_hMapFile = INVALID_HANDLE_VALUE;
		throw CException(errOutOfMemory);
		}
	}

void CSharedMemoryBuffer::SetMaxSize (int iMaxSize)

//	SetMaxSize
//
//	Set the maximum size of the buffer. We use this when we open an
//	existing file mapping.

	{
	ASSERT(m_hMapFile);
	ASSERT(m_pBlock);

	char *pNewBlock = (char *)::MapViewOfFile(m_hMapFile,
			FILE_MAP_ALL_ACCESS,				//	Read-write access
			0,									//	Offset (high DWORD)
			0,									//	Offset (low DWORD)
			iMaxSize);							//	Bytes to map
	if (pNewBlock == NULL)
		throw CException(errOutOfMemory);

	//	Clean up the previous view

	::UnmapViewOfFile(m_pBlock);

	//	Replace

	m_pBlock = pNewBlock;
	m_iMaxSize = iMaxSize;
	}
