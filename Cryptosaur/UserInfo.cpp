//	UserInfo.cpp
//
//	User info routines
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")

DECLARE_CONST_STRING(FIELD_ACTUAL,						"actual")
DECLARE_CONST_STRING(FIELD_AUTH_DESC,					"authDesc")
DECLARE_CONST_STRING(FIELD_AUTH_DESC_SUFFIX,			".authDesc")
DECLARE_CONST_STRING(FIELD_AUTH_TOKEN,					"authToken")
DECLARE_CONST_STRING(FIELD_AUTH_TOKEN_INFINITE,			"authTokenInfinite")
DECLARE_CONST_STRING(FIELD_AUTH_TOKEN_LIFETIME,			"authTokenLifetime")
DECLARE_CONST_STRING(FIELD_AUTHORITY,					"authority");
DECLARE_CONST_STRING(FIELD_CHALLENGE,					"challenge")
DECLARE_CONST_STRING(FIELD_CHALLENGE_CREDENTIALS,		"challengeCredentials")
DECLARE_CONST_STRING(FIELD_CHALLENGE_EXPIRATION,		"challengeExpiration")
DECLARE_CONST_STRING(FIELD_CREDENTIALS,					"credentials")
DECLARE_CONST_STRING(FIELD_LAST_LOGIN_FAILURE_ON,		"lastLoginFailureOn")
DECLARE_CONST_STRING(FIELD_LAST_LOGIN_ON,				"lastLoginOn")
DECLARE_CONST_STRING(FIELD_LOGIN_FAILURE_COUNT,			"loginFailureCount")
DECLARE_CONST_STRING(FIELD_PASSWORD,					"password")
DECLARE_CONST_STRING(FIELD_RIGHTS,						"rights")
DECLARE_CONST_STRING(FIELD_SCOPE,						"scope")
DECLARE_CONST_STRING(FIELD_USERNAME,					"username")

DECLARE_CONST_STRING(MUTATE_DELETE,						"delete")
DECLARE_CONST_STRING(MUTATE_DATE_MODIFIED,				"dateModified")
DECLARE_CONST_STRING(MUTATE_INCREMENT,					"increment")

