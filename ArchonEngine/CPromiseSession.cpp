//	CPromiseSession.cpp
//
//	CPromiseSession class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_OK,							"OK");

CPromiseSession::CPromiseSession (TUniquePtr<SArchonPromise> &&pCode) : m_pCode(std::move(pCode))

//	CPromiseSession constructor

	{
	}

void CPromiseSession::Then (TUniquePtr<SArchonPromise> &&pCode)

//	Then
//
//	Append the given code to the end.

	{
	if (m_pCode)
		{
		SArchonPromise *pLast = m_pCode;
		while (pLast->pThen)
			pLast = pLast->pThen;

		pLast->pThen = std::move(pCode);
		}
	else
		{
		m_pCode = std::move(pCode);
		}
	}

bool CPromiseSession::HandleReply (EPromiseResult iResult, const SArchonMessage &Reply)

//	HandleReply
//
//	Handle a reply from the promise object.

	{
	switch (iResult)
		{
		//	Success!

		case EPromiseResult::OK:
			m_pCurrent = m_pCurrent->pThen;
			if (!m_pCurrent || !m_pCurrent->fnStart)
				{
				SendMessageReply(Reply.sMsg, Reply.dPayload);
				return false;
				}
			else
				{
				SArchonMessage NextReply;
				auto iNextResult = m_pCurrent->fnStart(*this, Reply, NextReply);

				return HandleReply(iNextResult, NextReply);
				}

		//	Wait for reply

		case EPromiseResult::Wait:
			return true;

		//	Error

		case EPromiseResult::Error:
			SendMessageReplyError(Reply.sMsg, Reply.dPayload);
			return false;

		default:
			throw CException(errFail);
		}
	}

void CPromiseSession::OnMark (void)

//	OnMark
//
//	Mark data in use.

	{
	SArchonPromise *pPromise = m_pCode;
	while (pPromise)
		{
		if (pPromise->fnMark)
			pPromise->fnMark();

		pPromise = pPromise->pThen;
		}
	}

bool CPromiseSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start a session.

	{
	//	Start with the first one.

	m_pCurrent = m_pCode;
	if (!m_pCurrent || !m_pCurrent->fnStart)
		{
		SendMessageReply(MSG_OK);
		return false;
		}

	//	Start

	SArchonMessage Reply;
	auto iResult = m_pCurrent->fnStart(*this, Msg, Reply);

	return HandleReply(iResult, Reply);
	}

bool CPromiseSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a message reply.

	{
	if (m_pCurrent->fnProcess)
		{
		SArchonMessage Reply;
		auto iResult = m_pCurrent->fnProcess(*this, Msg, Reply);

		return HandleReply(iResult, Reply);
		}
	else if (IsError(Msg))
		{
		SendMessageReplyError(Msg.sMsg, Msg.dPayload);
		return false;
		}
	else
		{
		SendMessageReply(Msg.sMsg, Msg.dPayload);
		return false;
		}
	}

