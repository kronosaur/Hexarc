//	CCodeSlingerEngine.cpp
//
//	CCodeSlingerEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(PORT_AEON_NOTIFY,					"Aeon.notify");
DECLARE_CONST_STRING(PORT_CODE_COMMAND,					"Code.command");
DECLARE_CONST_STRING(ADDRESS_CODE_COMMAND,				"Code.command@~/~");

DECLARE_CONST_STRING(ENGINE_NAME_CODE_SLINGER,			"CodeSlinger");

DECLARE_CONST_STRING(FIELD_PROGRAM_ID,					"programID");
DECLARE_CONST_STRING(FIELD_RUN_BY,						"runBy");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_OK,							"OK");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin");

DECLARE_CONST_STRING(ERR_CANT_CREATE_THREADS,			"Unable to create CodeSlinger threads.");

//	Message table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_AEON_ON_START,					"Aeon.onStart");
DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping");

DECLARE_CONST_STRING(MSG_CODE_GET_VIEW,					"Code.getView");
DECLARE_CONST_STRING(MSG_CODE_MANDELBROT,				"Code.mandelbrot");
DECLARE_CONST_STRING(MSG_CODE_MANDELBROT_TASK,			"Code.mandelbrotTask");
DECLARE_CONST_STRING(MSG_CODE_RUN,						"Code.run");
DECLARE_CONST_STRING(MSG_CODE_STATUS,					"Code.status");

CCodeSlingerEngine::SMessageHandler CCodeSlingerEngine::m_MsgHandlerList[] =
	{
		//	Aeon.onStart
		{	MSG_AEON_ON_START,					&CCodeSlingerEngine::MsgAeonOnStart },

		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,				&TSimpleEngine::MsgNull },

		//	Code.getView
		{	MSG_CODE_GET_VIEW,					&CCodeSlingerEngine::MsgGetView },

		//	Code.mandelbrot
		{	MSG_CODE_MANDELBROT,				&CCodeSlingerEngine::MsgMandelbrot },

		//	Code.mandelbrotTask
		{	MSG_CODE_MANDELBROT_TASK,			&CCodeSlingerEngine::MsgMandelbrotTask },

		//	Code.run
		{	MSG_CODE_RUN,						&CCodeSlingerEngine::MsgRun },

		//	Code.status
		{	MSG_CODE_STATUS,					&CCodeSlingerEngine::MsgStatus },
	};

int CCodeSlingerEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CCodeSlingerEngine::m_MsgHandlerList);

CCodeSlingerEngine::CCodeSlingerEngine (void) : TSimpleEngine(ENGINE_NAME_CODE_SLINGER, INITIAL_THREADS),
		m_ExecPool(m_Programs)

//	CCodeSlingerEngine constructor

	{
	}

CCodeSlingerEngine::~CCodeSlingerEngine (void)

//	CCodeSlingerEngine destructor

	{
	}

void CCodeSlingerEngine::MakeAsyncRequest (SRunResult &Result)

//	MakeAsyncRequest
//
//	This method is called when a program instance makes an async request.

	{
	if (Result.pProgram == NULL || Result.iRunCode != CHexeProcess::runAsyncRequest)
		return;

	const CString &sAddr = Result.dResult.GetElement(0);
	const CString &sMsg = Result.dResult.GetElement(1);
	CDatum dPayload = Result.dResult.GetElement(2);
	DWORD dwTicket = MakeCustomTicket();

	//	Associate the ticket with the program instance.

	m_Programs.SetAsyncTicket(*Result.pProgram, dwTicket);

	//	Make the request

	SendMessageCommand(sAddr, sMsg, GenerateAddress(PORT_CODE_COMMAND), dwTicket, dPayload);
	}

void CCodeSlingerEngine::MsgGetView (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetView
//
//	Code.getView {instanceID} {username} {seq}
//
//	We return a structure, where each field is a port. Each field has an array 
//	of entries (the format of each entry depends on the port type).
//
//	The special field $Seq is the latest sequence for the instance.
//
//	The special field $Status is the current status of the program instance:
//
//		asyncWait
//		loading
//		notStarted
//		ready
//		running
//		terminated

	{
	DWORD dwID = Msg.dPayload.GetElement(0);
	const CString &sUsername = Msg.dPayload.GetElement(1);
	SequenceNumber Seq = Msg.dPayload.GetElement(2);

	CDatum dResult;
	if (!m_Programs.GetInstanceView(dwID, sUsername, Seq, &dResult))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, dResult, Msg);
		return;
		}

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CCodeSlingerEngine::MsgRun (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRun
//
//	Code.run {code} [{options}]
//
//	options:
//
//		programID: ID of program being run.
//		runBy: Username running the program.

	{
	CDatum dCode = Msg.dPayload.GetElement(0);
	CDatum dOptions = Msg.dPayload.GetElement(1);

	SRunOptions Options;
	Options.sProgramID = dOptions.GetElement(FIELD_PROGRAM_ID);
	Options.sRunBy = dOptions.GetElement(FIELD_RUN_BY);

	CString sError;
	DWORD dwID;
	if (!m_Programs.CreateInstance(dCode, Options, &dwID, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	SendMessageReply(MSG_REPLY_DATA, CDatum(dwID), Msg);
	}

void CCodeSlingerEngine::MsgStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgStatus
//
//	Code.status

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

	//	Register some Hexe libraries

	RegisterCodeSlingerLibrary();
	}

void CCodeSlingerEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark datums for garbage collection (we can also use this
//	opportunity to garbage collect our own stuff).

	{
	m_Programs.Mark();
	}

void CCodeSlingerEngine::OnProcessReply (const SArchonMessage &Msg)

//	OnProcessReply
//
//	Handles a custom reply.

	{
	m_Programs.OnAsyncResult(Msg);
	}

void CCodeSlingerEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Start running

	{
	//	Start our threads.

	if (!m_ExecPool.Boot(*GetProcessCtx(), *this, DEFAULT_EXEC_THREAD_COUNT))
		{
		Log(MSG_LOG_ERROR, ERR_CANT_CREATE_THREADS);
		}
	}

void CCodeSlingerEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Stop running

	{
	}
