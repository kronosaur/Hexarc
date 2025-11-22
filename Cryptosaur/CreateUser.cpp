//	CreateUser.cpp
//
//	User creation routines
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")

DECLARE_CONST_STRING(AUTH_TYPE_SHA1,					"SHA1")

DECLARE_CONST_STRING(FIELD_ACTUAL,						"actual")
DECLARE_CONST_STRING(FIELD_AUTH_DESC,					"authDesc")
DECLARE_CONST_STRING(FIELD_CREATED_ON,					"createdOn")
DECLARE_CONST_STRING(FIELD_CREDENTIALS,					"credentials")
DECLARE_CONST_STRING(FIELD_PASSWORD,					"password")
DECLARE_CONST_STRING(FIELD_RIGHTS,						"rights")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")
DECLARE_CONST_STRING(FIELD_USER_CONTACT,				"userContact")
DECLARE_CONST_STRING(FIELD_USERNAME,					"username")

DECLARE_CONST_STRING(MSG_AEON_INSERT_NEW,				"Aeon.insertNew")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin")

DECLARE_CONST_STRING(USERS_TABLE_NAME,					"Arc.users")

DECLARE_CONST_STRING(ERR_INVALID_USERNAME,				"Invalid username: %s.")
DECLARE_CONST_STRING(ERR_USERNAME_EXISTS,				"Username already exists: %s.")

const DWORD MESSAGE_TIMEOUT =							30 * 1000;
const DWORD DEFAULT_AUTH_TOKEN_TIMEOUT =				24 * 60 * 60;

class CCreateUserSession : public ISessionHandler
	{
	public:
		CCreateUserSession (CCryptosaurEngine *pEngine, CDatum dUserRecord) : 
				m_pEngine(pEngine),
				m_dUserRecord(dUserRecord) { }

	protected:
		//	ISessionHandler virtuals
		virtual void OnMark (void) { m_dUserRecord.Mark(); }
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		CCryptosaurEngine *m_pEngine;
		CDatum m_dUserRecord;
		CString m_sUsername;
	};

