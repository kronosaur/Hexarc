//	HexarcMsg.cpp
//
//	HexarcMsg functions
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command")
DECLARE_CONST_STRING(ADDR_CODE_COMMAND,					"Code.command")
DECLARE_CONST_STRING(ADDR_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")
DECLARE_CONST_STRING(ADDR_DRHOUSE_COMMAND,				"Diagnostics.command")
DECLARE_CONST_STRING(ADDR_ESPER_COMMAND,				"Esper.command")
DECLARE_CONST_STRING(ADDR_EXARCH_COMMAND,				"Exarch.command@~/CentralModule")
DECLARE_CONST_STRING(ADDR_HEXE_COMMAND,					"Hexe.command")
DECLARE_CONST_STRING(ADDR_HYPERION_COMMAND,				"Hyperion.command")

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_AEON_DELETE,					"Aeon.delete")
DECLARE_CONST_STRING(MSG_AEON_DELETE_TABLE,				"Aeon.deleteTable")
DECLARE_CONST_STRING(MSG_AEON_DELETE_VIEW,				"Aeon.deleteView")
DECLARE_CONST_STRING(MSG_AEON_FILE_DIRECTORY,			"Aeon.fileDirectory")
DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD,			"Aeon.fileDownload")
DECLARE_CONST_STRING(MSG_AEON_FILE_GET_DESC,			"Aeon.fileGetDesc")
DECLARE_CONST_STRING(MSG_AEON_FILE_UPLOAD,				"Aeon.fileUpload")
DECLARE_CONST_STRING(MSG_AEON_FLUSH_DB,					"Aeon.flushDb")
DECLARE_CONST_STRING(MSG_AEON_GET_KEY_RANGE,			"Aeon.getKeyRange")
DECLARE_CONST_STRING(MSG_AEON_GET_MORE_ROWS,			"Aeon.getMoreRows")
DECLARE_CONST_STRING(MSG_AEON_GET_ROWS,					"Aeon.getRows")
DECLARE_CONST_STRING(MSG_AEON_GET_TABLES,				"Aeon.getTables")
DECLARE_CONST_STRING(MSG_AEON_GET_DATA,					"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_GET_VIEW_INFO,			"Aeon.getViewInfo")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_AEON_INSERT_NEW,				"Aeon.insertNew")
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate")
DECLARE_CONST_STRING(MSG_ARC_SANDBOX_MSG,				"Arc.sandboxMsg")
DECLARE_CONST_STRING(MSG_CODE_FIRE_EVENT,				"Code.fireEvent")
DECLARE_CONST_STRING(MSG_CODE_GET_USER_DESC,			"Code.getUserDesc")
DECLARE_CONST_STRING(MSG_CODE_GET_VIEW,					"Code.getView")
DECLARE_CONST_STRING(MSG_CODE_LIST_RUNNING,				"Code.listRunning")
DECLARE_CONST_STRING(MSG_CODE_MANDELBROT,				"Code.mandelbrot")
DECLARE_CONST_STRING(MSG_CODE_RUN,						"Code.run")
DECLARE_CONST_STRING(MSG_CODE_STATUS,					"Code.status")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ADD_RIGHTS,			"Cryptosaur.addRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CHANGE_PASSWORD,	"Cryptosaur.changePassword")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_SCOPED_CREDENTIALS,	"Cryptosaur.createScopedCredentials")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_USER,		"Cryptosaur.createUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_GET_KEY,			"Cryptosaur.getKey")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_GET_USER,			"Cryptosaur.getUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_HAS_RIGHTS,			"Cryptosaur.hasRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_LIST_KEYS,			"Cryptosaur.listKeys")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_LOGIN_USER,			"Cryptosaur.loginUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_REMOVE_RIGHTS,		"Cryptosaur.removeRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_REQUEST_LOGIN,		"Cryptosaur.requestLogin")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_RESET_PASSWORD_MANUAL,		"Cryptosaur.resetPasswordManual")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_SET_CERTIFICATE,	"Cryptosaur.setCertificate")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_SET_KEY,			"Cryptosaur.setKey")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_SIGN_DATA,			"Cryptosaur.signData")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_VALIDATE_AUTH_TOKEN,"Cryptosaur.validateAuthToken")
DECLARE_CONST_STRING(MSG_DIAGNOSTICS_CREATE_LOG_SEARCH,	"Diagnostics.createLogSearch")
DECLARE_CONST_STRING(MSG_DIAGNOSTICS_GET_LOG_SEARCH,	"Diagnostics.getLogSearch")
DECLARE_CONST_STRING(MSG_DIAGNOSTICS_PORT_CACHE_DUMP,	"Diagnostics.portCacheDump")
DECLARE_CONST_STRING(MSG_DRHOUSE_CREATE_TEST_TABLE,		"DrHouse.createTestTable")
DECLARE_CONST_STRING(MSG_DRHOUSE_UNIT_TEST,				"DrHouse.unitTest")
DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_UNKNOWN,					"Error.unknownMessage")
DECLARE_CONST_STRING(MSG_ESPER_GET_USAGE_HISTORY,		"Esper.getUsageHistory")
DECLARE_CONST_STRING(MSG_ESPER_HTTP,					"Esper.http")
DECLARE_CONST_STRING(MSG_EXARCH_ADD_MACHINE,			"Exarch.addMachine")
DECLARE_CONST_STRING(MSG_EXARCH_ADD_MODULE,				"Exarch.addModule")
DECLARE_CONST_STRING(MSG_EXARCH_ADD_VOLUME,				"Exarch.addVolume")
DECLARE_CONST_STRING(MSG_EXARCH_COMPLETE_UPGRADE,		"Exarch.completeUpgrade")
DECLARE_CONST_STRING(MSG_EXARCH_CREATE_TEST_VOLUME,		"Exarch.createTestVolume")
DECLARE_CONST_STRING(MSG_EXARCH_DELETE_TEST_DRIVE,		"Exarch.deleteTestDrive")
DECLARE_CONST_STRING(MSG_EXARCH_GET_LOG_ROWS,			"Exarch.getLogRows")
DECLARE_CONST_STRING(MSG_EXARCH_GET_MACHINE_STATUS,		"Exarch.getMachineStatus")
DECLARE_CONST_STRING(MSG_EXARCH_GET_MODULE_LIST,		"Exarch.getModuleList")
DECLARE_CONST_STRING(MSG_EXARCH_GET_STATUS,				"Exarch.getStatus")
DECLARE_CONST_STRING(MSG_EXARCH_GET_STORAGE_LIST,		"Exarch.getStorageList")
DECLARE_CONST_STRING(MSG_EXARCH_MNEMOSYNTH_ENDPOINT_LIST,	"Exarch.mnemosynthEndpointList")
DECLARE_CONST_STRING(MSG_EXARCH_MNEMOSYNTH_READ,		"Exarch.mnemosynthRead")
DECLARE_CONST_STRING(MSG_EXARCH_PING,					"Exarch.ping")
DECLARE_CONST_STRING(MSG_EXARCH_REMOVE_MACHINE,			"Exarch.removeMachine")
DECLARE_CONST_STRING(MSG_EXARCH_REMOVE_MODULE,			"Exarch.removeModule")
DECLARE_CONST_STRING(MSG_EXARCH_REMOVE_VOLUME,			"Exarch.removeVolume")
DECLARE_CONST_STRING(MSG_EXARCH_REQUEST_UPGRADE,		"Exarch.requestUpgrade")
DECLARE_CONST_STRING(MSG_EXARCH_RESTART_MACHINE,		"Exarch.restartMachine")
DECLARE_CONST_STRING(MSG_EXARCH_RESTART_MODULE,			"Exarch.restartModule")
DECLARE_CONST_STRING(MSG_EXARCH_SHUTDOWN,				"Exarch.shutdown")
DECLARE_CONST_STRING(MSG_EXARCH_UPLOAD_UPGRADE,			"Exarch.uploadUpgrade")
DECLARE_CONST_STRING(MSG_HEXE_RUN,						"Hexe.run")
DECLARE_CONST_STRING(MSG_HYPERION_GET_OPTIONS,			"Hyperion.getOptions")
DECLARE_CONST_STRING(MSG_HYPERION_GET_PACKAGE_LIST,		"Hyperion.getPackageList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_SERVICE_LIST,		"Hyperion.getServiceList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_SESSION_LIST,		"Hyperion.getSessionList")
DECLARE_CONST_STRING(MSG_HYPERION_GET_TASK_LIST,		"Hyperion.getTaskList")
DECLARE_CONST_STRING(MSG_HYPERION_REFRESH,				"Hyperion.refresh")
DECLARE_CONST_STRING(MSG_HYPERION_RESIZE_IMAGE,			"Hyperion.resizeImage")
DECLARE_CONST_STRING(MSG_HYPERION_SERVICE_MSG_SANDBOXED,"Hyperion.serviceMsgSandboxed")
DECLARE_CONST_STRING(MSG_HYPERION_SET_OPTION,			"Hyperion.setOption")
DECLARE_CONST_STRING(MSG_HYPERION_SET_TASK_RUN_ON,		"Hyperion.setTaskRunOn")
DECLARE_CONST_STRING(MSG_HYPERION_STOP_TASK,			"Hyperion.stopTask")
DECLARE_CONST_STRING(MSG_TRANSPACE_DOWNLOAD,			"Transpace.download")

