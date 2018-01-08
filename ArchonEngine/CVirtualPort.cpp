//	CVirtualPort.cpp
//
//	CVirtualPort class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

CVirtualPort::CVirtualPort (const CString &sMsg, IArchonMessagePort *pPort) :
		m_sMsg(sMsg),
		m_pPort(pPort)

//	CVirtualPort constructor

	{
	ASSERT(m_pPort);
	}

bool CVirtualPort::SendMessage (const SArchonMessage &Msg)

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
