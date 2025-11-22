//	CMessagePortSplitter.cpp
//
//	CMessagePortSplitter class
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CLASS,						"class");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");

DECLARE_CONST_STRING(STR_MESSAGE_PORT_SPLITTER,			"CMessagePortSplitter");
DECLARE_CONST_STRING(STR_STATUS_ONE_PORT,				"1 port");
DECLARE_CONST_STRING(STR_STATUS_MULTIPLE_PORTS,			"%d ports");

void CMessagePortSplitter::AddPort (CMessagePort *pPort)

//	AddPort
//
//	Adds a port to the splitter

	{
	m_Ports.Insert(pPort);
	}

CDatum CMessagePortSplitter::GetPortStatus (void) const

//	GetPortStatus
//
//	Returns a struct with well-known fields for status.

	{
	CDatum dStatus = CDatum(CDatum::typeStruct);
	dStatus.SetElement(FIELD_CLASS, STR_MESSAGE_PORT_SPLITTER);
	if (m_Ports.GetCount() == 1)
		dStatus.SetElement(FIELD_STATUS, STR_STATUS_ONE_PORT);
	else
		dStatus.SetElement(FIELD_STATUS, strPattern(STR_STATUS_MULTIPLE_PORTS, m_Ports.GetCount()));

	return dStatus;
	}

bool CMessagePortSplitter::SendMessage (const SArchonMessage &Msg)

//	SendMessage
//
//	Sends a message

	{
	int i;

	bool bSomeSucceeded = false;
	for (i = 0; i < m_Ports.GetCount(); i++)
		{
		if (m_Ports[i]->SendMessage(Msg))
			bSomeSucceeded = true;
		}

	return bSomeSucceeded;
	}
