//	BootSession.cpp
//
//	Boot session for the engine
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command")
DECLARE_CONST_STRING(ADDR_CRYPTOSAUR_NOTIFY,			"Cryptosaur.notify")

DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")

DECLARE_CONST_STRING(KEY_CRYPTOSAUR_AUTH_TOKEN,			"Cryptosaur.authToken01")

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_AEON_GET_KEY_RANGE,			"Aeon.getKeyRange")
DECLARE_CONST_STRING(MSG_AEON_GET_ROWS,					"Aeon.getRows")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ON_ADMIN_NEEDED,	"Cryptosaur.onAdminNeeded")
DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ON_START,			"Cryptosaur.onStart")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

DECLARE_CONST_STRING(CERTIFICATES_TABLE_NAME,			"Arc.certificates")
DECLARE_CONST_STRING(CERTIFICATES_TABLE_DESC,			"{name:\"Arc.certificates\" x:{keyType:utf8} y:{keyType:utf8}}")

DECLARE_CONST_STRING(KEYS_TABLE_NAME,					"Arc.keys")
DECLARE_CONST_STRING(KEYS_TABLE_DESC,					"{name:\"Arc.keys\" x:{keyType:utf8}}")

DECLARE_CONST_STRING(TYPE_ARC_TABLE,					"Arc.table")
DECLARE_CONST_STRING(TYPE_SSL_CERTIFICATE,				"sslCertificate")

DECLARE_CONST_STRING(USERS_TABLE_NAME,					"Arc.users")
DECLARE_CONST_STRING(USERS_TABLE_DESC,					"Arc.table Arc.users "
														"{"
														"name:Arc.users "
														"x:{keyType:utf8} "

														"secondaryViews: ("
															"{ name:byEmail "
																"x: {"
																"	key: (lambda (data) (lowercase (@ (@ data 'userContact) 'email))) "
																"	keyType:utf8 "
																"	keySort:ascending "
																"	} "
																"columns: (primaryKey userContact) "
																"excludeNilKeys: true "
																"}"
															")"
														"}")

DECLARE_CONST_STRING(ERR_INVALID_KEY,					"Invalid Arc.keys entry: %s")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_CERTIFICATES_TABLE,	"Unable to create Arc.certificates table: %s")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_KEYS_TABLE,	"Unable to create Arc.keys table: %s")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_USERS_TABLE,	"Unable to create Arc.users table: %s")
DECLARE_CONST_STRING(ERR_UNABLE_TO_READ_CERTIFICATES_TABLE,	"Unable to read Arc.certificates table.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_READ_USERS_TABLE,	"Unable to read Arc.users table.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_ADD_KEY,				"Unable to add key to Arc.keys table.")
DECLARE_CONST_STRING(ERR_EXPECTED_ARC_TABLE,			"Unable to interpret table definition.")

const DWORD MESSAGE_TIMEOUT =							3000 * 1000;
const int MAX_CERTIFICATES =							1000;
const int MAX_KEYS =									100;
const int DEFAULT_KEY_SIZE_BYTES =						64;

class CBootSession : public ISessionHandler
	{
	public:
		CBootSession (CCryptosaurEngine *pEngine) : 
				m_pEngine(pEngine),
				m_iState(stateUnknown)
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum States
			{
			stateUnknown,
			stateCreatingUserTable,
			stateCreatingKeyTable,
			stateCreatingCertificatesTable,
			stateReadingKeys,
			stateCreatingAuthToken,
			stateReadingCertificates,
			stateReadingUsers,
			};

		CCryptosaurEngine *m_pEngine;
		States m_iState;
	};

void CCryptosaurEngine::MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAeonOnStart
//
//	Aeon.onStart

	{
	CSmartLock Lock(m_cs);

	//	If Aeon is already running, then skip this. This can happen if Aeon
	//	restarts.

	if (m_bAeonInitialized)
		return;

	//	Start a session to create the users table

	StartSession(Msg, new CBootSession(this));
	}

void CCryptosaurEngine::SetAdminExists (bool bExists)