DECLARE_CONST_STRING(ERR_CANNOT_INVOKE,					"You are not authorized to call Invoke")
DECLARE_CONST_STRING(ERR_UNKNOWN_MSG,					"Unknown Hexarc message: %s.")

CHexeProcess::SHexarcMsgInfo CHexeProcess::m_HexarcMsgInfo[] =
	{
		{	MSG_AEON_CREATE_TABLE,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_DELETE,				ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_DELETE_TABLE,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_DELETE_VIEW,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_FILE_DIRECTORY,		ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_FILE_DOWNLOAD,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_FILE_GET_DESC,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_FILE_UPLOAD,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_FLUSH_DB,				ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_GET_KEY_RANGE,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_GET_MORE_ROWS,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_GET_ROWS,				ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_GET_TABLES,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_GET_DATA,				ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_GET_VIEW_INFO,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_INSERT,				ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_INSERT_NEW,			ADDR_AEON_COMMAND,			0	},
		{	MSG_AEON_MUTATE,				ADDR_AEON_COMMAND,			0	},
		{	MSG_TRANSPACE_DOWNLOAD,			ADDR_AEON_COMMAND,			0	},

		{	MSG_CODE_FIRE_EVENT,			ADDR_CODE_COMMAND,			0	},
		{	MSG_CODE_GET_USER_DESC,			ADDR_CODE_COMMAND,			0	},
		{	MSG_CODE_GET_VIEW,				ADDR_CODE_COMMAND,			0	},
		{	MSG_CODE_LIST_RUNNING,			ADDR_CODE_COMMAND,			0	},
		{	MSG_CODE_MANDELBROT,			ADDR_CODE_COMMAND,			0	},
		{	MSG_CODE_RUN,					ADDR_CODE_COMMAND,			0	},
		{	MSG_CODE_STATUS,				ADDR_CODE_COMMAND,			0	},

		{	MSG_CRYPTOSAUR_ADD_RIGHTS,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_CHANGE_PASSWORD,	ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_CREATE_SCOPED_CREDENTIALS,	ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_CREATE_USER,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_GET_KEY,			ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_GET_USER,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_HAS_RIGHTS,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_LIST_KEYS,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_LOGIN_USER,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_REMOVE_RIGHTS,	ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_REQUEST_LOGIN,	ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_RESET_PASSWORD_MANUAL,	ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_SET_CERTIFICATE,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_SET_KEY,			ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_SIGN_DATA,		ADDR_CRYPTOSAUR_COMMAND,	0	},
		{	MSG_CRYPTOSAUR_VALIDATE_AUTH_TOKEN,	ADDR_CRYPTOSAUR_COMMAND,	0	},

		{	MSG_DIAGNOSTICS_CREATE_LOG_SEARCH,ADDR_DRHOUSE_COMMAND,		0	},
		{	MSG_DIAGNOSTICS_GET_LOG_SEARCH,	ADDR_DRHOUSE_COMMAND,		0	},
		{	MSG_DIAGNOSTICS_PORT_CACHE_DUMP,ADDR_DRHOUSE_COMMAND,		0	},
		{	MSG_DRHOUSE_CREATE_TEST_TABLE,	ADDR_DRHOUSE_COMMAND,		0	},
		{	MSG_DRHOUSE_UNIT_TEST,			ADDR_DRHOUSE_COMMAND,		0	},

		{	MSG_ESPER_GET_USAGE_HISTORY,	ADDR_ESPER_COMMAND,		0	},
		{	MSG_ESPER_HTTP,					ADDR_ESPER_COMMAND,		0	},

		{	MSG_EXARCH_ADD_MACHINE,			ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_ADD_MODULE,			ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_ADD_VOLUME,			ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_COMPLETE_UPGRADE,	ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_CREATE_TEST_VOLUME,	ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_DELETE_TEST_DRIVE,	ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_GET_LOG_ROWS,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_GET_MACHINE_STATUS,	ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_GET_MODULE_LIST,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_GET_STATUS,			ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_GET_STORAGE_LIST,	ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_MNEMOSYNTH_ENDPOINT_LIST,	ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_MNEMOSYNTH_READ,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_PING,				ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_REMOVE_MACHINE,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_REMOVE_MODULE,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_REMOVE_VOLUME,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_REQUEST_UPGRADE,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_RESTART_MACHINE,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_RESTART_MODULE,		ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_SHUTDOWN,			ADDR_EXARCH_COMMAND,		0	},
		{	MSG_EXARCH_UPLOAD_UPGRADE,		ADDR_EXARCH_COMMAND,		0	},

		{	MSG_HEXE_RUN,					ADDR_HEXE_COMMAND,		0	},

		{	MSG_HYPERION_GET_OPTIONS,		ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_GET_PACKAGE_LIST,	ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_GET_SERVICE_LIST,	ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_GET_SESSION_LIST,	ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_GET_TASK_LIST,		ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_REFRESH,			ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_RESIZE_IMAGE,		ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_SET_OPTION,		ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_SET_TASK_RUN_ON,	ADDR_HYPERION_COMMAND,		0	},
		{	MSG_HYPERION_STOP_TASK,			ADDR_HYPERION_COMMAND,		0	},
	};

