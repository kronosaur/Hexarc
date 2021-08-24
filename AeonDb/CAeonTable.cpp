//	CAeonTable.cpp
//
//	CAeonTable class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	DIRECTORY STRUCTURE
//
//	{tableName}
//		desc.ars					Table descriptor
//
//		segments					Current segment files
//			{########_####}.aseg
//			...
//
//		files						Data files (for typeFile tables only)
//			{########_########}.afile
//
//		recovery					Temporary write log for memory rows
//
//		scrap						Temporary files (such as uploads and partials)
//
//	ROBUSTNESS AND RECOVERY
//
//	At table can be in one of the following states:
//
//			Segments	PrimaryVolume				BackupVolume
// -----------------------------------------------------------------------------
//	AA:		Open		m_bPrimaryLost = false		m_bBackupLost = false
//						m_sPrimaryVolume is valid	m_sBackupVolume is valid
//													m_bBackupNeeded = false
//
//	AB:		Open		m_bPrimaryLost = false		m_bBackupLost = true
//						m_sPrimaryVolume is valid	m_sBackupVolume is valid
//													m_bBackupNeeded = true
//
//	AC:		Open		m_bPrimaryLost = false		m_bBackupLost = true
//						m_sPrimaryVolume is valid	m_sBackupVolume is blank
//													m_bBackupNeeded = false
//
//	CC:		Closed		m_bPrimaryLost = true		m_bBackupLost = true
//						m_sPrimaryVolume is blank	m_sBackupVolume is blank
//													m_bBackupNeeded = false
//
//
//	Event				AA					AB					AC					CC
// ---------------------------------------------------------------------------------------------------------------------
//	Error reading/		Swap to backup		CC					CC					-
//		writing
//
//	Primary volume		Swap to backup		CC					CC					-
//		lost
//
//	Backup volume		New backup			New backup			-					-
//		lost
//
//	Backup complete		-					AA					-					-
//
//	New volume added	-					-					New backup			See if data is there
//

#include "stdafx.h"

const int MAX_CHANGES_IN_MEMORY =						100;

DECLARE_CONST_STRING(FILE_DATA_TYPE_BINARY,				"binary");
DECLARE_CONST_STRING(FILE_DATA_TYPE_TEXT,				"text");

DECLARE_CONST_STRING(FILESPEC_TABLE_DESC_FILE,			"desc.ars");
DECLARE_CONST_STRING(FILESPEC_FILES_DIR,				"files");
DECLARE_CONST_STRING(FILESPEC_RECOVERY_DIR,				"recovery");
DECLARE_CONST_STRING(FILESPEC_SCRAP_CONS,				"%s\\scrap\\%s");
DECLARE_CONST_STRING(FILESPEC_SCRAP_DIR,				"scrap");
DECLARE_CONST_STRING(FILESPEC_SEGMENTS_DIR,				"segments");
DECLARE_CONST_STRING(FILESPEC_SEGMENTS_FILTER,			"segments\\*.aseg");
DECLARE_CONST_STRING(FILESPEC_SEGMENT_CONS,				"%s\\segments\\%04x%04x_%04x.aseg");
DECLARE_CONST_STRING(FILESPEC_ALL,						"*.*");

DECLARE_CONST_STRING(FIELD_BACKUP_VOLUMES,				"backupVolumes");
DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_DATA_TYPE,					"dataType");
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc");
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath");
DECLARE_CONST_STRING(FIELD_GLOBAL_ENV,					"globalEnv");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_IF_MODIFIED_AFTER,			"ifModifiedAfter")
DECLARE_CONST_STRING(FIELD_KEY,							"key");
DECLARE_CONST_STRING(FIELD_KEY_SORT,					"keySort");
DECLARE_CONST_STRING(FIELD_KEY_TYPE,					"keyType");
DECLARE_CONST_STRING(FIELD_LAST_SAVE_ON,				"lastSaveOn");
DECLARE_CONST_STRING(FIELD_LAST_UPDATE_ON,				"lastUpdateOn");
DECLARE_CONST_STRING(FIELD_MODIFIED_BY,					"modifiedBy");
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_PARTIAL_MAX_SIZE,			"partialMaxSize")
DECLARE_CONST_STRING(FIELD_PARTIAL_POS,					"partialPos");
DECLARE_CONST_STRING(FIELD_PRIMARY_KEY,					"primaryKey");
DECLARE_CONST_STRING(FIELD_PRIMARY_VOLUME,				"primaryVolume");
DECLARE_CONST_STRING(FIELD_SECONDARY_KEY,				"secondaryKey");
DECLARE_CONST_STRING(FIELD_SECONDARY_VIEWS,				"secondaryViews");
DECLARE_CONST_STRING(FIELD_SIZE,						"size");
DECLARE_CONST_STRING(FIELD_STORAGE_PATH,				"storagePath");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");
DECLARE_CONST_STRING(FIELD_UNMODIFIED,					"unmodified");
DECLARE_CONST_STRING(FIELD_VERSION,						"version");
DECLARE_CONST_STRING(FIELD_X,							"x");
DECLARE_CONST_STRING(FIELD_Y,							"y");
DECLARE_CONST_STRING(FIELD_Z,							"z");

DECLARE_CONST_STRING(KEY_SORT_ASCENDING,				"ascending");
DECLARE_CONST_STRING(KEY_SORT_DESCENDING,				"descending");

DECLARE_CONST_STRING(KEY_TYPE_DATE_TIME,				"dateTime");
DECLARE_CONST_STRING(KEY_TYPE_INT32,					"int32");
DECLARE_CONST_STRING(KEY_TYPE_INT64,					"int64");
DECLARE_CONST_STRING(KEY_TYPE_LIST_UTF8,				"list_utf8");
DECLARE_CONST_STRING(KEY_TYPE_UTF8,						"utf8");

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");

DECLARE_CONST_STRING(MUTATE_ADD_TO_SET,					"addToSet");
DECLARE_CONST_STRING(MUTATE_APPEND,						"append");
DECLARE_CONST_STRING(MUTATE_CODE_6_5,					"code6-5");
DECLARE_CONST_STRING(MUTATE_CODE_8,						"code8");
DECLARE_CONST_STRING(MUTATE_CONSUME,					"consume");
DECLARE_CONST_STRING(MUTATE_DATE_CREATED,				"dateCreated");
DECLARE_CONST_STRING(MUTATE_DATE_MODIFIED,				"dateModified");
DECLARE_CONST_STRING(MUTATE_DECREMENT,					"decrement");
DECLARE_CONST_STRING(MUTATE_DELETE,						"delete");
DECLARE_CONST_STRING(MUTATE_IGNORE,						"ignore");
DECLARE_CONST_STRING(MUTATE_INCREMENT,					"increment");
DECLARE_CONST_STRING(MUTATE_MAX,						"max");
DECLARE_CONST_STRING(MUTATE_MIN,						"min");
DECLARE_CONST_STRING(MUTATE_PREPEND,					"prepend");
DECLARE_CONST_STRING(MUTATE_PRIMARY_KEY,				"primaryKey");
DECLARE_CONST_STRING(MUTATE_REMOVE_FROM_SET,			"removeFromSet");
DECLARE_CONST_STRING(MUTATE_ROW_ID,						"rowID");
DECLARE_CONST_STRING(MUTATE_UPDATE_GREATER,				"updateGreater");
DECLARE_CONST_STRING(MUTATE_UPDATE_GREATER_NO_ERROR,	"updateGreaterNoError");
DECLARE_CONST_STRING(MUTATE_UPDATE_NIL,					"updateNil");
DECLARE_CONST_STRING(MUTATE_UPDATE_NIL_NO_ERROR,		"updateNilNoError");
DECLARE_CONST_STRING(MUTATE_UPDATE_VERSION,				"updateVersion");
DECLARE_CONST_STRING(MUTATE_UPDATE_VERSION_NO_ERROR,	"updateVersionNoError");
DECLARE_CONST_STRING(MUTATE_WRITE,						"write");
DECLARE_CONST_STRING(MUTATE_WRITE_NEW,					"writeNew");

DECLARE_CONST_STRING(OP_CREATE_CORE_DIRECTORIES,		"creating core directories");
DECLARE_CONST_STRING(OP_INSERT,							"inserting a row");
DECLARE_CONST_STRING(OP_MUTATE,							"mutating a row");

DECLARE_CONST_STRING(OPTION_RECURSIVE,					"recursive");

DECLARE_CONST_STRING(TABLE_TYPE_FILE,					"file");
DECLARE_CONST_STRING(TABLE_TYPE_STANDARD,				"standard");

DECLARE_CONST_STRING(TYPENAME_HEXE_FUNCTION,			"hexeFunction");

