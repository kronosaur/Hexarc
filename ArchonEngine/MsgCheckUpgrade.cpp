//	MsgCheckUpgrade.cpp
//
//	CExarchEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

//#define DEBUG_CHECK_UPGRADE

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")
DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule")

DECLARE_CONST_STRING(EXT_EXE,							".exe");

DECLARE_CONST_STRING(FIELD_CHECKSUM,					"checksum");
DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath");
DECLARE_CONST_STRING(FIELD_PRODUCT_NAME,				"productName");
DECLARE_CONST_STRING(FIELD_PRODUCT_VERSION,				"productVersion");
DECLARE_CONST_STRING(FIELD_PRODUCT_VERSION_NUMBER,		"productVersionNumber");
DECLARE_CONST_STRING(FIELD_RECURSIVE,					"recursive");
DECLARE_CONST_STRING(FIELD_VERSION_INFO,				"versionInfo");

DECLARE_CONST_STRING(FILESPEC_ARC_INSTALL,				"/Arc.install/");
DECLARE_CONST_STRING(FILESPEC_UPGRADE_FOLDER,			"Upgrade");

DECLARE_CONST_STRING(MSG_AEON_FILE_DIRECTORY,			"Aeon.fileDirectory")
DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD,			"Aeon.fileDownload")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_EXARCH_RESTART_MODULE,			"Exarch.restartModule")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");
DECLARE_CONST_STRING(MSG_OK,							"OK");

DECLARE_CONST_STRING(PLATFORM_WIN10,					"Win10");

DECLARE_CONST_STRING(STR_CHECKING_FOR_UPGRADE,			"Checking for upgrades.");
DECLARE_CONST_STRING(STR_NO_UPGRADE_NEEDED,				"No upgrade required: Machine is up to date.");
DECLARE_CONST_STRING(STR_UPGRADED_FILE,					"Upgraded %s/%s.");
DECLARE_CONST_STRING(STR_UPGRADED_MODULE_FILE,			"Upgraded %s to version %s.");

DECLARE_CONST_STRING(ERR_UPGRADE_FAILED,				"Failed upgrading; please check the log for errors.");
DECLARE_CONST_STRING(ERR_INVALID_AEON_FILE_PATH,		"Invalid Aeon filePath: %s.");
DECLARE_CONST_STRING(ERR_CANT_SEND_MESSAGE,				"Unable to send message to %s.");
DECLARE_CONST_STRING(ERR_CANT_WRITE_TO_DISK,			"Unable to write to %s: %s");
DECLARE_CONST_STRING(ERR_CANT_GET_VERSION_INFO,			"Unable to read version info for %s: %s");
DECLARE_CONST_STRING(ERR_CANT_RENAME_FILE,				"Unable to rename file from %s to %s.");
DECLARE_CONST_STRING(ERR_CANT_RECOVER_UPGRADED_FILE,	"Unable to restore original file after failed upgrade: %s.");
DECLARE_CONST_STRING(ERR_CANT_MOVE_FILE,				"Unable to copy upgrade file from %s to %s.");
DECLARE_CONST_STRING(ERR_CANT_RESTART,					"Unable to restart machine.");

static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;

