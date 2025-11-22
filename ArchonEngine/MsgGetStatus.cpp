//	MsgGetStatus.cpp
//
//	CExarchEngine class
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule")
DECLARE_CONST_STRING(VIRTUAL_PORT_ESPER_COMMAND,		"Esper.command")
DECLARE_CONST_STRING(VIRTUAL_PORT_HYPERION_COMMAND,		"Hyperion.command")

DECLARE_CONST_STRING(MSG_ARC_GET_STATUS,				"Arc.getStatus")
DECLARE_CONST_STRING(MSG_HYPERION_GET_STATUS,			"Hyperion.getStatus")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

class CStatusSession : public CMessageSerializerSession
	{
	public:
		CStatusSession (CExarchEngine *pEngine);
		~CStatusSession (void);

	protected:
		//	CMessageSerializerSession virtuals
		virtual void OnComplete (void);
		virtual void OnReply (const SArchonEnvelope &Msg, const SArchonMessage &Reply);

	private:
		CComplexStruct *m_pResult;
	};

void CExarchEngine::MsgGetStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetStatus
//
//	Exarch.getStatus

	{
	//	No need to lock because we don't change state.

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Start a sessions that will handle this

	StartSession(Msg, new CStatusSession(this));
	}

//	CStatusSession -------------------------------------------------------------

CStatusSession::CStatusSession (CExarchEngine *pEngine)

//	CStatusSession constructor

	{
	m_pResult = new CComplexStruct;

	//	Generate the list of messages to send to get statuses from other 
	//	engines.

	SArchonEnvelope Msg;
	Msg.Msg.sMsg = MSG_ARC_GET_STATUS;
	Msg.Msg.sReplyAddr = ADDRESS_EXARCH_COMMAND;

	Msg.sAddr = VIRTUAL_PORT_HYPERION_COMMAND;
	AddMessage(Msg);

	Msg.sAddr = VIRTUAL_PORT_ESPER_COMMAND;
	AddMessage(Msg);
	}

CStatusSession::~CStatusSession (void)

//	CStatusSession destructor

	{
	if (m_pResult)
		delete m_pResult;
	}

void CStatusSession::OnComplete (void)

//	OnComplete
//
//	All messages have returned successfully.

	{
	//	We're done with our results

	SendMessageReply(MSG_REPLY_DATA, CDatum(m_pResult));
	m_pResult = NULL;
	}

void CStatusSession::OnReply (const SArchonEnvelope &Msg, const SArchonMessage &Reply)

//	OnReply
//
//	We've received a reply

	{
	//	Add the reply to our result

	m_pResult->Append(Reply.dPayload);
	}
