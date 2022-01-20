//	MsgCreateAuthToken.cpp
//
//	Cryptosaur.createAuthToken
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command");
DECLARE_CONST_STRING(ADDRESS_CRYPTOSAUR_COMMAND,		"Cryptosaur.command");

DECLARE_CONST_STRING(FIELD_AUTH_DESC,					"authDesc");
DECLARE_CONST_STRING(FIELD_AUTH_TOKEN,					"authToken");
DECLARE_CONST_STRING(FIELD_AUTHORITY,					"authority");
DECLARE_CONST_STRING(FIELD_CREATED_ON,					"createdOn");
DECLARE_CONST_STRING(FIELD_CREDENTIALS,					"credentials");
DECLARE_CONST_STRING(FIELD_LAST_LOGIN_FAILURE_ON,		"lastLoginFailureOn");
DECLARE_CONST_STRING(FIELD_LAST_LOGIN_ON,				"lastLoginOn");
DECLARE_CONST_STRING(FIELD_LIFETIME,					"lifetime");
DECLARE_CONST_STRING(FIELD_LOGIN_FAILURE_COUNT,			"loginFailureCount");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_RIGHTS,						"rights");
DECLARE_CONST_STRING(FIELD_SCOPE,						"scope");
DECLARE_CONST_STRING(FIELD_TOKEN_ID,					"tokenID");
DECLARE_CONST_STRING(FIELD_USER_ID,						"userID");
DECLARE_CONST_STRING(FIELD_USERNAME,					"username");

DECLARE_CONST_STRING(MSG_AEON_GET_VALUE,				"Aeon.getValue");
DECLARE_CONST_STRING(MSG_AEON_INSERT_NEW,				"Aeon.insertNew");
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate");
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_USER,		"Cryptosaur.createUser");
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(MUTATE_DELETE,						"delete");
DECLARE_CONST_STRING(MUTATE_DATE_MODIFIED,				"dateModified");
DECLARE_CONST_STRING(MUTATE_INCREMENT,					"increment");

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command");

DECLARE_CONST_STRING(TABLE_ARC_USERS,					"Arc.users");

DECLARE_CONST_STRING(ERR_INVALID_PARAMS,				"Invalid parameters.");
DECLARE_CONST_STRING(ERR_INTERNAL_ERROR,				"Internal error creating an authToken.");
DECLARE_CONST_STRING(ERR_INVALID_USERNAME_OR_PASSWORD,	"Invalid username or password.");

const DWORD MESSAGE_TIMEOUT =							30 * 1000;
const DWORD DEFAULT_AUTH_TOKEN_LIFETIME =				48 * 60 * 60;

class CCreateAuthTokenSession : public ISessionHandler
	{
	public:
		CCreateAuthTokenSession (CCryptosaurEngine &Engine, const CString &sScope, const CString &sUsername, CDatum dAuthDesc) : 
				m_Engine(Engine),
				m_sScope(sScope),
				m_sUsername(sUsername),
				m_dAuthDesc(dAuthDesc)
			{ }

	protected:

		//	ISessionHandler virtuals

		virtual void OnMark (void) override { m_dAuthDesc.Mark(); }
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		enum class EState
			{
			Unknown,

			waitingForUserInfo,
			creatingUserInfo,
			updatingUserInfo,
			};

		bool CreateAuthToken ();
		bool SendCreateUser ();
		bool SendUpdateUserInfo ();
		bool SendUserGetValue (const CString &sUsernameKey);

		CCryptosaurEngine &m_Engine;
		CString m_sScope;
		CString m_sUsername;					//	Propercase username
		CDatum m_dAuthDesc;

		CString m_sAuthority;					//	NULL if local
		CString m_sUserID;						//	Username key to use in Arc.users
		DWORD m_dwTokenID = 0;					//	Token ID
		DWORD m_dwLifetime = 0;					//	Token lifetime in seconds

		EState m_iState = EState::Unknown;
	};

