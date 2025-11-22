//	CAeonTable.cpp
//
//	CAeonTable class
//	Copyright (c) 2018 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FILESPEC_RECOVERY_DIR,				"recovery");

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");

DECLARE_CONST_STRING(OP_UPDATING_VIEW,					"updating a view");

DECLARE_CONST_STRING(STR_BACKING_UP,					"Table %s: Backing up to: %s.");
DECLARE_CONST_STRING(STR_BACKUP_COMPLETE,				"Table %s: Backup complete.");
DECLARE_CONST_STRING(STR_MERGE_COMPLETE,				"Table %s: Segment merge complete.");
DECLARE_CONST_STRING(STR_UPDATING_VIEW,					"Updating view %s in table %s.");
DECLARE_CONST_STRING(STR_VIEW_UPDATED,					"View update complete: view %s in table %s.");

DECLARE_CONST_STRING(ERR_MERGING_SEGMENTS,				"Merging segments in table %s: %s (%d rows) with %s (%d rows).");
DECLARE_CONST_STRING(ERR_MERGE_TOO_BIG,					"Segment merge exceeds size limits: %s and %s.");
DECLARE_CONST_STRING(ERR_NOT_ENOUGH_SPACE_TO_MERGE,		"Insufficient disk space to merge segments: %s and %s.");
DECLARE_CONST_STRING(ERR_INVALID_BACKUP,				"Table %s: Backup volume %s is invalid: %s");
DECLARE_CONST_STRING(ERR_SEGMENT_BACKUP_FAILED,			"Unable to create backup for new segment: %s.");
DECLARE_CONST_STRING(ERR_CANT_DELETE_FILE,				"Unable to delete file: %s.");
DECLARE_CONST_STRING(ERR_UPDATE_VIEW_ABORTED,			"Unable to update view %s in table %s.");
DECLARE_CONST_STRING(ERR_DELETING_EXTRA_SEGMENT_FILE,	"Table %s: Deleting extraneous segment file: %s.");
DECLARE_CONST_STRING(ERR_SEGMENT_MERGE_FAILED,			"Failed merging segments %s and %s: %s.");

bool CAeonTable::HousekeepingBackup (CSmartLock &Lock)

//	HousekeepingBackup
//
//	Backs up the table. Returns FALSE if we got an error.

	{
	CString sError;

	m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_BACKING_UP, m_sName, m_sBackupVolume));

	//	Set state so we don't do anything else until we're done.

	m_iHousekeeping = stateBackup;

	//	Copy from primary to backup. We do it outside the lock so that we
	//	can continue reading and such.

	CString sFrom = m_sPrimaryVolume;
	CString sTo = m_sBackupVolume;

	Lock.Unlock();

	if (!CopyVolume(sFrom, sTo, &sError))
		{
		//	LATER: Need to figure something out.
		m_pProcess->Log(MSG_LOG_ERROR, sError);
		m_iHousekeeping = stateReady;
		return false;
		}

	Lock.Lock();

	//	Restore complete

	m_bBackupNeeded = false;
	m_bBackupLost = false;

	//	Save new descriptor

	if (!SaveDesc())
		{
		//	Only fails if we've lost all data, so there is nothing we can
		//	do.
		m_iHousekeeping = stateReady;
		return false;
		}

	//	Done

	m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_BACKUP_COMPLETE, m_sName));
	m_iHousekeeping = stateReady;
	return true;
	}

bool CAeonTable::HousekeepingMergeSegments (CSmartLock &Lock)

