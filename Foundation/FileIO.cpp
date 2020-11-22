//	FileIO.cpp
//
//	File IO functions
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_DOT,							".")
DECLARE_CONST_STRING(STR_DOT_DOT,						"..")
DECLARE_CONST_STRING(STR_STAR,							"*")

DECLARE_CONST_STRING(STR_COMPANY_NAME,					"CompanyName")
DECLARE_CONST_STRING(STR_COPYRIGHT,						"LegalCopyright")
DECLARE_CONST_STRING(STR_PRODUCT_NAME,					"ProductName")
DECLARE_CONST_STRING(STR_PRODUCT_VERSION,				"ProductVersion")

DECLARE_CONST_STRING(ERR_CANT_CREATE_DIRECTORY,			"Unable to create directory: %s. [%x]")

CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString);

CString fileAppend (const CString &sPath, const CString &sComponent)

//	fileAppend
//
//	Concatenates the given component to the given path and returns
//	the result
//
//	sPath: full pathname to a directory (e.g. "c:\", "\\lawrence\cdrom", "d:\test")
//	sComponent: directory, filename, or wildcard.

	{
	if (sComponent.IsEmpty())
		return sPath;
	else if (sPath.IsEmpty())
		return sComponent;

	const char *pComponent = sComponent.GetParsePointer();
	const char *pPathEnd = sPath.GetParsePointer() + sPath.GetLength() - 1;

	//	If the path has a trailing backslash...

	if (*pPathEnd == '\\' || *pPathEnd == '/')
		{
		//	If the component has a leading backslash, then we need to remove it

		if (pComponent[0] == '\\' || pComponent[0] == '/')
			return strPattern("%s%s", sPath, (LPSTR)CString(pComponent + 1));

		//	If the component has a leading .\ then we remove it

		else if (pComponent[0] == '.' && (pComponent[1] == '\\' || pComponent[1] == '/'))
			return strPattern("%s%s", sPath, (LPSTR)CString(pComponent + 2));

		//	Otherwise, we just concatenate them together

		else
			return strPattern("%s%s", sPath, (LPSTR)sComponent);
		}

	//	Otherwise, if the path does NOT have a trailing backslash

	else
		{
		//	If the component has a leading backslash, then we concatenate

		if (pComponent[0] == '\\' || pComponent[0] == '/')
			return strPattern("%s%s", sPath, (LPSTR)sComponent);

		//	If the component has a leading .\ then we remove the dot

		else if (pComponent[0] == '.' && (pComponent[1] == '\\' || pComponent[1] == '/'))
			return strPattern("%s%s", sPath, (LPSTR)CString(pComponent + 1));

		//	Otherwise, we append on

		else
			return strPattern("%s\\%s", sPath, (LPSTR)sComponent);
		}
	}

CString fileAppendExtension (const CString &sFilespec, const CString &sExtension)

//	fileAppendExtension
//
//	Appends an extension to the given filespec as long as it does not already
//	have an extension.

	{
	ASSERT(!sFilespec.IsEmpty());
	if (sExtension.IsEmpty())
		return sFilespec;

	//	See if the extension that we have has a leading dot

	bool bHasDot = (*(sExtension.GetParsePointer()) == '.');

	//	Get the current extension

	CString sRemainder;
	CString sCurrentExtension = fileGetExtension(sFilespec, &sRemainder);

	//	If we already have an extension, then we don't do anything

	if (!sCurrentExtension.IsEmpty())
		return sFilespec;

	//	Otherwise, replace with the current

	if (bHasDot)
		return strPattern("%s%s", sRemainder, sExtension);
	else
		return strPattern("%s.%s", sRemainder, sExtension);
	}

bool fileCompare (const CString &sFilespec1, const CString &sFilespec2, bool bQuick)