void CCryptosaurEngine::MsgCreateAuthToken (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateAuthToken
//
//	Cryptosaur.createAuthToken {username} {authDesc}
//
//	Authenticates the user and creates a temporary authToken for access. We 
//	support several modes of authentication, depending on the contents of 
//	authDesc.
//
//	DELEGATED AUTH
//
//	In Delegated Authentication, the caller has authenticated with a 3rd party
//	system (e.g., OAUTH) and simply needs an authToken from us.
//
//	authority: A string representing the authority that authenticated the user.
//		We generate an internal username of the form:
//
//		@authority:username
//
//	lifetime: The lifetime of the resulting authToken in seconds. We default to
//		48 hours.
//
//	scope: If the calling service is an admin service, this is a required field
//		that specifies the scope for the authToken. Otherwise, we assume the 
//		scope of the service.
//
//	RETURN
//
//	The result of this function is a user record with the following information:
//
//	userID: This is the unique key to identify the user. For local users, this 
//		is just the username in key form (lowercase). For delegated users, this
//		is the key @authority:username.
//
//	name: The display name of the user.
//
//	authToken: The authToken; callers can check with 
//		Cryptosaur.validateAuthToken

	{
	const CString &sUsername = Msg.dPayload.GetElement(0);
	CDatum dAuthDesc = Msg.dPayload.GetElement(1);

	if (sUsername.IsEmpty() || dAuthDesc.IsNil())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS, Msg);
		return;
		}

	//	Figure out the scope. For sandboxed services, we get the scope from the
	//	security info.

	CString sScope;
	if (pSecurityCtx)
		sScope = pSecurityCtx->GetSandbox();

	//	If no scope, get from parameters.
	//
	//	NOTE: A scope always ends in a dot. ValidateSandbox makes sure that we
	//	have the proper format.

	if (sScope.IsEmpty())
		sScope = ValidateSandbox(dAuthDesc.GetElement(FIELD_SCOPE));

	//	Scope is required.

	if (sScope.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS, Msg);
		return;
		}

	//	Do it.

	StartSession(Msg, new CCreateAuthTokenSession(*this, sScope, sUsername, dAuthDesc));
	}

bool CCreateAuthTokenSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)
	{
	//	If we have an authority, then this is a delegated auth.

	m_sAuthority = strToLower(m_dAuthDesc.GetElement(FIELD_AUTHORITY));
	if (!m_sAuthority.IsEmpty())
		{
		//	LATER: Move to a function.

		m_sUserID = strPattern("@%s:%s", m_sAuthority, strToLower(m_sUsername));
		}

	//	Otherwise, this is a local user.

	else
		{
		m_sUserID = strToLower(m_sUsername);
		}

	//	Lifetime

	m_dwLifetime = m_dAuthDesc.GetElement(FIELD_LIFETIME);
	if (m_dwLifetime == 0)
		m_dwLifetime = DEFAULT_AUTH_TOKEN_LIFETIME;

	//	Read the user.

	m_iState = EState::waitingForUserInfo;
	return SendUserGetValue(m_sUserID);
	}

bool CCreateAuthTokenSession::OnProcessMessage (const SArchonMessage &Msg)
	{
	switch (m_iState)
		{
		case EState::waitingForUserInfo:
			{
			//	If we get an error then something is wrong with the database.

			if (IsError(Msg))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INTERNAL_ERROR);
				return false;
				}

			//	If we get back nil then it means the user was not found.

			else if (Msg.dPayload.IsNil())
				{
				//	If we're a delegated authentication, then we should create 
				//	the user record.

				if (!m_sAuthority.IsEmpty())
					{
					m_iState = EState::creatingUserInfo;
					m_dwTokenID = 1;
					return SendCreateUser();
					}

				//	Otherwise, error

				else
					{
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_USERNAME_OR_PASSWORD);
					return false;
					}
				}

			//	Otherwise, we have it.

			else
				{
				CDatum dUserInfo = Msg.dPayload;
				CDatum dAuthDesc = dUserInfo.GetElement(FIELD_AUTH_DESC);
				
				//	Get info from the account

				m_sUsername = dUserInfo.GetElement(FIELD_USERNAME);
				m_dwTokenID = dAuthDesc.GetElement(FIELD_TOKEN_ID);

				//	Sign in

				m_iState = EState::updatingUserInfo;
				return SendUpdateUserInfo();
				}
			}

		case EState::creatingUserInfo:
			{
			if (IsError(Msg))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INTERNAL_ERROR);
				return false;
				}

			return CreateAuthToken();
			}

		case EState::updatingUserInfo:
			{
			if (IsError(Msg))
				{
				SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INTERNAL_ERROR);
				return false;
				}

			return CreateAuthToken();
			}

		default:
			throw CException(errFail);
		}
	}

