//	CExarchEngine.cpp
//
//	CExarchEngine class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_ARCOLOGY_PRIME,				"ArcologyPrime")
DECLARE_CONST_STRING(STR_CENTRAL_MODULE,				"CentralModule")
DECLARE_CONST_STRING(STR_CONFIG_FILENAME,				"Config.ars")
DECLARE_CONST_STRING(STR_CONFIG_SEGMENT_FILENAME,		"ConfigSegment.ars")
DECLARE_CONST_STRING(STR_ARCOLOGY_DIR,					"Arcology")
DECLARE_CONST_STRING(STR_TEMP_DIR,						"Temp")
DECLARE_CONST_STRING(STR_ALL_FILES,						"*.*")
DECLARE_CONST_STRING(STR_DEFAULT_ARCOLOGY_NAME,			"Unnamed Arcology")
DECLARE_CONST_STRING(FILESPEC_UPGRADE_ARS,				"Upgrade.ars")
DECLARE_CONST_STRING(FILESPEC_UPGRADE_FOLDER,			"Upgrade")

DECLARE_CONST_STRING(AMP1_PING,							"PING")
DECLARE_CONST_STRING(AMP1_REJOIN,						"REJOIN")

DECLARE_CONST_STRING(MODULE_ARCOLOGY,					"Arcology.exe")
DECLARE_CONST_STRING(MODULE_AEON_DB,					"AeonDB.exe")
DECLARE_CONST_STRING(MODULE_NAME_AEON_DB,				"AeonDB")

DECLARE_CONST_STRING(PORT_EXARCH_COMMAND,				"Exarch.command")
DECLARE_CONST_STRING(PORT_EXARCH_LOG,					"Exarch.log")
DECLARE_CONST_STRING(PORT_MNEMOSYNTH_COMMAND,			"Mnemosynth.command")

DECLARE_CONST_STRING(PROTOCOL_AMP1,						"amp1")

DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command@~/~")
DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule")
DECLARE_CONST_STRING(ADDRESS_EXARCH_LOG,				"Exarch.log@~/~")
DECLARE_CONST_STRING(ADDRESS_EXARCH_NOTIFY,				"Exarch.notify")
DECLARE_CONST_STRING(ADDRESS_MNEMOSYNTH_COMMAND,		"Mnemosynth.command@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_EXARCH,				"Exarch")

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address")
DECLARE_CONST_STRING(FIELD_AMP1_PORT,					"amp1Port")
DECLARE_CONST_STRING(FIELD_ARCOLOGY_PRIME_KEY,			"arcologyPrimeKey")
DECLARE_CONST_STRING(FIELD_CHECKSUM,					"checksum")
DECLARE_CONST_STRING(FIELD_DEFINITION,					"definition")
DECLARE_CONST_STRING(FIELD_FILENAME,					"filename")
DECLARE_CONST_STRING(FIELD_FILESPEC,					"filespec")
DECLARE_CONST_STRING(FIELD_HOST_ADDRESS,				"hostAddress")
DECLARE_CONST_STRING(FIELD_KEY,							"key")
DECLARE_CONST_STRING(FIELD_LOCAL_PATH,					"localPath")
DECLARE_CONST_STRING(FIELD_MACHINE_NAME,				"machineName")
DECLARE_CONST_STRING(FIELD_MACHINES,					"machines")
DECLARE_CONST_STRING(FIELD_MODULES,						"modules")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_NEXT_VOLUME,					"nextVolume")
DECLARE_CONST_STRING(FIELD_PARTIAL_POS,					"partialPos")
DECLARE_CONST_STRING(FIELD_QUOTA,						"quota")
DECLARE_CONST_STRING(FIELD_STATUS,						"status")
DECLARE_CONST_STRING(FIELD_STORAGE,						"storage")
DECLARE_CONST_STRING(FIELD_TEST_VOLUME,					"testVolume")
DECLARE_CONST_STRING(FIELD_UPGRADE_DESC,				"upgradeDesc")
DECLARE_CONST_STRING(FIELD_UPGRADE_ID,					"upgradeID")
DECLARE_CONST_STRING(FIELD_VERSION,						"version")
DECLARE_CONST_STRING(FIELD_VOLUME_NAME,					"volumeName")
DECLARE_CONST_STRING(FIELD_VOLUMES,						"volumes")

DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_INTERPRET,		"Error.unableToInterpret")
DECLARE_CONST_STRING(MSG_ESPER_CREATE_LISTENER,			"Esper.startListener")
DECLARE_CONST_STRING(MSG_ESPER_ON_AMP1,					"Esper.onAMP1")
DECLARE_CONST_STRING(MSG_ESPER_ON_CONNECT,				"Esper.onConnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_DISCONNECT,			"Esper.onDisconnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_LISTENER_STARTED,		"Esper.onListenerStarted")
DECLARE_CONST_STRING(MSG_ESPER_ON_READ,					"Esper.onRead")
DECLARE_CONST_STRING(MSG_ESPER_ON_WRITE,				"Esper.onWrite")
DECLARE_CONST_STRING(MSG_ESPER_READ,					"Esper.read")
DECLARE_CONST_STRING(MSG_ESPER_WRITE,					"Esper.write")
DECLARE_CONST_STRING(MSG_EXARCH_ON_LOG,					"Exarch.onLog")
DECLARE_CONST_STRING(MSG_EXARCH_ON_MACHINE_START,		"Exarch.onMachineStart")
DECLARE_CONST_STRING(MSG_EXARCH_ON_MODULE_START,		"Exarch.onModuleStart")
DECLARE_CONST_STRING(MSG_EXARCH_ON_MODULE_STOP,			"Exarch.onModuleStop")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")
DECLARE_CONST_STRING(MSG_LOG_WARNING,					"Log.warning")
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ENDPOINT_LIST,		"Mnemosynth.endpointList")
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_NOTIFY_ON_ARCOLOGY_UPDATE,	"Mnemosynth.notifyOnArcologyUpdate")
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_NOTIFY_ON_UPDATE,	"Mnemosynth.notifyOnEndpointUpdate")
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ON_ARCOLOGY_UPDATED,"Mnemosynth.onArcologyUpdated")
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_READ,				"Mnemosynth.read")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(MNEMO_ARC_ARCOLOGY,				"Arc.arcology")
DECLARE_CONST_STRING(MNEMO_ARC_MACHINES,				"Arc.machines")
DECLARE_CONST_STRING(MNEMO_ARC_MODULES,					"Arc.modules")
DECLARE_CONST_STRING(MNEMO_ARC_PORTS,					"Arc.ports")
DECLARE_CONST_STRING(MNEMO_ARC_STORAGE,					"Arc.storage")

DECLARE_CONST_STRING(MNEMO_STATUS_BOOTING,				"booting")
DECLARE_CONST_STRING(MNEMO_STATUS_ONLINE,				"online")
DECLARE_CONST_STRING(MNEMO_STATUS_RUNNING,				"running")
DECLARE_CONST_STRING(MNEMO_STATUS_STOPPED,				"stopped")

DECLARE_CONST_STRING(STR_PORT_EXARCH,					"7397")

DECLARE_CONST_STRING(STR_PROTOCOL_AMP1_00,				"AMP/1.00")

DECLARE_CONST_STRING(STR_AMP1_ERROR_TYPE,				"ERROR")
DECLARE_CONST_STRING(STR_AMP1_MSG_TYPE,					"MSG")
DECLARE_CONST_STRING(STR_AMP1_OK_TYPE,					"OK")

DECLARE_CONST_STRING(STR_DEBUG,							"debug")
DECLARE_CONST_STRING(STR_REPLY_MSG,						"replyMsg")
DECLARE_CONST_STRING(STR_WATERMARK,						"watermark")

DECLARE_CONST_STRING(CONFIG_ARCOLOGY,					"arcology")
DECLARE_CONST_STRING(CONFIG_DEBUG,						"debug")
DECLARE_CONST_STRING(CONFIG_LOCAL_PATH,					"localPath")
DECLARE_CONST_STRING(CONFIG_MODULE,						"module")
DECLARE_CONST_STRING(CONFIG_MODULES,					"modules")
DECLARE_CONST_STRING(CONFIG_NAME,						"name")
DECLARE_CONST_STRING(CONFIG_QUOTA,						"quota")
DECLARE_CONST_STRING(CONFIG_STORAGE,					"storage")

DECLARE_CONST_STRING(STR_DEBUG_LOG_PREFIX,				"DEBUG: ")
DECLARE_CONST_STRING(STR_ERROR_LOG_PREFIX,				"ERROR: ")
DECLARE_CONST_STRING(STR_WARNING_LOG_PREFIX,			"WARNING: ")

DECLARE_CONST_STRING(STR_MACHINE_AUTH,					"Arcology machine authenticated: %s.")
DECLARE_CONST_STRING(STR_MACHINE_STARTED,				"Machine started.")
DECLARE_CONST_STRING(STR_NO_KEYS_FOUND,					"(no keys found)")
DECLARE_CONST_STRING(STR_NO_TABLES_FOUND,				"(no tables found)")
DECLARE_CONST_STRING(STR_EMPTY_DATA,					"(nil)")

