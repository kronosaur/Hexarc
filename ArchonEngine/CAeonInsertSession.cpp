//	CAeonInsertSession.cpp
//
//	CAeonInsertSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")

DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")

const DWORD MESSAGE_TIMEOUT =							30 * 1000;

CAeonInsertSession::CAeonInsertSession (const CString &sReplyAddr, const CString &sTableName, CDatum dKeyPath, CDatum dData) :
		m_sReplyAddr(sReplyAddr),
		m_sTableName(sTableName),
		m_dKeyPath(dKeyPath),
		m_dData(dData)

//	CAeonInsertSession constructor

	{
	}

void CAeonInsertSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	m_dKeyPath.Mark();
	m_dData.Mark();
	}

bool CAeonInsertSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a response

	{
	//	If we got an error back, then pass it back to the caller.

	if (IsError(Msg))
		{
		SendMessageReplyError(Msg.sMsg, Msg.dPayload.GetElement(0));
		return false;
		}

	//	Otherwise, let our subclass handle it.

	else
		{
		OnSuccess();
		return false;
		}
	}

bool CAeonInsertSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Initiate

	{
	//	Send an Aeon message to insert the user record

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_sTableName);
	pPayload->Insert(m_dKeyPath);
	pPayload->Insert(m_dData);

	//	Send message

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_INSERT,
			m_sReplyAddr,
			CDatum(pPayload),
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}