//	fileCompare
//
//	Compare two files. Returns TRUE if they are the same.

	{
	CFileBuffer File1;
	if (!File1.OpenReadOnly(sFilespec1))
		return false;

	CFileBuffer File2;
	if (!File2.OpenReadOnly(sFilespec2))
		return false;

	if (File1.GetLength() != File2.GetLength())
		return false;

	//	If all we want is a quick compare, then just check that the modified
	//	file times are the same.

	if (bQuick)
		return (File1.GetModifiedTime() == File2.GetModifiedTime());

	//	Otherwise, do a byte-for-byte comparison.

	DWORD dwDWORDLen = File1.GetLength() / sizeof(DWORD);
	DWORD dwExtraLen = File1.GetLength() - (dwDWORDLen * sizeof(DWORD));

	DWORD *pPos1 = (DWORD *)File1.GetPointer();
	DWORD *pPosEnd = pPos1 + dwDWORDLen;
	DWORD *pPos2 = (DWORD *)File2.GetPointer();
	while (pPos1 < pPosEnd)
		if (*pPos1++ != *pPos2++)
			return false;

	char *pPosRemainder1 = (char *)pPos1;
	char *pPosRemainderEnd = pPosRemainder1 + dwExtraLen;
	char *pPosRemainder2 = (char *)pPos2;
	while (pPosRemainder1 < pPosRemainderEnd)
		if (*pPosRemainder1++ != *pPosRemainder2++)
			return false;

	return true;
	}

int fileCompareModifiedTime (const CString &sFilespec1, const CString &sFilespec2)

//	fileCompareModifiedTime
//
//	If Filespec1 > Filespec2,		1
//	If Filespec1 == Filespec2,		0
//	If Filespec1 < Filespec2,		-1
//
//	NOTE: If either filespec1 or filespec2 don't exist, we treat them as being
//	at the beginning of time.

	{
	return fileGetModifiedTime(sFilespec1).Compare(fileGetModifiedTime(sFilespec2));
	}

DWORD fileChecksumAdler32 (const CString &sFilespec)

//	fileChecksumAdler32
//
//	Computes an Adler-32 checksum for the entire file. Returns 0 if there is
//	an error (e.g., opening the file).

	{
	CFileBuffer File;
	if (!File.OpenReadOnly(sFilespec))
		return 0;

	DWORD dwAdler32;
	try
		{
		dwAdler32 = utlAdler32(File);
		}
	catch (...)
		{
		dwAdler32 = 0;
		}

	return dwAdler32;
	}

bool fileCopy (const CString &sFrom, const CString &sTo)

//	fileCopy
//
//	Copies a file.

	{
	if (!::CopyFile(CString16(sFrom), CString16(sTo), FALSE))
		return false;

	return true;
	}

bool fileCreateDrive (const CString &sPath, CString *retsDriveRoot)

//	fileCreateDrive
//
//	Creates a new drive mapped to sPath

	{
	int i;

	//	Get a drive letter that is open

	DWORD dwDriveMask = ::GetLogicalDrives();
	if (dwDriveMask == 0)
		return false;

	//	NOTE: GetLogicalDrives does not include drives A and B.

	CString sNewDrive = CString("?:");
	char *pPos = sNewDrive.GetParsePointer();
	for (i = 2; i < 32; i++)
		if (!(dwDriveMask & (1 << i)))
			{
			*pPos = 'A' + i;
			break;
			}

	if (*pPos == '?')
		return false;

	if (!::DefineDosDevice(DDD_NO_BROADCAST_SYSTEM, 
			CString16(sNewDrive),
			CString16(sPath)))
		{
		DWORD dwError = ::GetLastError();
		return false;
		}

	//	Done

	if (retsDriveRoot)
		*retsDriveRoot = strPattern("%s\\", sNewDrive);

	return true;
	}

bool fileDelete (const CString &sFilespec)

//	fileDelete
//
//	Delete the file permanently from disk.

	{
	if (!::DeleteFile(CString16(sFilespec)))
		return false;

	return true;
	}

bool fileDeleteDrive (const CString &sDriveRoot)

//	fileDeleteDrive
//
//	Deletes a drive previously created by fileCreateDrive

	{
	CString sDrive = CString("?:");
	char *pPos = sDrive.GetParsePointer();
	*pPos = *sDriveRoot.GetParsePointer();

	if (!::DefineDosDevice(DDD_REMOVE_DEFINITION, CString16(sDrive), NULL))
		return false;

	return true;
	}

bool fileExists (const CString &sFilespec, bool *retbIsFile)

//	fileExists
//
//	Returns TRUE if the given path exists

	{
	DWORD dwResult = ::GetFileAttributes(CString16(sFilespec));

	if (retbIsFile)
		*retbIsFile = ((dwResult & FILE_ATTRIBUTE_DIRECTORY) ? false : true);

	return (dwResult != 0xffffffff);
	}

