//	CHyperionPackageList.cpp
//
//	CHyperionPackageList class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_ARC,							"Arc")
DECLARE_CONST_STRING(STR_PACKAGE,						"package")

DECLARE_CONST_STRING(FIELD_CODE,						"code")
DECLARE_CONST_STRING(FIELD_COMMANDS,					"commands")
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath")
DECLARE_CONST_STRING(FIELD_HELP,						"help")
DECLARE_CONST_STRING(FIELD_MODIFIED_BY,					"modifiedBy")
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_PARAMETERS,					"parameters")
DECLARE_CONST_STRING(FIELD_RIGHTS,						"rights")
DECLARE_CONST_STRING(FIELD_SCHEDULE,					"schedule")
DECLARE_CONST_STRING(FIELD_VERSION,						"version")

DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

DECLARE_CONST_STRING(NAMESPACE_ARC,						"Arc.")

DECLARE_CONST_STRING(OPTION_OPTIONAL,					"optional")

DECLARE_CONST_STRING(RESTYPE_PACKAGE,					"Arc.package")

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin")
DECLARE_CONST_STRING(RIGHT_ARC_DEVELOPER,				"Arc.developer")

DECLARE_CONST_STRING(SCHEDULE_DAILY,					"daily")
DECLARE_CONST_STRING(SCHEDULE_HOURLY,					"hourly")

DECLARE_CONST_STRING(TYPE_ARC_PACKAGE,					"Arc.package")
DECLARE_CONST_STRING(TYPE_ARC_SERVICE,					"Arc.service")
DECLARE_CONST_STRING(TYPE_ARC_TABLE,					"Arc.table")
DECLARE_CONST_STRING(TYPE_FUNCTION,						"function")
DECLARE_CONST_STRING(TYPE_INCLUDE_MESSAGES,				"$includeMessages")

DECLARE_CONST_STRING(ERR_MISSING_PACKAGE_DEF,			"Cannot find Arc.package definition in package: %s.")
DECLARE_CONST_STRING(ERR_CANT_FIND_RESOURCE,			"Cannot find ArcService resource: %s")
DECLARE_CONST_STRING(ERR_ARC_PACKAGE_NAME_RESERVED,		"Cannot use reserved package name: %s.")
DECLARE_CONST_STRING(ERR_ARC_RESERVED,					"Cannot use reserved service name: %s.")
DECLARE_CONST_STRING(ERR_DUPLICATE_NAME,				"Duplicate package name: %s.")
DECLARE_CONST_STRING(ERR_INVALID_MESSAGE_INCLUDE,		"Invalid include message address: %s.")
DECLARE_CONST_STRING(ERR_NO_SERVICES_IN_PACKAGE,		"No services found in package: %s.")
DECLARE_CONST_STRING(ERR_COMMAND_SET_OUT_OF_SANDBOX,	"Package %s may not define command set outside its namespace: %s.")
DECLARE_CONST_STRING(ERR_SERVICE_OUT_OF_SANDBOX,		"Package %s may not define service outside its namespace: %s.")
DECLARE_CONST_STRING(ERR_TABLE_OUT_OF_SANDBOX,			"Package %s may not define table outside its namespace: %s.")
DECLARE_CONST_STRING(ERR_MULTIPLE_PACKAGE_DEF,			"Too many Arc.package definitions in package: %s.")
DECLARE_CONST_STRING(ERR_INVALID_SCHEDULE,				"Invalid schedule: %s.")

CHyperionPackageList::CHyperionPackageList (void)

//	CHyperionPackageList constructor

	{
	}

CHyperionPackageList::~CHyperionPackageList (void)

//	CHyperionPackageList destructor

	{
	int i;

	//	Clean up all services

	for (i = 0; i < m_Packages.GetCount(); i++)
		CleanUpPackage(&m_Packages[i]);

	//	Delete for good

	CollectGarbage();
	}

bool CHyperionPackageList::AddPackage (IArchonProcessCtx *pProcess, CDatum dFileDesc, CHexeProcess &Process, CHexeDocument &Doc, CString *retsName, CString *retsError)

