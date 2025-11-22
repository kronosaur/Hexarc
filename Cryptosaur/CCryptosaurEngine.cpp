//	CCryptosaurEngine.cpp
//
//	CCryptosaurEngine class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_CRYPTOSAUR_COMMAND,		"Cryptosaur.command@~/~")
DECLARE_CONST_STRING(ADDRESS_CRYPTOSAUR_NOTIFY,			"Cryptosaur.notify")

DECLARE_CONST_STRING(AUTH_TYPE_SHA1,					"SHA1")

DECLARE_CONST_STRING(ENGINE_NAME_CRYPTOSAUR,			"Cryptosaur")

DECLARE_CONST_STRING(FIELD_ACTUAL,						"actual")
DECLARE_CONST_STRING(FIELD_AUTH_DESC,					"authDesc")
DECLARE_CONST_STRING(FIELD_AUTH_TOKEN,					"authToken")
DECLARE_CONST_STRING(FIELD_CREDENTIALS,					"credentials")
DECLARE_CONST_STRING(FIELD_SCOPE,						"scope")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")
DECLARE_CONST_STRING(FIELD_USERNAME,					"username")

DECLARE_CONST_STRING(KEY_CRYPTOSAUR_AUTH_TOKEN,			"Cryptosaur.authToken01")

DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(VIRTUAL_PORT_AEON_NOTIFY,			"Aeon.notify")
DECLARE_CONST_STRING(VIRTUAL_PORT_CRYPTOSAUR_COMMAND,	"Cryptosaur.command")

DECLARE_CONST_STRING(ERR_AUTHDESC_ACTUAL_REQUIRED,		"AuthDesc credentials must be for actual.")
DECLARE_CONST_STRING(ERR_AUTHDESC_CREDENTIALS_REQUIRED,	"AuthDesc credentials required.")
DECLARE_CONST_STRING(ERR_INVALID_AUTHDESC_TYPE,			"Invalid authDesc type: %s.")
DECLARE_CONST_STRING(ERR_INVALID_AUTH_TOKEN,			"Invalid or expired authToken.")
DECLARE_CONST_STRING(ERR_CANNOT_GRANT_RIGHT,			"Service not authorized to grant right: %s")
DECLARE_CONST_STRING(ERR_NOT_ADMIN,						"Service does not have Arc.admin right.")
DECLARE_CONST_STRING(ERR_MUST_HAVE_PRINTABLE_CHARS,		"Username must have at least three printable characters.")
DECLARE_CONST_STRING(ERR_CANT_HAVE_CONTROL_CHARS,		"Username may not have control characters.")
DECLARE_CONST_STRING(ERR_INVALID_WHITESPACE,			"Username may not have leading or trailing whitespace.")
DECLARE_CONST_STRING(ERR_TOO_MUCH_WHITESPACE,			"Username may not have more than one whitespace character between words.")
DECLARE_CONST_STRING(ERR_INVALID_CHAR,					"Username may not contain '{', '}', or '\\'.")
DECLARE_CONST_STRING(ERR_INVALID_LEADING_CHAR,			"Username may not start with '#', '&', or '['.")
DECLARE_CONST_STRING(ERR_WAITING_FOR_AEON,				"Waiting for AeonDB to start up.")

//	Message Table --------------------------------------------------------------

DECLARE_CONST_STRING(MSG_AEON_ON_START,					"Aeon.onStart")
DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ADD_RIGHTS,			"Cryptosaur.addRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CHANGE_PASSWORD,	"Cryptosaur.changePassword")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_ADMIN,		"Cryptosaur.createAdmin")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_AUTH_TOKEN,	"Cryptosaur.createAuthToken")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_SCOPED_CREDENTIALS,	"Cryptosaur.createScopedCredentials")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CREATE_USER,		"Cryptosaur.createUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_DELETE_USER,		"Cryptosaur.deleteUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_GET_CERTIFICATE,	"Cryptosaur.getCertificate")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_GET_KEY,			"Cryptosaur.getKey")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_GET_USER,			"Cryptosaur.getUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_LIST_KEYS,			"Cryptosaur.listKeys")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_CHECK_PASSWORD_SHA1,"Cryptosaur.login_SHA1")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_HAS_RIGHTS,			"Cryptosaur.hasRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_LOGIN_USER,			"Cryptosaur.loginUser")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_REMOVE_RIGHTS,		"Cryptosaur.removeRights")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_REQUEST_LOGIN,		"Cryptosaur.requestLogin")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_RESET_PASSWORD_MANUAL,	"Cryptosaur.resetPasswordManual")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_SET_CERTIFICATE,	"Cryptosaur.setCertificate")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_SET_KEY,			"Cryptosaur.setKey")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_SIGN_DATA,			"Cryptosaur.signData")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_VALIDATE_AUTH_TOKEN,"Cryptosaur.validateAuthToken")