//	SetAdminExists
//
//	If we transition from no admin to having an admin, then we send the
//	onStart message

	{
	CSmartLock Lock(m_cs);

	//	LATER: We don't handle reverting to having no admin.
	ASSERT(bExists);

	//	Send a notification if we have an admin now

	if (bExists && !m_bAdminExists)
		{
		m_bAdminExists = true;

		SendMessageNotify(ADDR_CRYPTOSAUR_NOTIFY, MSG_CRYPTOSAUR_ON_START, CDatum());
		}
	}

//	CBootSession ---------------------------------------------------------------

bool CBootSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a response

	{
	int i;

	switch (m_iState)
		{
		case stateCreatingUserTable:
			{
			//	If we got an error creating the table, then something is
			//	seriously wrong with the arcology. [So we log it and wait for
			//	someone to fix it.]

			if (IsError(Msg) && !strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_USERS_TABLE, Msg.dPayload.AsString()));
				return false;
				}

			//	Now create the Arc.keys table

			m_iState = stateCreatingKeyTable;

			CDatum dTableDesc;
			CDatum::Deserialize(CDatum::formatAEONScript, CStringBuffer(KEYS_TABLE_DESC), &dTableDesc);

			CComplexArray *pPayload = new CComplexArray;
			pPayload->Insert(dTableDesc);

			SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);

			return true;
			}

		case stateCreatingKeyTable:
			{
			//	If we got an error creating the table, then something is
			//	seriously wrong with the arcology. [So we log it and wait for
			//	someone to fix it.]

			if (IsError(Msg) && !strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_KEYS_TABLE, Msg.dPayload.AsString()));
				return false;
				}

			//	Now create the Arc.certificates table

			m_iState = stateCreatingCertificatesTable;

			CDatum dTableDesc;
			CDatum::Deserialize(CDatum::formatAEONScript, CStringBuffer(CERTIFICATES_TABLE_DESC), &dTableDesc);

			CComplexArray *pPayload = new CComplexArray;
			pPayload->Insert(dTableDesc);

			SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);

			return true;
			}

		case stateCreatingCertificatesTable:
			{
			//	If we got an error creating the table, then something is
			//	seriously wrong with the arcology. [So we log it and wait for
			//	someone to fix it.]

			if (IsError(Msg) && !strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_CERTIFICATES_TABLE, Msg.dPayload.AsString()));
				return false;
				}

			//	Aeon is running

			m_pEngine->SetAeonInitialized();

			//	Read the table of keys

			m_iState = stateReadingKeys;

			CComplexArray *pPayload = new CComplexArray;
			pPayload->Insert(KEYS_TABLE_NAME);
			pPayload->Insert(CDatum());
			pPayload->Insert(MAX_KEYS);

			SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);
			return true;
			}

		case stateReadingKeys:
			{
			//	If we got an error then something is seriously wrong. We log
			//	it and wait for someone to fix it.

			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_UNABLE_TO_READ_USERS_TABLE);
				return false;
				}

			//	Add all the keys to our cache

			CDatum dKeyList = Msg.dPayload;
			for (i = 0; i < dKeyList.GetCount(); i += 2)
				{
				CDatum dKeyName = dKeyList.GetElement(i);
				CDatum dKey = dKeyList.GetElement(i + 1);

				if (dKeyName.IsNil() || ((const CString &)dKeyName).IsEmpty() 
						|| dKey.IsNil() || dKey.GetBasicType() != CDatum::typeIntegerIP)
					{
					GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_KEY, dKeyName.AsString()));
					return false;
					}

				m_pEngine->InsertKey(dKeyName, dKey);
				}

			//	Do we have the key to create authTokens? If not, then we need to
			//	create one.

			if (!m_pEngine->FindKey(KEY_CRYPTOSAUR_AUTH_TOKEN))
				{
				m_iState = stateCreatingAuthToken;

				CIPInteger AuthTokenKey;
				cryptoRandom(DEFAULT_KEY_SIZE_BYTES, &AuthTokenKey);
				m_pEngine->InsertKey(KEY_CRYPTOSAUR_AUTH_TOKEN, AuthTokenKey);

				CDatum dKey;
				CDatum::CreateIPIntegerFromHandoff(AuthTokenKey, &dKey);	//	Mungs AuthTokenKey

				CComplexArray *pPayload = new CComplexArray;
				pPayload->Insert(KEYS_TABLE_NAME);
				pPayload->Insert(KEY_CRYPTOSAUR_AUTH_TOKEN);
				pPayload->Insert(dKey);

				SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_INSERT, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);
				return true;
				}

			//	Otherwise, we continue by reading the Arc.users table

			else
				{
				//	Now read the certificates table

				m_iState = stateReadingCertificates;

				CComplexArray *pPayload = new CComplexArray;
				pPayload->Insert(CERTIFICATES_TABLE_NAME);
				pPayload->Insert(CDatum());
				pPayload->Insert(MAX_CERTIFICATES);

				SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);
				return true;
				}
			}

		case stateCreatingAuthToken:
			{
			//	If we got an error then something is seriously wrong. We log
			//	it and wait for someone to fix it.

			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_UNABLE_TO_ADD_KEY);
				return false;
				}

			//	Now read the certificates table

			m_iState = stateReadingCertificates;

			CComplexArray *pPayload = new CComplexArray;
			pPayload->Insert(CERTIFICATES_TABLE_NAME);
			pPayload->Insert(CDatum());
			pPayload->Insert(MAX_CERTIFICATES);

			SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_ROWS, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);
			return true;
			}

		case stateReadingCertificates:
			{
			//	If we got an error then something is seriously wrong. We log
			//	it and wait for someone to fix it.

			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_UNABLE_TO_READ_CERTIFICATES_TABLE);
				return false;
				}

			//	Add all the certificates to our cache

			CDatum dCertList = Msg.dPayload;
			for (i = 0; i < dCertList.GetCount(); i += 2)
				{
				CDatum dCertKey = dCertList.GetElement(i);
				CDatum dCert = dCertList.GetElement(i + 1);

				//	We only handle ssl certificates

				if (!strEqualsNoCase(dCert.GetElement(FIELD_TYPE), TYPE_SSL_CERTIFICATE))
					continue;

				//	Add to our cache

				m_pEngine->InsertCertificate(dCert.GetElement(FIELD_NAME), dCert);
				}

			//	Now read the Users table to see if we have at least one username

			m_iState = stateReadingUsers;

			CComplexArray *pPayload = new CComplexArray;
			pPayload->Insert(USERS_TABLE_NAME);
			pPayload->Insert(1);

			SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_KEY_RANGE, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);
			return true;
			}

		case stateReadingUsers:
			{
			//	If we got an error then something is seriously wrong. We log
			//	it and wait for someone to fix it.

			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_UNABLE_TO_READ_USERS_TABLE);
				return false;
				}

			//	If we have no users then it means that the arcology is being installed,
			//	so we send a message indicating that we need an admin

			if (Msg.dPayload.GetCount() == 0)
				{
				SendMessageNotify(ADDR_CRYPTOSAUR_NOTIFY, MSG_CRYPTOSAUR_ON_ADMIN_NEEDED, CDatum());

				//	We're done booting for now. When someone creates an admin account we can continue.

				return false;
				}

			//	An admin must exist if we have at least one user
			//	(This will also send out the onStart message).

			m_pEngine->SetAdminExists(true);

			//	We're done initializing

			return false;
			}

		default:
			return false;
		}
	}

bool CBootSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start session

	{
	CString sError;
	m_iState = stateCreatingUserTable;

	//	Create the /Arc.users table
	//	NOTE: We always create the table and leave it to Aeon to send back an "already exists"
	//	message.

	CHexeProcess Process;
	if (!Process.LoadStandardLibraries(&sError))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, sError);
		return false;
		}

	//	Parse the table definition into a document

	CHexeDocument TableDoc;
	if (!TableDoc.InitFromStream(CStringBuffer(USERS_TABLE_DESC), Process, &sError))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, sError);
		return false;
		}

	//	Get the table description (we expect only 1)

	int iTableIndex = TableDoc.GetTypeIndex(TYPE_ARC_TABLE);
	if (TableDoc.GetTypeIndexCount(iTableIndex) != 1)
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_EXPECTED_ARC_TABLE);
		return false;
		}

	const CString &sTableName = TableDoc.GetTypeIndexName(iTableIndex, 0);
	CDatum dTableDesc = TableDoc.GetTypeIndexData(iTableIndex, 0);

	//	Create the command

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dTableDesc);

	SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, GenerateAddress(PORT_CRYPTOSAUR_COMMAND), CDatum(pPayload), MESSAGE_TIMEOUT);

	return true;
	}