DECLARE_CONST_STRING(STR_SAVE_COMPLETE,					"Table %s: Saved updates.");
DECLARE_CONST_STRING(STR_ERROR_ABSOLUTE_PATH_REQUIRED,	"Absolute filePath expected.");
DECLARE_CONST_STRING(STR_ERROR_OUT_OF_DATE,				"Another client uploaded a newer version: %s.");
DECLARE_CONST_STRING(ERR_CRASH,							"Crash: %s.");
DECLARE_CONST_STRING(ERR_EXCEPTION,						"Exception (%s): %s");
DECLARE_CONST_STRING(ERR_CANT_CREATE_KEY_TYPE,			"Unique keys cannot be of type dateTime or int32.");
DECLARE_CONST_STRING(ERR_CANT_DELETE_DEFAULT_VIEW,		"Default view cannot be deleted.");
DECLARE_CONST_STRING(STR_ERROR_FILE_TABLE_EXPECTED,		"File table expected.");
DECLARE_CONST_STRING(ERR_NOT_ENOUGH_DISK_SPACE,			"Insufficient disk space at: %s.");
DECLARE_CONST_STRING(ERR_INVALID_FILE_PATH,				"Invalid filePath: %s.");
DECLARE_CONST_STRING(STR_ERROR_INVALID_TABLE_TYPE,		"Invalid table type: %s.");
DECLARE_CONST_STRING(ERR_INVALID_VOLUME,				"Invalid volume: %s.");
DECLARE_CONST_STRING(ERR_CANT_CREATE_MULTI_D_KEY,		"Multidimensional keys cannot be generated.");
DECLARE_CONST_STRING(ERR_PATH_EXISTS,					"Path already exists.");
DECLARE_CONST_STRING(ERR_KEY_REQUIRED,					"Secondary views must specify key.");
DECLARE_CONST_STRING(ERR_SEGMENT_FOR_INVALID_VIEW,		"Segment %s refers to unknown view: %x.");
DECLARE_CONST_STRING(STR_BACKUP_OK,						"Table %s: Backup volume %s is healthy.");
DECLARE_CONST_STRING(STR_BACKUP_NEEDED,					"Table %s: Backup volume offline.");
DECLARE_CONST_STRING(STR_BACKUP_LOST,					"Table %s: Backup volume offline. Waiting for new volume.");
DECLARE_CONST_STRING(STR_BACKUP_ONLINE_RESTORE_NEEDED,	"Table %s: Backup volume reconnected.");
DECLARE_CONST_STRING(STR_BACKUP_ONLINE_RESTORE_WAITING,	"Table %s: Backup volume reconnected. Waiting for new volume to restore to.");
DECLARE_CONST_STRING(ERR_NO_DEFAULT_VIEW,				"Table %s: Cannot find default view.");
DECLARE_CONST_STRING(ERR_UNKNOWN_VIEW_IN_TABLE,			"Table %s: Unknown view: %s.");
DECLARE_CONST_STRING(STR_MOVING_BACKUP,					"Table %s: Found backup data on volume: %s.");
DECLARE_CONST_STRING(STR_MOVING_PRIMARY,				"Table %s: Found primary data on volume: %s.");
DECLARE_CONST_STRING(STR_NO_BACKUP_VOLUME,				"Table %s: No backup volume found.");
DECLARE_CONST_STRING(STR_LOST_DATA,						"Table %s: Primary and backup volumes offline.");
DECLARE_CONST_STRING(STR_PRIMARY_AND_BACKUP_ONLINE,		"Table %s: Primary and backup volumes reconnected.");
DECLARE_CONST_STRING(STR_RESTORE_NEEDED,				"Table %s: Primary volume offline.");
DECLARE_CONST_STRING(STR_PRIMARY_LOST,					"Table %s: Primary volume offline. Waiting for new volume.");
DECLARE_CONST_STRING(STR_PRIMARY_ONLINE_BACKUP_NEEDED,	"Table %s: Primary volume reconnected.");
DECLARE_CONST_STRING(STR_PRIMARY_ONLINE_BACKUP_WAITING,	"Table %s: Primary volume reconnected. Waiting for new volume for backup.");
DECLARE_CONST_STRING(STR_RECOVERED_ROWS,				"Table %s: Recovered %d row%p.");
DECLARE_CONST_STRING(STR_RESTORING,						"Table %s: Restoring primary to: %s.");
DECLARE_CONST_STRING(STR_RESTORE_COMPLETE,				"Table %s: Restore complete.");
DECLARE_CONST_STRING(STR_SWAP_TO_BACKUP,				"Table %s: Using backup volume as primary.");
DECLARE_CONST_STRING(STR_NEW_BACKUP,					"Table %s: Using new volume for backup: %s.");
DECLARE_CONST_STRING(STR_NEW_PRIMARY,					"Table %s: Using new volume for primary: %s.");
DECLARE_CONST_STRING(ERR_TOO_MANY_VOLUMES,				"Table appears on more then two volumes.");
DECLARE_CONST_STRING(ERR_TABLE_DESC_DOESNT_MATCH,		"Table desc does not match across volumes.");
DECLARE_CONST_STRING(STR_ERROR_BAD_TABLE_NAME,			"Table desc name does not match directory: %s.");
DECLARE_CONST_STRING(STR_ERROR_TABLE_DIR_EXISTS,		"Table directory already exists: %s.");
DECLARE_CONST_STRING(ERR_NOT_TYPE_FILE,					"Table is not a file table: %s.");
DECLARE_CONST_STRING(STR_ERROR_NO_TABLE_IN_PATH,		"Table name expected.");
DECLARE_CONST_STRING(ERR_CANNOT_ADD_ROW,				"Unable to add a row to table: %s.");
DECLARE_CONST_STRING(ERR_PRIMARY_OFFLINE,				"Unable to access primary volume for table: %s.");
DECLARE_CONST_STRING(ERR_CANNOT_COPY_FILE,				"Unable to copy from %s to %s.");
DECLARE_CONST_STRING(ERR_SEGMENT_BACKUP_FAILED,			"Unable to create backup for new segment: %s.");
DECLARE_CONST_STRING(ERR_CANT_CREATE_DIRECTORY,			"Unable to create directory: %s.");
DECLARE_CONST_STRING(STR_ERROR_BAD_ITERATOR,			"Unable to create iterator for table: %s.");
DECLARE_CONST_STRING(STR_ERROR_CANT_CREATE_DIRS,		"Unable to create table directories: %s.");
DECLARE_CONST_STRING(ERR_CANT_CREATE_WITH_NO_NAME,		"Unable to create table with no name.");
DECLARE_CONST_STRING(ERR_CANT_FIND_PRIMARY_VOLUME,		"Unable to find table's primary volume.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_LIST_FILES,			"Unable to list files in: %s.");
DECLARE_CONST_STRING(ERR_CANT_LOAD_SEGMENT,				"Unable to load segment file: %s.");
DECLARE_CONST_STRING(STR_ERROR_CANT_PARSE_DESC,			"Unable to parse table desc: %s.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_READ_STORAGE,		"Unable to read file in arcology storage: %s.");
DECLARE_CONST_STRING(STR_ERROR_CANT_SAVE_DESC,			"Unable to save table desc: %s.");
DECLARE_CONST_STRING(STR_ERROR_CANT_WRITE_FILE,			"Unable to write file to table: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN,						"Unknown error accessing table: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN_MUTATE_OP,				"Unknown mutation: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN_VIEW,					"Unknown view: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN_VIEW_ID,				"Unknown viewID: %d.");
DECLARE_CONST_STRING(ERR_UNKNOWN_VOLUME,				"Unknown volume: %s.");
DECLARE_CONST_STRING(ERR_VIEW_NOT_READY,				"View %s: Unable to query until update is complete.");
DECLARE_CONST_STRING(ERR_MISSING_FILE,					"Volume does not have required file: %s.");
DECLARE_CONST_STRING(ERR_OLD_RECOVERY_FILE,				"Table %s: View %s using old version of recovery file.");
DECLARE_CONST_STRING(ERR_VIEW_INSERT_FAILURE,			"Unable to insert row in view %s: %s.");
DECLARE_CONST_STRING(ERR_CANT_MOVE_TO_SCRAP,			"Unable to move old file to scrap: %s.");
DECLARE_CONST_STRING(ERR_UPDATE_GREATER_OUT_OF_DATE,	"Unable to mutate row %s. Field %s value (%s) not greater than original (%s).");
DECLARE_CONST_STRING(ERR_UPDATE_VERSION_OUT_OF_DATE,	"Unable to mutate row %s. Field %s value (%s) does not match original (%s).");
DECLARE_CONST_STRING(ERR_CANNOT_CONSUME,				"Unable to mutate row %s. Field %s value (%s) insufficient.");
DECLARE_CONST_STRING(ERR_UPDATE_NIL,					"Unable to mutate row %s. Field %s is not nil.");
DECLARE_CONST_STRING(ERR_INVALID_FILE_PATH_CODE,		"Unknown filePath generation code: %s.");
DECLARE_CONST_STRING(ERR_PARTIAL_POS_CANNOT_BE_NEGATIVE,"partialPos cannot be negative.");
DECLARE_CONST_STRING(ERR_INVALID_FILE_DATA_TYPE,		"Invalid dataType: %s");
DECLARE_CONST_STRING(ERR_FILE_NOT_FOUND,				"File not found: %s");

CAeonTable::~CAeonTable (void)

//	CAeonTable destructor

	{
	}

void CAeonTable::CloseSegments (bool bMarkForDelete)

//	CloseSegments
//
//	Closes all segments

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Close all segments in all views

	for (i = 0; i < m_Views.GetCount(); i++)
		m_Views[i].CloseSegments(bMarkForDelete);
	}

void CAeonTable::CollectGarbage (void)

//	CollectGarbage
//
//	Clean up garbage. This is guaranteed to be called on a single thread
//	when the world is stopped.

	{
	//	Delete files in scrap directory

	CString sTablePath = fileAppend(m_pStorage->GetPath(m_sPrimaryVolume), m_sName);
	if (!filePathDelete(fileAppend(sTablePath, FILESPEC_SCRAP_DIR), FPD_FLAG_RECURSIVE | FPD_FLAG_CONTENT_ONLY))
		{
		//	LATER: Log inability to delete
		}

	if (!m_bBackupLost)
		{
		sTablePath = fileAppend(m_pStorage->GetPath(m_sBackupVolume), m_sName);
		if (!filePathDelete(fileAppend(sTablePath, FILESPEC_SCRAP_DIR), FPD_FLAG_RECURSIVE | FPD_FLAG_CONTENT_ONLY))
			{
			//	LATER: Log inability to delete
			}
		}
	}

bool CAeonTable::CopyDirectory (const CString &sFromPath, const CString &sToPath, CString *retsError)

//	CopyDirectory
//
//	Copies the files in a directory from sFromPath to sToPath.
//
//	If any files in the destination are the same as the source, then it
//	(optimistically) leaves them. If there are files in the destination that are
//	not in the source, it deletes them.

	{
	int i;

	//	Make a list of all files in the source.

	TArray<CString> SourceFiles;
	if (!fileGetFileList(sFromPath, NULL_STR, FILESPEC_ALL, FFL_FLAG_RELATIVE_FILESPEC, &SourceFiles))
		{
		m_pProcess->ReportVolumeFailure(sFromPath);
		*retsError = strPattern(ERR_UNABLE_TO_LIST_FILES, sFromPath);
		return false;
		}

	//	Prepare the destination

	TArray<CString> DestFiles;
	if (fileExists(sToPath))
		{
		//	Now list all files in the destination

		if (!fileGetFileList(sToPath, NULL_STR, FILESPEC_ALL, FFL_FLAG_RELATIVE_FILESPEC, &DestFiles))
			{
			m_pProcess->ReportVolumeFailure(sToPath);
			*retsError = strPattern(ERR_UNABLE_TO_LIST_FILES, sToPath);
			return false;
			}

		//	Delete all files in the destination that are not in the source.

		for (i = 0; i < DestFiles.GetCount(); i++)
			{
			if (!SourceFiles.Find(DestFiles[i]))
				{
				fileDelete(fileAppend(sToPath, DestFiles[i]));
				DestFiles.Delete(i);
				i--;
				}
			}
		}
	else
		{
		if (!filePathCreate(sToPath))
			{
			m_pProcess->ReportVolumeFailure(sToPath);
			*retsError = strPattern(ERR_CANT_CREATE_DIRECTORY, sToPath);
			return false;
			}
		}

	//	Figure out which files we need to copy and add up the total disk space
	//	that we will need.

	DWORDLONG dwSpaceNeeded = 0;
	for (i = 0; i < SourceFiles.GetCount(); i++)
		{
		CString sSource = fileAppend(sFromPath, SourceFiles[i]);
		CString sDest = fileAppend(sToPath, SourceFiles[i]);

		//	If the file is already at the destination, then we skip it

		if (fileCompare(sSource, sDest, true))
			{
			SourceFiles[i] = NULL_STR;
			continue;
			}

		//	Otherwise, add up the size

		dwSpaceNeeded += fileGetSize(sSource) - fileGetSize(sDest);
		}

	//	Do we have enough space?

	DWORDLONG dwSpaceAvailable;
	fileGetDriveSpace(sToPath, &dwSpaceAvailable);
	if (dwSpaceAvailable < dwSpaceNeeded)
		{
		*retsError = strPattern(ERR_NOT_ENOUGH_DISK_SPACE, sToPath);
		return false;
		}

	//	Now loop over all files in the source and copy them to the destination.

	for (i = 0; i < SourceFiles.GetCount(); i++)
		if (!SourceFiles[i].IsEmpty())
			{
			CString sSource = fileAppend(sFromPath, SourceFiles[i]);
			CString sDest = fileAppend(sToPath, SourceFiles[i]);

			//	Copy

			if (!fileCopy(sSource, sDest))
				{
				//	LATER: Don't know which filespec failed, so we can't report it.
				*retsError = strPattern(ERR_CANNOT_COPY_FILE, sSource, sDest);
				return false;
				}
			}

	//	Done

	return true;
	}

bool CAeonTable::CopyVolume (const CString &sFrom, const CString &sTo, CString *retsError)

//	CopyVolume
//
//	Copies a volume

	{
	//	Get the paths

	CString sFromPath = m_pStorage->GetPath(sFrom);
	if (sFromPath.IsEmpty())
		{
		*retsError = strPattern(ERR_INVALID_VOLUME, sFromPath);
		return false;
		}

	CString sToPath = m_pStorage->GetPath(sTo);
	if (sToPath.IsEmpty())
		{
		*retsError = strPattern(ERR_INVALID_VOLUME, sToPath);
		return false;
		}

	CString sFromTablePath = fileAppend(sFromPath, m_sName);
	CString sToTablePath = fileAppend(sToPath, m_sName);

	//	Delete the path, in case it has some stale files. We ignore errors 
	//	because the path might not exist.

	filePathDelete(sToTablePath, FPD_FLAG_RECURSIVE);

	//	Create the destination path

	if (!filePathCreate(sToTablePath))
		{
		*retsError = strPattern(ERR_CANT_CREATE_DIRECTORY, sToTablePath);
		return false;
		}

	//	Copy the descriptor

	if (!fileCopy(fileAppend(sFromTablePath, FILESPEC_TABLE_DESC_FILE), fileAppend(sToTablePath, FILESPEC_TABLE_DESC_FILE)))
		{
		*retsError = strPattern(ERR_CANNOT_COPY_FILE, fileAppend(sFromTablePath, FILESPEC_TABLE_DESC_FILE), fileAppend(sToTablePath, FILESPEC_TABLE_DESC_FILE));
		return false;
		}

	//	Copy the segments directory

	if (!CopyDirectory(fileAppend(sFromTablePath, FILESPEC_SEGMENTS_DIR), fileAppend(sToTablePath, FILESPEC_SEGMENTS_DIR), retsError))
		return false;

	//	Copy the files directory (if necessary)

	if (GetType() == typeFile)
		{
		if (!CopyDirectory(fileAppend(sFromTablePath, FILESPEC_FILES_DIR), fileAppend(sToTablePath, FILESPEC_FILES_DIR), retsError))
			return false;
		}
	else
		{
		//	Delete it if it is not supposed to be there.

		filePathDelete(fileAppend(sToTablePath, FILESPEC_FILES_DIR), FPD_FLAG_RECURSIVE);
		}

	//	Make sure scrap directory exists

	filePathCreate(fileAppend(sToTablePath, FILESPEC_SCRAP_DIR));

	//	Make sure recovery directory exists

	filePathCreate(fileAppend(sToTablePath, FILESPEC_RECOVERY_DIR));

	return true;
	}

bool CAeonTable::Create (IArchonProcessCtx *pProcess, CMachineStorage *pStorage, CDatum dDesc, CString *retsError)

//	Create
//
//	Create a new table

	{
	m_pProcess = pProcess;
	m_pStorage = pStorage;

	//	See if the caller specifies a primary and backup volume.

	CString sPrimaryVolume = dDesc.GetElement(FIELD_PRIMARY_VOLUME);
	CString sBackupVolume = dDesc.GetElement(FIELD_BACKUP_VOLUMES);

	//	If not, pick a random volume for primary storage and backup storage
	//	NOTE: We rely on the fact that our caller has locked the engine so we
	//	don't have to worry about another thread changing the storage object.

	if (sPrimaryVolume.IsEmpty() && sBackupVolume.IsEmpty())
		{
		sPrimaryVolume = m_pStorage->GetRedundantVolume(NULL_STR);
		sBackupVolume = m_pStorage->GetRedundantVolume(sPrimaryVolume);
		}
	else if (sPrimaryVolume.IsEmpty())
		sPrimaryVolume = m_pStorage->GetRedundantVolume(sBackupVolume);
	else if (sBackupVolume.IsEmpty())
		sBackupVolume = m_pStorage->GetRedundantVolume(sPrimaryVolume);

	//	Add volume info to the descriptor

	CComplexStruct *pNewDesc = new CComplexStruct(dDesc);
	pNewDesc->SetElement(FIELD_PRIMARY_VOLUME, sPrimaryVolume);
	pNewDesc->SetElement(FIELD_BACKUP_VOLUMES, sBackupVolume);
	CDatum dNewDesc(pNewDesc);

	//	Create the basic directories. We need to create at least the root
	//	directory because Init requires it.

	CString sTablePath;
	if (!CreateCoreDirectories(sPrimaryVolume, dNewDesc, &sTablePath, retsError))
		return false;

	//	Initialize from the descriptor

	if (!Init(sTablePath, dNewDesc, retsError))
		return false;

	//	Now the we have valid primary and secondary volumes, initialize upload sessions

	m_UploadSessions.Init(m_pStorage, m_sName, m_sPrimaryVolume, m_sBackupVolume);
		
	//	Create table on primary and backup volumes

	if (!Create(m_sPrimaryVolume, dNewDesc, retsError))
		//	LATER: Clean up.
		//	LATER: Probably should go into safe mode.
		return false;

	if (!m_bBackupLost)
		{
		if (!CreateCoreDirectories(m_sBackupVolume, dNewDesc, NULL, retsError))
			return false;

		if (!Create(m_sBackupVolume, dNewDesc, retsError))
			//	LATER: Clean up.
			//	LATER: Probably should go into safe mode.
			return false;
		}
	else
		m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_NO_BACKUP_VOLUME, m_sName));

	//	Done

	return true;
	}

bool CAeonTable::Create (const CString &sVolume, CDatum dDesc, CString *retsError)

//	Create
//
//	Creates the table on the given volume.

	{
	CString sVolPath = m_pStorage->GetPath(sVolume);
	if (sVolPath.IsEmpty())
		{
		*retsError = strPattern(ERR_UNKNOWN_VOLUME, sVolume);
		return false;
		}

	CString sTablePath = fileAppend(sVolPath, m_sName);

	//	Files directory

	if (m_iType == typeFile)
		{
		if (!filePathCreate(fileAppend(sTablePath, FILESPEC_FILES_DIR)))
			{
			m_pProcess->ReportVolumeFailure(fileAppend(sTablePath, FILESPEC_FILES_DIR));
			*retsError = strPattern(STR_ERROR_CANT_CREATE_DIRS, fileAppend(sTablePath, FILESPEC_FILES_DIR));
			return false;
			}
		}

	//	Save the table descriptor

	CString sDescFilespec = fileAppend(sTablePath, FILESPEC_TABLE_DESC_FILE);
	if (!SaveDesc(dDesc, sDescFilespec, retsError))
		return false;

	//	Done

	return true;
	}

bool CAeonTable::CreateCoreDirectories (const CString &sVolume, CDatum dDesc, CString *retsTablePath, CString *retsError)

//	CreateCoreDirectories
//
//	Creates the table on the given volume. NOTE that this is called before Init,
//	so we can't rely on any member variables.

	{
	CString sVolPath = m_pStorage->GetPath(sVolume);
	if (sVolPath.IsEmpty())
		{
		*retsError = strPattern(ERR_UNKNOWN_VOLUME, sVolume);
		return false;
		}

	//	Get the table name

	CString sName = dDesc.GetElement(FIELD_NAME);
	if (sName.IsEmpty())
		{
		*retsError = ERR_CANT_CREATE_WITH_NO_NAME;
		return false;
		}

	CString sTablePath = fileAppend(sVolPath, sName);

	//	If this table path already exists then we can't create it

	if (fileExists(sTablePath))
		{
		if (retsError)
			*retsError = strPattern(STR_ERROR_TABLE_DIR_EXISTS, sTablePath);
		return false;
		}

	//	Create the appropriate directories

	if (!filePathCreate(sTablePath))
		{
		m_pProcess->ReportVolumeFailure(sTablePath);
		*retsError = strPattern(STR_ERROR_CANT_CREATE_DIRS, sTablePath);
		return false;
		}

	//	Create other required directories

	if (!filePathCreate(fileAppend(sTablePath, FILESPEC_SEGMENTS_DIR)))
		{
		m_pProcess->ReportVolumeFailure(fileAppend(sTablePath, FILESPEC_SEGMENTS_DIR));
		*retsError = strPattern(STR_ERROR_CANT_CREATE_DIRS, fileAppend(sTablePath, FILESPEC_SEGMENTS_DIR));
		return false;
		}

	if (!filePathCreate(fileAppend(sTablePath, FILESPEC_RECOVERY_DIR)))
		{
		m_pProcess->ReportVolumeFailure(fileAppend(sTablePath, FILESPEC_RECOVERY_DIR), OP_CREATE_CORE_DIRECTORIES);
		*retsError = strPattern(STR_ERROR_CANT_CREATE_DIRS, fileAppend(sTablePath, FILESPEC_RECOVERY_DIR));
		return false;
		}

	if (!filePathCreate(fileAppend(sTablePath, FILESPEC_SCRAP_DIR)))
		{
		m_pProcess->ReportVolumeFailure(fileAppend(sTablePath, FILESPEC_SCRAP_DIR));
		*retsError = strPattern(STR_ERROR_CANT_CREATE_DIRS, fileAppend(sTablePath, FILESPEC_SCRAP_DIR));
		return false;
		}

	if (retsTablePath)
		*retsTablePath = sTablePath;

	//	Done

	return true;
	}

bool CAeonTable::CreatePrimaryKey (const CTableDimensions &Dims, CDatum dMutateDesc, SEQUENCENUMBER RowID, CRowKey *retKey, CString *retsError)

//	CreatePrimaryKey
//
//	Generates a primary key based on the mutation desc.

	{
	CSmartLock Lock(m_cs);

	//	See what kind of primary key we want.

	CDatum dKeyDesc = dMutateDesc.GetElement(FIELD_PRIMARY_KEY);
	CDatum dKey;
	int iDims;
	bool bAllowUTF8;
	bool bAllowInt64;

	//	Use the RowID

	if (dKeyDesc.IsNil() || strEquals(dKeyDesc, MUTATE_ROW_ID))
		{
		dKey = CDatum(RowID);

		iDims = 1;
		bAllowUTF8 = true;
		bAllowInt64 = true;
		}

	//	Generate a unique character code

	else if (strEquals(dKeyDesc, MUTATE_CODE_6_5))
		{
		//	Make sure the key is unique

		do
			{
			dKey = strPattern("%s-%s", cryptoRandomCodeBlock(6), cryptoRandomCodeBlock(5));
			}
		while (RowExists(Dims, dKey));

		iDims = 1;
		bAllowUTF8 = true;
		bAllowInt64 = false;
		}

	else if (strEquals(dKeyDesc, MUTATE_CODE_8))
		{
		//	Make sure the key is unique

		do
			{
			dKey = cryptoRandomCode(8);
			}
		while (RowExists(Dims, dKey));

		iDims = 1;
		bAllowUTF8 = true;
		bAllowInt64 = false;
		}

	else
		{
		*retsError = strPattern(ERR_UNKNOWN_MUTATE_OP, dKeyDesc.AsString());
		return false;
		}

	//	Make sure we have the proper number of dimensions.

	if (Dims.GetCount() != iDims)
		{
		*retsError = ERR_CANT_CREATE_MULTI_D_KEY;
		return false;
		}

	//	Create the key based on the data type.

	switch (Dims[0].iKeyType)
		{
		case keyInt64:
			if (!bAllowInt64)
				{
				*retsError = ERR_CANT_CREATE_KEY_TYPE;
				return false;
				}

			if (!CRowKey::ParseKeyForCreate(Dims, dKey, retKey, retsError))
				return false;
			break;

		case keyUTF8:
			if (!bAllowUTF8)
				{
				*retsError = ERR_CANT_CREATE_KEY_TYPE;
				return false;
				}

			if (!CRowKey::ParseKeyForCreate(Dims, dKey.AsString(), retKey, retsError))
				return false;
			break;

		default:
			*retsError = ERR_CANT_CREATE_KEY_TYPE;
			return false;
		}

	return true;
	}

bool CAeonTable::DebugDumpView (DWORD dwViewID, CDatum *retdResult) const

//	DebugDumpView
//
//	Returns info about the given view.

	{
	CSmartLock Lock(m_cs);

	const CAeonView *pView = m_Views.GetAt(dwViewID);
	if (pView == NULL)
		{
		*retdResult = strPattern(ERR_UNKNOWN_VIEW_ID, dwViewID);
		return false;
		}

	//	Get data

	*retdResult = pView->DebugDump();

	//	Done

	return true;
	}

bool CAeonTable::Delete (void)