//	AddPackage
//
//	Adds a new package

	{
	CSmartLock Lock(m_cs);

	const CString &sFilePath = dFileDesc.GetElement(FIELD_FILE_PATH);
	DWORD dwVersion = (DWORD)(int)dFileDesc.GetElement(FIELD_VERSION);

	int iIndex;
	SPackage *pPackage;
	if (FindPackage(sFilePath, &iIndex))
		{
		//	If we already have the package, add it only if it is
		//	a better version

		if (dwVersion <= m_Packages[iIndex].dwVersion)
			{
			*retsName = m_Packages[iIndex].sName;
			return true;
			}

		//	Prepare for services

		pPackage = &m_Packages[iIndex];
		CleanUpPackage(&m_Packages[iIndex]);
		}
	
	//	Add it

	else
		{
		//	Make sure the package name does not conflict

		CString sName = ComputePackageName(sFilePath);
		if (FindPackageByName(sName))
			{
			*retsError = strPattern(ERR_DUPLICATE_NAME, sName);
			return false;
			}

		//	Some names are reserved

		if (strEquals(sName, STR_ARC))
			{
			*retsError = strPattern(ERR_ARC_PACKAGE_NAME_RESERVED, sName);
			return false;
			}

		pPackage = m_Packages.Insert();
		pPackage->sName = sName;
		pPackage->sFilePath = sFilePath;
		pPackage->bBuiltIn = false;
		}

	//	Update info

	pPackage->dwVersion = dwVersion;
	pPackage->sModifiedBy = dFileDesc.GetElement(FIELD_MODIFIED_BY);
	pPackage->ModifiedOn = dFileDesc.GetElement(FIELD_MODIFIED_ON);

	//	return the name

	*retsName = pPackage->sName;

	//	Parse the service into a document

	return LoadPackageDoc(pProcess, pPackage, Process, Doc, retsError);
	}

bool CHyperionPackageList::AddPackage (IArchonProcessCtx *pProcess, CDatum dFileDesc, IByteStream &Stream, CString *retsName, CString *retsError)

//	AddPackage
//
//	Adds a new package

	{
	CSmartLock Lock(m_cs);

	const CString &sFilePath = dFileDesc.GetElement(FIELD_FILE_PATH);
	DWORD dwVersion = (DWORD)(int)dFileDesc.GetElement(FIELD_VERSION);

	int iIndex;
	SPackage *pPackage;
	if (FindPackage(sFilePath, &iIndex))
		{
		//	If we already have the package, add it only if it is
		//	a better version

		if (dwVersion <= m_Packages[iIndex].dwVersion)
			{
			*retsName = m_Packages[iIndex].sName;
			return true;
			}

		//	Prepare for services

		pPackage = &m_Packages[iIndex];
		CleanUpPackage(&m_Packages[iIndex]);
		}
	
	//	Add it

	else
		{
		//	Make sure the package name does not conflict

		CString sName = ComputePackageName(sFilePath);
		if (FindPackageByName(sName))
			{
			*retsError = strPattern(ERR_DUPLICATE_NAME, sName);
			return false;
			}

		//	Some names are reserved

		if (strEquals(sName, STR_ARC))
			{
			*retsError = strPattern(ERR_ARC_PACKAGE_NAME_RESERVED, sName);
			return false;
			}

		pPackage = m_Packages.Insert();
		pPackage->sName = sName;
		pPackage->sFilePath = sFilePath;
		pPackage->bBuiltIn = false;
		}

	//	Update info

	pPackage->dwVersion = dwVersion;
	pPackage->sModifiedBy = dFileDesc.GetElement(FIELD_MODIFIED_BY);
	pPackage->ModifiedOn = dFileDesc.GetElement(FIELD_MODIFIED_ON);

	//	return the name

	*retsName = pPackage->sName;

	//	Parse the service into a document

	return LoadPackageDoc(pProcess, pPackage, Stream, retsError);
	}

bool CHyperionPackageList::AddPackage (IArchonProcessCtx *pProcess, const CString &sResName, CString *retsError)

//	AddPackage
//
//	Adds a new package from an EXE resource

	{
	CSmartLock Lock(m_cs);

	//	Load the resource

	CResource Res;
	if (!Res.Open(RESTYPE_PACKAGE, sResName))
		{
		*retsError = strPattern(ERR_CANT_FIND_RESOURCE, sResName);
		return false;
		}

	//	Add it

	SPackage *pPackage = m_Packages.Insert();
	pPackage->sName = ComputePackageName(sResName);
	pPackage->sFilePath = sResName;
	pPackage->dwVersion = 1;
	pPackage->bBuiltIn = true;

	//	Parse the service into a document

	return LoadPackageDoc(pProcess, pPackage, Res, retsError);
	}

void CHyperionPackageList::CleanUpPackage (SPackage *pEntry)