class CExarchCheckUpgrade : public ISessionHandler
	{
	public:
		CExarchCheckUpgrade (CExarchEngine &Engine) : 
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

			waitingForFileList,
			waitingForDownload,
			restartingModule,

			done,
			};

		struct SFileEntry
			{
			CString sModuleName;					//	File belongs to this module
			CString sFilename;						//	Filename
			CString sAeonFilespec;					//	Filespec on AeonDb
			CString sLocalFilespec;					//	Filespec on local machine
			CString sUpgradeFilespec;				//	Filespec for staged upgrade
			CString sUndoFilespec;					//	Filespec where we save file we're overwriting

			CString sAeonVersion;					//	Human-readable version on AeonDb

			bool bIsModuleExe = false;

			bool bMoved = false;
			bool bUpgraded = false;
			};

		bool CalcUpgradeList (CDatum dData, CString *retsError = NULL);
		bool DownloadFile (SFileEntry &Entry, CString *retsError = NULL);
		int FindFirstModuleIndex () const;
		int FindNextModuleIndex (int iIndex) const;
		bool IsRestartNeeded () const;
		void LogUpgrade ();
		bool MoveOriginalFiles ();
		static void ParseAeonFilePath (const CString &sAeonFilePath, CString *retsModuleName, CString *retsPlatform, CString *retsFilename);
		bool RestartMachine (CString *retsError);
		bool RestartModule (const SFileEntry &Entry, CString *retsError);
		bool StageFile (SFileEntry &Entry, CDatum dFileDownloadDesc, CString *retsError = NULL);
		void UndoUpgrade (CString *retsError);
		bool UpgradeFiles ();
		bool UpgradeMachine (CString *retsError = NULL);

		CExarchEngine &m_Engine;
		CString m_sRootFolder;
		CString m_sUpgradeFolder;
		TArray<SFileEntry> m_Files;

		State m_iState = State::unknown;
		int m_iStateIndex = -1;			//	File that we're currently processing.

	};

void CExarchEngine::MsgCheckUpgrade (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	CompleteUpgrade
//
//	Exarch.checkUpgrade
//
//	Checks to see if we need to upgrade any of our files from Arc.install. If
//	necessary, we restart modules or the machine.

	{
	StartSession(Msg, new CExarchCheckUpgrade(*this));
	}

//	CExarchCheckUpgradeSession -------------------------------------------------

bool CExarchCheckUpgrade::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Starts a new session.

	{
	CString sError;

	GetProcessCtx()->Log(MSG_LOG_INFO, STR_CHECKING_FOR_UPGRADE);

	m_sRootFolder = fileGetPath(fileGetExecutableFilespec());
	m_sUpgradeFolder = fileAppend(m_sRootFolder, FILESPEC_UPGRADE_FOLDER);
	if (!filePathCreate(m_sUpgradeFolder, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_WRITE_TO_DISK, m_sUpgradeFolder, sError));
		return false;
		}

	m_iState = State::waitingForFileList;

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(FILESPEC_ARC_INSTALL);
	dPayload.Append(CDatum());
	dPayload.Append(FIELD_RECURSIVE);

	if (!SendMessageCommand(ADDRESS_AEON_COMMAND, MSG_AEON_FILE_DIRECTORY, ADDRESS_EXARCH_COMMAND, dPayload, MESSAGE_TIMEOUT))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_SEND_MESSAGE, ADDRESS_AEON_COMMAND));
		return false;
		}

	//	Expect reply

	return true;
	}

bool CExarchCheckUpgrade::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle messages.

	{
	CString sError;

	switch (m_iState)
		{
		case State::waitingForFileList:
			{
			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, Msg.dPayload.AsString());
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload.AsString());
				return false;
				}

			//	Start by getting a list of all files from Arc.install.

			if (!CalcUpgradeList(Msg.dPayload, &sError))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
				return false;
				}

			//	If nothing to do, then no need to upgrade.

			if (m_Files.GetCount() == 0)
				{
				GetProcessCtx()->Log(MSG_LOG_INFO, STR_NO_UPGRADE_NEEDED);
				SendMessageReply(MSG_OK, CDatum());
				return false;
				}

			//	Download the files we need

			m_iState = State::waitingForDownload;
			m_iStateIndex = 0;
			if (!DownloadFile(m_Files[0], &sError))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
				return false;
				}

			return true;
			}

		case State::waitingForDownload:
			{
			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, Msg.dPayload.AsString());
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload.AsString());
				return false;
				}

			//	Stage the file in the Upgrade folder

			if (!StageFile(m_Files[m_iStateIndex], Msg.dPayload, &sError))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
				return false;
				}

			//	Download the next file.

			m_iStateIndex++;
			if (m_iStateIndex < m_Files.GetCount())
				{
				if (!DownloadFile(m_Files[m_iStateIndex], &sError))
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
					return false;
					}

				return true;
				}

			//	Now that we have all files, upgrade.

			if (!UpgradeMachine(&sError))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
				return false;
				}

			//	If we need to restart, do it now.

			if (IsRestartNeeded())
				{
				if (!RestartMachine(&sError))
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
					return false;
					}

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
				GetProcessCtx()->Log(MSG_LOG_ERROR, Msg.dPayload.AsString());
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

			//	If there are no more modules, then we're done.

			SendMessageReply(MSG_OK, CDatum());
			return false;
			}

		default:
			//	Should never happen.
			return false;
		}
	}