DECLARE_CONST_STRING(ERR_ARCOLOGY_EXISTS,				"%s is already part of an arcology.")
DECLARE_CONST_STRING(ERR_NO_ARCOLOGY,					"%s is not yet part of an arcology.")
DECLARE_CONST_STRING(ERR_INVALID_ARCOLOGY_NAME,			"\"%s\" is not a valid arcology name.")
DECLARE_CONST_STRING(ERR_NOTHING_TO_UPGRADE,			"All files already up to date.")
DECLARE_CONST_STRING(ERR_CANT_RESTART_CENTRAL_MODULE,	"Cannot restart CentralModule.")
DECLARE_CONST_STRING(ERR_BAD_UPGRADE_CHECKSUM,			"Checksum for file does not match: %s.")
DECLARE_CONST_STRING(ERR_DELETING_BAD_FILE,				"Deleting bad file on volume %s: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_MSG,					"Exarch: Unknown message: %s.")
DECLARE_CONST_STRING(ERR_UPGRADE_FAILED,				"Failed upgrading; please check the log for errors.")
DECLARE_CONST_STRING(ERR_INVALID_LINES,					"Invalid line count: %d.")
DECLARE_CONST_STRING(ERR_INVALID_VOLUME_NAME,			"Invalid volume name: %s.")
DECLARE_CONST_STRING(ERR_DELETING_BAD_VOLUME,			"Removing bad volume: %s.")
DECLARE_CONST_STRING(ERR_PATH_ALREADY_EXISTS,			"Test path already exists: %s.")
DECLARE_CONST_STRING(ERR_CANT_DELETE_ORIGINALS,			"Unable to clean up previous files after upgrade.")
DECLARE_CONST_STRING(ERR_CANT_SEND_TO_MNEMOSYNTH,		"Unable to contact mnemosynth in %s module.")
DECLARE_CONST_STRING(ERR_CANT_MOVE_FILE,				"Unable to copy upgrade file from %s to %s.")
DECLARE_CONST_STRING(ERR_CANT_CREATE_DRIVE,				"Unable to create a new drive mapped to: %s.")
DECLARE_CONST_STRING(ERR_CANT_CREATE_FILE,				"Unable to create file: %s.")
DECLARE_CONST_STRING(ERR_CANT_CREATE_PATH,				"Unable to create path: %s.")
DECLARE_CONST_STRING(ERR_CANT_DELETE_BAD_FILE,			"Unable to delete bad file: %s.")
DECLARE_CONST_STRING(ERR_CANT_DELETE_DRIVE,				"Unable to delete test drive: %s.")
DECLARE_CONST_STRING(ERR_CANT_DELETE_FOLDER_CONTENTS,	"Unable to delete folder contents: %s.")
DECLARE_CONST_STRING(ERR_NO_DISK_ERROR_FOUND,			"Unable to determine cause of disk error on: %s.")
DECLARE_CONST_STRING(ERR_CANT_READ_VOLUME_DIR,			"Unable to get file list from volume %s at path: %s.")
DECLARE_CONST_STRING(ERR_CANT_GET_VERSION_INFO,			"Unable to obtain file version info: %s.")
DECLARE_CONST_STRING(ERR_INVALID_STORAGE_PATH,			"Unable to open volume %s: cannot access %s.")
DECLARE_CONST_STRING(ERR_CANT_FIND_STORAGE,				"Unable to find default storage volume.")
DECLARE_CONST_STRING(ERR_UNKNOWN_VOLUME,				"Unable to find volume %s on %s.")
DECLARE_CONST_STRING(ERR_VOLUME_NOT_FOUND,				"Unable to find volume for disk error report: %s.")
DECLARE_CONST_STRING(ERR_INVALID_MODULE_PATH,			"Unable to interpret module specification: %s.")
DECLARE_CONST_STRING(ERR_CANT_OPEN_CONFIG_FILE,			"Unable to open arcology configuration file: %s.")
DECLARE_CONST_STRING(ERR_CANT_PARSE_CONFIG_FILE,		"Unable to parse arcology configuration file: %s.")
DECLARE_CONST_STRING(ERR_CANT_READ_FILE,				"Unable to read file on volume %s: %s.")
DECLARE_CONST_STRING(ERR_CANT_READ_LOG,					"Unable to read log file.")
DECLARE_CONST_STRING(ERR_CANT_REMOVE_MODULE,			"Unable to remove module: %s.")
DECLARE_CONST_STRING(ERR_CANT_RENAME_FILE,				"Unable to rename file from %s to %s.")
DECLARE_CONST_STRING(ERR_CANT_RESTART,					"Unable to restart machine.")
DECLARE_CONST_STRING(ERR_CANT_RECOVER_UPGRADED_FILE,	"Unable to restore original file after failed upgrade: %s.")
DECLARE_CONST_STRING(ERR_CANT_WRITE_CONFIG_FILE,		"Unable to write arcology configuration file: %s.")
DECLARE_CONST_STRING(ERR_CANT_WRITE,					"Unable to write to file: %s.")
DECLARE_CONST_STRING(ERR_MODULE_NOT_FOUND,				"Unknown module: %s.")
DECLARE_CONST_STRING(ERR_UPGRADED_FILE,					"Upgraded arcology system file: %s.")
DECLARE_CONST_STRING(ERR_VOLUME_ALREADY_DELETED,		"Volume %s on %s already deleted.")
DECLARE_CONST_STRING(ERR_SAME_VOLUME,					"Volume drive already part of arcology: %s.")
DECLARE_CONST_STRING(ERR_VOLUME_ALREADY_EXISTS,			"Volume name already exists: %s.")
DECLARE_CONST_STRING(ERR_DISK_SPACE_CRITICAL,			"Volume %s is out of disk space: %d MB left.")
DECLARE_CONST_STRING(ERR_DISK_SPACE_WARNING,			"Volume %s is low on disk space: %d MB left.")
DECLARE_CONST_STRING(ERR_MODULE_STOPPED,				"[%s] Module terminated.")
DECLARE_CONST_STRING(ERR_DISK_ERROR_OP,					"Disk error when %s on filespec: %s.")
DECLARE_CONST_STRING(ERR_TOO_MANY_LINES,				"Unable to return %d lines; maximum is %d.")
DECLARE_CONST_STRING(ERR_MODULE_NOT_RESTARTED,			"[%s] Module not restarted because it failed quickly.")

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_EXARCH_ADD_MACHINE,			"Exarch.addMachine")
DECLARE_CONST_STRING(MSG_EXARCH_ADD_MODULE,				"Exarch.addModule")
DECLARE_CONST_STRING(MSG_EXARCH_ADD_VOLUME,				"Exarch.addVolume")
DECLARE_CONST_STRING(MSG_EXARCH_COMPLETE_UPGRADE,		"Exarch.completeUpgrade")
DECLARE_CONST_STRING(MSG_EXARCH_CREATE_TEST_VOLUME,		"Exarch.createTestVolume")
DECLARE_CONST_STRING(MSG_EXARCH_DELETE_TEST_DRIVE,		"Exarch.deleteTestDrive")
DECLARE_CONST_STRING(MSG_EXARCH_GET_LOG_ROWS,			"Exarch.getLogRows")
DECLARE_CONST_STRING(MSG_EXARCH_GET_MACHINE_STATUS,		"Exarch.getMachineStatus")
DECLARE_CONST_STRING(MSG_EXARCH_GET_MODULE_LIST,		"Exarch.getModuleList")
DECLARE_CONST_STRING(MSG_EXARCH_GET_STATUS,				"Exarch.getStatus")
DECLARE_CONST_STRING(MSG_EXARCH_GET_STORAGE_LIST,		"Exarch.getStorageList")
DECLARE_CONST_STRING(MSG_EXARCH_MNEMOSYNTH_ENDPOINT_LIST,	"Exarch.mnemosynthEndpointList")
DECLARE_CONST_STRING(MSG_EXARCH_MNEMOSYNTH_READ,		"Exarch.mnemosynthRead")
DECLARE_CONST_STRING(MSG_EXARCH_REMOVE_MODULE,			"Exarch.removeModule")
DECLARE_CONST_STRING(MSG_EXARCH_REMOVE_VOLUME,			"Exarch.removeVolume")
DECLARE_CONST_STRING(MSG_EXARCH_REPORT_DISK_ERROR,		"Exarch.reportDiskError")
DECLARE_CONST_STRING(MSG_EXARCH_REQUEST_UPGRADE,		"Exarch.requestUpgrade")
DECLARE_CONST_STRING(MSG_EXARCH_RESTART_MACHINE,		"Exarch.restartMachine")
DECLARE_CONST_STRING(MSG_EXARCH_RESTART_MODULE,			"Exarch.restartModule")
DECLARE_CONST_STRING(MSG_EXARCH_SEND_TO_MACHINE,		"Exarch.sendToMachine")
DECLARE_CONST_STRING(MSG_EXARCH_SHUTDOWN,				"Exarch.shutdown")
DECLARE_CONST_STRING(MSG_EXARCH_UPLOAD_UPGRADE,			"Exarch.uploadUpgrade")

const DWORD DEFAULT_AMP1_PORT =							7397;

const DWORDLONG LOW_DISK_SPACE_ERROR =					MEGABYTE_DISK;
const DWORDLONG LOW_DISK_SPACE_WARNING =				200 * MEGABYTE_DISK;

const int DEFAULT_LOG_LINES =							25;
const int MAX_LOG_LINES =								10000;

#ifdef DEBUG_MODULE_RESTART
const DWORDLONG MIN_RUN_TIME_TO_RESTART =				5;	//	seconds
#else
const DWORDLONG MIN_RUN_TIME_TO_RESTART =				60;	//	seconds
#endif

CExarchEngine::SMessageHandler CExarchEngine::m_MsgHandlerList[] =
	{
		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,				&TSimpleEngine::MsgNull },

		//	Exper.onAMP1
		{	MSG_ESPER_ON_AMP1,					&CExarchEngine::MsgEsperOnAMP1 },

		//	Exper.onListenerStarted
		{	MSG_ESPER_ON_LISTENER_STARTED,		&TSimpleEngine::MsgNull },

		//	Exarch.addMachine {address} [{port}]
		{	MSG_EXARCH_ADD_MACHINE,				&CExarchEngine::MsgAddMachine },

		//	Exarch.addModule {filePath}
		{	MSG_EXARCH_ADD_MODULE,				&CExarchEngine::MsgAddModule },

		//	Exarch.addVolume {filePath} {quota}
		{	MSG_EXARCH_ADD_VOLUME,				&CExarchEngine::MsgAddVolume },

		//	Exarch.completeUpgrade
		{	MSG_EXARCH_COMPLETE_UPGRADE,		&CExarchEngine::MsgCompleteUpgrade },

		//	Exarch.createTestVolume {volume}
		{	MSG_EXARCH_CREATE_TEST_VOLUME,		&CExarchEngine::MsgCreateTestVolume },

		//	Exarch.deleteTestDrive {volume}
		{	MSG_EXARCH_DELETE_TEST_DRIVE,		&CExarchEngine::MsgDeleteTestDrive },

		//	Exarch.getLogRows {lines}
		{	MSG_EXARCH_GET_LOG_ROWS,			&CExarchEngine::MsgGetLogRows },

		//	Exarch.getMachineStatus
		{	MSG_EXARCH_GET_MACHINE_STATUS,		&CExarchEngine::MsgGetMachineStatus },

		//	Exarch.getModuleList
		{	MSG_EXARCH_GET_MODULE_LIST,			&CExarchEngine::MsgGetModuleList },

		//	Exarch.getStatus
		{	MSG_EXARCH_GET_STATUS,				&CExarchEngine::MsgGetStatus },

		//	Exarch.getStorageList
		{	MSG_EXARCH_GET_STORAGE_LIST,		&CExarchEngine::MsgGetStorageList },

		//	Exarch.mnemosynthEndpointList [{module}]
		{	MSG_EXARCH_MNEMOSYNTH_ENDPOINT_LIST,	&CExarchEngine::MsgMnemosynthEndpointList },

		//	Exarch.mnemosynthRead {collection} {key} [{module}]
		{	MSG_EXARCH_MNEMOSYNTH_READ,			&CExarchEngine::MsgMnemosynthRead },

		//	Exarch.onLog
		{	MSG_EXARCH_ON_LOG,					&CExarchEngine::MsgOnLog },

		//	Exarch.onModuleStart
		{	MSG_EXARCH_ON_MODULE_START,			&CExarchEngine::MsgOnModuleStart },

		//	Exarch.onModuleStop
		{	MSG_EXARCH_ON_MODULE_STOP,			&CExarchEngine::MsgOnModuleStop },

		//	Exarch.removeModule {module}
		{	MSG_EXARCH_REMOVE_MODULE,			&CExarchEngine::MsgRemoveModule },

		//	Exarch.removeVolume {volume}
		{	MSG_EXARCH_REMOVE_VOLUME,			&CExarchEngine::MsgRemoveVolume },

		//	Exarch.reportDiskError {filespec}
		{	MSG_EXARCH_REPORT_DISK_ERROR,		&CExarchEngine::MsgReportDiskError },

		//	Exarch.requestUpgrade {upgradeDesc}
		{	MSG_EXARCH_REQUEST_UPGRADE,			&CExarchEngine::MsgRequestUpgrade },

		//	Exarch.restartMachine
		{	MSG_EXARCH_RESTART_MACHINE,			&CExarchEngine::MsgRestartMachine },

		//	Exarch.restartModule {module}
		{	MSG_EXARCH_RESTART_MODULE,			&CExarchEngine::MsgRestartModule },

		//	Exarch.sendToMachine {machineName} {address} {msg} {ticket} {replyAddr} {payload}
		{	MSG_EXARCH_SEND_TO_MACHINE,			&CExarchEngine::MsgSendToMachine },

		//	Exarch.shutdown
		{	MSG_EXARCH_SHUTDOWN,				&CExarchEngine::MsgShutdown },

		//	Exarch.uploadUpgrade {filename} {fileUploadDesc} {data}
		{	MSG_EXARCH_UPLOAD_UPGRADE,			&CExarchEngine::MsgUploadUpgrade },

		//	Mnemosynth.onArcologyUpdated
		{	MSG_MNEMOSYNTH_ON_ARCOLOGY_UPDATED,	&CExarchEngine::MsgMnemosynthOnArcologyUpdated },
	};

int CExarchEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CExarchEngine::m_MsgHandlerList);

CExarchEngine::CExarchEngine (const SOptions &Options) : TSimpleEngine(ENGINE_NAME_EXARCH, 3),
		m_sArcologyPrime(Options.sArcologyPrime),
		m_sConfigFilename(Options.sConfigFilename),
		m_pLoggingPort(NULL),
		m_dwNextVolume(0),
		m_bInStopRunning(false)

//	CExarchEngine constructor

	{
	m_dwAMP1Port = Options.dwAMP1Port;
	if (m_dwAMP1Port == 0)
		m_dwAMP1Port = DEFAULT_AMP1_PORT;
	}

CExarchEngine::~CExarchEngine (void)

//	CExarchEngine destructor

	{
	if (m_pLoggingPort)
		delete m_pLoggingPort;
	}

bool CExarchEngine::AddMachineConnection (const CString &sHostname, CString *retsError)
	{
	CSmartLock Lock(m_cs);

//	SendMessageCommand(ADDRESS_ESPER_COMMAND, MSG_ESPER_CREATE_LISTENER, ADDRESS_EXARCH_COMMAND, 0, CDatum(pPayload));

	//	Done

	return true;
	}

bool CExarchEngine::AddModule (const CString &sModuleInput, bool bDebug, CString *retsModuleName, CString *retsError)

//	AddModule
//
//	Loads a new module on this machine

	{
	CSmartLock Lock(m_cs);

	if (IsArcologyPrime() && !CheckArcology(retsError))
		return false;

	//	The module path better not be absolute or dotted (otherwise people could load modules
	//	outside of the module directory).

	if (fileIsAbsolute(sModuleInput) || fileIsDotted(sModuleInput))
		{
		*retsError = strPattern(ERR_INVALID_MODULE_PATH, sModuleInput);
		return false;
		}

	//	Load the module

	CString sModuleName;
	if (!m_MecharcologyDb.LoadModule(sModuleInput, bDebug, &sModuleName, retsError))
		return false;

	//	Register an event so that we get a message when the process terminates

	CProcess *pProcess = m_MecharcologyDb.GetModuleProcess(sModuleName);
	if (pProcess == NULL)
		{
		ASSERT(false);
		return false;
		}

#ifdef DEBUG_MODULE_RESTART
	printf("AddModule: Requesting event for %s = %s = %s\n", (LPSTR)sModuleInput, (LPSTR)sModuleName, (LPSTR)pProcess->GetExecutableFilespec());
#endif

	AddEventRequest(sModuleName, *pProcess, this, MSG_EXARCH_ON_MODULE_STOP, 0);

	//	Done

	if (retsModuleName)
		*retsModuleName = sModuleName;

	return true;
	}

bool CExarchEngine::AddStorage (const CString &sName, const CString &sLocalPath, DWORD dwQuota, const CString &sTestDrive, CString *retsError)