CCryptosaurEngine::SMessageHandler CCryptosaurEngine::m_MsgHandlerList[] =
	{
		//	Aeon.onStart
		{	MSG_AEON_ON_START,							&CCryptosaurEngine::MsgAeonOnStart },

		//	Arc.housekeeping
		{	MSG_ARC_HOUSEKEEPING,						&TSimpleEngine::MsgNull },

		//	Cryptosaur.addRights {username} {rights}
		{	MSG_CRYPTOSAUR_ADD_RIGHTS,					&CCryptosaurEngine::MsgAddRights },

		//	Cryptosaur.changePassword {username} {oldAuthDesc} {newAuthDesc}
		{	MSG_CRYPTOSAUR_CHANGE_PASSWORD,				&CCryptosaurEngine::MsgChangePassword },

		//	Cryptosaur.login_SHA1 {username} {challenge} {response}
		{	MSG_CRYPTOSAUR_CHECK_PASSWORD_SHA1,			&CCryptosaurEngine::MsgCheckPasswordSHA1 },

		//	Cryptosaur.createAdmin {username} {authDesc}
		{	MSG_CRYPTOSAUR_CREATE_ADMIN,				&CCryptosaurEngine::MsgCreateAdmin },

		//	Cryptosaur.createAuthToken {username} {authDesc}
		{	MSG_CRYPTOSAUR_CREATE_AUTH_TOKEN,			&CCryptosaurEngine::MsgCreateAuthToken },

		//	Cryptosaur.createScopedCredentials {username} {authDesc}
		{	MSG_CRYPTOSAUR_CREATE_SCOPED_CREDENTIALS,	&CCryptosaurEngine::MsgCreateScopedCredentials },

		//	Cryptosaur.createUser {username} {authDesc}
		{	MSG_CRYPTOSAUR_CREATE_USER,					&CCryptosaurEngine::MsgCreateUser },

		//	Cryptosaur.deleteUser {username}
		{	MSG_CRYPTOSAUR_DELETE_USER,					&CCryptosaurEngine::MsgDeleteUser },

		//	Cryptosaur.getCertificate {type} {name}
		{	MSG_CRYPTOSAUR_GET_CERTIFICATE,				&CCryptosaurEngine::MsgGetCertificate },

		//	Cryptosaur.getKey {keyName}
		{	MSG_CRYPTOSAUR_GET_KEY,						&CCryptosaurEngine::MsgGetKey },

		//	Cryptosaur.getUser {username}
		{	MSG_CRYPTOSAUR_GET_USER,					&CCryptosaurEngine::MsgGetUser },

		//	Cryptosaur.hasRights {username} {rights}
		{	MSG_CRYPTOSAUR_HAS_RIGHTS,					&CCryptosaurEngine::MsgHasRights },

		//	Cryptosaur.listKeys
		{	MSG_CRYPTOSAUR_LIST_KEYS,					&CCryptosaurEngine::MsgListKeys },

		//	Cryptosaur.loginUser {username} {authDesc}
		{	MSG_CRYPTOSAUR_LOGIN_USER,					&CCryptosaurEngine::MsgLoginUser },

		//	Cryptosaur.removeRights {username} {rights}
		{	MSG_CRYPTOSAUR_REMOVE_RIGHTS,				&CCryptosaurEngine::MsgRemoveRights },

		//	Cryptosaur.requestLogin {username}
		{	MSG_CRYPTOSAUR_REQUEST_LOGIN,				&CCryptosaurEngine::MsgRequestLogin },

		//	Cryptosaur.resetPasswordManual {username}
		{	MSG_CRYPTOSAUR_RESET_PASSWORD_MANUAL,		&CCryptosaurEngine::MsgResetPasswordManual },

		//	Cryptosaur.setCertificate {type} {data}
		{	MSG_CRYPTOSAUR_SET_CERTIFICATE,				&CCryptosaurEngine::MsgSetCertificate },

		//	Cryptosaur.setKey {type} {data}
		{	MSG_CRYPTOSAUR_SET_KEY,						&CCryptosaurEngine::MsgSetKey },

		//	Cryptosaur.signData {keyName} {data}
		{	MSG_CRYPTOSAUR_SIGN_DATA,					&CCryptosaurEngine::MsgSignData },

		//	Cryptosaur.validateAuthToken {authToken}
		{	MSG_CRYPTOSAUR_VALIDATE_AUTH_TOKEN,			&CCryptosaurEngine::MsgValidateAuthToken },
	};

