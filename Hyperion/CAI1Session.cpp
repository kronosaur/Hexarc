//	CAI1Session.cpp
//
//	CAI1Session class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	IMPLEMENTATION NOTES
//
//	The two major functions are ProcessCommand and ProcessRPC. ProcessCommand
//	handles commands from the client. Sometimes, while processing a command
//	we need to send out an RPC message to another engine. In that case,
//	ProcessCommand sends the RPC and ProcessRPC handles it.

#include "stdafx.h"

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command")

DECLARE_CONST_STRING(ADDRESS_CRYPTOSAUR_COMMAND,		"Cryptosaur.command")
DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command")

DECLARE_CONST_STRING(AUTH_TYPE_SHA1,					"SHA1")

DECLARE_CONST_STRING(CMD_ADMIN_NEEDED,					"ADMIN-REQUIRED")
DECLARE_CONST_STRING(CMD_AUTH,							"AUTH")
DECLARE_CONST_STRING(CMD_AUTH_V1,						"AUTH-V1")
DECLARE_CONST_STRING(CMD_AUTH_INVALID,					"AUTH-INVALID")
DECLARE_CONST_STRING(CMD_AUTH_REQUIRED,					"AUTH-REQUIRED")
DECLARE_CONST_STRING(CMD_CONNECT,						"CONNECT")
DECLARE_CONST_STRING(CMD_CREATE_ADMIN,					"createAdmin")
DECLARE_CONST_STRING(CMD_ERROR,							"ERROR")
DECLARE_CONST_STRING(CMD_OK,							"OK")
DECLARE_CONST_STRING(CMD_OUT,							"OUT")
DECLARE_CONST_STRING(CMD_WELCOME,						"WELCOME")

DECLARE_CONST_STRING(FIELD_ACTUAL,						"actual")
DECLARE_CONST_STRING(FIELD_CHALLENGE_CREDENTIALS,		"challengeCredentials")
DECLARE_CONST_STRING(FIELD_PROTOCOL,					"protocol")
DECLARE_CONST_STRING(FIELD_RIGHTS,						"rights")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")
DECLARE_CONST_STRING(FIELD_USERNAME,					"username")

DECLARE_CONST_STRING(INTERFACE_ARC_CONSOLE,				"Arc.console")

DECLARE_CONST_STRING(LIBRARY_HYPERION,					"hyperion")
DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")
DECLARE_CONST_STRING(LIBRARY_SESSION_CTX,				"sessionCtx")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_BODY_BUILDER,	"sessionHTTPBodyBuilder")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_REQUEST,		"sessionHTTPRequest")

DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_ADMIN,		"Cryptosaur.createAdmin")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CHECK_PASSWORD_SHA1,"Cryptosaur.login_SHA1")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_LOGIN_USER,			"Cryptosaur.loginUser")
DECLARE_CONST_STRING(MSG_ERROR_DOES_NOT_EXIST,			"Error.doesNotExist")
DECLARE_CONST_STRING(MSG_ERROR_TIMEOUT,					"Error.timeout")
DECLARE_CONST_STRING(MSG_ESPER_DISCONNECT,				"Esper.disconnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_DISCONNECT,			"Esper.onDisconnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_READ,					"Esper.onRead")
DECLARE_CONST_STRING(MSG_ESPER_ON_WRITE,				"Esper.onWrite")
DECLARE_CONST_STRING(MSG_ESPER_READ,					"Esper.read")
DECLARE_CONST_STRING(MSG_ESPER_WRITE,					"Esper.write")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_REPLY_PROGRESS,				"Reply.progress")

DECLARE_CONST_STRING(PROTOCOL_AI1,						"ai1")

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin")