//	AddStorage
//
//	Adds a storage location for this machine

	{
	CSmartLock Lock(m_cs);

	//	Make sure the path exists

	if (!fileExists(sLocalPath))
		{
		if (!filePathCreate(sLocalPath))
			{
			*retsError = strPattern(ERR_INVALID_STORAGE_PATH, sName, sLocalPath);
			return false;
			}
		}

	//	Generate a unique resource name
	//	NOTE: This is not a stable name. It will change across boot cycles.

	CString sResourceName = GenerateResourceNameFromFilespec(sLocalPath);

	//	Create a descriptor for this storage location

	CComplexStruct *pStruct = new CComplexStruct;
	pStruct->SetElement(FIELD_VOLUME_NAME, sName);
	pStruct->SetElement(FIELD_LOCAL_PATH, sLocalPath);
	pStruct->SetElement(FIELD_MACHINE_NAME, GetMachineName());
	pStruct->SetElement(FIELD_QUOTA, (int)dwQuota);
	pStruct->SetElement(FIELD_STATUS, MNEMO_STATUS_ONLINE);
	if (!sTestDrive.IsEmpty())
		pStruct->SetElement(FIELD_TEST_VOLUME, sTestDrive);
	MnemosynthWrite(MNEMO_ARC_STORAGE, sResourceName, CDatum(pStruct));

	//	Done

	return true;
	}

bool CExarchEngine::CheckArcology (CString *retsError)

//	CheckArcology
//
//	Returns TRUE if the arcology is well-formed. If not, then we return false and
//	set retsError appropriately.

	{
	CDatum dArcDef = MnemosynthRead(MNEMO_ARC_ARCOLOGY, FIELD_DEFINITION);
	if (dArcDef.GetElement(FIELD_NAME).IsNil())
		{
		SMachineDesc MachineDesc;
		m_MecharcologyDb.GetMachine(NULL_STR, &MachineDesc);
		*retsError = strPattern(ERR_NO_ARCOLOGY, MachineDesc.sName);
		return false;
		}

	return true;
	}

void CExarchEngine::CleanUpUpgrade (void)

//	CleanUpUpgrade
//
//	Cleans up temp files after an upgrade

	{
	int i;

	CString sRootFolder = fileGetPath(fileGetExecutableFilespec());
	CString sUpgradeFolder = fileAppend(sRootFolder, FILESPEC_UPGRADE_FOLDER);

	if (!fileExists(sUpgradeFolder))
		return;

	//	Clean up the upgrade directory

	if (!filePathDelete(sUpgradeFolder, FPD_FLAG_RECURSIVE))
		Log(MSG_LOG_ERROR, strPattern(ERR_CANT_DELETE_FOLDER_CONTENTS, sUpgradeFolder));

	//	Now delete all original files

	TArray<CString> FileList;
	if (!fileGetFileList(sRootFolder, NULL_STR, CString("Original_*.*"), 0, &FileList))
		Log(MSG_LOG_ERROR, ERR_CANT_DELETE_ORIGINALS);

	bool bSucceded = true;
	for (i = 0; i < FileList.GetCount(); i++)
		if (!fileDelete(FileList[i]))
			bSucceded = false;

	//	Success?

	if (!bSucceded)
		Log(MSG_LOG_ERROR, ERR_CANT_DELETE_ORIGINALS);

	//	Done
	}

bool CExarchEngine::CreateArcology (const CString &sName, const CString &sHostAddress, CString *retsError)

//	CreateArcology
//
//	Creates a new arcology of the given name (and with the given host address).
//	Returns FALSE if creation failed.

	{
	CSmartLock Lock(m_cs);

	//	Must be a valid name

	if (sName.IsEmpty())
		{
		*retsError = strPattern(ERR_INVALID_ARCOLOGY_NAME, sName);
		return false;
		}

	//	Get the machine name

	SMachineDesc MachineDesc;
	if (!m_MecharcologyDb.GetMachine(NULL_STR, &MachineDesc))
		{
		ASSERT(false);
		return false;
		}

	//	If we already have an arcology, then we fail.

	CDatum dArcDef = MnemosynthRead(MNEMO_ARC_ARCOLOGY, FIELD_DEFINITION);
	if (!dArcDef.GetElement(FIELD_NAME).IsNil())
		{
		*retsError = strPattern(ERR_ARCOLOGY_EXISTS, MachineDesc.sName);
		return false;
		}

	//	Set the arcology name

	CComplexStruct *pStruct = new CComplexStruct;
	pStruct->SetElement(FIELD_NAME, sName);
	MnemosynthWrite(MNEMO_ARC_ARCOLOGY, FIELD_DEFINITION, CDatum(pStruct));

	//	Set the machine host

	if (!sHostAddress.IsEmpty())
		{
		MachineDesc.sAddress = sHostAddress;
		if (!m_MecharcologyDb.SetHostAddress(NULL_STR, sHostAddress))
			{
			ASSERT(false);
			return false;
			}
		}

	//	Done

	return true;
	}

bool CExarchEngine::CreateStorageConfig (void)

//	CreateStorageConfig
//
//	Creates an initial storage configuration.
//
//	FORMAT
//
//	{
//	volumes:
//		(
//			{ name:"vol01" localPath:"..." status:"online" }
//			...
//		)
//
//	nextVolume: 2
//	}

	{
	m_dwNextVolume = 1;

	//	If we're Arcology Prime, then add at one storage volume by default

	if (IsArcologyPrime())
		{
		//	Start with a single volume: the drive where the executable is.

		CString sDrive = fileGetDrive(fileGetExecutableFilespec());
		if (sDrive.IsEmpty())
			{
			Log(MSG_LOG_ERROR, ERR_CANT_FIND_STORAGE);
			return false;
			}

		//	Get a path name

		CString sLocalPath = fileAppend(sDrive, STR_ARCOLOGY_DIR);

		//	Make sure the path exists

		if (!fileExists(sLocalPath))
			{
			if (!filePathCreate(sLocalPath))
				{
				Log(MSG_LOG_ERROR, ERR_CANT_FIND_STORAGE);
				return false;
				}
			}

		//	Generate a volume name

		CString sName = strPattern("vol%03d", m_dwNextVolume++);

		//	Add the storage

		CString sError;
		if (!AddStorage(sName, sLocalPath, 1000, NULL_STR, &sError))
			{
			Log(MSG_LOG_ERROR, sError);
			return false;
			}

		//	Write the storage config file.

		if (!WriteStorageConfig())
			return false;
		}

	//	Done

	return true;
	}

void CExarchEngine::DeleteMachineResources (const CString &sName)

//	DeleteMachineResources
//
//	Delete all Mnemosynth resources for the given machine. NOTE: We assume that
//	callers have locked.

	{
	int i;

	//	Delete the machine

	GetMnemosynth().Delete(MNEMO_ARC_MACHINES, sName);

	//	Delete all modules for this machine, by searching for a given pattern.

	CString sPattern = strPattern("%s/", sName);

	TArray<CString> Modules;
	MnemosynthReadCollection(MNEMO_ARC_MODULES, &Modules);
	for (i = 0; i < Modules.GetCount(); i++)
		if (strStartsWith(Modules[i], sPattern))
			DeleteModuleResources(Modules[i], true);

	//	Delete all storage 

	TArray<CString> Storage;
	MnemosynthReadCollection(MNEMO_ARC_STORAGE, &Storage);
	for (i = 0; i < Storage.GetCount(); i++)
		if (strStartsWith(Storage[i], sPattern))
			GetMnemosynth().Delete(MNEMO_ARC_STORAGE, Storage[i]);
	}

void CExarchEngine::DeleteModuleResources (const CString &sName, bool bDeleteModule)

//	DeleteModuleResources
//
//	Delete all Mnemosynth resources for the given module. NOTE: We assume that
//	callers have locked.

	{
	//	Delete the module

	if (bDeleteModule)
		GetMnemosynth().Delete(MNEMO_ARC_MODULES, sName);

	//	Otherwise, just mark the module as stopped.

	else
		{
		CComplexStruct *pModuleData = new CComplexStruct(MnemosynthRead(MNEMO_ARC_MODULES, sName));
		pModuleData->SetElement(FIELD_STATUS, MNEMO_STATUS_STOPPED);

		MnemosynthWrite(MNEMO_ARC_MODULES, 
				sName, 
				CDatum(pModuleData));
		}

	//	Delete all ports for this module

	GetTransporter().OnModuleDeleted(sName);
	}

bool CExarchEngine::DeleteStorage (const CString &sVolume, CString *retsError)

//	DeleteStorage
//
//	Deletes a volume.

	{
	CSmartLock Lock(m_cs);
	CString sVolumeLower = strToLower(sVolume);

	//	Find the volume.

	CDatum dVolumeDesc;
	CString sKey;
	if (!FindVolume(sVolume, &dVolumeDesc, &sKey))
		{
		//	If we can't find the volume, but the volume is listed as a deleted
		//	volume, then we continue normally.
		//
		//	This can happen during testing if we detect that a volume should be
		//	deleted	before the test script deletes the volume (or vice versa).
		//
		//	We log the issue, but continue normally.

		if (m_DeletedVolumes.Find(sVolumeLower))
			{
			Log(MSG_LOG_INFO, strPattern(ERR_VOLUME_ALREADY_DELETED, sVolume, GetMachineName()));
			return true;
			}

		//	Otherwise, we return an error.

		else
			{
			*retsError = strPattern(ERR_UNKNOWN_VOLUME, sVolume, GetMachineName());
			return false;
			}
		}

	//	Delete the storage

	GetMnemosynth().Delete(MNEMO_ARC_STORAGE, sKey);

	//	If this is a test drive, try to delete the mapped path.

	const CString &sMappedPath = dVolumeDesc.GetElement(FIELD_TEST_VOLUME);
	if (!sMappedPath.IsEmpty())
		filePathDelete(sMappedPath, FPD_FLAG_RECURSIVE);

	//	Write the storage config file.

	if (!WriteStorageConfig())
		return false;

	//	We add this volume to the list of volumes that have been deleted. We
	//	do this only for UI purposes. No need to persist this.

	if (!m_DeletedVolumes.Find(sVolumeLower))
		m_DeletedVolumes.Insert(sVolumeLower);

	//	Done

	return true;
	}

bool CExarchEngine::ExarchAuthenticateMachine (const CString &sMachineName, const CIPInteger &Key)

//	ExarchAuthenticateMachine
//
//	Authenticate the given machine. We return TRUE if the given key is valid 
//	for the given machine name.

	{
	return m_MecharcologyDb.AuthenticateMachine(sMachineName, Key);
	}

void CExarchEngine::ExecuteProtocol (ISessionCtx *pCtx, const CString &sInput, CString *retsOutput)

//	ExecuteProtocol
//
//	Executes a command coming over Esper.

	{
	//	Dispatch to appropriate protocol handler

	if (pCtx->GetType() == stypeArcology)
		ExecuteProtocolAMP1((CArcologySession *)pCtx, sInput, 0, retsOutput);

	//	Otherwise, unknown protocol

	else
		*retsOutput = strPattern("%s ERROR %s|", STR_PROTOCOL_AMP1_00, MSG_ERROR_UNABLE_TO_INTERPRET);
	}

void CExarchEngine::ExecuteProtocolAMP1 (CArcologySession *pCtx, const CString &sInput, DWORD dwVersion, CString *retsOutput)

//	ExecuteProtocolAMP1
//
//	Executes Archon Message Port protocol 1. This is used to send messages from one
//	machine in the arcology to another.
//
//	There are three types of inputs:
//
//	AMP/1.00 MSG {replyAddr} {ticket} {msg} {payload}
//	AMP/1.00 ERROR {ticket} {msg}
//	AMP/1.00 OK {ticket}
//
//	LATER: Figure out how to cryptographically identify members of the arcology.

	{
	CBuffer Buffer(sInput);

	//	Read the input type

	CDatum dDatum;
	if (!CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &dDatum))
		{
		*retsOutput = strPattern("%s ERROR 0 %s", STR_PROTOCOL_AMP1_00, MSG_ERROR_UNABLE_TO_INTERPRET);
		return;
		}

	CString sType = (const CString &)dDatum;
	if (strEquals(sType, STR_AMP1_MSG_TYPE))
		{
		SArchonMessage Msg;

		//	The input consists of an array of Datums sufficient to create a message.
		//	The Datums are in the following order:
		//
		//	'MSG' (the string "MSG" indicating that this is a message)
		//	Reply address (a full physical address or Nil)
		//	Ticket # (a 32-bit ticket number integer)
		//	Message
		//	Payload element 1
		//	Payload element 2
		//	...
		//	Payload element n

		//	Get the reply address
		
		CDatum dDatum;
		if (!CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &dDatum))
			{
			*retsOutput = strPattern("%s ERROR 0 %s", STR_PROTOCOL_AMP1_00, MSG_ERROR_UNABLE_TO_INTERPRET);
			return;
			}

		Msg.sReplyAddr = (const CString &)dDatum;

		//	Get the ticket

		if (!CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &dDatum))
			{
			*retsOutput = strPattern("%s ERROR 0 %s", STR_PROTOCOL_AMP1_00, MSG_ERROR_UNABLE_TO_INTERPRET);
			return;
			}

		Msg.dwTicket = (int)dDatum;

		//	Get the message

		if (!CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &dDatum))
			{
			*retsOutput = strPattern("%s ERROR &d %s", STR_PROTOCOL_AMP1_00, Msg.dwTicket, MSG_ERROR_UNABLE_TO_INTERPRET);
			return;
			}

		Msg.sMsg = (const CString &)dDatum;

		//	The remaining datums are the payload

		CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &Msg.dPayload);

		//	Now send the message (we process it synchronously [and recursively], since we have the thread)

		CArchonMessageList List;
		List.Insert(Msg);
		OnProcessMessages(List);

		//	Send an ack (this just means that we received the message, not necessarily
		//	that we could process it).

		*retsOutput = strPattern("%s OK %d", STR_PROTOCOL_AMP1_00, Msg.dwTicket);
		}
	else if (strEquals(sType, STR_AMP1_ERROR_TYPE))
		{
		//	ERROR: no need for return
		*retsOutput = NULL_STR;
		}
	else if (strEquals(sType, STR_AMP1_OK_TYPE))
		{
		//	OK: no need for return
		*retsOutput = NULL_STR;
		}
	else
		*retsOutput = strPattern("%s ERROR 0 %s", STR_PROTOCOL_AMP1_00, MSG_ERROR_UNABLE_TO_INTERPRET);
	}