int CCryptosaurEngine::m_iMsgHandlerListCount = SIZEOF_STATIC_ARRAY(CCryptosaurEngine::m_MsgHandlerList);

CCryptosaurEngine::CCryptosaurEngine (void) : TSimpleEngine(ENGINE_NAME_CRYPTOSAUR),
		m_bAeonInitialized(false),
		m_bAdminExists(false)

//	CCryptosaurEngine constructor

	{
	}

CCryptosaurEngine::~CCryptosaurEngine (void)

//	CCryptosaurEngine destructor

	{
	}

CDatum CCryptosaurEngine::GenerateAuthToken (CDatum dData, DWORD dwLifetime)

//	GenerateAuthToken
//
//	Generates an authToken for the given user, scope, and lifetime.
//
//	If dwLifetime == 0 then the token does not expire.

	{
	//	Calculate the expiration time

	CDateTime ExpireTime;
	if (dwLifetime > 0)
		{
		DWORD dwLifetimeDays = dwLifetime / SECONDS_PER_DAY;
		DWORD dwLifetimeSeconds = dwLifetime % SECONDS_PER_DAY;
		ExpireTime = timeAddTime(CDateTime(CDateTime::Now), CTimeSpan(dwLifetimeDays, dwLifetimeSeconds * 1000));
		}
	else
		ExpireTime = CDateTime(CDateTime::BeginningOfTime);

	//	Get the key to sign with (the key is guaranteed to exist because we
	//	checked at boot time).

	CIPInteger *pAuthTokenKey = m_Keys.GetAt(KEY_CRYPTOSAUR_AUTH_TOKEN);

	//	Create the token

	CString sToken;
	CCryptosaurInterface::CreateAuthToken(dData, ExpireTime, *pAuthTokenKey, &sToken);

	return CDatum(sToken);
	}

void CCryptosaurEngine::InsertKey (const CString &sKeyName, CDatum dKey)

//	InsertKey
//
//	Insert a key to our cache

	{
	CSmartLock Lock(m_cs);
	
	if (dKey.GetBasicType() == CDatum::typeIntegerIP)
		m_Keys.SetAt(sKeyName, dKey); 
	else
		m_ExternalKeys.SetAt(sKeyName, dKey.AsString());
	}

bool CCryptosaurEngine::IsRunning (CString *retsError)

//	IsRunning
//
//	Returns TRUE if the engine is accepting commands. Otherwise it return with
//	a reason why it cannot accept the command.

	{
	if (!m_bAeonInitialized)
		{
		if (retsError)
			*retsError = ERR_WAITING_FOR_AEON;
		return false;
		}

	//	Running

	return true;
	}

void CCryptosaurEngine::MsgListKeys (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgListKeys
//
//	Cryptosaur.listKeys

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Generate a list of keys in sorted order

	CSmartLock Lock(m_cs);
	TArray<CString> Keys;
	for (int i = 0; i < m_Keys.GetCount(); i++)
		Keys.Insert(m_Keys.GetKey(i));

	for (int i = 0; i < m_ExternalKeys.GetCount(); i++)
		Keys.Insert(m_ExternalKeys.GetKey(i));

	Lock.Unlock();
	Keys.Sort();

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(Keys.GetCount());
	for (int i = 0; i < Keys.GetCount(); i++)
		dResult.Append(Keys[i]);

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CCryptosaurEngine::MsgValidateAuthToken (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgValidateAuthToken
//
//	Cryptosaur.validateAuthToken {authToken}

	{
	//	Any service can validate a token

	if (!ValidateMessage(Msg, pSecurityCtx, false))
		return;

	//	Get the key to sign with (the key is guaranteed to exist because we
	//	checked at boot time).

	CIPInteger *pAuthTokenKey = m_Keys.GetAt(KEY_CRYPTOSAUR_AUTH_TOKEN);

	//	Validate

	CDatum dData;
	if (!CCryptosaurInterface::ValidateAuthToken(Msg.dPayload.GetElement(0).AsStringView(), *pAuthTokenKey, &dData))
		{
		SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);
		return;
		}

	//	A sandboxed authtoken is not valid outside of its scope.
	//	(But it is valid in admin services).

	CStringView sScope = dData.GetElement(FIELD_SCOPE);
	if (!sScope.IsEmpty() 
			&& pSecurityCtx
			&& !pSecurityCtx->IsNamespaceAccessible(sScope))
		{
		SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);
		return;
		}

	//	Return the data in the auth token

	SendMessageReply(MSG_REPLY_DATA, dData, Msg);
	}

