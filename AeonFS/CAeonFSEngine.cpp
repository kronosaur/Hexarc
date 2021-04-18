//	CAeonFSEngine.cpp
//
//	CAeonFSEngine class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEONFS_COMMAND,			"AeonFS.command@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_AEONFS,				"AeonFS")

DECLARE_CONST_STRING(PORT_AEONFS_COMMAND,				"AeonFS.command")

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")

CAeonFSEngine::SMessageHandler CAeonFSEngine::m_MsgHandlerList[] =
	{
		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,				&CAeonFSEngine::MsgHousekeeping },
	};

int CAeonFSEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CAeonFSEngine::m_MsgHandlerList);

CAeonFSEngine::CAeonFSEngine (void) : TSimpleEngine(ENGINE_NAME_AEONFS, 3)

//	CAeonEngine constructor

	{
	}

void CAeonFSEngine::MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgHousekeeping
//
//	Arc.housekeeping

	{
	}

void CAeonFSEngine::OnBoot (void)

//	OnBoot
//
//	Boot the engine

	{
	//	Register our ports

	AddPort(ADDRESS_AEONFS_COMMAND);
	AddVirtualPort(PORT_AEONFS_COMMAND, ADDRESS_AEONFS_COMMAND, FLAG_PORT_NEAREST);
	}

void CAeonFSEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark datums for garbage collection (we can also use this
//	opportunity to garbage collect our own stuff.

	{
	}

void CAeonFSEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Start running

	{
	}

void CAeonFSEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Stop running

	{
#ifdef DEBUG
	printf("AeonFS terminated.\n");
#endif
	}