bool CExarchEngine::FindVolume (const CString &sVolume, CDatum *retdVolumeDesc, CString *retsKey)

//	FindVolume
//
//	Finds a volume by name.

	{
	int i;

	CString sMachine = GetMachineName();
	TArray<CString> VolumeKeys;
	MnemosynthReadCollection(MNEMO_ARC_STORAGE, &VolumeKeys);

	for (i = 0; i < VolumeKeys.GetCount(); i++)
		{
		//	Only check for volumes on this machine

		if (strStartsWith(VolumeKeys[i], sMachine))
			{
			CDatum dVolumeDesc = MnemosynthRead(MNEMO_ARC_STORAGE, VolumeKeys[i]);
			if (strEquals(sVolume, dVolumeDesc.GetElement(FIELD_VOLUME_NAME)))
				{
				if (retdVolumeDesc)
					*retdVolumeDesc = dVolumeDesc;

				if (retsKey)
					*retsKey = VolumeKeys[i];

				return true;
				}
			}
		}

	return false;
	}

CString CExarchEngine::GenerateResourceNameFromFilespec (const CString &sFilespec)

//	GenerateResourceNameFromFilespec
//
//	Returns a unique resource name

	{
	ASSERT(!sFilespec.IsEmpty());

	CString sAbsolute = fileGetAbsoluteFilespec(sFilespec);
	CString sCleaned = sAbsolute;

	char *pSource = sAbsolute.GetParsePointer();
	char *pSourceEnd = pSource + sAbsolute.GetLength();
	char *pDest = sCleaned.GetParsePointer();
	bool bUnderbar = false;
	while (pSource < pSourceEnd)
		{
		if ((BYTE)*pSource <= (BYTE)' ' || *pSource == '\\' || *pSource == '.' || *pSource == ':'
				|| *pSource == '(' || *pSource == ')' || *pSource == '{' || *pSource == '}'
				|| *pSource == '/' || *pSource == ',' || *pSource == '[' || *pSource == ']')
			{
			if (!bUnderbar)
				{
				*pDest++ = '-';
				bUnderbar = true;
				}
			}
		else
			{
			*pDest++ = *pSource;
			bUnderbar = false;
			}

		pSource++;
		}

	//	We're sure that we have enough room because we never add characters
	//	[Note that the NULL might be at a different place from the end of the string.]

	*pDest++ = '\0';

	//	Done

	return strPattern("%s/%s", GetMachineName(), sCleaned);
	}

CString CExarchEngine::GetMachineStatus (void)

//	GetMachineStatus
//
//	Returns the current status of the machine

	{
	CDatum dData = MnemosynthRead(MNEMO_ARC_MACHINES, GetMachineName());
	return (const CString &)dData.GetElement(FIELD_STATUS);
	}

bool CExarchEngine::IsRestartRequired (const CString &sFilename)

//	IsRestartRequired
//
//	Returns TRUE if upgrading the given file requires us to restart the
//	machine.

	{
	return (fileIsPathEqual(sFilename, MODULE_ARCOLOGY)
			|| fileIsPathEqual(sFilename, MODULE_AEON_DB));
	}

bool CExarchEngine::LoadStorageConfig (CDatum dStorageConfig)

//	LoadStorageConfig
//
//	Loads the Storage.ars file and adds any storage volumes that it finds.

	{
	CString sError;
	int i;

	//	If no storage then we need to create some

	if (dStorageConfig.IsNil())
		return CreateStorageConfig();

	//	Get the list of volumes

	CDatum dVolumes = dStorageConfig.GetElement(FIELD_VOLUMES);
	for (i = 0; i < dVolumes.GetCount(); i++)
		{
		CDatum dVolumeDesc = dVolumes.GetElement(i);
		CString sName = dVolumeDesc.GetElement(FIELD_VOLUME_NAME);
		CString sLocalPath = dVolumeDesc.GetElement(FIELD_LOCAL_PATH);
		DWORD dwQuota = (DWORD)(int)dVolumeDesc.GetElement(CONFIG_QUOTA);
		CString sTestDrive = dVolumeDesc.GetElement(FIELD_TEST_VOLUME);

		if (!AddStorage(sName, sLocalPath, dwQuota, sTestDrive, &sError))
			Log(MSG_LOG_ERROR, sError);
		}

	//	Remember the next volume number, in case we need to add more volumes

	m_dwNextVolume = (int)dStorageConfig.GetElement(FIELD_NEXT_VOLUME);

	//	Done

	return true;
	}

void CExarchEngine::MsgAddVolume (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAddVolume
//
//	Exarch.addVolume {filePath} {quota}

	{
	CSmartLock Lock(m_cs);
	int i;
	CString sError;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	if (!CheckArcology(&sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Get parameters

	CString sNewFilePath = Msg.dPayload.GetElement(0);
	CString sNewDrive = fileGetDrive(sNewFilePath);
	DWORD dwQuota = (int)Msg.dPayload.GetElement(1);
	if (dwQuota == 0)
		dwQuota = 1000;

	//	Make sure that this new volume does not conflict with any existing
	//	volume.

	CString sMachine = GetMachineName();
	TArray<CString> VolumeKeys;
	MnemosynthReadCollection(MNEMO_ARC_STORAGE, &VolumeKeys);

	for (i = 0; i < VolumeKeys.GetCount(); i++)
		{
		//	Only check for volumes on this machine

		if (strStartsWith(VolumeKeys[i], sMachine))
			{
			CDatum dVolumeDesc = MnemosynthRead(MNEMO_ARC_STORAGE, VolumeKeys[i]);
			const CString &sFilePath = dVolumeDesc.GetElement(FIELD_LOCAL_PATH);

			//	If both are on the same drive then this is a conflict

			if (strEquals(fileGetDrive(sFilePath), sNewDrive))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_SAME_VOLUME, sNewFilePath), Msg);
				return;
				}
			}
		}

	//	Generate a volume name

	CString sName = strPattern("vol%03d", m_dwNextVolume++);

	//	Add the storage

	if (!AddStorage(sName, sNewFilePath, dwQuota, NULL_STR, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Write the storage config file.

	WriteStorageConfig();

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::MsgCompleteUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	CompleteUpgrade
//
//	Exarch.completeUpgrade
//
//	Upgrades the machine after files have been uploaded.

	{
	CSmartLock Lock(m_cs);
	int i;
	CString sError;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get a path to the upgrade folder.

	CString sRootFolder = fileGetPath(fileGetExecutableFilespec());
	CString sUpgradeFolder = fileAppend(sRootFolder, FILESPEC_UPGRADE_FOLDER);
	CString sUpgradeConfig = fileAppend(sUpgradeFolder, FILESPEC_UPGRADE_ARS);

	//	Load the config file

	CDatum dConfig;
	if (!CDatum::CreateFromFile(sUpgradeConfig, CDatum::formatAEONScript, &dConfig, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	CDatum dUpgradeDesc = dConfig.GetElement(FIELD_UPGRADE_DESC);

	//	Loop over all files to upgrade and make sure the checksum is valid.

	for (i = 0; i < dUpgradeDesc.GetCount(); i++)
		{
		CDatum dFileDesc = dUpgradeDesc.GetElement(i);
		const CString &sFilename = dFileDesc.GetElement(FIELD_FILENAME);
		CString sFilespec = fileAppend(sUpgradeFolder, sFilename);

		DWORD dwChecksum = fileChecksumAdler32(sFilespec);
		if (dwChecksum != (DWORD)(int)dFileDesc.GetElement(FIELD_CHECKSUM))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_BAD_UPGRADE_CHECKSUM, sFilename), Msg);
			return;
			}
		}

	//	Rename all the files that we want to replace. On Windows we can rename a
	//	a file even if it is open.
	//
	//	We keep track of files that we moved so that if we fail we can recover.

	bool bFailed = false;
	TArray<CString> Moved;
	for (i = 0; i < dUpgradeDesc.GetCount(); i++)
		{
		CDatum dFileDesc = dUpgradeDesc.GetElement(i);
		const CString &sFilename = dFileDesc.GetElement(FIELD_FILENAME);
		CString sFilespec = fileAppend(sRootFolder, sFilename);
		CString sDest = fileAppend(sRootFolder, strPattern("Original_%s", sFilename));

		//	If the file doesn't exist it means that there is no original
		//	(We are uploading a brand new file.)

		if (!fileExists(sFilespec))
			continue;

		//	If the backup file exists then we need to delete it.

		if (fileExists(sDest))
			fileDelete(sDest);

		//	Rename the original file

		if (!fileMove(sFilespec, sDest))
			{
			Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RENAME_FILE, sFilespec, sDest));

			bFailed = true;
			break;
			}

		Moved.Insert(sFilename);
		}

	//	If the move succeeded then we move the upgraded files to the proper place

	TArray<CString> Upgraded;
	if (!bFailed)
		{
		for (i = 0; i < Moved.GetCount(); i++)
			{
			CString sSource = fileAppend(sUpgradeFolder, Moved[i]);
			CString sDest = fileAppend(sRootFolder, Moved[i]);

			if (!fileMove(sSource, sDest))
				{
				Log(MSG_LOG_ERROR, strPattern(ERR_CANT_MOVE_FILE, sSource, sDest));

				bFailed = true;
				break;
				}

			Upgraded.Insert(Moved[i]);
			}
		}

	//	If we failed either step then we need to recover

	if (bFailed)
		{
		//	Delete any files that got upgraded

		for (i = 0; i < Upgraded.GetCount(); i++)
			{
			CString sFilespec = fileAppend(sRootFolder, Upgraded[i]);
			if (!fileDelete(sFilespec))
				Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, sFilespec));
			}

		//	Move back the original files

		for (i = 0; i < Moved.GetCount(); i++)
			{
			CString sSource = fileAppend(sRootFolder, strPattern("Original_%s", Moved[i]));
			CString sDest = fileAppend(sRootFolder, Moved[i]);

			if (!fileMove(sSource, sDest))
				Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, sDest));
			}

		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_UPGRADE_FAILED, Msg);
		return;
		}

	//	Log the upgrade and see if we need to restart

	bool bRestartNeeded = false;
	for (i = 0; i < Upgraded.GetCount(); i++)
		{
		Log(MSG_LOG_INFO, strPattern(ERR_UPGRADED_FILE, Upgraded[i]));

		if (IsRestartRequired(Upgraded[i]))
			bRestartNeeded = true;
		}

	//	If we need to restart, do it.

	if (bRestartNeeded)
		{
		//	Launch a process that will shutdown and restart the service.

		bool bSuccess;
		CProcess Restart;
		try
			{
			Restart.Create(CString("Arcology.exe /restart"));
			bSuccess = true;
			}
		catch (...)
			{
			bSuccess = false;
			}

		if (!bSuccess)
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_CANT_RESTART, Msg);
			return;
			}
		}

	//	Otherwise, we restart specific modules.

	else
		{
		for (i = 0; i < Upgraded.GetCount(); i++)
			{
			//	Look for this file in the list of loaded modules. If we find it
			//	then stop it (it will get restarted automatically).

			SModuleDesc ModuleDesc;
			if (m_MecharcologyDb.FindModuleByFilespec(Upgraded[i], &ModuleDesc))
				{
				SendMessageCommand(CMessageTransporter::GenerateAddress(PORT_MNEMOSYNTH_COMMAND, ModuleDesc.sName),
						MSG_EXARCH_SHUTDOWN,
						NULL_STR, 
						0, 
						CDatum());
				}
			}
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::MsgCreateTestVolume (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateTestVolume
//
//	Exarch.createTestVolume {volume}

	{
	CSmartLock Lock(m_cs);

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Volume name must be valid

	CString sVolume = Msg.dPayload.GetElement(0);
	if (sVolume.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_VOLUME_NAME, sVolume), Msg);
		return;
		}

	//	Make sure the volume name is not already in use

	if (FindVolume(sVolume))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_VOLUME_ALREADY_EXISTS, sVolume), Msg);
		return;
		}

	//	Generate a path for the new volume (we map a drive letter to some path
	//	on the executable drive).
	//
	//	The path will be something like:
	//
	//	c:\Temp\{volName}

	CString sNewPath = fileAppend(fileAppend(fileGetDrive(fileGetExecutableFilespec()), STR_TEMP_DIR), sVolume);

	//	The path should be a new path (should not exist)

	if (fileExists(sNewPath))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_PATH_ALREADY_EXISTS, sNewPath), Msg);
		return;
		}

	//	Create the path

	if (!filePathCreate(sNewPath))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_CREATE_PATH, sNewPath), Msg);
		return;
		}

	//	Create a drive letter for that path

	CString sDriveRoot;
	if (!fileCreateDrive(sNewPath, &sDriveRoot))
		{
		filePathDelete(sNewPath, FPD_FLAG_RECURSIVE);
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_CREATE_DRIVE, sNewPath), Msg);
		return;
		}

	//	Map the volume to a subdirectory of the new drive. We do this in case 
	//	there is a race condition adding and deleting the same drive. The
	//	volume will be mapped to something like:
	//
	//	x:\{volName}
	//
	//	and x: will be mapped to:
	//
	//	c:\Temp\{volName}
	//
	//	Thus the actual directory for the volume will be:
	//
	//	c:\Temp\{volName}\{volName}

	//	Add the storage

	CString sError;
	if (!AddStorage(sVolume, fileAppend(sDriveRoot, sVolume), 0, sNewPath, &sError))
		{
		fileDeleteDrive(sDriveRoot);
		filePathDelete(sNewPath, FPD_FLAG_RECURSIVE);
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Write the storage config file.

	if (!WriteStorageConfig())
		{
		fileDeleteDrive(sDriveRoot);
		filePathDelete(sNewPath, FPD_FLAG_RECURSIVE);
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_WRITE_CONFIG_FILE, m_sConfigFilename), Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::MsgDeleteTestDrive (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgDeleteTestDrive
//
//	Exarch.deleteTestDrive {volume}

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get parameters

	CString sVolume = Msg.dPayload.GetElement(0);

	//	Find the volume.

	CDatum dVolumeDesc;
	CString sKey;
	if (!FindVolume(sVolume, &dVolumeDesc, &sKey))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNKNOWN_VOLUME, sVolume, GetMachineName()), Msg);
		return;
		}

	//	Delete the drive (but don't delete the volume because we want to test
	//	how the code detects drive failure).

	CString sDrive = dVolumeDesc.GetElement(FIELD_LOCAL_PATH);
	if (!fileDeleteDrive(sDrive))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_DELETE_DRIVE, sDrive), Msg);
		return;
		}

	CString sMappedPath = fileAppend(fileAppend(fileGetDrive(fileGetExecutableFilespec()), STR_TEMP_DIR), sVolume);
	if (!filePathDelete(sMappedPath, FPD_FLAG_RECURSIVE))
		{
		//	If we fail to delete the drive then it is likely because AeonDB
		//	still has files open. To simulate a drive failure, we blank out the
		//	files.

		TArray<CString> AllFiles;
		fileGetFileList(fileAppend(sMappedPath, STR_ALL_FILES), FFL_FLAG_RECURSIVE, &AllFiles);
		for (i = 0; i < AllFiles.GetCount(); i++)
			{
			CFile theFile;

			if (!theFile.Create(AllFiles[i], 0))
				continue;

			theFile.SetLength(0);
			}
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::MsgGetLogRows (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetLogRows
//
//	Exarch.getLogRows {lines}

	{
	//	No need to lock because we don't change state.

	int iLines;
	CString sFind;

	if (Msg.dPayload.GetCount() == 2)
		{
		sFind = Msg.dPayload.GetElement(0);
		iLines = Msg.dPayload.GetElement(1);
		}
	else if (Msg.dPayload.GetElement(0).IsNumber())
		iLines = Msg.dPayload.GetElement(0);
	else
		{
		sFind = Msg.dPayload.GetElement(0);
		iLines = DEFAULT_LOG_LINES;
		}

	//	Sanitize

	if (iLines > MAX_LOG_LINES)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_TOO_MANY_LINES, iLines, MAX_LOG_LINES), Msg);
		return;
		}
	else if (iLines <= 0)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_LINES, iLines), Msg);
		return;
		}

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get the lines

	TArray<CString> Lines;
	if (!ReadBlackBox(sFind, iLines, &Lines))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_CANT_READ_LOG, Msg);
		return;
		}

	//	Return the lines

	CComplexArray *pLines = new CComplexArray(Lines);
	SendMessageReply(MSG_REPLY_DATA, CDatum(pLines), Msg);
	}

