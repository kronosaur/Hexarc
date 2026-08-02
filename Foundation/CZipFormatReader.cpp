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
DECLARE_CONST_STRING(ERR_ZIP_BAD_PASSWORD,				"Invalid zip password.");
DECLARE_CONST_STRING(ERR_ZIP_FILE_CORRUPT,				"Zip file corrupted.");
DECLARE_CONST_STRING(ERR_ZIP_FILE_ENCRYPTED,			"Zip file encrypted.");
DECLARE_CONST_STRING(ERR_ZIP_PASSWORD_REQUIRED,			"Zip password required.");

static int ZipCryptoDecryptByte (DWORD* pKeys)
	{
	DWORD dwTemp = (pKeys[2] & 0xffff) | 2;
	return (int)(((dwTemp * (dwTemp ^ 1)) >> 8) & 0xff);
	}

static int ZipCryptoUpdateKeys (DWORD* pKeys, const z_crc_t* pcrc32, int c)
	{
	pKeys[0] = (DWORD)(pcrc32[((int)pKeys[0] ^ c) & 0xff] ^ (pKeys[0] >> 8));
	pKeys[1] += pKeys[0] & 0xff;
	pKeys[1] = pKeys[1] * 134775813L + 1;
	pKeys[2] = (DWORD)(pcrc32[((int)pKeys[2] ^ (int)(pKeys[1] >> 24)) & 0xff] ^ (pKeys[2] >> 8));

	return c;
	}

static void ZipCryptoInitKeys (CStringView sPassword, DWORD* pKeys, const z_crc_t* pcrc32)
	{
	pKeys[0] = 305419896L;
	pKeys[1] = 591751049L;
	pKeys[2] = 878082192L;

	for (int i = 0; i < sPassword.GetLength(); i++)
		ZipCryptoUpdateKeys(pKeys, pcrc32, (BYTE)sPassword.GetParsePointer()[i]);
	}

static int ZipCryptoDecode (DWORD* pKeys, const z_crc_t* pcrc32, int c)
	{
	c ^= ZipCryptoDecryptByte(pKeys);
	return ZipCryptoUpdateKeys(pKeys, pcrc32, c);
	}

CZipFormatReader::CZipFormatReader (const IMemoryBlock& Data)

//	CZipFormat constructor

	{
	Init(Data);
	}

DWORD CZipFormatReader::CalcCRC32 (const IMemoryBlock64& Data)

//	CalcCRC32
//
//	Computes CRC32 for a memory block.

	{
	DWORD dwCRC = crc32(0, Z_NULL, 0);

	DWORDLONG dwLeft = Data.GetLength();
	const Bytef* pPos = (const Bytef*)Data.GetPointer();

	while (dwLeft > 0)
		{
		const uInt dwChunk = (uInt)Min(dwLeft, (DWORDLONG)0xffffffff);
		dwCRC = crc32(dwCRC, pPos, dwChunk);

		pPos += dwChunk;
		dwLeft -= dwChunk;
		}

	return dwCRC;
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

bool CZipFormatReader::HasEncryptedFiles () const

//	HasEncryptedFiles
//
//	Returns TRUE if any file in the archive is encrypted.

	{
	for (int i = 0; i < m_Directory.GetCount(); i++)
		if (IsFileEncrypted(i))
			return true;

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
	return ReadFile(iIndex, retBuffer, NULL_STR, retsError);
	}

bool CZipFormatReader::ReadFile (int iIndex, CBuffer64& retBuffer, CStringView sPassword, CString* retsError) const

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
		if (!ReadFileAtOffset(m_Directory[iIndex], retBuffer, sPassword, retsError))
			return false;
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_ZIP_FILE_CORRUPT;
			return false;
		}

	if (CalcCRC32(retBuffer) != m_Directory[iIndex].dwCRC32)
		{
		if (retsError) *retsError = (IsFileEncrypted(iIndex) ? ERR_ZIP_BAD_PASSWORD : ERR_ZIP_FILE_CORRUPT);
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

bool CZipFormatReader::ReadFile (const CString& sFilename, CBuffer64& retBuffer, CStringView sPassword, CString* retsError) const

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

	return ReadFile(iIndex, retBuffer, sPassword, retsError);
	}

//	Internal Methods -----------------------------------------------------------

bool CZipFormatReader::DecryptZipCrypto (const char* pFileData, DWORD dwCompressedSize, DWORD dwCRC32, DWORD dwFlags, WORD wModTime, CStringView sPassword, CBuffer& retDecrypted, CString* retsError)

