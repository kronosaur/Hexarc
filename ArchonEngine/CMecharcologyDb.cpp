//	CMecharcologyDb.cpp
//
//	CMecharcologyDb class
//	Copyright (c) 2010 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_FILESPEC,					"filespec");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_NODE_ID,						"nodeID");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");
DECLARE_CONST_STRING(FIELD_VERSION,						"version");

DECLARE_CONST_STRING(STR_ERROR_MODULE_ALREADY_LOADED,	"%s has already been loaded.");
DECLARE_CONST_STRING(STR_ERROR_CANNOT_FIND_MODULE_PATH,	"Unable to find module %s.");
DECLARE_CONST_STRING(STR_ERROR_CANNOT_LAUNCH_MODULE,	"Unable to launch module %s : %s");

DECLARE_CONST_STRING(STR_ARCOLOGY_PRIME,				"Arcology Prime");
DECLARE_CONST_STRING(STR_MODULE_EXTENSION,				".exe");
DECLARE_CONST_STRING(STR_DEBUG_SWITCH,					" /debug");
DECLARE_CONST_STRING(STR_UNKNOWN_VERSION,				"unknown");

DECLARE_CONST_STRING(STR_MACHINE_STATUS_ACTIVE,			"running");
DECLARE_CONST_STRING(STR_MACHINE_STATUS_UNKNOWN,		"unknown");

DECLARE_CONST_STRING(STR_MODULE_STATUS_LAUNCHED,		"launched");
DECLARE_CONST_STRING(STR_MODULE_STATUS_ON_START,		"onStart");
DECLARE_CONST_STRING(STR_MODULE_STATUS_REMOVED,			"removed");
DECLARE_CONST_STRING(STR_MODULE_STATUS_RESTARTED,		"restarted");
DECLARE_CONST_STRING(STR_MODULE_STATUS_RUNNING,			"running");
DECLARE_CONST_STRING(STR_MODULE_STATUS_UNKNOWN,			"unknown");

DECLARE_CONST_STRING(ERR_DUPLICATE_MACHINE,				"Duplicate machine address.");
DECLARE_CONST_STRING(ERR_CANT_JOIN,						"Unable to join another arcology.");
DECLARE_CONST_STRING(ERR_CANT_LEAVE,					"Unable to leave arcology.");

bool CMecharcologyDb::AddMachine (CStringView sNodeID, CStringView sDisplayName, CStringView sAddress, const CIPInteger &SecretKey, bool bCheckUpgrade, CString *retsError)

//	AddMachine
//
//	Adds an entry for this machine. We leave machine name and state empty 
//	because we haven't yet heard from the actual machine.

	{
	CSmartLock Lock(m_cs);

	//	Make sure we don't have a machine with the same address or the same
	//	secret key.

	for (int i = 0; i < m_Machines.GetCount(); i++)
		if (m_Machines[i].SecretKey == SecretKey
				|| strEquals(m_Machines[i].sAddress, sAddress))
			{
			*retsError = ERR_DUPLICATE_MACHINE;
			return false;
			}

	//	Add it

	SMachineEntry *pMachine = m_Machines.Insert();
	pMachine->sNodeID = sNodeID;
	pMachine->iStatus = connectNone;
	pMachine->sAddress = sAddress;
	pMachine->sDisplayName = sDisplayName;
	pMachine->SecretKey = SecretKey;
	pMachine->bCheckUpgrade = bCheckUpgrade;

	return true;
	}

CString CMecharcologyDb::AllocNodeID () const

//	AllocNodeID
//
//	Generate a unique node ID.

	{
	CSmartLock Lock(m_cs);

	//	We allocate node IDs starting at 2 (1 is always Arcology Prime).
	DWORD dwID = 2;

	while (true)
		{
		CString sNodeID = MakeNodeID(dwID);

		bool bFound = false;
		for (int i = 0; i < m_Machines.GetCount(); i++)
			if (strEquals(m_Machines[i].sNodeID, MakeNodeID(dwID)))
				{
				bFound = true;
				break;
				}

		if (!bFound)
			return sNodeID;

		dwID++;
		}
	}

SMachineDesc CMecharcologyDb::AsMachineDesc (const SMachineEntry& Entry)