DECLARE_CONST_STRING(MSG_AEON_GET_VALUE,				"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CHECK_PASSWORD_SHA1,"Cryptosaur.login_SHA1")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_GET_USER,			"Cryptosaur.getUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_HAS_RIGHTS,			"Cryptosaur.hasRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_LOGIN_USER,			"Cryptosaur.loginUser")
DECLARE_CONST_STRING(MSG_ERROR_DOES_NOT_EXIST,			"Error.doesNotExist")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(USERS_TABLE_NAME,					"Arc.users")

DECLARE_CONST_STRING(ERR_FAILURE_TIMEOUT,				"Too many login failures; please wait one hour.")
DECLARE_CONST_STRING(ERR_INVALID_USERNAME_OR_PASSWORD,	"Invalid username or password.")
DECLARE_CONST_STRING(ERR_SCOPE_REQUIRED,				"Scope required.")
DECLARE_CONST_STRING(ERR_UNKNOWN_USERNAME,				"Unknown username: %s.")
DECLARE_CONST_STRING(ERR_USERNAME_TIMEOUT,				"Login failure for user %s (%d attempts).")

const int MAX_LOGIN_ATTEMPTS =							5;
const int LOGIN_TIMEOUT =								60 * 60;
const DWORD MESSAGE_TIMEOUT =							30 * 1000;
const DWORD DEFAULT_AUTH_TOKEN_TIMEOUT =				48 * 60 * 60;

class CUserInfoSession : public ISessionHandler
	{
	public:
		CUserInfoSession (CCryptosaurEngine *pEngine, const CString &sScope, const CString &sUsername, CDatum dPayload = CDatum()) : 
				m_pEngine(pEngine),
				m_sScope(sScope),
				m_sUsername(sUsername),
				m_sUsernameKey(strToLower(sUsername)),
				m_dPayload(dPayload),
				m_iState(stateWaitingForUserRecord)
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual void OnMark (void) { m_dPayload.Mark(); }
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum EStates
			{
			stateWaitingForUserRecord,		//	Waiting to read user record
			stateWaitingForSuccessUpdate,	//	Waiting to update login success
			stateWaitingForFailureUpdate,	//	Waiting to update login failure
			stateWaitingForCredentials,		//	Waiting to update login success and return credentials
			};

		CDatum CreateSanitizedUserRecord (CDatum dRecord);
		bool UpdateLoginFailure (void);
		bool UpdateLoginSuccess (EStates iNewState);

		CCryptosaurEngine *m_pEngine;
		CString m_sScope;
		CString m_sUsername;
		CString m_sUsernameKey;
		CDatum m_dPayload;
		EStates m_iState;

		//	Initialized in stateWaitingForCredentials
		DWORD m_dwAuthTokenLifetime;
		bool m_bActual;
	};

void CCryptosaurEngine::MsgCheckPasswordSHA1 (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCheckPasswordSHA1
//
//	Cryptosaur.login_SHA1 {username} {challenge} {response}

	{
	StartSession(Msg, new CUserInfoSession(this, NULL_STR, Msg.dPayload.GetElement(0).AsStringView()));
	}

void CCryptosaurEngine::MsgGetUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetUser
//
//	Cryptosaur.getUser {username}

	{
	StartSession(Msg, new CUserInfoSession(this, (pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR), Msg.dPayload.GetElement(0).AsStringView(), Msg.dPayload));
	}

void CCryptosaurEngine::MsgHasRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgHasRights
//
//	Cryptosaur.hasRights {username} {rights}

	{
	StartSession(Msg, new CUserInfoSession(this, (pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR), Msg.dPayload.GetElement(0).AsStringView(), Msg.dPayload));
	}

void CCryptosaurEngine::MsgLoginUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgLoginUser
//
//	Cryptosaur.loginUser {username} {authDesc}

	{
	StartSession(Msg, new CUserInfoSession(this, (pSecurityCtx ? pSecurityCtx->GetSandbox() : NULL_STR), Msg.dPayload.GetElement(0).AsStringView()));
	}

//	CUserInfoSession -----------------------------------------------------------

CDatum CUserInfoSession::CreateSanitizedUserRecord (CDatum dRecord)

//	CreateSanitizedUserRecord
//
//	Creates a user record suitable for returning to clients. In partincular,
//	we remove the authentication information.

	{
	int i;

	//	Create a destination

	CComplexStruct *pDest = new CComplexStruct;

	//	Copy all appropriate fields

	for (i = 0; i < dRecord.GetCount(); i++)
		{
		//	If this is an auth field, then skip it

		if (strEquals(dRecord.GetKey(i), FIELD_AUTH_DESC))
			;

		else if (strEndsWith(dRecord.GetKey(i), FIELD_AUTH_DESC_SUFFIX))
			;
		
		//	Otherwise, copy it

		else
			pDest->SetElement(dRecord.GetKey(i), dRecord.GetElement(i));
		}

	//	Done

	return CDatum(pDest);
	}

bool CUserInfoSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	We received a reply from Aeon

	{
	int i;

	//	If this is an error, then we return the error back to the client

	if (IsError(Msg))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload.AsStringView());
		return false;
		}

	//	If we're waiting for the user record, then see if we can process it now.

	if (m_iState == stateWaitingForUserRecord)
		{
		//	Cryptosaur.getUser

		if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_GET_USER))
			{
			CDatum dUserData = Msg.dPayload;

			//	If the user does not exist, then we return Nil

			if (dUserData.IsNil())
				{
				SendMessageReply(MSG_REPLY_DATA, CDatum());
				return false;
				}

			//	Generate a sanitized user record

			CComplexStruct *pReply = new CComplexStruct;
			pReply->SetElement(FIELD_USERNAME, dUserData.GetElement(FIELD_USERNAME));

			//	Sanitize rights

			CDatum dRights = dUserData.GetElement(FIELD_RIGHTS);
			if (!m_sScope.IsEmpty())
				{
				CComplexArray *pRights = new CComplexArray;

				for (i = 0; i < dRights.GetCount(); i++)
					if (strStartsWith(dRights.GetElement(i).AsStringView(), m_sScope))
						pRights->Insert(dRights.GetElement(i));

				pReply->SetElement(FIELD_RIGHTS, CDatum(pRights));
				}
			else
				pReply->SetElement(FIELD_RIGHTS, dRights);

			//	Done

			SendMessageReply(MSG_REPLY_DATA, CDatum(pReply));
			return false;
			}

		//	Otherwise, we handle the result based on the original message

		else if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_CHECK_PASSWORD_SHA1))
			{
			//	Handle invalid user the same as invalid password.

			if (Msg.dPayload.IsNil())
				{
				SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
				return false;
				}

			//	Get the parameters from the original message

			CDatum dChallenge = GetOriginalMsg().dPayload.GetElement(1);
			CDatum dResponse = GetOriginalMsg().dPayload.GetElement(2);

			//	Get the password has from the response

			CDatum dAuthDesc = Msg.dPayload.GetElement(FIELD_AUTH_DESC);
			CDatum dPasswordHash = dAuthDesc.GetElement(FIELD_CREDENTIALS);

			//	Create a response to the challenge based on the password hash that
			//	we have stored.

			CDatum dCorrect = CAI1Protocol::CreateSHAPasswordChallengeResponse(dPasswordHash, dChallenge);

			//	Compare the correct response to the actual

			if ((const CIPInteger &)dResponse == (const CIPInteger &)dCorrect)
				return UpdateLoginSuccess(stateWaitingForSuccessUpdate);
			else
				return UpdateLoginFailure();
			}

		//	Cryptosaur.hasRights

		else if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_HAS_RIGHTS))
			{
			//	Treat invalid user as no rights, so that we don't leak 
			//	user information.

			if (Msg.dPayload.IsNil())
				{
				SendMessageReply(MSG_REPLY_DATA, CDatum());
				return false;
				}

			CDatum dRights = Msg.dPayload.GetElement(FIELD_RIGHTS);
			CDatum dRightsRequired = m_dPayload.GetElement(1);

			//	Get the rights from the user

			CAttributeList Rights;
			dRights.AsAttributeList(&Rights);

			//	Check

			for (i = 0; i < dRightsRequired.GetCount(); i++)
				{
				if (!Rights.HasAttribute(dRightsRequired.GetElement(i).AsStringView()))
					{
					SendMessageReply(MSG_REPLY_DATA, CDatum());
					return false;
					}
				}

			//	We have all rights

			SendMessageReply(MSG_REPLY_DATA, CDatum(true));
			return false;
			}

		//	Cryptosaur.loginUser

		else if (strEquals(GetOriginalMsg().sMsg, MSG_CRYPTOSAUR_LOGIN_USER))
			{
			//	Handle invalid user the same as invalid password.

			if (Msg.dPayload.IsNil())
				{
				SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
				return false;
				}

			//	Get the parameters from the original message

			CDatum dRequestAuthDesc = GetOriginalMsg().dPayload.GetElement(1);
			CDatum dCredentials = dRequestAuthDesc.GetElement(FIELD_CREDENTIALS);
			CDatum dChallengeCredentials = dRequestAuthDesc.GetElement(FIELD_CHALLENGE_CREDENTIALS);
			CDatum dPassword = dRequestAuthDesc.GetElement(FIELD_PASSWORD);
			m_bActual = !dRequestAuthDesc.GetElement(FIELD_ACTUAL).IsNil();

			if (!dRequestAuthDesc.GetElement(FIELD_AUTH_TOKEN_INFINITE).IsNil())
				m_dwAuthTokenLifetime = 0;
			else
				{
				m_dwAuthTokenLifetime = (DWORD)(int)dRequestAuthDesc.GetElement(FIELD_AUTH_TOKEN_LIFETIME);
				if (m_dwAuthTokenLifetime == 0)
					m_dwAuthTokenLifetime = DEFAULT_AUTH_TOKEN_TIMEOUT;
				}

			//	If we're not actual and have no scope, then we are an admin-level
			//	service and we must get the scope from parameters.

			if (m_sScope.IsEmpty())
				m_sScope = CCryptosaurEngine::ValidateSandbox(dRequestAuthDesc.GetElement(FIELD_SCOPE).AsStringView());
			if (!m_bActual && m_sScope.IsEmpty())
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_SCOPE_REQUIRED);
				return false;
				}

			//	User data

			CDatum dUserData = Msg.dPayload;
			CDatum dAuthDesc;
			if (m_bActual)
				dAuthDesc = dUserData.GetElement(FIELD_AUTH_DESC);
			else
				dAuthDesc = dUserData.GetElement(strPattern("%s%s", m_sScope, FIELD_AUTH_DESC));

			//	If we have no authdesc, then we can't continue. This is likely 
			//	because the client is in a sandbox that the user has not registered
			//	with. We treat it the same as a username/password failure.

			if (dAuthDesc.IsNil())
				{
				SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
				return false;
				}

			//	If this record is a delegated user (i.e., some other system has 
			//	the user's credentials) then we fail.

			else if (!dAuthDesc.GetElement(FIELD_AUTHORITY).IsNil())
				{
				SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
				return false;
				}

			//	If we've failed more than 5 consecutive times, we may need to delay
			//	the next login attempt.

			if ((int)dUserData.GetElement(FIELD_LOGIN_FAILURE_COUNT) > MAX_LOGIN_ATTEMPTS)
				{
				CDateTime LastLoginFailure = dUserData.GetElement(FIELD_LAST_LOGIN_FAILURE_ON);
				CTimeSpan TimeSinceLastFailure = timeSpan(LastLoginFailure, CDateTime(CDateTime::Now));

				//	If it has not been at least 1 hour, we return an error.

				if (TimeSinceLastFailure.Days() == 0 && TimeSinceLastFailure.Seconds() < LOGIN_TIMEOUT)
					{
					//	Timeout

					SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_FAILURE_TIMEOUT);
					return false;
					}
				}

			//	If we have straight credentials, then just compare

			bool bSuccess;
			if (!dCredentials.IsNil())
				bSuccess = ((const CIPInteger &)dCredentials == (const CIPInteger &)dAuthDesc.GetElement(FIELD_CREDENTIALS));

			//	Otherwise, we compare against the challenge

			else if (!dChallengeCredentials.IsNil())
				{
				//	Get the challenge. If not provided then we get it from the user
				//	record.

				CDatum dChallenge = GetOriginalMsg().dPayload.GetElement(2);
				if (dChallenge.IsNil())
					{
					//	Get the expiration time of the challenge

					const CDateTime &Expires = dAuthDesc.GetElement(FIELD_CHALLENGE_EXPIRATION);
					if (Expires < CDateTime(CDateTime::Now))
						{
						SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
						return false;
						}

					dChallenge = dAuthDesc.GetElement(FIELD_CHALLENGE);
					}

				//	Create a response to the challenge based on the password hash that
				//	we have stored.

				CDatum dCorrectChallenge = CAI1Protocol::CreateSHAPasswordChallengeResponse(
						dAuthDesc.GetElement(FIELD_CREDENTIALS),
						dChallenge
						);

				bSuccess = ((const CIPInteger &)dChallengeCredentials == (const CIPInteger &)dCorrectChallenge);
				}

			//	Otherwise we expect a clear text password

			else if (!dPassword.IsNil())
				{
				//	We have to hash the password to compare with credentials.

				CIPInteger Credentials;
				CCryptosaurInterface::CreateCredentials(dUserData.GetElement(FIELD_USERNAME).AsStringView(), dPassword.AsStringView(), &Credentials);

				//	Compare

				bSuccess = (Credentials == (const CIPInteger &)dAuthDesc.GetElement(FIELD_CREDENTIALS));
				}
			else
				bSuccess = false;

			//	Success or failure

			if (bSuccess)
				return UpdateLoginSuccess(stateWaitingForCredentials);
			else
				return UpdateLoginFailure();
			}

		//	Can never get here.

		else
			{
			ASSERT(false);
			return false;
			}
		}

	//	Otherwise, if we're waiting for the user record update, then continue

	else if (m_iState == stateWaitingForSuccessUpdate)
		{
		//	Since we succeeded, we send the user sanitized user record back.

		SendMessageReply(MSG_REPLY_DATA, CreateSanitizedUserRecord(Msg.dPayload));
		return false;
		}

	//	If we're waiting for credentials, compose them

	else if (m_iState == stateWaitingForCredentials)
		{
		//	The mutation returns the full record

		CDatum dUserData = Msg.dPayload;

		//	Compute the result

		CComplexStruct *pAuthToken = new CComplexStruct;
		pAuthToken->SetElement(FIELD_USERNAME, dUserData.GetElement(FIELD_USERNAME));
		pAuthToken->SetElement(FIELD_RIGHTS, dUserData.GetElement(FIELD_RIGHTS));
		if (!m_bActual)
			pAuthToken->SetElement(FIELD_SCOPE, m_sScope);

		CDatum dAuthToken = m_pEngine->GenerateAuthToken(CDatum(pAuthToken), m_dwAuthTokenLifetime);

		//	Compose a basic user record

		CComplexStruct *pReply = new CComplexStruct;
		pReply->SetElement(FIELD_AUTH_TOKEN, dAuthToken);
		pReply->SetElement(FIELD_RIGHTS, dUserData.GetElement(FIELD_RIGHTS));
		pReply->SetElement(FIELD_USERNAME, dUserData.GetElement(FIELD_USERNAME));

		//	Send the reply

		SendMessageReply(MSG_REPLY_DATA, CDatum(pReply));

		//	Done

		return false;
		}

	//	Otherwise, failure

	else if (m_iState == stateWaitingForFailureUpdate)
		{
		CDatum dUserData = Msg.dPayload;

		//	If we've exceeded our limit, log it

		int iAttempts = (int)dUserData.GetElement(FIELD_LOGIN_FAILURE_COUNT);
		if (iAttempts > MAX_LOGIN_ATTEMPTS)
			GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(ERR_USERNAME_TIMEOUT, m_sUsername, iAttempts));

		//	Send a failure

		SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, ERR_INVALID_USERNAME_OR_PASSWORD);
		return false;
		}

	//	Can never get here

	else
		{
		ASSERT(false);
		return false;
		}
	}

