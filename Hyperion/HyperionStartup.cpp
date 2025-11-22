//	HyperionStartup.cpp
//
//	HypersionStartup
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.
//
//	START UP SEQUENCE
//
//	OnStartRunning: When the engine boots we add the built-in packages
//		to the package list. This includes the Arc.console service.
//
//	Cryptosaur.onAdminNeeded: If we get this message that it means that 
//		Cryptosaur needs an admin account to continue. Arc.console can set
//		up the admin account.
//
//	Cryptosaur.onStart: We receive this message when Cryptosaur is ready. This
//		also implies that Aeon is running (since Cryptosaur requires Aeon).
//		[LATER: We can remove this assumption by listening to the Aeon.onStart
//		message ourselves.]
//
//		At this point we load the services from /Arc.services and start them
//		all listening.

#include "stdafx.h"

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command")
DECLARE_CONST_STRING(ADDR_AEON,							"Aeon.command")

DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_AEON_FILE_DIRECTORY,			"Aeon.fileDirectory")
DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD,			"Aeon.fileDownload")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_HYPERION_REFRESH,				"Hyperion.refresh")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")
DECLARE_CONST_STRING(MSG_OK,							"OK")

DECLARE_CONST_STRING(FIELD_DATA,						"data")
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc")
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_VERSION,						"version")

DECLARE_CONST_STRING(STR_ARC_SERVICES,					"/Arc.services/")
DECLARE_CONST_STRING(STR_INCLUDE_PATTERN,				"/Arc.services/%s/%s")

DECLARE_CONST_STRING(TYPE_INCLUDE,						"$include")	//	This is a keyword in HexeDocument

DECLARE_CONST_STRING(ERR_ADMIN_REQUIRED,				"Arc.admin right required.")
DECLARE_CONST_STRING(ERR_NO_PACKAGES,					"No packages installed: %s")
DECLARE_CONST_STRING(STR_ADDED_PACKAGE,					"Package added: %s [version: %d].")
DECLARE_CONST_STRING(STR_TABLE_CREATED,					"Table %s created.")
DECLARE_CONST_STRING(ERR_INVALID_SERVICE_DESC,			"Unable to add service %s: %s")
DECLARE_CONST_STRING(ERR_CREATING_TABLE,				"Unable to create table %s: %s")
DECLARE_CONST_STRING(ERR_CANT_INIT_PROCESS,				"Unable to initialize process: %s")
DECLARE_CONST_STRING(ERR_LOADING_PACKAGE,				"Unable to load package %s: %s")
DECLARE_CONST_STRING(ERR_LOADING_FILE,					"Unable to load include file %s: %s")
DECLARE_CONST_STRING(ERR_CANT_LOAD_DOC,					"[%s]: %s")
DECLARE_CONST_STRING(ERR_INVALID_PROTOCOL_ON_PORT,		"Cannot use protocol %s on %s: Another service is using a different protocol.")

class CLoadServicesSession : public ISessionHandler
	{
	public:
		CLoadServicesSession (CHyperionEngine *pEngine) : 
				m_pEngine(pEngine),
				m_iState(stateNone),
				m_pPackageProc(NULL),
				m_pPackageDoc(NULL),
				m_iPos(-1) 
			{ }

		virtual ~CLoadServicesSession (void);

	protected:
		//	ISessionHandler virtuals
		virtual void OnMark (void);
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum States
			{
			stateNone,
			stateGetPackageDirectory,
			stateLoadPackage,
			stateLoadInclude,
			stateCreateTable,
			};

		void CleanUpTempPackageDoc (void);
		ErrorCodes LoadPackageDoc (CDatum dFileDesc, IByteStream &Stream);
		bool ReplyOK (void);
		bool RequestCreateTable (CDatum dDesc);
		bool RequestFileDownload (const CString &sFilePath);
		bool RequestNextPackage (void);
		bool RequestProcessPackages (void);

		CHyperionEngine *m_pEngine;

		States m_iState;
		TArray<CString> m_PackageFiles;
		CDatum m_dPackageFileDesc;
		CHexeProcess *m_pPackageProc;
		CHexeDocument *m_pPackageDoc;
		TArray<CString> m_IncludeFiles;
		TArray<CString> m_PackagesAdded;
		TArray<CDatum> m_TableDefs;
		int m_iPos;
		int m_iIncludePos;
	};

bool CHyperionEngine::AddListener (const CString &sProtocol, const CString &sPort, IHyperionService *pService)