//	AsMachineDesc
//
//	Convert to a machine description.

	{
	SMachineDesc Desc;
	Desc.sNodeID = Entry.sNodeID;
	Desc.sName = Entry.sName;
	Desc.sAddress = Entry.sAddress;
	Desc.Key = Entry.SecretKey;

	return Desc;
	}

bool CMecharcologyDb::AuthenticateMachine (const CString &sAuthName, const CIPInteger &AuthKey)

//	AuthenticateMachine
//
//	A machine wants to authenticate by giving us their secret key. We return 
//	TRUE if we can authenticate the machine. FALSE otherwise.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Look for a machine with this key

	for (i = 0; i < m_Machines.GetCount(); i++)
		if (m_Machines[i].SecretKey == AuthKey)
			{
			//	If this is not the same name, then we need to reload Mnemosynth,
			//	etc.

			if (!strEquals(m_Machines[i].sName, sAuthName))
				{
#ifdef DEBUG_STARTUP
				printf("[CMecharcologyDb::AuthenticateMachine]: Replacing entry %d from %s to %s\n", i, (LPSTR)m_Machines[i].sName, (LPSTR)sAuthName);
#endif
				//	Keep track of the old name so that we clean up Mnemosynth.
				//
				//	NOTE: For secondary machines, the first time we authenticate Arcology Prime,
				//	we replace blank with the actual name. Thus we only track an old name if it
				//	is valid.

				if (!m_Machines[i].sName.IsEmpty())
					m_OldMachines.Insert(m_Machines[i].sName);

				//	New name

				m_Machines[i].sName = sAuthName;
				m_Machines[i].iStatus = connectAuth;
				}

			//	Otherwise, if we're not yet connected, then we flip to auth mode.

			else if (m_Machines[i].iStatus == connectNone)
				{
				m_Machines[i].iStatus = connectAuth;
				}

			//	Success!

			return true;
			}

	//	Did not authenticate

	return false;
	}

void CMecharcologyDb::Boot (const SInit &Init)

//	Boot
//
//	Boot up the mecharcology with a single machine

	{
	CSmartLock Lock(m_cs);

	ASSERT(m_Machines.GetCount() == 0);

	m_sModulePath = Init.sModulePath;

	SetCurrentModule(Init.sCurrentModule);

	m_MachineDesc.sName = Init.sMachineName;
	m_MachineDesc.sAddress = Init.sMachineHost;

	//	If we've got an Arcology Prime address, then we add an entry.
	//	We only do this for secondary machines (Arcology Prime does not need
	//	to keep an entry for itself).

	if (!Init.sArcologyPrimeAddress.IsEmpty())
		{
		SMachineEntry *pPrime = m_Machines.Insert();
		pPrime->sNodeID = ArcologyPrimeNodeID();
		pPrime->sName = NULL_STR;	//	We don't know the machine name yet, we'll get it during AUTH
		pPrime->iStatus = connectNone;
		pPrime->sAddress = Init.sArcologyPrimeAddress;
		pPrime->sDisplayName = STR_ARCOLOGY_PRIME;

		//	We won't know secret key until we load the config file (or until we 
		//	get a JOIN command). We won't know name until we get our first 
		//	message.

		m_iArcologyPrime = 0;

		//	We don't set up the NodeID until we connect.
		}
	else
		{
		m_iArcologyPrime = -1;
		m_MachineDesc.sNodeID = ArcologyPrimeNodeID();
		}
	}

bool CMecharcologyDb::CanRestartModule (const CString &sName) const

//	CanRestartModule
//
//	Returns TRUE if we can restart the module.

	{
	CSmartLock Lock(m_cs);

	//	Find the module by name (if not found, then OK to restart).

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return true;

	//	If we've been asked to restart then do it.

	if (m_Modules[iModule].iStatus == moduleRestarted)
		return true;

	//	How long has the module been running, if it hasn't been too long, then
	//	don't restart, in case we've got a crash bug.

	CTimeSpan Uptime = timeSpan(m_Modules[iModule].StartTime, CDateTime(CDateTime::Now));
	if (Uptime.Seconds64() < MIN_RUN_TIME_TO_RESTART)
		return false;

	//	OK to restart.

	return true;
	}

bool CMecharcologyDb::CheckForUpgrade (const CString &sMachineName)