DECLARE_CONST_STRING(ERR_MESSAGE_TOO_BIG,				"AI1 message is too big to parse.")
DECLARE_CONST_STRING(ERR_ADMIN_NEEDED,					"Arcology needs an admin account.")
DECLARE_CONST_STRING(ERR_CONNECT_EXPECTED,				"CONNECT expected.")
DECLARE_CONST_STRING(ERR_INTERFACE_UNAVAILABLE,			"Interface not yet available: %s.")
DECLARE_CONST_STRING(ERR_ON_WRITE_UNDERFLOW,			"Too many onWrite messages received.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_MSG,				"Unexpected msg: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_COMMAND,				"Unknown command: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_INTERFACE,				"Unknown interface: %s.")
DECLARE_CONST_STRING(ERR_NO_CONNECT_PERMISSION,			"You are not authorized to connect to %s.")
DECLARE_CONST_STRING(ERR_RPC_TIMEOUT,					"No response from RPC message.")

#ifdef DEBUGX
const int DEFAULT_TIMEOUT =								300 * 1000;
#else
const int DEFAULT_TIMEOUT =								30 * 1000;
#endif

CAI1Session::CAI1Session (CHyperionEngine *pEngine, const CString &sListener, CDatum dSocket, const CString &sNetAddress) :
		CHyperionSession(pEngine, sListener, NULL_STR, dSocket, sNetAddress),
		m_pService(NULL),
		m_iState(stateUnknown),
		m_iWritesOutstanding(0)

//	CAI1Session constructor

	{
	}

CString CAI1Session::GetDebugInfo (void) const

//	GetDebugInfo
//
//	Info about a session

	{
	return strPattern("[%x] AI1: state=%d.", CEsperInterface::ConnectionToFriendlyID(m_dSocket), m_iState);
	}

bool CAI1Session::ExecuteHexeCommand (const CString &sCommand, CDatum dPayload)

//	ExecuteHexeCommand
//
//	Executes the command

	{
	//	Generate an array of arguments

	TArray<CDatum> Args;

	//	Look for a function of the form {interface}_{command}. If we find it,
	//	call it with the payload.

	CString sCall;
	CString sFunction;
	if (m_Process.FindGlobalDef(sFunction = strPattern("%s+%s", m_pService->GetInterface(), sCommand)))
		Args.Insert(dPayload);

	//	Otherwise, see if we have the generic handler. In that case, we call
	//	it with the command and the payload.

	else if (m_Process.FindGlobalDef(sFunction = strPattern("%s+*", m_pService->GetInterface())))
		{
		Args.Insert(sCommand);
		Args.Insert(dPayload);
		}

	//	Otherwise, we have an error

	else
		return SendReplyError(strPattern(ERR_UNKNOWN_COMMAND, sCommand));

	//	Create a function invocation

	CDatum dCode;
	CHexeDocument::CreateFunctionCall(sFunction, Args, &dCode);

	//	Run

	CDatum dResult;
	CHexeProcess::ERun iRun = m_Process.Run(dCode, &dResult);

	//	Handle the result

	return ProcessRunResult(iRun, dResult);
	}

void CAI1Session::FillStream (void)

//	FillStream
//
//	Request data from Esper to fill the stream

	{
	//	Initialize context state

	m_iState = stateWaitingForData;

	//	Compose message

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);

	//	Start by reading from the client (we expect a request)

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_READ,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);
	}

bool CAI1Session::HandleOnWrite (const SArchonMessage &Msg)

//	HandleOnWrite
//
//	If this is an onWrite message then decrement the count of outstanding
//	messages. We return TRUE if this was an onWrite message and we handled it.

	{
	if (strEquals(Msg.sMsg, MSG_ESPER_ON_WRITE))
		{
		if (m_iWritesOutstanding == 0)
			GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_ON_WRITE_UNDERFLOW);
		else
			m_iWritesOutstanding--;

		//	Handled

		return true;
		}

	//	Not handled

	return false;
	}

void CAI1Session::OnEndSession (DWORD dwTicket)

//	OnEndSession
//
//	Session ends

	{
	if (m_pService)
		{
		m_pService->DeleteSession(this);
		m_pService = NULL;
		}
	}