CString fileGetAbsoluteFilespec (const CString &sFilespec)

//	fileGetAbsoluteFilespec
//
//	Makes the given path absolute (based on the current directory)
//
//	Test:
//
//		c:\
//		\\leibowitz\c\
//		\test\
//		\test\test2
//		.\test
//		..\test\
//		..\..\test\
//		test\test2

	{
	enum States
		{
		stateStart,
		stateSingleDot,
		stateSingleSlash,
		stateComponent,
		stateFirstCharacter,
		};

	//	We use this buffer to build up the result

	char szCurrentDir[4096];

	//	Get the current path. NOTE: GetCurrentDirectory does not return
	//	a trailing backslash UNLESS the current directory is the root.

	const int BUFFER_SIZE = 4096;
	TCHAR szBuffer[BUFFER_SIZE];
	int iLen = ::GetCurrentDirectory(BUFFER_SIZE, szBuffer);

	//	Convert to UTF8 and copy to szCurrentDir

	CString sCurDir(szBuffer, iLen);
	char *pSrc = sCurDir.GetParsePointer();
	char *pDest = szCurrentDir;
	while (*pSrc != '\0')
		*pDest++ = *pSrc++;

	//	Append a backslash

	if (*(pDest - 1) != '\\')
		*pDest++ = '\\';

	//	We use szCurrentDir to build the resulting path.

	char *pPos = sFilespec.GetParsePointer();
	int iState = stateStart;
	while (*pPos != '\0')
		{
		switch (iState)
			{
			case stateStart:
				{
				//	If we have a leading backslash then this is
				//	either an absolute path or a UNC path.

				if (*pPos == '\\')
					iState = stateSingleSlash;

				//	A leading dot means either this directory or one
				//	directory up.

				else if (*pPos == '.')
					iState = stateSingleDot;

				//	A character means a drive letter or a directory/filename

				else
					{
					iState = stateFirstCharacter;
					*pDest++ = *pPos;
					}
				break;
				}

			case stateSingleSlash:
				{
				//	If we get a second slash then this is an absolute
				//	UNC path name.

				if (*pPos == '\\')
					return sFilespec;

				//	If we get character then this is an absolute path
				//	starting with the current drive.

				else
					{
					pDest = szCurrentDir;
					while (*pDest != '\\' && *pDest != '\0')
						pDest++;

					pPos = sFilespec.GetParsePointer();
					while (*pPos != '\0')
						*pDest++ = *pPos++;

					*pDest = '\0';
					return CString(szCurrentDir);
					}
				break;
				}

			case stateSingleDot:
				{
				//	If we get a second dot then this says that we
				//	should pop up one directory

				if (*pPos == '.')
					{
					pDest--;
					pDest--;
					while (*pDest != '\\')
						pDest--;

					pDest++;
					pPos++;		//	skip next backslash
					iState = stateStart;
					}

				//	Otherwise, a slash means that we are relative
				//	to the current directory.

				else if (*pPos == '\\')
					{
					iState = stateStart;
					}
				else
					ASSERT(false);
				break;
				}

			case stateFirstCharacter:
				{
				//	A colon means that we've got an absolute path

				if (*pPos == ':')
					return sFilespec;

				//	Otherwise, we've got a normal component

				else
					{
					*pDest++ = *pPos;
					iState = stateComponent;
					}
				break;
				}

			case stateComponent:
				{
				*pDest++ = *pPos;
				break;
				}
			}

		pPos++;
		}

	//	Done

	*pDest = '\0';
	return CString(szCurrentDir);
	}

CString fileGetDrive (const CString &sFilespec)

//	fileGetDrive
//
//	Returns the drive for the given filespec.

	{
	char *pPos = sFilespec.GetParsePointer();

	char *pStart = pPos;
	while (*pPos != ':' && *pPos != '\0')
		pPos++;

	if (*pPos == '\0')
		return NULL_STR;

	pPos++;

	return strPattern("%s\\", CString(pStart, pPos - pStart));
	}

void fileGetDriveSpace (const CString &sDrive, DWORDLONG *retdwAvailable, DWORDLONG *retdwTotalSize)