//	CheckForUpgrade
//
//	Returns TRUE if we need to ask this machine to check for upgrades, and then
//	clears the flag.

	{
	CSmartLock Lock(m_cs);

	int iMachine;
	if ((iMachine = FindMachine(sMachineName)) == -1)
		return false;

	if (!m_Machines[iMachine].bCheckUpgrade)
		return false;

	m_Machines[iMachine].bCheckUpgrade = false;
	return true;
	}

bool CMecharcologyDb::DeleteMachine (const CString &sName)

//	DeleteMachine
//
//	Removes the given machine from the arcology. Returns TRUE if the machine was
//	deleted.

	{
	CSmartLock Lock(m_cs);

	int iMachine;
	if ((iMachine = FindMachine(sName)) == -1)
		return false;

	//	Delete the machine

	m_Machines.Delete(iMachine);
	return true;
	}

bool CMecharcologyDb::DeleteMachine (const SMachineDesc &Desc)

//	DeleteMachine
//
//	Removes the machine based on whatever information we have.

	{
	CSmartLock Lock(m_cs);

	int iMachine;
	if ((iMachine = FindMachine(Desc)) == -1)
		return false;

	//	Delete the machine

	m_Machines.Delete(iMachine);
	return true;
	}

bool CMecharcologyDb::DeleteMachineByKey (const CIPInteger &Key)

//	DeleteMachineByKey
//
//	Delets the machine with the given secret key.

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Machines.GetCount(); i++)
		if (m_Machines[i].SecretKey == Key)
			{
			m_Machines.Delete(i);
			return true;
			}

	return false;
	}

bool CMecharcologyDb::DeleteModule (const CString &sName)

//	DeleteModule
//
//	Removes the module from the arcology. This is called when the process terminates.
//	Returns TRUE if the modules was deleted.

	{
	CSmartLock Lock(m_cs);

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return false;

	//	Delete the module

	m_Modules.Delete(iModule);
	return true;
	}

bool CMecharcologyDb::FindArcologyPrime (SMachineDesc *retDesc) const

//	FindArcologyPrime
//
//	Returns Arcology Prime, if we have proper keys

	{
	CSmartLock Lock(m_cs);
	if (!retDesc)
		throw CException(errFail);

	if (m_iArcologyPrime == -1)
		return false;

	*retDesc = AsMachineDesc(m_Machines[m_iArcologyPrime]);
	return true;
	}

int CMecharcologyDb::FindMachine (const SMachineDesc &Desc) const

//	FindMachine
//
//	Finds the machine by descriptor. Returns -1 if not found.

	{
	for (int i = 0; i < m_Machines.GetCount(); i++)
		{
		if (!Desc.Key.IsEmpty())
			{
			if (Desc.Key == m_Machines[i].SecretKey)
				return i;
			}
		else if (!Desc.sName.IsEmpty())
			{
			if (strEquals(Desc.sName, m_Machines[i].sName))
				return i;
			}
		else if (!Desc.sAddress.IsEmpty())
			{
			if (strEquals(Desc.sAddress, m_Machines[i].sAddress))
				return i;
			}
		}

	return -1;
	}

int CMecharcologyDb::FindMachine (const CString &sName)

//	FindMachine
//
//	Returns the index of the given machine (or -1 if not found).
//	This must be used inside a lock.

	{
	int i;

	CString sNameToFind = strToLower(sName);

	for (i = 0; i < m_Machines.GetCount(); i++)
		if (strEquals(sNameToFind, strToLower(m_Machines[i].sName)))
			return i;

	return -1;
	}

bool CMecharcologyDb::FindMachineByAddress (const CString &sAddress, SMachineDesc *retDesc)

//	FindMachineByAddress
//
//	Looks for the given machine by address.

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Machines.GetCount(); i++)
		if (strEquals(sAddress, m_Machines[i].sAddress))
			{
			if (retDesc)
				*retDesc = AsMachineDesc(m_Machines[i]);

			return true;
			}

	return false;
	}

bool CMecharcologyDb::FindMachineByName (const CString &sName, SMachineDesc *retDesc) const