//	HousekeepingMergeSegments
//
//	Merges segments if we can. Returns FALSE if we get an error.

	{
	int i;
	CString sError;

	//	If we have a lot of segments then we should consolidate a couple.
	//	We loop over all views until we find segments to merge.
	//
	//	Set a flag so a different thread doesn't try to merge the same
	//	segments.

	m_iHousekeeping = stateMergingSegments;

	//	Loop

	for (i = 0; i < m_Views.GetCount(); i++)
		{
		CAeonSegment *pSeg1;
		CAeonSegment *pSeg2;

		if (m_Views[i].GetSegmentsToMerge(&pSeg1, &pSeg2))
			{
			CString sSeg1File = m_pStorage->MachineToCanonicalRelative(pSeg1->GetFilespec());
			CString sSeg2File = m_pStorage->MachineToCanonicalRelative(pSeg2->GetFilespec());

			//	Log

			m_pProcess->Log(MSG_LOG_INFO, strPattern(ERR_MERGING_SEGMENTS, m_sName, sSeg1File, pSeg1->GetCount(), sSeg2File, pSeg2->GetCount()));

			//	If the resulting segment is too big, then we can't do anything

			DWORDLONG dwSeg1Size = pSeg1->GetFileSize();
			DWORDLONG dwSeg2Size = pSeg2->GetFileSize();
			if (dwSeg1Size + dwSeg2Size > GIGABYTE_DISK)
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_MERGE_TOO_BIG, sSeg1File, sSeg2File));
				continue;	//	Loop to the next view.
				}

			//	See if we have enough disk space.

			DWORDLONG dwAvailable;
			fileGetDriveSpace(pSeg1->GetFilespec(), &dwAvailable);
			if (dwAvailable < dwSeg1Size + dwSeg2Size)
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_NOT_ENOUGH_SPACE_TO_MERGE, sSeg1File, sSeg2File));
				continue;	//	Loop to the next view.
				}

			//	Create an iterator that will traverse both segments in order.

			CRowIterator Rows;
			Rows.Init(m_Views[i].GetDimensions());
			Rows.AddSegment(pSeg1);
			Rows.AddSegment(pSeg2);

			//	Get the name of the new segment

			CString sBackup;
			bool bBackupFailed = false;

			CString sFilespec = GetUniqueSegmentFilespec(&sBackup);

			//	Get the new sequence number (we always use the first segment
			//	because it is most recent).

			SEQUENCENUMBER NewSeq = pSeg1->GetSequence();

			//	Merge the segments. We do this outside the lock because no other
			//	thread should try to merge segments (because of the flag) and
			//	because existing segments are not modified.

			Lock.Unlock();

			//	Create the segment

			DWORD dwSegFlags = 0;
			dwSegFlags |= (i == 0 ? CAeonSegment::FLAG_HAS_ROW_ID : 0);
			dwSegFlags |= (i != 0 ? CAeonSegment::FLAG_SECONDARY_VIEW : 0);
			dwSegFlags |= CAeonSegment::CREATE_OPTION_DEBUG_ORDER;

			CAeonSegment *pNewSeg = new CAeonSegment;
			if (!pNewSeg->Create(m_Views[i].GetID(), m_Views[i].GetDimensions(), NewSeq, Rows, sFilespec, dwSegFlags, &sError))
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_SEGMENT_MERGE_FAILED, pSeg1->GetFilespec(), pSeg2->GetFilespec(), sError));
				pNewSeg->Release();
				break;	//	Stop looping over views.
				}

			//	Make a backup

			if (!m_bBackupLost)
				{
				if (!fileCopy(sFilespec, sBackup))
					bBackupFailed = true;
				}

			Lock.Lock();

			//	Replace the segments in the view's data structure

			m_Views[i].SegmentMergeComplete(pSeg1, pSeg2, pNewSeg);

			//	If the backup failed we need to recover

			if (bBackupFailed)
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_SEGMENT_BACKUP_FAILED, sBackup));

				//	LATER:
				}

			//	Otherwise, delete the old segment backups

			else if (!m_bBackupLost)
				{
				if (!fileDelete(m_pStorage->CanonicalRelativeToMachine(m_sBackupVolume, sSeg1File)))
					m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_DELETE_FILE, m_pStorage->CanonicalRelativeToMachine(m_sBackupVolume, sSeg1File)));

				if (!fileDelete(m_pStorage->CanonicalRelativeToMachine(m_sBackupVolume, sSeg2File)))
					m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_DELETE_FILE, m_pStorage->CanonicalRelativeToMachine(m_sBackupVolume, sSeg2File)));
				}

			//	Done. Wait for next housekeeping event to do more.

			m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_MERGE_COMPLETE, m_sName));
			break;
			}
		}

	//	Done merging

	m_iHousekeeping = stateReady;

	return true;
	}