//	fileGetDriveSpace
//
//	Returns the amount of disk space available.

	{
	ULARGE_INTEGER liAvailable;
	ULARGE_INTEGER liTotalSize;

	if (!::GetDiskFreeSpaceEx(CString16(fileGetPath(sDrive)), &liAvailable, &liTotalSize, NULL))
		{
		liAvailable.QuadPart = 0;
		liTotalSize.QuadPart = 0;
		}

	//	Done

	if (retdwAvailable)
		*retdwAvailable = liAvailable.QuadPart;

	if (retdwTotalSize)
		*retdwTotalSize = liTotalSize.QuadPart;
	}

CString fileGetExecutableFilespec (void)

//	fileGetExecutableFilespec
//
//	Returns the path of the current executable file

	{
	//	Get the filespec

	TCHAR szBuffer[MAX_FILE_PATH_CHARS];
	int iLen = ::GetModuleFileName(NULL, szBuffer, MAX_FILE_PATH_CHARS);

	//	Convert to UTF-8

	return CString(szBuffer, iLen);
	}

CString fileGetExtension (const CString &sFilespec, CString *retsRemainder)

//	fileGetExtension
//
//	Returns an extension (including the leading dot). If the filespec
//	has no extension we return NULL_STR.

	{
	if (sFilespec.IsEmpty())
		{
		if (retsRemainder)
			*retsRemainder = sFilespec;
		return NULL_STR;
		}

	char *pPos = sFilespec.GetParsePointer();
	char *pEnd = pPos + sFilespec.GetLength() - 1;

	while (pEnd > pPos && *pEnd != '.')
		pEnd--;

	if (pEnd == pPos)
		{
		if (retsRemainder)
			*retsRemainder = sFilespec;
		return NULL_STR;
		}

	//	If we end in a dot, then strip it

	if (pEnd == pPos + sFilespec.GetLength() - 1)
		{
		if (retsRemainder)
			*retsRemainder = CString(pPos, sFilespec.GetLength() - 1);
		return NULL_STR;
		}

	//	Done

	if (retsRemainder)
		*retsRemainder = CString(pPos, (int)(pEnd - pPos));

	return CString(pEnd);
	}

bool fileGetFileList (const CString &sRoot, const CString &sPath, const CString &sSearch, DWORD dwFlags, TArray<CString> *retFiles)

//	fileGetFileList
//
//	Returns a list of filespecs that match the given filespec.
//
//	NOTE: We expect retFiles to be empty. [This function can recurse to add 
//	subdirectories, so we don't empty it ourselves.]

	{
	WIN32_FIND_DATA FileInfo;

	CString sFullPath = fileAppend(sRoot, sPath);
	CString sFilespec = fileAppend(sFullPath, sSearch);

	CString16 sFilespec16(sFilespec);
	HANDLE hFind = ::FindFirstFile(sFilespec16, &FileInfo);
	if (hFind == INVALID_HANDLE_VALUE)
		{
		if (::GetLastError() == ERROR_FILE_NOT_FOUND)
			return true;
		else
			return false;
		}

	bool bDirectoriesOnly = ((dwFlags & FFL_FLAG_DIRECTORIES_ONLY) ? true : false);

	do
		{
		CString sFound(FileInfo.cFileName, -1);

		//	Skip . and ..

		if (strEquals(sFound, STR_DOT) || strEquals(sFound, STR_DOT_DOT))
			continue;

		//	Skip system and hidden files

		if ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
				|| (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
			continue;

		//	Is this a directory?

		bool bIsDirectory = ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);

		//	Add to the list (but only if not a directory)

		if (bDirectoriesOnly == bIsDirectory)
			{
			if (dwFlags & FFL_FLAG_RELATIVE_FILESPEC)
				retFiles->Insert(fileAppend(sPath, sFound));
			else
				retFiles->Insert(fileAppend(sFullPath, sFound));
			}

		//	Recurse, if necessary

		if (bIsDirectory && (dwFlags & FFL_FLAG_RECURSIVE))
			{
			if (!fileGetFileList(sRoot, fileAppend(sPath, sFound), sSearch, dwFlags, retFiles))
				return false;
			}
		}
	while (::FindNextFile(hFind, &FileInfo));

	//	Done

	::FindClose(hFind);
	return true;
	}

CString fileGetFilename (const CString &sFilespec)