//	FindMachineByName
//
//	Looks for the given machine by name.

	{
	CSmartLock Lock(m_cs);

	if (sName.IsEmpty())
		return FindArcologyPrime(retDesc);

	for (int i = 0; i < m_Machines.GetCount(); i++)
		if (strEquals(sName, m_Machines[i].sName))
			{
			if (retDesc)
				*retDesc = AsMachineDesc(m_Machines[i]);

			return true;
			}

	return false;
	}

bool CMecharcologyDb::FindMachineByNodeID (CStringView sNodeID, SMachineDesc* retDesc) const

//	FindMachineByNodeID
//
//	Looks for the given machine by node ID.

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Machines.GetCount(); i++)
		if (strEquals(sNodeID, m_Machines[i].sNodeID))
			{
			if (retDesc)
				*retDesc = AsMachineDesc(m_Machines[i]);

			return true;
			}

	return false;
	}

bool CMecharcologyDb::FindMachineByPartialName (const CString &sName, SMachineDesc *retDesc) const

//	FindMachineByPartialName
//
//	Looks for the given machine by partial name. If we don't the partial name
//	matches more than one machine, we return FALSE.

	{
	CSmartLock Lock(m_cs);

	if (sName.IsEmpty())
		return FindArcologyPrime(retDesc);

	int iMatch = -1;
	for (int i = 0; i < m_Machines.GetCount(); i++)
		{
		//	Use either the name or the address. [Sometimes there is no name
		//	if there was an error adding the machine.]

		const CString &sTargetName = (m_Machines[i].sName.IsEmpty() ? m_Machines[i].sAddress : m_Machines[i].sName);

		//	If no match, then skip

		if (!strStartsWithNoCase(sTargetName, sName))
			continue;

		//	If we have an exact match, then we're done.

		if (sTargetName.GetLength() == sName.GetLength())
			{
			iMatch = i;
			break;
			}

		//	Otherwise, if we've already got a match then it means that multiple
		//	names match, in which case we return FALSE.

		else if (iMatch != -1)
			return false;

		//	Else, we have a match, but we need to continue searching to make
		//	sure there are no duplicates.

		else
			{
			iMatch = i;
			}
		}

	//	Done

	if (iMatch == -1)
		return false;

	if (retDesc)
		*retDesc = AsMachineDesc(m_Machines[iMatch]);

	return true;
	}

int CMecharcologyDb::FindModule (const CString &sName) const

//	FindModule
//
//	Returns the index of the given module (or -1 if not found).
//	This must be used inside a lock.

	{
	int i;

	CString sNameToFind = strToLower(sName);

	for (i = 0; i < m_Modules.GetCount(); i++)
		if (strEquals(sNameToFind, strToLower(m_Modules[i].sName)))
			return i;

	return -1;
	}

bool CMecharcologyDb::FindModuleByFilespec (const CString &sFilespec, SModuleDesc *retDesc)

//	FindModuleByFilespec
//
//	Finds the module

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Modules.GetCount(); i++)
		if (fileIsPathEqual(sFilespec, m_Modules[i].sFilespec))
			{
			if (retDesc)
				{
				retDesc->sName = m_Modules[i].sName;
				retDesc->sFilespec = m_Modules[i].sFilespec;
				retDesc->sVersion = m_Modules[i].sVersion;
				retDesc->sStatus = StatusToID(m_Modules[i].iStatus);
				retDesc->dwProcessID = m_Modules[i].hProcess.GetID();
				}

			return true;
			}

	return false;
	}

bool CMecharcologyDb::GetCentralModule (SModuleDesc *retDesc)

//	GetCentralModule
//
//	Returns the process information (NOTE: hProcess is not initialized)

	{
	CSmartLock Lock(m_cs);

	//	Central module is always module 0

	if (m_Modules.GetCount() == 0)
		return false;

	if (retDesc)
		{
		retDesc->sName = m_Modules[0].sName;
		retDesc->sFilespec = m_Modules[0].sFilespec;
		retDesc->sVersion = m_Modules[0].sVersion;
		retDesc->sStatus = StatusToID(m_Modules[0].iStatus);
		retDesc->dwProcessID = m_Modules[0].hProcess.GetID();
		}

	return true;
	}

bool CMecharcologyDb::GetMachine (const CString &sName, SMachineDesc *retDesc)

