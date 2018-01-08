//	RequestLogin.cpp
//
//	Cryptosaur.requestLogin
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")

DECLARE_CONST_STRING(AUTH_TYPE_SHA1,					"SHA1")

DECLARE_CONST_STRING(FIELD_AUTH_DESC,					"authDesc")
DECLARE_CONST_STRING(FIELD_CHALLENGE,					"challenge")
DECLARE_CONST_STRING(FIELD_CHALLENGE_EXPIRATION,		"challengeExpiration")
DECLARE_CONST_STRING(FIELD_CREDENTIALS,					"credentials")
DECLARE_CONST_STRING(FIELD_EMAIL,						"email")
DECLARE_CONST_STRING(FIELD_NEW_PASSWORD,				"newPassword")
DECLARE_CONST_STRING(FIELD_PASSWORD,					"password")
DECLARE_CONST_STRING(FIELD_RIGHTS,						"rights")
DECLARE_CONST_STRING(FIELD_USER_CONTACT,				"userContact")
DECLARE_CONST_STRING(FIELD_USERNAME,					"username")

DECLARE_CONST_STRING(MSG_AEON_GET_VALUE,				"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ADD_RIGHTS,			"Cryptosaur.addRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CHANGE_PASSWORD,	"Cryptosaur.changePassword")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_REMOVE_RIGHTS,		"Cryptosaur.removeRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_RESET_PASSWORD_MANUAL,	"Cryptosaur.resetPasswordManual")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_ERROR_DOES_NOT_EXIST,			"Error.doesNotExist")
DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(MUTATE_ADD_TO_SET,					"addToSet")
DECLARE_CONST_STRING(MUTATE_REMOVE_FROM_SET,			"removeFromSet")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(TABLE_ARC_USERS,					"Arc.users")

DECLARE_CONST_STRING(ERR_CREDENTIALS_ALREADY_EXIST,		"%s already exists.")
DECLARE_CONST_STRING(ERR_INVALID_AUTHORIZATION,			"Invalid authorization from user: %s.")
DECLARE_CONST_STRING(ERR_INVALID_USERNAME,				"Invalid username.")
DECLARE_CONST_STRING(ERR_INVALID_USERNAME_OR_PASSWORD,	"Invalid username or password.")
DECLARE_CONST_STRING(ERR_NULL_PASSWORD,					"Password cannot be empty.")
DECLARE_CONST_STRING(ERR_SCOPE_REQUIRED,				"Scope required.")
DECLARE_CONST_STRING(ERR_UNKNOWN_USERNAME,				"Unknown username: %s.")

const DWORD MESSAGE_TIMEOUT =							30 * 1000;
const int CHALLENGE_LIFETIME =							10 * 60 * 1000;
const DWORD DEFAULT_AUTH_TOKEN_TIMEOUT =				24 * 60 * 60;

class CChangeUserSession : public ISessionHandler
	{
	public:
		CChangeUserSession (CCryptosaurEngine *pEngine, const CString &sScope, const CString &sUsername, CDatum dPayload = CDatum()) : 
				m_pEngine(pEngine),
				m_iState(stateUnknown),
				m_sScope(sScope),
				m_sUsername(sUsername),
				m_sUsernameKey(strToLower(sUsername)),
				m_dPayload(dPayload) 
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual void OnMark (void) { m_dReply.Mark(); m_dPayload.Mark(); }
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
		CString m_sScope;
		CString m_sUsername;
		CString m_sUsernameKey;
		CDatum m_dPayload;

		CDatum m_dReply;
	};