bool CExarchCheckUpgrade::CalcUpgradeList (CDatum dData, CString *retsError)

//	CalcUpgradeList
//
//	Initializes m_Files from the Aeon directory of /Arc.install/.

	{
	CString sError;

	for (int i = 0; i < dData.GetCount(); i++)
		{
		CDatum dFileDesc = dData.GetElement(i);
		const CString &sAeonFilePath = dFileDesc.GetElement(FIELD_FILE_PATH);

		//	Parse the file path into the appropriate components.

		CString sModuleName;
		CString sPlatform;
		CString sFilename;
		ParseAeonFilePath(sAeonFilePath, &sModuleName, &sPlatform, &sFilename);

		//	All elements are required.

		if (sModuleName.IsEmpty() || sPlatform.IsEmpty() || sFilename.IsEmpty())
			{
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_AEON_FILE_PATH, sAeonFilePath));
			continue;
			}

		//	Skip modules that aren't installed.

		if (!m_Engine.IsLocalModule(sModuleName))
			continue;

		//	Skip if this is not for our platform

		if (!strEquals(sPlatform, PLATFORM_WIN10))
			continue;

		//	Calc some values for this file.

		CString sName;
		CString sExt = fileGetExtension(fileGetFilename(sFilename), &sName);
		bool bIsModuleExe = strEqualsNoCase(sExt, EXT_EXE);
		CString sLocalFilespec;
		CString sUndoFilespec;
		if (bIsModuleExe)
			{
			sLocalFilespec = fileAppend(m_sRootFolder, sFilename);
			sUndoFilespec = fileAppend(m_sRootFolder, strPattern("Original_%s", sFilename));
			}
		else
			{
			sLocalFilespec = fileAppend(fileAppend(m_sRootFolder, sModuleName), sFilename);
			sUndoFilespec = fileAppend(fileAppend(m_sRootFolder, sModuleName), strPattern("Original_%s", sFilename));
			}

		CString sUpgradeFilespec = fileAppend(fileAppend(m_sUpgradeFolder, sModuleName), sFilename);

		//	Get the version info for the file.

		CDatum dVersionInfo = dFileDesc.GetElement(FIELD_VERSION_INFO);
		DWORDLONG dwAeonProductVersion = dVersionInfo.GetElement(FIELD_PRODUCT_VERSION_NUMBER);
		CString sAeonVersion = dVersionInfo.GetElement(FIELD_PRODUCT_VERSION);
		DWORD dwAeonChecksum = dFileDesc.GetElement(FIELD_CHECKSUM);

		//	If this is an executable, then check the version number.

		if (bIsModuleExe)
			{
			SFileVersionInfo VersionInfo;

			if (!::fileGetVersionInfo(sLocalFilespec, &VersionInfo, &sError))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_GET_VERSION_INFO, sLocalFilespec, sError));
				continue;
				}

			//	If we've already got the proper version, then we're OK.

			if (VersionInfo.dwProductVersion >= dwAeonProductVersion)
				continue;
			}

		//	Otherwise, we calculate a checksum

		else
			{
			DWORD dwChecksum = ::fileChecksumAdler32(sLocalFilespec);

			//	If checksums match, then no need to upgrade

			if (dwChecksum == dwAeonChecksum)
				continue;
			}

		//	If we get this far then we need to upgrade.

		SFileEntry *pNewEntry = m_Files.Insert();
		pNewEntry->sModuleName = sModuleName;
		pNewEntry->sFilename = sFilename;
		pNewEntry->sAeonFilespec = sAeonFilePath;
		pNewEntry->sLocalFilespec = sLocalFilespec;
		pNewEntry->sAeonVersion = sAeonVersion;
		pNewEntry->sUpgradeFilespec = sUpgradeFilespec;
		pNewEntry->sUndoFilespec = sUndoFilespec;
		pNewEntry->bIsModuleExe = bIsModuleExe;

