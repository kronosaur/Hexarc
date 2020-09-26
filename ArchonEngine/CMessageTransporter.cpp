//	CMessageTransporter.cpp
//
//	CMessageTransporter class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_ARCOLOGY_PRIME,				"ArcologyPrime");
DECLARE_CONST_STRING(STR_LOCAL_SYMBOL,					"~");
DECLARE_CONST_STRING(STR_LOCALHOST_MACHINE_NAME,		"localhost");

DECLARE_CONST_STRING(FIELD_NAME,						"name");

DECLARE_CONST_STRING(MNEMO_ARC_MACHINES,				"Arc.machines");
DECLARE_CONST_STRING(MNEMO_ARC_PORTS,					"Arc.ports");

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(ERR_CANT_BIND,						"Unable to bind to address: %s.");

CMessageTransporter::~CMessageTransporter (void)

//	CMessageTransporter destructor

	{
	int i;

	for (i = 0; i < m_Ports.GetCount(); i++)
		delete m_Ports[i];

	//	Clean up any bindings that we need to delete

	for (i = 0; i < m_LocalPorts.GetCount(); i++)
		delete m_LocalPorts[i];

	for (i = 0; i < m_EngineBinding.GetCount(); i++)
		delete m_EngineBinding[i];

	for (i = 0; i < m_ProcessQueues.GetCount(); i++)
		delete m_ProcessQueues[i];
	}

void CMessageTransporter::AddLocalPort (const CString &sPort, IArchonMessagePort *pPort)

//	AddLocalPort
//
//	Adds a local port

	{
	ASSERT(!sPort.IsEmpty());
	ASSERT(pPort);

	//	sPort can be either a naked port or an address. In the latter case, we parse
	//	the port name out of the address.

	CString sPortName = ParseAddressPort(sPort);
	if (sPortName.IsEmpty())
		return;

	//	Local ports are never deleted or expire, so we don't have to worry about them.

	CMessagePort *pBinding = new CMessagePort(*this, sPort);
	pBinding->SetPort(pPort, false);
	m_EngineBinding.Insert(pBinding);

	//	Add to the splitter for this port name.

	int iPos;
	CMessagePortSplitter *pSplitter;
	if (m_LocalPorts.FindPos(sPortName, &iPos))
		pSplitter = m_LocalPorts.GetValue(iPos);
	else
		{
		pSplitter = new CMessagePortSplitter;
		m_LocalPorts.Insert(sPortName, pSplitter);
		}

	pSplitter->AddPort(pBinding);
	}

void CMessageTransporter::AddVirtualPort (const CString &sPort, const CString &sAddress, DWORD dwFlags)

//	AddVirtualPort
//
//	Adds a virtual port mapping

	{
	ASSERT(!sPort.IsEmpty());
	ASSERT(!sAddress.IsEmpty());

	//	Parse the address so that we remember whether it is local or not

	CString sProcess;
	CString sMachine;
	bool bVirtual;
	if (!ParseAddress(sAddress, NULL, &sProcess, &sMachine, &bVirtual))
		{
		ASSERT(false);
		return;
		}

	ASSERT(!bVirtual);
	if (bVirtual)
		return;

	//	Get the list of addresses associated with this virtual port

	int iPos;
	CVirtualPortArray *pPortList;
	if (m_VirtualPorts.FindPos(sPort, &iPos))
		pPortList = &m_VirtualPorts[iPos];
	else
		pPortList = m_VirtualPorts.Insert(sPort);

	//	Add it

	SVirtualPortEntry *pEntry = pPortList->Insert();
	pEntry->sAddress = sAddress;
	pEntry->dwFlags = dwFlags;

	//	If this is on the local machine, set the local machine flag

	if (IsLocalMachine(sMachine))
		{
		pEntry->dwFlags |= FLAG_PORT_LOCAL_MACHINE;

		//	If this is on the local modue, set the local module flag

		if (IsLocalProcess(sProcess))
			pEntry->dwFlags |= FLAG_PORT_LOCAL_MODULE;
		}

	//	Done
	}

CMessagePort *CMessageTransporter::Bind (const CString &sAddress)

//	Bind
//
//	Binds an address to a message port interface. Returns NULL if we could
//	not bind properly. We continue to own the pointer, so callers do not (and
//	should not) free the result.

	{
	CSmartLock Lock(m_cs);

	//	See if we already have this port. If not, we need to add it.

	CMessagePort *pPort;
	if (!m_Ports.Find(sAddress, &pPort))
		{
		//	Get the raw port

		bool bFree;
		IArchonMessagePort *pRawPort = BindRaw(sAddress, &bFree);

		//	If we could not bind, then we fail.

		if (pRawPort == NULL)
			return NULL;

		//	Otherwise, add our new port

		pPort = new CMessagePort(*this, sAddress);
		pPort->SetPort(pRawPort, bFree);

		m_Ports.Insert(sAddress, pPort);
		}

	return pPort;
	}

