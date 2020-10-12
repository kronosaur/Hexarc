//	CExarchEngine.cpp
//
//	CExarchEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

//#define DEBUG_COMPLETE_UPGRADE

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

DECLARE_CONST_STRING(FILENAME_ARCOLOGY,					"Arcology.exe");
DECLARE_CONST_STRING(FILESPEC_UPGRADE_ARS,				"Upgrade.ars");
DECLARE_CONST_STRING(FILESPEC_UPGRADE_FOLDER,			"Upgrade");

DECLARE_CONST_STRING(MODULE_CENTRAL_MODULE,				"CentralModule")

DECLARE_CONST_STRING(MSG_AEON_FILE_UPLOAD,				"Aeon.fileUpload")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_EXARCH_CHECK_UPGRADE,			"Exarch.checkUpgrade")
DECLARE_CONST_STRING(MSG_EXARCH_RESTART_MODULE,			"Exarch.restartModule")
DECLARE_CONST_STRING(MSG_EXARCH_SHUTDOWN,				"Exarch.shutdown");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");
DECLARE_CONST_STRING(MSG_OK,							"OK");
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(PLATFORM_WIN10,					"Win10");

DECLARE_CONST_STRING(PORT_EXARCH_COMMAND,				"Exarch.command")
DECLARE_CONST_STRING(PORT_MNEMOSYNTH_COMMAND,			"Mnemosynth.command");

DECLARE_CONST_STRING(STR_UNKNOWN,						"Unknown");
DECLARE_CONST_STRING(STR_UPDATED_FILE,					"Uploaded file %s %s to /Arc.install/%s/%s/%s.");
DECLARE_CONST_STRING(STR_UPGRADED_FILE,					"Upgraded %s/%s.");
DECLARE_CONST_STRING(STR_UPGRADED_MODULE_FILE,			"Upgraded %s to version %s.");
DECLARE_CONST_STRING(STR_REQUEST_UPGRADE,				"Requesting %s to check for upgrades.");

DECLARE_CONST_STRING(ERR_BAD_UPGRADE_CHECKSUM,			"Checksum for file does not match: %s.");
DECLARE_CONST_STRING(ERR_UPGRADE_FAILED,				"Failed upgrading; please check the log for errors.");
DECLARE_CONST_STRING(ERR_NO_FILES_FOUND,				"No files to upgrade.");
DECLARE_CONST_STRING(ERR_CANT_RENAME_FILE,				"Unable to rename file from %s to %s.");
DECLARE_CONST_STRING(ERR_CANT_MOVE_FILE,				"Unable to copy upgrade file from %s to %s.");
DECLARE_CONST_STRING(ERR_CANT_RECOVER_UPGRADED_FILE,	"Unable to restore original file after failed upgrade: %s.");
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
			restartingModule,
			done,
			};

		struct SFileEntry
			{
			CString sModuleName;
			CString sFilename;
			CString sFilespec;
			CString sPlatform;
			DWORD dwChecksum = 0;

			bool bIsModuleExe = false;
			SFileVersionInfo VersionInfo;

			bool bMoved = false;
			bool bUpgraded = false;
			};

		int FindFirstModuleIndex () const;
		int FindNextModuleIndex (int iIndex) const;
		bool IsRestartNeeded () const;
		void LogUpgrade ();
		bool MoveOriginalFiles ();
		bool ReadFileList (const CString &sConfigFilespec, CString *retsError);
		bool RestartMachine ();
		bool RestartModule (const SFileEntry &Entry, CString *retsError);
		bool SaveFile (const SFileEntry &Entry, CString *retsError);
		void UndoUpgrade ();
		bool UpgradeFiles ();
		bool UpgradeMachine ();
		bool UpgradeOtherMachines (CString *retsError);
		bool ValidateChecksums (CString *retsError);

		CExarchEngine &m_Engine;
		CString m_sRootFolder;
		CString m_sUpgradeFolder;
		TArray<SFileEntry> m_Files;

		State m_iState = State::unknown;
		int m_iStateIndex = -1;			//	File that we're currently processing.

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

int CExarchUpgradeSession::FindFirstModuleIndex () const

//	FindFirstModuleIndex
//
//	Returns the index of the first file that represents a module.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		if (m_Files[i].bIsModuleExe && m_Engine.IsLocalModule(m_Files[i].sModuleName))
			return i;

	return -1;
	}