//	DecryptZipCrypto
//
//	Decrypts a traditional PKWARE-encrypted ZIP entry. The encrypted data starts
//	with a 12-byte encryption header, followed by compressed bytes.

	{
	constexpr DWORD ZIP_CRYPTO_HEADER_SIZE = 12;

	if (sPassword.IsEmpty())
		{
		if (retsError) *retsError = ERR_ZIP_PASSWORD_REQUIRED;
		return false;
		}
	else if (dwCompressedSize < ZIP_CRYPTO_HEADER_SIZE)
		{
		if (retsError) *retsError = ERR_ZIP_FILE_CORRUPT;
		return false;
		}

	DWORD Keys[3];
	const z_crc_t* pcrc_32_tab = get_crc_table();
	ZipCryptoInitKeys(sPassword, Keys, pcrc_32_tab);

	BYTE Header[ZIP_CRYPTO_HEADER_SIZE];
	for (DWORD i = 0; i < ZIP_CRYPTO_HEADER_SIZE; i++)
		{
		int iByte = (BYTE)pFileData[i];
		Header[i] = (BYTE)ZipCryptoDecode(Keys, pcrc_32_tab, iByte);
		}

	const BYTE byExpectedCheck = (BYTE)(((dwFlags & 8) ? wModTime : (dwCRC32 >> 16)) >> 8);
	if (Header[ZIP_CRYPTO_HEADER_SIZE - 1] != byExpectedCheck)
		{
		if (retsError) *retsError = ERR_ZIP_BAD_PASSWORD;
		return false;
		}

	const DWORD dwDecryptedSize = dwCompressedSize - ZIP_CRYPTO_HEADER_SIZE;
	retDecrypted.SetLength(dwDecryptedSize);

	char* pDest = retDecrypted.GetPointer();
	const char* pSrc = pFileData + ZIP_CRYPTO_HEADER_SIZE;
	for (DWORD i = 0; i < dwDecryptedSize; i++)
		{
		int iByte = (BYTE)pSrc[i];
		pDest[i] = (char)ZipCryptoDecode(Keys, pcrc_32_tab, iByte);
		}

	return true;
	}

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

bool CZipFormatReader::ReadFileAtOffset (const SEntry& Entry, IByteStream64& Stream, CStringView sPassword, CString* retsError) const

//	ReadFileAtOffset
//
//	Reads a file with the file header starting at the given offset.

	{
	if (!m_pData)
		throw CException(errFail);

	DWORD dwOffset = Entry.dwLocalHeaderOffset;
	if (dwOffset + sizeof(CZipFormat::SLocalFileHeader) > m_pData->GetLength())
		{
		if (retsError) *retsError = ERR_ZIP_FILE_CORRUPT;
		return false;
		}

	const CZipFormat::SLocalFileHeader *pFileHeader = (const CZipFormat::SLocalFileHeader*)(m_pData->GetPointer() + dwOffset);
	DWORD dwDataOffset = dwOffset + sizeof(CZipFormat::SLocalFileHeader) + pFileHeader->wFilenameLen + pFileHeader->wExtraLen;
	const char *pFileData = m_pData->GetPointer() + dwDataOffset;

	if ((DWORDLONG)dwDataOffset + Entry.dwCompressedSize > (DWORDLONG)m_pData->GetLength())
		{
		if (retsError) *retsError = ERR_ZIP_FILE_CORRUPT;
		return false;
		}

	CBuffer DecryptedData;
	CBuffer FileData;
	const IMemoryBlock* pDataToDecompress = NULL;

	if (pFileHeader->wFlags & CZipFormat::FLAG_ENCRYPTED)
		{
		if (sPassword.IsEmpty())
			{
			if (retsError) *retsError = ERR_ZIP_FILE_ENCRYPTED;
			return false;
			}

		if (!DecryptZipCrypto(pFileData, Entry.dwCompressedSize, Entry.dwCRC32, pFileHeader->wFlags, pFileHeader->wModTime, sPassword, DecryptedData, retsError))
			return false;

		pDataToDecompress = &DecryptedData;
		}
	else
		{
		FileData = CBuffer(pFileData, Entry.dwCompressedSize, false);
		pDataToDecompress = &FileData;
		}

	switch (pFileHeader->wCompression)
		{
		case CZipFormat::COMP_NONE:
			compDecompress(*pDataToDecompress, compressionNone, Stream);
			break;

		case CZipFormat::COMP_DEFLATED:
			compDecompress(*pDataToDecompress, compressionZipFile, Stream);
			break;

		default:
			if (retsError) *retsError = strPattern(ERR_UNKNOWN_COMPRESSION, (int)pFileHeader->wCompression);
			return false;
		}

	return true;
	}