#ifdef DEBUG_CHECK_UPGRADE
		GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Upgrade %s from %s to version %s.", sLocalFilespec, sAeonFilePath, sAeonVersion));
#endif
		}

	return true;
	}

bool CExarchCheckUpgrade::DownloadFile (SFileEntry &Entry, CString *retsError)

//	DownloadFile
//
//	Download the file.

	{
#ifdef DEBUG_CHECK_UPGRADE
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Downloading %s.", Entry.sAeonFilespec));
#endif

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(Entry.sAeonFilespec);

	if (!SendMessageCommand(ADDRESS_AEON_COMMAND, MSG_AEON_FILE_DOWNLOAD, ADDRESS_EXARCH_COMMAND, dPayload, MESSAGE_TIMEOUT))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_SEND_MESSAGE, ADDRESS_AEON_COMMAND);
		return false;
		}

	return true;
	}

int CExarchCheckUpgrade::FindFirstModuleIndex () const

//	FindFirstModuleIndex
//
//	Returns the index of the first file that represents a module.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		if (m_Files[i].bIsModuleExe)
			return i;

	return -1;
	}

int CExarchCheckUpgrade::FindNextModuleIndex (int iIndex) const

//	FindNextModuleIndex
//
//	Finds the next module index.

	{
	for (int i = iIndex + 1; i < m_Files.GetCount(); i++)
		if (m_Files[i].bIsModuleExe)
			return i;

	return -1;
	}

bool CExarchCheckUpgrade::IsRestartNeeded () const

//	IsRestartNeeded
//
//	Returns TRUE if a restart is needed.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		if (m_Files[i].bIsModuleExe && m_Engine.IsRestartRequired(m_Files[i].sFilename))
			return true;

	return false;
	}

void CExarchCheckUpgrade::LogUpgrade ()

//	LogUpgrade
//
//	Report upgrade status.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		if (m_Files[i].bIsModuleExe)
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(STR_UPGRADED_MODULE_FILE, m_Files[i].sFilename, m_Files[i].sAeonVersion));
		else
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(STR_UPGRADED_FILE, m_Files[i].sModuleName, m_Files[i].sFilename));
		}
	}

bool CExarchCheckUpgrade::MoveOriginalFiles ()

//	MoveOriginalFiles
//
//	Rename files in use so that we can upgrade in place. Returns FALSE if one or
//	more moves failed. Either way, retMoved contains the list of filenames
//	actually moved.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		CString sSrc = m_Files[i].sLocalFilespec;
		CString sDest = m_Files[i].sUndoFilespec;

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

void CExarchCheckUpgrade::ParseAeonFilePath (const CString &sAeonFilePath, CString *retsModuleName, CString *retsPlatform, CString *retsFilename)

//	ParseAeonFilePath
//
//	Parses the filePath into components:
//
//	/Arc.install/{moduleName}/{platform}/{filename}

	{
	//	Pre-initialize in case of error.

	if (retsModuleName) *retsModuleName = NULL_STR;
	if (retsPlatform) *retsPlatform = NULL_STR;
	if (retsFilename) *retsFilename = NULL_STR;

	//	Parse

	const char *pPos = sAeonFilePath.GetParsePointer();
	const char *pPosEnd = pPos + sAeonFilePath.GetLength();

	if (pPos == pPosEnd)
		return;

	//	/Arc.install/moduleName/platform/filename
	//	^
	//
	//	Skip leading /

	if (*pPos == '/')
		pPos++;

	//	Scan to next /

	while (pPos < pPosEnd && *pPos != '/')
		pPos++;

	if (pPos == pPosEnd)
		return;

	pPos++;

	//	/Arc.install/moduleName/platform/filename
	//	             ^

	const char *pStart = pPos;
	while (pPos < pPosEnd && *pPos != '/')
		pPos++;

	if (retsModuleName) *retsModuleName = CString(pStart, pPos - pStart);

	if (pPos == pPosEnd)
		return;

	pPos++;

	//	/Arc.install/moduleName/platform/filename
	//	                        ^

	pStart = pPos;
	while (pPos < pPosEnd && *pPos != '/')
		pPos++;

	if (retsPlatform) *retsPlatform = CString(pStart, pPos - pStart);

	if (pPos == pPosEnd)
		return;

	pPos++;

	//	/Arc.install/moduleName/platform/filename
	//	                                 ^

	if (retsFilename) *retsFilename = CString(pPos, pPosEnd - pPos);
	}