IArchonMessagePort *CMessageTransporter::BindRaw (const CString &sAddress, bool *retbFree)

//	BindRaw
//
//	Binds an address to a message port interface. Returns NULL if we could
//	not bind properly.

	{
	CSmartLock Lock(m_cs);

	//	We start by parsing the address.

	CString sPort;
	CString sProcess;
	CString sMachine;
	bool bVirtual;
	if (!ParseAddress(sAddress, &sPort, &sProcess, &sMachine, &bVirtual))
		return NULL;

	//	If this is a virtual port, then we need to process it as such

	if (bVirtual)
		{
		*retbFree = true;
		return CreateVirtualPortBinding(sAddress);
		}

	//	If this is on a different machine, then we need to generate
	//	a remote machine transporter.

	else if (!IsLocalMachine(sMachine))
		{
		//	Create a new intermachine port

		*retbFree = true;
		return new CIntermachinePort(m_pProcess, sMachine, sAddress);
		}

	//	Otherwise, if this is on another process we need to generate
	//	a remote process transporter.

	else if (!IsLocalProcess(sProcess))
		{
		CString sKey = strPattern("%s-%s", m_pProcess->GetMachineName(), sProcess);

		//	Look for the queue for this process

		CInterprocessMessageQueue *pQueue;
		if (!m_ProcessQueues.Find(sKey, &pQueue))
			{
			pQueue = new CInterprocessMessageQueue;
			pQueue->Open(m_pProcess, m_pProcess->GetMachineName(), sProcess);
			m_ProcessQueues.Insert(sKey, pQueue);
			}

		//	Create a new foreign port proxy (we know this is not in the list
		//	already because it would have been found in the cache)

		*retbFree = true;
		return new CInterprocessPort(m_pProcess, pQueue, sAddress);
		}

	//	Otherwise we just look up the port

	else
		{
		IArchonMessagePort *pPort;

		if (!m_LocalPorts.Find(sPort, (CMessagePortSplitter **)&pPort))
			return NULL;

		*retbFree = false;
		return pPort;
		}
	}

void CMessageTransporter::Boot (IArchonProcessCtx *pProcess)

//	Boot
//
//	Boots the transporter

	{
	m_pProcess = pProcess;
	}

CString CMessageTransporter::GenerateAbsoluteAddress (const CString &sAddress)

//	GenerateAbsoluteAddress
//
//	Converts an address to an absolute address

	{
	//	OK if address is null.

	if (sAddress.IsEmpty())
		return sAddress;

	CString sPort;
	CString sProcess;
	CString sMachine;
	if (!ParseAddress(sAddress, &sPort, &sProcess, &sMachine))
		{
		ASSERT(false);
		return sAddress;
		}

	if (IsLocalProcess(sProcess))
		sProcess = m_pProcess->GetModuleName();

	if (IsLocalMachine(sMachine))
		sMachine = m_pProcess->GetMachineName();

	return GenerateAddress(sPort, sProcess, sMachine);
	}

CString CMessageTransporter::GenerateAddress (const CString &sPort, const CString &sProcessName, const CString &sMachineName)

//	GenerateAddress
//
//	Creates an address string

	{
	ASSERT(!sPort.IsEmpty());

	if (!sMachineName.IsEmpty())
		{
		if (!sProcessName.IsEmpty())
			return strPattern("%s@%s/%s", sPort, sMachineName, sProcessName);
		else
			return strPattern("%s@%s/CentralModule", sPort, sMachineName);
		}
	else
		{
		if (!sProcessName.IsEmpty())
			return strPattern("%s@~/%s", sPort, sProcessName);
		else
			return strPattern("%s@~/~", sPort);
		}
	}

CString CMessageTransporter::GenerateMachineAddress (const CString &sMachineName, const CString &sAddress)