//	GetMachine
//
//	Returns machine information. Returns FALSE if the machine cannot be found

	{
	CSmartLock Lock(m_cs);

	//	If sID is blank then we return the current machine

	if (sName.IsEmpty())
		{
		if (retDesc)
			*retDesc = m_MachineDesc;
		return true;
		}

	//	Find the machine by name

	int iMachine;
	if ((iMachine = FindMachine(sName)) == -1)
		return false;

	if (retDesc)
		*retDesc = AsMachineDesc(m_Machines[iMachine]);

	return true;
	}

bool CMecharcologyDb::GetMachine (int iIndex, SMachineDesc *retDesc) const

//	GetMachine
//
//	Returns the machine information.

	{
	CSmartLock Lock(m_cs);

	if (iIndex < 0 || iIndex >= m_Machines.GetCount())
		return false;

	if (retDesc)
		*retDesc = AsMachineDesc(m_Machines[iIndex]);

	return true;
	}

bool CMecharcologyDb::GetModule (const CString &sName, SModuleDesc *retDesc) const

//	GetModule
//
//	Returns the process information (NOTE: hProcess is not initialized)

	{
	CSmartLock Lock(m_cs);

	//	Find the module by name

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return false;

	if (retDesc)
		{
		retDesc->sName = m_Modules[iModule].sName;
		retDesc->sFilespec = m_Modules[iModule].sFilespec;
		retDesc->sVersion = m_Modules[iModule].sVersion;
		retDesc->sStatus = StatusToID(m_Modules[iModule].iStatus);
		retDesc->dwProcessID = m_Modules[iModule].hProcess.GetID();
		}

	return true;
	}

CDatum CMecharcologyDb::GetModuleList (void) const

//	GetModuleList
//
//	Returns an array of module info

	{
	CSmartLock Lock(m_cs);
	int i;

	CComplexArray *pArray = new CComplexArray;
	for (i = 0; i < m_Modules.GetCount(); i++)
		{
		CComplexStruct *pModuleInfo = new CComplexStruct;

		pModuleInfo->SetElement(FIELD_NAME, m_Modules[i].sName);
		pModuleInfo->SetElement(FIELD_FILESPEC, m_Modules[i].sFilespec);
		pModuleInfo->SetElement(FIELD_VERSION, m_Modules[i].sVersion);
		pModuleInfo->SetElement(FIELD_STATUS, StatusToID(m_Modules[i].iStatus));

		//	Add to list

		pArray->Append(CDatum(pModuleInfo));
		}

	//	Done

	return CDatum(pArray);
	}

CProcess *CMecharcologyDb::GetModuleProcess (const CString &sName)

//	GetModuleProcess
//
//	Returns the module's process (or NULL if not found)

	{
	CSmartLock Lock(m_cs);

	//	Find the module by name

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return NULL;

	return &m_Modules[iModule].hProcess;
	}

CTimeSpan CMecharcologyDb::GetModuleRunTime (const CString &sName) const

//	GetModuleRunTime
//
//	Return the time that the module has been running.

	{
	CSmartLock Lock(m_cs);

	//	Find the module by name

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return CTimeSpan();

	return timeSpan(m_Modules[iModule].StartTime, CDateTime(CDateTime::Now));
	}

bool CMecharcologyDb::IsModuleRemoved (const CString &sName) const

//	IsModuleRemove
//
//	Returns TRUE if the module has been removed

	{
	CSmartLock Lock(m_cs);

	//	Find the module by name

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return true;

	return (m_Modules[iModule].iStatus == moduleRemoved);
	}

bool CMecharcologyDb::IsModuleRunning (const CString &sName) const

//	IsModuleRunning
//
//	Returns TRUE if the given module is running.

	{
	CSmartLock Lock(m_cs);

	//	Find the module by name

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return true;

	return (m_Modules[iModule].iStatus == moduleRunning);
	}

bool CMecharcologyDb::GetStatus (CDatum *retStatus)

