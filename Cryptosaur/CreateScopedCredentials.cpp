//	CreateScopedCredentials.cpp
//
//	Cryptosaur.createScopedCredentials
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")

DECLARE_CONST_STRING(AUTH_TYPE_SHA1,					"SHA1")

DECLARE_CONST_STRING(FIELD_AUTH_DESC,					"authDesc")
DECLARE_CONST_STRING(FIELD_CREDENTIALS,					"credentials")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")

DECLARE_CONST_STRING(MSG_AEON_GET_VALUE,				"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_ERROR_DOES_NOT_EXIST,			"Error.doesNotExist")
DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(TABLE_ARC_USERS,					"Arc.users")

DECLARE_CONST_STRING(ERR_CREDENTIALS_ALREADY_EXIST,		"%s already exists.")
DECLARE_CONST_STRING(ERR_INVALID_AUTHORIZATION,			"Invalid authorization from user: %s.")
DECLARE_CONST_STRING(ERR_INVALID_USERNAME,				"Invalid username.")
DECLARE_CONST_STRING(ERR_SCOPE_REQUIRED,				"Scope required.")
DECLARE_CONST_STRING(ERR_UNKNOWN_USERNAME,				"Unknown username: %s.")

const DWORD MESSAGE_TIMEOUT =							30 * 1000;

class CCreateScopedCredentialsSession : public ISessionHandler
	{
	public:
		CCreateScopedCredentialsSession (CCryptosaurEngine *pEngine, const CString &sUsername, const CString &sScope, CDatum dAuthDesc, bool bRegen) : 
				m_pEngine(pEngine),
				m_iState(stateUnknown),
				m_sUsername(sUsername),
				m_sUsernameKey(strToLower(sUsername)),
				m_sScope(sScope),
				m_dAuthDesc(dAuthDesc),
				m_bRegen(bRegen) { }

	protected:
		//	ISessionHandler virtuals
		virtual void OnMark (void) { m_dCredentials.Mark(); m_dAuthDesc.Mark(); }
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum States
			{
			stateUnknown,
			stateWaitingForUserRecord,
			stateWaitingForUpdate,
			};

		CCryptosaurEngine *m_pEngine;
		States m_iState;
		CString m_sUsername;
		CString m_sUsernameKey;
		CString m_sScope;
		CDatum m_dAuthDesc;
		bool m_bRegen;

		CDatum m_dCredentials;
	};

void CCryptosaurEngine::MsgCreateScopedCredentials (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateScopedCredentials
//
//	Cryptosaur.createScopedCredentials {username} {authDesc}

	{
	//	Any service can create a new user

	if (!ValidateMessage(Msg, pSecurityCtx, false))
		return;

	//	Make sure we have a username

	const CString &sUsername = Msg.dPayload.GetElement(0);
	if (sUsername.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_USERNAME, Msg);
		return;
		}

	//	Get authDesc

	CDatum dAuthDesc = Msg.dPayload.GetElement(1);

	//	Make sure we have a scope

	if (pSecurityCtx == NULL || pSecurityCtx->GetSandbox().IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_SCOPE_REQUIRED, Msg);
		return;
		}

	//	Begin a session to create the credentials

	StartSession(Msg, new CCreateScopedCredentialsSession(this, 
			sUsername,
			pSecurityCtx->GetSandbox(),
			dAuthDesc,
			!Msg.dPayload.GetElement(2).IsNil()
			));
	}

bool CCreateScopedCredentialsSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	We received a reply from Aeon

	{
	//	If we got an error, then we return the error.

	if (IsError(Msg))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload);
		return false;
		}

	//	Otherwise, process based on our state.

	switch (m_iState)
		{
		case stateWaitingForUserRecord:
			{
			CDatum dUserData = Msg.dPayload;

			//	If Nil, then the user does not exist

			if (dUserData.IsNil())
				{
				SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, strPattern(ERR_UNKNOWN_USERNAME, m_sUsername));
				return false;
				}

			//	Make sure we are authorized to do this

			if (!m_pEngine->ValidateAuthDescActual(m_dAuthDesc, m_sScope, dUserData))
				{
				SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_INVALID_AUTHORIZATION, m_sUsername));
				return false;
				}

			//	Store under a scope-specific authDesc field

			CString sScopedAuthDesc = strPattern("%s%s", m_sScope, FIELD_AUTH_DESC);

			//	If auth credentials already exist, and if we haven't been asked to 
			//	recreate the credentials, then we just return the existing ones.

			CDatum dCurrentAuthDesc = dUserData.GetElement(sScopedAuthDesc);
			CDatum dCurrentCredentials = dCurrentAuthDesc.GetElement(FIELD_CREDENTIALS);
			if (!dCurrentCredentials.IsNil() && !m_bRegen)
				{
				SendMessageReply(MSG_REPLY_DATA, dCurrentCredentials);
				return false;
				}

			//	Generate a random password.
			//	NOTE: Since this is already a random password, revealing it is
			//	no worse than revealing the hash, so we don't bother to hash it.

			CIPInteger Password;
			cryptoRandom(64, &Password);

			CDatum::CreateIPIntegerFromHandoff(Password, &m_dCredentials);

			//	Generate credentials

			CComplexStruct *pAuthDesc = new CComplexStruct;
			pAuthDesc->SetElement(FIELD_TYPE, AUTH_TYPE_SHA1);
			pAuthDesc->SetElement(FIELD_CREDENTIALS, m_dCredentials);

			//	Clone the user data and set the element

			CComplexStruct *pNewUserData = new CComplexStruct;
			pNewUserData->SetElement(sScopedAuthDesc, CDatum(pAuthDesc));

			//	Store it back in Aeon

			CComplexArray *pPayload = new CComplexArray;
			pPayload->Insert(TABLE_ARC_USERS);
			pPayload->Insert(m_sUsernameKey);
			pPayload->Insert(CDatum(pNewUserData));

			//	Send message

			m_iState = stateWaitingForUpdate;

			ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
					MSG_AEON_MUTATE,
					GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
					CDatum(pPayload),
					MESSAGE_TIMEOUT);

			//	Expect a reply

			return true;
			}

		case stateWaitingForUpdate:
			{
			//	Since we're succesful, return the credentials.

			SendMessageReply(MSG_REPLY_DATA, m_dCredentials);

			//	Done

			return false;
			}

		default:
			return false;
		}
	}

bool CCreateScopedCredentialsSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session.

	{
	m_iState = stateWaitingForUserRecord;

	//	Get the full user record from Aeon.

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(TABLE_ARC_USERS);
	pPayload->Insert(m_sUsernameKey);

	//	Send message

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_GET_VALUE,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			CDatum(pPayload),
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}
