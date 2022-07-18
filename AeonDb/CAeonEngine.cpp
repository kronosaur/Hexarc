//	CAeonEngine.cpp
//
//	CAeonEngine class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(VIRTUAL_PORT_AEON_COMMAND,			"Aeon.command")
DECLARE_CONST_STRING(VIRTUAL_PORT_EXARCH_NOTIFY,		"Exarch.notify")
DECLARE_CONST_STRING(VIRTUAL_PORT_MNEMOSYNTH_NOTIFY,	"Mnemosynth.notify")

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command@~/~")
DECLARE_CONST_STRING(ADDRESS_AEON_NOTIFY,				"Aeon.notify")
DECLARE_CONST_STRING(ADDRESS_AEON_PRIVATE,				"Aeon.private@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_AEON,					"Aeon")

DECLARE_CONST_STRING(MNEMO_ARC_STORAGE,					"Arc.storage")

DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD_DESC,		"Aeon.fileDownloadDesc")
DECLARE_CONST_STRING(MSG_AEON_FILE_UPLOAD_PROGRESS,		"Aeon.fileUploadProgress")
DECLARE_CONST_STRING(MSG_AEON_ON_START,					"Aeon.onStart")
DECLARE_CONST_STRING(MSG_AEON_RESULT_KEYS,				"Aeon.resultKeys")
DECLARE_CONST_STRING(MSG_AEON_RESULT_TABLES,			"Aeon.resultTables")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_NOT_FOUND,				"Error.notFound")
DECLARE_CONST_STRING(MSG_ERROR_OUT_OF_DATE,				"Error.outOfDate")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ON_MODIFIED,		"Mnemosynth.onModified")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(OPTION_INCLUDE_KEY,				"includeKey")
DECLARE_CONST_STRING(OPTION_NO_KEY,						"noKey")

DECLARE_CONST_STRING(FILESPEC_TABLE_DIR_FILTER,			"*")

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address")
DECLARE_CONST_STRING(FIELD_BACKUP_VOLUMES,				"backupVolumes")
DECLARE_CONST_STRING(FIELD_COLLECTIONS,					"collections")
DECLARE_CONST_STRING(FIELD_DATA,						"data")
DECLARE_CONST_STRING(FIELD_ENTRIES,						"entries")
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc")
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath")
DECLARE_CONST_STRING(FIELD_IF_MODIFIED_AFTER,			"ifModifiedAfter")
DECLARE_CONST_STRING(FIELD_KEY_TYPE,					"keyType")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_PARTIAL_MAX_SIZE,			"partialMaxSize")
DECLARE_CONST_STRING(FIELD_PARTIAL_POS,					"partialPos")
DECLARE_CONST_STRING(FIELD_PRIMARY_KEY,					"primaryKey");
DECLARE_CONST_STRING(FIELD_PRIMARY_VOLUME,				"primaryVolume")
DECLARE_CONST_STRING(FIELD_STORAGE_PATH,				"storagePath")
DECLARE_CONST_STRING(FIELD_X,							"x")
DECLARE_CONST_STRING(FIELD_Y,							"y")
DECLARE_CONST_STRING(FIELD_Z,							"z")

DECLARE_CONST_STRING(STR_AEON_FOLDER,					"AeonDB")
DECLARE_CONST_STRING(STR_TABLE_DESC_FILENAME,			"Tables.ars")
DECLARE_CONST_STRING(STR_FILE_TABLE_DESC_PATTERN,		"{ name: \"%s\" type: file x: { keyType: utf8 } }")

