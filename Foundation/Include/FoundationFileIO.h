//	FoundationFileIO.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

const int MAX_FILE_PATH_CHARS =				1024;
const DWORDLONG MEGABYTE_DISK =				1000 * 1000;
const DWORDLONG GIGABYTE_DISK =				1000 * MEGABYTE_DISK;

class CFile : public IByteStream
	{
	public:
		enum Flags
			{
			FLAG_CREATE_ALWAYS =	0x00000001,		//	Always creates a new file
			FLAG_OPEN_READ_ONLY =	0x00000002,		//	Open read-only
			FLAG_CREATE_NEW =		0x00000004,		//	Open for read-write but only if new
			FLAG_ALLOW_DELETE =		0x00000008,		//	Allow others to delete file while open
			FLAG_OPEN_ALWAYS =		0x00000010,		//	Create the file if it doesn't exist
			FLAG_WRITE_THROUGH =	0x00000020,		//	Do not cache writes
			FLAG_ALLOW_WRITE =		0x00000040,		//	Allow others to write to the file while open

			//	If no flags are specified, we open read-write
			};

		CFile (void) : m_hFile(INVALID_HANDLE_VALUE) { }
		~CFile (void);

		void Close (void);
		bool Create (const CString &sFilespec, DWORD dwFlags, CString *retsError = NULL);
		bool Flush (void);
		inline const CString &GetFilespec (void) const { return m_sFilespec; }
		CDateTime GetModifiedTime (void);
		DWORDLONG GetSize (void);
		inline bool IsOpen (void) const { return m_hFile != INVALID_HANDLE_VALUE; }
		bool Lock (int iPos, int iLength, int iTimeout = 0);
		bool SetLength (int iLength);
		bool SetModifiedTime (const CDateTime &ModifiedOn);
		void Unlock (int iPos, int iLength);

		//	IByteStream virtuals
		virtual int GetPos (void) override;
		virtual int GetStreamLength (void) override;
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override;
		virtual int Write (const void *pData, int iLength) override;

		static CString TranslateError (DWORD dwError);

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream::Write;

	private:
		CString m_sFilespec;
		HANDLE m_hFile;
	};

class CFileBuffer : public CMemoryBlockImpl
	{
	public:
		enum Flags
			{
			FLAG_OPEN_READ_ONLY =	0x00000001,		//	Open read-only
			FLAG_SHARE_WRITE =		0x00000002,		//	Allow others to write to the file
			};

		CFileBuffer (void);
		virtual ~CFileBuffer (void);

		void Close (void);
		CDateTime GetModifiedTime (void);
		inline bool OpenReadOnly (const CString &sFilespec, CString *retsError = NULL) { return Open(sFilespec, FLAG_OPEN_READ_ONLY | FLAG_SHARE_WRITE, retsError); }

		//	IMemoryBlock virtuals
		virtual int GetLength (void) const { return m_iCurrentSize; }
		virtual char *GetPointer (void) const { return m_pBlock; }
		virtual void SetLength (int iLength) { }

	private:
		bool Open (const CString &sFilespec, DWORD dwFlags, CString *retsError);

		CString m_sFilespec;

		HANDLE m_hFile;
		HANDLE m_hFileMap;

		int m_iMaxSize;
		int m_iCommittedSize;
		int m_iCurrentSize;

		char *m_pBlock;
	};

