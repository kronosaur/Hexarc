//	FoundationCompression.h
//
//	Foundation header file
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

enum ECompressionTypes
	{
	compressionNone =						0,
	compressionZlib =						1,	//	Zlib format
	compressionGzip =						2,	//	Gzip format
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

void compCompress (IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer);
void compDecompress (IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer);
TUniquePtr<ICompressedFile> compZipFile (const CString &sFilespec, CString *retsError = NULL);