//	GenerateMachineAddress
//
//	Converts the given address to an absolute address for the target machine.

	{
	int i;

	//	Make sure that the machine is known to us.

	TArray<CString> Machines;
	m_pProcess->MnemosynthReadCollection(MNEMO_ARC_MACHINES, &Machines);
	CString sCanonicalName;

	//	If we're looking for ArcologyPrime, then find it.

	if (strEqualsNoCase(sMachineName, STR_ARCOLOGY_PRIME))
		{
		for (i = 0; i < Machines.GetCount(); i++)
			{
			CDatum dMachineInfo = m_pProcess->MnemosynthRead(MNEMO_ARC_MACHINES, Machines[i]);
			const CString &sName = dMachineInfo.GetElement(FIELD_NAME);
			if (strEqualsNoCase(sName, STR_ARCOLOGY_PRIME))
				{
				sCanonicalName = Machines[i];
				break;
				}
			}

		if (sCanonicalName.IsEmpty())
			sCanonicalName = STR_LOCAL_SYMBOL;
		}

	//	Otherwise, we find it by canonical name

	else
		{
		for (i = 0; i < Machines.GetCount(); i++)
			if (strEqualsNoCase(Machines[i], sMachineName))
				{
				sCanonicalName = Machines[i];
				break;
				}
		}

	if (sCanonicalName.IsEmpty())
		return NULL_STR;

	//	Generate the address

	CString sPort;
	CString sProcess;
	CString sMachine;
	if (!ParseAddress(sAddress, &sPort, &sProcess, &sMachine))
		{
		ASSERT(false);
		return NULL_STR;
		}

	if (IsLocalProcess(sProcess))
		sProcess = m_pProcess->GetModuleName();

	return GenerateAddress(sPort, sProcess, sCanonicalName);
	}

TArray<CString> CMessageTransporter::GetArcologyPortAddresses (const CString &sPortToFind) const

//	GetArcologyPortAddresses
//
//	Returns an array of addresses for all ports of the given name in the entire
//	arcology.

	{
	TArray<CString> Result;

	//	Get the list of ports by module

	TArray<CString> Modules;
	m_pProcess->MnemosynthReadCollection(MNEMO_ARC_PORTS, &Modules);

	//	Loop over all modules in the arcology and find the specific port.

	for (int i = 0; i < Modules.GetCount(); i++)
		{
		CDatum dPortList = m_pProcess->MnemosynthRead(MNEMO_ARC_PORTS, Modules[i]);

		for (int j = 0; j < dPortList.GetCount(); j++)
			{
			CDatum dPortMapping = dPortList.GetElement(j);
			const CString &sPort = dPortMapping.GetElement(0);
			if (strEquals(sPort, sPortToFind))
				{
				const CString &sAddress = dPortMapping.GetElement(1);
				Result.Insert(sAddress);
				}
			}
		}

	//	Done

	return Result;
	}

TArray<CMessagePort *> CMessageTransporter::GetPortCacheList (void) const

//	GetPortCacheList
//
//	Returns a list of the ports in the cache.
//	Since CMessagePort structures never get deleted, and since they have their
//	own lock, it is safe to return this list.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	We lock to generate the list because other threads might be adding to it.

	TArray<CMessagePort *> Result;
	Result.InsertEmpty(m_Ports.GetCount());
	for (i = 0; i < m_Ports.GetCount(); i++)
		Result[i] = m_Ports[i];

	//	Done

	return Result;
	}

IArchonMessagePort *CMessageTransporter::CreateVirtualPortBinding (const CString &sAddress)