void CCryptosaurEngine::OnBoot (void)

//	OnBoot
//
//	Boot

	{
	//	Register our command port

	AddPort(ADDRESS_CRYPTOSAUR_COMMAND);
	AddVirtualPort(VIRTUAL_PORT_CRYPTOSAUR_COMMAND, ADDRESS_CRYPTOSAUR_COMMAND, FLAG_PORT_NEAREST);

	//	Subscribe to Aeon notifications

	AddVirtualPort(VIRTUAL_PORT_AEON_NOTIFY, ADDRESS_CRYPTOSAUR_COMMAND, FLAG_PORT_ALWAYS);
	}

void CCryptosaurEngine::OnMarkEx (void)

//	OnMarkEx
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Certificates.GetCount(); i++)
		m_Certificates[i].Mark();
	}

void CCryptosaurEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Start running

	{
	}

void CCryptosaurEngine::OnStopRunning (void)

//	OnStopRunning
//
//	Stop running

	{
	}

bool CCryptosaurEngine::ValidateAuthDescActual (CDatum dAuthDesc, const CString &sScope, CDatum dUserData)

//	ValidateAuthDescActual
//
//	Validates that dAuthDesc is valid authorization from the actual user.

	{
	CDatum dAuthToken = dAuthDesc.GetElement(FIELD_AUTH_TOKEN);
	CDatum dCredentials = dAuthDesc.GetElement(FIELD_CREDENTIALS);

	//	Do we have an authToken?

	if (!dAuthToken.IsNil())
		{
		//	Get the key to sign with (the key is guaranteed to exist because we
		//	checked at boot time).

		CIPInteger *pAuthTokenKey = m_Keys.GetAt(KEY_CRYPTOSAUR_AUTH_TOKEN);

		//	Validate

		CDatum dData;
		if (!CCryptosaurInterface::ValidateAuthToken(dAuthToken.AsStringView(), *pAuthTokenKey, &dData))
			return false;

		//	The proper user?

		if (!strEquals(strToLower(dData.GetElement(FIELD_USERNAME).AsStringView()), strToLower(dUserData.GetElement(FIELD_USERNAME).AsStringView())))
			return false;
		
		//	AuthToken for actual?

		if (!dData.GetElement(FIELD_SCOPE).IsNil())
			return false;

		//	OK

		return true;
		}

	//	Otherwise, we better have credentials

	else if (!dCredentials.IsNil())
		{
		//	Compare credentials against user record

		CDatum dTrueAuthDesc = dUserData.GetElement(FIELD_AUTH_DESC);
		if (!((const CIPInteger &)dCredentials == (const CIPInteger &)dTrueAuthDesc.GetElement(FIELD_CREDENTIALS)))
			return false;

		//	OK

		return true;
		}

	//	Otherwise we fail.

	else
		return false;
	}

bool CCryptosaurEngine::ValidateAuthDescCreate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, CDatum dAuthDesc)

//	ValidateAuthDescCreate
//
//	Make sure that the authDesc structure has the proper fields for creating
//	a new user (either admin or not).

	{
	if (!strEquals(dAuthDesc.GetElement(FIELD_TYPE).AsStringView(), AUTH_TYPE_SHA1))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_AUTHDESC_TYPE, dAuthDesc.GetElement(FIELD_TYPE).AsString()), Msg);
		return false;
		}

	if (dAuthDesc.GetElement(FIELD_CREDENTIALS).IsNil())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_AUTHDESC_CREDENTIALS_REQUIRED, Msg);
		return false;
		}

	if (dAuthDesc.GetElement(FIELD_ACTUAL).IsNil())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_AUTHDESC_ACTUAL_REQUIRED, Msg);
		return false;
		}

	return true;
	}

bool CCryptosaurEngine::ValidateMessage (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, bool bRequireAdmin)