void CExarchEngine::MsgGetMachineStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetMachineStatus
//
//	Exarch.getMachineStatus

	{
	CDatum dResult;
	if (!m_MecharcologyDb.GetStatus(&dResult))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, dResult, Msg);
		return;
		}

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CExarchEngine::MsgGetModuleList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetModuleList
//
//	Exarch.getModuleList

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get the list from Mnemosynth

	TArray<CString> Modules;
	MnemosynthReadCollection(MNEMO_ARC_MODULES, &Modules);

	CComplexArray *pList = new CComplexArray;
	for (i = 0; i < Modules.GetCount(); i++)
		{
		CComplexStruct *pModule = new CComplexStruct(MnemosynthRead(MNEMO_ARC_MODULES, Modules[i]));

		CString sModuleName;
		CMnemosynthDb::ParseEndpointName(Modules[i], NULL, &sModuleName);

		pModule->SetElement(FIELD_NAME, sModuleName);

		pList->Append(CDatum(pModule));
		}

	SendMessageReply(MSG_REPLY_DATA, CDatum(pList), Msg);
	}

void CExarchEngine::MsgMnemosynthEndpointList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgMnemosynthEndpointList
//
//	Exarch.mnemosynthEndpointList [{module}]

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get the machine/module

	CString sMachineName;
	CString sModule;
	if (!ParseModuleName(Msg.dPayload.GetElement(0), &sMachineName, &sModule))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unknown module: %s.", Msg.dPayload.GetElement(0).AsString()), Msg);
		return;
		}

	//	Send a message to mnemosynth

	SendMessageCommand(CMessageTransporter::GenerateAddress(PORT_MNEMOSYNTH_COMMAND, sModule, sMachineName),
			MSG_MNEMOSYNTH_ENDPOINT_LIST,
			Msg.sReplyAddr, 
			Msg.dwTicket, 
			Msg.dPayload);
	}

void CExarchEngine::MsgMnemosynthRead (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgMnemosynthRead
//
//	Exarch.mnemosynthRead {collection} {key} [{module}]

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get the machine/module

	CString sMachineName;
	CString sModule;
	if (!ParseModuleName(Msg.dPayload.GetElement(2), &sMachineName, &sModule))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unknown module: %s.", Msg.dPayload.GetElement(2).AsString()), Msg);
		return;
		}

	//	Send a message to mnemosynth

	SendMessageCommand(CMessageTransporter::GenerateAddress(PORT_MNEMOSYNTH_COMMAND, sModule, sMachineName),
			MSG_MNEMOSYNTH_READ,
			Msg.sReplyAddr, 
			Msg.dwTicket, 
			Msg.dPayload);
	}