//	CleanUpPackage
//
//	Cleans up a package

	{
	int i;

	for (i = 0; i < pEntry->Services.GetCount(); i++)
		m_Deleted.Insert(pEntry->Services[i].pService);

	pEntry->Services.DeleteAll();

	pEntry->CommandSet.DeleteAll();
	pEntry->Tables.DeleteAll();
	}

void CHyperionPackageList::CollectGarbage (void)

//	CollectGarbage
//
//	Must be called when the world is stopped

	{
	int i;

	for (i = 0; i < m_Deleted.GetCount(); i++)
		if (m_Deleted[i]->GetSessionCount() == 0)
			{
			delete m_Deleted[i];
			m_Deleted.Delete(i);
			i--;
			}
	}

CString CHyperionPackageList::ComputePackageName (const CString &sFilePath)

//	ComputePackageName
//
//	Generates a package name from a filepath

	{
	CString sPackageName;

	//	Get the name after the last slash

	char *pPos = sFilePath.GetParsePointer();
	char *pEndPos = pPos + sFilePath.GetLength();

	char *pStart = pPos;
	while (pPos < pEndPos)
		{
		if (*pPos == '/')
			pStart = pPos + 1;

		pPos++;
		}

	//	Can't end in a slash

	if (pStart == pEndPos)
		return sFilePath;

	//	If we have an underscore or a dot, then stop there

	pPos = pStart;
	while (pPos < pEndPos && *pPos != '_' && *pPos != '.')
		pPos++;

	if (pPos > pStart)
		sPackageName = CString(pStart, pPos - pStart);

	//	Otherwise, see if we end in the word "package". If so, we
	//	take everything before it.

	if (strEndsWith(strToLower(sPackageName), STR_PACKAGE) && sPackageName.GetLength() != STR_PACKAGE.GetLength())
		return strSubString(sPackageName, 0, sPackageName.GetLength() - STR_PACKAGE.GetLength());

	//	Otherwise, take the whole name

	return sPackageName;
	}

bool CHyperionPackageList::FindCommand (const CString &sAttrib, const CString &sCommand, CHyperionCommandSet::SCommandInfo *retInfo)

//	FindCommand
//
//	Looks for the given command by name (optionally restricted to a given
//	commandSet).

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Look in all packages

	for (i = 0; i < m_Packages.GetCount(); i++)
		{
		if (m_Packages[i].CommandSet.FindCommand(sAttrib, sCommand, retInfo))
			return true;
		}

	//	Not found

	return false;
	}

bool CHyperionPackageList::FindCommand (const CString &sPackage, const CString &sAttrib, const CString &sCommand, CHyperionCommandSet::SCommandInfo *retInfo)

//	FindCommand
//
//	Looks for the given command by name (optionally restricted to a given
//	commandSet).

	{
	CSmartLock Lock(m_cs);

	int iIndex;
	if (!FindPackageByName(sPackage, &iIndex))
		return false;

	return m_Packages[iIndex].CommandSet.FindCommand(sAttrib, sCommand, retInfo);
	}

bool CHyperionPackageList::FindPackage (const CString &sFilePath, int *retiIndex)

