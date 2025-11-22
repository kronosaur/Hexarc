//	CMessageDispatchSession.cpp
//
//	CMessageDispatchSession class
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CMessageDispatchSession::CMessageDispatchSession (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, CDatum dPayload) :
		m_iState(stateNone),
		m_sAddress(sAddress),
		m_sMsg(sMsg),
		m_sReplyAddr(sReplyAddr),
		m_dPayload(dPayload)

//	CMessageDispatchSession constructor

	{
	}

void CMessageDispatchSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	m_dPayload.Mark();
	}

bool CMessageDispatchSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a message

	{
	switch (m_iState)
		{
		case stateWaitForMsg:
			{
			if (IsError(Msg))
				{
				SendMessageReplyError(Msg.sMsg, Msg.dPayload.GetElement(0).AsStringView());
				return false;
				}

			SendMessageReply(Msg.sMsg, Msg.dPayload);
			return false;
			}

		default:
			return false;
		}
	}

bool CMessageDispatchSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start

	{
	//	Send the message

	m_iState = stateWaitForMsg;
	SendMessageCommand(m_sAddress, m_sMsg, m_sReplyAddr, m_dPayload);

	//	Expect reply

	return true;
	}