//	Delete
//
//	Delete the table from disk.
//	NOTE: We expect this to only be called from a single thread.

	{
	int i;
	bool bSuccess = true;

	//	Close the recovery file on all views

	for (i = 0; i < m_Views.GetCount(); i++)
		m_Views[i].CloseRecovery();

	//	Delete all segment (so that we close the segment files).

	CloseSegments(true);

	//	Delete all table directories

	if (!Delete(m_sPrimaryVolume))
		bSuccess = false;

	if (!m_bBackupLost)
		{
		if (!Delete(m_sBackupVolume))
			bSuccess = false;
		}

	//	Done

	return bSuccess;
	}

bool CAeonTable::Delete (const CString &sVolume)

//	Delete
//
//	Delete from the given volume

	{
	bool bSuccess = true;
	CString sTablePath = fileAppend(m_pStorage->GetPath(sVolume), m_sName);

	//	Delete all files and directories

	//	Desc file

	if (!fileDelete(fileAppend(sTablePath, FILESPEC_TABLE_DESC_FILE)))
		bSuccess = false;

	//	Directories

	if (!filePathDelete(fileAppend(sTablePath, FILESPEC_SEGMENTS_DIR), FPD_FLAG_RECURSIVE))
		bSuccess = false;

	if (!filePathDelete(fileAppend(sTablePath, FILESPEC_RECOVERY_DIR), FPD_FLAG_RECURSIVE))
		bSuccess = false;

	if (!filePathDelete(fileAppend(sTablePath, FILESPEC_SCRAP_DIR), FPD_FLAG_RECURSIVE))
		bSuccess = false;

	if (m_iType == typeFile)
		{
		if (!filePathDelete(fileAppend(sTablePath, FILESPEC_FILES_DIR), FPD_FLAG_RECURSIVE))
			bSuccess = false;
		}

	//	Table root

	if (!filePathDelete(sTablePath))
		bSuccess = false;

	return bSuccess;
	}

bool CAeonTable::DeleteView (DWORD dwViewID, CString *retsError)

//	DeleteView
//
//	Deletes the given view

	{
	CSmartLock Lock(m_cs);

	//	Cannot delete the default view

	if (dwViewID == DEFAULT_VIEW)
		{
		*retsError = ERR_CANT_DELETE_DEFAULT_VIEW;
		return false;
		}

	CAeonView *pView = m_Views.GetAt(dwViewID);
	if (pView == NULL)
		{
		*retsError = strPattern(ERR_UNKNOWN_VIEW_ID, dwViewID);
		return false;
		}

	//	Delete all segments

	pView->CloseSegments(true);

	//	Remove from list

	m_Views.DeleteAt(dwViewID);

	//	Save descriptor

	SaveDesc();

	//	Done

	return true;
	}

bool CAeonTable::DiffDesc (CDatum dDesc, TArray<CDatum> *retNewViews, CString *retsError)

//	DiffDesc
//
//	Compares the given descriptor to the current table descriptor.
//	If there are changes that we can apply (e.g., adding new views) then we
//	return those.
//
//	If there are changes that cannot be applied, we return an error.

	{
	int i;
	retNewViews->DeleteAll();

	//	Loop over all secondary views and see if there are any new ones.

	CDatum dSecondaryViews = dDesc.GetElement(FIELD_SECONDARY_VIEWS);
	for (i = 0; i < dSecondaryViews.GetCount(); i++)
		{
		CDatum dViewDesc = dSecondaryViews.GetElement(i);
		const CString &sView = dViewDesc.GetElement(FIELD_NAME);

		//	If this view does not exist, add it to the list of new views.

		DWORD dwViewID;
		if (!FindView(sView, &dwViewID))
			retNewViews->Insert(dViewDesc);
		}

	//	Done

	return true;
	}

bool CAeonTable::FileDirectory (const CString &sDirKey, CDatum dRequestedFields, CDatum dOptions, CDatum *retResult, CString *retsError)

//	FileDirectory
//
//	Returns a list of files in the directory

	{
	int i;

	//	Get the set of options

	bool bRecursive = false;
	bool bFilePathsOnly = (dRequestedFields.GetCount() == 1 && strEquals(dRequestedFields.GetElement(0), FIELD_FILE_PATH));

	for (i = 0; i < dOptions.GetCount(); i++)
		{
		if (strEquals(dOptions.GetElement(i), OPTION_RECURSIVE))
			bRecursive = true;
		}

	//	Is it a directory?

	if (GetType() != CAeonTable::typeFile)
		{
		*retsError = strPattern(ERR_NOT_TYPE_FILE, m_sName);
		return false;
		}

	//	Protect from read errors

	bool bRecovery = false;
	CString sDiskError;
	try
		{
		//	Create an iterator

		CRowIterator Sel;
		if (!InitIterator(DEFAULT_VIEW, &Sel, NULL, retsError))
			return false;

		//	Select the path

		CRowKey DirKey;
		CRowKey::CreateFromFilePath(sDirKey, &DirKey, true);
		Sel.SelectKey(DirKey);
		int iDirLen = sDirKey.GetLength();

		//	We return an array

		CComplexArray *pArray = new CComplexArray;

		//	The contents of the array depend on dRequestedFields.
		//	If all we want are filePaths, returns those in the array.

		if (bFilePathsOnly)
			{
			while (Sel.HasMore())
				{
				CDatum dFileDesc;
				CString sKey = Sel.GetNextRow(&dFileDesc);
				if (!strStartsWith(sKey, sDirKey))
					break;

				if (!bRecursive && *(sKey.GetParsePointer() + iDirLen) == '[')
					break;

				if (!dFileDesc.IsNil())
					pArray->Insert(strPattern("/%s%s", m_sName, CRowKey::KeyToFilePath(sKey)));
				}
			}

		//	Otherwise we return an array of FileDescs.

		else
			{
			while (Sel.HasMore())
				{
				CDatum dFileDesc;
				CString sKey = Sel.GetNextRow(&dFileDesc);
				if (!strStartsWith(sKey, sDirKey))
					break;

				if (!bRecursive && *(sKey.GetParsePointer() + iDirLen) == '[')
					break;

				//	If nil, then it was deleted

				if (!dFileDesc.IsNil())
					{
					//	Create a new fileDesc containing only the fields
					//	that the caller wants

					CComplexStruct *pNewFileDesc = new CComplexStruct;

					//	We always add filePath

					pNewFileDesc->SetElement(FIELD_FILE_PATH, strPattern("/%s%s", m_sName, CRowKey::KeyToFilePath(sKey)));

					//	If Nil, then we add all fields

					if (dRequestedFields.IsNil())
						{
						for (i = 0; i < dFileDesc.GetCount(); i++)
							{
							const CString &sField = dFileDesc.GetKey(i);

							if (!strEquals(sField, FIELD_FILE_PATH)
									&& !strEquals(sField, FIELD_STORAGE_PATH))
								pNewFileDesc->SetElement(sField, dFileDesc.GetElement(sField));
							}
						}

					//	Otherwise we add the fields in the list

					else
						{
						for (i = 0; i < dRequestedFields.GetCount(); i++)
							{
							const CString &sField = dRequestedFields.GetElement(i);

							if (!strEquals(sField, FIELD_FILE_PATH))
								pNewFileDesc->SetElement(sField, dFileDesc.GetElement(sField));
							}
						}

					//	Add to the array

					pArray->Insert(CDatum(pNewFileDesc));
					}
				}
			}

		//	Done

		*retResult = CDatum(pArray);
		}
	catch (CFileException e)
		{
		bRecovery = true;
		sDiskError = e.GetFilespec();
		}
	catch (...)
		{
		m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CRASH, CString(__FUNCTION__)));
		*retsError = strPattern(ERR_UNKNOWN, m_sName);
		return false;
		}

	//	On disk failure we try the backup

	if (bRecovery)
		{
		m_pProcess->ReportVolumeFailure(sDiskError);

		//	Try to restore from backup. If we fail, then we're screwed.

		if (!RecoveryRestore())
			{
			*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
			return false;
			}

		//	Retry

		return FileDirectory(sDirKey, dRequestedFields, dOptions, retResult, retsError);
		}

	//	Done

	return true;
	}

DWORD CAeonTable::FindSecondaryViewToUpdate (void) const

//	FindSecondaryViewToUpdate
//
//	Returns the ViewID of a the next view to update. If none need updating, we 
//	return 0.
//
//	Must be called inside a lock.

	{
	int i;

	for (i = 0; i < m_Views.GetCount(); i++)
		if (!m_Views[i].IsUpToDate() 
				&& m_Views[i].IsSecondaryView()
				&& m_Views[i].IsValid())
			{
			return m_Views[i].GetID();
			}

	return 0;
	}

bool CAeonTable::FindTableVolumes (TArray<CString> *retVolumes)

//	FindTableVolumes
//
//	Returns a list of volumes that have data for this table.

	{
	int i;

	for (i = 0; i < m_pStorage->GetCount(); i++)
		{
		if (fileExists(fileAppend(m_pStorage->GetPath(i), m_sName)))
			retVolumes->Insert(m_pStorage->GetVolume(i));
		}

	return (retVolumes->GetCount() > 0);
	}

bool CAeonTable::FindView (const CString &sView, DWORD *retdwViewID)

//	FindView
//
//	Finds the given view by name.

	{
	CSmartLock Lock(m_cs);

	CAeonView *pView;
	if (!FindView(sView, &pView))
		return false;

	//	Done

	if (retdwViewID)
		*retdwViewID = pView->GetID();

	return true;
	}

bool CAeonTable::FindView (const CString &sView, CAeonView **retpView)

//	FindView
//
//	Finds the given view.
//
//	NOTE: The returned pointer may move if the table is unlocked. The caller
//	must keep the table locked while using the pointer.

	{
	int i;

	//	NULL_STR means default view.

	if (sView.IsEmpty())
		{
		if (retpView)
			*retpView = m_Views.GetAt(DEFAULT_VIEW);

		return true;
		}

	//	Otherwise, look by name

	for (i = 0; i < m_Views.GetCount(); i++)
		if (strEquals(m_Views[i].GetName(), sView))
			{
			if (retpView)
				*retpView = &m_Views[i];

			return true;
			}

	//	Not found

	return false;
	}

bool CAeonTable::FindViewAndPath (const CString &sView, DWORD *retdwViewID, CDatum dKey, CRowKey *retKey, CString *retsError)

//	FindViewAndPath
//
//	Finds the given view by name and looks up the key.

	{
	CSmartLock Lock(m_cs);

	CAeonView *pView;
	if (!FindView(sView, &pView))
		{
		if (retsError)
			*retsError = strPattern(ERR_UNKNOWN_VIEW_IN_TABLE, GetName(), sView);
		return false;
		}

	//	Get the key
	
	if (retKey)
		{
		if (!CRowKey::ParseKey(pView->GetDimensions(), dKey, retKey, retsError))
			return false;
		}

	//	Done

	if (retdwViewID)
		*retdwViewID = pView->GetID();

	return true;
	}

int CAeonTable::FindVolumeToOpen (const TArray<CString> &Volumes, TArray<SEQUENCENUMBER> *retSeq)

//	FindVolumeToOpen
//
//	Returns the index of the volume in the array that we should open.
//	If there is more than one volume we pick the one with the highest sequence
//	number.
//
//	Optionally returns all sequence numbers.

	{
	int i;
	ASSERT(Volumes.GetCount() > 0);

	if (Volumes.GetCount() == 1 && retSeq == NULL)
		return 0;

	//	Pick the best sequence number

	int iBestVolume = -1;
	SEQUENCENUMBER BestSeq;

	for (i = 0; i < Volumes.GetCount(); i++)
		{
		SEQUENCENUMBER Seq = GetVolumeSeq(Volumes[i]);
		if (iBestVolume == -1 || Seq > BestSeq)
			{
			iBestVolume = i;
			BestSeq = Seq;
			}

		if (retSeq)
			retSeq->Insert(Seq);
		}

	//	Done

	return iBestVolume;
	}

bool CAeonTable::GetData (DWORD dwViewID, const CRowKey &Path, CDatum *retData, SEQUENCENUMBER *retRowID, CString *retsError)

//	GetData
//
//	Returns the data at the given row

	{
	CSmartLock Lock(m_cs);

	//	Make sure we have the primary volume

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return false;
		}

	//	Get the view

	CAeonView *pView = m_Views.GetAt(dwViewID);

	//	Try to get the data, but if we fail, we recover

	bool bRecovery = false;
	CString sDiskError;
	try
		{
		//	If this is a primary view then we can get the data directly

		if (!pView->IsSecondaryView())
			return pView->GetData(Path, retData, retRowID, retsError);

		//	Otherwise we need to search for it (because we don't know the last
		//	column value, which is a rowID).

		CTableDimensions Dims;
		CRowIterator Sel;

		//	Do not include nil (deleted) rows

		Sel.SetIncludeNil(false);
		if (!InitIterator(dwViewID, &Sel, &Dims, retsError))
			return false;

		//	Select the path

		if (!Sel.SelectKey(Path) || !Sel.HasMore())
			{
			//	If not found, we return Nil

			*retData = CDatum();
			if (retRowID)
				*retRowID = 0;
			return true;
			}

		//	Otherwise, get the data

		Sel.GetRow(NULL, retData, retRowID);

		//	Done

		return true;
		}
	catch (CFileException e)
		{
		bRecovery = true;
		sDiskError = e.GetFilespec();
		}
	catch (...)
		{
		m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CRASH, CString(__FUNCTION__)));
		*retsError = strPattern(ERR_UNKNOWN, m_sName);
		return false;
		}

	//	On failure we try the backup

	if (bRecovery)
		{
		m_pProcess->ReportVolumeFailure(sDiskError);

		if (!RecoveryRestore())
			{
			*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
			return false;
			}

		return GetData(dwViewID, Path, retData, retRowID, retsError);
		}

	//	Can't get here

	return false;
	}

CDatum CAeonTable::GetDesc (void)

//	GetDesc
//
//	Returns a descriptor

	{
	CSmartLock Lock(m_cs);
	int i;

	CComplexStruct *pDesc = new CComplexStruct;

	//	Name

	pDesc->SetElement(FIELD_NAME, m_sName);
	pDesc->SetElement(FIELD_PRIMARY_VOLUME, (m_bPrimaryLost ? NULL_STR : m_sPrimaryVolume));
	pDesc->SetElement(FIELD_BACKUP_VOLUMES, (m_bBackupLost ? NULL_STR : m_sBackupVolume));

	//	Type

	switch (m_iType)
		{
		case typeStandard:
			pDesc->SetElement(FIELD_TYPE, TABLE_TYPE_STANDARD);
			break;

		case typeFile:
			pDesc->SetElement(FIELD_TYPE, TABLE_TYPE_FILE);
			break;
		}

	//	Default view

	CAeonView *pView = m_Views.GetAt(DEFAULT_VIEW);
	pView->WriteDesc(pDesc);

	//	Write out the secondary views

	if (m_Views.GetCount() > 1)
		{
		CComplexArray *pArray = new CComplexArray;

		for (i = 1; i < m_Views.GetCount(); i++)
			{
			CComplexStruct *pDesc = new CComplexStruct;
			m_Views[i].WriteDesc(pDesc);

			pArray->Insert(CDatum(pDesc));
			}

		pDesc->SetElement(FIELD_SECONDARY_VIEWS, CDatum(pArray));
		}

	//	Stats and other info

	pDesc->SetElement(FIELD_LAST_UPDATE_ON, CDateTime::FromTick(m_dwLastUpdate));
	pDesc->SetElement(FIELD_LAST_SAVE_ON, CDateTime::FromTick(m_dwLastSave));

	return CDatum(pDesc);
	}

CDatum CAeonTable::GetDimensionDesc (SDimensionDesc &Dim)

//	GetDimensionDesc
//
//	Converts fomr a dimension structure to a datum

	{
	CComplexStruct *pDesc = new CComplexStruct;
	SetDimensionDesc(pDesc, Dim);
	return CDatum(pDesc);
	}

CDatum CAeonTable::GetDimensionDescForSecondaryView (SDimensionDesc &Dim, CDatum dKey)

//	GetDimensionDescForSecondaryView
//
//	Converts from a dimension structure to a datum

	{
	CComplexStruct *pDesc = new CComplexStruct;
	SetDimensionDesc(pDesc, Dim);
	pDesc->SetElement(FIELD_KEY, dKey);
	return CDatum(pDesc);
	}

CDatum CAeonTable::GetDimensionPath (const CTableDimensions &Dims, const CString &sKey)

//	GetDimensionPath
//
//	Returns a dimension path from a stored key

	{
	int i;
	char *pPos = sKey.GetParsePointer();
	char *pPosEnd = pPos + sKey.GetLength();

	if (Dims.GetCount() == 1)
		return GetDimensionPathElement(Dims[0].iKeyType, &pPos, pPosEnd);
	else
		{
		CComplexArray *pPath = new CComplexArray;
		
		for (i = 0; i < Dims.GetCount(); i++)
			pPath->Insert(GetDimensionPathElement(Dims[i].iKeyType, &pPos, pPosEnd));

		return CDatum(pPath);
		}
	}

CDatum CAeonTable::GetDimensionPathElement (EKeyTypes iKeyType, char **iopPos, char *pPosEnd)
	{
	switch (iKeyType)
		{
		case keyInt32:
			if ((*iopPos) + sizeof(int) <= pPosEnd)
				{
				CDatum dKey(*(int *)(*iopPos));
				(*iopPos) += sizeof(int);

				if ((*iopPos) < pPosEnd)
					(*iopPos)++;

				return dKey;
				}
			else
				return CDatum();

		case keyInt64:
			if ((*iopPos) + sizeof(DWORDLONG) <= pPosEnd)
				{
				CDatum dKey(*(DWORDLONG *)(*iopPos));
				(*iopPos) += sizeof(DWORDLONG);

				if ((*iopPos) < pPosEnd)
					(*iopPos)++;

				return dKey;
				}
			else
				return CDatum();

		case keyDatetime:
		case keyListUTF8:
		case keyUTF8:
			{
			char *pStart = (*iopPos);
			while ((*iopPos) < pPosEnd && *(*iopPos) != '\0')
				(*iopPos)++;

			CDatum dKey(CString(pStart, (int)((*iopPos) - pStart)));

			if ((*iopPos) < pPosEnd)
				(*iopPos)++;

			return dKey;
			}

		default:
			ASSERT(false);
			return CDatum();
		}
	}