bool CAeonTable::HousekeepingUpdateView (CSmartLock &Lock, DWORD dwViewID)

//	HousekeepingUpdateView
//
//	Updates the given view. Returns FALSE if we got an error.

	{
	int i;
	CString sError;
	CAeonView *pPrimaryView = m_Views.GetAt(DEFAULT_VIEW);
	CAeonView *pSecondaryView = m_Views.GetAt(dwViewID);

	m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_UPDATING_VIEW, pSecondaryView->GetName(), m_sName));
	m_iHousekeeping = stateUpdatingView;

	//	Create an iterator to go over all segments in the primary view
	//	(we go through rows later). NOTE: Since we are in housekeeping
	//	we can guarantee that no new segments will get created while
	//	we process.

	CRowIterator Rows;
	pPrimaryView->InitIterator(&Rows, CAeonView::FLAG_EXCLUDE_MEMORY_ROWS);
	CTableDimensions PrimaryDims = pPrimaryView->GetDimensions();

	//	We generate secondary view records from each row and save them
	//	to a separate (in memory) array

	CAeonRowArray SecondaryRows;
	SecondaryRows.Init(pSecondaryView->GetDimensions());

	//	Keep track of segments we've created

	TArray<SNewSegment> NewSegments;

	//	We assign arbitrary sequence numbers to the new segments. Only the
	//	relative numbers matter (seqments superceed others). Though in this
	//	case even that doesn't matter since we're looping over all segments
	//	at the same time.

	SEQUENCENUMBER SegSeq = 1;

	//	Create a process for evaluation. We create our own process instead 
	//	of using the table process because we're doing this outside a lock.

	CHexeProcess Process;
	if (!Process.LoadStandardLibraries(&sError))
		m_pProcess->Log(MSG_LOG_ERROR, sError);

	//	We loop outside the lock because the segments aren't going 
	//	anywhere.

	Lock.Unlock();

	//	Keep looping until we've saved all rows

	while (Rows.HasMore())
		{
		CRowKey Key;
		CDatum dData;
		SEQUENCENUMBER RowID;

		//	Get the row from the primary table

		Rows.GetNextRow(&Key, &dData, &RowID);

		//	Create the secondary view row (but only if not deleted)

		if (!dData.IsNil())
			pSecondaryView->CreateSecondaryRows(PrimaryDims, Process, Key, dData, RowID, &SecondaryRows);

		//	After a certain number of rows we save out a segment so we don't have to
		//	keep everything in memory.

		if (SecondaryRows.GetCount() > 10000 || (!Rows.HasMore() && SecondaryRows.GetCount() > 0))
			{
			Lock.Lock();

			//	After we lock we need to recompute the view pointers because
			//	they may have changed.

			pSecondaryView = m_Views.GetAt(dwViewID);

			//	Figure out the name for the new segment

			CString sBackup;
			CString sFilespec = GetUniqueSegmentFilespec(&sBackup);

			//	Create a segment

			CString sError;
			CAeonSegment *pNewSeg;
			if (!pSecondaryView->CreateSegment(sFilespec, SegSeq, &SecondaryRows, &pNewSeg, &sError))
				{
				m_pProcess->ReportVolumeFailure(sFilespec);

				//	Delete all previously created segments

				for (i = 0; i < NewSegments.GetCount(); i++)
					NewSegments[i].pSeg->Release();

				//	We abort this process and wait for a better time.

				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_UPDATE_VIEW_ABORTED, pSecondaryView->GetName(), m_sName));
				m_iHousekeeping = stateReady;
				return false;
				}

			//	Add the new segment to our array and remember the backup filespec
			//	also.

			SNewSegment *pEntry = NewSegments.Insert();
			pEntry->dwViewID = dwViewID;
			pEntry->pSeg = pNewSeg;
			pEntry->sFilespec = sFilespec;
			pEntry->sBackup = sBackup;

			//	Delete all rows

			SecondaryRows.DeleteAll();

			//	Unlock and continue adding rows.

			SegSeq++;
			Lock.Unlock();
			}
		}

	//	Lock again

	Lock.Lock();

	pPrimaryView = m_Views.GetAt(DEFAULT_VIEW);
	pSecondaryView = m_Views.GetAt(dwViewID);

	//	Now that we've successfully created all segments, add them to the proper
	//	views and delete the in-memory rows.

	for (i = 0; i < NewSegments.GetCount(); i++)
		pSecondaryView->SegmentSaveComplete(NewSegments[i].pSeg);

	//	Mark the secondary view as up to date (so that the Insert call below
	//	works.)

	pSecondaryView->SetUpToDate();

	//	Save the descriptors

	SaveDesc();

	//	Now we iterate over all in memory rows add apply them to the secondary view

	CRowIterator MemoryRows;
	pPrimaryView->InitIterator(&MemoryRows, CAeonView::FLAG_EXCLUDE_SEGMENTS);

	//	Keep looping until we've saved all rows

	bool bFallback = false;
	while (MemoryRows.HasMore())
		{
		CRowKey Key;
		CDatum dData;
		SEQUENCENUMBER RowID;

		//	Get the row from the primary table

		MemoryRows.GetNextRow(&Key, &dData, &RowID);

		//	Get the old data from the segments.
		//	NOTE: We need to do this because the in memory rows might
		//	overwrite something in the segments. In that case we need to
		//	potentially remove the secondary key.

		CDatum dOldData;
		if (Rows.SelectKey(Key))
			Rows.GetRow(NULL, &dOldData);

		//	Insert to the view

		bool bLogFailed;
		pSecondaryView->Insert(PrimaryDims, Process, Key, dData, dOldData, RowID, &bLogFailed);
		if (bLogFailed)
			bFallback = true;
		}

	//	If we failed to write to the log file then we need to recover to
	//	the backup volume.

	if (bFallback)
		{
		m_pProcess->ReportVolumeFailure(fileAppend(fileAppend(m_pStorage->GetPath(m_sPrimaryVolume), m_sName), FILESPEC_RECOVERY_DIR), OP_UPDATING_VIEW);

		//	We reset our state because RecoveryRestore needs waits until 
		//	housekeeping is done. We're inside a lock, so no other thread
		//	should interfere anyway.

		m_iHousekeeping = stateReady;

		if (!RecoveryRestore())
			return true;

		m_iHousekeeping = stateUpdatingView;
		}

	//	Outside of the lock we create backups

	Lock.Unlock();

	if (!m_bBackupLost)
		{
		for (i = 0; i < NewSegments.GetCount(); i++)
			{
			if (!fileCopy(NewSegments[i].sFilespec, NewSegments[i].sBackup))
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_SEGMENT_BACKUP_FAILED, NewSegments[i].sBackup));

				//	LATER: Figure out which volume failed and report it.
				}
			}
		}

	//	Relock so we can set the state

	Lock.Lock();

	//	Done. We wait for the next housekeeping event to do other stuff
	//	(including updating other views).

	m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_VIEW_UPDATED, pSecondaryView->GetName(), m_sName));
	m_iHousekeeping = stateReady;
	return true;
	}

void CAeonTable::HousekeepingValidateBackup (void)

//	ValidateBackup
//
//	Make sure the backup is valid. If not, we set a flag to trigger a backup.

	{
	int i;
	CString sError;

	//	If we validate, log it.

	TArray<CString> Extra;
	if (ValidateVolume(m_sBackupVolume, Extra, &sError))
		{
#ifdef DEBUG_VERBOSE
		m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_BACKUP_OK, m_sName, m_sBackupVolume));
#endif

		//	If we have extra segment files, we need to delete them

		for (i = 0; i < Extra.GetCount(); i++)
			{
			m_pProcess->Log(MSG_LOG_INFO, strPattern(ERR_DELETING_EXTRA_SEGMENT_FILE, m_sName, Extra[i]));
			if (!fileDelete(Extra[i]))
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_DELETE_FILE, Extra[i]));
			}
		}

	//	Otherwise, we log an error and force a new backup

	else
		{
		//	Log an error, because this should not happen

		m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_BACKUP, m_sName, m_sBackupVolume, sError));

		//	Force a backup

		m_bBackupNeeded = true;
		}

	//	Either way, no need to validate backup again until next random check.

	m_bValidateBackup = false;
	}
