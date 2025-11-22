//	CIntermachinePort.cpp
//
//	CIntermachinePort class
//	Copyright (c) 2015 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule");

DECLARE_CONST_STRING(FIELD_CLASS,						"class");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");

DECLARE_CONST_STRING(MSG_EXARCH_SEND_TO_MACHINE,		"Exarch.sendToMachine");

DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_PREFIX,					"Log.");

DECLARE_CONST_STRING(STR_INTERMACHINE_PORT,				"CIntermachinePort");

DECLARE_CONST_STRING(ERR_UNABLE_TO_BIND_TO_EXARCH,		"Unable to bind to Exarch.");

CDatum CIntermachinePort::GetPortStatus (void) const

//	GetPortStatus
//
//	Returns a struct with well-known fields for status.

	{
	CDatum dStatus = CDatum(CDatum::typeStruct);
	dStatus.SetElement(FIELD_CLASS, STR_INTERMACHINE_PORT);
	dStatus.SetElement(FIELD_STATUS, NULL_STR);
	return dStatus;
	}

bool CIntermachinePort::SendMessage (const SArchonMessage &Msg)

//	SendMessage
//
//	Sends a message to this foreign port

	{
#ifdef DEBUG_INTERMACHINE_PORT
	if (!strStartsWith(Msg.sMsg, MSG_LOG_PREFIX))
		m_pProcess->Log(MSG_LOG_DEBUG, strPattern("Sending %s to %s.", Msg.sMsg, m_sAddr));
#endif
	//	We send this to our local machine's Exarch engine

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Append(m_sMachine);
	pPayload->Append(m_sAddr);
	pPayload->Append(Msg.sMsg);
	pPayload->Append(Msg.dwTicket);
	pPayload->Append(m_pProcess->GenerateAbsoluteAddress(Msg.sReplyAddr));
	pPayload->Append(Msg.dPayload);
	pPayload->Append(m_pProcess->GetMachineName());
	pPayload->Append(strStartsWith(Msg.sMsg, MSG_LOG_PREFIX) ? CDatum(true) : CDatum());

	SArchonMessage ExarchMsg;
	ExarchMsg.sMsg = MSG_EXARCH_SEND_TO_MACHINE;
	ExarchMsg.dwTicket = 0;
	ExarchMsg.dPayload = CDatum(pPayload);

	//	Send

	return m_pProcess->SendMessage(ADDRESS_EXARCH_COMMAND, ExarchMsg);
	}