void CCryptosaurEngine::MsgCreateAdmin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateAdmin
//
//	Cryptosaur.createAdmin {username} {authDesc}

	{
	//	Only services with the Arc.admin right can create an administrator
	//	account.

	if (!ValidateMessage(Msg, pSecurityCtx, true))
		return;

	//	Get some parameters

	CStringView sUsername = Msg.dPayload.GetElement(0);
	CDatum dAuthDesc = Msg.dPayload.GetElement(1);

	//	Make sure the username is valid

	CString sError;
	if (!ValidateUsername(sUsername, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Make sure authDesc is valid

	if (!ValidateAuthDescCreate(Msg, pSecurityCtx, dAuthDesc))
		return;

	//	Create an appropriate user record

	CComplexStruct *pUserRecord = new CComplexStruct;
	pUserRecord->SetElement(FIELD_USERNAME, sUsername);
	pUserRecord->SetElement(FIELD_AUTH_DESC, dAuthDesc);
	pUserRecord->SetElement(FIELD_CREATED_ON, CDatum(CDateTime(CDateTime::Now)));

	CAttributeList Rights;
	Rights.Insert(RIGHT_ARC_ADMIN);
	CDatum dRights;
	CDatum::CreateFromAttributeList(Rights, &dRights);

	pUserRecord->SetElement(FIELD_RIGHTS, dRights);

	//	Begin a session to create the user

	StartSession(Msg, new CCreateUserSession(this, CDatum(pUserRecord)));
	}

void CCryptosaurEngine::MsgCreateUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateUser
//
//	Cryptosaur.createUser {username} {authDesc} {userContact} [{rights}]

	{
	//	Any service can create a new user

	if (!ValidateMessage(Msg, pSecurityCtx, false))
		return;

	//	Get some parameters

	CStringView sUsername = Msg.dPayload.GetElement(0);
	CDatum dAuthDesc = Msg.dPayload.GetElement(1);
	CDatum dUserContact = Msg.dPayload.GetElement(2);
	CDatum dRights = Msg.dPayload.GetElement(3);

	CAttributeList Rights;
	if (dRights.GetCount() > 0)
		dRights.AsAttributeList(&Rights);

	//	Make sure the username is valid

	CString sError;
	if (!ValidateUsername(sUsername, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	If we have a plain-text password, hash it and create a new authDesc

	if (!dAuthDesc.GetElement(FIELD_PASSWORD).IsNil())
		{
		CComplexStruct *pAuthDesc = new CComplexStruct;

		pAuthDesc->SetElement(FIELD_TYPE, AUTH_TYPE_SHA1);
		pAuthDesc->SetElement(FIELD_ACTUAL, CDatum(true));

		CIPInteger Credentials;
		CCryptosaurInterface::CreateCredentials(sUsername, dAuthDesc.GetElement(FIELD_PASSWORD).AsStringView(), &Credentials);

		CDatum dCredentials;
		CDatum::CreateIPIntegerFromHandoff(Credentials, &dCredentials);
		pAuthDesc->SetElement(FIELD_CREDENTIALS, dCredentials);

		dAuthDesc = CDatum(pAuthDesc);
		}

	//	Otherwise, make sure authDesc is valid

	else
		{
		if (!ValidateAuthDescCreate(Msg, pSecurityCtx, dAuthDesc))
			return;
		}

	//	Make sure the service is allowed to give these rights.

	if (!ValidateRightsGrant(Msg, pSecurityCtx, Rights))
		return;

	//	Convert back from attribute list because we might have removed some 
	//	duplicate or invalid rights.

	CDatum::CreateFromAttributeList(Rights, &dRights);

	//	Create an appropriate user record

	CComplexStruct *pUserRecord = new CComplexStruct;
	pUserRecord->SetElement(FIELD_USERNAME, sUsername);
	pUserRecord->SetElement(FIELD_AUTH_DESC, dAuthDesc);
	pUserRecord->SetElement(FIELD_RIGHTS, dRights);
	pUserRecord->SetElement(FIELD_USER_CONTACT, dUserContact);
	pUserRecord->SetElement(FIELD_CREATED_ON, CDatum(CDateTime(CDateTime::Now)));

	//	Begin a session to create the user

	StartSession(Msg, new CCreateUserSession(this, CDatum(pUserRecord)));
	}

//	CCreateUserSession ---------------------------------------------------------

bool CCreateUserSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a reply

	{
	//	If we get Error.alreadyExists then it means that the username already
	//	exists. We return the appropriate error.

	if (strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
		{
		SendMessageReplyError(MSG_ERROR_ALREADY_EXISTS, strPattern(ERR_USERNAME_EXISTS, m_sUsername));
		return false;
		}

	//	If another error, return it too

	else if (IsError(Msg))
		{
		SendMessageReplyError(Msg.sMsg, Msg.dPayload.GetElement(0).AsStringView());
		return false;
		}

	//	Otherwise we succeeded

	else
		{
		CComplexStruct *pAuthToken = new CComplexStruct;
		pAuthToken->SetElement(FIELD_USERNAME, m_sUsername);

		//	Generate an authToken

		CDatum dAuthToken = m_pEngine->GenerateAuthToken(CDatum(pAuthToken), DEFAULT_AUTH_TOKEN_TIMEOUT);

		//	Send the reply

		SendMessageReply(MSG_REPLY_DATA, dAuthToken);

		//	If this is the first admin, then we also take this opportunity to
		//	set the admin-exists flag. This will also send out the onStart
		//	message.

		if (!m_pEngine->DoesAdminExist())
			m_pEngine->SetAdminExists(true);

		//	No reply expected

		return false;
		}
	}

bool CCreateUserSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session.

	{
	//	Get the username out of the record. We save it because we may need it
	//	later to report errors.

	m_sUsername = m_dUserRecord.GetElement(FIELD_USERNAME).AsStringView();

	//	Send an Aeon message to insert the user record

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(USERS_TABLE_NAME);
	pPayload->Insert(strToLower(m_sUsername));
	pPayload->Insert(m_dUserRecord);

	//	Send message

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_INSERT_NEW,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			CDatum(pPayload),
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}
