//	CHexeEngine.cpp
//
//	CHexeEngine class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(VIRTUAL_PORT_HEXE_COMMAND,			"Hexe.command")

DECLARE_CONST_STRING(ADDRESS_HEXE_COMMAND,				"Hexe.command@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_HEXE,					"Hexe")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")
DECLARE_CONST_STRING(MSG_HEXE_HTTP,						"Hexe.http")
DECLARE_CONST_STRING(MSG_HEXE_RUN,						"Hexe.run")

CHexeEngine::SMessageHandler CHexeEngine::m_MsgHandlerList[] =
	{
		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,				&TSimpleEngine::MsgNull },

		//	Hexe.http {method} {URL} {headers} {body} {options}
		{	MSG_HEXE_HTTP,						&CHexeEngine::MsgHTTP },

		//	Hexe.run {code}
		{	MSG_HEXE_RUN,						&CHexeEngine::MsgRun },
	};

int CHexeEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CHexeEngine::m_MsgHandlerList);

CHexeEngine::CHexeEngine (void) : TSimpleEngine(ENGINE_NAME_HEXE)

//	CHexeEngine constructor

	{
	}

CHexeEngine::~CHexeEngine (void)

//	CHexeEngine destructor

	{
	}

void CHexeEngine::MsgHTTP (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgHTTP
//
//	Hexe.http {method} {URL} {headers} {body} {options}

	{
	const CString &sMethod = Msg.dPayload.GetElement(0);
	const CString &sURL = Msg.dPayload.GetElement(1);
	CDatum dHeaders = Msg.dPayload.GetElement(2);
	CDatum dBody = Msg.dPayload.GetElement(3);
	CDatum dOptions = Msg.dPayload.GetElement(4);

	CDatum dResult;
	if (!CEsperInterface::HTTP(sMethod,
			sURL,
			dHeaders,
			dBody,
			dOptions,
			&dResult))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, dResult.AsString(), Msg);
		return;
		}

	//	Done

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CHexeEngine::OnBoot (void)

//	OnBoot
//
//	Boot

	{
	//	Register our command port

	AddPort(ADDRESS_HEXE_COMMAND);
	AddVirtualPort(VIRTUAL_PORT_HEXE_COMMAND, ADDRESS_HEXE_COMMAND, FLAG_PORT_NEAREST);
	}

void CHexeEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark data in use

	{
	}

void CHexeEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Start running

	{
	}

void CHexeEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Stop running

	{
	}