class CFileBuffer64 : public CMemoryBlockImpl64
	{
	public:
		enum Flags
			{
			FLAG_OPEN_READ_ONLY =	0x00000001,		//	Open read-only
			FLAG_SHARE_WRITE =		0x00000002,		//	Allow others to write to the file
			};

		CFileBuffer64 (void);
		virtual ~CFileBuffer64 (void);

		void Close (void);
		CDateTime GetModifiedTime (void);
		inline bool IsOpen (void) const { return m_hFile != INVALID_HANDLE_VALUE; }
		inline bool OpenReadOnly (const CString &sFilespec, CString *retsError = NULL) { return Open(sFilespec, 0, 0, FLAG_OPEN_READ_ONLY | FLAG_SHARE_WRITE, retsError); }
		inline bool OpenReadOnly (const CString &sFilespec, DWORDLONG dwStart, DWORDLONG dwLength, CString *retsError = NULL) { return Open(sFilespec, dwStart, dwLength, FLAG_OPEN_READ_ONLY | FLAG_SHARE_WRITE, retsError); }

		//	IMemoryBlock virtuals

		virtual DWORDLONG GetLength (void) const { return m_dwCurrentSize; }
		virtual char *GetPointer (void) const { return m_pBlock; }
		virtual void SetLength (DWORDLONG iLength) { throw CException(errFail); }

	private:
		bool Open (const CString &sFilespec, DWORDLONG dwStart, DWORDLONG dwLength, DWORD dwFlags, CString *retsError);

		CString m_sFilespec;

		HANDLE m_hFile;
		HANDLE m_hFileMap;

		DWORDLONG m_dwMaxSize;
		DWORDLONG m_dwCommittedSize;
		DWORDLONG m_dwCurrentSize;

		char *m_pBlock;
	};

class CFileMultiplexer : public IByteStream
	{
	public:
		CFileMultiplexer (void) { }
		~CFileMultiplexer (void) { }

		void Close (void);
		bool Create (const CString &sFilespec, DWORD dwFlags, CString *retsError = NULL);
		bool CreateMirror (const CString &sFilespec, CString *retsError = NULL);
		bool Delete (void);
		bool Flush (void);
		bool OpenMirror (const CString &sFilespec, CString *retsError = NULL);

		//	IByteStream virtuals
		virtual int GetPos (void) override { return m_Primary.GetPos(); }
		virtual int GetStreamLength (void) override { return m_Primary.GetStreamLength(); }
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override;
		virtual int Write (const void *pData, int iLength) override;

		//	Helpers	(Needed because IByteStream::Write is hidden when we
		//	have the derived class)
		inline int Write (const CString &sString) { return IByteStream::Write(sString); }
		inline int Write (IMemoryBlock &Block) { return IByteStream::Write(Block); }

	private:
		CFile m_Primary;
		TArray<CFile> m_Mirrors;
	};

class CModuleResource : public CMemoryBlockImpl
	{
	public:
		CModuleResource (void) : m_hRes(NULL) { }
		virtual ~CModuleResource (void) { Close(); }

		void Close (void);
		bool Open (const CString &sType, const CString &sName);

		//	IMemoryBlock virtuals
		virtual int GetLength (void) const { return m_iLength; }
		virtual char *GetPointer (void) const { return (char *)::LockResource(m_hRes); }
		virtual void SetLength (int iLength) { }

	private:
		HGLOBAL m_hRes;
		int m_iLength;
	};

//	SFileVersionInfo -----------------------------------------------------------

struct SFileVersionInfo
	{
	CString sProductName;
	CString sProductVersion;
	CString sCompanyName;
	CString sCopyright;

	DWORDLONG dwFileVersion = 0;
	DWORDLONG dwProductVersion = 0;
	};

//	Utility functions ----------------------------------------------------------

CString fileAppend (const CString &sPath, const CString &sComponent);
CString fileAppendExtension (const CString &sFilespec, const CString &sExtension);
DWORD fileChecksumAdler32 (const CString &sFilespec);
bool fileCompare (const CString &sFilespec1, const CString &sFilespec2, bool bQuick = false);
int fileCompareModifiedTime (const CString &sFilespec1, const CString &sFilespec2);
bool fileCopy (const CString &sFrom, const CString &sTo, CString *retsError = NULL);
bool fileCreateDrive (const CString &sPath, CString *retsDriveRoot);
bool fileDelete (const CString &sFilespec);
bool fileDeleteDrive (const CString &sDriveRoot);
bool fileExists (const CString &sFilespec, bool *retbIsFile = NULL);
CString fileGetAbsoluteFilespec (const CString &sFilespec);
CString fileGetDrive (const CString &sFilespec);
void fileGetDriveSpace (const CString &sDrive, DWORDLONG *retdwAvailable, DWORDLONG *retdwTotalSize = NULL);
CString fileGetExecutableFilespec (void);
CString fileGetExtension (const CString &sFilespec, CString *retsRemainder = NULL);

