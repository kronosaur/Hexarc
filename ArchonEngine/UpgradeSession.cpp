//	CExarchEngine.cpp
//
//	CExarchEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define DEBUG_COMPLETE_UPGRADE

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")
DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule")

DECLARE_CONST_STRING(EXT_EXE,							".exe");

DECLARE_CONST_STRING(FIELD_CHECKSUM,					"checksum");
DECLARE_CONST_STRING(FIELD_COMPANY_NAME,				"companyName");
DECLARE_CONST_STRING(FIELD_COPYRIGHT,					"copyright");
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc");
DECLARE_CONST_STRING(FIELD_FILENAME,					"filename");
DECLARE_CONST_STRING(FIELD_FILESPEC,					"filespec");
DECLARE_CONST_STRING(FIELD_PLATFORM,					"platform");
DECLARE_CONST_STRING(FIELD_PRODUCT_NAME,				"productName");
DECLARE_CONST_STRING(FIELD_PRODUCT_VERSION,				"productVersion");
DECLARE_CONST_STRING(FIELD_PRODUCT_VERSION_NUMBER,		"productVersionNumber");
DECLARE_CONST_STRING(FIELD_UPGRADE_DESC,				"upgradeDesc");
DECLARE_CONST_STRING(FIELD_UPGRADE_ID,					"upgradeID");
DECLARE_CONST_STRING(FIELD_VERSION,						"version");
DECLARE_CONST_STRING(FIELD_VERSION_INFO,				"versionInfo");

DECLARE_CONST_STRING(FILESPEC_UPGRADE_ARS,				"Upgrade.ars");
DECLARE_CONST_STRING(FILESPEC_UPGRADE_FOLDER,			"Upgrade");

DECLARE_CONST_STRING(MSG_AEON_FILE_UPLOAD,				"Aeon.fileUpload")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_EXARCH_SHUTDOWN,				"Exarch.shutdown");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");
DECLARE_CONST_STRING(MSG_OK,							"OK");
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(PLATFORM_WIN10,					"Win10");

DECLARE_CONST_STRING(PORT_MNEMOSYNTH_COMMAND,			"Mnemosynth.command");

DECLARE_CONST_STRING(STR_UNKNOWN,						"Unknown");
DECLARE_CONST_STRING(STR_UPDATED_FILE,					"Uploaded file %s %s to /Arc.install/%s/%s/%s.");

DECLARE_CONST_STRING(ERR_BAD_UPGRADE_CHECKSUM,			"Checksum for file does not match: %s.");
DECLARE_CONST_STRING(ERR_UPGRADE_FAILED,				"Failed upgrading; please check the log for errors.");
DECLARE_CONST_STRING(ERR_NO_FILES_FOUND,				"No files to upgrade.");
DECLARE_CONST_STRING(ERR_CANT_RENAME_FILE,				"Unable to rename file from %s to %s.");
DECLARE_CONST_STRING(ERR_CANT_MOVE_FILE,				"Unable to copy upgrade file from %s to %s.");
DECLARE_CONST_STRING(ERR_CANT_RECOVER_UPGRADED_FILE,	"Unable to restore original file after failed upgrade: %s.");
DECLARE_CONST_STRING(ERR_UPGRADED_FILE,					"Upgraded arcology system file: %s.");
DECLARE_CONST_STRING(ERR_CANT_RESTART,					"Unable to restart machine.");
DECLARE_CONST_STRING(ERR_CANT_LOAD_FILE,				"Unable to read file: %s.");
DECLARE_CONST_STRING(ERR_CANT_SEND_MESSAGE,				"Unable to send message to %s.");
DECLARE_CONST_STRING(ERR_NO_VERSION_INFO,				"Unable to get version info for %s: %s");

static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;
static constexpr DWORD VERSION_ALWAYS_OVERWRITE =		0xffffffff;