void CAI1Session::OnGetHyperionStatusReport (CComplexStruct *pStatus) const

//	OnGetHyperionStatusReport
//
//	Fills in the structure with status information

	{
	pStatus->SetElement(FIELD_PROTOCOL, PROTOCOL_AI1);
	}

void CAI1Session::OnMark (void)

//	OnMark
//
//	Mark data in user

	{
	m_Stream.Mark();
	m_Process.Mark();
	m_dChallenge.Mark();

	//	Let our ancestor mark

	CHyperionSession::OnMark();
	}

bool CAI1Session::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a reply

	{
	switch (m_iState)
		{
		case stateWaitingForData:
			{
			//	If this is a timeout then reset the timeout (since we can wait for a while
			//	for the client. [LATER: Keep track of how long we've kept the connection open
			//	and close it after a period of inactivity.]

			if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
				{
				ResetTimeout(GenerateAddress(PORT_HYPERION_COMMAND), DEFAULT_TIMEOUT);
				return true;
				}

			//	If this is a disconnect, then we can drop the session

			else if (strEquals(Msg.sMsg, MSG_ESPER_ON_DISCONNECT))
				return false;

			//	If this is not an onRead message, then we disconnect

			else if (!strEquals(Msg.sMsg, MSG_ESPER_ON_READ))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_MSG, Msg.sMsg));
				SendDisconnect();
				return false;
				}

			//	Add the buffer to our stream

			m_Stream.ReadFromEsper(Msg.dPayload.GetElement(0));

			//	If we still don't have a full message, read some more

			bool bOverflow;
			bool bMore = m_Stream.HasMore(&bOverflow);
			if (bOverflow)
				GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_MESSAGE_TOO_BIG);

			if (!bMore)
				{
				FillStream();
				return true;
				}

			//	Otherwise, process the command

			CString sCommand;
			CDatum dPayload;
			m_Stream.GetNext(&sCommand, &dPayload);

			return ProcessCommand(sCommand, dPayload);
			}

		case stateWaitingForRPC:
		case stateWaitingForCreateAdminRPC:
		case stateWaitingForAuthRPC:
			{
			//	If this is a timeout, then we reply with an error

			if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
				return SendReplyError(ERR_RPC_TIMEOUT);

			//	If this is an onWrite message, then handle the ack. This happens
			//	when we send progress replies. We return TRUE because we keep
			//	waiting for a response.

			else if (HandleOnWrite(Msg))
				return true;

			//	Process the RPC response

			return ProcessRPC(Msg);
			}

		case stateWaitingForWriteConfirm:
			{
			//	If this is not an onWrite message, then we disconnect

			if (!strEquals(Msg.sMsg, MSG_ESPER_ON_WRITE))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_MSG, Msg.sMsg));
				SendDisconnect();
				return false;
				}

			//	If we're still waiting for acks, stay in this state

			if (--m_iWritesOutstanding > 0)
				return true;

			//	If we don't have a full message, read some more

			if (!m_Stream.HasMore())
				{
				FillStream();
				return true;
				}

			//	Otherwise, process the command

			CString sCommand;
			CDatum dPayload;
			m_Stream.GetNext(&sCommand, &dPayload);

			return ProcessCommand(sCommand, dPayload);
			}

		case stateWaitingForFinalWrite:
			{
			//	If this is not an onWrite message, then we disconnect

			if (!strEquals(Msg.sMsg, MSG_ESPER_ON_WRITE))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_MSG, Msg.sMsg));
				SendDisconnect();
				return false;
				}

			//	If we're still waiting for acks, stay in this state

			if (--m_iWritesOutstanding > 0)
				return true;

			//	Disconnect

			SendDisconnect();
			return false;
			}

		default:
			//	If we get this far, then something went wrong
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_MSG, Msg.sMsg));
			SendDisconnect();
			return false;
		}
	}

bool CAI1Session::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start a new session

	{
	FillStream();
	return true;
	}