bool CAeonTable::GetFileData (const CString &sFilePath, const SFileDataOptions &Options, CDatum *retdFileDownloadDesc, CString *retsError)

//	GetFileData
//
//	Returns file data

	{
	//	Generate a path

	CRowKey Path;
	CRowKey::CreateFromFilePath(sFilePath, &Path);

	//	Lock so to make sure that we get the right version of the file

	CSmartLock Lock(m_cs);

	//	Look up the file in the database.

	CDatum dFileDesc;
	if (!GetData(DEFAULT_VIEW, Path, &dFileDesc, NULL, retsError))
		return false;

	//	If the file has not been modified since the given date, then we return

	if (Options.IfModifiedAfter.IsValid()
			&& Options.IfModifiedAfter >= (const CDateTime &)dFileDesc.GetElement(FIELD_MODIFIED_ON))
		{
		CComplexStruct *pFileDownloadDesc = new CComplexStruct;
		pFileDownloadDesc->SetElement(FIELD_UNMODIFIED, CDatum(CDatum::constTrue));
		pFileDownloadDesc->SetElement(FIELD_FILE_DESC, PrepareFileDesc(m_sName, sFilePath, dFileDesc, (Options.bTranspace ? FLAG_TRANSPACE : 0)));

		*retdFileDownloadDesc = CDatum(pFileDownloadDesc);
		return true;
		}

	//	Open the file

	CString sFilespec = m_pStorage->CanonicalRelativeToMachine(m_sPrimaryVolume, dFileDesc.GetElement(FIELD_STORAGE_PATH));

	CFile theFile;
	if (!theFile.Create(sFilespec, CFile::FLAG_OPEN_READ_ONLY))
		{
		*retsError = strPattern(ERR_UNABLE_TO_READ_STORAGE, sFilespec);
		return false;
		}

	//	Seek to the right position

	try
		{
		if (Options.iPos != 0)
			theFile.Seek(Options.iPos);
		}
	catch (...)
		{
		if (retsError)
			*retsError = strPattern(ERR_UNABLE_TO_READ_STORAGE, sFilespec);
		return false;
		}

	//	Unlock because we're only protecting the connection between
	//	the fileDesc and the file itself.

	Lock.Unlock();

	//	Read the file into a datum

	CString sDataType;
	CDatum dData;
	switch (Options.iDataType)
		{
		case FileDataType::text:
			{
			int iDataRemaining = theFile.GetStreamLength() - theFile.GetPos();
			int iBufferSize = (Options.iMaxSize < 0 ? iDataRemaining : Min(iDataRemaining, Options.iMaxSize));
			if (iBufferSize > 0)
				{
				CStringBuffer Buffer;
				Buffer.SetLength(iBufferSize);
				theFile.Read(Buffer.GetPointer(), iBufferSize);

				if (!CDatum::CreateStringFromHandoff(Buffer, &dData))
					{
					if (retsError)
						*retsError = strPattern(ERR_UNABLE_TO_READ_STORAGE, sFilespec);
					return false;
					}
				}

			sDataType = FILE_DATA_TYPE_TEXT;
			break;
			}

		default:
			if (!CDatum::CreateBinary(theFile, Options.iMaxSize, &dData))
				{
				if (retsError)
					*retsError = strPattern(ERR_UNABLE_TO_READ_STORAGE, sFilespec);
				return false;
				}

			sDataType = FILE_DATA_TYPE_BINARY;
			break;
		}

	//	Close

	theFile.Close();

	//	Return a fileDownloadDesc
	//
	//	NOTE: It is important that we create a new structure because there are 
	//	cases in which our caller will modify it.

	CComplexStruct *pFileDownloadDesc = new CComplexStruct;

	//	Transpace message has a slightly different format

	pFileDownloadDesc->SetElement(FIELD_FILE_DESC, PrepareFileDesc(m_sName, sFilePath, dFileDesc, (Options.bTranspace ? FLAG_TRANSPACE : 0)));
	pFileDownloadDesc->SetElement(FIELD_PARTIAL_POS, Options.iPos);
	pFileDownloadDesc->SetElement(FIELD_DATA, dData);
	pFileDownloadDesc->SetElement(FIELD_DATA_TYPE, sDataType);

	//	Done

	*retdFileDownloadDesc = CDatum(pFileDownloadDesc);
	return true;
	}

CString CAeonTable::GetFileDataTypeID (FileDataType iDataType)

//	GetFileDataTypeID
//
//	Returns the ID.

	{
	switch (iDataType)
		{
		case FileDataType::binary:
			return FILE_DATA_TYPE_BINARY;

		case FileDataType::text:
			return FILE_DATA_TYPE_TEXT;

		default:
			return NULL_STR;
		}
	}

bool CAeonTable::GetFileDesc (const CString &sFilePath, CDatum *retdFileDesc, CString *retsError)

//	GetFileDesc
//
//	Returns the fileDesc for the given file

	{
	//	Generate a path

	CRowKey Path;
	CRowKey::CreateFromFilePath(sFilePath, &Path);

	//	Look up the file in the database.

	if (!GetData(DEFAULT_VIEW, Path, retdFileDesc, NULL, retsError))
		return false;

	//	Done

	return true;
	}

bool CAeonTable::GetKeyRange (int iCount, CDatum *retdResult, CString *retsError)

//	GetKeyRange
//
//	Returns a list of keys

	{
	CSmartLock Lock(m_cs);

	//	Make sure we have the primary volume

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return false;
		}

	Lock.Unlock();

	bool bRecovery = false;
	CString sDiskError;
	try
		{
		CTableDimensions Dims;
		CRowIterator Sel;
		if (!InitIterator(DEFAULT_VIEW, &Sel, &Dims, retsError))
			return false;

		//	We return an array of keys

		CComplexArray *pArray = new CComplexArray;

		while (Sel.HasMore() && iCount > 0)
			{
			pArray->Insert(GetDimensionPath(Dims, Sel.GetNextKey()));
			iCount--;
			}

		//	Done

		*retdResult = CDatum(pArray);
		}
	catch (CFileException e)
		{
		bRecovery = true;
		sDiskError = e.GetFilespec();
		}
	catch (...)
		{
		m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CRASH, CString(__FUNCTION__)));
		*retsError = strPattern(ERR_UNKNOWN, m_sName);
		return false;
		}

	if (bRecovery)
		{
		m_pProcess->ReportVolumeFailure(sDiskError);

		if (!RecoveryRestore())
			{
			*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
			return false;
			}

		return GetKeyRange(iCount, retdResult, retsError);
		}

	return true;
	}

CString CAeonTable::GetRecoveryFilespec (const CString &sTablePath, DWORD dwViewID)

//	GetRecoveryFilespec
//
//	Returns the filespec of the recovery file.

	{
	return fileAppend(fileAppend(sTablePath, FILESPEC_RECOVERY_DIR), strPattern("Log_%x.dat", dwViewID));
	}

CString CAeonTable::GetRecoveryFilespec (DWORD dwViewID)

//	GetRecoveryFilespec
//
//	Returns the filespec of the recovery file for the given view

	{
	if (m_bPrimaryLost)
		return NULL_STR;

	CString sVolumePath = m_pStorage->GetPath(m_sPrimaryVolume);
	if (sVolumePath.IsEmpty())
		return NULL_STR;

	return GetRecoveryFilespec(fileAppend(sVolumePath, m_sName), dwViewID);
	}

void CAeonTable::GetSegmentFilespecs (const CString &sTablePath, TArray<CString> *retList) const

//	GetSegmentFilespecs
//
//	Returns a list of all segment files for the table

	{
	fileGetFileList(fileAppend(sTablePath, FILESPEC_SEGMENTS_FILTER),
			0,
			retList);
	}

bool CAeonTable::GetRows (DWORD dwViewID, CDatum dLastKey, int iRowCount, const TArray<int> &Limits, DWORD dwFlags, CDatum *retdResult, CString *retsError)

//	GetRows
//
//	Returns rows in the table.

	{
	CSmartLock Lock(m_cs);

	//	Make sure we have the primary volume

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return false;
		}

	//	Some flags

	CAeonView *pView = m_Views.GetAt(dwViewID);
	if (pView == NULL)
		{
		*retsError = strPattern(ERR_UNKNOWN_VIEW_ID, dwViewID);
		return false;
		}

	bool bMore = ((dwFlags & FLAG_MORE_ROWS) ? true : false);
	bool bIsSecondaryView = (pView->IsSecondaryView());

	//	OK to unlock

	Lock.Unlock();

	bool bRecovery = false;
	CString sDiskError;
	try
		{
		CTableDimensions Dims;
		CRowIterator Sel;

		//	Do not include nil (deleted) rows

		Sel.SetIncludeNil(false);
		if (!InitIterator(dwViewID, &Sel, &Dims, retsError))
			return false;

		//	If we have a last key, advance the selector appropriately.

		if (!dLastKey.IsNil())
			{
			CRowKey LastKey;
			if (!CRowKey::ParseKey(Dims, dLastKey, &LastKey, retsError))
				return false;

			if (!Sel.SelectKey(LastKey))
				{
				//	If not found, we return Nil

				*retdResult = CDatum();
				return true;
				}

			//	Advance one, since we want the first row after the last key.

			if (bMore)
				Sel.GetNextKey();
			}

		//	Set limits

		if (Limits.GetCount() > 0)
			Sel.SetLimits(Limits);

		//	We return an array of keys

		CComplexArray *pArray = new CComplexArray;

		while (Sel.HasMore() && (iRowCount > 0 || iRowCount == -1))
			{
			//	Get the key and data for the row

			CRowKey Key;
			CDatum dData;
			Sel.GetNextRow(&Key, &dData);

			//	Insert two elements, but only if not deleted

			if (!dData.IsNil())
				{
				if (dwFlags & FLAG_NO_KEY)
					pArray->Insert(dData);
				else if (dwFlags & FLAG_INCLUDE_KEY)
					{
					CComplexStruct *pNewData = new CComplexStruct(dData);
					if (bIsSecondaryView)
						pNewData->SetElement(FIELD_SECONDARY_KEY, Key.AsDatum(Dims));
					else
						pNewData->SetElement(FIELD_PRIMARY_KEY, Key.AsDatum(Dims));

					pArray->Insert(CDatum(pNewData));
					}
				else
					{
					pArray->Insert(Key.AsDatum(Dims));
					pArray->Insert(dData);
					}

				if (iRowCount != -1)
					iRowCount--;
				}
			}

		//	Done

		*retdResult = CDatum(pArray);
		}
	catch (CFileException e)
		{
		bRecovery = true;
		sDiskError = e.GetFilespec();
		}
	catch (CException e)
		{
		m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_EXCEPTION, CString(__FUNCTION__), e.GetErrorString()));
		*retsError = strPattern(ERR_UNKNOWN, m_sName);
		return false;
		}
	catch (...)
		{
		m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CRASH, CString(__FUNCTION__)));
		*retsError = strPattern(ERR_UNKNOWN, m_sName);
		return false;
		}

	if (bRecovery)
		{
		m_pProcess->ReportVolumeFailure(sDiskError);

		if (!RecoveryRestore())
			{
			*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
			return false;
			}

		return GetRows(dwViewID, dLastKey, iRowCount, Limits, dwFlags, retdResult, retsError);
		}

	return true;
	}

CString CAeonTable::GetTableFilenamePrefix (void)

//	GetTableFilenamePrefix
//
//	Returns a filename prefix based on the table name that is suitable
//	for filesystems

	{
	//	LATER: Remove invalid characters.
	//	0x20 or lower
	//	Symbols: '<', '>', ':', '"', '\', '/', '|', '?', '*'
	//	Symbols: '!', '@', '#', '$', '%', '^', '=', '[', ']', '{', '}'

	return m_sName;
	}

bool CAeonTable::GetTablePath (const CString &sVolume, CString *retsTablePath, CString *retsError)

//	GetTablePath
//
//	Returns the table path for the given volume.

	{
	CString sRoot = m_pStorage->GetPath(sVolume);
	if (sRoot.IsEmpty())
		{
		*retsError = strPattern(ERR_INVALID_VOLUME, sVolume);
		return false;
		}

	//	Done

	*retsTablePath = fileAppend(sRoot, m_sName);
	return true;
	}

CString CAeonTable::GetUniqueSegmentFilespec (CString *retsBackup)

//	GetUniqueSegmentFilespec
//
//	Returns a unique filename for a new segment

	{
	//	Pick the primary volume

	const CString &sVolPath = m_pStorage->GetPath(m_sPrimaryVolume);

	//	Make sure name is unique

	CString sFilespec;
	CString sRelativePath;
	do
		{
		sRelativePath = strPattern(FILESPEC_SEGMENT_CONS, m_sName, HIDWORD(m_Seq), LODWORD(m_Seq), (DWORD)mathRandom(1, 65535));
		sFilespec = fileAppend(sVolPath, sRelativePath);
		}
	while (fileExists(sFilespec));

	//	Done

	if (!m_bBackupLost)
		*retsBackup = fileAppend(m_pStorage->GetPath(m_sBackupVolume), sRelativePath);
	else
		*retsBackup = NULL_STR;

	return sFilespec;
	}

bool CAeonTable::GetViewStatus (DWORD dwViewID, bool *retbUpToDate, CString *retsError)

//	GetViewStatus
//
//	Returns the status of the view

	{
	CSmartLock Lock(m_cs);

	CAeonView *pView = m_Views.GetAt(dwViewID);
	if (pView == NULL)
		{
		*retsError = strPattern(ERR_UNKNOWN_VIEW_ID, dwViewID);
		return false;
		}

	//	Done

	*retbUpToDate = pView->IsUpToDate();
	return true;
	}

SEQUENCENUMBER CAeonTable::GetVolumeSeq (const CString &sVolume)

//	GetVolumeSeq
//
//	Returns the sequence number of the highest segment in the volume.

	{
	int i;

	//	Get the table path

	CString sError;
	CString sTablePath;
	if (!GetTablePath(sVolume, &sTablePath, &sError))
		return 0;

	//	Get the list of segment files (We just enumerate the directory for
	//	files of the proper name. Why? Because otherwise we would need a two-phase
	//	commit to create a new segment file.)

	TArray<CString> SegmentFilespecs;
	GetSegmentFilespecs(sTablePath, &SegmentFilespecs);

	//	We remember the highest sequence in any segment

	SEQUENCENUMBER HighSeq = 0;

	//	Load all segments

	for (i = 0; i < SegmentFilespecs.GetCount(); i++)
		{
		CAeonSegment::SInfo SegInfo;
		if (!CAeonSegment::GetInfo(SegmentFilespecs[i], &SegInfo))
			continue;

		//	Get the sequence number

		if (SegInfo.Seq > HighSeq)
			HighSeq = SegInfo.Seq;
		}

	//	Done

	return HighSeq;
	}

bool CAeonTable::Housekeeping (DWORD dwMaxMemoryUse)

//	Housekeeping
//
//	This is called periodically to let the table persist itself.
//	Returns FALSE if there is an error.

	{
	CSmartLock Lock(m_cs);

	//	If some other thread is doing something, then skip.

	if (m_iHousekeeping != stateReady)
		return true;

	//	If we don't have a primary volume then we can't continue. We just wait
	//	for it to be restored.

	if (m_bPrimaryLost)
		return true;

	//	Every 15 minutes or so, check to see if the backup volume has all the
	//	segments it needs.

	if (!m_bBackupNeeded
			&& !m_sBackupVolume.IsEmpty()
			&& (m_bValidateBackup || (mathRandom(1, 60) == 1)))
		HousekeepingValidateBackup();

	//	Process

	DWORD dwViewID;

	//	If we need to do a backup, do it now.

	if (m_bBackupNeeded)
		return HousekeepingBackup(Lock);

	//	If we have any secondary views that need to be updated then we do that
	//	now before we merge any segments.

	else if ((dwViewID = FindSecondaryViewToUpdate()) != 0)
		return HousekeepingUpdateView(Lock, dwViewID);

	//	See if we need to flush our data

	else if (SaveChangesNeeded())
		{
		CString sError;
		if (!Save(&sError))
			{
			m_pProcess->Log(MSG_LOG_ERROR, strPattern("Unable to save table %s: %s", GetName(), sError));
			return false;
			}

		m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_SAVE_COMPLETE, m_sName));
		return true;
		}

	//	Otherwise, merge segments

	else
		return HousekeepingMergeSegments(Lock);
	}

bool CAeonTable::Init (const CString &sTablePath, CDatum dDesc, CString *retsError)