int CHexeProcess::m_iHexarcMsgInfoCount = SIZEOF_STATIC_ARRAY(CHexeProcess::m_HexarcMsgInfo);
TSortMap<CString, int> CHexeProcess::m_HexarcMsgIndex;
TArray<CHexeProcess::SHexarcMsgPattern> CHexeProcess::m_HexarcMsgPatterns;

void CHexeProcess::AddHexarcMsgPattern (const CString &sPrefix, const CString &sAddr)

//	AddHexarcMsgPattern
//
//	Adds a pattern mapping for Hexarc messages.

	{
	int i;

	//	Replace, if duplicate

	for (i = 0; i < m_HexarcMsgPatterns.GetCount(); i++)
		if (strEquals(sPrefix, m_HexarcMsgPatterns[i].sPrefix))
			{
			m_HexarcMsgPatterns[i].sAddr = sAddr;
			return;
			}

	//	Insert

	SHexarcMsgPattern *pPattern = m_HexarcMsgPatterns.Insert();
	pPattern->sPrefix = sPrefix;
	pPattern->sAddr = sAddr;
	}

bool CHexeProcess::Boot (void)

//	Boot
//
//	Initialize

	{
	int i;

	//	Initialize the message index

	for (i = 0; i < m_iHexarcMsgInfoCount; i++)
		m_HexarcMsgIndex.Insert(m_HexarcMsgInfo[i].sMsg, i);

	return true;
	}