//	CreateVirtualPortBinding
//
//	Looks up the virtual port in Mnemosynth and returns the appropriate binding
//	(Caller is responsible for freeing the object)

	{
	int i, j;

	//	Allocate a new splitter, which will hold all the ports

	CMessagePortSplitter *pSplitter = new CMessagePortSplitter;

	//	Cache our local module and machine

	const CString &sMachineName = m_pProcess->GetMachineName();
	const CString &sModuleName = m_pProcess->GetModuleName();

	//	Get the list of ports by module

	TArray<CString> Modules;
	m_pProcess->MnemosynthReadCollection(MNEMO_ARC_PORTS, &Modules);

	//	Iterate of Mnemosynth's port list

	TArray<CString> RandomList;
	CString sNearest;
	DWORD dwNearestFlags = 0;

	for (i = 0; i < Modules.GetCount(); i++)
		{
		CDatum dPortList = m_pProcess->MnemosynthRead(MNEMO_ARC_PORTS, Modules[i]);

		for (j = 0; j < dPortList.GetCount(); j++)
			{
			CDatum dPortMapping = dPortList.GetElement(j);

			const CString &sPort = dPortMapping.GetElement(0);
			if (strEquals(sPort, sAddress))
				{
				const CString &sDestAddr = dPortMapping.GetElement(1);
				DWORD dwFlags = (DWORD)(int)dPortMapping.GetElement(2);

				//	If we always add the port, then add it

				if (dwFlags & FLAG_PORT_ALWAYS)
					{
					CMessagePort *pDestPort = Bind(sDestAddr);
					if (pDestPort)
						pSplitter->AddPort(pDestPort);
					}

				//	Otherwise, if this is only randomly added, add it to the list

				else if (dwFlags & FLAG_PORT_RANDOM)
					RandomList.Insert(sDestAddr);

				//	Otherwise, we need to parse the address to figure out how
				//	close we are to the destination

				else
					{
					//	Parse the address

					CString sDestMachine;
					CString sDestModule;
					if (!ParseAddress(sDestAddr, NULL, &sDestModule, &sDestMachine))
						continue;

					//	Add some flags based on our relationship to the dest port

					if (IsLocalMachine(sDestMachine))
						{
						dwFlags |= FLAG_PORT_LOCAL_MACHINE;

						if (IsLocalProcess(sDestModule))
							dwFlags |= FLAG_PORT_LOCAL_MODULE;
						}

					//	If we always send to within the same module

					if (dwFlags & FLAG_PORT_ALWAYS_MODULE)
						{
						if (dwFlags & FLAG_PORT_LOCAL_MODULE)
							{
							CMessagePort *pDestPort = Bind(sDestAddr);
							if (pDestPort)
								pSplitter->AddPort(pDestPort);
							}
						}

					//	Otherwise, if we always send to within the same machine

					else if (dwFlags & FLAG_PORT_ALWAYS_MACHINE)
						{
						if (dwFlags & FLAG_PORT_LOCAL_MACHINE)
							{
							CMessagePort *pDestPort = Bind(sDestAddr);
							if (pDestPort)
								pSplitter->AddPort(pDestPort);
							}
						}

					//	Otherwise, we assume FLAG_PORT_NEAREST

					else
						{
						//	If we don't already have a nearest port, then this is it

						if (sNearest.IsEmpty())
							{
							sNearest = sDestAddr;
							dwNearestFlags = dwFlags;
							}

						//	If the nearest is already in the local module, then we don't need
						//	to do anything.

						else if (dwNearestFlags & FLAG_PORT_LOCAL_MODULE)
							;

						//	If the nearest is in the local machine and this dest port is in
						//	the local process, then take it

						else if (dwNearestFlags & FLAG_PORT_LOCAL_MACHINE)
							{
							if (dwFlags & FLAG_PORT_LOCAL_MODULE)
								{
								sNearest = sDestAddr;
								dwNearestFlags = dwFlags;
								}
							}

						//	If the nearest is not on the local machine, then any port that is
						//	on the local machine is closer.
						//	
						//	(By definition, anything in the local process is also in the local
						//	machine.)

						else
							{
							if (dwFlags & FLAG_PORT_LOCAL_MACHINE)
								{
								sNearest = sDestAddr;
								dwNearestFlags = dwFlags;
								}
							}
						}
					}
				}
			}
		}

	//	Add the nearest port

	if (!sNearest.IsEmpty())
		{
		CMessagePort *pDestPort = Bind(sNearest);
		if (pDestPort)
			pSplitter->AddPort(pDestPort);
		}

	//	Add the random port

	if (RandomList.GetCount() > 0)
		{
		CMessagePort *pDestPort = Bind(RandomList.Random());
		if (pDestPort)
			pSplitter->AddPort(pDestPort);
		}

	//	Done
	//	NOTE: It is OK if pSplitter has no ports.

	return pSplitter;
	}

CDatum CMessageTransporter::GetVirtualPortList (void)

//	GetVirtualPortList
//
//	Returns a datum encoding all the local virtual ports defined by this
//	module.
//
//	Each element in the array is itself an array with 3 elements:
//
//	0 = port name
//	1 = address (absolute)
//	2 = flags

	{
	int i, j;

	CComplexArray *pList = new CComplexArray;

	for (i = 0; i < m_VirtualPorts.GetCount(); i++)
		{
		CVirtualPortArray *pArray = &m_VirtualPorts[i];
		for (j = 0; j < pArray->GetCount(); j++)
			{
			SVirtualPortEntry *pEntry = &pArray->GetAt(j);

			if (pEntry->dwFlags & FLAG_PORT_LOCAL_MODULE)
				{
				//	Get rid of the location flags because they will not be the same
				//	when we cross modules

				DWORD dwFlags = pEntry->dwFlags & ~FLAG_PORT_LOCATION_MASK;

				//	Add an entry

				CComplexArray *pItem = new CComplexArray;
				pItem->Insert(m_VirtualPorts.GetKey(i));
				pItem->Insert(GenerateAbsoluteAddress(pEntry->sAddress));
				pItem->Insert((int)dwFlags);

				pList->Insert(CDatum(pItem));
				}
			}
		}

	//	Done

	return CDatum(pList);
	}