//	FindPackage
//
//	Finds the service by filePath. We assume that callers lock the semaphore

	{
	int i;

	for (i = 0; i < m_Packages.GetCount(); i++)
		if (strEquals(m_Packages[i].sFilePath, sFilePath))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

bool CHyperionPackageList::FindPackageByName (const CString &sName, int *retiIndex)

//	FindPackage
//
//	Finds the service by filePath. We assume that callers lock the semaphore

	{
	int i;

	//	We lowercase the comparison so that conflicts are case-insensitive

	CString sNameLower = strToLower(sName);

	for (i = 0; i < m_Packages.GetCount(); i++)
		if (strEquals(strToLower(m_Packages[i].sName), sNameLower))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

void CHyperionPackageList::GetCommands (const CString &sAttrib, TArray<CHyperionCommandSet::SCommandInfo> *retList)

//	GetCommands
//
//	Returns a list of commands with the given attribute.

	{
	CSmartLock Lock(m_cs);
	int i;

	retList->DeleteAll();

	//	Look in all packages

	for (i = 0; i < m_Packages.GetCount(); i++)
		m_Packages[i].CommandSet.GetCommands(sAttrib, retList);
	}

void CHyperionPackageList::GetPackageList (TArray<SPackageInfo> *retPackages)

//	GetPackageList
//
//	Returns a list of packages.

	{
	CSmartLock Lock(m_cs);
	int i;

	retPackages->DeleteAll();
	retPackages->InsertEmpty(m_Packages.GetCount());
	for (i = 0; i < m_Packages.GetCount(); i++)
		{
		SPackageInfo &Info = retPackages->GetAt(i);
		Info.sName = m_Packages[i].sName;
		Info.sFilePath = m_Packages[i].sFilePath;
		Info.sVersion = m_Packages[i].sVersion;
		Info.sModifiedBy = m_Packages[i].sModifiedBy;
		Info.ModifiedOn = m_Packages[i].ModifiedOn;
		}
	}

void CHyperionPackageList::GetPackageTables (const CString &sName, TArray<CDatum> *retTables)

//	GetPackageTables
//
//	Returns the list of tables defined by the given package

	{
	CSmartLock Lock(m_cs);
	int i;

	int iIndex;
	if (!FindPackageByName(sName, &iIndex))
		return;

	SPackage *pPackage = &m_Packages[iIndex];
	for (i = 0; i < pPackage->Tables.GetCount(); i++)
		retTables->Insert(pPackage->Tables[i].dDesc);
	}

void CHyperionPackageList::GetServices (TArray<SServiceInfo> *retList)

//	GetServices
//
//	Returns a flat list of services

	{
	CSmartLock Lock(m_cs);
	int i;

	retList->DeleteAll();
	for (i = 0; i < m_Packages.GetCount(); i++)
		retList->Insert(m_Packages[i].Services);
	}

void CHyperionPackageList::GetScheduledTasks (IArchonProcessCtx *pProcess, TArray<STaskInfo> *retList)

//	GetScheduleTasks
//
//	Returns the list of scheduled tasks defined by all packages

	{
	CSmartLock Lock(m_cs);
	int i, j;

	retList->DeleteAll();
	for (i = 0; i < m_Packages.GetCount(); i++)
		{
		TArray<CHyperionCommandSet::SCommandInfo> Commands;
		m_Packages[i].CommandSet.GetCommands(NULL_STR, &Commands);

		//	Loop over all commands

		for (j = 0; j < Commands.GetCount(); j++)
			{
			//	Get the schedule definition. If none, then we skip

			CDatum dSchedule = Commands[j].dDesc.GetElement(FIELD_SCHEDULE);
			if (dSchedule.IsNil())
				continue;

			int iInterval;
			if (strEquals(dSchedule, SCHEDULE_HOURLY))
				iInterval = 60 * 60;

			else if (strEquals(dSchedule, SCHEDULE_DAILY))
				iInterval = 60 * 60 * 24;

			else
				{
				pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_SCHEDULE, dSchedule.AsString()));
				continue;
				}

			//	Add the command

			STaskInfo *pNewTask = retList->Insert();
			pNewTask->sName = Commands[j].sName;
			pNewTask->iInterval = iInterval;
			pNewTask->dCode = Commands[j].dCode;
			}
		}
	}

bool CHyperionPackageList::InitPackage (SPackage *pPackage, CDatum dPackageDef, CString *retsError)

//	InitPackage
//
//	Initializes the package from a package definition.

	{
	//	Version

	pPackage->sVersion = dPackageDef.GetElement(FIELD_VERSION).AsString();

	//	Load the rights from the service desc

	pPackage->PackageSecurity.SetServiceRights(dPackageDef.GetElement(FIELD_RIGHTS));
	if (pPackage->PackageSecurity.HasNoServiceRights())
		pPackage->PackageSecurity.InsertServiceRight(RIGHT_ARC_DEVELOPER);

	//	We use the sandbox, unless we have admin rights

	if (!pPackage->PackageSecurity.HasServiceRight(RIGHT_ARC_ADMIN))
		{
		CString sSandbox = strPattern("%s.", pPackage->sName);
		pPackage->PackageSecurity.SetSandbox(sSandbox);
		}

	//	Done

	return true;
	}

bool CHyperionPackageList::LoadPackageDoc (IArchonProcessCtx *pProcess, SPackage *pPackage, CHexeProcess &Process, CHexeDocument &PackageDoc, CString *retsError)

