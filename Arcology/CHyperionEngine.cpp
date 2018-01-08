//	CHyperionEngine.cpp
//
//	CHyperionEngine class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(VIRTUAL_PORT_AEON_NOTIFY,			"Aeon.notify")
DECLARE_CONST_STRING(VIRTUAL_PORT_HYPERION_COMMAND,		"Hyperion.command")

DECLARE_CONST_STRING(ADDRESS_HYPERION_COMMAND,			"Hyperion.command@~/~")

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_AEON_ON_START,					"Aeon.onStart")

CHyperionEngine::SMessageHandler CHyperionEngine::m_MsgHandlerList[] =
	{
		//	Aeon.onStart
		{	MSG_AEON_ON_START,					&CHyperionEngine::MsgAeonOnStart },
	};

int CHyperionEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CHyperionEngine::m_MsgHandlerList);

CHyperionEngine::CHyperionEngine (void)

//	CHyperionEngine constructor

	{
	}

CHyperionEngine::~CHyperionEngine (void)

//	CHyperionEngine destructor

	{
	}

void CHyperionEngine::OnBoot (void)

//	OnBoot
//
//	Boot up the engine

	{
	//	Register our command port

	AddPort(ADDRESS_HYPERION_COMMAND);
	AddVirtualPort(VIRTUAL_PORT_HYPERION_COMMAND, ADDRESS_HYPERION_COMMAND, FLAG_PORT_NEAREST);

	//	Subscribe to Aeon notifications

	AddVirtualPort(VIRTUAL_PORT_AEON_NOTIFY, ADDRESS_HYPERION_COMMAND, FLAG_PORT_ALWAYS);
	}

void CHyperionEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark data in use (and garbage collect)

	{
	}

void CHyperionEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Engine is running

	{
	}

void CHyperionEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Engine has stopped

	{
	}