//	Init
//
//	Initialize the table from a descriptor.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Initialize the process used to evaluate functions

	if (!m_Process.LoadStandardLibraries(retsError))
		return false;

	//	Parse the table name

	m_sName = dDesc.GetElement(FIELD_NAME);
	if (m_sName.IsEmpty())
		{
		*retsError = CString("Cannot create a table with no name.");
		return false;
		}

	//	Get the volume info.
	//
	//	NOTE: We cannot assume that the volume path is valid at this point. We 
	//	need to use the table path passed in to this function.

	m_sPrimaryVolume = dDesc.GetElement(FIELD_PRIMARY_VOLUME);
	if (m_sPrimaryVolume.IsEmpty())
		m_bPrimaryLost = true;

	m_sBackupVolume = dDesc.GetElement(FIELD_BACKUP_VOLUMES).GetElement(0);
	if (m_sBackupVolume.IsEmpty())
		m_bBackupLost = true;

	//	Parse the table type

	if (!ParseTableType(dDesc.GetElement(FIELD_TYPE), &m_iType, retsError))
		return false;

	//	The default view is special

	CAeonView *pDefaultView = m_Views.Insert(DEFAULT_VIEW);
	pDefaultView->SetID(DEFAULT_VIEW);

	//	If this is a file table, then we hard-code the dimensions

	if (m_iType == typeFile)
		{
		if (!pDefaultView->InitAsFileView(GetRecoveryFilespec(sTablePath, 0), &m_iRowsRecovered, retsError))
			return false;
		}

	//	Otherwise, parse the dimensions

	else
		{
		if (!pDefaultView->InitAsPrimaryView(dDesc, GetRecoveryFilespec(sTablePath, 0), &m_iRowsRecovered, retsError))
			return false;
		}

	//	If we recovered rows then we need to increment the sequence number (but
	//	we have to wait until the segment files are loaded).


	//	Add secondary views

	CDatum dSecondaryViews = dDesc.GetElement(FIELD_SECONDARY_VIEWS);
	for (i = 0; i < dSecondaryViews.GetCount(); i++)
		{
		CDatum dViewDesc = dSecondaryViews.GetElement(i);
		DWORD dwViewID = (DWORD)(int)dViewDesc.GetElement(FIELD_ID);

		//	If no ID, then we assume that we are creating a new table, so we
		//	assign an ID.

		if (dwViewID == 0)
			dwViewID = i + 1;

		CAeonView *pView = m_Views.Insert(dwViewID);
		pView->SetID(dwViewID);

		//	Initialize

		if (!pView->InitAsSecondaryView(dViewDesc, m_Process, GetRecoveryFilespec(sTablePath, dwViewID), false, retsError))
			return false;
		}

	//	If any of the views are using an old version of the recovery file, then
	//	log it.

	for (i = 0; i < m_Views.GetCount(); i++)
		if (m_Views[i].GetRecoveryFileVersion() == 0)
			m_pProcess->Log(MSG_LOG_INFO, strPattern(ERR_OLD_RECOVERY_FILE, m_sName, m_Views[i].GetName()));

	//	Init

	m_Seq = 1;

	//	Stats

	CDatum dLastSaveOn = dDesc.GetElement(FIELD_LAST_SAVE_ON);
	m_dwLastSave = ((const CDateTime &)dLastSaveOn).AsTick();

	CDatum dLastUpdateOn = dDesc.GetElement(FIELD_LAST_UPDATE_ON);
	m_dwLastUpdate = ((const CDateTime &)dLastUpdateOn).AsTick();

	//	Done

	return true;
	}

bool CAeonTable::InitFromFileDownloadDesc (CDatum dFileDownloadDesc, SFileDataOptions &retOptions, CString *retsError)

//	InitFromFileDownloadDesc
//
//	Initializes options from a fileDownloadDesc structure. Returns FALSE if 
//	the parameters are invalid.

	{
	CDatum dPartialMaxSize = dFileDownloadDesc.GetElement(FIELD_PARTIAL_MAX_SIZE);
	retOptions.iMaxSize = (dPartialMaxSize.IsNil() ? -1 : (int)dPartialMaxSize);

	CDatum dPartialPos = dFileDownloadDesc.GetElement(FIELD_PARTIAL_POS);
	retOptions.iPos = (dPartialPos.IsNil() ? 0 : (int)dPartialPos);
	if (retOptions.iPos < 0)
		{
		if (retsError) *retsError = ERR_PARTIAL_POS_CANNOT_BE_NEGATIVE;
		return false;
		}

	retOptions.IfModifiedAfter = dFileDownloadDesc.GetElement(FIELD_IF_MODIFIED_AFTER);

	const CString &sDataType = dFileDownloadDesc.GetElement(FIELD_DATA_TYPE);
	if (sDataType.IsEmpty() || strEquals(sDataType, FILE_DATA_TYPE_BINARY))
		retOptions.iDataType = FileDataType::binary;
	else if (strEquals(sDataType, FILE_DATA_TYPE_TEXT))
		retOptions.iDataType = FileDataType::text;
	else
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_FILE_DATA_TYPE, sDataType);
		return false;
		}

	return true;
	}

bool CAeonTable::InitIterator (DWORD dwViewID, CRowIterator *retIterator, CTableDimensions *retDims, CString *retsError)

//	InitIterator
//
//	Initialize an iterator

	{
	CSmartLock Lock(m_cs);

	//	Find the view

	CAeonView *pView = m_Views.GetAt(dwViewID);
	if (pView == NULL)
		{
		if (retsError)
			*retsError = strPattern(ERR_UNKNOWN_VIEW_ID, dwViewID);
		return false;
		}

	//	If the view is not ready, then we cannot continue

	if (!pView->IsUpToDate())
		{
		if (retsError)
			*retsError = strPattern(ERR_VIEW_NOT_READY, pView->GetName());
		return false;
		}

	//	Get dimensions

	if (retDims)
		*retDims = pView->GetDimensions();

	//	Let the view initialize

	if (!pView->InitIterator(retIterator))
		{
		if (retsError)
			*retsError = strPattern(STR_ERROR_BAD_ITERATOR, m_sName);
		return false;
		}

	//	Done

	return true;
	}

AEONERR CAeonTable::Insert (const CRowKey &Path, CDatum dData, bool bInsertNew, CString *retsError)

//	Insert
//
//	Insert the data into the table.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Make sure we have the primary volume

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return AEONERR_FAIL;
		}

	//	If we have secondary views then we need the previous value.
	//	If we insert only if new, then we also need the previous value.

	CDatum dOldData;
	SEQUENCENUMBER RowID = 0;
	if (HasSecondaryViews() || bInsertNew)
		{
		if (!GetData(DEFAULT_VIEW, Path, &dOldData, &RowID, retsError))
			return AEONERR_FAIL;

		//	If we have a new row, we use the current sequence number as a
		//	rowID. Otherwise we get the rowID from the existing row.

		if (dOldData.IsNil())
			RowID = m_Seq;
		}

	//	Otherwise, if we don't need to get the old row, then we just update the
	//	rowID even if we're overwriting. [We need to do this so that we always
	//	have unique rowIDs in case we ever add a secondary view later.]

	else
		RowID = m_Seq;

	//	If we have old data and we only insert if new, then nothing to do.

	if (bInsertNew && !dOldData.IsNil())
		{
		*retsError = ERR_PATH_EXISTS;
		return AEONERR_ALREADY_EXISTS;
		}

	//	See if we can insert. We try first before we do anything so that we
	//	can insert atomically to all views

	for (i = 0; i < m_Views.GetCount(); i++)
		if (m_Views[i].IsValid() && !m_Views[i].CanInsert(Path, dData, retsError))
			return AEONERR_FAIL;

	//	Get the dimensions of the primary view

	const CTableDimensions &PrimaryDims = m_Views.GetAt(DEFAULT_VIEW)->GetDimensions();

	//	Insert in all views. This method cannot (should not) fail since
	//	we checked above.

	bool bFallback = false;
	for (i = 0; i < m_Views.GetCount(); i++)
		if (m_Views[i].IsValid())
			{
			bool bLogFailed;
			CString sError;

			m_Views[i].Insert(PrimaryDims, m_Process, Path, dData, dOldData, RowID, &bLogFailed, &sError);
			if (bLogFailed)
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_VIEW_INSERT_FAILURE, m_Views[i].GetName(), sError));
				bFallback = true;
				}
			}

	//	Increment sequence number

	m_Seq++;
	m_dwLastUpdate = ::sysGetTickCount64();

	//	If we failed to log to the recovery file then it means that the primary
	//	volume is bad, so we switch to the back up.

	if (bFallback)
		{
		m_pProcess->ReportVolumeFailure(fileAppend(fileAppend(m_pStorage->GetPath(m_sPrimaryVolume), m_sName), FILESPEC_RECOVERY_DIR), OP_INSERT);

		if (!RecoveryRestore())
			{
			*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
			return AEONERR_FAIL;
			}
		}

	//	Done

	return AEONERR_OK;
	}

bool CAeonTable::LastComponentOfKeyIsDirectory (const CString &sKey)

//	LastComponentOfKeyIsDirectory
//
//	Returns TRUE if the file table key is of the form:
//
//	.../[ddd]

	{
	//	Edge conditions

	if (sKey.GetLength() < 3)
		return false;

	char *pLastChar = sKey.GetParsePointer() + sKey.GetLength() - 1;
	return (*pLastChar == ']');
	}

void CAeonTable::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	//	Mark all views

	for (i = 0; i < m_Views.GetCount(); i++)
		m_Views[i].Mark();

	//	Mark our process
	
	m_Process.Mark();

	//	Mark upload sessions

	m_UploadSessions.Mark();

	//	Since we know the world is stopped we take this opportunity to collect
	//	garbage (i.e., delete some unused structures).

	CollectGarbage();
	}

bool CAeonTable::MoveToScrap (const CString &sFilespec)

//	MoveToScrap
//
//	Moves the file to the scrap

	{
	CString sFilename = fileGetFilename(sFilespec);
	CString sPath = m_pStorage->GetVolumePath(sFilespec);
	if (sPath.IsEmpty())
		return false;

	CString sDestFilespec = fileAppend(sPath, strPattern(FILESPEC_SCRAP_CONS, m_sName, sFilename));
	if (!fileMove(sFilespec, sDestFilespec))
		{
		CString sToTablePath = fileAppend(sPath, m_sName);

		//	If this fails, then make sure the scrap directory exists

		filePathCreate(fileAppend(sToTablePath, FILESPEC_SCRAP_DIR));

		//	Now try again

		if (!fileMove(sFilespec, sDestFilespec))
			{
			//	LATER: Add to a list to move later.
			return false;
			}

		//	Success!

		return true;
		}

	return true;
	}

AEONERR CAeonTable::Mutate (const CRowKey &Path, CDatum dData, CDatum dMutateDesc, CDatum *retdResult, CString *retsError)

//	Mutate
//
//	Mutates a row

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Make sure we have the primary volume

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return AEONERR_FAIL;
		}

	//	Get the dimensions of the primary view

	const CTableDimensions &PrimaryDims = m_Views.GetAt(DEFAULT_VIEW)->GetDimensions();

	//	We always start with the original value

	CDatum dOriginalData;
	SEQUENCENUMBER RowID;
	CRowKey NewPath;
	const CRowKey *pPath;
	bool bReturnPrimaryKey;

	//	If we have a NULL path then it means that we need to generate a unique
	//	key for the row.

	if (Path.GetCount() == 0)
		{
		RowID = m_Seq;

		if (!CreatePrimaryKey(PrimaryDims, dMutateDesc, RowID, &NewPath, retsError))
			return AEONERR_FAIL;

		pPath = &NewPath;
		bReturnPrimaryKey = true;
		}

	//	Otherwise, use the path to load the original value (if any).

	else 
		{
		pPath = &Path;

		if (!GetData(DEFAULT_VIEW, *pPath, &dOriginalData, &RowID, retsError))
			return AEONERR_FAIL;

		//	If we have a new row, we use the current sequence number as a
		//	rowID. Otherwise we get the rowID from the existing row.

		if (dOriginalData.IsNil())
			RowID = m_Seq;

		bReturnPrimaryKey = false;
		}

	//	Create a structure that we can modify

	CComplexStruct *pResult = new CComplexStruct(dOriginalData);
	CDatum dNewData(pResult);

	//	Get the current date so we can consistently set the modified time
	//	(in case multiple fields want it).

	CDateTime Now(CDateTime::Now);

	//	Loop over all fields (in both dData and dMutateDesc)

	int iData = 0;
	int iMutate = 0;
	while (true)
		{
		//	Do we have a value field?

		CString sField;
		CDatum dValue;
		if (iData < dData.GetCount())
			{
			sField = dData.GetKey(iData);
			dValue = dData.GetElement(iData);
			}

		//	Do we have a mutate desc?

		CString sMutateField;
		CString sOp;
		if (iMutate < dMutateDesc.GetCount())
			{
			sMutateField = dMutateDesc.GetKey(iMutate);
			sOp = dMutateDesc.GetElement(iMutate);
			}

		//	If we're out of mutate fields and data fields then we're done

		if (sField.IsEmpty() && sMutateField.IsEmpty())
			break;

		//	If field and mutate field match, then follow instructions;
		//	we advance both.

		int iCompare = KeyCompare(sField, sMutateField);
		if (iCompare == 0)
			{
			iData++;
			iMutate++;
			}

		//	If the field is (alphabetically) before the mutate desc then it 
		//	means that we have no mutate instructure for the field.

		else if (!sField.IsEmpty() && (sMutateField.IsEmpty() || iCompare == -1))
			{
			sOp = MUTATE_WRITE;

			iData++;
			}

		//	Otherwise, we interpret the mutate op with no data

		else
			{
			sField = sMutateField;
			dValue = CDatum();

			iMutate++;
			}

		//	primaryKey is a special field name so we never process it.

		if (strEquals(sField, FIELD_PRIMARY_KEY))
			;

		//	Now process the mutate operation

		else if (strEquals(sOp, MUTATE_ADD_TO_SET))
			{
			if (dValue.GetCount() > 0)
				{
				CComplexArray *pNewArray = new CComplexArray(pResult->GetElement(sField));
				for (i = 0; i < dValue.GetCount(); i++)
					{
					CDatum dElement = dValue.GetElement(i);

					//	Add it if not already there

					if (!pNewArray->FindElement(dElement))
						pNewArray->Insert(dElement);
					}

				pResult->SetElement(sField, CDatum(pNewArray));
				}
			}

		else if (strEquals(sOp, MUTATE_APPEND))
			{
			if (dValue.GetCount() > 0)
				{
				CComplexArray *pNewArray = new CComplexArray(pResult->GetElement(sField));
				if (dValue.GetBasicType() == CDatum::typeStruct)
					pNewArray->Insert(dValue);
				else
					{
					for (i = 0; i < dValue.GetCount(); i++)
						pNewArray->Insert(dValue.GetElement(i));
					}

				pResult->SetElement(sField, CDatum(pNewArray));
				}
			}

		else if (strEquals(sOp, MUTATE_CONSUME))
			{
			CNumberValue Number(pResult->GetElement(sField));
			CNumberValue Consume(dValue);
			if (dValue.IsNil())
				Consume.SetInteger(1);

			//	If the value < value-to-consume then we fail

			if (Number.Compare(Consume) == -1)
				{
				*retsError = strPattern(ERR_CANNOT_CONSUME, Path.AsDatum(PrimaryDims).AsString(), sField, pResult->GetElement(sField).AsString());
				return AEONERR_OUT_OF_DATE;
				}

			Number.Subtract(Consume.GetDatum());
			pResult->SetElement(sField, Number.GetDatum());
			}

		else if (strEquals(sOp, MUTATE_DATE_CREATED))
			{
			if (dOriginalData.IsNil())
				pResult->SetElement(sField, CDatum(Now));
			}

		else if (strEquals(sOp, MUTATE_DATE_MODIFIED))
			pResult->SetElement(sField, CDatum(Now));

		else if (strEquals(sOp, MUTATE_DECREMENT))
			{
			CDatum dOriginal = pResult->GetElement(sField);
			CNumberValue Number(dOriginal.IsNil() ? CDatum((int)0) : dOriginal);
			Number.Subtract(dValue.IsNil() ? CDatum(1) : dValue);
			pResult->SetElement(sField, Number.GetDatum());
			}

		else if (strEquals(sOp, MUTATE_DELETE))
			pResult->DeleteElement(sField);

		else if (strEquals(sOp, MUTATE_IGNORE))
			;

		else if (strEquals(sOp, MUTATE_INCREMENT))
			{
			CDatum dOriginal = pResult->GetElement(sField);
			CNumberValue Number(dOriginal.IsNil() ? CDatum((int)0) : dOriginal);
			Number.Add(dValue.IsNil() ? CDatum(1) : dValue);
			pResult->SetElement(sField, Number.GetDatum());
			}

		else if (strEquals(sOp, MUTATE_MAX))
			{
			CNumberValue Number(pResult->GetElement(sField));
			if (Number.Compare(dValue) == -1)
				pResult->SetElement(sField, dValue);
			}

		else if (strEquals(sOp, MUTATE_MIN))
			{
			CNumberValue Number(pResult->GetElement(sField));
			if (Number.Compare(dValue) == 1)
				pResult->SetElement(sField, dValue);
			}

		else if (strEquals(sOp, MUTATE_PREPEND))
			{
			if (dValue.GetCount() > 0)
				{
				CComplexArray *pNewArray = new CComplexArray(pResult->GetElement(sField));
				if (dValue.GetBasicType() == CDatum::typeStruct)
					pNewArray->Insert(dValue, 0);
				else
					{
					for (i = 0; i < dValue.GetCount(); i++)
						pNewArray->Insert(dValue.GetElement(i), i);
					}

				pResult->SetElement(sField, CDatum(pNewArray));
				}
			}

		else if (strEquals(sOp, MUTATE_PRIMARY_KEY))
			{
			pResult->SetElement(sField, pPath->AsDatum(PrimaryDims));
			}

		else if (strEquals(sOp, MUTATE_REMOVE_FROM_SET))
			{
			if (dValue.GetCount() > 0)
				{
				CComplexArray *pNewArray = new CComplexArray(pResult->GetElement(sField));
				for (i = 0; i < dValue.GetCount(); i++)
					{
					CDatum dElement = dValue.GetElement(i);

					//	Remove it if in the array

					int iIndex;
					if (pNewArray->FindElement(dElement, &iIndex))
						pNewArray->Delete(iIndex);
					}

				pResult->SetElement(sField, CDatum(pNewArray));
				}
			}

		else if (strEquals(sOp, MUTATE_ROW_ID))
			{
			//	Only on create

			if (dOriginalData.IsNil())
				{
				//	If we the rowID is the key and if the key is utf8, then store
				//	the rowID as utf8.

				if (Path.GetCount() == 0 && PrimaryDims[0].iKeyType == keyUTF8)
					pResult->SetElement(sField, CDatum(RowID).AsString());

				//	Otherwise store rowID as a 64-bit integer

				else
					pResult->SetElement(sField, CDatum(RowID));
				}
			}

		else if (strEquals(sOp, MUTATE_UPDATE_GREATER))
			{
			CDatum dCurrent = pResult->GetElement(sField);

			if (dCurrent.IsNil())
				pResult->SetElement(sField, dValue);
			else
				{
				CNumberValue Number(dCurrent);
				if (Number.Compare(dValue) != -1)
					{
					*retsError = strPattern(ERR_UPDATE_GREATER_OUT_OF_DATE, Path.AsDatum(PrimaryDims).AsString(), sField, dValue.AsString(), dCurrent.AsString());
					return AEONERR_OUT_OF_DATE;
					}

				pResult->SetElement(sField, dValue);
				}
			}

		else if (strEquals(sOp, MUTATE_UPDATE_GREATER_NO_ERROR))
			{
			CDatum dCurrent = pResult->GetElement(sField);

			if (dCurrent.IsNil())
				pResult->SetElement(sField, dValue);
			else
				{
				CNumberValue Number(dCurrent);
				if (Number.Compare(dValue) != -1)
					{
					if (retdResult)
						*retdResult = CDatum();
					return AEONERR_OK;
					}

				pResult->SetElement(sField, dValue);
				}
			}

		else if (strEquals(sOp, MUTATE_UPDATE_NIL))
			{
			if (!pResult->GetElement(sField).IsNil())
				{
				*retsError = strPattern(ERR_UPDATE_NIL, Path.AsDatum(PrimaryDims).AsString(), sField);
				return AEONERR_OUT_OF_DATE;
				}

			pResult->SetElement(sField, dValue);
			}

		else if (strEquals(sOp, MUTATE_UPDATE_NIL_NO_ERROR))
			{
			if (!pResult->GetElement(sField).IsNil())
				{
				if (retdResult)
					*retdResult = CDatum();
				return AEONERR_OK;
				}

			pResult->SetElement(sField, dValue);
			}

		else if (strEquals(sOp, MUTATE_UPDATE_VERSION))
			{
			CDatum dCurrent = pResult->GetElement(sField);

			if (dCurrent.IsNil())
				pResult->SetElement(sField, CDatum((int)1));
			else
				{
				CNumberValue Number(dCurrent);
				if (Number.Compare(dValue) != 0)
					{
					*retsError = strPattern(ERR_UPDATE_VERSION_OUT_OF_DATE, Path.AsDatum(PrimaryDims).AsString(), sField, dValue.AsString(), dCurrent.AsString());
					return AEONERR_OUT_OF_DATE;
					}

				Number.Add(CDatum((int)1));
				pResult->SetElement(sField, Number.GetDatum());
				}
			}

		else if (strEquals(sOp, MUTATE_UPDATE_VERSION_NO_ERROR))
			{
			CDatum dCurrent = pResult->GetElement(sField);

			if (dCurrent.IsNil())
				pResult->SetElement(sField, CDatum((int)1));
			else
				{
				CNumberValue Number(dCurrent);
				if (Number.Compare(dValue) != 0)
					{
					if (retdResult)
						*retdResult = CDatum();
					return AEONERR_OK;
					}

				Number.Add(CDatum((int)1));
				pResult->SetElement(sField, Number.GetDatum());
				}
			}

		else if (strEquals(sOp, MUTATE_WRITE))
			pResult->SetElement(sField, dValue);

		else if (strEquals(sOp, MUTATE_WRITE_NEW))
			{
			if (pResult->GetElement(sField).IsNil())
				pResult->SetElement(sField, dValue);
			}
		else
			{
			*retsError = strPattern(ERR_UNKNOWN_MUTATE_OP, sOp);
			return AEONERR_FAIL;
			}
		}

	//	See if we can insert. We try first before we do anything so that we
	//	can insert atomically to all views

	for (i = 0; i < m_Views.GetCount(); i++)
		if (m_Views[i].IsValid() && !m_Views[i].CanInsert(*pPath, dNewData, retsError))
			return AEONERR_FAIL;

	//	Insert in all views. This method cannot (should not) fail since
	//	we checked above.

	bool bFallback = false;
	for (i = 0; i < m_Views.GetCount(); i++)
		if (m_Views[i].IsValid())
			{
			bool bLogFailed;

			m_Views[i].Insert(PrimaryDims, m_Process, *pPath, dNewData, dOriginalData, RowID, &bLogFailed);
			if (bLogFailed)
				bFallback = true;
			}

	//	Increment sequence number

	m_Seq++;
	m_dwLastUpdate = ::sysGetTickCount64();

	//	If we failed to log to the recovery file then it means that the primary
	//	volume is bad, so we switch to the back up.

	if (bFallback)
		{
		m_pProcess->ReportVolumeFailure(fileAppend(fileAppend(m_pStorage->GetPath(m_sPrimaryVolume), m_sName), FILESPEC_RECOVERY_DIR), OP_MUTATE);

		if (!RecoveryRestore())
			{
			*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
			return AEONERR_FAIL;
			}
		}

	//	If necessary, return the primary key in the data

	if (bReturnPrimaryKey)
		dNewData.SetElement(FIELD_PRIMARY_KEY, pPath->AsDatum(PrimaryDims));

	//	Done

	if (retdResult)
		*retdResult = dNewData;

	return AEONERR_OK;
	}