void CExarchEngine::MsgMnemosynthOnArcologyUpdated (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgMnemosynthOnArcologyUpdated
//
//	Mnemosynth.onArcologyUpdated (({endpointName} {sequence}) (...))

	{
	CSmartLock Lock(m_cs);
	int i;

	//	All modules are now running
	//
	//	LATER: There is probably a race condition if multiple modules are started concurrently.

	m_MecharcologyDb.OnMnemosynthUpdated();

	//	If we're not yet started, then start

	if (!strEquals(GetMachineStatus(), MNEMO_STATUS_RUNNING))
		{
		//	All modules are now running

		OnMachineStart();
		}

	//	Otherwise, this means that one or more modules just re-started. Send out
	//	a special Exarch.onMachineStart message to all modules with parameters that
	//	specify which modules are started. Receiving modules must only handle this
	//	message if they're not yet initialized and if they see their module name in
	//	the list.

	else
		{
		CComplexArray *pModules = new CComplexArray;
		for (i = 0; i < Msg.dPayload.GetCount(); i++)
			{
			CDatum dWatermark = Msg.dPayload.GetElement(i);

			CString sModule;
			CString sMachine;
			CMnemosynthDb::ParseEndpointName(dWatermark.GetElement(0).AsString(), &sMachine, &sModule);
			if (!strEquals(sMachine, GetMachineName()))
				continue;

			pModules->Append(CDatum(sModule));
			}

		SendMessageNotify(ADDRESS_EXARCH_NOTIFY, MSG_EXARCH_ON_MACHINE_START, CDatum(pModules));
		}
	}

void CExarchEngine::MsgOnLog (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgOnLog
//
//	Exarch.onLog

	{
	//	This comes from a virtual port, so we decode the original
	//	message and payload.

	const CString &sMsg = Msg.dPayload.GetElement(0);
	const CString &sText = Msg.dPayload.GetElement(1).GetElement(0);
	const CString &sSender = Msg.dPayload.GetElement(1).GetElement(1);

	//	Parse the sender

	CString sModuleName;
	CString sMachineName;
	CMessageTransporter::ParseAddress(sSender, NULL, &sModuleName, &sMachineName);

	CString sID;
	if (!strEquals(sMachineName, GetMachineName()))
		sID = strPattern("[%s/%s] ", sMachineName, sModuleName);
	else if (!strEquals(sModuleName, GetModuleName()))
		sID = strPattern("[%s] ", sModuleName);

	//	Parse the log type

	CString sPrefix;
    if (strEquals(sMsg, MSG_LOG_DEBUG))
        sPrefix = STR_DEBUG_LOG_PREFIX;
    else if (strEquals(sMsg, MSG_LOG_ERROR))
        sPrefix = STR_ERROR_LOG_PREFIX;
    else if (strEquals(sMsg, MSG_LOG_WARNING))
        sPrefix = STR_WARNING_LOG_PREFIX;

	//	Log

	LogBlackBox(strPattern("%s%s%s", sID, sPrefix, strEscapePrintf(sText)));
	}

void CExarchEngine::MsgOnModuleStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgOnModuleStart
//
//	Exarch.onModuleStart {moduleName} {seq}

	{
	CSmartLock Lock(m_cs);
	const CString &sModuleName(Msg.dPayload.GetElement(0));

	//	Make sure this module is in the list; if not, then we don't need to do
	//	anything (this could happen in a race-condition adding and removing the
	//	module).

	SModuleDesc ModuleData;
	if (!m_MecharcologyDb.GetModule(sModuleName, &ModuleData))
		{
		Log(MSG_LOG_INFO, strPattern("Module not found: %s.", sModuleName));
		return;
		}

	//	Add the module as an endpoint

	CString sFullName = CMnemosynthDb::GenerateEndpointName(GetMachineName(), sModuleName);
	GetMnemosynth().AddEndpoint(sFullName, ModuleData.dwProcessID);

	//	Set the module data in Mnemosynth

	CComplexStruct *pStruct = new CComplexStruct;
	pStruct->SetElement(FIELD_MACHINE_NAME, GetMachineName());
	pStruct->SetElement(FIELD_STATUS, MNEMO_STATUS_RUNNING);
	pStruct->SetElement(FIELD_FILESPEC, ModuleData.sFilespec);
	pStruct->SetElement(FIELD_VERSION, ModuleData.sVersion);
	MnemosynthWrite(MNEMO_ARC_MODULES, 
			sFullName, 
			CDatum(pStruct));

	bool bAllDone;
	m_MecharcologyDb.OnModuleStart(Msg.dPayload.GetElement(0), (DWORD)(int)Msg.dPayload.GetElement(1), &bAllDone);

	//	If all modules have started, then we ask our Mnemosynth to wait for
	//	all modules to reach a given watermark (based on what each module told us
	//	is the current sequence)

	if (bAllDone)
		{
		RegisterForMnemosynthUpdate();
		}
	}

void CExarchEngine::MsgOnModuleStop (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgOnModuleStop
//
//	Exarch.onModuleStop {moduleName}

	{
	CSmartLock Lock(m_cs);
	CString sModule = Msg.dPayload.GetElement(0);
	SModuleDesc OldModule;

	//	If the module is not part of the arcology, then there is nothing
	//	to do.

	if (!m_MecharcologyDb.GetModule(sModule, &OldModule))
		return;

#ifdef DEBUG_MODULE_RESTART
	printf("MsgOnModuleStop: %s\n", (LPSTR)sModule);
#endif

	//	Check to see how long this module has been up, and if it hasn't been that 
	//	long, then don't restart.

	bool bRemove = m_MecharcologyDb.IsModuleRemoved(sModule);
	bool bRestart = (m_MecharcologyDb.GetModuleRunTime(sModule).Seconds64() > MIN_RUN_TIME_TO_RESTART)
			&& !bRemove;

	//	Delete ourselves from the list of modules.

	m_MecharcologyDb.DeleteModule(sModule);

	//	If the machine is shutting down, then we don't need to do anything else.

	if (m_bInStopRunning)
		return;

	//	Log

	Log(MSG_LOG_INFO, strPattern(ERR_MODULE_STOPPED, sModule));

	//	Remove the module as an endpoint

	CString sFullName = CMnemosynthDb::GenerateEndpointName(GetMachineName(), sModule);
	GetMnemosynth().RemoveEndpoint(sFullName);

	//	Remove all module data from Mnemosynth

	DeleteModuleResources(sFullName, bRemove);

	//	If we're restarting, launch the module again

	if (bRestart)
		{
		CString sError;
		CString sModuleName;
		if (!m_MecharcologyDb.LoadModule(OldModule.sFilespec, false, &sModuleName, &sError))
			{
			Log(MSG_LOG_ERROR, sError);
			return;
			}

		//	Register an event so that we get a message when the process terminates

		CProcess *pProcess = m_MecharcologyDb.GetModuleProcess(sModuleName);
		if (pProcess == NULL)
			{
			ASSERT(false);
			return;
			}

		AddEventRequest(sModuleName, *pProcess, this, MSG_EXARCH_ON_MODULE_STOP, 0);
		}
	else
		Log(MSG_LOG_ERROR, strPattern(ERR_MODULE_NOT_RESTARTED, sModule));
	}

void CExarchEngine::MsgRemoveVolume (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRemoveVolume
//
//	Exarch.removeVolume {volume}

	{
	CSmartLock Lock(m_cs);

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get parameters

	CString sVolume = Msg.dPayload.GetElement(0);

	//	Delete the volume

	CString sError;
	if (!DeleteStorage(sVolume, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::MsgReportDiskError (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgReportDiskError
//
//	Exarch.reportDiskError {filespec}

	{
	CSmartLock Lock(m_cs);
	CString sError;
	int i;

	const CString &sFilespec = Msg.dPayload.GetElement(0);
	const CString &sOperation = Msg.dPayload.GetElement(1);

	if (!sOperation.IsEmpty())
		Log(MSG_LOG_INFO, strPattern(ERR_DISK_ERROR_OP, sOperation, sFilespec));

	//	Find the volume that holds this file.

	CString sMachine = GetMachineName();
	TArray<CString> VolumeKeys;
	MnemosynthReadCollection(MNEMO_ARC_STORAGE, &VolumeKeys);
	CDatum dVolumeDesc;
	CString sVolume;
	CString sVolumePath;
	bool bFound = false;

	for (i = 0; i < VolumeKeys.GetCount(); i++)
		{
		//	Only check for volumes on this machine

		if (strStartsWith(VolumeKeys[i], sMachine))
			{
			dVolumeDesc = MnemosynthRead(MNEMO_ARC_STORAGE, VolumeKeys[i]);
			sVolumePath = dVolumeDesc.GetElement(FIELD_LOCAL_PATH);

			//	See if we match

			if (strStartsWith(sFilespec, sVolumePath))
				{
				sVolume = dVolumeDesc.GetElement(FIELD_VOLUME_NAME);
				bFound = true;
				break;
				}
			}
		}

	//	If we didn't find the volume then it must have been deleted.

	if (!bFound)
		{
		Log(MSG_LOG_INFO, strPattern(ERR_VOLUME_NOT_FOUND, sFilespec));
		return;
		}

	//	Test to see if we can read the volume. If not, we remove the volume
	//	from the machine.
	//
	//	Start by getting a list of all files in the volume.

	bool bReadSuccessful = true;
	TArray<CString> AllFiles;
	if (!fileGetFileList(sVolumePath, NULL_STR, STR_ALL_FILES, FFL_FLAG_RECURSIVE, &AllFiles))
		{
		Log(MSG_LOG_INFO, strPattern(ERR_CANT_READ_VOLUME_DIR, sVolume, sVolumePath));
		bReadSuccessful = false;
		}

	//	If there are no files, then we clearly have an error.

	if (bReadSuccessful && AllFiles.GetCount() == 0)
		{
		Log(MSG_LOG_INFO, strPattern(ERR_CANT_READ_VOLUME_DIR, sVolume, sVolumePath));
		bReadSuccessful = false;
		}

	//	Try reading a few random files.

	if (bReadSuccessful)
		{
		int iTestReads = 4;
		int iReadsLeft = iTestReads;
		int iFailures = 0;
		while (iReadsLeft > 0)
			{
			//	Try reading the file.

			CString sTestFile = AllFiles.Random();
			if (!TestFileRead(sTestFile))
				{
				Log(MSG_LOG_INFO, strPattern(ERR_CANT_READ_FILE, sVolume, sTestFile));
				iFailures++;
				}

			//	Next

			iReadsLeft--;
			}

		//	If we have 75% success then we're OK (sporadic failures could be
		//	caused by sharing violations).

		if (iFailures > (iTestReads / 4))
			bReadSuccessful = false;
		}

	//	If reading failed then we need to take the volume offline.

	if (!bReadSuccessful)
		{
		Log(MSG_LOG_INFO, strPattern(ERR_DELETING_BAD_VOLUME, sVolume));

		if (!DeleteStorage(sVolume, &sError))
			Log(MSG_LOG_ERROR, sError);

		return;
		}

	//	See if we have enough disk space. If not, then that could be the problem.

	DWORDLONG dwAvailable;
	fileGetDriveSpace(sVolumePath, &dwAvailable);
	if (dwAvailable < LOW_DISK_SPACE_ERROR)
		{
		Log(MSG_LOG_INFO, strPattern(ERR_DISK_SPACE_CRITICAL, sVolume, dwAvailable / MEGABYTE_DISK));
		return;
		}
	
	//	If disk space is low but not critical, warn

	else if (dwAvailable < LOW_DISK_SPACE_WARNING)
		Log(MSG_LOG_INFO, strPattern(ERR_DISK_SPACE_WARNING, sVolume, dwAvailable / MEGABYTE_DISK));

	//	If reading succeeded then the problem is probably localized to the file.
	//	If the filespec does not exist then there is nothing we can do

	bool bIsFile;
	if (!fileExists(sFilespec, &bIsFile) || !bIsFile)
		{
		Log(MSG_LOG_INFO, strPattern(ERR_NO_DISK_ERROR_FOUND, sFilespec));
		return;
		}

	//	Try reading the file. If we fail then we attempt to delete it.

	if (!TestFileRead(sFilespec))
		{
		Log(MSG_LOG_INFO, strPattern(ERR_DELETING_BAD_FILE, sVolume, sFilespec));

		if (!fileDelete(sFilespec))
			Log(MSG_LOG_ERROR, strPattern(ERR_CANT_DELETE_BAD_FILE, sFilespec));

		return;
		}

	//	Otherwise, we don't know what's wrong.

	Log(MSG_LOG_INFO, strPattern(ERR_NO_DISK_ERROR_FOUND, sFilespec));
	}

void CExarchEngine::MsgRequestUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRequestUpgrade
//
//	Exarch.requestUpgrade {upgradeDesc}

	{
	CSmartLock Lock(m_cs);
	int i;
	CString sError;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get a path to the upgrade folder. Create it if it doesn't exist, empty
	//	it if it already exists.

	CString sRootFolder = fileGetPath(fileGetExecutableFilespec());
	CString sUpgradeFolder = fileAppend(sRootFolder, FILESPEC_UPGRADE_FOLDER);

	if (!filePathCreate(sUpgradeFolder, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	if (!filePathDelete(sUpgradeFolder, FPD_FLAG_RECURSIVE | FPD_FLAG_CONTENT_ONLY))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_DELETE_FOLDER_CONTENTS, sUpgradeFolder), Msg);
		return;
		}

	//	The upgradeDesc is an array of structures. Each structure has:
	//
	//	filename: A file to upgrade
	//	checksum: A checksum value for the file
	//	version: The version of the file
	//
	//	We create our own version of this structure that contains only the files
	//	that need to be upgraded.

	CDatum dRequest = Msg.dPayload.GetElement(0);
	CComplexArray *pUpgradeDesc = new CComplexArray;
	CDatum dUpgradeDesc(pUpgradeDesc);

	for (i = 0; i < dRequest.GetCount(); i++)
		{
		CDatum dFileDesc = dRequest.GetElement(i);
		CString sFilespec = fileAppend(sRootFolder, dFileDesc.GetElement(FIELD_FILENAME));

		//	If the file exists, then check to see if it needs to be upgraded

		bool bIsFile;
		if (fileExists(sFilespec, &bIsFile))
			{
			//	If not a file, then we can't upgrade

			if (!bIsFile)
				continue;

			//	Get the version

			SFileVersionInfo VersionInfo;
			if (!fileGetVersionInfo(sFilespec, &VersionInfo))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_GET_VERSION_INFO, sFilespec), Msg);
				return;
				}

			//	If the version that we have installed is greater or equal than
			//	the requested upgrade, then we skip this file.

			DWORDLONG dwRequestVer = (DWORDLONG)dFileDesc.GetElement(FIELD_VERSION);
			if (VersionInfo.dwProductVersion >= dwRequestVer)
				continue;
			}

		//	Add this file to the list of files to upgrade

		pUpgradeDesc->Insert(dFileDesc);
		}

	//	If there is nothing to do, then return an error

	if (pUpgradeDesc->GetCount() == 0)
		{
		SendMessageReplyError(MSG_ERROR_ALREADY_EXISTS, ERR_NOTHING_TO_UPGRADE, Msg);
		return;
		}

	//	We create a directory called "Upgrade" and create a file called
	//	"Upgrade.ars" which contains the upgradeDesc and an upgradeID
	//	(which is a random number).

	CComplexStruct *pUpgrade = new CComplexStruct;
	CDatum dConfig(pUpgrade);
	pUpgrade->SetElement(FIELD_UPGRADE_DESC, dUpgradeDesc);
	pUpgrade->SetElement(FIELD_UPGRADE_ID, mathRandom(1, 1000000));

	CString sUpgradeConfig = fileAppend(sUpgradeFolder, FILESPEC_UPGRADE_ARS);
	CFile ConfigFile;
	if (!ConfigFile.Create(sUpgradeConfig, CFile::FLAG_CREATE_ALWAYS))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_GET_VERSION_INFO, sUpgradeConfig), Msg);
		return;
		}

	dConfig.Serialize(CDatum::formatAEONScript, ConfigFile);

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dConfig, Msg);
	}