//	GetStatus
//
//	Returns the status

	{
	CSmartLock Lock(m_cs);

	CDatum dList(CDatum::typeArray);

	//	Add Arcology Prime as the first

	if (m_iArcologyPrime == -1)
		{
		CDatum dData(CDatum::typeStruct);
		dData.SetElement(FIELD_NODE_ID, ArcologyPrimeNodeID());
		dData.SetElement(FIELD_ID, m_MachineDesc.sName);
		dData.SetElement(FIELD_NAME, STR_ARCOLOGY_PRIME);
		dData.SetElement(FIELD_ADDRESS, m_MachineDesc.sAddress);
		dData.SetElement(FIELD_STATUS, STR_MACHINE_STATUS_ACTIVE);
		dList.Append(dData);
		}

	//	Add all machines

	for (int i = 0; i < m_Machines.GetCount(); i++)
		{
		const SMachineEntry &Entry = m_Machines[i];
		CDatum dData(CDatum::typeStruct);

		dData.SetElement(FIELD_NODE_ID, Entry.sNodeID);
		dData.SetElement(FIELD_ID, Entry.sName);
		dData.SetElement(FIELD_NAME, Entry.sDisplayName);
		dData.SetElement(FIELD_ADDRESS, Entry.sAddress);

		switch (Entry.iStatus)
			{
			case connectActive:
				dData.SetElement(FIELD_STATUS, STR_MACHINE_STATUS_ACTIVE);
				break;

			default:
				dData.SetElement(FIELD_STATUS, STR_MACHINE_STATUS_UNKNOWN);
			}

		//	Add to list

		dList.Append(dData);
		}

	//	Done

	*retStatus = dList;
	return true;
	}

bool CMecharcologyDb::HasArcologyKey (CIPInteger* retsKey) const

//	HasArcologyKey
//
//	Returns TRUE if we have the keys to be part of an arcology

	{
	CSmartLock Lock(m_cs);

	//	If we're Arcology Prime, then we have a key

	if (m_iArcologyPrime == -1)
		//	We never get back a key because it depends on who we are talking to
		return true;

	//	Make sure we have a key

	const SMachineEntry &Prime = m_Machines[m_iArcologyPrime];
	if (Prime.SecretKey.IsEmpty())
		return false;

	//	Done

	if (retsKey)
		*retsKey = Prime.SecretKey;
	return true;
	}

bool CMecharcologyDb::IsArcologyPrime (const CString &sName) const

//	IsArcologyPrime
//
//	Returns TRUE if the given machine (by name) is Arcology Prime.

	{
	CSmartLock Lock(m_cs);

	if (m_iArcologyPrime == -1)
		return false;

	return strEquals(sName, m_Machines[m_iArcologyPrime].sName);
	}

bool CMecharcologyDb::JoinArcology (const CString &sPrimeName, const CIPInteger &PrimeKey, CString *retsError)

//	JoinArcology
//
//	We've been asked by Arcology Prime to join.

	{
	CSmartLock Lock(m_cs);

	if (m_iArcologyPrime == -1
			|| m_iArcologyPrime >= m_Machines.GetCount())
		{
		*retsError = ERR_CANT_JOIN;
		return false;
		}

	SMachineEntry &Prime = m_Machines[m_iArcologyPrime];
	if (!Prime.SecretKey.IsEmpty())
		{
		*retsError = ERR_CANT_JOIN;
		return false;
		}

	Prime.sName = sPrimeName;
	Prime.SecretKey = PrimeKey;

	return true;
	}

bool CMecharcologyDb::LeaveArcology (CString *retsError)

//	LeaveArcology
//
//	We've been asked by Arcology Prime to leave.

	{
	CSmartLock Lock(m_cs);

	if (m_iArcologyPrime == -1
			|| m_iArcologyPrime >= m_Machines.GetCount())
		{
		*retsError = ERR_CANT_LEAVE;
		return false;
		}

	SMachineEntry &Prime = m_Machines[m_iArcologyPrime];
	Prime.sName = NULL_STR;
	Prime.SecretKey = CIPInteger();

	return true;
	}

bool CMecharcologyDb::LoadModule (const CString &sFilespec, bool bDebug, CString *retsName, CString *retsError)