bool CAeonTable::OnVolumesChanged (const TArray<CString> &VolumesDeleted)

//	OnVolumesChanged
//
//	Volumes have changed. Returns TRUE if the table changed in some way.

	{
	CSmartLock Lock(m_cs);
	int i;
	CString sError;
	bool bSaveDesc = false;

	//	See if either the primary volume or the backup volume have been deleted.

	bool bPrimaryDeleted = false;
	bool bBackupDeleted = false;
	for (i = 0; i < VolumesDeleted.GetCount(); i++)
		if (strEquals(m_sPrimaryVolume, VolumesDeleted[i]))
			bPrimaryDeleted = true;
		else if (strEquals(m_sBackupVolume, VolumesDeleted[i]))
			bBackupDeleted = true;

	//	If both volumes have been deleted then we are really screwed--we just
	//	have to hope that the volumes come back eventually.
	//
	//	We go to state CC.

	if (bPrimaryDeleted && bBackupDeleted)
		RecoveryFailure();

	//	If the backup volume was deleted then we need to pick a new backup
	//	volume and generate backups from the primary.

	else if (bBackupDeleted)
		{
		//	If we still have the primary, then we just need to schedule a backup

		if (!m_bPrimaryLost)
			RecoveryBackup();

		//	Otherwise, we've lost all data

		else
			RecoveryFailure();
		}

	//	If the primary volume was deleted then we need to pick a new primary
	//	and restore from backup.

	else if (bPrimaryDeleted)
		{
		//	Try to restore from backup

		RecoveryRestore();
		}

	//	If we have no storage then see if a newly added volume restores our
	//	data. NOTE: For now we assume that the volume was restored intact.

	else if (m_bPrimaryLost)
		{
		//	Look for all the volumes that have table data

		TArray<CString> Volumes;
		if (FindTableVolumes(&Volumes))
			{
			//	Find a volume that has the highest sequence number.

			int iVolumeToUse = FindVolumeToOpen(Volumes);

			//	Swap to that volume. [We treat that as the backup and restore
			//	from it.]

			m_bBackupLost = false;
			m_sBackupVolume = Volumes[iVolumeToUse];
			m_bBackupNeeded = false;

			RecoveryRestore();
			}
		}

	//	If we've lost the backup and are waiting for a new volume, then see
	//	if we've got one.

	else if (m_bBackupLost && !m_bBackupNeeded)
		RecoveryBackup();

	//	Otherwise, none of our volumes have been deleted and we don't need
	//	any new volumes.

	else
		;

	//	Done

	return true;
	}

bool CAeonTable::Open (IArchonProcessCtx *pProcess, CMachineStorage *pStorage, const CString &sName, const TArray<CString> &Volumes, CString *retsError)

//	Open
//
//	Open the table from disk

	{
	int i;

	m_pProcess = pProcess;
	m_pStorage = pStorage;

	//	We pick one of the volumes as the one to open.
	//
	//	If we have more than one volume, then pick the one with the highest
	//	sequence number.

	TArray<SEQUENCENUMBER> VolumeSeqs;
	int iVolumeToOpen = FindVolumeToOpen(Volumes, &VolumeSeqs);
	SEQUENCENUMBER BestSeq = VolumeSeqs[iVolumeToOpen];

	//	Load the descriptor from the best volume

	CDatum dDesc;
	CString sTablePath = fileAppend(m_pStorage->GetPath(Volumes[iVolumeToOpen]), sName);
	CString sDescFilespec = fileAppend(sTablePath, FILESPEC_TABLE_DESC_FILE);

	//	Initialize from the descriptor

	if (!OpenDesc(sDescFilespec, &dDesc, retsError))
		return false;

	//	Initialize the table

	if (!Init(sTablePath, dDesc, retsError))
		return false;

	//	Table name better match. If not, then we can't open this table. The user
	//	will have to manually repair it.

	if (!strEquals(sName, m_sName))
		{
		*retsError = strPattern(STR_ERROR_BAD_TABLE_NAME, sDescFilespec);
		return false;
		}

	//	So far this is what we've got:
	//
	//	1.	The table exists on one or more volumes (in the Volumes array).
	//
	//	2.	m_sPrimaryVolume has one of the following values:
	//
	//		= blank, meaning the volume was missing at last shutdown.
	//			(and m_bPrimaryLost is TRUE). In this case we set
	//			m_sPrimaryVolume to one of the existing Volumes and clear
	//			the m_bPrimaryLost flag.
	//
	//		= a volume in Volumes. In this case there is nothing to do.
	//			The volume is fine.
	//
	//		= a volume not in Volumes, but in m_pStorage. In this case
	//			we set m_sPrimaryVolume to one of the existing Volumes.
	//
	//		= a volume not in Volumes or m_pStorage. In this case we set
	//			m_sPrimaryVolume to one of the existing Volumes.
	//
	//	3.	m_sBackupVolume has the same possible states.
	//
	//	NOTE: m_sPrimaryVolume is not necessarily the same as the volume
	//	that we opened.

	//	Determine whether m_sPrimaryVolume is one of the volumes that has
	//	the table data.

	int iPrimaryIndex;
	bool bPrimaryOK = Volumes.Find(m_sPrimaryVolume, &iPrimaryIndex);
	int iBackupIndex;
	bool bBackupOK = Volumes.Find(m_sBackupVolume, &iBackupIndex);
	bool bSaveDesc = false;

	//	If m_sPrimaryVolume is not set properly, or if it not the volume that
	//	we opened, then we need to change it to the current.

	if (!bPrimaryOK || iPrimaryIndex != iVolumeToOpen)
		{
		iPrimaryIndex = iVolumeToOpen;
		m_sPrimaryVolume = Volumes[iVolumeToOpen];
		m_bPrimaryLost = false;

		m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_MOVING_PRIMARY, m_sName, m_sPrimaryVolume));

		//	If necessary we find a new backup volume.

		if (strEquals(m_sPrimaryVolume, m_sBackupVolume))
			{
			m_sBackupVolume = NULL_STR;
			bBackupOK = false;
			}

		bSaveDesc = true;
		}

	//	If the backup is not OK then we need to set it to some volume.

	if (!bBackupOK || VolumeSeqs[iBackupIndex] < BestSeq)
		{
		//	Find another volume at the same sequence number.

		iBackupIndex = -1;
		for (i = 0; i < Volumes.GetCount(); i++)
			if (i != iPrimaryIndex && VolumeSeqs[i] == BestSeq)
				{
				iBackupIndex = i;
				break;
				}

		//	If we found it, then use it.

		if (iBackupIndex != -1)
			{
			m_sBackupVolume = Volumes[iBackupIndex];
			m_bBackupLost = false;
			m_bBackupNeeded = false;

			m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_MOVING_BACKUP, m_sName, m_sBackupVolume));
			}

		//	Otherwise we need to find another volume and backup the primary to it.

		else
			{
			m_bBackupLost = true;
			m_sBackupVolume = m_pStorage->GetRedundantVolume(m_sPrimaryVolume);
			m_bBackupNeeded = !m_sBackupVolume.IsEmpty();

			if (m_bBackupNeeded)
				m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_NEW_BACKUP, m_sName, m_sBackupVolume));
			else
				m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_BACKUP_LOST, m_sName));
			}

		bSaveDesc = true;
		}

	//	Now the we have valid primary and secondary volumes, initialize upload sessions

	m_UploadSessions.Init(m_pStorage, m_sName, m_sPrimaryVolume, m_sBackupVolume);
		
	//	Update the table desc if we changed it

	if (bSaveDesc)
		{
		//	It doesn't matter if we fail saving; this function will take care of
		//	restoring from backup and logging errors.

		SaveDesc();
		}

	//	Open all segments.
	//
	//	NOTE: The only way m_bPrimaryLost can be TRUE is if the drive failed
	//	after we read the descriptor but just before we try to save them out.
	//	In that case, we don't return an error (since we've logged) it and 
	//	wait for the drive to be restored (somehow).

	if (!m_bPrimaryLost)
		{
		if (!OpenSegments(m_sPrimaryVolume, &m_Seq, retsError))
			return false;

		//	If we had to recover some rows, then we need to bump up the sequence
		//	number.

		if (m_iRowsRecovered)
			{
			m_Seq += m_iRowsRecovered;

			m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_RECOVERED_ROWS, m_sName, m_iRowsRecovered));
			}
		}

	//	If we have a backup volume, validate the backup at our next opportunity.

	m_bValidateBackup = (!m_bBackupNeeded && !m_sBackupVolume.IsEmpty());

	//	Done

	return true;
	}

bool CAeonTable::OpenDesc (const CString &sFilespec, CDatum *retdDesc, CString *retsError)

//	OpenDesc
//
//	Open the table descriptor

	{
	return CDatum::CreateFromFile(sFilespec, CDatum::formatAEONScript, retdDesc, retsError);
	}

bool CAeonTable::OpenSegments (const CString &sVolume, SEQUENCENUMBER *retHighSeq, CString *retsError)

//	OpenSegments
//
//	Open segments from the given volume (and closes previous ones if possible).

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Queue all current segments for deletion.

	CloseSegments();

	//	Get the table path

	CString sTablePath;
	if (!GetTablePath(sVolume, &sTablePath, retsError))
		return false;

	//	Get the list of segment files (We just enumerate the directory for
	//	files of the proper name. Why? Because otherwise we would need a two-phase
	//	commit to create a new segment file.)

	TArray<CString> SegmentFilespecs;
	GetSegmentFilespecs(sTablePath, &SegmentFilespecs);

	//	We remember the highest sequence in any segment

	SEQUENCENUMBER HighSeq = 0;

	//	Load all segments

	for (i = 0; i < SegmentFilespecs.GetCount(); i++)
		{
		//	LATER: Have this happen inside of CAeonView so that we can pass the
		//	dimensions appropriately.

		CAeonSegment *pNewSeg = new CAeonSegment;
		if (!pNewSeg->Open(SegmentFilespecs[i], retsError))
			{
			//	Delete this segment

			pNewSeg->Release();

			//	Delete previous segments

			CloseSegments();

			//	Error return

			*retsError = strPattern(ERR_CANT_LOAD_SEGMENT, SegmentFilespecs[i]);
			return false;
			}

		//	Get the sequence number

		SEQUENCENUMBER Seq = pNewSeg->GetSequence();
		if (Seq > HighSeq)
			HighSeq = Seq;

		//	Add to the appropriate view

		CAeonView *pView = m_Views.GetAt(pNewSeg->GetViewID());
		if (pView == NULL)
			{
			//	Ignore the segment; we assume that it is old.

			m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_SEGMENT_FOR_INVALID_VIEW, SegmentFilespecs[i], pNewSeg->GetViewID()));
			pNewSeg->Release();
			continue;
			}

		//	Add
		//
		//	NOTE: This also sets the dimensions of the segment from the view.

		pView->InsertSegment(pNewSeg);
		}

	//	Done

	if (retHighSeq)
		*retHighSeq = HighSeq;

	return true;
	}

bool CAeonTable::ParseDimensionDesc (CDatum dDimDesc, SDimensionDesc *retDimDesc, CString *retsError)