bool CCreateAuthTokenSession::CreateAuthToken ()
	{
	//	Compute the token

	CDatum dAuthTokenData(CDatum::typeStruct);
	dAuthTokenData.SetElement(FIELD_USERNAME, m_sUsername);
	dAuthTokenData.SetElement(FIELD_RIGHTS, CDatum());
	dAuthTokenData.SetElement(FIELD_SCOPE, m_sScope);
	dAuthTokenData.SetElement(FIELD_TOKEN_ID, m_dwTokenID);

	CDatum dAuthToken = m_Engine.GenerateAuthToken(dAuthTokenData, m_dwLifetime);

	//	Compose a basic user record

	CDatum dReply(CDatum::typeStruct);
	dReply.SetElement(FIELD_USER_ID, m_sUserID);
	dReply.SetElement(FIELD_NAME, m_sUsername);
	dReply.SetElement(FIELD_AUTH_TOKEN, dAuthToken);

	//	Send the reply

	SendMessageReply(MSG_REPLY_DATA, dReply);
	return false;
	}

bool CCreateAuthTokenSession::SendCreateUser ()
	{
	CDatum dAuthDesc(CDatum::typeStruct);
	dAuthDesc.SetElement(FIELD_AUTHORITY, m_sAuthority);
	dAuthDesc.SetElement(FIELD_TOKEN_ID, m_dwTokenID);

	CDatum dUserRecord(CDatum::typeStruct);
	dUserRecord.SetElement(FIELD_USERNAME, m_sUsername);
	dUserRecord.SetElement(FIELD_AUTH_DESC, dAuthDesc);
	dUserRecord.SetElement(FIELD_CREATED_ON, CDatum(CDateTime(CDateTime::Now)));
	dUserRecord.SetElement(FIELD_LAST_LOGIN_ON, CDatum(CDateTime(CDateTime::Now)));

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(TABLE_ARC_USERS);
	dPayload.Append(m_sUserID);
	dPayload.Append(dUserRecord);

	SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_INSERT_NEW,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			dPayload,
			MESSAGE_TIMEOUT);

	return true;
	}

bool CCreateAuthTokenSession::SendUpdateUserInfo ()
	{
	//	Mutate the record to set the login date

	CDatum dMutation(CDatum::typeStruct);
	dMutation.SetElement(FIELD_LAST_LOGIN_FAILURE_ON, MUTATE_DELETE);
	dMutation.SetElement(FIELD_LAST_LOGIN_ON, MUTATE_DATE_MODIFIED);
	dMutation.SetElement(FIELD_LOGIN_FAILURE_COUNT, MUTATE_DELETE);

	//	Create a payload

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(TABLE_ARC_USERS);
	dPayload.Append(m_sUserID);
	dPayload.Append(CDatum());
	dPayload.Append(dMutation);

	//	Send message

	SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_MUTATE,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			dPayload,
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}

bool CCreateAuthTokenSession::SendUserGetValue (const CString &sUsernameKey)
	{
	//	Send an Aeon message to get the user record

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(TABLE_ARC_USERS);
	dPayload.Append(sUsernameKey);

	//	Send message

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_GET_VALUE,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			dPayload,
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}
