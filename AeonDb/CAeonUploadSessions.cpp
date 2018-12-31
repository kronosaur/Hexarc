//	CAeonUploadSessions.cpp
//
//	CAeonUploadSessions class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc")
DECLARE_CONST_STRING(FIELD_PARTIAL_POS,					"partialPos")
DECLARE_CONST_STRING(FIELD_UPLOAD_LENGTH,				"uploadSize")
DECLARE_CONST_STRING(FIELD_UPLOAD_POS,					"uploadPos")
DECLARE_CONST_STRING(FIELD_VERSION,						"version")

DECLARE_CONST_STRING(FILESPEC_TEMP_FILE_CONS,			"%s\\files\\%04x%04x_%08x.afile")

DECLARE_CONST_STRING(STR_ERROR_INSUFFICIENT_DATA,		"Insufficient data for file upload: %s.")
DECLARE_CONST_STRING(STR_ERROR_CANT_CREATE_TEMP_FILE,	"Unable to create temp file.")
DECLARE_CONST_STRING(STR_ERROR_WRITING_TEMP_FILE,		"Unable to write to temp file: %s.")
DECLARE_CONST_STRING(STR_ERROR_INVALID_PARTIAL,			"Invalid partial upload: %s.")
DECLARE_CONST_STRING(ERR_NO_UPLOAD_TARGET,				"uploadPos specified, but original file does not exist: %s.")

const DWORD SESSION_TIMEOUT =							15 * 60 * 1000;

CAeonUploadSessions::~CAeonUploadSessions (void)

//	CAeonUploadSessions destructor

	{
	int i;

	for (i = 0; i < m_Sessions.GetCount(); i++)
		delete m_Sessions[i];
	}

void CAeonUploadSessions::AbortUpload (const SReceipt &Receipt)

//	AbortUpload
//
//	Aborts an upload

	{
	if (Receipt.bNewFile)
		{
		if (!fileDelete(Receipt.sFilespec))
			{
			//	LATER: Log?
			}
		}
	}

int CAeonUploadSessions::CalcUploadCompletion (SUploadSessionCtx *pSession)

//	CalcUploadCompletion
//
//	Calculates what percent of upload is done

	{
	if (pSession->iUploadLen == 0)
		return 100;

	int i;
	int iTotalNeeded = 0;
	for (i = 0; i < pSession->DataExpected.GetCount(); i++)
		iTotalNeeded += pSession->DataExpected[i].iLength;

	return 100 * (pSession->iUploadLen - iTotalNeeded) / pSession->iUploadLen;
	}

bool CAeonUploadSessions::CreateTempFile (CString *retsFilespec, CFileMultiplexer *retFile, CString *retsError)

//	CreateTempFile
//
//	Creates a temporary file for the upload

	{
	CString sFilespec;
	CString sVolumePath = m_pStorage->GetPath(m_sPrimaryVolume);
	CString sBackupVolumePath = (!m_sBackupVolume.IsEmpty() ? m_pStorage->GetPath(m_sBackupVolume) : NULL_STR);

	bool bSuccess = false;
	int iTries = 0;
	do
		{
		iTries++;

		//	Pick a random ID for the file

		CDateTime Now(CDateTime::Now);
		DWORD dwID1 = (DWORD)(Now.DaysSince1AD() - (2000 * 365));
		DWORD dwID2 = mathRandom(1, 65535);
		DWORD dwID3 = (DWORD)Now.MillisecondsSinceMidnight();

		//	Attempt to create a file of that name

		CString sFilename = strPattern(FILESPEC_TEMP_FILE_CONS, m_sTableName,  dwID1, dwID2, dwID3);
		sFilespec = fileAppend(sVolumePath, sFilename);
		bSuccess = retFile->Create(sFilespec, CFile::FLAG_CREATE_NEW);

		//	Create a backup file

		if (!sBackupVolumePath.IsEmpty())
			retFile->CreateMirror(fileAppend(sBackupVolumePath, sFilename));
		}
	while (!bSuccess && iTries < 20);

	//	Done

	if (!bSuccess)
		{
		if (retsError)
			*retsError = STR_ERROR_CANT_CREATE_TEMP_FILE;
		return false;
		}

	if (retsFilespec)
		*retsFilespec = sFilespec;

	return true;
	}

void CAeonUploadSessions::Delete (const CString &sSessionID)

//	Delete
//
//	Delete the session

	{
	CSmartLock Lock(m_cs);

	int iPos;
	if (m_Sessions.FindPos(sSessionID, &iPos))
		{
		delete m_Sessions[iPos];
		m_Sessions.Delete(iPos);
		}
	}