//	fileGetFilename
//
//	Returns the filename (without the path)

	{
	LPSTR pPos = sFilespec.GetParsePointer();
	LPSTR pStart = pPos;

	//	Keep looping

	while (*pPos != '\0')
		{
		if (*pPos == '\\' || *pPos == '/')
			pStart = pPos + 1;

		pPos++;
		}

	//	pStart points to the start of the filename

	return CString(pStart);
	}

CDateTime fileGetModifiedTime (const CString &sFilespec)

//	fileGetModifiedTime
//
//	Returns the time when the file was modified.

	{
	HANDLE hFile = ::CreateFile(CString16(sFilespec),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return CDateTime(CDateTime::BeginningOfTime);

	//	Get modified time

	FILETIME ftWrite;
	if (!::GetFileTime(hFile, NULL, NULL, &ftWrite))
		{
		::CloseHandle(hFile);
		return CDateTime(CDateTime::BeginningOfTime);
		}

	//	Convert

	SYSTEMTIME SystemTime;
	::FileTimeToSystemTime(&ftWrite, &SystemTime);

	//	Done

	::CloseHandle(hFile);
	return CDateTime(SystemTime);
	}

CString fileGetPath (const CString &sFilespec)

//	fileGetPath
//
//	Returns the path without the filename. We always include
//	a terminating '\' if possible.

	{
	char *pStart = sFilespec.GetParsePointer();
	char *pPos = pStart + sFilespec.GetLength() + 1;

	//	Look for the first backslash from the right

	while (pPos > pStart && *(pPos - 1) != '\\' && *(pPos - 1) != '/')
		pPos--;

	return CString(pStart, pPos - pStart);
	}

DWORDLONG fileGetSize (const CString &sFilespec)

//	fileGetSize
//
//	Returns the size of the file (in bytes)

	{
	HANDLE hFile = ::CreateFile(CString16(sFilespec),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	//	Get size

	LARGE_INTEGER liSize;
	if (!::GetFileSizeEx(hFile, &liSize))
		{
		::CloseHandle(hFile);
		return 0;
		}

	//	Done

	::CloseHandle(hFile);
	return liSize.QuadPart;
	}

CString fileGetSystemPath ()

//	fileGetSystemPath
//
//	Returns the c:\windows\system32 path.

	{
	TCHAR szBuffer[MAX_PATH+1];
	int iLen = ::GetSystemDirectoryW(szBuffer, MAX_PATH);
	return CString(CString16(szBuffer, iLen));
	}

CString fileGetTempPath (void)

//	fileGetTempPath
//
//	Returns the temp path for the current process

	{
	TCHAR szBuffer[1024];
	int iLen = ::GetTempPath(sizeof(szBuffer) / sizeof(szBuffer[0]), szBuffer);
	return CString(CString16(szBuffer, iLen));
	}

bool fileGetVersionInfo (const CString &sFilespec, SFileVersionInfo *retInfo, CString *retsError)

//	fileGetVersionInfo
//
//	Returns version information for the file (if sFilename is NULL_STRING then
//	we return information for the current module.)
//
//	Returns FALSE if we could not find the information

	{
	CString16 sPath = sFilespec;
	if (sPath.IsEmpty())
		{
		TCHAR szBuffer[1024];
		int iLen = ::GetModuleFileName(NULL, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
		sPath = CString16(szBuffer, iLen);
		}

	//	Initialize

	retInfo->dwFileVersion = 0;
	retInfo->dwProductVersion = 0;

	//	Figure out how big the version information is

	DWORD dwDummy;
	DWORD dwSize = ::GetFileVersionInfoSize(sPath, &dwDummy);
	if (dwSize == 0)
		{
		if (retsError) *retsError = strPattern("No version info found: %x", ::GetLastError());
		return false;
		}

	//	Load the info

	CString sData((char *)NULL, dwSize);
	if (!::GetFileVersionInfo(sPath, 0, dwSize, (char *)sData))
		{
		if (retsError) *retsError = strPattern("Error in GetFileVersionInfo: %x", ::GetLastError());
		return false;
		}

	//	Get the fixed-size portion

	VS_FIXEDFILEINFO *pFileInfo;
	DWORD dwFileInfoSize;
	if (::VerQueryValue(sData.GetParsePointer(), _T("\\"), (LPVOID *)&pFileInfo, (PUINT)&dwFileInfoSize))
		{
		retInfo->dwFileVersion = ((DWORDLONG)pFileInfo->dwFileDateMS << 32) | (DWORDLONG)pFileInfo->dwFileDateLS;
		retInfo->dwProductVersion = ((DWORDLONG)pFileInfo->dwProductVersionMS << 32) | (DWORDLONG)pFileInfo->dwProductVersionLS;
		}

	//	Get language information

	WORD *pLangInfo;
	DWORD dwLangInfoSize;
	::VerQueryValue(sData.GetParsePointer(), _T("\\VarFileInfo\\Translation"), (LPVOID *)&pLangInfo, (PUINT)&dwLangInfoSize);

	//	Get the strings

	retInfo->sCompanyName = GetVersionString(sData.GetParsePointer(), pLangInfo, STR_COMPANY_NAME);
	retInfo->sCopyright = GetVersionString(sData.GetParsePointer(), pLangInfo, STR_COPYRIGHT);
	retInfo->sProductName = GetVersionString(sData.GetParsePointer(), pLangInfo, STR_PRODUCT_NAME);
	retInfo->sProductVersion = GetVersionString(sData.GetParsePointer(), pLangInfo, STR_PRODUCT_VERSION);

	return true;
	}

bool fileGetVolumeList (DWORD dwFlags, TArray<CString> *retVolumes)

//	fileGetVolumeList
//
//	Returns a list of volumes on this computer.

	{
	//	LATER
	ASSERT(false);
	return false;
	}

CString fileGetWindowsPath ()

//	fileGetWindowsPath
//
//	Returns the c:\windows path.

	{
	TCHAR szBuffer[MAX_PATH+1];
	int iLen = ::GetWindowsDirectoryW(szBuffer, MAX_PATH);
	return CString(CString16(szBuffer, iLen));
	}

bool fileIsAbsolute (const CString &sFilespec)

//	fileIsAbsolute
//
//	Returns TRUE if the filespec starts with a backslash
//	or has a drive letter.

	{
	if (sFilespec.IsEmpty())
		return false;

	char *pPos = sFilespec.GetParsePointer();
	if (*pPos == '\\' || *pPos == '/')
		return true;

	if (pPos[1] == ':')
		return true;

	//	Not absolute

	return false;
	}

bool fileIsDotted (const CString &sFilespec)

//	fileIsDotted
//
//	Returns TRUE if the filespec has embedded .. syntax
//	(which goes up a directory)

	{
	char *pPos = sFilespec.GetParsePointer();
	bool bFoundDot = false;

	while (*pPos != '\0')
		{
		if (*pPos == '.')
			{
			if (bFoundDot)
				return true;

			bFoundDot = true;
			}
		else
			bFoundDot = false;

		pPos++;
		}

	return false;
	}

bool fileIsFilename (const CString &sFilespec)

//	fileIsFilename
//
//	Returns TRUE if the given filespec has an extension (or even a dot with no
//	extension).

	{
	const char *pStart = sFilespec.GetParsePointer();
	const char *pPos = pStart + sFilespec.GetLength() - 1;

	while (pPos >= pStart && *pPos != '.' && *pPos != '/' && *pPos != '\\')
		pPos--;

	return (pPos >= pStart && *pPos == '.');
	}

bool fileIsPathEqual (const CString &sFilespec1, const CString &sFilespec2)

//	fileIsPathEqual
//
//	Returns TRUE if the two filespecs are the same. Handles / vs. \
//	and lowercase/uppercase.

	{
	char *pPos1 = (LPSTR)sFilespec1;
	char *pEndPos1 = pPos1 + sFilespec1.GetLength();
	char *pPos2 = (LPSTR)sFilespec2;
	char *pEndPos2 = pPos2 + sFilespec2.GetLength();
	while (pPos1 < pEndPos1 && pPos2 < pEndPos2)
		{
		UTF32 dwCodePoint1 = strParseUTF8Char(&pPos1, pEndPos1);
		UTF32 dwLower1 = strToLowerChar(dwCodePoint1);

		UTF32 dwCodePoint2 = strParseUTF8Char(&pPos2, pEndPos2);
		UTF32 dwLower2 = strToLowerChar(dwCodePoint2);

		//	If both are slashes (backwards or forwards) then we're OK

		if ((dwLower1 == '/' || dwLower1 == '\\')
				&& (dwLower2 == '/' || dwLower2 == '\\'))
			continue;

		//	Otherwise, they both must be the same

		if (dwLower1 != dwLower2)
			return false;
		}

	return (pPos1 == pEndPos1 && pPos2 == pEndPos2);
	}

bool fileIsWildcard (const CString &sFilespec)

//	fileIsWildcard
//
//	Returns TRUE if the filespec has a wildcard.

	{
	const char *pPos = sFilespec.GetParsePointer();
	const char *pPosEnd = pPos + sFilespec.GetLength();

	while (pPos < pPosEnd)
		{
		if (*pPos == '?' || *pPos == '*')
			return true;

		pPos++;
		}

	return false;
	}

bool fileMove (const CString &sSourceFilespec, const CString &sDestFilespec)

//	fileMove
//
//	Moves a file

	{
	if (!::MoveFile(CString16(sSourceFilespec), CString16(sDestFilespec)))
		return false;

	return true;
	}

bool filePathCreate (const CString &sPath, CString *retsError)

//	filePathCreate
//
//	Makes sure that the given path exists. Creates all intermediate folders.

	{
	CString sTest = sPath;
	char *pPos = sTest.GetParsePointer();

	//	Make sure the path exists from the top down.

	while (*pPos != '\0')
		{
		//	Skip over this backslash

		while (*pPos == '\\' && *pPos != '0')
			pPos++;

		//	Skip to the next backslash

		while (*pPos != '\\' && *pPos != '\0')
			pPos++;

		//	Trim the path here and see if it exists so far

		char chSaved = *pPos;
		*pPos = '\0';
		
		//	If the path doesn't exist, create it.

		if (!fileExists(sTest))
			{
			if (!::CreateDirectory(CString16(sTest), NULL))
				{
				if (retsError)
					*retsError = strPattern(ERR_CANT_CREATE_DIRECTORY, sTest, ::GetLastError());

				return false;
				}
			}

		*pPos = chSaved;
		}

	return true;
	}

bool filePathDelete (const CString &sPath, DWORD dwFlags)

//	filePathDelete
//
//	Deletes the empty directory

	{
	int i;
	bool bSuccess = true;

	//	See if we need to remove the directory recursively before
	//	we delete the root.

	if (dwFlags & FPD_FLAG_RECURSIVE)
		{
		//	First we recursively delete all directories

		TArray<CString> Dirs;
		fileGetFileList(fileAppend(sPath, STR_STAR), FFL_FLAG_DIRECTORIES_ONLY, &Dirs);
		for (i = 0; i < Dirs.GetCount(); i++)
			{
			if (!filePathDelete(Dirs[i], dwFlags))
				bSuccess = false;
			}

		//	Next we delete all files

		TArray<CString> Files;
		fileGetFileList(fileAppend(sPath, STR_STAR), 0, &Files);
		for (i = 0; i < Files.GetCount(); i++)
			{
			if (!fileDelete(Files[i]))
				bSuccess = false;
			}

		//	Now we delete the root directory.
		}

	//	Remove the single directory (it must be empty or this function fails).

	if (!(dwFlags & FPD_FLAG_CONTENT_ONLY))
		if (!::RemoveDirectory(CString16(sPath)))
			return false;

	//	Done

	return bSuccess;
	}

CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString)
	{
	CString16 sQuery = strPattern("\\StringFileInfo\\%04x%04x\\%s", pLangInfo[0], pLangInfo[1], sString);

	TCHAR *pPos;
	DWORD dwSize;
	if (::VerQueryValue(pData, sQuery, (LPVOID *)&pPos, (PUINT)&dwSize))
		return CString16(pPos, (int)dwSize-1);
	else
		return NULL_STR;
	}

CString fileGetWorkingDirectory (void)

//	fileGetWorkingDirector
//
//	Returns the current directory.

	{
	TCHAR szBuffer[MAX_FILE_PATH_CHARS];
	int iLen = ::GetCurrentDirectory(MAX_FILE_PATH_CHARS, szBuffer);

	//	Convert to UTF-8

	return CString(szBuffer, iLen);
	}

bool fileSetWorkingDirectory (const CString &sPath)

//	fileSetWorkingDirectory
//
//	Sets the current working directory for the process.

	{
	if (!::SetCurrentDirectory(CString16(sPath)))
		return false;

	return true;
	}