//	ParseDimensionDesc
//
//	Parses a dimension descriptor

	{
	CDatum dKeyType = dDimDesc.GetElement(FIELD_KEY_TYPE);
	if (dKeyType.IsNil())
		{
		*retsError = CString("KeyType required for table dimension.");
		return false;
		}

	if (strEquals(dKeyType, KEY_TYPE_UTF8))
		{
		retDimDesc->iKeyType = keyUTF8;
		retDimDesc->iSort = AscendingSort;
		}
	else if (strEquals(dKeyType, KEY_TYPE_INT32))
		{
		retDimDesc->iKeyType = keyInt32;
		retDimDesc->iSort = AscendingSort;
		}
	else if (strEquals(dKeyType, KEY_TYPE_INT64))
		{
		retDimDesc->iKeyType = keyInt64;
		retDimDesc->iSort = AscendingSort;
		}
	else if (strEquals(dKeyType, KEY_TYPE_DATE_TIME))
		{
		retDimDesc->iKeyType = keyDatetime;
		retDimDesc->iSort = DescendingSort;
		}
	else if (strEquals(dKeyType, KEY_TYPE_LIST_UTF8))
		{
		retDimDesc->iKeyType = keyListUTF8;
		retDimDesc->iSort = AscendingSort;
		}
	else
		{
		*retsError = strPattern("Unknown keyType: %s", (const CString &)dKeyType);
		return false;
		}

	//	If we have a sort value, override the default above.

	CDatum dKeySort = dDimDesc.GetElement(FIELD_KEY_SORT);
	if (!dKeySort.IsNil())
		{
		if (strEquals(dKeySort, KEY_SORT_DESCENDING))
			retDimDesc->iSort = DescendingSort;
		else if (strEquals(dKeySort, KEY_SORT_ASCENDING))
			retDimDesc->iSort = AscendingSort;
		else
			{
			*retsError = strPattern("Unknown keySort: %s", (const CString &)dKeySort);
			return false;
			}
		}

	//	Done

	return true;
	}

bool CAeonTable::ParseDimensionDescForSecondaryView (CDatum dDimDesc, CHexeProcess &Process, SDimensionDesc *retDimDesc, CDatum *retdKey, CString *retsError)

//	ParseDimensionDescForSecondaryView
//
//	Parses a dimension descriptor for a secondary view.

	{
	if (!ParseDimensionDesc(dDimDesc, retDimDesc, retsError))
		return false;

	*retdKey = dDimDesc.GetElement(FIELD_KEY);
	if (retdKey->IsNil())
		{
		*retsError = ERR_KEY_REQUIRED;
		return false;
		}

	//	If the key is a function then we set its global environment to the
	//	process.

	if (strEquals(retdKey->GetTypename(), TYPENAME_HEXE_FUNCTION))
		retdKey->SetElement(FIELD_GLOBAL_ENV, Process.GetGlobalEnv());

	return true;
	}

bool CAeonTable::ParseDimensionPath (const CString &sView, CDatum dPath, CRowKey *retPath, CString *retsError)

//	ParseDimensionPath
//
//	Parses a dimension path given a view
	
	{
	//	Find the view

	CAeonView *pView;
	if (sView.IsEmpty())
		{
		pView = m_Views.GetAt(DEFAULT_VIEW);
		if (pView == NULL)
			{
			*retsError = strPattern(ERR_NO_DEFAULT_VIEW, m_sName);
			return false;
			}
		}
	else
		{
		if (!FindView(sView, &pView))
			{
			*retsError = strPattern(ERR_UNKNOWN_VIEW, sView);
			return false;
			}
		}

	return CRowKey::ParseKey(pView->GetDimensions(), dPath, retPath, retsError);
	}

bool CAeonTable::ParseDimensionPathForCreate (CDatum dPath, CRowKey *retPath, CString *retsError)
	
//	ParseDimensionPath
//
//	Parses a dimension path given a view
	
	{
	//	We always use the default view when creating a new row.

	CAeonView *pView = m_Views.GetAt(DEFAULT_VIEW);
	if (pView == NULL)
		return false;

	return CRowKey::ParseKeyForCreate(pView->GetDimensions(), dPath, retPath, retsError);
	}

bool CAeonTable::ParseFilePath (const CString &sPath, CString *retsTable, CString *retsFilePath, CString *retsError)

//	ParseFilePath
//
//	Parses a filePath

	{
	return CAeonInterface::ParseTableFilePath(sPath, retsTable, retsFilePath, retsError);
	}

bool CAeonTable::ParseFilePathForCreate (const CString &sPath, CString *retsTable, CString *retsFilePath, CString *retsError)

//	ParseFilePathForCreate
//
//	Make sure that this is a valid path for creating a new file entry

	{
	CString sFilePath;

	if (!ParseFilePath(sPath, retsTable, &sFilePath, retsError))
		return false;

	//	Parse all components of the path

	char *pPos = sFilePath.GetParsePointer();
	char *pEndPos = pPos + sFilePath.GetLength();

	//	Loop over all components

	char *pStart = NULL;
	while (pPos < pEndPos)
		{
		switch (*pPos)
			{
			case '/':
				{
				//	OK if this is first slash

				if (pStart)
					{
					//	Component better be non-empty

					if (pStart == pPos)
						{
						if (retsError)
							*retsError = strPattern(ERR_INVALID_FILE_PATH, sPath);
						return false;
						}
					}

				pStart = pPos + 1;

				//	This better not be the last character

				if (pStart == pEndPos)
					{
					if (retsError)
						*retsError = strPattern(ERR_INVALID_FILE_PATH, sPath);
					return false;
					}
				break;
				}

			default:
				{
				//	Must start with a slash

				if (pStart == NULL)
					{
					if (retsError)
						*retsError = strPattern(ERR_INVALID_FILE_PATH, sPath);
					return false;
					}

				//	Must be valid character
				//	numbers
				//	letters
				//	other unicode characters
				//	- _ . ~

				if (!strIsASCIIAlpha(pPos)					//	alpha
						&& !strIsDigit(pPos)				//	numbers
						&& *pPos != '-'
						&& *pPos != '_'
						&& *pPos != '.'
						&& *pPos != '~'
						&& !strIsASCIIHigh(pPos))			//	unicode characters
					{
					if (retsError)
						*retsError = strPattern(ERR_INVALID_FILE_PATH, sPath);
					return false;
					}

				break;
				}
			}

		//	Next

		pPos++;
		}

	//	If we get this far, we're OK

	if (retsFilePath)
		*retsFilePath = sFilePath;

	return true;
	}

bool CAeonTable::ParseTableType (const CString &sType, Types *retiType, CString *retsError)

//	ParseTableType
//
//	Parses a table type string

	{
	Types iType;
	if (sType.IsEmpty() || strEquals(sType, TABLE_TYPE_STANDARD))
		iType = typeStandard;
	else if (strEquals(sType, TABLE_TYPE_FILE))
		iType = typeFile;
	else
		{
		if (retsError)
			*retsError = strPattern(STR_ERROR_INVALID_TABLE_TYPE, sType);
		return false;
		}

	//	Done

	if (retiType)
		*retiType = iType;

	return true;
	}

CDatum CAeonTable::PrepareFileDesc (const CString &sTable, const CString &sFilePath, CDatum dFileDesc, DWORD dwFlags)

//	PrepareFileDesc
//
//	Returns a fileDesc suitable for the client

	{
	int i;

	bool bTranspace = ((dwFlags & FLAG_TRANSPACE) ? true : false);
	bool bIncludeStoragePath = ((dwFlags & FLAG_STORAGE_PATH) ? true : false);

	CComplexStruct *pNewFileDesc = new CComplexStruct;

	//	Add an absolute filePath, if necessary
	//	[This is not necessary for Transpace calls because we add the original
	//	address at a higher level.]

	if (!bTranspace)
		pNewFileDesc->SetElement(FIELD_FILE_PATH, CAeonInterface::CreateTableFilePath(sTable, sFilePath));

	//	Then we add all fields (except filePath and storagePath)

	for (i = 0; i < dFileDesc.GetCount(); i++)
		{
		const CString &sField = dFileDesc.GetKey(i);

		if (!strEquals(sField, FIELD_FILE_PATH)
				&& (bIncludeStoragePath || !strEquals(sField, FIELD_STORAGE_PATH)))
			pNewFileDesc->SetElement(sField, dFileDesc.GetElement(sField));
		}

	return CDatum(pNewFileDesc);
	}

bool CAeonTable::RecoverTableRows (CString *retsError)

//	RecoverTableRows
//
//	Assumes that m_pRows is invalid or corrupt and restores it from the recovery
//	file.

	{
	CSmartLock Lock(m_cs);
	int i, j;

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return false;
		}

	//	First we load rows from the recovery file to new row arrays.
	//	We do this for each view.

	TArray<CAeonRowArray *> Rows;
	for (i = 0; i < m_Views.GetCount(); i++)
		{
		CAeonView *pView = &m_Views[i];

		CAeonRowArray *pRows;
		if (!pView->LoadRecoveryFile(GetRecoveryFilespec(pView->GetID()), &pRows, NULL, retsError))
			{
			//	If we failed then clean up previous arrays

			for (j = 0; j < Rows.GetCount(); j++)
				Rows[j]->Release();

			return false;
			}

		Rows.Insert(pRows);
		}

	//	If we get this far then we've got valid row arrays for all views
	//	Note that we've left the recovery file intact (we haven't flushed)
	//	which is consistent with how we recover on start up.

	for (i = 0; i < m_Views.GetCount(); i++)
		{
		CAeonView *pView = &m_Views[i];

		//	This can't fail since it is just a pointer move.

		pView->SetUnsavedRows(Rows[i]);
		}

	//	Done

	return true;
	}

bool CAeonTable::RecoveryBackup (void)

//	RecoveryBackup
//
//	We've lost the backup so we need to make a new one. Returns FALSE if the
//	primary drive was lost.

	{
	CSmartLock Lock(m_cs);

	if (m_bPrimaryLost)
		return false;

	m_bBackupLost = true;

	m_sBackupVolume = m_pStorage->GetRedundantVolume(m_sPrimaryVolume);
	if (m_bBackupNeeded = !m_sBackupVolume.IsEmpty())
		m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_NEW_BACKUP, m_sName, m_sBackupVolume));
	else
		m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_BACKUP_LOST, m_sName));

	//	Let the upload object know

	m_UploadSessions.SetVolumes(m_sPrimaryVolume, m_sBackupVolume);

	//	Save the descriptors.
	//
	//	NOTE: This itself might trigger a recovery failure, since the write to
	//	the primary volume might fail.

	if (!SaveDesc())
		return false;

	return true;
	}

bool CAeonTable::RecoveryFailure (void)

//	RecoveryFailure
//
//	We've lost the primary volume and there is no backup. We move to state CC
//	and hope that a new volume comes online with our data.
//
//	Returns FALSE if the primary drive was lost (which is always the case).

	{
	CSmartLock Lock(m_cs);

	CloseSegments();

	m_bPrimaryLost = true;
	m_sPrimaryVolume = NULL_STR;
	m_bBackupLost = true;
	m_sBackupVolume = NULL_STR;

	//	Can't backup or restore to a missing volume
	m_bBackupNeeded = false;

	m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_LOST_DATA, m_sName));

	//	NOTE: Since both volumes are lost there is no point in saving a descriptor.

	return false;
	}

bool CAeonTable::RecoveryRestore (void)

//	RecoveryRestore
//
//	Converts the backup volume to a primary volume and initiates a backup.
//	Returns FALSE if the primary volume was lost.

	{
	CSmartLock Lock(m_cs);
	CString sError;

	if (m_bBackupLost)
		return RecoveryFailure();

	//	NOTE: At this point we don't bother checking to see if the backup is
	//	stale (i.e., sequence number less than current). It's probably the best
	//	we're going to get at this point.

	m_sPrimaryVolume = m_sBackupVolume;
	m_bPrimaryLost = false;
	m_bBackupLost = true;
	m_bBackupNeeded = false;

	//	We need to write out any segments in memory. We wait until the 
	//	housekeeping thread is done.

	while (m_iHousekeeping != stateReady)
		{
		Lock.Unlock();
		::Sleep(250);
		Lock.Lock();
		}

	//	Save segment to the new volume

	m_iHousekeeping = stateCreatingSegment;
	bool bSuccess = Save(&sError);
	m_iHousekeeping = stateReady;

	if (!bSuccess)
		{
		m_pProcess->Log(MSG_LOG_ERROR, sError);
		return RecoveryFailure();
		}

	//	Now close old segments and load new ones

	if (!OpenSegments(m_sPrimaryVolume, &m_Seq, &sError))
		{
		m_pProcess->Log(MSG_LOG_ERROR, sError);
		return RecoveryFailure();
		}

	//	Set the upload sessions volumes

	m_UploadSessions.SetVolumes(m_sPrimaryVolume, m_sBackupVolume);

	//	Save the descriptors.

	if (!SaveDesc())
		return false;

	m_pProcess->Log(MSG_LOG_INFO, strPattern(STR_SWAP_TO_BACKUP, m_sName));

	//	Initiate a backup

	return RecoveryBackup();
	}

bool CAeonTable::Recreate (IArchonProcessCtx *pProcess, CDatum dDesc, bool *retbUpdated, CString *retsError)

//	Recreate
//
//	Checks to see if we need to update the table definition.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Pre-fill

	if (retbUpdated)
		*retbUpdated = false;

	//	Figure out what has changed. If there are changes that cannot be applied
	//	(e.g., changing table type) then we return an error.

	TArray<CDatum> NewViews;
	if (!DiffDesc(dDesc, &NewViews, retsError))
		return false;

	//	Add new views, if necessary

	if (NewViews.GetCount() > 0)
		{
		//	First we have to figure out the highest ViewID so that we can assign
		//	new one.

		DWORD dwNextViewID = 0;

		for (i = 0; i < m_Views.GetCount(); i++)
			{
			DWORD dwViewID = m_Views[i].GetID();
			if (dwViewID > dwNextViewID)
				dwNextViewID = dwViewID;
			}

		dwNextViewID++;

		//	Now loop over all new views adding them one at a time.

		bool bSomeSuccesses = false;
		bool bSomeFailures = false;
		for (i = 0; i < NewViews.GetCount(); i++)
			{
			DWORD dwViewID = dwNextViewID++;

			CAeonView *pView = m_Views.Insert(dwViewID);
			pView->SetID(dwViewID);

			//	Initialize. If we get an error creating any one view, continue
			//	creating others.

			if (!pView->InitAsSecondaryView(NewViews[i], m_Process, GetRecoveryFilespec(dwViewID), true, retsError))
				bSomeFailures = true;
			else
				bSomeSuccesses = true;
			}

		//	If we created at least one view then we need to save the descriptors

		if (bSomeSuccesses)
			{
			SaveDesc();

			//	We definitely updated the table

			if (retbUpdated)
				*retbUpdated = true;
			}

		//	If we had at least one failure then we return an error.
		//	(retsError is initialized by the call that failed).

		if (bSomeFailures)
			return false;
		}

	//	Done

	return true;
	}

bool CAeonTable::RowExists (const CTableDimensions &Dims, CDatum dKey)

//	RowExists
//
//	Returns TRUE if a row with the given key exists.

	{
	CRowKey Key;
	CString sError;
	if (!CRowKey::ParseKeyForCreate(Dims, dKey, &Key, &sError))
		return false;

	CDatum dData;
	if (!GetData(DEFAULT_VIEW, Key, &dData, NULL, &sError))
		return false;

	return !dData.IsNil();
	}

bool CAeonTable::Save (CString *retsError)

//	Save
//
//	Save the in-memory rows to a new segment

	{
	CSmartLock Lock(m_cs);
	int i, j;

	//	If no primary, then we can't save

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return false;
		}

	//	Loop over all views

	TArray<SNewSegment> SegmentsCreated;
	for (i = 0; i < m_Views.GetCount(); i++)
		{
		CAeonView *pView = &m_Views[i];

		//	If we have no unsaved rows, then nothing to do

		if (!pView->HasUnsavedRows())
			continue;

		//	Figure out the name for the new segment

		CString sBackup;
		CString sFilespec = GetUniqueSegmentFilespec(&sBackup);

		//	Create a segment

		CAeonSegment *pNewSeg;
		if (!pView->CreateSegment(sFilespec, m_Seq, NULL, &pNewSeg, retsError))
			{
			m_pProcess->ReportVolumeFailure(sFilespec);

			//	Delete all previously created segments

			for (j = 0; j < SegmentsCreated.GetCount(); j++)
				SegmentsCreated[j].pSeg->Release();

			//	If we have a backup, switch to that.

			if (!RecoveryRestore())
				{
				*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
				return false;
				}

			//	Retry

			return Save(retsError);
			}

		//	Add the new segment to our array and remember the backup filespec
		//	also.

		SNewSegment *pEntry = SegmentsCreated.Insert();
		pEntry->dwViewID = m_Views.GetKey(i);
		pEntry->pSeg = pNewSeg;
		pEntry->sFilespec = sFilespec;
		pEntry->sBackup = sBackup;
		}

	//	Now that we've successfully created all segments, add them to the proper
	//	views and delete the in-memory rows.

	for (i = 0; i < SegmentsCreated.GetCount(); i++)
		{
		CAeonView *pView = m_Views.GetAt(SegmentsCreated[i].dwViewID);
		pView->SegmentSaveComplete(SegmentsCreated[i].pSeg);
		}

	m_dwLastSave = ::sysGetTickCount64();

	//	Save the desc so we can update the last save time and the last update 
	//	time.

	if (!SaveDesc())
		return false;

	//	Outside of the lock we create backups

	Lock.Unlock();

	if (!m_bBackupLost)
		{
		for (i = 0; i < SegmentsCreated.GetCount(); i++)
			{
			if (!fileCopy(SegmentsCreated[i].sFilespec, SegmentsCreated[i].sBackup))
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_SEGMENT_BACKUP_FAILED, SegmentsCreated[i].sBackup));

				//	LATER: Figure out which volume failed and report it.
				}
			}
		}

	//	Done

	return true;
	}

