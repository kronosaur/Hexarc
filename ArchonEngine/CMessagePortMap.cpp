//	CMessagePortMap.cpp
//
//	CMessagePortMap class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

CMessagePortMap::CMessagePortMap (void) : m_bPortCountValid(false)

//	CMessagePortMap contructor

	{
	}

void CMessagePortMap::AddPort (LPSTR sName, IArchonMessagePort *pPort, DWORD dwFlags)

//	AddPort
//
//	Adds a port to the map

	{
	//	If a port is processOnly then it is also machineOnly (i.e., it cannot be
	//	seen outside the machine). We do this so that IsVisible can be a little
	//	more efficient.

	if (dwFlags & flagProcessOnly)
		dwFlags |= flagMachineOnly;

	//	Add the port

	SPortDesc *pDesc = m_PortList.Insert(sName);
	pDesc->iCount = -1;
	pDesc->sPortName = sName;
	pDesc->pPort = pPort;
	pDesc->dwFlags = dwFlags;

	//	Port count is no longer valid (we need to fix it up before we search for ports)

	m_bPortCountValid = false;
	}

void CMessagePortMap::FixupPortCount (void)

//	FixupPortCount
//
//	Makes sure that the iCount parameter in the port descriptor is accurate

	{
	if (!m_bPortCountValid)
		{
		int i;

		int iCount = 0;
		CString sCurPort = NULL_STR;
		SPortDesc *pFirst = NULL;

		for (i = 0; i < m_PortList.GetCount(); i++)
			{
			SPortDesc &Desc = m_PortList.GetValue(i);
			if (!strEquals(sCurPort, Desc.sPortName))
				{
				if (pFirst)
					pFirst->iCount = iCount;

				sCurPort = Desc.sPortName;
				pFirst = &Desc;
				iCount = 1;
				}
			else
				Desc.iCount = -1;
			}

		m_bPortCountValid = true;
		}
	}

void CMessagePortMap::GetPorts (LPSTR sName, DWORD dwFlags, TArray<IArchonMessagePort *> *retResult)

//	GetPorts
//
//	Returns a list of ports that match the given name and flags

	{
	int i;

	FixupPortCount();

	//	Find the first port with the given name

	SPortDesc *pDesc = m_PortList.GetAt(sName);
	int iCount = pDesc->iCount;
	for (i = 0; i < iCount; i++)
		{
		retResult->Insert(pDesc->pPort);
		pDesc++;
		}
	}

bool CMessagePortMap::IsVisible (SPortDesc *pDesc, DWORD dwFlags)

//	IsVisible
//
//	Returns TRUE if the given port is visible to the given scope

	{
	//	If we want ports available to the process, then we can return
	//	all ports.

	if (dwFlags & flagProcessWidePorts)
		return true;

	//	If we want ports available to any process on the machine then
	//	we return all ports that aren't processOnly.

	else if (dwFlags & flagMachineWidePorts)
		return ((pDesc->dwFlags & flagProcessOnly) ? false : true);

	//	Otherwise, we're looking for global ports. We return any port
	//	that isn't machineOnly (in AddPort we make sure that all processOnly
	//	ports are also machineOnly).

	else
		return ((pDesc->dwFlags & flagMachineOnly) ? false : true);
	}
