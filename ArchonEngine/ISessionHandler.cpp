//	ISessionHandler.cpp
//
//	ISessionHandler class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ERROR_PREFIX,					"Error.")
DECLARE_CONST_STRING(MSG_SIMPLE_ENGINE_TIMEOUT,			"Error.timeout")
DECLARE_CONST_STRING(MSG_REPLY_PROGRESS,				"Reply.progress")

CDatum ISessionHandler::GetStatusReport (void) const

//	GetStatusReport
//
//	Returns a struct containing information about the session
	
	{
	CComplexStruct *pStatus = new CComplexStruct;

	//	Let our subclasses fill in

	OnGetStatusReport(pStatus);

	//	Done

	return CDatum(pStatus);
	}

bool ISessionHandler::IsError (const SArchonMessage &Msg)

//	IsError
//
//	Returns TRUE if message is an error

	{
	return strStartsWith(Msg.sMsg, MSG_ERROR_PREFIX);
	}

bool ISessionHandler::ProcessKeepAlive (const SArchonMessage &Msg)

//  ProcessKeepAlive
//
//  We received a message indicating that we should keep waiting (not time out)
//  while waiting for a command to reply.

    {
	CSmartLock Lock(m_cs);

    if (m_dwTimeoutID)
        {
        m_pEngine->KeepAliveTimedMessage(m_dwTimeoutID);
        }

    //  We always continue processing.

    return true;
    }

bool ISessionHandler::ProcessMessage (const SArchonMessage &Msg)

//	ProcessMessage
//
//	We received a response
	
	{
	CSmartLock Lock(m_cs);

	//	If we are waiting for a timeout, delete it

	if (m_dwTimeoutID)
		{
		m_pEngine->DeleteTimedMessage(m_dwTimeoutID);
		m_dwTimeoutID = 0;
		}

	//	We can unlock because all we are trying to protect is m_dwTimeoutID.

	Lock.Unlock();

	//	Let our derived class handle the message

	return OnProcessMessage(Msg);
	}

bool ISessionHandler::ProcessTimeout (const SArchonMessage &Msg)

//	ProcessTimeout
//
//	We received a timeout message

	{
	CSmartLock Lock(m_cs);

	//	If we're waiting for this timeout, then call our derived class

	if ((int)m_dwTimeoutID == (int)Msg.dPayload)
		{
		m_dwTimeoutID = 0;

		//	We can unlock because all we are trying to protect is m_dwTimeoutID.

		Lock.Unlock();

		return OnTimeout(Msg);
		}

	//	Otherwise this is a timeout for a previous message, so ignore it

	return true;
	}

void ISessionHandler::ResetTimeout (const CString &sReplyAddr, DWORD dwTimeout)

//	ResetTimeout
//
//	Resets the timeout. This should be called from an OnTimeout event.

	{
	CSmartLock Lock(m_cs);

	if (m_dwTimeoutID == 0 && dwTimeout > 0)
		m_pEngine->SendTimeoutMessage(dwTimeout, sReplyAddr, MSG_SIMPLE_ENGINE_TIMEOUT, m_dwTicket, &m_dwTimeoutID);
	}

bool ISessionHandler::SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, CDatum dPayload, DWORD dwTimeout)

//	SendMessageCommand
//
//	Sends a message that we expect a reply from
	
	{
	if (dwTimeout)
		SetTimeout(sReplyAddr, dwTimeout);

	//	Send message

	return m_pProcess->SendMessageCommand(sAddress, sMsg, sReplyAddr, m_dwTicket, dPayload);
	}

void ISessionHandler::SendMessageNotify (const CString &sAddress, const CString &sMsg, CDatum dPayload)

//	SendMessageNotify
//
//	Sends a message that we do not expect a reply from

	{
	m_pEngine->SendMessageNotify(sAddress, sMsg, dPayload);
	}

void ISessionHandler::SendMessageReply (const CString &sMsg, CDatum dData)

//	SendMessageReply
//
//	Sends a data reply

	{
	m_pProcess->SendMessageReply(sMsg, dData, m_OriginalMsg);
	}

void ISessionHandler::SendMessageReplyError (const CString &sMsg, const CString &sText)

//	SendMessageReplyError
//
//	Sends an error

	{
	m_pEngine->SendMessageReplyError(sMsg, sText, m_OriginalMsg);
	}

void ISessionHandler::SendMessageReplyProgress (const CString &sText, int iProgress)

//	SendMessageReplyProgress
//
//	Sends a progress message

	{
	//	Create a payload

	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(sText);

	if (iProgress != -1)
		pArray->Insert(iProgress);

	//	Send the message

	m_pProcess->SendMessageReply(MSG_REPLY_PROGRESS, CDatum(pArray), m_OriginalMsg);
	}

void ISessionHandler::SetTimeout (const CString &sReplyAddr, DWORD dwTimeout)

//	SetTimeout
//
//	Sets a timeout

	{
	CSmartLock Lock(m_cs);

	//	If we are waiting for a timeout, delete it.
	//	(In theory we should never get here since the derived class won't call SendMessageCommand
	//	unless it receives a reply. But if the derived class sends multiple SendMessageCommands
	//	[which should not be allowed] then this might happen.]

	if (m_dwTimeoutID)
		{
		m_pEngine->DeleteTimedMessage(m_dwTimeoutID);
		m_dwTimeoutID = 0;
		}

	//	Add a timeout message

	if (dwTimeout)
		m_pEngine->SendTimeoutMessage(dwTimeout, sReplyAddr, MSG_SIMPLE_ENGINE_TIMEOUT, m_dwTicket, &m_dwTimeoutID);
	}

void ISessionHandler::TranspaceDownload (const CString &sAddress, const CString &sReplyAddr, CDatum dDownloadDesc, const CHexeSecurityCtx *pSecurityCtx, DWORD dwTimeout)

//	TranspaceDownload
//
//	Downloads a file by Transpace address.

	{
	SetTimeout(sReplyAddr, dwTimeout);
	m_pProcess->TranspaceDownload(sAddress, sReplyAddr, m_dwTicket, dDownloadDesc, pSecurityCtx);
	}