bool CAeonTable::SaveChangesNeeded () const

//	SaveChangesNeeded
//
//	Returns TRUE if we need to save changed rows out during housekeeping.
//	NOTE: We assume the table is locked.

	{
	//	If we've saved after the last update, then we're OK.

	if (m_dwLastSave >= m_dwLastUpdate)
		return false;

	//	If we've saved recently, then let changes accumulate.

	DWORDLONG dwNow = ::sysGetTickCount64();
	if (dwNow - m_dwLastSave < TABLE_SAVE_THRESHOLD)
		return false;

	//	If the last update was recent, then wait a bit in case there are more
	//	updates coming.

	if (dwNow - m_dwLastUpdate < UPDATE_FREQUENCY_THRESHOLD)
		return false;

	//	Save

	return true;
	}

bool CAeonTable::SaveDesc (void)

//	SaveDesc
//
//	Saves the descriptor to all volumes.

	{
	CString sError;
	CDatum dDesc = GetDesc();

	//	Save to primary volume.

	if (!m_bPrimaryLost)
		{
		CString sTablePath = fileAppend(m_pStorage->GetPath(m_sPrimaryVolume), m_sName);
		CString sDescFilespec = fileAppend(sTablePath, FILESPEC_TABLE_DESC_FILE);

		if (!SaveDesc(dDesc, sDescFilespec, &sError))
			{
			m_pProcess->ReportVolumeFailure(sDescFilespec);

			if (!RecoveryRestore())
				{
				m_pProcess->Log(MSG_LOG_ERROR, sError);
				return false;
				}

			return SaveDesc();
			}
		}

	//	Save to backup volume.

	if (!m_bBackupLost)
		{
		CString sTablePath = fileAppend(m_pStorage->GetPath(m_sBackupVolume), m_sName);
		CString sDescFilespec = fileAppend(sTablePath, FILESPEC_TABLE_DESC_FILE);

		if (!SaveDesc(dDesc, sDescFilespec, &sError))
			{
			m_pProcess->ReportVolumeFailure(sDescFilespec);
			return RecoveryBackup();
			}
		}

	//	Done
	
	return true;
	}

bool CAeonTable::SaveDesc (CDatum dDesc, const CString &sFilespec, CString *retsError)

//	SaveDesc
//
//	Saves the table descriptor to the path

	{
	//	Open the file

	CFile DescFile;
	if (!DescFile.Create(sFilespec, CFile::FLAG_CREATE_ALWAYS))
		{
		m_pProcess->ReportVolumeFailure(sFilespec);
		*retsError = strPattern(STR_ERROR_CANT_SAVE_DESC, sFilespec);
		return false;
		}

	//	Write

	try
		{
		dDesc.Serialize(CDatum::formatAEONScript, DescFile);
		}
	catch (...)
		{
		m_pProcess->ReportVolumeFailure(sFilespec);
		*retsError = strPattern(STR_ERROR_CANT_SAVE_DESC, sFilespec);
		return false;
		}

	//	Done

	return true;
	}

void CAeonTable::SetDimensionDesc (CComplexStruct *pDesc, const SDimensionDesc &Dim)

//	SetDimensionDesc
//
//	Writes out the dimension desc to the given structure.

	{
	switch (Dim.iKeyType)
		{
		case keyUTF8:
			pDesc->SetElement(FIELD_KEY_TYPE, KEY_TYPE_UTF8);
			break;

		case keyInt32:
			pDesc->SetElement(FIELD_KEY_TYPE, KEY_TYPE_INT32);
			break;

		case keyInt64:
			pDesc->SetElement(FIELD_KEY_TYPE, KEY_TYPE_INT64);
			break;

		case keyDatetime:
			pDesc->SetElement(FIELD_KEY_TYPE, KEY_TYPE_DATE_TIME);
			break;

		case keyListUTF8:
			pDesc->SetElement(FIELD_KEY_TYPE, KEY_TYPE_LIST_UTF8);
			break;

		default:
			ASSERT(false);
		}

	switch (Dim.iSort)
		{
		case AscendingSort:
			pDesc->SetElement(FIELD_KEY_SORT, KEY_SORT_ASCENDING);
			break;

		case DescendingSort:
			pDesc->SetElement(FIELD_KEY_SORT, KEY_SORT_DESCENDING);
			break;

		default:
			ASSERT(false);
		}
	}

bool CAeonTable::UpdateFileDesc (const CString &sFilePath, CDatum dFileDesc, CDatum *retdResult)

//	UpdateFileDesc
//
//	Updates the file descriptor (without modifying the actual file).

	{
	CSmartLock Lock(m_cs);

	//	Generate a path

	CRowKey Path;
	CRowKey::CreateFromFilePath(sFilePath, &Path);

	//	Look up the file in the database.

	CString sError;
	CDatum dCurFileDesc;
	if (!GetData(DEFAULT_VIEW, Path, &dCurFileDesc, NULL, &sError))
		{
		if (retdResult) *retdResult = sError;
		return false;
		}

	//	If the file does not exist, then this is an error.

	if (dCurFileDesc.IsNil())
		{
		if (retdResult) *retdResult = strPattern(ERR_FILE_NOT_FOUND, sFilePath);
		return false;
		}

	//	Update the file descriptor

	DWORD dwCurrentVersion = dCurFileDesc.GetElement(FIELD_VERSION);

	CDatum dNewFileDesc(CDatum::typeStruct);
	dNewFileDesc.Append(dFileDesc);
	dNewFileDesc.SetElement(FIELD_FILE_PATH, dCurFileDesc.GetElement(FIELD_FILE_PATH));
	dNewFileDesc.SetElement(FIELD_MODIFIED_ON, CDateTime(CDateTime::Now));
	dNewFileDesc.SetElement(FIELD_SIZE, dCurFileDesc.GetElement(FIELD_SIZE));
	dNewFileDesc.SetElement(FIELD_STORAGE_PATH, dCurFileDesc.GetElement(FIELD_STORAGE_PATH));
	dNewFileDesc.SetElement(FIELD_VERSION, CDatum((int)(dwCurrentVersion + 1)));

	//	Write the data

	if (!Insert(Path, dNewFileDesc, &sError))
		{
		if (retdResult) *retdResult = strPattern(STR_ERROR_CANT_WRITE_FILE, sFilePath);
		return false;
		}

	//	Done. We return the new file desc

	if (retdResult)
		*retdResult = dNewFileDesc;

	return true;
	}

AEONERR CAeonTable::UploadFile (CMsgProcessCtx &Ctx, const CString &sSessionID, const CString &sFilePathArg, CDatum dUploadDesc, CDatum dData, CAeonUploadSessions::SReceipt *retReceipt, CString *retsError)

//	UploadFile
//
//	Uploads a file to the given table.
//
//	AEONERR_OK:
//		retiComplete is the % of the file that has been successfully uploaded.
//		When this returns 100, the file has been properly stored in the database.
//
//	AEONERR_FAIL:
//		Upload failed. retsError is the message and the upload is cancelled.
//
//	AEONERR_OUT_OF_DATE:
//		A different client uploaded a new version of the file. Upload is cancelled.

	{
	CSmartLock Lock(m_cs);

	//	Must be a file table

	if (m_iType != typeFile)
		{
		if (retsError)
			*retsError = STR_ERROR_FILE_TABLE_EXPECTED;
		return AEONERR_FAIL;
		}

	//	Make sure we have the primary volume

	if (m_bPrimaryLost)
		{
		*retsError = strPattern(ERR_PRIMARY_OFFLINE, m_sName);
		return AEONERR_FAIL;
		}

	//	Need the path and any existing file descriptor

	CString sFilePath;
	CRowKey Path;
	CDatum dCurrentFileDesc;

	//	If we don't need to generate a filePath, then we just take the filePath
	//	that's been given to us.

	CDatum dKeyGen = dUploadDesc.GetElement(FIELD_PRIMARY_KEY);
	if (dKeyGen.IsNil())
		{
		sFilePath = sFilePathArg;
		if (sFilePath.IsEmpty())
			{
			if (retsError)
				*retsError = strPattern(ERR_INVALID_FILE_PATH, sFilePath);
			return AEONERR_FAIL;
			}

		//	Generate a path

		CRowKey::CreateFromFilePath(sFilePath, &Path);

		//	Get the existing data for this file (if any)

		if (!GetData(DEFAULT_VIEW, Path, &dCurrentFileDesc, NULL, retsError))
			return AEONERR_FAIL;
		}

	//	Otherwise, we need to generate a unique filePath

	else if (strStartsWith(dKeyGen, MUTATE_CODE_8))
		{
		//	Make sure the key is unique

		do
			{
			sFilePath = strPattern("/%s", cryptoRandomCode(8));
			CRowKey::CreateFromFilePath(sFilePath, &Path);

			if (!GetData(DEFAULT_VIEW, Path, &dCurrentFileDesc, NULL, retsError))
				return AEONERR_FAIL;
			}
		while (!dCurrentFileDesc.IsNil());
		}

	//	Otherwise, invalid option

	else
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_FILE_PATH_CODE, (const CString &)dKeyGen);
		return AEONERR_FAIL;
		}

	CString sCurrentFilespec;
	if (!dCurrentFileDesc.IsNil())
		sCurrentFilespec = m_pStorage->CanonicalRelativeToMachine(m_sPrimaryVolume, dCurrentFileDesc.GetElement(FIELD_STORAGE_PATH));

	//	If dUploadDesc and dData are empty then we are deleting the file.

	if (dUploadDesc.GetCount() == 0 && dData.IsNil())
		{
		//	Write the data

		if (!Insert(Path, CDatum(), retsError))
			{
			*retsError = strPattern(STR_ERROR_CANT_WRITE_FILE, sFilePath);
			return AEONERR_FAIL;
			}

		//	Move the original file to scrap

		if (!sCurrentFilespec.IsEmpty())
			{
			if (!MoveToScrap(sCurrentFilespec))
				{
				//	This is not an error, since the file was uploaded properly.
				//	We can find this file later while checking for consistency.

				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_MOVE_TO_SCRAP, sCurrentFilespec));
				}
			}

		//	Done

		if (retReceipt)
			{
			*retReceipt = CAeonUploadSessions::SReceipt();
			retReceipt->iComplete = 100;
			retReceipt->iFileSize = 0;
			retReceipt->sFilePath = CAeonInterface::CreateTableFilePath(m_sName, sFilePath);
			}

		return AEONERR_OK;
		}

	//	Unlock while we process the upload

	Lock.Unlock();

	//	Process the upload until we get the entire thing

	CAeonUploadSessions::SReceipt Receipt;
	if (!m_UploadSessions.ProcessUpload(Ctx, sSessionID, sFilePath, dUploadDesc, dData, sCurrentFilespec, &Receipt))
		{
		if (retsError)
			*retsError = Receipt.sError;
		return AEONERR_FAIL;
		}

	//	If we're not yet complete, we're done; wait until more data comes in

	if (Receipt.iComplete != 100)
		{
		if (retReceipt)
			{
			*retReceipt = Receipt;
			retReceipt->sFilePath = CAeonInterface::CreateTableFilePath(m_sName, sFilePath);
			}

		return AEONERR_OK;
		}

	//	Lock while we update the table

	Lock.Lock();

	//	Look up the file in the database again because it might have
	//	changed while we were outside the lock.

	dCurrentFileDesc;
	if (!GetData(DEFAULT_VIEW, Path, &dCurrentFileDesc, NULL, retsError))
		{
		m_UploadSessions.AbortUpload(Receipt);
		return AEONERR_FAIL;
		}

	//	Get the current version. If this doesn't equal the version that we
	//	expect, then we return an error

	DWORD dwCurrentVersion = (dCurrentFileDesc.IsNil() ? 0 : (DWORD)(int)dCurrentFileDesc.GetElement(FIELD_VERSION));
	if (Receipt.dwVersion != 0xFFFFFFFF && dwCurrentVersion != Receipt.dwVersion)
		{
		m_UploadSessions.AbortUpload(Receipt);
		*retsError = strPattern(STR_ERROR_OUT_OF_DATE, sFilePath);
		return AEONERR_OUT_OF_DATE;
		}

	//	Update the file descriptor

	CDatum dNewFileDesc(CDatum::typeStruct);
	dNewFileDesc.Append(Receipt.dFileDesc);
	dNewFileDesc.SetElement(FIELD_VERSION, CDatum((int)(dwCurrentVersion + 1)));
	dNewFileDesc.SetElement(FIELD_FILE_PATH, sFilePath);
	dNewFileDesc.SetElement(FIELD_STORAGE_PATH, m_pStorage->MachineToCanonicalRelative(Receipt.sFilespec));
	dNewFileDesc.SetElement(FIELD_MODIFIED_ON, CDateTime(CDateTime::Now));
	dNewFileDesc.SetElement(FIELD_SIZE, Receipt.iFileSize);

	//	Write the data

	if (!Insert(Path, dNewFileDesc, retsError))
		{
		m_UploadSessions.AbortUpload(Receipt);
		*retsError = strPattern(STR_ERROR_CANT_WRITE_FILE, sFilePath);
		return AEONERR_FAIL;
		}

	//	Move the original file to scrap

	if (Receipt.bNewFile && !sCurrentFilespec.IsEmpty())
		{
		if (!MoveToScrap(sCurrentFilespec))
			{
			//	This is not an error, since the file was uploaded properly.
			//	We can find this file later while checking for consistency.

			m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_MOVE_TO_SCRAP, sCurrentFilespec));
			}

		//	Also move the backup file

		if (!m_bBackupLost)
			{
			CString sBackupFilespec = m_pStorage->CanonicalRelativeToMachine(m_sBackupVolume, m_pStorage->MachineToCanonicalRelative(sCurrentFilespec));
			if (!MoveToScrap(sBackupFilespec))
				{
				m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_MOVE_TO_SCRAP, sBackupFilespec));
				}
			}
		}

	//	Done

	if (retReceipt)
		{
		*retReceipt = Receipt;
		retReceipt->dFileDesc = PrepareFileDesc(m_sName, sFilePath, dNewFileDesc);
		retReceipt->sFilePath = CAeonInterface::CreateTableFilePath(m_sName, sFilePath);
		}

	return AEONERR_OK;
	}

bool CAeonTable::ValidateTableName (const CString &sName)

//	ValidateTableName
//
//	Returns TRUE if the given name is a valid table name.
//
//	Valid characters are:
//
//	numbers
//	letters
//	other unicode characters
//	- _ . ~
//
//	Invalid characters are:
//
//	control codes
//	space
//	! " # $ % & ' ( ) * + , / : ; < = > ? @ [ \ ] ^ ` { | }

	{
	//	Make sure table name is not empty

	if (sName.IsEmpty())
		return false;

	//	Make sure we don't have invalid characters

	char *pPos = sName.GetParsePointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case ' ':
			case '!':
			case '\"':
			case '#':
			case '$':
			case '%':
			case '&':
			case '\'':
			case '(':
			case ')':
			case '*':
			case '+':
			case ',':
			case '/':
			case ':':
			case ';':
			case '<':
			case '=':
			case '>':
			case '?':
			case '@':
			case '[':
			case '\\':
			case ']':
			case '^':
			case '`':
			case '{':
			case '|':
			case '}':
				return false;

			default:
				if (strIsASCIIControl(pPos))
					return false;
			}

		pPos++;
		}

	return true;
	}

bool CAeonTable::ValidateVolume (const CString &sVolume, TArray<CString> &retUnused, CString *retsError) const

//	ValidateVolume
//
//	Checks to make sure all the necessary files are on the given volume. Returns
//	FALSE if not.

	{
	CSmartLock Lock(m_cs);
	int i, j;

	//	Get the paths

	CString sVolPath = m_pStorage->GetPath(sVolume);
	if (sVolPath.IsEmpty())
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_VOLUME, sVolPath);
		return false;
		}

	CString sTablePath = fileAppend(sVolPath, m_sName);
	bool bOtherVol = !strEqualsNoCase(sVolume, m_sPrimaryVolume);

	//	Make a list of all segment files on the volume

	TArray<CString> SegmentFilespecs;
	GetSegmentFilespecs(sTablePath, &SegmentFilespecs);

	//	Create a map of existing segment files. We mark each one TRUE if it is
	//	being used.

	TSortMap<CString, bool> Existing;
	for (i = 0; i < SegmentFilespecs.GetCount(); i++)
		Existing.SetAt(SegmentFilespecs[i], false);

	//	Loop over all views checking each segment

	for (i = 0; i < m_Views.GetCount(); i++)
		{
		for (j = 0; j < m_Views[i].GetSegmentCount(); j++)
			{
			const CAeonSegment &Seg = m_Views[i].GetSegment(j);

			//	Get the segment file

			CString sSegFile = Seg.GetFilespec();

			//	If this is not the primary volume, then we need to convert the path

			if (bOtherVol)
				sSegFile = fileAppend(sVolPath, m_pStorage->MachineToCanonicalRelative(sSegFile));

			//	Check to see if the file exists. If not, then validation fails

			bool *pMarked = Existing.SetAt(sSegFile);
			if (pMarked == NULL)
				{
				if (retsError) *retsError = strPattern(ERR_MISSING_FILE, sSegFile);
				return false;
				}

			//	If the segment file does exist, mark it as in use

			*pMarked = true;
			}
		}

	//	Now loop over an segment files that are not used and return them

	retUnused.DeleteAll();
	for (i = 0; i < Existing.GetCount(); i++)
		if (!Existing[i])
			{
			retUnused.Insert(Existing.GetKey(i));
			}

	//	If we get this far, then the volume is valid.

	return true;
	}
