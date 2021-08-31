//	CZipFile.cpp
//
//	CZipeFile class
//	Copyright (c) 2021 Kronosaur Production, LLC. All Rights Reserved.

#include "stdafx.h"

#define ZLIB_WINAPI
#include "..\zlib-1.2.7\zlib.h"
#include "..\zlib-1.2.7\unzip.h"

class CZipFile : public ICompressedFile
	{
	public:
		virtual void CleanUp () override;
		virtual bool Decompress (int iIndex, const CString &sDestDir, CString *retsError = NULL) const override;
		virtual int GetCount () const override;
		virtual CString GetFilespec (int iIndex) const override;
		virtual CDateTime GetModifiedOn (int iIndex) const override;
		virtual bool Open (const CString &sFilespec, DWORD dwFlags, CString *retsError = NULL) override;

	private:
		struct SZipFileEntry
			{
			CString sFilespec;
			CDateTime ModifiedOn;
			DWORDLONG dwCompressedBytes = 0;
			DWORDLONG dwUncompressedBytes = 0;

			unz_file_pos FilePos = { 0 };
			};

		unzFile m_ZipFile = NULL;

		unz_global_info m_GlobalInfo;
		TArray<SZipFileEntry> m_Directory;
	};

TUniquePtr<ICompressedFile> compZipFile (const CString &sFilespec, CString *retsError)

//	compZipFile
//
//	Returns an object to manipulate an existing zip file.

	{
	TUniquePtr<ICompressedFile> pZipFile(new CZipFile);
	if (!pZipFile)
		throw CException(errOutOfMemory);

	if (!pZipFile->Open(sFilespec, 0, retsError))
		return NULL;

	return pZipFile;
	}

//	CZipFile -------------------------------------------------------------------

void CZipFile::CleanUp ()

//	CleanUp
//
//	Clean up resources.

	{
	if (m_ZipFile)
		{
		unzClose(m_ZipFile);
		m_ZipFile = NULL;
		m_Directory.DeleteAll();
		}
	}

bool CZipFile::Decompress (int iIndex, const CString &sDestDir, CString *retsError) const

//	Decompress
//
//	Decompress a file in the zip file.

	{
	if (!m_ZipFile)
		throw CException(errFail);

	if (iIndex < 0 || iIndex >= m_Directory.GetCount())
		throw CException(errFail);

	//	Select the file

	if (unzGoToFilePos(m_ZipFile, &m_Directory[iIndex].FilePos) != UNZ_OK)
		{
		if (retsError) *retsError = strPattern("Unable to locate %s.", m_Directory[iIndex].sFilespec);
		return false;
		}

	//	Open the file

	if (unzOpenCurrentFile(m_ZipFile) != UNZ_OK)
		{
		if (retsError) *retsError = strPattern("Unable to open %s.", m_Directory[iIndex].sFilespec);
		return false;
		}

	//	Create a file to write out the data

	CString sDestFilespec = fileAppend(sDestDir, m_Directory[iIndex].sFilespec);
	CFile DestFile;
	if (!DestFile.Create(sDestFilespec, CFile::FLAG_CREATE_ALWAYS, retsError))
		{
		unzCloseCurrentFile(m_ZipFile);
		return false;
		}

	//	Keep writing until we're done

	constexpr size_t READ_SIZE = 8192;
	char ReadBuffer[READ_SIZE];

	int iBytesWritten = 0;
	do 
		{
		iBytesWritten = unzReadCurrentFile(m_ZipFile, ReadBuffer, READ_SIZE);
		if (iBytesWritten < 0)
			{
			unzCloseCurrentFile(m_ZipFile);
			if (retsError) *retsError = strPattern("Unable to read %s.", m_Directory[iIndex].sFilespec);
			return false;
			}

		try
			{
			DestFile.Write(ReadBuffer, iBytesWritten);
			}
		catch (...)
			{
			unzCloseCurrentFile(m_ZipFile);
			if (retsError) *retsError = strPattern("Unable to write to %s.", sDestFilespec);
			return false;
			}
		}
	while (iBytesWritten > 0);

	//	Done

	unzCloseCurrentFile(m_ZipFile);
	DestFile.SetModifiedTime(m_Directory[iIndex].ModifiedOn);

	return true;
	}

int CZipFile::GetCount () const

//	GetCount
//
//	Returns the number of file entries.

	{
	return m_Directory.GetCount();
	}

CString CZipFile::GetFilespec (int iIndex) const

//	GetFilespec
//
//	Returns the filespec for the entry.

	{
	if (!m_ZipFile)
		throw CException(errFail);

	if (iIndex < 0 || iIndex >= m_Directory.GetCount())
		throw CException(errFail);

	return m_Directory[iIndex].sFilespec;
	}

CDateTime CZipFile::GetModifiedOn (int iIndex) const

//	GetModifiedOn
//
//	Returns the modified datetime.

	{
	if (!m_ZipFile)
		throw CException(errFail);

	if (iIndex < 0 || iIndex >= m_Directory.GetCount())
		throw CException(errFail);

	return m_Directory[iIndex].ModifiedOn;
	}

bool CZipFile::Open (const CString &sFilespec, DWORD dwFlags, CString *retsError)

//	Open
//
//	Open the zip file for reading.

	{
	CleanUp();

	m_ZipFile = unzOpen((LPCSTR)sFilespec);
	if (!m_ZipFile)
		{
		if (retsError) *retsError = strPattern("Unable to open zip file: %s.", sFilespec);
		return false;
		}

	if (unzGetGlobalInfo(m_ZipFile, &m_GlobalInfo) != UNZ_OK)
		{
		CleanUp();
		if (retsError) *retsError = strPattern("Unable to read zip file info: %s.", sFilespec);
		return false;
		}

	constexpr char DIR_DELIMITER = '/';

	for (int i = 0; i < (int)m_GlobalInfo.number_entry; i++)
		{
		unz_file_info FileInfo;
		constexpr size_t MAX_FILENAME = 512;
		char szFilename[MAX_FILENAME];

		if (unzGetCurrentFileInfo(m_ZipFile, &FileInfo, szFilename, MAX_FILENAME, NULL, 0, NULL, 0) != UNZ_OK)
			{
			CleanUp();
			if (retsError) *retsError = strPattern("Unable to read file info: %s.", sFilespec);
			return false;
			}

		//	If this is a directory, skip it.

		const size_t dwFilenameLen = strlen(szFilename);
		if (szFilename[dwFilenameLen - 1] == DIR_DELIMITER)
			{ }

		//	Otherwise, add this file to our list.

		else
			{
			auto *pEntry = m_Directory.Insert();
			pEntry->sFilespec = CString(szFilename);
			pEntry->dwCompressedBytes = FileInfo.compressed_size;
			pEntry->dwUncompressedBytes = FileInfo.uncompressed_size;
			pEntry->ModifiedOn = CDateTime::FromDOSDateTime(FileInfo.dosDate);

			//	Remember the file position

			if (unzGetFilePos(m_ZipFile, &pEntry->FilePos) != UNZ_OK)
				{
				CleanUp();
				if (retsError) *retsError = strPattern("Unable to read file pos: %s.", sFilespec);
				return false;
				}
			}

		//	Next

		if ((i + 1) < (int)m_GlobalInfo.number_entry)
			{
			if (unzGoToNextFile(m_ZipFile) != UNZ_OK)
				{
				CleanUp();
				if (retsError) *retsError = strPattern("Unable to read next file in zip: %s.", sFilespec);
				return false;
				}
			}
		}

	//	Done

	return true;
	}