const DWORD FFL_FLAG_DIRECTORIES_ONLY =		0x00000001;
const DWORD FFL_FLAG_RELATIVE_FILESPEC =	0x00000002;
const DWORD FFL_FLAG_RECURSIVE =			0x00000004;
bool fileGetFileList (const CString &sRoot, const CString &sPath, const CString &sSearch, DWORD dwFlags, TArray<CString> *retFiles);

CString fileGetFilename (const CString &sFilespec);
CDateTime fileGetModifiedTime (const CString &sFilespec);
CString fileGetPath (const CString &sFilespec);
DWORDLONG fileGetSize (const CString &sFilespec);
CString fileGetSystemPath ();
CString fileGetTempPath (void);
bool fileGetVersionInfo (const CString &sFilespec, SFileVersionInfo *retInfo, CString *retsError = NULL);
bool fileGetVolumeList (DWORD dwFlags, TArray<CString> *retVolumes);
CString fileGetWindowsPath ();
CString fileGetWorkingDirectory (void);
bool fileIsAbsolute (const CString &sFilespec);
bool fileIsDotted (const CString &sFilespec);
bool fileIsFilename (const CString &sFilespec);
bool fileIsPathEqual (const CString &sFilespec1, const CString &sFilespec2);
bool fileIsWildcard (const CString &sFilespec);
bool fileMove (const CString &sSourceFilespec, const CString &sDestFilespec);
bool filePathCreate (const CString &sPath, CString *retsError = NULL);

const DWORD FPD_FLAG_RECURSIVE =			0x00000001;	//	Delete files and subdirectories inside
const DWORD FPD_FLAG_CONTENT_ONLY =			0x00000002;	//	Leave parent directory (only with _RECURSIVE)
bool filePathDelete (const CString &sPath, DWORD dwFlags = 0);

inline bool fileGetFileList (const CString &sFilespec, DWORD dwFlags, TArray<CString> *retFiles)
	{ return fileGetFileList(fileGetPath(sFilespec), NULL_STR, fileGetFilename(sFilespec), dwFlags, retFiles); }

bool fileSetWorkingDirectory (const CString &sPath);

//	Block Box Implementation ----------------------------------------------------

class CBlackBox : public ILogService
	{
	public:
		CBlackBox (void) { }
		~CBlackBox (void) { Shutdown(); }

		void Boot (const CString &sPath, const CString &sLogPrefix = NULL_STR, bool bSetConsoleOutput = false);
		void Log (const CString &sLine);
		void LogExecVersion (void);
		void SetConsoleOutput (bool bEnabled = true) { ::SetConsoleOutputCP(65001); m_bConsoleOut = bEnabled; }
		void Shutdown (void);

		static bool ReadRecent (const CString &sPath, const CString &sFind, int iLines, TArray<CString> *retLines);

		//	ILogService

		virtual void Write (const CString &sLine, ELogClasses iClass = logInfo) override { Log(sLine); }

	private:
		CFile m_File;
		bool m_bConsoleOut = false;				//	If TRUE, we also printf to the console
	};

//	Utilities ------------------------------------------------------------------

class CSaveWorkingDirectory
	{
	public:
		CSaveWorkingDirectory (void)
			{
			m_sOldDir = fileGetWorkingDirectory();
			}

		explicit CSaveWorkingDirectory (const CString &sFilespec)
			{
			m_sOldDir = fileGetWorkingDirectory();
			fileSetWorkingDirectory(sFilespec);
			}

		CSaveWorkingDirectory (const CSaveWorkingDirectory &Src) = delete;
		CSaveWorkingDirectory (CSaveWorkingDirectory &&Src) = delete;

		~CSaveWorkingDirectory (void)
			{
			fileSetWorkingDirectory(m_sOldDir);
			}

		CSaveWorkingDirectory &operator= (const CSaveWorkingDirectory &Src) = delete;
		CSaveWorkingDirectory &operator= (CSaveWorkingDirectory &&Src) = delete;

	private:
		CString m_sOldDir;
	};