//	AddListener
//
//	Adds the service as a listener on the given port.

	{
	CString sName = MakeListenerName(sPort);

	//	Look for the listener. If we don't find it,
	//	we need to add a new one.

	SListener *pListener;
	int iListener;
	if (FindListener(sName, &iListener))
		{
		pListener = &m_Listeners[iListener];

		//	Make sure we're using the same protocol. If not, we fail.

		if (!strEqualsNoCase(pListener->sProtocol, sProtocol))
			return false;
		}
	else
		{
		pListener = m_Listeners.Insert(sName);
		pListener->sName = sName;
		pListener->sProtocol = sProtocol;
		pListener->sPort = sPort;
		pListener->iStatus = statusStopped;
		pListener->iDesiredStatus = statusStopped;
		}

	//	Add the service

	pListener->Services.Insert(pService);

	//	Indicate that we need the listener

	pListener->iDesiredStatus = statusListening;

	//	Done

	return true;
	}

bool CHyperionEngine::AddListeners (IHyperionService *pService)

//	AddListeners
//
//	Adds listeners for the given service

	{
	int i;

	//	Get the list of listeners from the service.

	TArray<IHyperionService::SListenerDesc> Listeners;
	pService->GetListeners(Listeners);
	if (Listeners.GetCount() == 0)
		return true;

	//	Add the listeners

	for (i = 0; i < Listeners.GetCount(); i++)
		{
		if (!AddListener(Listeners[i].sProtocol, Listeners[i].sPort, pService))
			{
			//	If we failed to add, it is because we're asking for a different 
			//	protocol on the same port.

			Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_PROTOCOL_ON_PORT, Listeners[i].sProtocol, Listeners[i].sPort));
			continue;
			}
		}

	return true;
	}

bool CHyperionEngine::AddServicePackage (CDatum dFileDesc, CHexeProcess &Process, CHexeDocument &Doc, CString *retsName)

//	AddServicePackage
//
//	Adds a new service document. This will add the package and all its
//	definitions, but it will not start listening on any services.
//
//	NOTE: If this package upgrades an existing service, the old service will
//	still be running (kept in the deleted list) and will continue to handle
//	connections until we call CHyperionEngine::LoadServices.

	{
	CStringView sFilePath = dFileDesc.GetElement(FIELD_FILE_PATH);
	DWORD dwVersion = (DWORD)(int)dFileDesc.GetElement(FIELD_VERSION);

	CString sError;
	if (!m_Packages.AddPackage(GetProcessCtx(), dFileDesc, Process, Doc, retsName, &sError))
		{
		Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_SERVICE_DESC, sFilePath, sError));
		return false;
		}

	//	Done

	Log(MSG_LOG_INFO, strPattern(STR_ADDED_PACKAGE, *retsName, dwVersion));
	return true;
	}

void CHyperionEngine::AddServicePackage (const CString &sResName)

//	AddServicePackage
//
//	Adds a new service document from an EXE resource.

	{
	CString sError;

	if (!m_Packages.AddPackage(GetProcessCtx(), sResName, &sError))
		Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_SERVICE_DESC, sResName, sError));
	}

void CHyperionEngine::LoadServices (void)

//	LoadServices
//
//	This function gets the list of services from the package list and sets up
//	listeners as appropriate. If new services were added, listeners will be
//	started. If services were removed, listeners will be stopped.
//
//	This function may be called at any time.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Load the list of scheduled task

	TArray<CHyperionPackageList::STaskInfo> Tasks;
	m_Packages.GetScheduledTasks(GetProcessCtx(), &Tasks);
	m_Scheduler.SetTaskList(Tasks);

	//	Get the list of services from the package list

	TArray<CHyperionPackageList::SServiceInfo> Services;
	m_Packages.GetServices(&Services);

	//	In preparation for sorting services to their listeners,
	//	mark all listeners to stop. We will undo this for any
	//	listener that has a service.

	for (i = 0; i < m_Listeners.GetCount(); i++)
		{
		m_Listeners[i].iDesiredStatus = statusStopped;
		m_Listeners[i].Services.DeleteAll();
		}

	//	Deactivate all message handling services

	for (i = 0; i < m_MsgHandlers.GetCount(); i++)
		m_MsgHandlers[i].pService = NULL;

	//	Iterate over all services and sort them into listeners

	for (i = 0; i < Services.GetCount(); i++)
		{
		IHyperionService *pService = Services[i].pService;

		//	Is this a listener? If so, add the listeners for the service.

		if (pService->IsListener())
			{
			AddListeners(pService);
			}

		//	Otherwise, this is a message handler

		else
			{
			int iHandler;
			SMsgHandler *pHandler;
			if (FindHandler(Services[i].sName, &iHandler))
				pHandler = &m_MsgHandlers[iHandler];
			else
				{
				pHandler = m_MsgHandlers.Insert(Services[i].sName);
				pHandler->sName = Services[i].sName;
				}

			pHandler->pService = pService;
			}
		}

	//	Activate

	ActivateListeners();
	}

