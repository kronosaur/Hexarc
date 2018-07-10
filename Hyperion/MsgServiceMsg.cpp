//	MsgServiceMsg.cpp
//
//	Hyperion.serviceMsg
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(CMD_REDIRECT,						"redirect")

DECLARE_CONST_STRING(FIELD_PROTOCOL,					"protocol")

DECLARE_CONST_STRING(LIBRARY_HYPERION,					"hyperion")
DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")
DECLARE_CONST_STRING(LIBRARY_SESSION_CTX,				"sessionCtx")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_BODY_BUILDER,	"sessionHTTPBodyBuilder")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_REQUEST,		"sessionHTTPRequest")

DECLARE_CONST_STRING(MSG_ERROR_TIMEOUT,					"Error.timeout")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_OK,							"OK")

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command")

DECLARE_CONST_STRING(PROTOCOL_SERVICE_MSG,				"serviceMsg")

DECLARE_CONST_STRING(ERR_INVALID_RESPONSE,				"Cannot interpret service response.")
DECLARE_CONST_STRING(ERR_NO_HANDLER,					"Hexarc message handler not found: %s+%s.")
DECLARE_CONST_STRING(ERR_INVALID_MESSAGE,				"Invalid Hexarc message redirect: %s.")
DECLARE_CONST_STRING(ERR_SERVICE_NOT_FOUND,				"Service not found: %s.")
DECLARE_CONST_STRING(ERR_INVALID_ADDRESS,				"Unable to send Hexarc message to address: %s.")
DECLARE_CONST_STRING(ERR_TIMEOUT,						"Unable to complete computation within timeout limit.")

const int DEFAULT_TIMEOUT =								30 * 1000;

class CHexarcMsgSession : public CHyperionSession
	{
	public:
		CHexarcMsgSession (CHyperionEngine *pEngine, IHyperionService *pService, const CHexeSecurityCtx *pSecurityCtx, const CString &sMsg, CDatum dPayload);

	protected:
		//	ISessionHandler virtuals

		virtual void OnEndSession (DWORD dwTicket);
		virtual void OnMark (void);
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

		//	CHyperionSession virtuals

		virtual void OnGetHyperionStatusResport (CComplexStruct *pStatus) const;

	private:
		bool ComposeResponse (CHexeProcess::ERunCodes iRun, CDatum dResult);

		IHyperionService *m_pService;		//	Service handling the request
		const CHexeSecurityCtx *m_pCallerSecurityCtx;	//	Security ctx of caller
		CString m_sMsg;						//	Hexarc message to process
		CDatum m_dPayload;					//	Payload for message

		CHexeProcess m_Process;				//	Process to use to handle message
	};

void CHyperionEngine::MsgServiceMsg (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgServiceMsg
//
//	Hyperion.serviceMsg {service} {msg} {payload}

	{
	CString sService = Msg.dPayload.GetElement(0);
	CString sMsg = Msg.dPayload.GetElement(1);
	CDatum dPayload = Msg.dPayload.GetElement(2);

	//	Find the service that can handle this message

	IHyperionService *pService;
	if (!FindHexarcMsgService(sService, &pService))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_SERVICE_NOT_FOUND, sService), Msg);
		return;
		}

	//	Start a session

	return StartSession(Msg, new CHexarcMsgSession(this, pService, pSecurityCtx, sMsg, dPayload));
	}

//	CHexarcMsgSession ----------------------------------------------------------

CHexarcMsgSession::CHexarcMsgSession (CHyperionEngine *pEngine, IHyperionService *pService, const CHexeSecurityCtx *pSecurityCtx, const CString &sMsg, CDatum dPayload) : CHyperionSession(pEngine, NULL_STR, CDatum(), NULL_STR),
		m_pService(pService),
		m_pCallerSecurityCtx(pSecurityCtx),
		m_sMsg(sMsg),
		m_dPayload(dPayload)

//	CHexarcMsgSession constructor

	{
	}

bool CHexarcMsgSession::ComposeResponse (CHexeProcess::ERunCodes iRun, CDatum dResult)

