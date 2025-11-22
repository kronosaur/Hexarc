//	CZipFormatWriter.cpp
//
//	CZipeFormatWriter class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#define ZLIB_WINAPI
#include "../zlib-1.2.7/zlib.h"
#include "../zlib-1.2.7/unzip.h"

DECLARE_CONST_STRING(ERR_UNABLE_TO_WRITE,				"Failed writing zip file.");

static CBuffer NULL_BUFFER("", 0);

void CZipFormatWriter::AddFile (const CString& sFilename, const IMemoryBlock& FileData, const CDateTime& ModifiedOn)

//	AddFile
//
//	Adds a file to the zip file.
//	NOTE: Callers must guarantee that the FileData pointer will be valid until
//	the zip file is written.

	{
	SEntry* pEntry = m_Directory.Insert();
	pEntry->sFilename = sFilename;
	if (FileData.GetPointer())
		pEntry->pFileData = &FileData;
	else
		pEntry->pFileData = &NULL_BUFFER;

	pEntry->ModifiedOn = ModifiedOn;
	pEntry->dwCRC = CalcCRC(FileData);
	pEntry->dwCompression = CZipFormat::COMP_DEFLATED;
	}

void CZipFormatWriter::AddFile (const CString& sFilename, CString&& FileData, const CDateTime& ModifiedOn)
	{
	SEntry* pEntry = m_Directory.Insert();
	pEntry->sFilename = sFilename;
	pEntry->FileData = CStringBuffer(std::move(FileData));
	pEntry->pFileData = &pEntry->FileData;

	pEntry->ModifiedOn = ModifiedOn;
	pEntry->dwCRC = CalcCRC(pEntry->FileData);
	pEntry->dwCompression = CZipFormat::COMP_DEFLATED;
	}

DWORD CZipFormatWriter::CalcCRC (const IMemoryBlock& Data)

//	CalcCRC
//
//	Calculates the CRC for the given data.
	{
	DWORD dwCRC = crc32(0, Z_NULL, 0);
	dwCRC = crc32(dwCRC, (const Bytef *)Data.GetPointer(), Data.GetLength());
	return dwCRC;
	}

bool CZipFormatWriter::Write (IByteStream& Stream, CString* retsError) const

//	Write
//
//	Write out the zip file to the stream.

	{
	try
		{
		//	Loop over all files and write out the local header.

		for (int i = 0; i < m_Directory.GetCount(); i++)
			{
			const SEntry& Entry = m_Directory[i];

			//	Deflate the file.

			ECompressionTypes iCompression = Entry.dwCompression == CZipFormat::COMP_NONE ? ECompressionTypes::compressionNone : ECompressionTypes::compressionZipFile;

			CBuffer CompressedFile;
			::compCompress(*Entry.pFileData, iCompression, &CompressedFile);
			Entry.dwCompressedSize = CompressedFile.GetLength();
			Entry.dwLocalHeaderOffset = Stream.GetPos();

			//	Write the local header

			if (!WriteLocalHeader(Stream, Entry, retsError))
				return false;

			//	Write the compressed file

			Stream.Write(CompressedFile);
			}

		//	Now write the central directory

		DWORD dwCentralDirOffset = Stream.GetPos();

		for (int i = 0; i < m_Directory.GetCount(); i++)
			{
			const SEntry& Entry = m_Directory[i];

			if (!WriteCentralDirectory(Stream, Entry, retsError))
				return false;
			}

		//	Write the end of central directory record

		DWORD dwCentralDirSize = Stream.GetPos() - dwCentralDirOffset;
		CZipFormat::SEndOfCentralDirectory EndOfCentralDir;
		EndOfCentralDir.dwSignature = CZipFormat::SIG_END_OF_CENTRAL_DIRECTORY;
		EndOfCentralDir.wDiskNumber = 0;
		EndOfCentralDir.wDiskStart = 0;
		EndOfCentralDir.wDiskEntries = m_Directory.GetCount();
		EndOfCentralDir.wTotalEntries = m_Directory.GetCount();
		EndOfCentralDir.dwCentralDirectorySize = dwCentralDirSize;
		EndOfCentralDir.dwCentralDirectoryOffset = dwCentralDirOffset;
		EndOfCentralDir.wCommentLen = 0;

		Stream.Write(&EndOfCentralDir, sizeof(EndOfCentralDir));

		return true;
		}
	catch (...)
		{
		if (retsError)
			*retsError = ERR_UNABLE_TO_WRITE;
		return false;
		}
	}

bool CZipFormatWriter::WriteCentralDirectory (IByteStream& Stream, const SEntry& Entry, CString* retsError) const

//	WriteCentralDirectory
//
//	Writes a central directory entry.

	{
	CZipFormat::SCentralDirectoryFileHeader Header;

	Header.dwSignature = CZipFormat::SIG_CENTRAL_DIRECTORY_FILE_HEADER;
	Header.wVersion = (WORD)(CZipFormat::VERSION | CZipFormat::VERSION_MADE_BY_WINDOWS);
	Header.wVersionNeeded = CZipFormat::VERSION;
	Header.wFlags = 0x0000;
	Header.wFlags |= CZipFormat::FLAG_UTF8;
	Header.wCompression = (WORD)Entry.dwCompression;
	Header.wModTime = Entry.ModifiedOn.GetDOSTime();
	Header.wModDate = Entry.ModifiedOn.GetDOSDate();
	Header.dwCRC = Entry.dwCRC;
	Header.dwCompressedSize = Entry.dwCompressedSize;
	Header.dwUncompressedSize = Entry.pFileData->GetLength();
	Header.wFilenameLen = Entry.sFilename.GetLength();
	Header.wExtraLen = 0;
	Header.wCommentLen = 0;
	Header.wDiskStart = 0;
	Header.wInternal = 0;
	Header.dwExternal = 0;
	Header.dwLocalHeaderOffset = Entry.dwLocalHeaderOffset;

	//	Write the header

	Stream.Write(&Header, sizeof(Header));

	//	Now write the filename

	Stream.Write(Entry.sFilename);

	return true;
	}

bool CZipFormatWriter::WriteLocalHeader (IByteStream& Stream, const SEntry& Entry, CString* retsError) const

//	WriteLocalHeader
//
//	Writes a local header for this entry.

	{
	CZipFormat::SLocalFileHeader Header;

	Header.dwSignature = CZipFormat::SIG_LOCAL_FILE_HEADER;
	Header.wVersion = CZipFormat::VERSION;
	Header.wFlags = 0x0000;
	Header.wFlags |= CZipFormat::FLAG_UTF8;
	Header.wCompression = (WORD)Entry.dwCompression;
	Header.wModTime = Entry.ModifiedOn.GetDOSTime();
	Header.wModDate = Entry.ModifiedOn.GetDOSDate();
	Header.dwCRC = Entry.dwCRC;
	Header.dwCompressedSize = Entry.dwCompressedSize;
	Header.dwUncompressedSize = Entry.pFileData->GetLength();
	Header.wFilenameLen = Entry.sFilename.GetLength();
	Header.wExtraLen = 0;

	//	Write the header

	Stream.Write(&Header, sizeof(Header));

	//	Now write the filename

	Stream.Write(Entry.sFilename);

	return true;
	}