void CCryptosaurEngine::MsgAddRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAddRights
//
//	Cryptosaur.addRights {username} {rights}

	{
	const CString &sUsername = Msg.dPayload.GetElement(0);
	CDatum dRights = Msg.dPayload.GetElement(1);

	CAttributeList Rights;
	dRights.AsAttributeList(&Rights);

	//	Make sure that the service is allowed to add these rights

	if (!ValidateRightsGrant(Msg, pSecurityCtx, Rights))
		return;

	//	Make sure we have a username

	if (sUsername.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_USERNAME, Msg);
		return;
		}

	//	Make sure rights are lowercase (because we are case-insensitive)

	CComplexArray *pNewPayload = new CComplexArray;
	pNewPayload->Insert(Msg.dPayload.GetElement(0));
	
	CDatum::CreateFromAttributeList(Rights, &dRights);
	pNewPayload->Insert(dRights);

	//	Begin a session

	StartSession(Msg, new CChangeUserSession(this, 
			(pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR),
			sUsername,
			CDatum(pNewPayload)
			));
	}

void CCryptosaurEngine::MsgChangePassword (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgChangePassword
//
//	Cryptosaur.changePassword {username} {oldAuthDesc} {newAuthDesc}

	{
	//	Any service can request a password change

	if (!ValidateMessage(Msg, pSecurityCtx, false))
		return;

	//	Make sure we have a username

	const CString &sUsername = Msg.dPayload.GetElement(0);
	if (sUsername.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_USERNAME, Msg);
		return;
		}

	//	Begin a session to create the challenge

	StartSession(Msg, new CChangeUserSession(this, 
			(pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR),
			sUsername,
			Msg.dPayload
			));
	}

void CCryptosaurEngine::MsgRemoveRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRemoveRights
//
//	Cryptosaur.removeRights {username} {rights}

	{
	const CString &sUsername = Msg.dPayload.GetElement(0);
	CDatum dRights = Msg.dPayload.GetElement(1);

	CAttributeList Rights;
	dRights.AsAttributeList(&Rights);

	//	Make sure that the service is allowed to add these rights

	if (!ValidateRightsGrant(Msg, pSecurityCtx, Rights))
		return;

	//	Make sure we have a username

	if (sUsername.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_USERNAME, Msg);
		return;
		}

	//	Make sure rights are lowercase (because we are case-insensitive)

	CComplexArray *pNewPayload = new CComplexArray;
	pNewPayload->Insert(Msg.dPayload.GetElement(0));
	
	CDatum::CreateFromAttributeList(Rights, &dRights);
	pNewPayload->Insert(dRights);

	//	Begin a session

	StartSession(Msg, new CChangeUserSession(this, 
			(pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR),
			sUsername,
			CDatum(pNewPayload)
			));
	}

void CCryptosaurEngine::MsgRequestLogin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRequestLogin
//
//	Cryptosaur.requestLogin {username}

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

	//	Begin a session to change the user record

	StartSession(Msg, new CChangeUserSession(this, 
			(pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR),
			sUsername,
			Msg.dPayload
			));
	}

void CCryptosaurEngine::MsgResetPasswordManual (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgResetPasswordManual
//
//	Cryptosaur.resetPasswordManual {username}

	{
	//	Must be an administrator service

	if (!ValidateMessage(Msg, pSecurityCtx, true))
		return;

	//	Make sure we have a username

	const CString &sUsername = Msg.dPayload.GetElement(0);
	if (sUsername.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_USERNAME, Msg);
		return;
		}

	//	Begin a session to change the password

	StartSession(Msg, new CChangeUserSession(this, 
			(pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR),
			sUsername,
			Msg.dPayload
			));
	}