bool CAI1Session::ProcessCommand (const CString &sCommand, CDatum dPayload)

//	ProcessCommand
//
//	Process an AI1 command

	{
	//	Handle the CONNECT command

	if (strEquals(sCommand, CMD_CONNECT))
		{
		const CString &sInterface = dPayload.GetElement(0);

		//	If we don't have an admin, then refuse the connection, unless this
		//	is a well-known interface that knows how to deal with arcology
		//	start-up.

		if (m_pEngine->IsAdminNeeded() && !strEquals(sInterface, INTERFACE_ARC_CONSOLE))
			return SendReplyError(strPattern(ERR_INTERFACE_UNAVAILABLE, sInterface));

		//	Find the interface

		CAI1Service *pService;
		if (!m_pEngine->FindAI1Service(m_sListener, sInterface, &pService))
			return SendReplyError(strPattern(ERR_UNKNOWN_INTERFACE, sInterface));

		//	Link up the service with the session and vice versa

		if (pService != m_pService)
			{
			if (m_pService)
				m_pService->DeleteSession(this);

			ASSERT(pService);
			m_pService = pService;
			m_pService->InsertSession(this);

			//	Initialize our security context

			m_SecurityCtx.SetServiceSecurity(m_pService->GetSecurityCtx());
			}

		//	Now that we have an interface initialize the process

		m_pService->InitProcess(m_Process);
		m_Process.SetSecurityCtx(m_SecurityCtx);
		m_Process.SetLibraryCtx(LIBRARY_HYPERION, GetEngine());
		m_Process.SetLibraryCtx(LIBRARY_SESSION, (CHyperionSession *)this);
		m_Process.SetLibraryCtx(LIBRARY_SESSION_CTX, NULL);
		m_Process.SetLibraryCtx(LIBRARY_SESSION_HTTP_BODY_BUILDER, NULL);
		m_Process.SetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST, NULL);

		//	Do we require an admin? [We should only get here if we're the
		//	Arc.console interface because of a check above.]

		if (m_pEngine->IsAdminNeeded())
			return SendReply(CMD_ADMIN_NEEDED, CDatum());

		//	Do we require authorization?

		if (!m_pService->AllowsAnonymousAccess())
			{
			m_dChallenge = CAI1Protocol::CreateSHAPasswordChallenge();
			return SendReply(CMD_AUTH_REQUIRED, m_dChallenge);
			}

		//	WELCOME

		return SendReplyWelcome();
		}

	//	If we don't have a service yet, then the only command that we respond
	//	to is CONNECT

	else if (m_pService == NULL)
		return SendReplyError(ERR_CONNECT_EXPECTED);

	//	If we need an admin, then we need to handle the createAdmin command
	//	specially.

	else if (m_pEngine->IsAdminNeeded())
		{
		if (!strEquals(sCommand, CMD_CREATE_ADMIN))
			return SendReplyError(ERR_ADMIN_NEEDED);

		//	We remember the username and rigths because the reply from
		//	Cryptosaur will not include this info. If we get an error later
		//	we can revert this.

		m_SecurityCtx.SetUsername(dPayload.GetElement(0));
		m_SecurityCtx.InsertUserRight(RIGHT_ARC_ADMIN);

		//	Generate an RPC call to Cryptosaur, but remember our state so that
		//	when we return we can respond appropriately (i.e., continue with
		//	arcology installation).

		SArchonMessage CreateAdminMsg;
		CreateAdminMsg.sMsg = MSG_CRYPTOSAUR_CREATE_ADMIN;
		CreateAdminMsg.dPayload = dPayload;

		return SendRPC(ADDRESS_CRYPTOSAUR_COMMAND, CreateAdminMsg, stateWaitingForCreateAdminRPC);
		}

	//	Handle authentication

	else if (strEquals(sCommand, CMD_AUTH))
		{
		//	Create an authDesc

		CComplexStruct *pAuthDesc = new CComplexStruct;
		pAuthDesc->SetElement(FIELD_ACTUAL, CDatum(true));
		pAuthDesc->SetElement(FIELD_CHALLENGE_CREDENTIALS, dPayload.GetElement(1));
		pAuthDesc->SetElement(FIELD_TYPE, AUTH_TYPE_SHA1);

		//	Put together a payload that includes the username, challenge, and response

		CComplexArray *pPayload = new CComplexArray;
		pPayload->Insert(dPayload.GetElement(0));
		pPayload->Insert(CDatum(pAuthDesc));
		pPayload->Insert(m_dChallenge);

		//	Send a message to Cryptosaur to validate the username

		SArchonMessage AuthMsg;
		AuthMsg.sMsg = MSG_CRYPTOSAUR_LOGIN_USER;
		AuthMsg.dPayload = CDatum(pPayload);

		return SendRPC(ADDRESS_CRYPTOSAUR_COMMAND, AuthMsg, stateWaitingForAuthRPC);
		}

	else if (strEquals(sCommand, CMD_AUTH_V1))
		{
		//	Put together a payload that includes the username, challenge, and response

		CComplexArray *pPayload = new CComplexArray;
		pPayload->Insert(dPayload.GetElement(0));
		pPayload->Insert(m_dChallenge);
		pPayload->Insert(dPayload.GetElement(1));

		//	Send a message to Cryptosaur to validate the username

		SArchonMessage AuthMsg;
		AuthMsg.sMsg = MSG_CRYPTOSAUR_CHECK_PASSWORD_SHA1;
		AuthMsg.dPayload = CDatum(pPayload);

		return SendRPC(ADDRESS_CRYPTOSAUR_COMMAND, AuthMsg, stateWaitingForAuthRPC);
		}

	//	Otherwise, run the command 

	else
		return ExecuteHexeCommand(sCommand, dPayload);
	}