void CExarchEngine::MsgRestartMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRestartMachine
//
//	Exarch.restartMachine

	{
	CSmartLock Lock(m_cs);

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Launch a process that will shutdown and restart the service.

	bool bSuccess;
	CProcess Restart;
	try
		{
		Restart.Create(CString("Arcology.exe /restart"));
		bSuccess = true;
		}
	catch (...)
		{
		bSuccess = false;
		}

	if (!bSuccess)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_CANT_RESTART, Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::MsgRestartModule (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRestartModule
//
//	Exarch.restartModule {module}

	{
	CSmartLock Lock(m_cs);

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Find the module

	const CString sModuleName = Msg.dPayload.GetElement(0);
	SModuleDesc ModuleDesc;
	if (!m_MecharcologyDb.GetModule(sModuleName, &ModuleDesc))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_MODULE_NOT_FOUND, sModuleName), Msg);
		return;
		}

	//	Cannot restart central module

	if (strEquals(ModuleDesc.sName, STR_CENTRAL_MODULE))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_CANT_RESTART_CENTRAL_MODULE, Msg);
		return;
		}

	//	Restart the module

	SendMessageCommand(CMessageTransporter::GenerateAddress(PORT_MNEMOSYNTH_COMMAND, ModuleDesc.sName),
			MSG_EXARCH_SHUTDOWN,
			NULL_STR, 
			0, 
			CDatum());

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::MsgShutdown (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgShutdown
//
//	Shut down the machine.

	{
	SendMessageReply(MSG_OK, CDatum(), Msg);
	::Sleep(1000);
	GetProcessCtx()->InitiateShutdown();
	}

void CExarchEngine::MsgUploadUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgUploadUpgrade
//
//	Exarch.uploadUpgrade {filename} {fileUploadDesc} {data}
//
//	Accepts an upload of a file to upgrade.

	{
	CSmartLock Lock(m_cs);
	CString sError;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	CString sRootFolder = fileGetPath(fileGetExecutableFilespec());
	CString sUpgradeFolder = fileAppend(sRootFolder, FILESPEC_UPGRADE_FOLDER);

	//	Get the inputs

	CString sFilespec = fileAppend(sUpgradeFolder, Msg.dPayload.GetElement(0));
	CDatum dUploadDesc = Msg.dPayload.GetElement(1);
	CDatum dData = Msg.dPayload.GetElement(2);

	//	Open the file for write

	CFile theFile;
	if (!theFile.Create(sFilespec, CFile::FLAG_OPEN_ALWAYS, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Get some info from the upload desc

	DWORDLONG dwOffset = dUploadDesc.GetElement(FIELD_PARTIAL_POS);

	//	Write the data

	bool bSuccess;
	try
		{
		theFile.Seek((int)dwOffset);
		theFile.Write(dData);
		bSuccess = true;
		}
	catch (...)
		{
		bSuccess = false;
		}

	//	Failed

	if (!bSuccess)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_WRITE, sFilespec), Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::OnBoot (void)

//	OnBoot
//
//	Boot up. Initialize the engine.

	{
	//	Boot the mecharcology

	CMecharcologyDb::SInit Init;

	//	Get the module path

	Init.sModulePath = fileGetAbsoluteFilespec(fileGetPath(fileGetExecutableFilespec()));

	//	Initialize a descriptor for this process (we know that we are
	//	CentralModule because that's the only module that Exarch lives in).

	Init.sCurrentModule = STR_CENTRAL_MODULE;

	//	Initialize a descriptor for this machine

	Init.sMachineName = GetMachineName();
	Init.sMachineHost = sysGetDNSName();
	Init.sArcologyPrimeAddress = m_sArcologyPrime;

	m_MecharcologyDb.Boot(Init);

	//	Register our port

	AddPort(PORT_EXARCH_COMMAND);

	//	Register the logging port

	m_pLoggingPort = new CProxyPort(MSG_EXARCH_ON_LOG, Bind(ADDRESS_EXARCH_COMMAND));
	AddPort(PORT_EXARCH_LOG, m_pLoggingPort);

	//	Clean up in case we upgraded

	CleanUpUpgrade();
	}

void CExarchEngine::OnMachineConnection (const CString &sName)

//	OnMachineConnection
//
//	Called when another machine authenticates with us. This could either be a secondary 
//	machine connecting to us (Arcology Prime) or it could be Arcology Prime
//	connecting to us (a secondary machine).

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Remove any old endpoints. For example, if this same machine was 
	//	previously connected under a different name (because of a reboot)
	//	then we clear up the old name here.

	TArray<CString> OldNames;
	m_MecharcologyDb.ProcessOldMachines(OldNames);
	for (i = 0; i < OldNames.GetCount(); i++)
		GetMnemosynth().RemoveMachineEndpoints(OldNames[i]);

	//	Add this endpoint to Mnemosynth. (If this is NOT Arcology Prime, 
	//	then we trigger a sync now.)

	CString sFullName = CMnemosynthDb::GenerateEndpointName(sName, GetModuleName());
	GetMnemosynth().AddEndpoint(sFullName, 0, !IsArcologyPrime());

	//	Delete old machines and all their resources (these won't get deleted
	//	by ArcologyPrime because the owner is dead).

	for (i = 0; i < OldNames.GetCount(); i++)
		DeleteMachineResources(OldNames[i]);

	//	If we are Arcology Prime, then we also add this machine to Mnemosynth.

	if (IsArcologyPrime())
		{
		//	Add the machine
		//
		//	NOTE: This will also trigger a replication of Mnemosynth to all
		//	endpoints.

		CComplexStruct *pStruct = new CComplexStruct;
		pStruct->SetElement(FIELD_HOST_ADDRESS, NULL_STR);
		pStruct->SetElement(FIELD_STATUS, MNEMO_STATUS_RUNNING);

		MnemosynthWrite(MNEMO_ARC_MACHINES, 
				sName, 
				CDatum(pStruct));
		}

	//	Log it

	Log(MSG_LOG_INFO, strPattern(STR_MACHINE_AUTH, sName));

#ifdef DEBUG
	Log(MSG_LOG_DEBUG, strPattern("Added endpoint: %s.", sFullName));
	for (i = 0; i < OldNames.GetCount(); i++)
		printf("Old machine: %s\n", (LPSTR)OldNames[i]);

	TArray<CString> Machines;
	MnemosynthReadCollection(MNEMO_ARC_MACHINES, &Machines);

	for (i = 0; i < Machines.GetCount(); i++)
		{
		CDatum dMachineInfo = MnemosynthRead(MNEMO_ARC_MACHINES, Machines[i]);
		const CString &sName = dMachineInfo.GetElement(FIELD_NAME);
		printf("%s: %s\n", (LPSTR)Machines[i], (LPSTR)sName);
		}

#endif
	}

void CExarchEngine::OnMachineStart (void)

//	OnMachineStart
//
//	Called when all modules have started up. NOTE: This is called inside a lock, 
//	so we don't worry about other threads.

	{
	int i;

#ifdef DEBUG_STARTUP
	printf("[OnMachineStart]\n");
#endif

	SetMachineData(NULL_STR, MNEMO_STATUS_RUNNING);

	//	Send the onMachineAdded notification message to everyone

	SendMessageNotify(ADDRESS_EXARCH_NOTIFY, MSG_EXARCH_ON_MACHINE_START, CDatum());

	//	Ask Esper to listen on a the Hexarc port. At this point we are prepared
	//	to accept connections from other machines in the arcology.

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(ADDRESS_EXARCH_COMMAND);
	pPayload->Insert(m_dwAMP1Port);
	pPayload->Insert(PROTOCOL_AMP1);

	SendMessageCommand(ADDRESS_ESPER_COMMAND, MSG_ESPER_CREATE_LISTENER, ADDRESS_EXARCH_COMMAND, 0, CDatum(pPayload));

	//	If we're Arcology Prime, tell all our secondary machines that we're up
	//	and running.

	if (IsArcologyPrime())
		{
		for (i = 0; i < m_MecharcologyDb.GetMachineCount(); i++)
			{
			SMachineDesc Desc;
			m_MecharcologyDb.GetMachine(i, &Desc);

			SendAMP1Command(Desc, AMP1_REJOIN, CDatum());
			}
		}

	//	If we're a secondary machine, then ping Arcology Prime

	else
		{
#ifdef DEBUG_STARTUP
		printf("[OnMachineStart]: Send PING to Arcology Prime.\n");
#endif

		if (m_MecharcologyDb.HasArcologyKey())
			SendAMP1Command(NULL_STR, AMP1_PING, CDatum());
		}

	//	Done

	Log(MSG_LOG_INFO, STR_MACHINE_STARTED);
	}

void CExarchEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark datums to keep

	{
	m_dMachineConfig.Mark();

	//	Since this is guaranteed to be called from a single thread,
	//	we take the opportunity to purge some stuff

	m_Sessions.PurgeDeleted();
	}

void CExarchEngine::OnStartRunning (void)

//	OnStartRunning
//
//	This is called once after all engines have booted but before our
//	first call to OnProcessMessages. The engine can bind and send
//	messages at this point.

	{
	int i;
	CString sError;

	//	Load the configuration file

	if (!ReadConfig())
		return;

	//	Set our module info

	SModuleDesc ModuleData;
	m_MecharcologyDb.GetCentralModule(&ModuleData);

	CComplexStruct *pStruct = new CComplexStruct;
	pStruct->SetElement(FIELD_MACHINE_NAME, GetMachineName());
	pStruct->SetElement(FIELD_STATUS, MNEMO_STATUS_RUNNING);
	pStruct->SetElement(FIELD_FILESPEC, ModuleData.sFilespec);
	pStruct->SetElement(FIELD_VERSION, ModuleData.sVersion);

	CString sModuleName = strPattern("%s/CentralModule", GetMachineName());
	MnemosynthWrite(MNEMO_ARC_MODULES, 
			sModuleName, 
			CDatum(pStruct));

#ifdef DEBUG_STARTUP
	printf("[OnStartRunning]: Wrote out module: %s\n", (LPSTR)sModuleName);
#endif

	//	Add storage locations.

	if (!LoadStorageConfig(m_dMachineConfig.GetElement(FIELD_STORAGE)))
		return;

	//	Wait until we get a notification from each module that it has completed
	//	booting.

	SetMachineData(NULL_STR, MNEMO_STATUS_BOOTING);

	//	If we're Arcology Prime then we create the arcology and load all our
	//	modules. Otherwise, we wait until we connect to the arcology before
	//	we load our modules.

	int iModulesLoaded = 0;
	if (IsArcologyPrime())
		{
		//	Set arcology info

		if (!CreateArcology(STR_DEFAULT_ARCOLOGY_NAME, NULL_STR, &sError))
			{
			Log(MSG_LOG_ERROR, sError);
			return;
			}

		//	Now add the AeonDB module

		if (!AddModule(MODULE_AEON_DB, false, NULL, &sError))
			{
			Log(MSG_LOG_ERROR, sError);
			return;
			}

		iModulesLoaded++;

		//	Log

		Log(MSG_LOG_INFO, strPattern("%s loaded.", MODULE_AEON_DB));

		//	Add all the machines we know about to the arcology

		CDatum dMachines = m_dMachineConfig.GetElement(FIELD_MACHINES);
		for (i = 0; i < dMachines.GetCount(); i++)
			{
			CDatum dMachineDesc = dMachines.GetElement(i);

			if (!m_MecharcologyDb.AddMachine(dMachineDesc.GetElement(FIELD_NAME),
					dMachineDesc.GetElement(FIELD_ADDRESS),
					dMachineDesc.GetElement(FIELD_KEY),
					&sError))
				{
				Log(MSG_LOG_ERROR, sError);
				continue;
				}
			}
		}

	//	Otherwise we are not Arcology Prime

	else
		{
		//	Get the secret key to Arcology Prime stored in our config file.
		//	If it's not there, it means that we haven't yet joined an arcology.

		CIPInteger SecretKey = m_dMachineConfig.GetElement(FIELD_ARCOLOGY_PRIME_KEY);
		if (!SecretKey.IsEmpty())
			{
			if (!m_MecharcologyDb.SetArcologyKey(SecretKey, &sError))
				Log(MSG_LOG_ERROR, sError);
			}
		}

	//	Load other modules

	CDatum dModules = m_dMachineConfig.GetElement(FIELD_MODULES);
	for (i = 0; i < dModules.GetCount(); i++)
		{
		const CString &sModuleFilespec = dModules.GetElement(i);

		if (!AddModule(sModuleFilespec, false, NULL, &sError))
			{
			Log(MSG_LOG_ERROR, sError);
			continue;
			}

		iModulesLoaded++;

		Log(MSG_LOG_INFO, strPattern("%s loaded.", sModuleFilespec));
		}

	//	If we do not have any modules, then we don't need to wait for them, so 
	//	we go straight to registering for Mnemosynth

	if (iModulesLoaded == 0)
		{
#ifdef DEBUG_STARTUP
		printf("[OnStartRunning]: RegisterForMnemosynthUpdate\n");
#endif

		RegisterForMnemosynthUpdate();
		}
	}

void CExarchEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Process has asked us to stop

	{
	int i;

	//	Get a list of all modules to stop

	m_cs.Lock();

	TArray<CString> ModuleNames;
	TArray<HANDLE> ModuleWait;
	for (i = 1; i < m_MecharcologyDb.GetModuleCount(); i++)
		{
		ModuleNames.Insert(m_MecharcologyDb.GetModuleName(i));
		ModuleWait.Insert(m_MecharcologyDb.GetModuleProcess(i).GetWaitObject());
		}

	//	Remember that we are stopping

	m_bInStopRunning = true;
	m_cs.Unlock();

	//	Signal to all modules that they should quit

	for (i = 0; i < ModuleNames.GetCount(); i++)
		{
		SendMessageCommand(CMessageTransporter::GenerateAddress(PORT_MNEMOSYNTH_COMMAND, ModuleNames[i]),
				MSG_EXARCH_SHUTDOWN,
				NULL_STR, 
				0, 
				CDatum());
		}

	//	Wait for them to quit

	for (i = 0; i < ModuleWait.GetCount(); i++)
		{
		if (::WaitForSingleObject(ModuleWait[i], 30000) == WAIT_TIMEOUT)
			{
#ifdef DEBUG
			printf("Unable to wait for module termination\n");
#endif
			}
		}
	}

bool CExarchEngine::ParseMachineName (const CString &sValue, CString *retsMachineName) const

//	ParseMachineName
//
//	Parses a user-entered machine name. If the machine is found, we return TRUE.
//	retsMachineName is the full name of the machine (or blank if it is this machine).
//
//	The user may enter a partial match.

	{
	int i;

	//	If machine name is blank, then assume the current machine

	if (sValue.IsEmpty())
		{
		if (retsMachineName)
			*retsMachineName = NULL_STR;
		return true;
		}

	//	Get the list of machines.

	TArray<CString> Machines;
	MnemosynthReadCollection(MNEMO_ARC_MACHINES, &Machines);

	//	Loop over all machines

	CString sFoundMachineName;
	for (i = 0; i < Machines.GetCount(); i++)
		{
		//	If we have a partial machine name match, see if it is unique.

		if (strStartsWithNoCase(Machines[i], sValue))
			{
			//	If this is the first machine name match, then remember it so that we
			//	can see if there are other (conflicting) partial matches.

			if (sFoundMachineName.IsEmpty())
				sFoundMachineName = Machines[i];

			//	Otherwise, if this doesn't match a previous partial match, then we've
			//	got an ambiguous machine name.

			else if (!strEqualsNoCase(sFoundMachineName, Machines[i]))
				return false;
			}
		}

	//	If we did not find the machine, then we're done

	if (sFoundMachineName.IsEmpty())
		return false;

	//	Done

	if (retsMachineName)
		{
		if (strEquals(sFoundMachineName, GetMachineName()))
			*retsMachineName = NULL_STR;
		else
			*retsMachineName = sFoundMachineName;
		}

	return true;
	}

bool CExarchEngine::ParseModuleName (const CString &sValue, CString *retsMachineName, CString *retsModuleName) const

//	ParseModuleName
//
//	Parses a user-entered module name anywhere in the arcology. If the module
//	is found, we return TRUE. retsMachineName is the machine on which the module
//	is on (or blank if it is on this machine). retsModuleName is the module name.
//
//	The user may enter module names of the following form:
//
//	AeonDB: Returns the module on the current machine (retsMachine is blank)
//	Seldon-175efc/AeonDB: Fully qualified machine name
//	Seldon/AeonDB: Partial machine name match. Works only if the partial match
//		uniquely identifies a machine.

	{
	int i;

	//	If blank then it means CentralModule on the current machine

	if (sValue.IsEmpty())
		{
		if (retsMachineName) *retsMachineName = NULL_STR;
		if (retsModuleName) *retsModuleName = STR_CENTRAL_MODULE;
		return true;
		}

	//	Split the input

	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();
	char *pStart = pPos;
	bool bFoundMachineName = false;
	CString sMachineNameToFind;
	while (pPos < pPosEnd)
		{
		if (bFoundMachineName)
			{
			//	We don't allow slashes in a module name.

			if (*pPos == '/' || *pPos == '\\')
				return false;
			}
		else if (*pPos == '/')
			{
			sMachineNameToFind = CString(pStart, pPos - pStart);
			pPos++;
			pStart = pPos;
			bFoundMachineName = true;
			}

		pPos++;
		}

	CString sModuleNameToFind = CString(pStart, pPos - pStart);

	//	If machine name is blank, then assume the current machine

	if (sMachineNameToFind.IsEmpty())
		sMachineNameToFind = GetMachineName();

	//	Get the list of running modules. This call locks internally, so we don't
	//	need to lock additionally.

	TArray<CString> Modules;
	MnemosynthReadCollection(MNEMO_ARC_MODULES, &Modules);

	//	Make sure the module exists

	CString sFoundMachineName;
	CString sFoundModuleName;
	for (i = 0; i < Modules.GetCount(); i++)
		{
		CString sMachineName;
		CString sModuleName;
		CMnemosynthDb::ParseEndpointName(Modules[i], &sMachineName, &sModuleName);

		//	If we have a partial machine name match, see if it is unique.

		if (strStartsWithNoCase(sMachineName, sMachineNameToFind))
			{
			//	If this is the first machine name match, then remember it so that we
			//	can see if there are other (conflicting) partial matches.

			if (sFoundMachineName.IsEmpty())
				sFoundMachineName = sMachineName;

			//	Otherwise, if this doesn't match a previous partial match, then we've
			//	got an ambiguous machine name.

			else if (!strEqualsNoCase(sFoundMachineName, sMachineName))
				return false;

			//	Now see if the module name matches.

			if (strEqualsNoCase(sModuleName, sModuleNameToFind))
				{
				sFoundModuleName = sModuleName;
				}
			}
		}

	//	If we did not find the module name, then we fail.

	if (sFoundModuleName.IsEmpty())
		return false;

	//	Otherwise, we're done

	if (retsMachineName)
		{
		if (strEquals(sFoundMachineName, GetMachineName()))
			*retsMachineName = NULL_STR;
		else
			*retsMachineName = sFoundMachineName;
		}

	if (retsModuleName)
		*retsModuleName = sFoundModuleName;

	return true;
	}

SessionTypes CExarchEngine::ParseProtocol (const CString &sInput, CString *retsCommand)

//	ParseProtocol
//
//	Parses a message string and splits up the protocol header
//	from the rest of the command

	{
	//	Parse the protocol headers

	char *pPos = sInput.GetParsePointer();
	char *pStart = pPos;
	while (*pPos != ' ' && *pPos != '\0')
		pPos++;

	CString sProtocol(pStart, pPos - pStart);

	//	Return the command

	if (retsCommand)
		{
		if (*pPos != '\0')
			*retsCommand = CString(pPos + 1);
		else
			*retsCommand = NULL_STR;
		}

	//	Check the protocol

	if (strEquals(sProtocol, STR_PROTOCOL_AMP1_00))
		return stypeArcology;
	else
		return stypeUnknown;
	}

bool CExarchEngine::ReadConfig (void)

//	ReadConfig
//
//	Loads the configuration file into m_dMachineConfig

	{
	CString sError;

	//	If we don't have a name, then use the default name. If we're Arcology 
	//	Prime then we look in Config.ars. Otherwise we look in ConfigSegment.ars

	if (m_sConfigFilename.IsEmpty())
		m_sConfigFilename = (IsArcologyPrime() ? STR_CONFIG_FILENAME : STR_CONFIG_SEGMENT_FILENAME);

	//	Load the config file

	CString sConfigFilespec = fileAppend(fileGetPath(fileGetExecutableFilespec()), m_sConfigFilename);
	if (fileExists(sConfigFilespec))
		{
		if (!CDatum::CreateFromFile(sConfigFilespec, CDatum::formatAEONScript, &m_dMachineConfig, &sError))
			{
			Log(MSG_LOG_ERROR, sError);
			return false;
			}
		}

	//	We must be a structure.

	if (m_dMachineConfig.GetBasicType() != CDatum::typeStruct)
		m_dMachineConfig = CDatum(new CComplexStruct);

	return true;
	}

void CExarchEngine::RegisterForMnemosynthUpdate (void)

//	RegisterForMnemosynthUpdate
//
//	This will register for an event when all copies of Mnemosynth on the machine
//	have reached the proper sequence number.

	{
	int i;
	
	//	Construct a watermark (an array of tuples, each tuple is an
	//	endpoint name and a sequence number).

	CComplexArray *pWatermark = new CComplexArray;

	for (i = 0; i < m_MecharcologyDb.GetModuleCount(); i++)
		{
		CComplexArray *pEntry = new CComplexArray;

		//	Generate an endpoint name from a module name
		//	and insert it to the tuple

		CString sEndpoint = CMnemosynthDb::GenerateEndpointName(GetMachineName(), m_MecharcologyDb.GetModuleName(i));
		pEntry->Insert(sEndpoint);

		//	For our module (CentralModule) we get our sequence directly.
		//	For other modules, we got the sequence when they sent us on
		//	OnModuleStart (and we stored it in Mecharcology).

		if (i == 0)
			pEntry->Insert((int)GetMnemosynth().GetSequence());
		else
			pEntry->Insert((int)m_MecharcologyDb.GetModuleSeq(i));

		pWatermark->Insert(CDatum(pEntry));
		}

	//	Construct the payload

	CComplexStruct *pPayload = new CComplexStruct;
	pPayload->SetElement(STR_WATERMARK, CDatum(pWatermark));

	//	Send the message

	SendMessageCommand(ADDRESS_MNEMOSYNTH_COMMAND, MSG_MNEMOSYNTH_NOTIFY_ON_ARCOLOGY_UPDATE, ADDRESS_EXARCH_COMMAND, 0, CDatum(pPayload));
	}

bool CExarchEngine::RemoveModule (const CString &sModuleName, CString *retsError)

//	RemoveModule
//
//	Shuts down the module and does NOT restart it.

	{
	CSmartLock Lock(m_cs);

	//	Find the module

	SModuleDesc ModuleDesc;
	if (!m_MecharcologyDb.GetModule(sModuleName, &ModuleDesc))
		{
		if (retsError) *retsError = strPattern(ERR_MODULE_NOT_FOUND, sModuleName);
		return false;
		}

	//	Cannot remove central module

	if (strEquals(ModuleDesc.sName, STR_CENTRAL_MODULE))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_REMOVE_MODULE, sModuleName);
		return false;
		}

	//	If we're arcology prime, can't remove AeonDB

	if (IsArcologyPrime() 
			&& strEquals(ModuleDesc.sName, MODULE_NAME_AEON_DB))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_REMOVE_MODULE, sModuleName);
		return false;
		}

	//	Tell the mecharcology not to restart the module if it shuts down.

	m_MecharcologyDb.SetModuleRemoved(ModuleDesc.sName);

	//	Tell the module to shut down

	SendMessageCommand(CMessageTransporter::GenerateAddress(PORT_MNEMOSYNTH_COMMAND, ModuleDesc.sName),
			MSG_EXARCH_SHUTDOWN,
			NULL_STR, 
			0, 
			CDatum());

	//	Done

	return true;
	}

