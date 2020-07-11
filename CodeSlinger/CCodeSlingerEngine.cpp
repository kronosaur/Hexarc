//	CCodeSlingerEngine.cpp
//
//	CCodeSlingerEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(PORT_AEON_NOTIFY,					"Aeon.notify")
DECLARE_CONST_STRING(PORT_CODE_COMMAND,					"Code.command")
DECLARE_CONST_STRING(ADDRESS_CODE_COMMAND,				"Code.command@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_TRANSCENDENCE,			"Code")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin")

//	Message table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_AEON_ON_START,					"Aeon.onStart")
DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")

DECLARE_CONST_STRING(MSG_CODE_STATUS,					"Code.status")

CCodeSlingerEngine::SMessageHandler CCodeSlingerEngine::m_MsgHandlerList[] =
	{
		//	Aeon.onStart
//		{	MSG_AEON_ON_START,					&CCodeSlingerEngine::MsgAeonOnStart },

		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,				&TSimpleEngine::MsgNull },

		//	Code.status
		{	MSG_CODE_STATUS,					&CCodeSlingerEngine::MsgStatus },
	};

int CCodeSlingerEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CCodeSlingerEngine::m_MsgHandlerList);

CCodeSlingerEngine::CCodeSlingerEngine (void) : TSimpleEngine(ENGINE_NAME_TRANSCENDENCE, 3)

//	CCodeSlingerEngine constructor

	{
	}

CCodeSlingerEngine::~CCodeSlingerEngine (void)

//	CCodeSlingerEngine destructor

	{
	}

void CCodeSlingerEngine::MsgStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgStatus
//
//	Luminous.status

	{
	SendMessageReply(MSG_REPLY_DATA, CString("Status OK"), Msg);
	}

void CCodeSlingerEngine::OnBoot (void)

//	OnBoot
//
//	Boot the engine

	{
	//	Register our ports

	AddPort(ADDRESS_CODE_COMMAND);
	AddVirtualPort(PORT_CODE_COMMAND, ADDRESS_CODE_COMMAND, FLAG_PORT_NEAREST);

	//	Subscribe to Aeon notifications

	AddVirtualPort(PORT_AEON_NOTIFY, ADDRESS_CODE_COMMAND, FLAG_PORT_ALWAYS);
	}

void CCodeSlingerEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark datums for garbage collection (we can also use this
//	opportunity to garbage collect our own stuff).

	{
	}

void CCodeSlingerEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Start running

	{
	}

void CCodeSlingerEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Stop running

	{
	}