//	LoadModule
//
//	Loads the given module. Returns TRUE if successful.

	{
	CSmartLock Lock(m_cs);

	//	Get a module name by taking just the filename (without extension)

	CString sModuleName;
	fileGetExtension(fileGetFilename(sFilespec), &sModuleName);

	//	Don't load the module if it has already been loaded

	if (FindModule(sModuleName) != -1)
		{
		//	LATER: Due to a race condition, this module might have already terminated.
		//	Check to see if it has, and if so, just replace the module.

		*retsError = strPattern(STR_ERROR_MODULE_ALREADY_LOADED, sModuleName);
		return false;
		}

	//	Make sure we have an extension

	CString sModule = fileAppendExtension(sFilespec, STR_MODULE_EXTENSION);

	//	Generate a path using the module directory

	CString sModuleFilespec = fileAppend(m_sModulePath, sModule);

	//	Make sure it exists

	if (!fileExists(sModuleFilespec))
		{
		*retsError = strPattern(STR_ERROR_CANNOT_FIND_MODULE_PATH, sModule);
		return false;
		}

	//	Create a module entry

	int iIndex = m_Modules.GetCount();
	SModuleEntry *pModule = m_Modules.Insert();
	pModule->sName = sModuleName;
	pModule->sFilespec = sModule;
	pModule->StartTime = CDateTime(CDateTime::Now);

	//	Get the version

	SFileVersionInfo VersionInfo;
	if (fileGetVersionInfo(sModuleFilespec, &VersionInfo))
		pModule->sVersion = VersionInfo.sProductVersion;
	else
		pModule->sVersion = STR_UNKNOWN_VERSION;

	//	Command line options

	CString sOptions;
	if (bDebug)
		sOptions += CString(" /debug");

	if (IsArcologyPrime())
		sOptions += CString(" /onArcologyPrime");

	//	Generate the command line

	CString sCmdLine;
	sCmdLine = strPattern("%s /machine:%s %s", sModuleFilespec, m_MachineDesc.sName, sOptions);

	//	Create the process

	try
		{
		pModule->hProcess.Create(sCmdLine);
		}
	catch (CException e)
		{
		m_Modules.Delete(iIndex);

		*retsError = strPattern(STR_ERROR_CANNOT_LAUNCH_MODULE, sModule, e.GetErrorString());
		return false;
		}

	//	Set state

	pModule->iStatus = moduleLaunched;

	//	Return the name of the module

	*retsName = sModuleName;

	return true;
	}

CString CMecharcologyDb::MakeNodeID (DWORD dwID)
	{
	return strPattern("Node-%01d", dwID);
	}

bool CMecharcologyDb::OnCompleteAuth (CStringView sName, CString& retsNodeID)

//	OnCompleteAuth
//
//	Returns TRUE if we need to complete the authentication by adding an 
//	endpoint to Mnemosynth.

	{
	CSmartLock Lock(m_cs);

	//	Look for a machine by name

	for (int i = 0; i < m_Machines.GetCount(); i++)
		if (strEquals(m_Machines[i].sName, sName))
			{
			//	Return the node ID

			retsNodeID = m_Machines[i].sNodeID;

			//	If we're still in connectAuth mode, then we return TRUE

			if (m_Machines[i].iStatus == connectAuth)
				{
				m_Machines[i].iStatus = connectActive;
				return true;
				}

			//	Otherwise, we return false (because we've already completed this
			//	step).

			return false;
			}

	//	If we did not find the machine, then we can't complete auth

	return false;
	}

void CMecharcologyDb::OnModuleRestart (const CString &sName)

//	OnModuleRestart
//
//	We're going to restart the given module.

	{
	CSmartLock Lock(m_cs);

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return;

	m_Modules[iModule].iStatus = moduleRestarted;
	}

void CMecharcologyDb::OnModuleStart (const CString &sName, MnemosynthSequence dwMnemosynthSeq, bool *retbAllComplete)

//	OnModuleStart
//
//	The given module has started running

	{
	CSmartLock Lock(m_cs);
	int i;

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return;

	m_Modules[iModule].iStatus = moduleOnStart;
	m_Modules[iModule].dwSeq = dwMnemosynthSeq;

	//	See if all modules have completed this

	*retbAllComplete = true;
	for (i = 0; i < m_Modules.GetCount(); i++)
		if (m_Modules[i].iStatus != moduleOnStart && m_Modules[i].iStatus != moduleRunning)
			{
			*retbAllComplete = false;
			break;
			}
	}

void CMecharcologyDb::OnMnemosynthUpdated (void)