DECLARE_CONST_STRING(STR_AEON_STARTING,					"Aeon database starting up.")
DECLARE_CONST_STRING(STR_DATABASE_OPEN,					"Aeon database opened.")
DECLARE_CONST_STRING(ERR_NOT_READY,						"Aeon is not yet online.")
DECLARE_CONST_STRING(ERR_ADMIN_REQUIRED,				"Arc.admin right required.")
DECLARE_CONST_STRING(ERR_INVALID_PARAMETERS,			"Invalid parameters.")
DECLARE_CONST_STRING(STR_ERROR_INVALID_TABLE_NAME,		"Invalid table name: %s.")
DECLARE_CONST_STRING(STR_ERROR_KEY_TYPE_REQUIRED,		"keyType parameter expected.")
DECLARE_CONST_STRING(STR_REPLY_ADDRESS_EXPECTED,		"Reply address expected: %s.")
DECLARE_CONST_STRING(STR_ERROR_INVALID_PATH,			"rowPath has incorrect number of dimensions.")
DECLARE_CONST_STRING(STR_ERROR_TABLE_ALREADY_EXISTS,	"Table already exists and cannot be created: %s.")
DECLARE_CONST_STRING(ERR_NOT_IN_SANDBOX,				"Table %s cannot be accessed by service: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_VIEW,					"Table %s does not have specified view: %s.")
DECLARE_CONST_STRING(STR_ERROR_NO_LOCAL_STORAGE,		"No local storage found.")
DECLARE_CONST_STRING(STR_ERROR_TABLE_DESC_REQUIRED,		"Unable to create table without dimension descriptor.")
DECLARE_CONST_STRING(STR_ERROR_PARSING_FILE_PATH,		"Unable to parse filePath: %s")
DECLARE_CONST_STRING(STR_ERROR_PARSING_TRANSPACE_ADDR,	"Unable to parse Transpace address: %s")
DECLARE_CONST_STRING(STR_UNABLE_TO_DELETE_TABLE_FILES,	"Unable to delete table files.")
DECLARE_CONST_STRING(STR_ERROR_UNKNOWN_KEY_TYPE,		"Unknown keyType: %s.")
DECLARE_CONST_STRING(STR_ERROR_UNKNOWN_TABLE,			"Unknown table: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_FLUSH,				"Unable to save all tables to disk.")
DECLARE_CONST_STRING(ERR_INVALID_GET_ROWS_OPTION,		"Invalid %s option: %s.")
DECLARE_CONST_STRING(ERR_INVALID_FILE_PATH,				"Invalid filePath: %s.")
DECLARE_CONST_STRING(ERR_FILE_NOT_FOUND,				"File not found: %s.")

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_AEON_DELETE,					"Aeon.delete")
DECLARE_CONST_STRING(MSG_AEON_DELETE_TABLE,				"Aeon.deleteTable")
DECLARE_CONST_STRING(MSG_AEON_DELETE_VIEW,				"Aeon.deleteView")
DECLARE_CONST_STRING(MSG_AEON_FILE_DIRECTORY,			"Aeon.fileDirectory")
DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD,			"Aeon.fileDownload")
DECLARE_CONST_STRING(MSG_AEON_FILE_GET_DESC,			"Aeon.fileGetDesc")
DECLARE_CONST_STRING(MSG_AEON_FILE_UPDATE_DESC,			"Aeon.fileUpdateDesc")
DECLARE_CONST_STRING(MSG_AEON_FILE_UPLOAD,				"Aeon.fileUpload")
DECLARE_CONST_STRING(MSG_AEON_FLUSH_DB,					"Aeon.flushDb")
DECLARE_CONST_STRING(MSG_AEON_GET_KEY_RANGE,			"Aeon.getKeyRange")
DECLARE_CONST_STRING(MSG_AEON_GET_MORE_ROWS,			"Aeon.getMoreRows")
DECLARE_CONST_STRING(MSG_AEON_GET_ROWS,					"Aeon.getRows")
DECLARE_CONST_STRING(MSG_AEON_GET_TABLES,				"Aeon.getTables")
DECLARE_CONST_STRING(MSG_AEON_GET_DATA,					"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_GET_VIEW_INFO,			"Aeon.getViewInfo")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_AEON_INSERT_NEW,				"Aeon.insertNew")
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate")
DECLARE_CONST_STRING(MSG_AEON_RECOVER_TABLE_TEST,		"Aeon.recoverTableTest")
DECLARE_CONST_STRING(MSG_AEON_WAIT_FOR_VIEW,			"Aeon.waitForView")
DECLARE_CONST_STRING(MSG_AEON_WAIT_FOR_VOLUME,			"Aeon.waitForVolume")
DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")
DECLARE_CONST_STRING(MSG_EXARCH_ON_MACHINE_START,		"Exarch.onMachineStart")
DECLARE_CONST_STRING(MSG_TRANSPACE_DOWNLOAD,			"Transpace.download")

CAeonEngine::SMessageHandler CAeonEngine::m_MsgHandlerList[] =
	{
		//	Aeon.createTable {tableDesc}
		//
		//	{tableDesc} = { name: "MyTable1" x: { keyType: "utf8"} y: { keyType: "int32" } z: { keyType: "dateTime" }}
		{	MSG_AEON_CREATE_TABLE,				&CAeonEngine::MsgCreateTable },

		//	Aeon.deleteTable {tableName}
		{	MSG_AEON_DELETE_TABLE,				&CAeonEngine::MsgDeleteTable },

		//	Aeon.deleteView {tableAndView}
		{	MSG_AEON_DELETE_VIEW,				&CAeonEngine::MsgDeleteView },

		//	Aeon.fileDirectory {filePath}
		{	MSG_AEON_FILE_DIRECTORY,			&CAeonEngine::MsgFileDirectory },

		//	Aeon.fileDownload {filePath} [{fileDownloadDesc}]
		{	MSG_AEON_FILE_DOWNLOAD,				&CAeonEngine::MsgFileDownload },

		//	Aeon.fileGetDesc {filePath}
		{	MSG_AEON_FILE_GET_DESC,				&CAeonEngine::MsgFileGetDesc },

		//	Aeon.fileUpdateDesc {filePath} {fileDesc}
		{	MSG_AEON_FILE_UPDATE_DESC,			&CAeonEngine::MsgFileUpdateDesc },

		//	Aeon.fileUpload {filePath} {fileUploadDesc} {data}
		{	MSG_AEON_FILE_UPLOAD,				&CAeonEngine::MsgFileUpload },

		//	Aeon.flushDb
		{	MSG_AEON_FLUSH_DB,					&CAeonEngine::MsgFlushDb },

		//	Aeon.getValue {tableName} {rowPath}
		{	MSG_AEON_GET_DATA,					&CAeonEngine::MsgGetData },

		//	Aeon.getViewInfo {tableAndView}
		{	MSG_AEON_GET_VIEW_INFO,				&CAeonEngine::MsgGetViewInfo },

		//	Aeon.getKeyRange {tableName} {count} [{startingKey}]
		{	MSG_AEON_GET_KEY_RANGE,				&CAeonEngine::MsgGetKeyRange },

		//	Aeon.getMoreRows {tableAndView} {lastKey} {count}
		{	MSG_AEON_GET_MORE_ROWS,				&CAeonEngine::MsgGetRows },

		//	Aeon.getRows {tableAndView} {Key} {count}
		{	MSG_AEON_GET_ROWS,					&CAeonEngine::MsgGetRows },

		//	Aeon.getTables
		{	MSG_AEON_GET_TABLES,				&CAeonEngine::MsgGetTables },

		//	Aeon.insert {tableName} {rowPath} {data}
		//
		//	{rowPath} is an array with as many elements as the table dimensions
		{	MSG_AEON_INSERT,					&CAeonEngine::MsgInsert },

		//	Aeon.insertNew {tableName} {rowPath} {data}
		{	MSG_AEON_INSERT_NEW,				&CAeonEngine::MsgInsertNew },

		//	Aeon.mutate {tableName} {rowPath} {struct} {mutationDesc}
		{	MSG_AEON_MUTATE,					&CAeonEngine::MsgMutate },

		//	Aeon.recoverTableTest {tableName}
		{	MSG_AEON_RECOVER_TABLE_TEST,		&CAeonEngine::MsgRecoverTableTest },

		//	Aeon.waitForView {tableAndView}
		{	MSG_AEON_WAIT_FOR_VIEW,				&CAeonEngine::MsgWaitForView },

		//	Aeon.waitForVolume {volume}
		{	MSG_AEON_WAIT_FOR_VOLUME,			&CAeonEngine::MsgWaitForVolume },

		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,				&CAeonEngine::MsgHousekeeping },

		//	Exarch.onMachineStart
		{	MSG_EXARCH_ON_MACHINE_START,		&CAeonEngine::MsgOnMachineStart },

		//	Mnemosynth.onModified
		{	MSG_MNEMOSYNTH_ON_MODIFIED,			&CAeonEngine::MsgOnMnemosynthModified },

		//	Transpace.download {address} {originalAddress} [{fileDownload}]
		{	MSG_TRANSPACE_DOWNLOAD,				&CAeonEngine::MsgTranspaceDownload },
	};

int CAeonEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CAeonEngine::m_MsgHandlerList);

CAeonEngine::CAeonEngine (void) : TSimpleEngine(ENGINE_NAME_AEON, 3)

//	CAeonEngine constructor

	{
	}

CAeonEngine::~CAeonEngine (void)

//	CAeonEngine destructor

	{
	int i;

	for (i = 0; i < m_Tables.GetCount(); i++)
		delete m_Tables[i];
	}

bool CAeonEngine::CreateTable (CDatum dDesc, CAeonTable **retpTable, bool *retbExists, CString *retsError)

//	CreateTable
//
//	Creates a new table.

	{
	//	Prefill

	if (retbExists)
		*retbExists = false;

	//	No storage

	if (m_LocalVolumes.GetCount() == 0)
		{
		*retsError = STR_ERROR_NO_LOCAL_STORAGE;
		return false;
		}

	//	Check to see if this is a valid name

	const CString &sName = dDesc.GetElement(FIELD_NAME);
	if (!CAeonTable::ValidateTableName(sName))
		{
		if (retsError)
			*retsError = strPattern(STR_ERROR_INVALID_TABLE_NAME, sName);
		return false;
		}

	//	Add the table (if it doesn't already exist)

	CSmartLock Lock(m_cs);

	CAeonTable *pTable;
	if (m_Tables.Find(sName, &pTable))
		{
		//	If the table already exists see if we need to re-initialized it
		//	based on a new descriptor

		bool bUpdated;
		if (!pTable->Recreate(GetProcessCtx(), dDesc, &bUpdated, retsError))
			return false;

		//	If we updated the table then we succeeded

		if (bUpdated)
			{
			if (retpTable)
				*retpTable = pTable;
			return true;
			}

		//	Otherwise we reply that the table already exists.

		if (retsError)
			*retsError = strPattern(STR_ERROR_TABLE_ALREADY_EXISTS, sName);

		if (retbExists)
			*retbExists = true;

		return false;
		}

	//	Create a new table.
	//	NOTE: We rely on the fact that we've locked the engine to prevent
	//	local storage from changing while the table is created.

	pTable = new CAeonTable;
	if (!pTable->Create(GetProcessCtx(), &m_LocalVolumes, dDesc, retsError))
		{
		delete pTable;
		return false;
		}

	int iNewTableIndex = m_Tables.GetCount();
	m_Tables.Insert(sName, pTable);

	//	Done

	if (retpTable)
		*retpTable = pTable;

	return true;
	}

bool CAeonEngine::FindTable (const CString &sName, CAeonTable **retpTable)

//	FindTable
//
//	Returns the table by name (or FALSE if no table found).
//	This function locks the structure to find the table name. We guarantee
//	that a table pointer will never be freed (except at garbage collection time,
//	when we stop the world).

	{
	CSmartLock Lock(m_cs);
	return m_Tables.Find(sName, retpTable);
	}

bool CAeonEngine::FlushTableRows (void)

//	FlushTableRows
//
//	Save all in-memory rows to disk

	{
	int i;

	//	Get a list of all tables

	TArray<CAeonTable *> AllTables;
	GetTables(&AllTables);

	//	Loop over each table

	bool bAllSucceeded = true;
	for (i = 0; i < AllTables.GetCount(); i++)
		{
		CString sError;
		if (!AllTables[i]->Save(&sError))
			{
			Log(MSG_LOG_ERROR, strPattern("Unable to save table %s: %s", AllTables[i]->GetName(), sError));
			bAllSucceeded = false;
			//	Continue saving other tables
			}
		}
	
	//	Done

	return bAllSucceeded;
	}

bool CAeonEngine::GetKeyRange (const SArchonMessage &Msg, const CString &sTable, int iCount)

//	GetKeyRange
//
//	Processes a getKeyRange message

	{
	//	If the table doesn't exist, then we can't continue

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return false;
		}

	//	Create an iterator

	CDatum dResult;
	CString sError;
	if (!pTable->GetKeyRange(iCount, &dResult, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return false;
		}

	//	Reply

	SendMessageReply(MSG_AEON_RESULT_KEYS, dResult, Msg);
	return true;
	}

void CAeonEngine::GetTables (TArray<CAeonTable *> *retTables)

//	GetTables
//
//	Returns pointers to all the open tables in the system.

	{
	CSmartLock Lock(m_cs);
	int i;

	retTables->DeleteAll();
	retTables->InsertEmpty(m_Tables.GetCount());
	for (i = 0; i < m_Tables.GetCount(); i++)
		retTables->GetAt(i) = m_Tables[i];
	}

bool CAeonEngine::GetViewStatus (const CString &sTable, DWORD dwViewID, bool *retbUpToDate, CString *retsError)

//	GetViewStatus
//
//	Returns the status of the given view

	{
	CSmartLock Lock(m_cs);

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		*retsError = strPattern(STR_ERROR_UNKNOWN_TABLE, sTable);
		return false;
		}

	return pTable->GetViewStatus(dwViewID, retbUpToDate, retsError);
	}