void CAeonUploadSessions::DeleteTempFile (SUploadSessionCtx *pSession, CFileMultiplexer *pFile)

//	DeleteTempFile
//
//	Deletes the temp file

	{
	if (pSession->bTempFileCreated)
		{
		if (!pFile->Delete())
			{
			//	LATER: Log error
			}
		}
	else
		pFile->Close();
	}

void CAeonUploadSessions::DeleteTempFile (SUploadSessionCtx *pSession)

//	DeleteTempFile
//
//	Deletes the temp file

	{
	if (pSession->bTempFileCreated)
		{
		if (!fileDelete(pSession->sTempFilespec))
			{
			//	LATER: Log
			}
		}
	}

void CAeonUploadSessions::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Sessions.GetCount(); i++)
		m_Sessions[i]->dFileDesc.Mark();

	//	While we're at it, clean any sessions that have expired

	DWORD dwNow = sysGetTickCount();
	for (i = 0; i < m_Sessions.GetCount(); i++)
		{
		if (m_Sessions[i]->dwLastActivity + SESSION_TIMEOUT < dwNow)
			{
			DeleteTempFile(m_Sessions[i]);

			delete m_Sessions[i];
			m_Sessions.Delete(i);

			i--;
			}
		}
	}

bool CAeonUploadSessions::ProcessUpload (CMsgProcessCtx &Ctx,
                                         const CString &sSessionID, 
										 const CString &sFilePath, 
										 CDatum dUploadDesc, 
										 CDatum dData, 
										 const CString &sCurrentFilespec,
										 SReceipt *retReceipt)

