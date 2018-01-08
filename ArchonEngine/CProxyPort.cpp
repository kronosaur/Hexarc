//	CProxyPort.cpp
//
//	CProxyPort class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CLASS,						"class")
DECLARE_CONST_STRING(FIELD_STATUS,						"status")

DECLARE_CONST_STRING(STR_PROXY_PORT,					"CProxyPort")

CProxyPort::CProxyPort (const CString &sMsg, CMessagePort *pPort) :
		m_sMsg(sMsg),
		m_pPort(pPort)

//	CProxyPort constructor

	{
	ASSERT(m_pPort);
	}

CDatum CProxyPort::GetPortStatus (void) const

//	GetPortStatus
//
//	Returns a struct with well-known fields for status.

	{
	CDatum dStatus = CDatum(CDatum::typeStruct);
	dStatus.SetElement(FIELD_CLASS, STR_PROXY_PORT);
	dStatus.SetElement(FIELD_STATUS, NULL_STR);
	return dStatus;
	}

bool CProxyPort::SendMessage (const SArchonMessage &Msg)

//	SendMessage
//
//	Sends a message to this virtual port

	{
	//	We encode the message and payload

	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(Msg.sMsg);
	pArray->Insert(Msg.dPayload);

	//	Create a message

	SArchonMessage VirtMsg;
	VirtMsg.sMsg = m_sMsg;
	VirtMsg.sReplyAddr = Msg.sReplyAddr;
	VirtMsg.dwTicket = Msg.dwTicket;
	VirtMsg.dPayload = CDatum(pArray);

	//	Send

	return m_pPort->SendMessage(VirtMsg);
	}