int CExarchUpgradeSession::FindNextModuleIndex (int iIndex) const

//	FindNextModuleIndex
//
//	Finds the next module index.

	{
	for (int i = iIndex + 1; i < m_Files.GetCount(); i++)
		if (m_Files[i].bIsModuleExe && m_Engine.IsLocalModule(m_Files[i].sModuleName))
			return i;

	return -1;
	}

bool CExarchUpgradeSession::IsRestartNeeded () const

//	IsRestartNeeded
//
//	Returns TRUE if a restart is needed.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		if (m_Files[i].bIsModuleExe 
				&& m_Engine.IsLocalModule(m_Files[i].sModuleName)
				&& m_Engine.IsRestartRequired(m_Files[i].sFilename))
			return true;

	return false;
	}

void CExarchUpgradeSession::LogUpgrade ()

//	LogUpgrade
//
//	Report upgrade status.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		if (m_Files[i].bIsModuleExe)
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(STR_UPGRADED_MODULE_FILE, m_Files[i].sFilename, m_Files[i].VersionInfo.sProductVersion));
		else
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(STR_UPGRADED_FILE, m_Files[i].sModuleName, m_Files[i].sFilename));
		}
	}

bool CExarchUpgradeSession::MoveOriginalFiles ()

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

		m_Files[i].bMoved = true;
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
						m_Files[m_iStateIndex].sFilename, 
						m_Files[m_iStateIndex].VersionInfo.sProductVersion,
						m_Files[m_iStateIndex].sModuleName,
						m_Files[m_iStateIndex].sPlatform,
						m_Files[m_iStateIndex].sFilename)
				);

			//	If we've got more files to save, do it.

			m_iStateIndex++;
			if (m_iStateIndex < m_Files.GetCount())
				{
				if (!SaveFile(m_Files[m_iStateIndex], &sError))
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
					return false;
					}

				return true;
				}

			//	Otherwise, upgrade machine

			if (!UpgradeMachine())
				return false;

			//	If we need to restart, do it now.

			if (IsRestartNeeded())
				{
				if (!RestartMachine())
					return false;

				SendMessageReply(MSG_OK, CDatum());
				return false;
				}

			//	Otherwise, we restart all modules.

			m_iStateIndex = FindFirstModuleIndex();
			if (m_iStateIndex != -1)
				{
				m_iState = State::restartingModule;

				if (!RestartModule(m_Files[m_iStateIndex], &sError))
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
					return false;
					}

				return true;
				}

			//	We only get here if we've updated files without updating
			//	modules. This should never happen (callers should always 
			//	update the module EXE when updating files).
			//
			//	LATER: In the future we can be smarter about support files.
			//	We can restart a module if a support file changes, even if
			//	the module EXE has not.

			SendMessageReply(MSG_OK, CDatum());
			return false;
			}

		case State::restartingModule:
			{
			CString sError;

			if (IsError(Msg))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload.AsString());
				return false;
				}

			//	Restart the next module in our list

			m_iStateIndex = FindNextModuleIndex(m_iStateIndex);
			if (m_iStateIndex != -1)
				{
				if (!RestartModule(m_Files[m_iStateIndex], &sError))
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
					return false;
					}

				return true;
				}

			//	If there are no more modules to restart, then send messages to
			//	upgrade all machines.

			if (m_Engine.IsArcologyPrime())
				{
				if (!UpgradeOtherMachines(&sError))
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
					return false;
					}

				//	Fall through because we don't wait for replies.
				}

			//	Either way, we're done.

			SendMessageReply(MSG_OK, CDatum());
			return false;
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
	m_iStateIndex = 0;
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
			{
			if (strEqualsNoCase(pNewEntry->sFilename, FILENAME_ARCOLOGY))
				pNewEntry->sModuleName = MODULE_CENTRAL_MODULE;
			else
				pNewEntry->sModuleName = sName;
			pNewEntry->bIsModuleExe = true;
			}

		if (::fileChecksumAdler32(pNewEntry->sFilespec) != pNewEntry->dwChecksum)
			{
			if (retsError) *retsError = strPattern(ERR_BAD_UPGRADE_CHECKSUM, pNewEntry->sFilespec);
			return false;
			}

		//	NOTE: If we don't calculate the checksum above, the call to get 
		//	version info fails. I believe this is a race condition with some
		//	antivirus software. The file is opened by the antivirus software
		//	just after we're done adding it, and interferes with the call to get
		//	version history. Adding a delay also solves the problem.

		if (pNewEntry->bIsModuleExe)
			{
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

bool CExarchUpgradeSession::RestartModule (const SFileEntry &Entry, CString *retsError)

//	RestartModule
//
//	Restart the given module.

	{
	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(Entry.sModuleName);

	if (!SendMessageCommand(ADDRESS_EXARCH_COMMAND, MSG_EXARCH_RESTART_MODULE, ADDRESS_EXARCH_COMMAND, dPayload, MESSAGE_TIMEOUT))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_SEND_MESSAGE, ADDRESS_EXARCH_COMMAND);
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

	//	Upload to AEON

	CString sFilePath = strPattern("/Arc.install/%s/%s/%s", Entry.sModuleName, Entry.sPlatform, Entry.sFilename);
	CDatum dFileDesc(CDatum::typeStruct);
	dFileDesc.SetElement(FIELD_CHECKSUM, Entry.dwChecksum);

	if (Entry.VersionInfo.dwProductVersion)
		{
		CDatum dVersionInfo(CDatum::typeStruct);
		dVersionInfo.SetElement(FIELD_COMPANY_NAME, Entry.VersionInfo.sCompanyName);
		dVersionInfo.SetElement(FIELD_COPYRIGHT, Entry.VersionInfo.sCopyright);
		dVersionInfo.SetElement(FIELD_PRODUCT_NAME, Entry.VersionInfo.sProductName);
		dVersionInfo.SetElement(FIELD_PRODUCT_VERSION, Entry.VersionInfo.sProductVersion);
		dVersionInfo.SetElement(FIELD_PRODUCT_VERSION_NUMBER, Entry.VersionInfo.dwProductVersion);

		dFileDesc.SetElement(FIELD_VERSION_INFO, dVersionInfo);
		}

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
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		if (m_Files[i].bUpgraded)
			{
			CString sSrc = ::fileAppend(m_sRootFolder, m_Files[i].sFilename);
			if (!fileDelete(sSrc))
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, sSrc));
			}

		if (m_Files[i].bMoved)
			{
			CString sSrc = fileAppend(m_sRootFolder, strPattern("Original_%s", m_Files[i].sFilename));
			CString sDest = fileAppend(m_sRootFolder, m_Files[i].sFilename);

			if (!::fileMove(sSrc, sDest))
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, sDest));
			}
		}

	SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_UPGRADE_FAILED);
	}