class CExarchUpgradeSession : public ISessionHandler
	{
	public:
		CExarchUpgradeSession (CExarchEngine &Engine) : 
				m_Engine(Engine)
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		enum class State
			{
			unknown,

			waitingForAeonSave,
			done,
			};

		struct SFileEntry
			{
			CString sModuleName;
			CString sFilename;
			CString sFilespec;
			CString sPlatform;
			DWORD dwChecksum = 0;
			SFileVersionInfo VersionInfo;
			};

		bool IsRestartNeeded () const;
		void LogUpgrade ();
		bool MoveOriginalFiles (TArray<CString> &retMoved);
		bool ReadFileList (const CString &sConfigFilespec, CString *retsError);
		bool RestartMachine ();
		bool SaveFile (const SFileEntry &Entry, CString *retsError);
		void UndoUpgrade ();
		bool UpgradeFiles (TArray<CString> &retUpgraded);
		bool UpgradeMachine ();
		bool ValidateChecksums (CString *retsError);

		CExarchEngine &m_Engine;
		CString m_sRootFolder;
		CString m_sUpgradeFolder;
		TArray<SFileEntry> m_Files;

		TArray<CString> m_FilesMoved;
		TArray<CString> m_FilesUpgraded;

		State m_iState = State::unknown;
		int m_iSavingIndex = -1;			//	File that we're currently saving.

	};

void CExarchEngine::MsgCompleteUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	CompleteUpgrade
//
//	Exarch.completeUpgrade
//
//	Upgrades the machine after files have been uploaded.

	{
	StartSession(Msg, new CExarchUpgradeSession(*this));
	}

//	CExarchUpgradeSession ------------------------------------------------------

bool CExarchUpgradeSession::IsRestartNeeded () const

//	IsRestartNeeded
//
//	Returns TRUE if a restart is needed.

	{
	for (int i = 0; i < m_FilesUpgraded.GetCount(); i++)
		if (m_Engine.IsRestartRequired(m_FilesUpgraded[i]))
			return true;

	return false;
	}

void CExarchUpgradeSession::LogUpgrade ()

//	LogUpgrade
//
//	Report upgrade status.

	{
	for (int i = 0; i < m_FilesUpgraded.GetCount(); i++)
		{
		GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(ERR_UPGRADED_FILE, m_FilesUpgraded[i]));
		}
	}

bool CExarchUpgradeSession::MoveOriginalFiles (TArray<CString> &retMoved)

//	MoveOriginalFiles
//
//	Rename files in use so that we can upgrade in place. Returns FALSE if one or
//	more moves failed. Either way, retMoved contains the list of filenames
//	actually moved.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		CString sSrc = fileAppend(m_sRootFolder, m_Files[i].sFilename);
		CString sDest = fileAppend(m_sRootFolder, strPattern("Original_%s", m_Files[i].sFilename));

		//	If the file doesn't exist it means that there is no original
		//	(We are uploading a brand new file.)

		if (!::fileExists(sSrc))
			continue;

		//	If the backup file exists then we need to delete it.

		if (::fileExists(sDest))
			::fileDelete(sDest);

		//	Rename the original file

		if (!::fileMove(sSrc, sDest))
			{
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RENAME_FILE, sSrc, sDest));
			return false;
			}

		retMoved.Insert(m_Files[i].sFilename);
		}

	return true;
	}

bool CExarchUpgradeSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process messages.

	{
	switch (m_iState)
		{
		case State::waitingForAeonSave:
			{
			CString sError;
			if (IsError(Msg))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload.AsString());
				return false;
				}

			GetProcessCtx()->Log(MSG_LOG_INFO, 
				strPattern(STR_UPDATED_FILE, 
						m_Files[m_iSavingIndex].sFilename, 
						m_Files[m_iSavingIndex].VersionInfo.sProductVersion,
						m_Files[m_iSavingIndex].sModuleName,
						m_Files[m_iSavingIndex].sPlatform,
						m_Files[m_iSavingIndex].sFilename)
				);

			m_iSavingIndex++;
			if (m_iSavingIndex < m_Files.GetCount())
				{
				if (!SaveFile(m_Files[m_iSavingIndex], &sError))
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
					return false;
					}

				return true;
				}
			else
				{
				UpgradeMachine();

				SendMessageReply(MSG_OK, CDatum());
				return false;
				}
			break;
			}

		default:
			return false;
		}
	}

bool CExarchUpgradeSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session.

	{
	CString sError;

	m_sRootFolder = fileGetPath(fileGetExecutableFilespec());
	m_sUpgradeFolder = fileAppend(m_sRootFolder, FILESPEC_UPGRADE_FOLDER);

#ifdef DEBUG_COMPLETE_UPGRADE
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Starting upgrade..."));
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Upgrade folder: %s", m_sUpgradeFolder));
#endif

	//	Load the list of files to upgrade

	CString sUpgradeConfig = fileAppend(m_sUpgradeFolder, FILESPEC_UPGRADE_ARS);
	if (!ReadFileList(sUpgradeConfig, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
		return false;
		}

#ifdef DEBUG_COMPLETE_UPGRADE
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("%s: %s at %s", m_Files[i].sModuleName, m_Files[i].sFilename, m_Files[i].sFilespec));
		}
