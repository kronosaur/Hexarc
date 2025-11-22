//	CZipFormat.cpp
//
//	CZipeFormat class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#define ZLIB_WINAPI
#include "../zlib-1.2.7/zlib.h"
#include "../zlib-1.2.7/unzip.h"

DECLARE_CONST_STRING(ERR_FILE_NOT_FOUND,				"File not found: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN_COMPRESSION,			"Unknown compression method: %d.");
DECLARE_CONST_STRING(ERR_ZIP_FILE_CORRUPT,				"Zip file corrupted.");
DECLARE_CONST_STRING(ERR_ZIP_FILE_ENCRYPTED,			"Zip file encrypted.");

CZipFormatReader::CZipFormatReader (const IMemoryBlock& Data)

//	CZipFormat constructor

	{
	Init(Data);
	}

bool CZipFormatReader::FindFileByName (const CString& sFilename, int* retiIndex) const

//	FindFileByName
//
//	Looks for the file by full path name (case-insensitive).

	{
	for (int i = 0; i < m_Directory.GetCount(); i++)
		{
		if (strEqualsNoCase(sFilename, m_Directory[i].sFilename))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

bool CZipFormatReader::Init (const IMemoryBlock& Data)

//	Init
//
//	Initialize from a memory block. The memory block must remain valid for the
//	lifetime of this object. Returns FALSE if the data is not a valid Zip file 
//	format.
//
//	See: https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html

	{
	TArray<SEntry> Directory;

	const char* pStart = Data.GetPointer();
	const char* pEnd = pStart + Data.GetLength();
	if (Data.GetLength() < sizeof(DWORD))
		return false;

	//	Start by scanning for the end of central directory signature.

	const char* pPos = pEnd - sizeof(DWORD);
	while (pPos > pStart)
		{
		DWORD* pTest = (DWORD*)pPos;
		if (*pTest == CZipFormat::SIG_END_OF_CENTRAL_DIRECTORY)
			break;

		pPos -= 1;
		}

	if (pPos == pStart)
		return false;

	if ((pEnd - pPos) < sizeof(CZipFormat::SEndOfCentralDirectory))
		return false;

	CZipFormat::SEndOfCentralDirectory* pEndOfCentralDirectory = (CZipFormat::SEndOfCentralDirectory*)pPos;

	//	Point to the first entry.

	pPos = pStart + pEndOfCentralDirectory->dwCentralDirectoryOffset;

	for (int i = 0; i < (int)pEndOfCentralDirectory->wTotalEntries; i++)
		{
		if (pPos > pEnd || (pEnd - pPos) < sizeof(CZipFormat::SCentralDirectoryFileHeader))
			return false;

		CZipFormat::SCentralDirectoryFileHeader* pEntry = (CZipFormat::SCentralDirectoryFileHeader*)pPos;
		if (!ReadDirectoryFileHeader(Directory, pEntry, pEnd))
			return false;

		pPos += sizeof(CZipFormat::SCentralDirectoryFileHeader) + pEntry->wFilenameLen + pEntry->wExtraLen + pEntry->wCommentLen;
		}

	//	Success!

	m_pData = &Data;
	m_Directory = std::move(Directory);

	return true;
	}

bool CZipFormatReader::ReadFile (int iIndex, CBuffer64& retBuffer, CString* retsError) const

//	ReadFile
//
//	Uncompresses the given file and writes it to the given stream.
	{
	if (iIndex < 0 || iIndex >= m_Directory.GetCount())
		throw CException(errFail);

	retBuffer.SetLength(0);
	retBuffer.GrowToFit(m_Directory[iIndex].dwUncompressedSize);

	try
		{
		if (!ReadFileAtOffset(m_Directory[iIndex].dwLocalHeaderOffset, retBuffer, m_Directory[iIndex].dwCompressedSize, retsError))
			return false;
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_ZIP_FILE_CORRUPT;
			return false;
		}

	return true;
	}

bool CZipFormatReader::ReadFile (const CString& sFilename, CBuffer64& retBuffer, CString* retsError) const

//	ReadFile
//
//	Uncompresses the given file and writes it to the given stream.

	{
	int iIndex;
	if (!FindFileByName(sFilename, &iIndex))
		{
		if (retsError) *retsError = strPattern(ERR_FILE_NOT_FOUND, sFilename);
		return false;
		}

	return ReadFile(iIndex, retBuffer, retsError);
	}

//	Internal Methods -----------------------------------------------------------


bool CZipFormatReader::ReadDirectoryFileHeader (TArray<SEntry>& Directory, CZipFormat::SCentralDirectoryFileHeader* pHeader, const char* pEnd)
	{
	SEntry NewEntry;
	NewEntry.dwCompressedSize = pHeader->dwCompressedSize;
	NewEntry.dwUncompressedSize = pHeader->dwUncompressedSize;
	NewEntry.dwVersion = pHeader->wVersion;
	NewEntry.dwVersionNeeded = pHeader->wVersionNeeded;
	NewEntry.dwFlags = pHeader->wFlags;
	NewEntry.dwCompression = pHeader->wCompression;
	NewEntry.dwCRC32 = pHeader->dwCRC;
	NewEntry.dwLocalHeaderOffset = pHeader->dwLocalHeaderOffset;

	const char* pPos = (const char*)&pHeader[1];
	if (pPos + pHeader->wFilenameLen > pEnd)
		return false;

	NewEntry.sFilename = CString(pPos, (int)pHeader->wFilenameLen);

	Directory.Insert(NewEntry);

	return true;
	}

bool CZipFormatReader::ReadFileAtOffset (DWORD dwOffset, IByteStream64& Stream, DWORD dwCompressedSize, CString* retsError) const

//	ReadFileAtOffset
//
//	Reads a file with the file header starting at the given offset.

	{
	if (!m_pData)
		throw CException(errFail);

	if (dwOffset + sizeof(CZipFormat::SLocalFileHeader) > m_pData->GetLength())
		{
		if (retsError) *retsError = ERR_ZIP_FILE_CORRUPT;
		return false;
		}

	const CZipFormat::SLocalFileHeader *pFileHeader = (const CZipFormat::SLocalFileHeader*)(m_pData->GetPointer() + dwOffset);
	DWORD dwDataOffset = dwOffset + sizeof(CZipFormat::SLocalFileHeader) + pFileHeader->wFilenameLen + pFileHeader->wExtraLen;
	const char *pFileData = m_pData->GetPointer() + dwDataOffset;

	if (pFileHeader->wFlags & CZipFormat::FLAG_ENCRYPTED)
		{
		if (retsError) *retsError = ERR_ZIP_FILE_ENCRYPTED;
		return false;
		}

	CBuffer FileData(pFileData, dwCompressedSize, false);

	switch (pFileHeader->wCompression)
		{
		case CZipFormat::COMP_NONE:
			compDecompress(FileData, compressionNone, Stream);
			break;

		case CZipFormat::COMP_DEFLATED:
			compDecompress(FileData, compressionZipFile, Stream);
			break;

		default:
			if (retsError) *retsError = strPattern(ERR_UNKNOWN_COMPRESSION, (int)pFileHeader->wCompression);
			return false;
		}

	return true;
	}