//	LoadPackageDoc
//
//	Loads the document

	{
	int i;
	CString sError;

	//	Get the package sandbox prefix

	CString sSandboxPrefix = strPattern("%s.", pPackage->sName);

	//	Get the package definitions

	int iPackageDefIndex = PackageDoc.GetTypeIndex(TYPE_ARC_PACKAGE);
	int iPackageDefCount = PackageDoc.GetTypeIndexCount(iPackageDefIndex);
	if (iPackageDefCount == 0)
		{
		*retsError = strPattern(ERR_MISSING_PACKAGE_DEF, pPackage->sName);
		return false;
		}
	else if (iPackageDefCount > 1)
		{
		*retsError = strPattern(ERR_MULTIPLE_PACKAGE_DEF, pPackage->sName);
		return false;
		}

	//	Parse package definitions

	if (!InitPackage(pPackage, PackageDoc.GetTypeIndexData(iPackageDefIndex, 0), retsError))
		return false;

	//	Now that we know what rights the package requires, set the rights in the
	//	process global environment. (Again, each of the functions defined by the
	//	package will inherit the global environment).

	Process.SetSecurityCtx(pPackage->PackageSecurity);

	//	Get the list of service definitions

	int iServiceIndex = PackageDoc.GetTypeIndex(TYPE_ARC_SERVICE);
	for (i = 0; i < PackageDoc.GetTypeIndexCount(iServiceIndex); i++)
		{
		const CString &sServiceName = PackageDoc.GetTypeIndexName(iServiceIndex, i);
		CDatum dServiceDef = PackageDoc.GetTypeIndexData(iServiceIndex, i);

		if (!ValidateServiceName(pPackage, sSandboxPrefix, sServiceName, retsError))
			return false;

		//	Create the service

		IHyperionService *pService;
		if (!IHyperionService::CreateService(sServiceName, 
				dServiceDef, 
				PackageDoc, 
				pPackage->sName, 
				&pService, 
				retsError))
			return false;

		//	Add it to the package

		SServiceInfo *pEntry = pPackage->Services.Insert();
		pEntry->sName = sServiceName;
		pEntry->pService = pService;
		}

	//	If there are no services in the package then report a warning

	if (pPackage->Services.GetCount() == 0)
		{
		*retsError = strPattern(ERR_NO_SERVICES_IN_PACKAGE, pPackage->sName);
		return false;
		}

	//	Get a list of tables that the package wants to define

	int iTableIndex = PackageDoc.GetTypeIndex(TYPE_ARC_TABLE);
	for (i = 0; i < PackageDoc.GetTypeIndexCount(iTableIndex); i++)
		{
		const CString &sTableName = PackageDoc.GetTypeIndexName(iTableIndex, i);
		CDatum dDesc = PackageDoc.GetTypeIndexData(iTableIndex, i);

		//	Make sure the table name is in the sandbox (for now we cannot define
		//	tables out of the sandbox because there is no way to specify that
		//	a package has admin rights).

		if (!strStartsWith(sTableName, sSandboxPrefix))
			{
			pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_TABLE_OUT_OF_SANDBOX, pPackage->sName, sTableName));
			continue;
			}

		//	Make sure we add the table name to the descriptor

		CComplexStruct *pNewDesc = new CComplexStruct(dDesc);
		pNewDesc->SetElement(FIELD_NAME, sTableName);

		//	Add the descriptor

		STableInfo *pEntry = pPackage->Tables.Insert();
		pEntry->sName = sTableName;
		pEntry->dDesc = CDatum(pNewDesc);
		}

	//	Get a list of functions defined by the package

	int iFunctionIndex = PackageDoc.GetTypeIndex(TYPE_FUNCTION);
	for (i = 0; i < PackageDoc.GetTypeIndexCount(iFunctionIndex); i++)
		{
		const CString &sFunctionName = PackageDoc.GetTypeIndexName(iFunctionIndex, i);
		CDatum dDesc = PackageDoc.GetTypeIndexData(iFunctionIndex, i);

		CHyperionCommandSet::SCommandInfo *pCommandInfo;
		if (!pPackage->CommandSet.AddCommand(sFunctionName, dDesc, &pCommandInfo, &sError))
			{
			pProcess->Log(MSG_LOG_ERROR, sError);
			continue;
			}
		}

	//	Get the list of message sets to include

	int iMessageSetIndex = PackageDoc.GetTypeIndex(TYPE_INCLUDE_MESSAGES);
	for (i = 0; i < PackageDoc.GetTypeIndexCount(iMessageSetIndex); i++)
		{
		const CString &sAddr = PackageDoc.GetTypeIndexName(iMessageSetIndex, i);

		//	Generate the prefix from the address

		char *pPos = sAddr.GetParsePointer();
		char *pPosEnd = pPos + sAddr.GetLength();
		char *pStart = pPos;
		while (pPos < pPosEnd && *pPos != '.')
			pPos++;

		if (pPos == pPosEnd)
			{
			pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_MESSAGE_INCLUDE, sAddr));
			continue;
			}

		CString sPrefix(pStart, (pPos - pStart) + 1);

		//	Add to the mapping table

		CHexeProcess::AddHexarcMsgPattern(sPrefix, sAddr);
		}

	//	Define functions in the process so that other functions can get access
	//	to them later.

	if (!Process.LoadHexeDefinitions(PackageDoc, &sError))
		pProcess->Log(MSG_LOG_ERROR, sError);

	//	Done

	return true;
	}