bool CAI1Session::ProcessRPC (const SArchonMessage &RPCMsg)

//	ProcessRPC
//
//	Process an RPC reply

	{
	//	If this is a progress message, then return it to the client

	if (strEquals(RPCMsg.sMsg, MSG_REPLY_PROGRESS))
		return SendProgress(RPCMsg.dPayload);

	//	If we're waiting for the result of createAdmin, handle it now

	else if (m_iState == stateWaitingForCreateAdminRPC)
		{
		if (IsError(RPCMsg))
			{
			m_SecurityCtx.SetAnonymous();
			return SendReplyError(RPCMsg.dPayload);
			}

		//	If we succeed then we have successfully connected to the arcology
		//	and we can thus send the WELCOME message.

		return SendReplyWelcome();
		}

	//	If we're waiting for authentication, handle it now

	else if (m_iState == stateWaitingForAuthRPC)
		{
		//	If we get Error.doesNotExist, then auth failed and we need to ask
		//	again.

		if (strEquals(RPCMsg.sMsg, MSG_ERROR_DOES_NOT_EXIST))
			{
			m_dChallenge = CAI1Protocol::CreateSHAPasswordChallenge();
			return SendReply(CMD_AUTH_INVALID, m_dChallenge);
			}

		//	Otherwise, if we got a different error, we cannot continue

		else if (IsError(RPCMsg))
			return SendReplyError(RPCMsg.dPayload);

		//	Set our user credentials

		m_SecurityCtx.SetUsername(RPCMsg.dPayload.GetElement(FIELD_USERNAME));
		m_SecurityCtx.SetUserRights(RPCMsg.dPayload.GetElement(FIELD_RIGHTS));

		//	See if we have sufficient rights to connect to this interface. If
		//	not then we clear out the user info and return an error

		if (!m_SecurityCtx.HasUserRights(m_pService->GetRightsRequired()))
			{
			m_SecurityCtx.SetAnonymous();
			return SendReplyErrorAndDisconnect(strPattern(ERR_NO_CONNECT_PERMISSION, m_pService->GetInterface()));
			}

		//	We have successfully logged in, so send out a WELCOME.

		return SendReplyWelcome();
		}

	//	Otherwise, we have a result so we can continue running

	else
		{
		//	Call the function

		CDatum dResult;
		CHexeProcess::ERun iRun = m_Process.RunContinues(CSimpleEngine::MessageToHexeResult(RPCMsg), &dResult);

		//	Handle the result

		return ProcessRunResult(iRun, dResult);
		}
	}