bool CMessageTransporter::IsLocalMachine (const CString &sMachineName)

//	IsLocalMachine
//
//	Returns TRUE if sMachineName is the local machine

	{
	return (sMachineName.IsEmpty() 
			|| strEquals(sMachineName, STR_LOCAL_SYMBOL)
			|| strEquals(sMachineName, STR_LOCALHOST_MACHINE_NAME)
			|| strEquals(sMachineName, m_pProcess->GetMachineName()));
	}

bool CMessageTransporter::IsLocalProcess (const CString &sProcessName)

//	IsLocalProcess
//
//	Returns TRUE if sProcessName is the local process

	{
	return (sProcessName.IsEmpty()
				|| strEquals(sProcessName, STR_LOCAL_SYMBOL)
				|| strEquals(sProcessName, m_pProcess->GetModuleName()));
	}

void CMessageTransporter::OnModuleDeleted (const CString &sName)

//	OnModuleDeleted
//
//	The given module (machine/module) has been deleted. Remove all ports from it.

	{
	CSmartLock Lock(m_cs);
	int i;

	m_pProcess->GetMnemosynth().Delete(MNEMO_ARC_PORTS, sName);

	//	We need to delete all ports for the given module, but for now we just delete
	//	all ports (this event happens rarely, so we don't care about performance).
	//
	//	NOTE: We can't lock any of the CMessagePorts while we've got a lock on 
	//	Transporter (otherwise we'd have a lock inversion) so we make a copy of the
	//	port list and then unlock.
	//
	//	This will work since we never delete CMessagePorts (we can't since we don't
	//	know how long people hold on to them).

	TArray<CMessagePort *> Ports;
	for (i = 0; i < m_Ports.GetCount(); i++)
		Ports.Insert(m_Ports[i]);

	Lock.Unlock();

	//	Now loop over all ports and clear out the raw port so that we bind then 
	//	next time we use it.

	for (i = 0; i < Ports.GetCount(); i++)
		Ports[i]->InvalidatePort();
	}

bool CMessageTransporter::ParseAddress (const CString &sAddress, CString *retsPort, CString *retsProcessName, CString *retsMachineName, bool *retbVirtual)

//	ParseAddress
//
//	Parses an address
//
//	if retsProcessName is returned as blank, then it means the current process
//	if retsMachineName is returned as blank, then it means the current machine.
//
//	Returns FALSE if there is a parsing error.

	{
	char *pPos = sAddress.GetParsePointer();

	//	Get the port name

	char *pStart = pPos;
	while (*pPos != '@' && *pPos != '\0')
		pPos++;

	if (retsPort)
		{
		*retsPort = CString(pStart, pPos - pStart);
		if (retsPort->IsEmpty())
			return false;
		}

	//	See if we have a physical address. If not, then this is a
	//	virtual port.

	if (*pPos != '@')
		{
		if (retbVirtual)
			*retbVirtual = true;

		if (retsMachineName)
			*retsMachineName = NULL_STR;

		if (retsProcessName)
			*retsProcessName = NULL_STR;

		return true;
		}

	//	If we get this far, we know we have a physical address

	pPos++;
	if (retbVirtual)
		*retbVirtual = false;

	//	Get the machine name

	pStart = pPos;
	while (*pPos != '/' && *pPos != '\0')
		pPos++;

	if (retsMachineName)
		*retsMachineName = CString(pStart, pPos - pStart);

	if (*pPos == '/')
		pPos++;

	//	Get the process name

	if (retsProcessName)
		*retsProcessName = CString(pPos);

	//	Done

	return true;
	}

CString CMessageTransporter::ParseAddressPort (const CString &sAddress)

//	ParseAddressPort
//
//	Returns the port of the given address (or NULL_STR)

	{
	CString sPort;
	if (!ParseAddress(sAddress, &sPort, NULL, NULL))
		return NULL_STR;

	return sPort;
	}

void CMessageTransporter::PublishToMnemosynth (void)

//	PublishToMnemosynth
//
//	Publishes this module's virtual ports to Mnemosynth

	{
	CString sEntryName = CMnemosynthDb::GenerateEndpointName(m_pProcess->GetMachineName(), m_pProcess->GetModuleName());
	m_pProcess->MnemosynthWrite(MNEMO_ARC_PORTS, sEntryName, GetVirtualPortList());
	}
