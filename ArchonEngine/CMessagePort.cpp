//	CMessagePortMap.cpp
//
//	CMessagePortMap class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null")

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address")
DECLARE_CONST_STRING(FIELD_CLASS,						"class")

DECLARE_CONST_STRING(STR_NULL_CLASS,					"(unbound)")

CDatum CMessagePort::GetStatus (void) const

//	GetStatus
//
//	Returns a status.

	{
	CSmartLock Lock(m_cs);

	CDatum dStatus;
	if (m_pPort == NULL)
		{
		dStatus = CDatum(CDatum::typeStruct);
		dStatus.SetElement(FIELD_CLASS, STR_NULL_CLASS);
		}
	else
		dStatus = m_pPort->GetPortStatus();

	dStatus.SetElement(FIELD_ADDRESS, m_sAddress);

	return dStatus;
	}

bool CMessagePort::IsNullAddr (const CString &sAddr)

//	IsNullAddr
//
//	Returns TRUE if this is a null address

	{
	return (sAddr.IsEmpty() || strEquals(sAddr, ADDR_NULL));
	}

bool CMessagePort::SendMessage (const SArchonMessage &Msg)

//	SendMessage
//
//	Sends a message

	{
	CSmartLock Lock(m_cs);

	if (m_pPort == NULL)
		{
		m_pPort = m_Transporter.BindRaw(m_sAddress, &m_bFreePort);
		if (m_pPort == NULL)
			return false;

		//	If the port we got back is not valid, then we skip it. This can 
		//	happen when a module takes too long to load. We force a bind later.

		if (!m_pPort->IsValid())
			{
			CleanUpPort();
			return false;
			}
		}

	//	Send the message

	return m_pPort->SendMessage(Msg);
	}