//	OnMnemosynthUpdated
//
//	All modules are now running.

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Modules.GetCount(); i++)
		{
		if (m_Modules[i].iStatus == moduleOnStart)
			m_Modules[i].iStatus = moduleRunning;
		}
	}

bool CMecharcologyDb::ProcessOldMachines (TArray<CString> &OldMachines)

//	ProcessOldMachines
//
//	Returns a list of old machine names and clears the internal list. The caller
//	should remove the machines from Mnemosynth. We return TRUE if there are any
//	machines in the list.

	{
	CSmartLock Lock(m_cs);

	OldMachines = m_OldMachines;
	m_OldMachines.DeleteAll();

	return (OldMachines.GetCount() > 0);
	}

bool CMecharcologyDb::SetArcologyKey (const CIPInteger &PrimeKey, CString *retsError)

//	SetArcologyKey
//
//	Sets the key

	{
	CSmartLock Lock(m_cs);

	if (m_iArcologyPrime == -1
			|| m_iArcologyPrime >= m_Machines.GetCount())
		{
		*retsError = ERR_CANT_JOIN;
		return false;
		}

	SMachineEntry &Prime = m_Machines[m_iArcologyPrime];
	if (!Prime.SecretKey.IsEmpty())
		{
		*retsError = ERR_CANT_JOIN;
		return false;
		}

	Prime.SecretKey = PrimeKey;

	return true;
	}

void CMecharcologyDb::SetCurrentModule (const CString &sName)

//	SetCurrentModule
//
//	Sets the current module. This is only ever called by Exarch when
//	it is booting. [Remember that only the CentralModule has an
//	Exarch, and thus a mecharcology.]

	{
	CSmartLock Lock(m_cs);

	ASSERT(m_Modules.GetCount() == 0);

	SModuleEntry *pModule = m_Modules.Insert();
	pModule->sName = sName;
	pModule->sFilespec = fileGetFilename(fileGetExecutableFilespec());
	pModule->hProcess.CreateCurrentProcess();

	//	Get the version

	SFileVersionInfo VersionInfo;
	if (fileGetVersionInfo(fileGetExecutableFilespec(), &VersionInfo))
		pModule->sVersion = VersionInfo.sProductVersion;
	else
		pModule->sVersion = STR_UNKNOWN_VERSION;

	//	CentralModule doesn't need an OnStart because it launches
	//	all other modules. We set our state to OnStart so that the loop
	//	in OnModuleStart can see that all modules (including this one)
	//	have started.

	pModule->iStatus = moduleOnStart;

	//	We don't yet know the sequence number to check for; we will
	//	figure it out in OnModuleStart

	pModule->dwSeq = 0;
	}

bool CMecharcologyDb::SetHostAddress (const CString &sName, const CString &sHostAddress)

//	SetHostAddress
//
//	Sets the host address for the given machine.

	{
	CSmartLock Lock(m_cs);

	//	If NULL_STR then we mean the current machine

	if (sName.IsEmpty())
		{
		m_MachineDesc.sAddress = sHostAddress;
		return true;
		}

	//	Find the machine

	int iMachine;
	if ((iMachine = FindMachine(sName)) == -1)
		return false;

	m_Machines[iMachine].sAddress = sHostAddress;

	return true;
	}

void CMecharcologyDb::SetModuleRemoved (const CString &sName)

//	SetModuleRemove
//
//	Marks the module as having been removed.

	{
	CSmartLock Lock(m_cs);

	//	Find the module by name

	int iModule;
	if ((iModule = FindModule(sName)) == -1)
		return;

	m_Modules[iModule].iStatus = moduleRemoved;
	}

CString CMecharcologyDb::StatusToID (EModuleStates iStatus) const

//	StatusToID
//
//	Convert to a status string.

	{
	switch (iStatus)
		{
		case moduleLaunched:
			return STR_MODULE_STATUS_LAUNCHED;

		case moduleOnStart:
			return STR_MODULE_STATUS_ON_START;

		case moduleRemoved:
			return STR_MODULE_STATUS_REMOVED;

		case moduleRestarted:
			return STR_MODULE_STATUS_RESTARTED;

		case moduleRunning:
			return STR_MODULE_STATUS_RUNNING;

		default:
			return STR_MODULE_STATUS_UNKNOWN;
		}
	}