bool CAI1Session::ProcessRunResult (CHexeProcess::ERun iRun, CDatum dRunResult)

//	ProcessRunResult
//
//	Processes the result of a Run or RunContinues call

	{
	switch (iRun)
		{
		case CHexeProcess::ERun::OK:
			return SendReply(CMD_OK, dRunResult);

		case CHexeProcess::ERun::AsyncRequest:
			{
			const CString &sAddr = dRunResult.GetElement(0);

			SArchonMessage Msg;
			Msg.sMsg = dRunResult.GetElement(1);
			Msg.dPayload = dRunResult.GetElement(2);

			return SendRPC(sAddr, Msg);
			}

		case CHexeProcess::ERun::Error:
			return SendReplyError(dRunResult);

		default:
			//	Can't happen
			ASSERT(false);
			return false;
		}
	}

void CAI1Session::SendDisconnect (void)

//	SendDisconnect
//
//	Close connection

	{
	//	Compose message

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);

	//	Tell Esper to disconnect

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_DISCONNECT,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);
	}

bool CAI1Session::SendProgress (CDatum dPayload)

//	SendProgress
//
//	Sends a progress message to the client

	{
	//	Compose an AI1 message

	CDatum dData;
	CAI1Stream::WriteToEsper(CMD_OUT, dPayload, &dData);

	//	Remember to get an ack for this write

	m_iWritesOutstanding++;

	//	Send the esper message

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);
	pPayload->Insert(dData);

	//	Write to the client

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_WRITE,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

	//	Expect an ack

	return true;
	}

bool CAI1Session::SendReply (const CString &sCommand, CDatum dPayload, EStates iNewState)

//	SendReply
//
//	Sends a reply to the client

	{
	//	Compose an AI1 message

	CDatum dData;
	CAI1Stream::WriteToEsper(sCommand, dPayload, &dData);

	//	Wait for the write to succeed

	m_iState = iNewState;
	m_iWritesOutstanding++;

	//	Send the esper message

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dSocket);
	pPayload->Insert(dData);

	//	Write to the client

	ISessionHandler::SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_WRITE,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

	//	Expect an onWrite ack

	return true;
	}

bool CAI1Session::SendReplyError (const CString &sError)

//	SendReplyError
//
//	Error reply

	{
	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(sError);

	return SendReply(CMD_ERROR, CDatum(pArray));
	}

bool CAI1Session::SendReplyErrorAndDisconnect (const CString &sError)

//	SendReplyErrorAndDisconnect
//
//	Send a reply and disconnect after we get the write confirmation

	{
	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(sError);

	return SendReply(CMD_ERROR, CDatum(pArray), stateWaitingForFinalWrite);
	}

bool CAI1Session::SendReplyWelcome (void)

//	SendReplyWelcome
//
//	Sends a welcome message reply when the connection is first established

	{
	return SendReply(CMD_WELCOME, CString("Welcome to the arcology!"));
	}

bool CAI1Session::SendRPC (const CString &sAddr, const SArchonMessage &Msg, EStates iNewState)

//	SendRPC
//
//	Sends an RPC message

	{
	//	Initialize context state

	m_iState = iNewState;

	//	Send the message

	ISessionHandler::SendMessageCommand(sAddr,
			Msg.sMsg,
			GenerateAddress(PORT_HYPERION_COMMAND),
			Msg.dPayload,
			DEFAULT_TIMEOUT);

	//	Expect a reply

	return true;
	}