#endif

	//	Upload all the files to AeonDB.

	m_iState = State::waitingForAeonSave;
	m_iSavingIndex = 0;
	if (!SaveFile(m_Files[0], &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
		return false;
		}

	return true;
	}

bool CExarchUpgradeSession::ReadFileList (const CString &sConfigFilespec, CString *retsError)

//	ReadFileList
//
//	Reads the config file which contains the list of all files that we want to
//	upgrade.

	{
	//	Load the config file

	CDatum dConfig;
	if (!CDatum::CreateFromFile(sConfigFilespec, CDatum::formatAEONScript, &dConfig, retsError))
		return false;

	CDatum dUpgradeDesc = dConfig.GetElement(FIELD_UPGRADE_DESC);

	//	Loop over all files to upgrade and put them in our structure.

	for (int i = 0; i < dUpgradeDesc.GetCount(); i++)
		{
		CDatum dFileDesc = dUpgradeDesc.GetElement(i);
		SFileEntry *pNewEntry = m_Files.Insert();

		pNewEntry->sFilename = dFileDesc.GetElement(FIELD_FILENAME);
		pNewEntry->sFilespec = fileAppend(m_sUpgradeFolder, pNewEntry->sFilename);
		pNewEntry->dwChecksum = (DWORD)(int)dFileDesc.GetElement(FIELD_CHECKSUM);

		CString sName;
		CString sExt = fileGetExtension(fileGetFilename(pNewEntry->sFilename), &sName);
		if (strEqualsNoCase(sExt, EXT_EXE))
			pNewEntry->sModuleName = sName;

		if (::fileChecksumAdler32(pNewEntry->sFilespec) != pNewEntry->dwChecksum)
			{
			if (retsError) *retsError = strPattern(ERR_BAD_UPGRADE_CHECKSUM, pNewEntry->sFilespec);
			return false;
			}

		//	NOTE: If we don't calculate the checksum here, the call to get 
		//	version info fails. I believe this is a race condition with some
		//	antivirus software. The file is opened by the antivirus software
		//	just after we're done adding it, and interferes with the call to get
		//	version history. Adding a delay also solves the problem.

		CString sError;
		if (!::fileGetVersionInfo(pNewEntry->sFilespec, &pNewEntry->VersionInfo, &sError))
			{
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(ERR_NO_VERSION_INFO, pNewEntry->sFilespec, sError));

			pNewEntry->VersionInfo.sCompanyName = STR_UNKNOWN;
			pNewEntry->VersionInfo.sProductName = pNewEntry->sModuleName;
			pNewEntry->VersionInfo.sProductVersion = STR_UNKNOWN;

			pNewEntry->VersionInfo.dwFileVersion = 0;
			pNewEntry->VersionInfo.dwProductVersion = 0;
			}

		pNewEntry->sPlatform = dFileDesc.GetElement(FIELD_PLATFORM);
		if (pNewEntry->sPlatform.IsEmpty())
			pNewEntry->sPlatform = PLATFORM_WIN10;
		}

	if (m_Files.GetCount() == 0)
		{
		if (retsError) *retsError = ERR_NO_FILES_FOUND;
		return false;
		}

	return true;
	}

bool CExarchUpgradeSession::RestartMachine ()

//	RestartMachine
//
//	Restart the machine.

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
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_CANT_RESTART);
		return false;
		}

	return true;
	}

bool CExarchUpgradeSession::SaveFile (const SFileEntry &Entry, CString *retsError)

