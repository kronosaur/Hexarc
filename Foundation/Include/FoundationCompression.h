//	FoundationCompression.h
//
//	Foundation header file
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

#include "FoundationZipFile.h"

enum ECompressionTypes
	{
	compressionNone =						0,
	compressionZlib =						1,	//	Zlib format
	compressionGzip =						2,	//	Gzip format
	compressionZipFile =					3,	//	ZipFile format
	};

class ICompressedFile
	{
	public:
		virtual ~ICompressedFile () { CleanUp(); }
		virtual void CleanUp () { }
		virtual bool Decompress (int iIndex, const CString &sDestDir, CString *retsError = NULL) const { return false; }
		virtual int GetCount () const { return 0; }
		virtual CString GetFilespec (int iIndex) const { return NULL_STR; }
		virtual CDateTime GetModifiedOn (int iIndex) const { return CDateTime(); }
		virtual bool Open (const CString &sFilespec, DWORD dwFlags, CString *retsError = NULL) { return false; }
	};

class CZipFormatReader
	{
	public:

		CZipFormatReader () { }
		CZipFormatReader (const IMemoryBlock& Data);

		bool FindFileByName (const CString& sFilename, int* retiIndex = NULL) const;
		int GetCount () const { return m_Directory.GetCount(); }
		CString GetFilespec (int iIndex) const { return m_Directory[iIndex].sFilename; }
		CDateTime GetModifiedOn (int iIndex) const { return m_Directory[iIndex].ModifiedOn; }
		DWORDLONG GetFileSize (int iIndex) const { return m_Directory[iIndex].dwUncompressedSize; }
		bool Init (const IMemoryBlock& Data);
		bool IsEmpty () const { return m_Directory.GetCount() == 0; }
		bool ReadFile (int iIndex, CBuffer64& retBuffer, CString* retsError = NULL) const;
		bool ReadFile (const CString& sFilename, CBuffer64& retBuffer, CString* retsError = NULL) const;

	private:

		struct SEntry
			{
			CString sFilename;
			DWORD dwCompressedSize = 0;
			DWORD dwUncompressedSize = 0;
			CDateTime ModifiedOn;

			DWORD dwVersion = 0;
			DWORD dwVersionNeeded = 0;
			DWORD dwFlags = 0;
			DWORD dwCompression = 0;
			DWORD dwCRC32 = 0;

			DWORD dwLocalHeaderOffset = 0;
			};


		static bool ReadDirectoryFileHeader (TArray<SEntry>& Directory, CZipFormat::SCentralDirectoryFileHeader* pHeader, const char* pEnd);
		bool ReadFileAtOffset (DWORD dwOffset, IByteStream64& Stream, DWORD dwCompressedSize, CString* retsError = NULL) const;

		const IMemoryBlock* m_pData = NULL;
		TArray<SEntry> m_Directory;
	};

class CZipFormatWriter
	{
	public:

		CZipFormatWriter () { }
		~CZipFormatWriter () { }

		void AddFile (const CString& sFilename, const IMemoryBlock& FileData, const CDateTime& ModifiedOn);
		void AddFile (const CString& sFilename, CString&& FileData, const CDateTime& ModifiedOn);
		bool Write (IByteStream& Stream, CString* retsError = NULL) const;

	private:

		struct SEntry
			{
			CString sFilename;
			const IMemoryBlock* pFileData = NULL;
			CDateTime ModifiedOn;
			DWORD dwCRC = 0;						//	CRC of uncompressed file data
			DWORD dwCompression = CZipFormat::COMP_NONE;

			CStringBuffer FileData;					//	Uncompressed file data, if we keep ownership
													//	pFileData will point to this value.

			mutable DWORD dwCompressedSize = 0;		//	Compressed size of file data
			mutable DWORD dwLocalHeaderOffset = 0;	//	Offset of local header
			};

		static DWORD CalcCRC (const IMemoryBlock& Data);
		bool WriteCentralDirectory (IByteStream& Stream, const SEntry& Entry, CString* retsError = NULL) const;
		bool WriteLocalHeader (IByteStream& Stream, const SEntry& Entry, CString* retsError = NULL) const;

		TArray<SEntry> m_Directory;
	};

void compCompress (const IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer);
void compDecompress (const IMemoryBlock &Data, ECompressionTypes iType, IByteStream64 &Output);
TUniquePtr<ICompressedFile> compZipFile (const CString &sFilespec, CString *retsError = NULL);