void CHyperionEngine::MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAeonOnStart
//
//	Aeon.onStart

	{
	}

void CHyperionEngine::MsgCryptosaurOnAdminNeeded (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCryptosaurOnAdminNeeded
//
//	Cryptosaur.onAdminNeeded

	{
	//	Remember that we need to create an admin account

	m_bAdminNeeded = true;

	//	Load the services so that the person installing the machine can connect
	//	to Arc.console (a built-in service).

	LoadServices();
	}

void CHyperionEngine::MsgCryptosaurOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCryptosaurOnStart
//
//	Cryptosaur.onStart

	{
	//	If we get this message that Cryptosaur is fully initialized, which means
	//	that we don't need an admin.

	m_bAdminNeeded = false;

	//	Start a session to load all services

	StartSession(Msg, new CLoadServicesSession(this));
	}

void CHyperionEngine::MsgRefresh (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRefresh
//
//	Refresh one or more packages

	{
	//	Must be an administrator-class service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	StartSession(Msg, new CLoadServicesSession(this));
	}

//	CLoadServicesSession -------------------------------------------------------

CLoadServicesSession::~CLoadServicesSession (void)

//	CLoadServicesSession destructor

	{
	CleanUpTempPackageDoc();
	}

void CLoadServicesSession::CleanUpTempPackageDoc (void)

//	CleanUpTempPackageDoc
//
//	Clean up package and process doc

	{
	if (m_pPackageProc)
		{
		delete m_pPackageProc;
		m_pPackageProc = NULL;
		}

	if (m_pPackageDoc)
		{
		delete m_pPackageDoc;
		m_pPackageDoc = NULL;
		}

	m_dPackageFileDesc = CDatum();
	}

ErrorCodes CLoadServicesSession::LoadPackageDoc (CDatum dFileDesc, IByteStream &Stream)

//	LoadPackageDoc
//
//	Loads the given package doc into m_pPackageDoc. If we're done loading all
//	package files, then we add the whole package to the engine and return
//	errNone.
//
//	If we need to load more files, we return errNotFound and initialize the
//	list of include files.

	{
	int i;

	m_dPackageFileDesc = dFileDesc;
	CStringView sFilePath = dFileDesc.GetElement(FIELD_FILE_PATH);

	//	Compute the package name. We need it here because we need to load the
	//	include files.

	CString sPackageName = CHyperionPackageList::ComputePackageName(sFilePath);

	//	Create a process into which we will load the document. This process will
	//	not persist, but its global environment will be kept by any function
	//	that we define.
	//
	//	NOTE: It is important that all documents for the same package are loaded
	//	with this same process.

	ASSERT(m_pPackageProc == NULL);
	m_pPackageProc = new CHexeProcess;

	CString sError;
	if (!m_pPackageProc->LoadStandardLibraries(&sError))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_INIT_PROCESS, sError));
		CleanUpTempPackageDoc();
		return errFail;
		}

	if (!m_pPackageProc->LoadLibrary(LIBRARY_SESSION, &sError))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_INIT_PROCESS, sError));
		CleanUpTempPackageDoc();
		return errFail;
		}

	//	Load the document

	ASSERT(m_pPackageDoc == NULL);
	m_pPackageDoc = new CHexeDocument;
	if (!m_pPackageDoc->InitFromStream(Stream, *m_pPackageProc, &sError))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_LOAD_DOC, sFilePath, sError));
		CleanUpTempPackageDoc();
		return errFail;
		}

	//	Check to see if we have any include files. If so, we initialize m_IncludeFiles and
	//	return.

	m_IncludeFiles.DeleteAll();

	int iType = m_pPackageDoc->GetTypeIndex(TYPE_INCLUDE);
	for (i = 0; i < m_pPackageDoc->GetTypeIndexCount(iType); i++)
		{
		const CString &sFilename = m_pPackageDoc->GetTypeIndexName(iType, i);
		if (!sFilename.IsEmpty())
			m_IncludeFiles.Insert(strPattern(STR_INCLUDE_PATTERN, sPackageName, sFilename));
		}

	//	If we have include files, then we return errNotFound to indicate that we 
	//	must load the include files before proceeding.

	if (m_IncludeFiles.GetCount() > 0)
		return errNotFound;

	//	Tell the engine to load the package

	if (!m_pEngine->AddServicePackage(dFileDesc, *m_pPackageProc, *m_pPackageDoc, &sPackageName))
		{
		CleanUpTempPackageDoc();
		return errFail;
		}

	//	Success!

	m_PackagesAdded.Insert(sPackageName);
	CleanUpTempPackageDoc();

	//	Done

	return errNone;
	}

void CLoadServicesSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_TableDefs.GetCount(); i++)
		m_TableDefs[i].Mark();

	m_dPackageFileDesc.Mark();
	}

bool CLoadServicesSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a reply

	{
	int i;

	switch (m_iState)
		{
		case stateGetPackageDirectory:
			{
			//	If we have an error, then it likely means that we don't have a services table.
			//	We don't load any packages

			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(ERR_NO_PACKAGES, Msg.dPayload.AsString()));
				m_pEngine->LoadServices();
				return ReplyOK();
				}

			//	Otherwise, compose an array of package files

			TArray<CHyperionPackageList::SPackageFileInfo> ServiceList;
			for (i = 0; i < Msg.dPayload.GetCount(); i++)
				{
				CHyperionPackageList::SPackageFileInfo *pInfo = ServiceList.Insert();
				pInfo->sFilePath = Msg.dPayload.GetElement(i).GetElement(FIELD_FILE_PATH).AsStringView();
				pInfo->dwVersion = (DWORD)(int)Msg.dPayload.GetElement(i).GetElement(FIELD_VERSION);
				}

			//	Get back a list of service files to load. This works by sending the list of
			//	packages to the package manager and seeing which ones are newer versions.
			//	We get back a list of packages to load.
			//
			//	NOTE: This will also delete any packages that are not in the new list.
			//	(But it will not delete built-in packages).

			m_pEngine->SetServicePackages(ServiceList, &m_PackageFiles);

			//	If no packages, nothing to do

			if (m_PackageFiles.GetCount() == 0)
				{
				m_pEngine->LoadServices();
				return ReplyOK();
				}

			//	Start at the first file

			m_iPos = 0;
			m_iState = stateLoadPackage;
			return RequestFileDownload(m_PackageFiles[m_iPos]);
			}

		case stateLoadPackage:
			{
			//	If we have an error, then we skip this package but continue with others

			if (IsError(Msg))
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_LOADING_PACKAGE, m_PackageFiles[m_iPos], Msg.dPayload.AsStringView()));

			//	Otherwise, load the package file

			else
				{
				CDatum dFileDesc = Msg.dPayload.GetElement(FIELD_FILE_DESC);
				CDatum dData = Msg.dPayload.GetElement(FIELD_DATA);

				//	Load the package document.

				CBuffer Buffer(dData.AsStringView());
				ErrorCodes iResult = LoadPackageDoc(dFileDesc, Buffer);

				//	If we succeeded, then nothing more to do, continue one to
				//	the next file.

				if (iResult == errNone)
					;

				//	If we got an error (except for errNotFound) then we already 
				//	logged it and we can continue to the next file.

				else if (iResult != errNotFound)
					;

				//	Otherwise, we need to continue loading files before we can
				//	load the document

				else
					{
					ASSERT(m_IncludeFiles.GetCount() > 0);
					m_iIncludePos = 0;
					m_iState = stateLoadInclude;
					return RequestFileDownload(m_IncludeFiles[m_iIncludePos]);
					}
				}

			//	Next file

			return RequestNextPackage();
			}

		case stateLoadInclude:
			{
			//	If we have an error, then we're done with this package

			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_LOADING_FILE, m_IncludeFiles[m_iIncludePos], Msg.dPayload.AsStringView()));
				CleanUpTempPackageDoc();
				return RequestNextPackage();
				}

			//	Load the include file.

			CDatum dFileDesc = Msg.dPayload.GetElement(FIELD_FILE_DESC);
			CDatum dData = Msg.dPayload.GetElement(FIELD_DATA);
			CBuffer Buffer(dData.AsStringView());

			CHexeDocument IncludeDoc;
			CString sError;
			if (!IncludeDoc.InitFromStream(Buffer, *m_pPackageProc, &sError))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_LOADING_FILE, m_IncludeFiles[m_iIncludePos], sError));
				CleanUpTempPackageDoc();
				return RequestNextPackage();
				}

			//	Merge the include file with the root package document.

			m_pPackageDoc->Merge(&IncludeDoc);

			//	Continue loading include files if we have more

			if (++m_iIncludePos < m_IncludeFiles.GetCount())
				return RequestFileDownload(m_IncludeFiles[m_iIncludePos]);

			//	Otherwise, we're done with the package, so load it

			CString sPackageName;
			if (!m_pEngine->AddServicePackage(m_dPackageFileDesc, *m_pPackageProc, *m_pPackageDoc, &sPackageName))
				{
				CleanUpTempPackageDoc();
				return RequestNextPackage();
				}

			//	Success! Load the next package

			m_PackagesAdded.Insert(sPackageName);
			CleanUpTempPackageDoc();
			return RequestNextPackage();
			}

		case stateCreateTable:
			{
			//	If we get Error.alreadyExists then we ignore it.

			if (strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				;

			//	If we got a different error, log it, but continue.

			else if (IsError(Msg))
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CREATING_TABLE, m_TableDefs[m_iPos].GetElement(FIELD_NAME).AsStringView(), Msg.dPayload.AsString()));

			//	Otherwise log the creation of the table

			else
				GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(STR_TABLE_CREATED, m_TableDefs[m_iPos].GetElement(FIELD_NAME).AsStringView()));

			//	Next table

			if (++m_iPos < m_TableDefs.GetCount())
				return RequestCreateTable(m_TableDefs[m_iPos]);

			//	Now that we're done creating tables, load all services.
			//
			//	NOTE: Until this method is run we will still bind to previous
			//	services (even ones that were deleted out of the package list). This
			//	is the big switch that stops accepting connections from deleted
			//	services and starts accepting connections from new services.

			m_pEngine->LoadServices();
			return ReplyOK();
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool CLoadServicesSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session

	{
	m_iState = stateGetPackageDirectory;

	//	Ask Aeon for the list of all files under /Arc.services/
	//	(All we care about is the version field for now)

	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(STR_ARC_SERVICES);
	pArray->Insert(FIELD_VERSION);

	ISessionHandler::SendMessageCommand(ADDR_AEON,
			MSG_AEON_FILE_DIRECTORY,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pArray));

	return true;
	}