bool CChangeUserSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	We received a reply from Aeon

	{
	int i;

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

			//	Is this a change password request?

			else if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_CHANGE_PASSWORD))
				{
				CDatum dCurrentAuthDesc = dUserData.GetElement(FIELD_AUTH_DESC);

				CDatum dOldAuthDesc = m_dPayload.GetElement(1);
				CDatum dNewAuthDesc = m_dPayload.GetElement(2);

				//	Get the credentials (we accept hashed credentials or a 
				//	clear-text password).

				CDatum dOldCredentials = dOldAuthDesc.GetElement(FIELD_CREDENTIALS);
				if (dOldCredentials.IsNil())
					{
					CDatum dPassword = dOldAuthDesc.GetElement(FIELD_PASSWORD);
					if (dPassword.IsNil())
						{
						SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
						return false;
						}

					CIPInteger Credentials;
					CCryptosaurInterface::CreateCredentials(m_sUsername, dPassword, &Credentials);
					CDatum::CreateIPIntegerFromHandoff(Credentials, &dOldCredentials);
					}

				CDatum dNewCredentials = dNewAuthDesc.GetElement(FIELD_CREDENTIALS);
				if (dNewCredentials.IsNil())
					{
					CDatum dPassword = dNewAuthDesc.GetElement(FIELD_PASSWORD);
					if (dPassword.IsNil())
						{
						SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NULL_PASSWORD);
						return false;
						}

					CIPInteger Credentials;
					CCryptosaurInterface::CreateCredentials(m_sUsername, dPassword, &Credentials);
					CDatum::CreateIPIntegerFromHandoff(Credentials, &dNewCredentials);
					}

				//	Validate the credentials given by the person who wants to change the password

				if ((const CIPInteger &)dOldCredentials != (const CIPInteger &)dCurrentAuthDesc.GetElement(FIELD_CREDENTIALS))
					{
					SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
					return false;
					}

				//	Generate an auth token. We require actual for changing
				//	the password, so we return an actual auth token (i.e., no
				//	scope).

				CComplexStruct *pAuthToken = new CComplexStruct;
				pAuthToken->SetElement(FIELD_USERNAME, dUserData.GetElement(FIELD_USERNAME));

				m_dReply = m_pEngine->GenerateAuthToken(CDatum(pAuthToken), DEFAULT_AUTH_TOKEN_TIMEOUT);

				//	Update the credentials

				CComplexStruct *pNewAuthDesc = new CComplexStruct(dCurrentAuthDesc);
				pNewAuthDesc->SetElement(FIELD_CREDENTIALS, dNewCredentials);

				CComplexStruct *pNewUserData = new CComplexStruct;
				pNewUserData->SetElement(FIELD_AUTH_DESC, CDatum(pNewAuthDesc));

				//	Mutate the existing user data record to just replace the authDesc field

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

			//	Is this a reset password request?

			else if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_RESET_PASSWORD_MANUAL))
				{
				CDatum dCurrentAuthDesc = dUserData.GetElement(FIELD_AUTH_DESC);

				//	Generate a new (random) password (with 8 characters)

				CString sPassword = cryptoRandomUserPassword(8);

				//	Hash the password to create credentials

				CIPInteger Credentials;
				CCryptosaurInterface::CreateCredentials(m_sUsername, sPassword, &Credentials);
				CDatum dCredentials;
				CDatum::CreateIPIntegerFromHandoff(Credentials, &dCredentials);

				//	Compose a reply in the event that we succeed in updating 
				//	the user record.

				CComplexStruct *pReply = new CComplexStruct;
				pReply->SetElement(FIELD_USERNAME, m_sUsername);
				pReply->SetElement(FIELD_EMAIL, dUserData.GetElement(FIELD_USER_CONTACT).GetElement(FIELD_EMAIL));
				pReply->SetElement(FIELD_NEW_PASSWORD, sPassword);
				m_dReply = CDatum(pReply);

				//	Update the credentials

				CComplexStruct *pNewAuthDesc = new CComplexStruct(dCurrentAuthDesc);
				pNewAuthDesc->SetElement(FIELD_CREDENTIALS, dCredentials);

				CComplexStruct *pNewUserData = new CComplexStruct;
				pNewUserData->SetElement(FIELD_AUTH_DESC, CDatum(pNewAuthDesc));

				//	Mutate the existing user data record to just replace the authDesc field

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

			//	addRights

			else if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_ADD_RIGHTS))
				{
				CDatum dRightsToAdd = m_dPayload.GetElement(1);
				CDatum dCurrentRights = dUserData.GetElement(FIELD_RIGHTS);

				//	Optimistically combine the rights (technically it is 
				//	possible to have an update between our last read and the 
				//	mutation, but that's OK since what we're returning is not
				//	meant to be definitive [the rights stored in the record
				//	are definitive.])

				CComplexArray *pNewRights = new CComplexArray(dCurrentRights);
				for (i = 0; i < dRightsToAdd.GetCount(); i++)
					{
					CDatum dRight = dRightsToAdd.GetElement(i);

					//	Add it if not already there

					if (!pNewRights->FindElement(dRight))
						pNewRights->Insert(dRight);
					}

				m_dReply = CDatum(pNewRights);

				//	Mutate

				CComplexStruct *pNewValue = new CComplexStruct;
				pNewValue->SetElement(FIELD_RIGHTS, dRightsToAdd);

				CComplexStruct *pMutation = new CComplexStruct;
				pMutation->SetElement(FIELD_RIGHTS, MUTATE_ADD_TO_SET);

				CComplexArray *pPayload = new CComplexArray;
				pPayload->Insert(TABLE_ARC_USERS);
				pPayload->Insert(m_sUsernameKey);
				pPayload->Insert(CDatum(pNewValue));
				pPayload->Insert(CDatum(pMutation));

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

			//	removeRights

			else if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_REMOVE_RIGHTS))
				{
				CDatum dRightsToRemove = m_dPayload.GetElement(1);
				CDatum dCurrentRights = dUserData.GetElement(FIELD_RIGHTS);

				//	Optimistically combine the rights (technically it is 
				//	possible to have an update between our last read and the 
				//	mutation, but that's OK since what we're returning is not
				//	meant to be definitive [the rights stored in the record
				//	are definitive.])

				CComplexArray *pNewRights = new CComplexArray(dCurrentRights);
				for (i = 0; i < dRightsToRemove.GetCount(); i++)
					{
					CDatum dRight = dRightsToRemove.GetElement(i);

					//	Add it if not already there

					int iIndex;
					if (pNewRights->FindElement(dRight, &iIndex))
						pNewRights->Delete(iIndex);
					}

				m_dReply = CDatum(pNewRights);

				//	Mutate

				CComplexStruct *pNewValue = new CComplexStruct;
				pNewValue->SetElement(FIELD_RIGHTS, dRightsToRemove);

				CComplexStruct *pMutation = new CComplexStruct;
				pMutation->SetElement(FIELD_RIGHTS, MUTATE_REMOVE_FROM_SET);

				CComplexArray *pPayload = new CComplexArray;
				pPayload->Insert(TABLE_ARC_USERS);
				pPayload->Insert(m_sUsernameKey);
				pPayload->Insert(CDatum(pNewValue));
				pPayload->Insert(CDatum(pMutation));

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

			//	Otherwise, a request login

			else
				{
				bool bActual = (!m_dPayload.GetElement(1).IsNil() && !m_sScope.IsEmpty());

				//	Get the authDesc that we need to modify

				CString sAuthDescField;
				if (bActual)
					sAuthDescField = FIELD_AUTH_DESC;
				else
					sAuthDescField = strPattern("%s%s", m_sScope, FIELD_AUTH_DESC);

				//	Generate a challenge

				m_dReply = CAI1Protocol::CreateSHAPasswordChallenge();

				//	Clone the user data and set the element

				CComplexStruct *pNewUserData = new CComplexStruct;

				CComplexStruct *pAuthDesc = new CComplexStruct(dUserData.GetElement(sAuthDescField));
				pAuthDesc->SetElement(FIELD_CHALLENGE, m_dReply);
				pAuthDesc->SetElement(FIELD_CHALLENGE_EXPIRATION, CDatum(timeAddTime(CDateTime(CDateTime::Now), CTimeSpan(CHALLENGE_LIFETIME))));
				pNewUserData->SetElement(sAuthDescField, CDatum(pAuthDesc));

				//	Store it back in Aeon (we overwrite only the authDesc field).

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
			}

		case stateWaitingForUpdate:
			{
			//	Since we're succesful, return the result

			SendMessageReply(MSG_REPLY_DATA, m_dReply);

			//	Done

			return false;
			}

		default:
			return false;
		}
	}

bool CChangeUserSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

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