void CHexeProcess::ComposeHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CString *retsMsg, CDatum *retdPayload)

//	ComposeHexarcMessage
//
//	Encodes a message with a security context

	{
	//	Encode the payload

	CComplexArray *pNewPayload = new CComplexArray;
	pNewPayload->Insert(sMsg);
	pNewPayload->Insert(dPayload);
	pNewPayload->Insert(SecurityCtx.AsDatum());

	//	Always sandbox message. NOTE: We leave this to the end in case sMsg is 
	//	the same as retsMsg.

	*retsMsg = MSG_ARC_SANDBOX_MSG;
	*retdPayload = CDatum(pNewPayload);
	}

bool CHexeProcess::FindHexarcMessage (const CString &sMsg, CString *retsAddr)

//	FindHexarcMessage
//
//	Finds the message info by name

	{
	int i;

	//	Check message table

	int iPos;
	if (m_HexarcMsgIndex.Find(sMsg, &iPos))
		{
		if (retsAddr)
			*retsAddr = m_HexarcMsgInfo[iPos].sAddr;

		return true;
		}

	//	If we have an embedded plus sign then this is a service message. We 
	//	need this code because otherwise we might match a pattern below.

	const char *pPos = sMsg.GetParsePointer();
	while (*pPos != '\0')
		{
		if (*pPos == '+')
			return false;
		else
			pPos++;
		}

	//	If not found, check for pattern matches

	for (i = 0; i < m_HexarcMsgPatterns.GetCount(); i++)
		if (strStartsWith(sMsg, m_HexarcMsgPatterns[i].sPrefix))
			{
			if (retsAddr)
				*retsAddr = m_HexarcMsgPatterns[i].sAddr;

			return true;
			}

	//	Not found

	return false;
	}

bool CHexeProcess::ParseHyperionServiceMessage (const CString &sMsg, CDatum dPayload, CString &sAddr, CString &sNewMsg, CDatum &dNewPayload)