void CAeonEngine::MsgCreateTable (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateTable
//
//	Aeon.createTable {tableDesc}
//
//	{tableDesc} = { name:MyTable1 type:standard x:{keyType:utf8} y:{keyType:int32} z:{keyType:dateTime} }

	{
	CString sError;

	//	Get the table desc and table name

	CDatum dTableDesc = Msg.dPayload.GetElement(0);
	const CString &sTable = dTableDesc.GetElement(FIELD_NAME);

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	See if the table descriptor specifies storage volumes; if so, then we
	//	make sure that the calling service has admin rights.

	if (!dTableDesc.GetElement(FIELD_PRIMARY_VOLUME).IsNil()
			|| !dTableDesc.GetElement(FIELD_BACKUP_VOLUMES).IsNil())
		{
		if (!ValidateAdminAccess(Msg, pSecurityCtx))
			return;
		}

	//	Create

	bool bExists;
	if (!CreateTable(dTableDesc, NULL, &bExists, &sError))
		{
		if (bExists)
			SendMessageReplyError(MSG_ERROR_ALREADY_EXISTS, sError, Msg);
		else
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CAeonEngine::MsgDeleteView (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgDeleteView
//
//	Aeon.deleteView {tableAndView}

	{
	CAeonTable *pTable;
	DWORD dwViewID;
	if (!ParseTableAndView(Msg, pSecurityCtx, Msg.dPayload.GetElement(0), &pTable, &dwViewID))
		return;

	//	Delete

	CString sError;
	if (!pTable->DeleteView(dwViewID, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CAeonEngine::MsgDeleteTable (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgDeleteTable
//
//	Aeon.deleteTable {tableName}

	{
	//	Parameters

	const CString &sTable = Msg.dPayload.GetElement(0);

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	Lock while we remove the table

	CSmartLock Lock(m_cs);

	//	If the table doesn't exist, then we can't continue

	int iTablePos;
	if (!m_Tables.FindPos(sTable, &iTablePos))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Remove the table from the array

	CAeonTable *pTable = m_Tables.GetValue(iTablePos);
	m_Tables.Delete(iTablePos);

	//	Clean up the table

	if (!pTable->Delete())
		Log(MSG_LOG_ERROR, STR_UNABLE_TO_DELETE_TABLE_FILES);

	delete pTable;

	//	We can unlock now

	Lock.Unlock();

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CAeonEngine::MsgFileDirectory (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgFileDirectory
//
//	Aeon.fileDirectory {filePath} {requestedFields} {options}

	{
	CString sError;

	//	Get the filePath

	CString sTable;
	CString sFilePath;
	if (!CAeonTable::ParseFilePath(Msg.dPayload.GetElement(0), &sTable, &sFilePath, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_FILE_PATH, sError), Msg);
		return;
		}

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	Convert the filepath to a key

	CString sDirKey = CRowKey::FilePathToKey(sFilePath);

	//	Get the table

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Get the set of requested fields and options

	CDatum dRequestedFields = Msg.dPayload.GetElement(1);
	CDatum dOptions = Msg.dPayload.GetElement(2);

	//	Do it

	CDatum dResult;
	if (!pTable->FileDirectory(sDirKey, dRequestedFields, dOptions, &dResult, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Reply

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CAeonEngine::MsgFileDownload (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgFileDownload
//
//	Aeon.fileDownload {filePath} [{fileDownloadDesc}]

	{
	CString sError;

	//	Get the filePath

	CString sTable;
	CString sFilePath;
	if (!CAeonTable::ParseFilePath(Msg.dPayload.GetElement(0), &sTable, &sFilePath, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_FILE_PATH, sError), Msg);
		return;
		}

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	Initialize options from a fileDownloadDesc

	CAeonTable::SFileDataOptions Options;
	if (!CAeonTable::InitFromFileDownloadDesc(Msg.dPayload.GetElement(1), Options))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMETERS, Msg);
		return;
		}

	//	Get the table

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

#ifdef DEBUG_BLOB_PERF
	DWORD dwStart = ::sysGetTickCount();
#endif

	//	Get the data

	CDatum dFileDownloadDesc;
	if (!pTable->GetFileData(sFilePath, Options, &dFileDownloadDesc, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	If file not found, return a special error.

	if (dFileDownloadDesc.IsNil())
		{
		SendMessageReplyError(MSG_ERROR_NOT_FOUND, strPattern(ERR_FILE_NOT_FOUND, sFilePath), Msg);
		return;
		}

#ifdef DEBUG_BLOB_PERF
	DWORD dwTime = ::sysGetTicksElapsed(dwStart);
	if (dwTime > 100)
		Log(MSG_LOG_INFO, strPattern("GetFileData %s %D bytes took %D ms.", sFilePath, dFileDownloadDesc.GetElement(FIELD_DATA).GetBinarySize(), dwTime));
#endif

	//	Done

	SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, dFileDownloadDesc, Msg);
	}

void CAeonEngine::MsgFileGetDesc (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgFileGetDesc
//
//	Aeon.fileGetDesc {filePath} [{options}]

	{
	CString sError;

	//	Options

	CDatum dOptions = Msg.dPayload.GetElement(1);
	bool bIncludeStoragePath = !dOptions.GetElement(FIELD_STORAGE_PATH).IsNil();

	//	Get the filePath

	CString sTable;
	CString sFilePath;
	if (!CAeonTable::ParseFilePath(Msg.dPayload.GetElement(0), &sTable, &sFilePath, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_FILE_PATH, sError), Msg);
		return;
		}

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	Get the table

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Get the descriptor

	CDatum dFileDesc;
	if (!pTable->GetFileDesc(sFilePath, &dFileDesc, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Prepare the descriptor for return

	CDatum dNewFileDesc = CAeonTable::PrepareFileDesc(sTable, sFilePath, dFileDesc, (bIncludeStoragePath ? CAeonTable::FLAG_STORAGE_PATH : 0));

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dNewFileDesc, Msg);
	}

void CAeonEngine::MsgFileUpdateDesc (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgFileUpdateDesc
//
//	Aeon.fileUpdateDesc {filePath} {fileDesc} [{options}]

	{
	CString sError;

	CDatum dFileDesc = Msg.dPayload.GetElement(1);

	//	Options

	CDatum dOptions = Msg.dPayload.GetElement(2);
	bool bIncludeStoragePath = !dOptions.GetElement(FIELD_STORAGE_PATH).IsNil();

	//	Get the filePath

	CString sTable;
	CString sFilePath;
	if (!CAeonTable::ParseFilePath(Msg.dPayload.GetElement(0), &sTable, &sFilePath, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_FILE_PATH, sError), Msg);
		return;
		}

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	Get the table

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Set the descriptor and return the new verson.

	CDatum dNewFileDesc;
	if (!pTable->UpdateFileDesc(sFilePath, dFileDesc, &dNewFileDesc))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, dNewFileDesc, Msg);
		return;
		}

	//	Return the new file desc

	dNewFileDesc = CAeonTable::PrepareFileDesc(sTable, sFilePath, dNewFileDesc, (bIncludeStoragePath ? CAeonTable::FLAG_STORAGE_PATH : 0));
	SendMessageReply(MSG_REPLY_DATA, dNewFileDesc, Msg);
	}

void CAeonEngine::MsgFileUpload (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgFileUpload
//
//	Aeon.fileUpload {filePath} {fileUploadDesc} {data}

	{
	AEONERR error;
	CMsgProcessCtx Ctx(*GetProcessCtx(), Msg, pSecurityCtx);

	//	Get parameters

	CDatum dFilePath = Msg.dPayload.GetElement(0);
	CDatum dUploadDesc = Msg.dPayload.GetElement(1);
	CDatum dData = Msg.dPayload.GetElement(2);

	CDatum dKeyGen = dUploadDesc.GetElement(FIELD_PRIMARY_KEY);

	//	Get the filePath

	CString sError;
	CString sTable;
	CString sFilePath;

	//	If we need to generate a unique filename, then we only parse the table.

	if (!dKeyGen.IsNil())
		{
		if (!CAeonTable::ParseFilePath(dFilePath, &sTable, &sFilePath, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_FILE_PATH, sError), Msg);
			return;
			}
		}
	else
		{
		if (!CAeonTable::ParseFilePathForCreate(dFilePath, &sTable, &sFilePath, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_FILE_PATH, sError), Msg);
			return;
			}
		}

	//	NOTE: It is OK if sFilePath is empty because we might need to generate
	//	a unique file path.

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	Lock while we find or create a table

	CSmartLock Lock(m_cs);

	//	If the table doesn't exist, then we create a new one inside the lock

	CAeonTable *pTable;
	if (!m_Tables.Find(sTable, &pTable))
		{
		//	Compose a table descriptor

		CDatum dTableDesc;
		if (!CDatum::Deserialize(CDatum::EFormat::AEONScript, CBuffer(strPattern(STR_FILE_TABLE_DESC_PATTERN, sTable)), &dTableDesc))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_INVALID_TABLE_NAME, sTable), Msg);
			return;
			}

		//	Create the table

		if (!CreateTable(dTableDesc, &pTable, NULL, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			return;
			}
		}

	//	Unlock

	Lock.Unlock();

	//	Generate a unique session ID from the message

	CString sSessionID = strPattern("%s/%x%s", Msg.sReplyAddr, Msg.dwTicket, sFilePath);

#ifdef DEBUG_FILE_UPLOAD
	DWORD dwStart = ::sysGetTickCount();
	Log(MSG_LOG_INFO, strPattern("Aeon.fileUpload %s [%d bytes]", sFilePath, dData.GetBinarySize()));
#endif

	//	Let the table handle the rest

	CAeonUploadSessions::SReceipt Receipt;
	if (error = pTable->UploadFile(Ctx, sSessionID, sFilePath, dUploadDesc, dData, &Receipt, &sError))
		{
		if (error == AEONERR_OUT_OF_DATE)
			SendMessageReplyError(MSG_ERROR_OUT_OF_DATE, sError, Msg);
		else
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

#ifdef DEBUG_FILE_UPLOAD
	Log(MSG_LOG_INFO, strPattern("Aeon.fileUpload complete: %d seconds.", ::sysGetTicksElapsed(dwStart) / 1000));
#endif

	//	Reply

	if (Receipt.iComplete == 100)
		SendMessageReply(MSG_REPLY_DATA, Receipt.dFileDesc, Msg);
	else
		{
		CDatum dReceipt(CDatum::typeStruct);
		dReceipt.SetElement(FIELD_FILE_PATH, Receipt.sFilePath);

		//	LATER: Fill in receipt

		SendMessageReply(MSG_AEON_FILE_UPLOAD_PROGRESS, dReceipt, Msg);
		}
	}

void CAeonEngine::MsgFlushDb (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgFlushDb
//
//	Aeon.flushDb

	{
	if (!ValidateAdminAccess(Msg, pSecurityCtx))
		return;

	if (!FlushTableRows())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_UNABLE_TO_FLUSH, Msg);
		return;
		}

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CAeonEngine::MsgGetData (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetData
//
//	Aeon.getData {tableName} {rowPath}

	{
	CString sError;

	CAeonTable *pTable;
	DWORD dwViewID;
	CRowKey Path;

	if (!ParseTableAndView(Msg, 
			pSecurityCtx, 
			Msg.dPayload.GetElement(0), 
			&pTable, 
			&dwViewID,
			Msg.dPayload.GetElement(1),
			&Path))
		return;

	//	Do a lookup

	CDatum dData;
	if (!pTable->GetData(dwViewID, Path, &dData, NULL, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dData, Msg);
	}

void CAeonEngine::MsgGetKeyRange (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetKeyRanage
//
//	Aeon.getKeyRange {tableName} {count} [{startingKey}]

	{
	CString sTable = Msg.dPayload.GetElement(0);

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	GetKeyRange(Msg, sTable, Msg.dPayload.GetElement(1));
	}

void CAeonEngine::MsgGetRows (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetRows
//
//	Aeon.getRows {tableAndView} {key} {count}
//	Aeon.getMoreRows {tableAndView} {lastKey} {count}

	{
	int i;

	CAeonTable *pTable;
	DWORD dwViewID;
	if (!ParseTableAndView(Msg, pSecurityCtx, Msg.dPayload.GetElement(0), &pTable, &dwViewID))
		return;

	//	Get the row limits

	int iRowCount;
	TArray<int> Limits;
	CDatum dLimits = Msg.dPayload.GetElement(2);
	if (dLimits.IsNil())
		iRowCount = -1;
	else if (dLimits.GetCount() <= 1)
		{
		iRowCount = (int)dLimits.GetElement(0);
		if (iRowCount <= 0)
			iRowCount = -1;
		}
	else
		{
		iRowCount = (int)dLimits.GetElement(0);
		if (iRowCount <= 0)
			iRowCount = -1;

		Limits.InsertEmpty(dLimits.GetCount() - 1);
		for (i = 1; i < dLimits.GetCount(); i++)
			Limits[i - 1] = (int)dLimits.GetElement(i);
		}

	//	Set up flags and options

	DWORD dwFlags = 0;
	dwFlags |= (strEquals(Msg.sMsg, MSG_AEON_GET_ROWS) ? 0 : CAeonTable::FLAG_MORE_ROWS);

	CDatum dOptions = Msg.dPayload.GetElement(3);
	for (i = 0; i < dOptions.GetCount(); i++)
		{
		if (strEquals(dOptions.GetElement(i), OPTION_INCLUDE_KEY))
			dwFlags |= CAeonTable::FLAG_INCLUDE_KEY;
		else if (strEquals(dOptions.GetElement(i), OPTION_NO_KEY))
			dwFlags |= CAeonTable::FLAG_NO_KEY;
		else
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_GET_ROWS_OPTION, Msg.sMsg, dOptions.GetElement(i).AsString()), Msg);
			return;
			}
		}

	//	Ask the table

	CDatum dResult;
	CString sError;
	if (!pTable->GetRows(dwViewID, Msg.dPayload.GetElement(1), iRowCount, Limits, dwFlags, &dResult, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CAeonEngine::MsgGetTables (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetTables
//
//	Aeon.getTables

	{
	int i;

	if (!m_bReady)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NOT_READY, Msg);
		return;
		}

	//	Get a list of all tables

	TArray<CAeonTable *> AllTables;
	GetTables(&AllTables);

	//	Return an array of table descriptors

	CComplexArray *pArray = new CComplexArray;
	for (i = 0; i < AllTables.GetCount(); i++)
		{
		if (pSecurityCtx && !pSecurityCtx->IsNamespaceAccessible(AllTables[i]->GetName()))
			continue;

		pArray->Insert(AllTables[i]->GetDesc());
		}

	//	Reply

	SendMessageReply(MSG_AEON_RESULT_TABLES, CDatum(pArray), Msg);
	}

void CAeonEngine::MsgGetViewInfo (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetViewInfo
//
//	Aeon.getViewInfo {tableAndView}

	{
	if (!ValidateAdminAccess(Msg, pSecurityCtx))
		return;

	CAeonTable *pTable;
	DWORD dwViewID;
	if (!ParseTableAndView(Msg, pSecurityCtx, Msg.dPayload.GetElement(0), &pTable, &dwViewID))
		return;

	//	Delete

	CDatum dResult;
	if (!pTable->DebugDumpView(dwViewID, &dResult))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, dResult, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CAeonEngine::MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgHousekeeping
//
//	Arc.housekeeping

	{
	int i;

	ASSERT(pSecurityCtx == NULL);

	//	Get the list of all tables (in a semaphore)

	TArray<CAeonTable *> AllTables;
	GetTables(&AllTables);
	if (AllTables.GetCount() == 0)
		return;

	//	Compute how much memory data we want each table to keep in memory
	//	before it flushes to disk. (x2 because most tables won't use the
	//	maximum.

	DWORD dwMemoryPerTable = 2 * m_dwMaxMemoryUse / AllTables.GetCount();

	//	Loop over all tables and let them do some housekeeping tasks (such as
	//	saving updated rows and compacting segments).

	for (i = 0; i < AllTables.GetCount(); i++)
		AllTables[i]->Housekeeping(dwMemoryPerTable);
	}

void CAeonEngine::MsgInsert (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgInsert
//
//	Aeon.insert {tableName} {rowPath} {data}
//
//	{rowPath} is an array with as many elements as the table dimensions

	{
	const CString &sTable = Msg.dPayload.GetElement(0);
	CDatum dRowPath = Msg.dPayload.GetElement(1);
	CDatum dData = Msg.dPayload.GetElement(2);

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	If the table doesn't exist, then we can't continue

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Parse the path

	CString sError;
	CRowKey Path;
	if (!pTable->ParseDimensionPathForCreate(dRowPath, &Path, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Insert

	if (!pTable->Insert(Path, dData, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CAeonEngine::MsgInsertNew (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgInsertNew
//
//	Aeon.insertNew {tableName} {rowPath} {data}
//
//	{rowPath} is an array with as many elements as the table dimensions

	{
	AEONERR error;

	const CString &sTable = Msg.dPayload.GetElement(0);
	CDatum dRowPath = Msg.dPayload.GetElement(1);
	CDatum dData = Msg.dPayload.GetElement(2);

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	If the table doesn't exist, then we can't continue

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Parse the path

	CString sError;
	CRowKey Path;
	if (!pTable->ParseDimensionPathForCreate(dRowPath, &Path, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Insert

	if (error = pTable->Insert(Path, dData, true, &sError))
		{
		if (error == AEONERR_ALREADY_EXISTS)
			SendMessageReplyError(MSG_ERROR_ALREADY_EXISTS, sError, Msg);
		else
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CAeonEngine::MsgMutate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgMutate
//
//	Aeon.mutate {tableName} {rowPath} {data} {mutationDesc}

	{
	AEONERR error;

	const CString &sTable = Msg.dPayload.GetElement(0);
	CDatum dRowPath = Msg.dPayload.GetElement(1);
	CDatum dData = Msg.dPayload.GetElement(2);
	CDatum dMutateDesc = Msg.dPayload.GetElement(3);

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	If the table doesn't exist, then we can't continue

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Parse the path
	//
	//	NOTE: We don't need to validate that this is a valid path for create
	//	because we do that later inside Mutate. [Because Mutate accepts a nil
	//	path when generating a unique key.]

	CString sError;
	CRowKey Path;
	if (!pTable->ParseDimensionPath(NULL_STR, dRowPath, &Path, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Mutate

	CDatum dResult;
	if (error = pTable->Mutate(Path, dData, dMutateDesc, &dResult, &sError))
		{
		if (error == AEONERR_OUT_OF_DATE)
			SendMessageReplyError(MSG_ERROR_OUT_OF_DATE, sError, Msg);
		else
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CAeonEngine::MsgRecoverTableTest (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRecoverTableTest
//
//	Aeon.recoverTableTest {tableName}

	{
	//	This is an admin operation

	if (!ValidateAdminAccess(Msg, pSecurityCtx))
		return;

	//	Get the table

	const CString &sTable = Msg.dPayload.GetElement(0);

	//	If the table doesn't exist, then we can't continue

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Recover

	CString sError;
	if (!pTable->RecoverTableRows(&sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CAeonEngine::MsgOnMachineStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgOnMachineStart
//
//	Exarch.onMachineStart

	{
	CSmartLock Lock(m_cs);

	//	If we've already started, then nothing to do.

	if (m_bMachineStarted)
		return;

	//	Check to see if we're listed in the module list. If we're not, then we 
	//	continue waiting.

	if (!Msg.dPayload.IsNil() && !Msg.dPayload.Find(GetProcessCtx()->GetModuleName()))
		return;

	//	Aeon starting

	Log(MSG_LOG_INFO, STR_AEON_STARTING);

	//	Make a list of local volumes

	if (!OpenLocalVolumes())
		return;

	//	We've at least gotten to onMachineStart. From now on if we get any 
	//	storage added, we have to deal with it differently.

	m_bMachineStarted = true;

	//	If we have no local volumes then we cannot proceed. We wait until we
	//	have storage to send Aeon.onStart.

	if (m_LocalVolumes.GetCount() == 0)
		{
		Log(MSG_LOG_INFO, STR_ERROR_NO_LOCAL_STORAGE);
		return;
		}

	//	Make sure the database is open

	if (!Open())
		return;

	//	Tell any listeners that the database is ready

	SendMessageNotify(ADDRESS_AEON_NOTIFY, MSG_AEON_ON_START, CDatum());

	//	Done

	Log(MSG_LOG_INFO, STR_DATABASE_OPEN);
	}

void CAeonEngine::MsgOnMnemosynthModified (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgOnMnemosynthModified
//
//	Mnemosynth.onModified {updates}

	{
	int i;

	//	If we haven't yet started we don't care about this message.

	if (!m_bMachineStarted)
		return;

	//	Get the updates

	CDatum dCollections = Msg.dPayload.GetElement(FIELD_COLLECTIONS);
	CDatum dEntries = Msg.dPayload.GetElement(FIELD_ENTRIES);

	//	See what changed

	bool bStorageChanged = false;
	for (i = 0; i < dEntries.GetCount(); i++)
		{
		CDatum dEntry = dEntries.GetElement(i);

		const CString &sCollection = dCollections.GetElement((int)dEntry.GetElement(0));
		if (strEquals(sCollection, MNEMO_ARC_STORAGE))
			bStorageChanged = true;
		}

	//	If storage changed then update local storage (which will tell us what
	//	storage was added or deleted).

	if (bStorageChanged)
		{
		CSmartLock Lock(m_cs);

		CString sError;
		TArray<CString> VolumesAdded;
		TArray<CString> VolumesDeleted;
		if (!m_LocalVolumes.Reinit(GetProcessCtx(), &VolumesAdded, &VolumesDeleted, &sError))
			{
			//	LATER: We don't know how to recover from this, so we should go into
			//	safe mode.
			Log(MSG_LOG_ERROR, sError);
			return;
			}

		//	Tell every table that volumes were added and deleted so that they can
		//	change their primary and backup volumes.

		if (VolumesAdded.GetCount() > 0 || VolumesDeleted.GetCount() > 0)
			{
			for (i = 0; i < m_Tables.GetCount(); i++)
				m_Tables[i]->OnVolumesChanged(VolumesDeleted);

			//	LATER: See if there are any existing tables in the new volume.
			}
		}
	}

void CAeonEngine::MsgTranspaceDownload (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgTranspaceDownload
//
//	Transpace.download {address} {originalAddress} [{fileDownloadDesc}]

	{
	CString sError;
	CString sAddress = Msg.dPayload.GetElement(0);
	CString sOriginalAddress = Msg.dPayload.GetElement(1);
	CDatum dFileDownloadDesc = Msg.dPayload.GetElement(2);

	//	Parse the address to get the path

	CString sFullPath;
	if (!CTranspaceInterface::ParseAddress(sAddress, NULL, &sFullPath))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_TRANSPACE_ADDR, sAddress), Msg);
		return;
		}

	//	Get the filePath

	CString sTable;
	CString sFilePath;
	if (!CAeonTable::ParseFilePath(sFullPath, &sTable, &sFilePath, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_PARSING_FILE_PATH, sError), Msg);
		return;
		}

	//	Make sure we are allowed access to this table

	if (!ValidateTableAccess(Msg, pSecurityCtx, sTable))
		return;

	//	See if we have a fileDownloadDesc

	CAeonTable::SFileDataOptions Options;
	if (!CAeonTable::InitFromFileDownloadDesc(dFileDownloadDesc, Options))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMETERS, Msg);
		return;
		}

	Options.bTranspace = true;

	//	Get the table

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return;
		}

	//	Get the data

	if (!pTable->GetFileData(sFilePath, Options, &dFileDownloadDesc, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	If file not found, return a special error.

	if (dFileDownloadDesc.IsNil())
		{
		SendMessageReplyError(MSG_ERROR_NOT_FOUND, strPattern(ERR_FILE_NOT_FOUND, sFilePath), Msg);
		return;
		}

	//	Add the original address to the download descriptor
	//
	//	NOTE: It is OK to modify this field because we know that GetFileData 
	//	created a new structure.

	dFileDownloadDesc.GetElement(FIELD_FILE_DESC).SetElement(FIELD_ADDRESS, sOriginalAddress);

#ifdef DEBUG_FILES
	Log(MSG_LOG_DEBUG, strPattern("[%x]: Transpace.download address=%s original=%s filePath=%s", Msg.dwTicket, sAddress, sOriginalAddress, sFilePath));
#endif

	//	Done

	SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, dFileDownloadDesc, Msg);
	}

void CAeonEngine::OnBoot (void)

//	OnBoot
//
//	Boot the engine

	{
	//	Register, in case we need to store images.

	CAEONLuminous::Boot();

	//	Compute the maximum amount of memory that we should use

	SSystemMemoryInfo Memory;
	sysGetMemoryInfo(&Memory);

	//	Use 20% of physical memory for table row caches

	m_dwMaxMemoryUse = (DWORD)Min((DWORDLONG)INT_MAX, Memory.dwlTotalPhysicalMemory / 5);

	//	Register our ports

	AddPort(ADDRESS_AEON_COMMAND);
	AddVirtualPort(VIRTUAL_PORT_AEON_COMMAND, ADDRESS_AEON_COMMAND, FLAG_PORT_NEAREST);

	//	We subscribe to notifications from this machine's Exarch

	AddPort(ADDRESS_AEON_PRIVATE);
	AddVirtualPort(VIRTUAL_PORT_EXARCH_NOTIFY, ADDRESS_AEON_PRIVATE, FLAG_PORT_ALWAYS_MACHINE);

	//	We subscribe to Mnemosynth notifications

	AddVirtualPort(VIRTUAL_PORT_MNEMOSYNTH_NOTIFY, ADDRESS_AEON_PRIVATE, FLAG_PORT_ALWAYS_MODULE);
	}

void CAeonEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark datums for garbage collection (we can also use this
//	opportunity to garbage collect our own stuff.

	{
	int i;

	for (i = 0; i < m_Tables.GetCount(); i++)
		m_Tables[i]->Mark();
	}

void CAeonEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Start running

	{
	//	If we're in console mode, then we never get a machine start message, so 
	//	we need to initialize everything here.

	if (m_bConsoleMode)
		{
		CString sError;
		if (!InitConsoleMode(m_sConsoleStorage, &sError))
			{
			printf("ERROR: %s\n", (LPSTR)sError);
			return;
			}
		}
	}

void CAeonEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Stop running

	{
	FlushTableRows();

#ifdef DEBUG
	printf("AeonDB terminated.\n");
#endif
	}

bool CAeonEngine::Open (void)

//	Open
//
//	Open the database

	{
	//	Load the table definition file from all volumes

	if (!OpenTableDefinitions())
		return false;

	//	Done

	m_bReady = true;
	return true;
	}

bool CAeonEngine::OpenLocalVolumes (void)

//	OpenLocalVolumes
//
//	Initializes the m_LocalVolumes list. Returns TRUE if successful.

	{
	//	Get the list of volumes in the entire arcology

	CString sError;
	if (!m_LocalVolumes.Init(GetProcessCtx(), STR_AEON_FOLDER, &sError))
		{
		Log(MSG_LOG_ERROR, sError);
		return false;
		}

	//	Done

	return true;
	}

bool CAeonEngine::OpenTableDefinitions (void)

//	OpenTableDefinitions
//
//	Opens and reads all tables in all volumes. Note that we have no idea what
//	could have happened since our last boot--someone could have copied files
//	all over the place. We make almost no assumptions.

	{
	CSmartLock Lock(m_cs);

	int i, j;
	CString sError;

	//	Loop over all volumes and end up with a list of tables and volumes

	TSortMap<CString, TArray<CString>> Tables;
	for (i = 0; i < m_LocalVolumes.GetCount(); i++)
		{
		CString sVolume = m_LocalVolumes.GetVolume(i);

		TArray<CString> Dirs;
		fileGetFileList(fileAppend(m_LocalVolumes.GetPath(i), FILESPEC_TABLE_DIR_FILTER), FFL_FLAG_DIRECTORIES_ONLY | FFL_FLAG_RELATIVE_FILESPEC, &Dirs);

		for (j = 0; j < Dirs.GetCount(); j++)
			{
			TArray<CString> *pList = Tables.SetAt(Dirs[j]);
			pList->Insert(sVolume);
			}
		}

	//	Open all tables

	for (i = 0; i < Tables.GetCount(); i++)
		{
		CString sName = Tables.GetKey(i);

		CAeonTable *pTable = new CAeonTable;
		if (!pTable->Open(GetProcessCtx(), &m_LocalVolumes, sName, Tables[i], &sError))
			{
			Log(MSG_LOG_ERROR, strPattern("Unable to load %s: %s", sName, sError));
			delete pTable;
			continue;
			}

		m_Tables.Insert(sName, pTable);
		}

	//	Done

	return true;
	}

bool CAeonEngine::ParseTableAndView (const SArchonMessage &Msg, 
									 const CHexeSecurityCtx *pSecurityCtx, 
									 CDatum dTableAndView, 
									 CAeonTable **retpTable, 
									 DWORD *retdwViewID,
									 CDatum dKey,
									 CRowKey *retKey)

//	ParseTableAndView
//
//	Parses a datum as follows:
//
//	If a single string, it specifies a table and the default view.
//	If an array with two strings, the first is the table name; the second is the view name.

	{
	CString sError;

	//	If we're not ready, then error

	if (!m_bReady)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NOT_READY, Msg);
		return false;
		}

	//	Parse the table names

	CString sTable;
	CString sView;

	if (dTableAndView.GetCount() < 2)
		sTable = dTableAndView.AsString();
	else
		{
		sTable = dTableAndView.GetElement(0).AsString();
		sView = dTableAndView.GetElement(1).AsString();
		}

	//	Make sure we have access

	if (pSecurityCtx && !pSecurityCtx->IsNamespaceAccessible(sTable))
		{
		SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_NOT_IN_SANDBOX, sTable, pSecurityCtx->GetSandboxName()), Msg);
		return false;
		}

	//	Get the table

	CAeonTable *pTable;
	if (!FindTable(sTable, &pTable))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(STR_ERROR_UNKNOWN_TABLE, sTable), Msg);
		return false;
		}

	//	Get the view. If we want a key, take this opportunity to parse it.

	DWORD dwViewID;
	if (retKey)
		{
		if (!pTable->FindViewAndPath(sView, &dwViewID, dKey, retKey, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			return false;
			}
		}

	//	Otherwise just get the view.

	else
		{
		if (!pTable->FindView(sView, &dwViewID))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY,  strPattern(ERR_UNKNOWN_VIEW, sTable, sView), Msg);
			return false;
			}
		}

	//	Done

	if (retpTable)
		*retpTable = pTable;

	if (retdwViewID)
		*retdwViewID = dwViewID;

	return true;
	}

bool CAeonEngine::ValidateAdminAccess (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	ValidateAdminAccess
//
//	Returns TRUE if service is an admin.

	{
	//	If we're not ready, then error

	if (!m_bReady)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NOT_READY, Msg);
		return false;
		}

	//	Admin

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return false;

	return true;
	}

bool CAeonEngine::ValidateTableAccess (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, const CString &sTable)

//	ValidateTableAccess
//
//	Returns TRUE if the security context allows access to the given table.

	{
	//	If we're not ready, then error

	if (!m_bReady)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NOT_READY, Msg);
		return false;
		}

	//	Make sure we have access

	if (pSecurityCtx && !pSecurityCtx->IsNamespaceAccessible(sTable))
		{
		SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_NOT_IN_SANDBOX, sTable, pSecurityCtx->GetSandboxName()), Msg);
		return false;
		}

	return true;
	}