//	SaveFile
//
//	Sends a message to save the given file to AeonDb.

	{
	CString sError;

	if (Entry.sModuleName.IsEmpty() || Entry.sFilename.IsEmpty() || Entry.sFilespec.IsEmpty())
		return false;

#ifdef DEBUG_COMPLETE_UPGRADE
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Saving %s to Aeon", Entry.sFilename));
#endif

	//	Read the file into a CDatum binary object

	CDatum dData;
	if (!CDatum::CreateFromFile(Entry.sFilespec, CDatum::formatBinary, &dData, &sError))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_LOAD_FILE, Entry.sFilespec);
		return false;
		}

	CDatum dVersionInfo(CDatum::typeStruct);
	dVersionInfo.SetElement(FIELD_COMPANY_NAME, Entry.VersionInfo.sCompanyName);
	dVersionInfo.SetElement(FIELD_COPYRIGHT, Entry.VersionInfo.sCopyright);
	dVersionInfo.SetElement(FIELD_PRODUCT_NAME, Entry.VersionInfo.sProductName);
	dVersionInfo.SetElement(FIELD_PRODUCT_VERSION, Entry.VersionInfo.sProductVersion);
	dVersionInfo.SetElement(FIELD_PRODUCT_VERSION_NUMBER, Entry.VersionInfo.dwProductVersion);

	//	Upload to AEON

	CString sFilePath = strPattern("/Arc.install/%s/%s/%s", Entry.sModuleName, Entry.sPlatform, Entry.sFilename);
	CDatum dFileDesc(CDatum::typeStruct);
	dFileDesc.SetElement(FIELD_VERSION_INFO, dVersionInfo);

	CDatum dFileUploadDesc(CDatum::typeStruct);
	dFileUploadDesc.SetElement(FIELD_FILE_DESC, dFileDesc);
	dFileUploadDesc.SetElement(FIELD_VERSION, VERSION_ALWAYS_OVERWRITE);

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(sFilePath);
	dPayload.Append(dFileUploadDesc);
	dPayload.Append(dData);

	if (!SendMessageCommand(ADDRESS_AEON_COMMAND, MSG_AEON_FILE_UPLOAD, ADDRESS_EXARCH_COMMAND, dPayload, MESSAGE_TIMEOUT))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_SEND_MESSAGE, ADDRESS_AEON_COMMAND);
		return false;
		}

#ifdef DEBUG_COMPLETE_UPGRADE
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Uploading %s to %s", Entry.sFilespec, sFilePath));
#endif

	return true;
	}

void CExarchUpgradeSession::UndoUpgrade ()

//	UndoUpgrade
//
//	Undo the upgrade.

	{
	for (int i = 0; i < m_FilesUpgraded.GetCount(); i++)
		{
		CString sSrc = ::fileAppend(m_sRootFolder, m_FilesUpgraded[i]);
		if (!fileDelete(sSrc))
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, sSrc));
		}

	for (int i = 0; i < m_FilesMoved.GetCount(); i++)
		{
		CString sSrc = fileAppend(m_sRootFolder, strPattern("Original_%s", m_FilesMoved[i]));
		CString sDest = fileAppend(m_sRootFolder, m_FilesMoved[i]);

		if (!::fileMove(sSrc, sDest))
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, sDest));
		}

	SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_UPGRADE_FAILED);
	}

bool CExarchUpgradeSession::UpgradeFiles (TArray<CString> &retUpgraded)

//	UpgradeFiles
//
//	Upgrades all files.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		CString sSrc = fileAppend(m_sUpgradeFolder, m_Files[i].sFilename);
		CString sDest = fileAppend(m_sRootFolder, m_Files[i].sFilename);

		if (!::fileMove(sSrc, sDest))
			{
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_MOVE_FILE, sSrc, sDest));
			return false;
			}

		retUpgraded.Insert(m_Files[i].sFilename);
		}

	return true;
	}

bool CExarchUpgradeSession::UpgradeMachine ()

//	UpgradeMachine
//
//	Upgrades the current machine with files that were uploaded.

	{
#ifdef DEBUG_COMPLETE_UPGRADE
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Upgrading machine..."));
#endif

	//	Rename files that we want to replace.

	if (!MoveOriginalFiles(m_FilesMoved))
		{
		UndoUpgrade();
		return false;
		}

	if (!UpgradeFiles(m_FilesUpgraded))
		{
		UndoUpgrade();
		return false;
		}

	LogUpgrade();

	if (IsRestartNeeded())
		RestartMachine();
	else
		m_Engine.RestartModules(m_FilesUpgraded);

	return true;
	}

bool CExarchUpgradeSession::ValidateChecksums (CString *retsError)

//	ValidateChecksums
//
//	Loops over all files and makes sure their checksums match.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		if (fileChecksumAdler32(m_Files[i].sFilespec) != m_Files[i].dwChecksum)
			{
			if (retsError)
				*retsError = strPattern(ERR_BAD_UPGRADE_CHECKSUM, m_Files[i].sFilename);

			return false;
			}
		}

	return true;
	}