//	ParseHyperionServiceMessage
//
//	Parses a message of the form, {service}+{message} and returns its 
//	components.

	{
	const char *pPos = sMsg.GetParsePointer();
	const char *pStart = pPos;
	while (*pPos != '+' && *pPos != '\0')
		pPos++;

	if (*pPos != '+')
		return false;

	CString sService = CString(pStart, pPos - pStart);
	pPos++;

	pStart = pPos;
	while (*pPos != '\0')
		pPos++;

	CString sServiceMsg = CString (pStart, pPos - pStart);

	//	Compose the new payload

	dNewPayload = CDatum(CDatum::typeArray);
	dNewPayload.Append(sService);
	dNewPayload.Append(sServiceMsg);
	dNewPayload.Append(dPayload);

	//	We send Hyperion.serviceMsgSandboxed

	sAddr = ADDR_HYPERION_COMMAND;
	sNewMsg = MSG_HYPERION_SERVICE_MSG_SANDBOXED;

	return true;
	}

bool CHexeProcess::SendHexarcMessage (const CString &sMsg, CDatum dPayload, CDatum *retdResult)

//	SendHexarcMessage
//
//	Sends a message to the appropriate engine. This version checks to make sure
//	the caller has rights to call invoke.

	{
	CHexeSecurityCtx Ctx; 
	GetCurrentSecurityCtx(&Ctx); 
	
	//	Validate the security context

	if (!(Ctx.GetExecutionRights() & CHexeSecurityCtx::EXEC_RIGHT_INVOKE))
		{
		CHexeError::Create(MSG_ERROR_NOT_ALLOWED, ERR_CANNOT_INVOKE, retdResult);
		return false;
		}

	return SendHexarcMessage(Ctx, sMsg, dPayload, retdResult);
	}

bool CHexeProcess::SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sAddr, const CString &sMsg, CDatum dPayload, CDatum *retdResult)

//	SendHexarcMessage
//
//	Sends the given message to the appropriate engine. If the result requires
//	an asynchronous call, the function returns TRUE with retdResult initialized
//	with the async request. If the result is synchronous, the function returns FALSE
//	and retdResult is initialized to some (usually error) value.
//
//	If asynchronous, retdResult is an array with the following elements:
//
//	0: address to send to
//	1: message to send
//	2: payload

	{
	//	Encode

	CString sEncodedMsg;
	CDatum dEncodedPayload;
	ComposeHexarcMessage(SecurityCtx, sMsg, dPayload, &sEncodedMsg, &dEncodedPayload);

	//	Compose the message

	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(sAddr);
	pArray->Insert(sEncodedMsg);
	pArray->Insert(dEncodedPayload);

	//	Done

	*retdResult = CDatum(pArray);
	return true;
	}

bool CHexeProcess::SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CDatum *retdResult)

//	SendHexarcMessage
//
//	Sends the given message to the appropriate engine. If the result requires
//	an asynchronous call, the function returns TRUE with retdResult initialized
//	with the async request. If the result is synchronous, the function returns FALSE
//	and retdResult is initialized to some (usually error) value.
//
//	If asynchronous, retdResult is an array with the following elements:
//
//	0: address to send to
//	1: message to send
//	2: payload

	{
	//	If this is a standard Hexarc message, then we look up its address and
	//	send it.

	CString sAddr;
	if (FindHexarcMessage(sMsg, &sAddr))
		return SendHexarcMessage(SecurityCtx, sAddr, sMsg, dPayload, retdResult);

	//	Otherwise, see if this is a Hyperion service message

	CString sNewMsg;
	CDatum dNewPayload;
	if (ParseHyperionServiceMessage(sMsg, dPayload, sAddr, sNewMsg, dNewPayload))
		return SendHexarcMessage(SecurityCtx, sAddr, sNewMsg, dNewPayload, retdResult);

	//	Else, this is not a valid message
		
	CHexeError::Create(MSG_ERROR_UNKNOWN, strPattern(ERR_UNKNOWN_MSG, sMsg), retdResult);
	return false;
	}

bool CHexeProcess::ValidateHexarcMessage (const CString &sMsg, CDatum dPayload, CString *retsAddr, CDatum *retdResult)

//	ValidateHexarcMessage
//
//	If the message can be sent by this process then return TRUE and the address

	{
	if (!FindHexarcMessage(sMsg, retsAddr))
		{
		CHexeError::Create(MSG_ERROR_UNKNOWN, strPattern(ERR_UNKNOWN_MSG, sMsg), retdResult);
		return false;
		}

	return true;
	}