bool CExarchEngine::RestartModule (const CString &sFilename, CString *retsError)

//	RestartModule
//
//	Restarts the given module.

	{
	CSmartLock Lock(m_cs);

	return false;
	}

void CExarchEngine::SendReadRequest (CDatum dSocket)

//	SendReadRequests
//
//	Requests to read from a socket

	{
	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dSocket);

	SendMessageCommand(ADDRESS_ESPER_COMMAND, 
			MSG_ESPER_READ, 
			ADDRESS_EXARCH_COMMAND, 
			(DWORD)dSocket, 
			CDatum(pPayload));
	}

void CExarchEngine::SendWriteRequest (CDatum dSocket, const CString &sData)

//	SendWriteRequest
//
//	Requests to write to a socket

	{
	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dSocket);
	pPayload->Insert(sData);

	SendMessageCommand(ADDRESS_ESPER_COMMAND, 
			MSG_ESPER_WRITE, 
			ADDRESS_EXARCH_COMMAND, 
			(DWORD)dSocket, 
			CDatum(pPayload));
	}

void CExarchEngine::SetMachineData (const CString &sHostAddress, const CString &sStatus)

//	SetMachineData
//
//	Sets data about this machine to Mnemosynth

	{
	CComplexStruct *pStruct = new CComplexStruct;
	pStruct->SetElement(FIELD_HOST_ADDRESS, sHostAddress);
	pStruct->SetElement(FIELD_STATUS, sStatus);
	if (IsArcologyPrime())
		pStruct->SetElement(FIELD_NAME, STR_ARCOLOGY_PRIME);

	MnemosynthWrite(MNEMO_ARC_MACHINES, 
			GetMachineName(), 
			CDatum(pStruct));
	}

bool CExarchEngine::TestFileRead (const CString &sFilespec)

//	TestFileRead
//
//	Attempts to read the entire file and returns TRUE if successful.

	{
	//	Try opening

	CFile TestFile;
	if (!TestFile.Create(sFilespec, CFile::FLAG_OPEN_READ_ONLY))
		return false;

	//	Try reading the whole file

	try
		{
		int iLeftToRead = TestFile.GetStreamLength();
		BYTE szBuffer[65536];
		while (iLeftToRead > 0)
			{
			int iRead = Min(iLeftToRead, (int)sizeof(szBuffer));
			TestFile.Read(szBuffer, iRead);
			iLeftToRead -= iRead;
			}
		}
	catch (...)
		{
		return false;
		}

	return true;
	}

bool CExarchEngine::WriteConfig (void)

//	WriteConfig
//
//	Writes the configuration file (Config.ars)

	{
	CSmartLock Lock(m_cs);

	CString sFilespec = fileAppend(fileGetPath(fileGetExecutableFilespec()), m_sConfigFilename);

	//	Write it out

	CFile ConfigFile;
	if (!ConfigFile.Create(sFilespec, CFile::FLAG_CREATE_ALWAYS))
		{
		Log(MSG_LOG_ERROR, strPattern(ERR_CANT_WRITE_CONFIG_FILE, sFilespec));
		return false;
		}

	m_dMachineConfig.Serialize(CDatum::formatAEONScript, ConfigFile);

	return true;
	}

bool CExarchEngine::WriteStorageConfig (void)

//	WriteStorageConfig
//
//	Writes Storage.ars

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Add everything to a struct

	CComplexStruct *pConfig = new CComplexStruct;
	pConfig->SetElement(FIELD_NEXT_VOLUME, (int)m_dwNextVolume);

	//	Make a list of volumes

	CComplexArray *pVolumes = new CComplexArray;
	pConfig->SetElement(FIELD_VOLUMES, CDatum(pVolumes));

	CString sMachine = GetMachineName();
	TArray<CString> VolumeKeys;
	MnemosynthReadCollection(MNEMO_ARC_STORAGE, &VolumeKeys);
	for (i = 0; i < VolumeKeys.GetCount(); i++)
		{
		//	Only save volumes on this machine

		if (strStartsWith(VolumeKeys[i], sMachine))
			{
			CDatum dVolumeDesc = MnemosynthRead(MNEMO_ARC_STORAGE, VolumeKeys[i]);

			CComplexStruct *pVolumeDesc = new CComplexStruct;
			pVolumeDesc->SetElement(FIELD_VOLUME_NAME, dVolumeDesc.GetElement(FIELD_VOLUME_NAME));
			pVolumeDesc->SetElement(FIELD_LOCAL_PATH, dVolumeDesc.GetElement(FIELD_LOCAL_PATH));
			pVolumeDesc->SetElement(FIELD_QUOTA, dVolumeDesc.GetElement(FIELD_QUOTA));
			pVolumeDesc->SetElement(FIELD_STATUS, dVolumeDesc.GetElement(FIELD_STATUS));
			pVolumeDesc->SetElement(FIELD_TEST_VOLUME, dVolumeDesc.GetElement(FIELD_TEST_VOLUME));

			pVolumes->Insert(CDatum(pVolumeDesc));
			}
		}

	//	Add it to the main configuration

	CComplexStruct *pMachineConfig = new CComplexStruct(m_dMachineConfig);
	pMachineConfig->SetElement(FIELD_STORAGE, CDatum(pConfig));
	m_dMachineConfig = CDatum(pMachineConfig);

	//	Write it out

	return WriteConfig();
	}