bool CUserInfoSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start

	{
	//	Send an Aeon message to get the user record

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(USERS_TABLE_NAME);
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

bool CUserInfoSession::UpdateLoginFailure (void)

//	UpdateLoginFailure
//
//	Update the user record with the most recent failure

	{
	//	Set our state

	m_iState = stateWaitingForFailureUpdate;

	//	Mutate the record to set the login date

	CComplexStruct *pRecord = new CComplexStruct;
	pRecord->SetElement(FIELD_LOGIN_FAILURE_COUNT, CDatum((int)1));

	CComplexStruct *pMutation = new CComplexStruct;
	pMutation->SetElement(FIELD_LAST_LOGIN_FAILURE_ON, MUTATE_DATE_MODIFIED);
	pMutation->SetElement(FIELD_LOGIN_FAILURE_COUNT, MUTATE_INCREMENT);

	//	Create a payload

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(USERS_TABLE_NAME);
	pPayload->Insert(m_sUsernameKey);
	pPayload->Insert(CDatum(pRecord));
	pPayload->Insert(CDatum(pMutation));

	//	Send message

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_MUTATE,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			CDatum(pPayload),
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}

bool CUserInfoSession::UpdateLoginSuccess (EStates iNewState)

//	UpdateLoginSuccess
//
//	Update the user record with the most recent login

	{
	//	Set our state

	m_iState = iNewState;

	//	Mutate the record to set the login date

	CComplexStruct *pMutation = new CComplexStruct;
	pMutation->SetElement(FIELD_LAST_LOGIN_FAILURE_ON, MUTATE_DELETE);
	pMutation->SetElement(FIELD_LAST_LOGIN_ON, MUTATE_DATE_MODIFIED);
	pMutation->SetElement(FIELD_LOGIN_FAILURE_COUNT, MUTATE_DELETE);

	//	Create a payload

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(USERS_TABLE_NAME);
	pPayload->Insert(m_sUsernameKey);
	pPayload->Insert(CDatum());
	pPayload->Insert(CDatum(pMutation));

	//	Send message

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_MUTATE,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			CDatum(pPayload),
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}
