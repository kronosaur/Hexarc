//	CDrHouseEngine.cpp
//
//	CDrHouseEngine class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(VIRTUAL_PORT_DIAGNOSTICS_COMMAND,	"Diagnostics.command")
DECLARE_CONST_STRING(ADDRESS_DRHOUSE_COMMAND,			"DrHouse.command@~/~")

DECLARE_CONST_STRING(ENGINE_NAME_DRHOUSE,				"DrHouse")

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")
DECLARE_CONST_STRING(MSG_DIAGNOSTICS_CREATE_LOG_SEARCH,	"Diagnostics.createLogSearch")
DECLARE_CONST_STRING(MSG_DIAGNOSTICS_GET_LOG_SEARCH,	"Diagnostics.getLogSearch")
DECLARE_CONST_STRING(MSG_DIAGNOSTICS_PORT_CACHE_DUMP,	"Diagnostics.portCacheDump")
DECLARE_CONST_STRING(MSG_DRHOUSE_CREATE_TEST_TABLE,		"DrHouse.createTestTable")
DECLARE_CONST_STRING(MSG_DRHOUSE_PROCESS_LOG_SEARCH,	"DrHouse.processLogSearch")
DECLARE_CONST_STRING(MSG_DRHOUSE_UNIT_TEST,				"DrHouse.unitTest")

CDrHouseEngine::SMessageHandler CDrHouseEngine::m_MsgHandlerList[] =
	{
		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,				&TSimpleEngine::MsgNull },

		//	Diagnostics.createLogSearch {search} [{options}]
		//
		//	options:
		//
		//		maxResults: max number of results
		//		startOn: only results on or after this date
		//		stopOn: only results on or before this data
		//
		{	MSG_DIAGNOSTICS_CREATE_LOG_SEARCH,	&CDrHouseEngine::MsgCreateLogSearch },

		//	Diagnostics.getLogSearch {searchID} [{start at line}]
		{	MSG_DIAGNOSTICS_GET_LOG_SEARCH,		&CDrHouseEngine::MsgGetLogSearch },

		//	Diagnostics.portCacheDump
		{	MSG_DIAGNOSTICS_PORT_CACHE_DUMP,	&CDrHouseEngine::MsgPortCacheDump },

		//	DrHouse.createTestTable [{noOfRows} [{dataSize} [{tableName}]]]
		{	MSG_DRHOUSE_CREATE_TEST_TABLE,		&CDrHouseEngine::MsgCreateTestTable },

		//	DrHouse.processLogSearch
		{	MSG_DRHOUSE_PROCESS_LOG_SEARCH,		&CDrHouseEngine::MsgProcessLogSearch },

		//	DrHouse.unitTest [{testList}] 
		{	MSG_DRHOUSE_UNIT_TEST,				&CDrHouseEngine::MsgUnitTest },
	};

int CDrHouseEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CDrHouseEngine::m_MsgHandlerList);

CDrHouseEngine::CDrHouseEngine (void) : TSimpleEngine(ENGINE_NAME_DRHOUSE)

//	CDrHouseEngine constructor

	{
	}

CDrHouseEngine::~CDrHouseEngine (void)

//	CDrHouseEngine destructor

	{
	}

void CDrHouseEngine::OnBoot (void)

//	OnBoot
//
//	Start up

	{
	CString sError;

	//	Register our command port

	AddPort(ADDRESS_DRHOUSE_COMMAND);
	AddVirtualPort(VIRTUAL_PORT_DIAGNOSTICS_COMMAND, ADDRESS_DRHOUSE_COMMAND, FLAG_PORT_NEAREST);

	//	Initialize BlackBox processor

	CString sExecutableFilespec = fileGetExecutableFilespec();
	CString sExecutablePath = fileGetPath(sExecutableFilespec);
	if (!m_BlackBoxProcessor.Init(sExecutablePath, &sError))
		Log(MSG_LOG_ERROR, sError);
	}

void CDrHouseEngine::OnStartRunning (void)

//	OnStartRunning
//
//	All engines are running

	{
	}

void CDrHouseEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Process has asked us to stop

	{
	}