bool CLoadServicesSession::ReplyOK (void)

//	ReplyOK
//
//	Done with session.

	{
	//	If this is a refresh command, reply

	if (strEquals(GetOriginalMsg().sMsg, MSG_HYPERION_REFRESH))
		SendMessageReply(MSG_OK);

	//	Done with session

	return false;
	}

bool CLoadServicesSession::RequestCreateTable (CDatum dDesc)

//	RequestCreateTable
//
//	Create a table

	{
	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(dDesc);

	ISessionHandler::SendMessageCommand(ADDR_AEON,
			MSG_AEON_CREATE_TABLE,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pArray));

	return true;
	}

bool CLoadServicesSession::RequestFileDownload (const CString &sFilePath)

//	RequestFileDownload
//
//	Request a file download

	{
	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(sFilePath);

	ISessionHandler::SendMessageCommand(ADDR_AEON,
			MSG_AEON_FILE_DOWNLOAD,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pArray));

	return true;
	}

bool CLoadServicesSession::RequestNextPackage (void)

//	RequestNextPackage
//
//	Load the next package. We assume that m_iPos is set correctly.

	{
	if (++m_iPos < m_PackageFiles.GetCount())
		{
		m_iState = stateLoadPackage;
		return RequestFileDownload(m_PackageFiles[m_iPos]);
		}

	//	If we're done loading all packages then process them.

	else
		return RequestProcessPackages();
	}

bool CLoadServicesSession::RequestProcessPackages (void)

//	RequestProcessPackages
//
//	All packages are done loading; create any tables, etc.

	{
	int i;

	//	If we're done loading all packages then process generate a list 
	//	of all Aeon tables that are defined in the new packages.

	for (i = 0; i < m_PackagesAdded.GetCount(); i++)
		m_pEngine->GetPackageTables(m_PackagesAdded[i], &m_TableDefs);

	//	If we have no tables to define, then just load the services

	if (m_TableDefs.GetCount() == 0)
		{
		m_pEngine->LoadServices();
		return ReplyOK();
		}

	//	Otherwise we request a table create

	m_iPos = 0;
	m_iState = stateCreateTable;
	return RequestCreateTable(m_TableDefs[m_iPos]);
	}