bool CExarchUpgradeSession::UpgradeFiles ()

//	UpgradeFiles
//
//	Upgrades all files.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		//	Skip modules that aren't installed.

		if (!m_Engine.IsLocalModule(m_Files[i].sModuleName))
			continue;

		//	Upgrade

		CString sSrc = fileAppend(m_sUpgradeFolder, m_Files[i].sFilename);
		CString sDest = fileAppend(m_sRootFolder, m_Files[i].sFilename);

		if (!::fileMove(sSrc, sDest))
			{
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_MOVE_FILE, sSrc, sDest));
			return false;
			}

		m_Files[i].bUpgraded = true;
		}

	return true;
	}

bool CExarchUpgradeSession::UpgradeOtherMachines (CString *retsError)

//	UpgradeOtherMachines
//
//	Send messages to all other machines to upgrade.

	{
	TArray<CString> Addresses = GetProcessCtx()->GetTransporter().GetArcologyPortAddresses(PORT_EXARCH_COMMAND, CMessageTransporter::FLAG_EXCLUDE_LOCAL_MACHINE);

	for (int i = 0; i < Addresses.GetCount(); i++)
		{
		GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(STR_REQUEST_UPGRADE, Addresses[i]));
		SendMessageCommand(Addresses[i], MSG_EXARCH_CHECK_UPGRADE, NULL_STR, CDatum(), MESSAGE_TIMEOUT);
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

	if (!MoveOriginalFiles())
		{
		UndoUpgrade();
		return false;
		}

	if (!UpgradeFiles())
		{
		UndoUpgrade();
		return false;
		}

	LogUpgrade();

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