bool CExarchCheckUpgrade::RestartMachine (CString *retsError)

//	RestartMachine
//
//	Restart the machine.

	{
	//	Launch a process that will shutdown and restart the service.

	CProcess Restart;
	try
		{
		Restart.Create(CString("Arcology.exe /restart"));
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_CANT_RESTART;
		return false;
		}

	return true;
	}

bool CExarchCheckUpgrade::RestartModule (const SFileEntry &Entry, CString *retsError)

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

bool CExarchCheckUpgrade::StageFile (SFileEntry &Entry, CDatum dFileDownloadDesc, CString *retsError)

//	StageFile
//
//	Saves the file to the Upgrade folder.

	{
#ifdef DEBUG_CHECK_UPGRADE
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Ready to stage file %s.", Entry.sUpgradeFilespec));
#endif

	//	Make sure the directory exists

	CString sError;
	if (!::filePathCreate(fileGetPath(Entry.sUpgradeFilespec), &sError))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_WRITE_TO_DISK, Entry.sUpgradeFilespec, sError);
		return false;
		}

	//	Create the file

	CFile NewFile;
	if (!NewFile.Create(Entry.sUpgradeFilespec, CFile::FLAG_CREATE_ALWAYS, &sError))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_WRITE_TO_DISK, Entry.sUpgradeFilespec, sError);
		return false;
		}

	CDatum dData = dFileDownloadDesc.GetElement(FIELD_DATA);
	try
		{
		dData.WriteBinaryToStream(NewFile);
		}
	catch (...)
		{
		if (retsError) *retsError = strPattern(ERR_CANT_WRITE_TO_DISK, Entry.sUpgradeFilespec, sError);
		return false;
		}

	NewFile.Close();

#ifdef DEBUG_CHECK_UPGRADE
	GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Wrote %s.", Entry.sUpgradeFilespec));
#endif

	//	Success!

	return true;
	}

void CExarchCheckUpgrade::UndoUpgrade (CString *retsError)

//	UndoUpgrade
//
//	Undo the upgrade.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		if (m_Files[i].bUpgraded)
			{
			if (!fileDelete(m_Files[i].sLocalFilespec))
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, m_Files[i].sLocalFilespec));
			}

		if (m_Files[i].bMoved)
			{
			CString sSrc = m_Files[i].sUndoFilespec;
			CString sDest = m_Files[i].sLocalFilespec;

			if (!::fileMove(sSrc, sDest))
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RECOVER_UPGRADED_FILE, sDest));
			}
		}

	if (retsError) *retsError = ERR_UPGRADE_FAILED;
	}

bool CExarchCheckUpgrade::UpgradeFiles ()

//	UpgradeFiles
//
//	Upgrades all files.

	{
	for (int i = 0; i < m_Files.GetCount(); i++)
		{
		CString sSrc = m_Files[i].sUpgradeFilespec;
		CString sDest = m_Files[i].sLocalFilespec;

		if (!::fileMove(sSrc, sDest))
			{
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_MOVE_FILE, sSrc, sDest));
			return false;
			}

		m_Files[i].bUpgraded = true;
		}

	return true;
	}

bool CExarchCheckUpgrade::UpgradeMachine (CString *retsError)

//	UpgradeMachine
//
//	Upgrades the current machine with files that were uploaded.

	{
	//	Rename files that we want to replace.

	if (!MoveOriginalFiles())
		{
		UndoUpgrade(retsError);
		return false;
		}

	if (!UpgradeFiles())
		{
		UndoUpgrade(retsError);
		return false;
		}

	LogUpgrade();

	return true;
	}