//	ComposeResponse
//
//	Compose a session response

	{
	switch (iRun)
		{
		case CHexeProcess::runOK:
			{
			//	Parse the response. We expect one of the following formats:
			//
			//	payload
			//	(msg payload)
			//	('redirect address msg payload)

			if (dResult.GetCount() == 1)
				ISessionHandler::SendMessageReply(MSG_OK, dResult);

			else if (dResult.GetCount() == 2)
				ISessionHandler::SendMessageReply(dResult.GetElement(0), dResult.GetElement(1));

			else if (dResult.GetCount() == 4)
				{
				const CString &sCmd = dResult.GetElement(0);

				//	Redirect command

				if (strEquals(sCmd, CMD_REDIRECT))
					{
					const CString &sAddress = dResult.GetElement(1);
					const CString &sMsg = dResult.GetElement(2);
					CDatum dPayload = dResult.GetElement(3);
					const CString &sReplyAddr = GetOriginalMsg().sReplyAddr;
					DWORD dwTicket = GetOriginalMsg().dwTicket;

					//	Make sure this is a valid Hexarc message

					if (!CHexeProcess::IsHexarcMessage(sMsg))
						{
						ISessionHandler::SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_MESSAGE, sMsg));
						return false;
						}

					bool bSent;

					//	If we have a security context then use it to make the call.

					//	NOTE: We send a message through the process because we 
					//	need to specify our own ticket (the ticket of the original
					//	call) since we are redirecting.

					if (!m_SecurityCtx.IsEmpty())
						{
						CString sWrappedMsg;
						CDatum dWrappedPayload;

						CHexeProcess::ComposeHexarcMessage(m_SecurityCtx, sMsg, dPayload, &sWrappedMsg, &dWrappedPayload);
						bSent = GetProcessCtx()->SendMessageCommand(sAddress, sWrappedMsg, sReplyAddr, dwTicket, dWrappedPayload);
						}
					else
						bSent = GetProcessCtx()->SendMessageCommand(sAddress, sMsg, sReplyAddr, dwTicket, dPayload);

					//	Check for error sending

					if (!bSent)
						ISessionHandler::SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_ADDRESS, sAddress));
					}

				//	Unknown command

				else
					ISessionHandler::SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_RESPONSE);
				}

			else
				ISessionHandler::SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_RESPONSE);

			return false;
			}

		case CHexeProcess::runAsyncRequest:
			{
			const CString &sAddr = dResult.GetElement(0);
			CString sMsg = dResult.GetElement(1);
			CDatum dPayload = dResult.GetElement(2);

			//	Send the message

			ISessionHandler::SendMessageCommand(sAddr,
					sMsg,
					GenerateAddress(PORT_HYPERION_COMMAND),
					dPayload,
					DEFAULT_TIMEOUT);

			//	Expect a reply

			return true;
			}

		case CHexeProcess::runError:
			{
			ISessionHandler::SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, dResult);
			return false;
			}

		default:
			//	Can't happen
			ASSERT(false);
			ISessionHandler::SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_RESPONSE);
			return false;
		}
	}

void CHexarcMsgSession::OnEndSession (DWORD dwTicket)

//	OnEndSession
//
//	End the session

	{
	//	Remove from service

	if (m_pService)
		{
		m_pService->DeleteSession(this);
		m_pService = NULL;
		}
	}

void CHexarcMsgSession::OnGetHyperionStatusResport (CComplexStruct *pStatus) const

//	OnGetHyperionStatusReport
//
//	Returns information about the session

	{
	pStatus->SetElement(FIELD_PROTOCOL, PROTOCOL_SERVICE_MSG);
	}

void CHexarcMsgSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	m_Process.Mark();
	m_dPayload.Mark();

	//	Let our ancestor mark

	CHyperionSession::OnMark();
	}

bool CHexarcMsgSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a message

	{
	//	If this is a timeout, then we reply with an error

	if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
		{
		ISessionHandler::SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_TIMEOUT);
		return false;
		}

	//	Call the function

	CDatum dResult;
	CHexeProcess::ERunCodes iRun = m_Process.RunContinues(CSimpleEngine::MessageToHexeResult(Msg), &dResult);

	//	Handle the result

	return ComposeResponse(iRun, dResult);
	}

bool CHexarcMsgSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start a session

	{
	int i;

	//	Add our session to the service

	m_pService->InsertSession(this);

	//	Initialize our security context. First we take the service security
	//	(sandbox) from the service.

	m_SecurityCtx.SetServiceSecurity(m_pService->GetSecurityCtx());

	//	Take the user context from the caller (if appropriate)

	if (m_pCallerSecurityCtx)
		{
		//	If we are a user-process then take user context from the caller
		//	(If we want more security we should ask for an authToken in the 
		//	message.)
		//
		//	Otherwise, we only take the user context if the caller is an 
		//	admin process

		if (m_pService->IsSandboxed() || m_pCallerSecurityCtx->HasServiceRightArcAdmin())
			m_SecurityCtx.SetUserSecurity(*m_pCallerSecurityCtx);
		}

	//	Initialize our process

	m_pService->InitProcess(m_Process);
	m_Process.SetSecurityCtx(m_SecurityCtx);
	m_Process.SetLibraryCtx(LIBRARY_HYPERION, GetEngine());
	m_Process.SetLibraryCtx(LIBRARY_SESSION, (CHyperionSession *)this);
	m_Process.SetLibraryCtx(LIBRARY_SESSION_CTX, NULL);
	m_Process.SetLibraryCtx(LIBRARY_SESSION_HTTP_BODY_BUILDER, NULL);
	m_Process.SetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST, NULL);

	//	Generate an array of arguments to the function call

	TArray<CDatum> Args;
	for (i = 0; i < m_dPayload.GetCount(); i++)
		Args.Insert(m_dPayload.GetElement(i));

	//	Generate a call to the appropriate handler for this entry point

	CDatum dCode;
	if (m_Process.FindGlobalDef(strPattern("%s+%s", m_pService->GetName(), m_sMsg), &dCode))
		;

	//	Otherwise, see if we have the generic handler.

	else if (m_Process.FindGlobalDef(strPattern("%s+*", m_pService->GetName()), &dCode))
		{
		//	Add the message as the first arg

		Args.Insert(m_sMsg, 0);
		}

	//	Otherwise we reply with an error.

	else
		{
		ISessionHandler::SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_NO_HANDLER, m_pService->GetName(), m_sMsg));
		return false;
		}

	//	Run

	CDatum dResult;
	CHexeProcess::ERunCodes iRun = m_Process.Run(dCode, Args, &dResult);

	//	Handle it

	return ComposeResponse(iRun, dResult);
	}