//	ValidateMessage
//
//	Make sure that we can handle the message given the current security
//	requirements and the state of the engine.
//
//	Returns TRUE if we can handle the message. Otherwise, we reply with the
//	appropriate message and return FALSE.

	{
	CString sError;

	//	Make sure we're running properly

	if (!IsRunning(&sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return false;
		}

	//	Check sandbox

	if (bRequireAdmin && !ValidateSandboxAdmin(Msg, pSecurityCtx))
		return false;

	//	OK

	return true;
	}

bool CCryptosaurEngine::ValidateRightsGrant (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, const CAttributeList &Rights)

//	ValidRightsGrant
//
//	Validates that the service has the permission to grant the given rights to
//	a user.

	{
	int i;

	//	Check sandbox

	if (pSecurityCtx && !pSecurityCtx->HasServiceRightArcAdmin())
		{
		//	Get the sandbox prefix (rights are case-insensitive)

		CString sSandboxPrefix = strToLower(pSecurityCtx->GetSandbox());

		//	Get the list of rights

		TArray<CString> All;
		Rights.GetAll(&All);

		//	Make sure each right is sandboxed

		for (i = 0; i < All.GetCount(); i++)
			{
			if (!strStartsWith(All[i], sSandboxPrefix))
				{
				SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_CANNOT_GRANT_RIGHT, All[i]), Msg);
				return false;
				}
			}
		}

	//	OK

	return true;
	}

CString CCryptosaurEngine::ValidateSandbox (const CString &sSandbox)

//	ValidateSandbox
//
//	Makes sure the sandbox name is in the proper format.

	{
	//	Handle the degenerate case. If callers need to worry about null 
	//	sandbox IDs, then they should check for null when this returns.

	if (sSandbox.IsEmpty())
		return NULL_STR;

	//	See if we already have a dot

	bool bFoundDot = false;
	const char *pPos = sSandbox.GetParsePointer();
	const char *pPosEnd = pPos + sSandbox.GetLength();

	const char *pStart = pPos;
	while (pPos < pPosEnd)
		if (*pPos == '.')
			{
			pPos++;
			bFoundDot = true;
			break;
			}
		else
			pPos++;

	if (bFoundDot)
		return CString(sSandbox.GetParsePointer(), pPos - pStart);
	else
		return strPattern("%s.", sSandbox);
	}

bool CCryptosaurEngine::ValidateUsername (const CString &sUsername, CString *retsError)

//	ValidateUsername
//
//	Make sure that the username is valid. Usernames can have any character
//	except ASCII control codes and must have at least one printable character.
//
//	Also, usernames may not start with '#', '&', '@', '$', '%', or '['.
//
//	If valid we return TRUE.

	{
	bool bLeadingWhitespace = false;
	bool bTrailingWhitespace = false;
	bool bDoubleWhitespace = false;
	bool bHasPrintableChars = false;
	const char *pPos = sUsername.GetParsePointer();
	const char *pPosEnd = pPos + sUsername.GetLength();

	//	Can't lead with certain symbols

	if (*pPos == '[' || *pPos == '#' || *pPos == '&' || *pPos == '$' || *pPos == '@' || *pPos == '%')
		{
		*retsError = ERR_INVALID_LEADING_CHAR;
		return false;
		}

	//	Check the rest of the name

	int iCount = 0;
	while (pPos < pPosEnd)
		{
		if (strIsASCIIControl(pPos))
			{
			*retsError = ERR_CANT_HAVE_CONTROL_CHARS;
			return false;
			}

		//	Do not allow braces or backslash

		if (*pPos == '\\' || *pPos == '{' || *pPos == '}')
			{
			*retsError = ERR_INVALID_CHAR;
			return false;
			}

		//	Is this a printable char?

		UTF32 dwCodePoint = strParseUTF8Char(&pPos, pPosEnd);
		bool bIsPrintableChar = strIsPrintableChar(dwCodePoint);

		//	If this is not printable and we haven't yet seen a printable char
		//	then we have a leading whitespace

		if (!bIsPrintableChar && !bHasPrintableChars)
			bLeadingWhitespace = true;

		//	We have at least one printable char

		if (bIsPrintableChar)
			bHasPrintableChars = true;

		//	If the previous character was also whitespace, then that's an error

		else if (bTrailingWhitespace)
			bDoubleWhitespace = true;

		bTrailingWhitespace = !bIsPrintableChar;

		if (bIsPrintableChar)
			iCount++;
		}

	//	Printable characters

	if (iCount < 3)
		{
		*retsError = ERR_MUST_HAVE_PRINTABLE_CHARS;
		return false;
		}

	//	Leading or trailing whitespace

	if (bLeadingWhitespace || bTrailingWhitespace)
		{
		*retsError = ERR_INVALID_WHITESPACE;
		return false;
		}

	//	Double whitespace

	if (bDoubleWhitespace)
		{
		*retsError = ERR_TOO_MUCH_WHITESPACE;
		return false;
		}

	//	Done

	return true;
	}