//	ProcessUpload
//
//	Constructs an upload file from multiple calls.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Sometimes we have to copy data from the existing file

	CFile CurrentFile;
	int CopySeg[2];
	int CopySegLen[2];
	for (i = 0; i < 2; i++)
		{
		CopySeg[i] = 0;
		CopySegLen[i] = 0;
		}

	//	Do we have a session? If not, we need to create one

	SUploadSessionCtx *pSession;
	CFileMultiplexer TempFile;
	CString sTempFilespec;
	int iTempFileOffset = 0;
	int iUploadPos = 0;

	if (!m_Sessions.Find(sSessionID, &pSession))
		{
		pSession = new SUploadSessionCtx;
		pSession->sSessionID = sSessionID;
		pSession->dwLastActivity = sysGetTickCount();

		//	Get basic data about the upload

		pSession->sFilePath = sFilePath;
		CDatum dVersion = dUploadDesc.GetElement(FIELD_VERSION);
		pSession->dwVersion = (dVersion.IsNil() ? 0xFFFFFFFF : (DWORD)(int)dVersion);
		pSession->dFileDesc = dUploadDesc.GetElement(FIELD_FILE_DESC);

		//	Set the data expected

		pSession->iUploadLen = Max(dData.GetBinarySize(), (int)dUploadDesc.GetElement(FIELD_UPLOAD_LENGTH));
		SFileRange *pRange = pSession->DataExpected.Insert();
		pRange->iPos = 0;
		pRange->iLength = pSession->iUploadLen;

		//	Figure out where we upload to

		CDatum dUploadPos = dUploadDesc.GetElement(FIELD_UPLOAD_POS);
		int iUploadPos = (int)dUploadPos;

		//	If we have an upload position then we must have an existing file.

		if (!dUploadPos.IsNil() && sCurrentFilespec.IsEmpty())
			{
			delete pSession;
			retReceipt->iComplete = 0;
			retReceipt->sError = strPattern(ERR_NO_UPLOAD_TARGET, sFilePath);
			return false;
			}

		//	If we are appending to the end, then we open the existing file
		//
		//	This section initializes:
		//
		//	pSession->bTempFileCreated
		//	pSession->sTempFilespec
		//	pSession->iTempFileOffset
		//
		//	We also cache the following values on the stack so that
		//	we can do some processing without a lock:
		//
		//	sTempFilespec
		//	iTempFileOffset

		if (iUploadPos == -1)
			{
			sTempFilespec = sCurrentFilespec;
			if (!TempFile.Create(sTempFilespec, 0))
				{
				delete pSession;
				retReceipt->iComplete = 0;
				retReceipt->sError = STR_ERROR_CANT_CREATE_TEMP_FILE;
				return false;
				}

			pSession->bTempFileCreated = false;
			pSession->sTempFilespec = sTempFilespec;

			iTempFileOffset = TempFile.GetStreamLength();
			pSession->iTempFileOffset = iTempFileOffset;

			//	Compute the final size

			pSession->iTempFileSize = iTempFileOffset + pSession->iUploadLen;

			//	Create a backup file, if necessary

			if (!m_sBackupVolume.IsEmpty())
				{
				CString sFilespec = m_pStorage->MachineToCanonicalRelative(sTempFilespec);
				if (!TempFile.CreateMirror(m_pStorage->CanonicalRelativeToMachine(m_sBackupVolume, sFilespec)))
					{
					//	LATER: Log error
					}
				}
			}

		//	Otherwise, if we're applying the upload on top of the existing file,
		//	then we need to copy the existing file.

		else if (!dUploadPos.IsNil())
			{
			//	Create a temporary file to store the upload

			if (!CreateTempFile(&sTempFilespec, &TempFile, &retReceipt->sError))
				{
				delete pSession;
				retReceipt->iComplete = 0;
				return false;
				}

			pSession->bTempFileCreated = true;
			pSession->sTempFilespec = sTempFilespec;

			//	Open up the existing file and make a copy of the relevant areas
			//	to the temporary file.

			if (!CurrentFile.Create(sCurrentFilespec, CFile::FLAG_OPEN_READ_ONLY))
				{
				DeleteTempFile(pSession, &TempFile);
				delete pSession;
				retReceipt->iComplete = 0;
				retReceipt->sError = STR_ERROR_CANT_CREATE_TEMP_FILE;
				return false;
				}

			//	Copy stuff before the upload portion

			if (iUploadPos > 0)
				{
				CopySeg[0] = 0;
				CopySegLen[0] = iUploadPos;
				}

			//	Copy stuff after the upload portion

			int iCurrentFileLen = CurrentFile.GetStreamLength();
			if (iUploadPos + pSession->iUploadLen < iCurrentFileLen)
				{
				CopySeg[1] = iUploadPos + pSession->iUploadLen;
				CopySegLen[1] = iCurrentFileLen - (iUploadPos + pSession->iUploadLen);
				}

			iTempFileOffset = iUploadPos;
			pSession->iTempFileOffset = iTempFileOffset;

			//	Compute size

			pSession->iTempFileSize = Max(iCurrentFileLen, iUploadPos + pSession->iUploadLen);
			}

		//	Otherwise, we just create a temp file

		else
			{
			//	Create a temporary file to store the upload

			if (!CreateTempFile(&sTempFilespec, &TempFile, &retReceipt->sError))
				{
				delete pSession;
				retReceipt->iComplete = 0;
				return false;
				}

			pSession->bTempFileCreated = true;
			pSession->sTempFilespec = sTempFilespec;

			iTempFileOffset = 0;
			pSession->iTempFileOffset = iTempFileOffset;

			//	The size of the final file is the same as the total upload size

			pSession->iTempFileSize = pSession->iUploadLen;
			}

		//	Add the session to our map

		m_Sessions.Insert(sSessionID, pSession);
		}
	else
		{
		//	Open temp file

		sTempFilespec = pSession->sTempFilespec;
		iTempFileOffset = pSession->iTempFileOffset;
		if (!TempFile.Create(sTempFilespec, 0))
			{
			DeleteTempFile(pSession);
			Delete(sSessionID);
			retReceipt->iComplete = 0;
			retReceipt->sError = STR_ERROR_CANT_CREATE_TEMP_FILE;
			return false;
			}

		//	Open the backup file too

		if (!m_sBackupVolume.IsEmpty())
			{
			CString sFilespec = m_pStorage->MachineToCanonicalRelative(sTempFilespec);
			if (!TempFile.CreateMirror(m_pStorage->CanonicalRelativeToMachine(m_sBackupVolume, sFilespec)))
				{
				//	LATER: Log error
				}
			}
		}

	//	See how much we have in this upload

	CDatum dPartialPos = dUploadDesc.GetElement(FIELD_PARTIAL_POS);
	int iPartialPos = (dPartialPos.IsNil() ? 0 : (int)dPartialPos);

	int iPartialLen = dData.GetBinarySize();

	//	Unlock while we write out the data
	//	This allows other threads to write out data to other files (or even
	//	the same file).

	Lock.Unlock();

	//	Write the data to the file

	bool bSuccess;
	try
		{
        //  Create an object which will send keep-alive notifications.

        CMsgProcessKeepAlive KeepAlive(Ctx);

		//	Copy from the original, if necessary

		for (i = 0; i < 2; i++)
			if (CopySegLen[i])
				{
				char *pBuffer = new char [CopySegLen[i]];

				CurrentFile.Seek(CopySeg[i]);
				CurrentFile.Read(pBuffer, CopySegLen[i]);

				TempFile.Seek(CopySeg[i]);
				TempFile.Write(pBuffer, CopySegLen[i]);

				delete [] pBuffer;
				}

		//	Copy the uploaded data

		TempFile.Seek(iTempFileOffset + iPartialPos);
		dData.WriteBinaryToStream(TempFile, 0, -1, &KeepAlive);
		bSuccess = true;
		}
	catch (...)
		{
		bSuccess = false;
		}

	//	After writing we need to reacquire the lock and look up session
	//	again (since it could have been trashed).

	Lock.Lock();
	if (!m_Sessions.Find(sSessionID, &pSession))
		{
		//	If we could not find the session then it means that some other thread
		//	failed while writing. We abort.

		retReceipt->iComplete = 0;
		retReceipt->sError = strPattern(STR_ERROR_WRITING_TEMP_FILE, sFilePath);
		return false;
		}

	//	Check to see if we failed the write

	if (!bSuccess)
		{
		DeleteTempFile(pSession, &TempFile);
		Delete(sSessionID);
		retReceipt->iComplete = 0;
		retReceipt->sError = strPattern(STR_ERROR_WRITING_TEMP_FILE, sFilePath);
		return false;
		}

	//	Update the ranges

	bool bFound = false;
	for (i = 0; i < pSession->DataExpected.GetCount(); i++)
		{
		SFileRange &Range = pSession->DataExpected[i];
		if (iPartialPos < Range.iPos + Range.iLength)
			{
			//	We better fit within the block

			if (iPartialPos + iPartialLen > Range.iPos + Range.iLength)
				{
				DeleteTempFile(pSession, &TempFile);
				Delete(sSessionID);
				retReceipt->iComplete = 0;
				retReceipt->sError = strPattern(STR_ERROR_INVALID_PARTIAL, sFilePath);
				return false;
				}

			//	If we completely subsume this range, then delete the range.
			//	NOTE: We can never cover more than one range because all uploads are contiguous.

			else if (iPartialPos == Range.iPos && iPartialLen == Range.iLength)
				pSession->DataExpected.Delete(i);

			//	If we're at the beginning, then we need to shift the block

			else if (iPartialPos == Range.iPos)
				{
				Range.iPos += iPartialLen;
				Range.iLength -= iPartialLen;
				}

			//	If we're at the end, then we need to shift the block

			else if (iPartialPos + iPartialLen == Range.iPos + Range.iLength)
				{
				Range.iLength -= iPartialLen;
				}

			//	Otherwise we are in the middle

			else
				{
				SFileRange *pNewRange = pSession->DataExpected.InsertAt(i + 1);
				pNewRange->iPos = iPartialPos + iPartialLen;
				pNewRange->iLength = Range.iPos + Range.iLength - pNewRange->iPos;

				Range.iLength = iPartialPos - Range.iPos;
				}

			bFound = true;
			break;
			}
		}

	if (!bFound)
		{
		DeleteTempFile(pSession, &TempFile);
		Delete(sSessionID);
		retReceipt->iComplete = 0;
		retReceipt->sError = strPattern(STR_ERROR_INVALID_PARTIAL, sFilePath);
		return false;
		}

	//	See if the upload is complete

	if (pSession->DataExpected.GetCount() == 0)
		{
		//	Make sure we flush the temp file before we return.

		TempFile.Flush();

		//	Return the result

		*retReceipt = SReceipt();
		retReceipt->iComplete = 100;
		retReceipt->sFilePath = pSession->sFilePath;
		retReceipt->dwVersion = pSession->dwVersion;
		retReceipt->dFileDesc = pSession->dFileDesc;
		retReceipt->iFileSize = pSession->iTempFileSize;
		retReceipt->bNewFile = pSession->bTempFileCreated;
		retReceipt->sFilespec = sTempFilespec;

		//	We no longer need the session

		Delete(sSessionID);
		}
	
	//	Otherwise, we need more data

	else
		{
		*retReceipt = SReceipt();
		retReceipt->iComplete = CalcUploadCompletion(pSession);
		retReceipt->sFilePath = sFilePath;
		pSession->dwLastActivity = sysGetTickCount();
		}

	//	Done

	return true;
	}

void CAeonUploadSessions::SetVolumes (const CString &sPrimaryVolume, const CString &sBackupVolume)

//	SetVolumes
//
//	Sets volumes to use.

	{
	CSmartLock Lock(m_cs);
	m_sPrimaryVolume = sPrimaryVolume;
	m_sBackupVolume = sBackupVolume;
	}