bool CHyperionPackageList::LoadPackageDoc (IArchonProcessCtx *pProcess, SPackage *pPackage, IByteStream &Stream, CString *retsError)

//	LoadPackageDoc
//
//	Loads the document

	{
	//	Create a process into which we will load the document. This process will
	//	not persist, but its global environment will be kept by any function
	//	that we define.

	CHexeProcess Process;
	if (!Process.LoadStandardLibraries(retsError))
		return false;

	if (!Process.LoadLibrary(LIBRARY_SESSION, retsError))
		return false;

	//	Parse the package definition into a document

	CHexeDocument PackageDoc;
	if (!PackageDoc.InitFromStream(Stream, Process, retsError))
		return false;

	//	Get the package sandbox prefix

	return LoadPackageDoc(pProcess, pPackage, Process, PackageDoc, retsError);
	}

void CHyperionPackageList::Mark (void)

//	Mark
//
//	Mark all services in use

	{
	int i;

	//	Take the opportunity to collect garbage

	CollectGarbage();

	//	Mark

	for (i = 0; i < m_Packages.GetCount(); i++)
		MarkPackage(&m_Packages[i]);
	}

void CHyperionPackageList::MarkPackage (SPackage *pEntry)

//	MarkPackage
//
//	Marks a package

	{
	int i;

	for (i = 0; i < pEntry->Services.GetCount(); i++)
		pEntry->Services[i].pService->Mark();

	for (i = 0; i < pEntry->Tables.GetCount(); i++)
		pEntry->Tables[i].dDesc.Mark();

	pEntry->CommandSet.Mark();
	}

void CHyperionPackageList::SetPackages (const TArray<SPackageFileInfo> &List, TArray<CString> *retNeeded)

//	SetPackages
//
//	Given the list of services, we return a list of service files
//	that need to be updated.
//
//	In addition, we delete any service that is not in the provided list.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Assume that we should delete the package unless 
	//	it is build it or in the new list of packages.

	for (i = 0; i < m_Packages.GetCount(); i++)
		m_Packages[i].bDelete = !(m_Packages[i].bBuiltIn);

	//	Loop over the new list

	retNeeded->DeleteAll();
	for (i = 0; i < List.GetCount(); i++)
		{
		//	Look for this service in the current list

		int iIndex;
		if (FindPackage(List[i].sFilePath, &iIndex))
			{
			//	If there is a newer version of this, then we need it

			if (List[i].dwVersion > m_Packages[iIndex].dwVersion)
				retNeeded->Insert(List[i].sFilePath);

			//	No need to delete this service

			m_Packages[iIndex].bDelete = false;
			}

		//	Otherwise we need to add it

		else
			retNeeded->Insert(List[i].sFilePath);
		}

	//	Delete the packages that we no longer need

	for (i = 0; i < m_Packages.GetCount(); i++)
		if (m_Packages[i].bDelete)
			{
			CleanUpPackage(&m_Packages[i]);

			m_Packages.Delete(i);
			i--;
			}
	}

bool CHyperionPackageList::ValidateServiceName (SPackage *pPackage, const CString &sSandboxPrefix, const CString &sServiceName, CString *retsError)

//	ValidateServiceName
//
//	Make sure that the service name is appropriate

	{
	//	Built-in packages do not need to be checked

	if (pPackage->bBuiltIn)
		return true;

	//	Certain names are reserved for built-in packages

	if (strStartsWith(sServiceName, NAMESPACE_ARC))
		{
		*retsError = strPattern(ERR_ARC_RESERVED, sServiceName);
		return false;
		}

	if (!strStartsWith(sServiceName, sSandboxPrefix))
		{
		*retsError = strPattern(ERR_SERVICE_OUT_OF_SANDBOX, pPackage->sName, sServiceName);
		return false;
		}

	//	Otherwise, OK

	return true;
	}
