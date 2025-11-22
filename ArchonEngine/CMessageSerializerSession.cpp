//	CMessageSerializerSession.cpp
//
//	CMessageSerializerSession class
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const DWORD MESSAGE_TIMEOUT =							30 * 1000;

CMessageSerializerSession::CMessageSerializerSession (void) :
		m_iPos(-1)

//	CMessageSerializerSession constructor

	{
	}

void CMessageSerializerSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		m_List[i].Msg.dPayload.Mark();
	}

bool CMessageSerializerSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle reply

	{
	//	If we got an error back, then pass it back to the caller.

	if (IsError(Msg))
		{
		SendMessageReplyError(Msg.sMsg, Msg.dPayload.GetElement(0).AsStringView());
		return false;
		}

	//	Otherwise, let our subclass handle it.

	else
		{
		OnReply(m_List[m_iPos], Msg);

		//	Next message

		if (++m_iPos < m_List.GetCount())
			{
			ISessionHandler::SendMessageCommand(m_List[m_iPos].sAddr,
					m_List[m_iPos].Msg.sMsg,
					m_List[m_iPos].Msg.sReplyAddr,
					m_List[m_iPos].Msg.dPayload,
					MESSAGE_TIMEOUT);

			return true;
			}
		else
			{
			OnComplete();
			return false;
			}
		}
	}

bool CMessageSerializerSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start

	{
	if (m_List.GetCount() == 0)
		{
		OnComplete();
		return false;
		}

	m_iPos = 0;

	//	Send message

	ISessionHandler::SendMessageCommand(m_List[0].sAddr,
			m_List[0].Msg.sMsg,
			m_List[0].Msg.sReplyAddr,
			m_List[0].Msg.dPayload,
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}
