//	MsgSetKey.cpp
//
//	MsgSetKey
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ID_GOOGLE_APP_KEY,						"Google.appKey")
DECLARE_CONST_STRING(ID_GOOGLE_CLIENT_ID,					"Google.clientID")
DECLARE_CONST_STRING(ID_GOOGLE_CLIENT_SECRET,				"Google.clientSecret")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,			"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,				"Cryptosaur.command")

DECLARE_CONST_STRING(TABLE_ARC_KEYS,						"Arc.keys")

DECLARE_CONST_STRING(ERR_UNKNOWN_KEY_ID,					"Unknown key ID: %s.")

class CSetKeySession : public CAeonInsertSession
	{
	public:
		CSetKeySession (CCryptosaurEngine *pEngine, const CString &sReplyAddr, const CString &sTableName, CDatum dKeyPath, CDatum dData) :
			CAeonInsertSession(sReplyAddr, sTableName, dKeyPath, dData),
			m_pEngine(pEngine)
			{ }

	protected:
		virtual void OnSuccess (void);

	private:
		CCryptosaurEngine *m_pEngine;
	};

bool CCryptosaurEngine::IsKnownExternalKey (const CString &sID)

//	IsKnownExternalKey
//
//	Returns TRUE if this is one of our known external keys.

	{
	return (strEquals(sID, ID_GOOGLE_APP_KEY)
		|| strEquals(sID, ID_GOOGLE_CLIENT_ID)
		|| strEquals(sID, ID_GOOGLE_CLIENT_SECRET));
	}

void CCryptosaurEngine::MsgSetKey (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSetKey
//
//	Cryptosaur.setKey {id} {data}

	{
	CString sID = Msg.dPayload.GetElement(0);
	CDatum dData = Msg.dPayload.GetElement(1);

	//	We support an explicit list of keys

	if (IsKnownExternalKey(sID))
		{
		StartSession(Msg, new CSetKeySession(
			this, 
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			TABLE_ARC_KEYS,
			sID,
			dData));
		}

	//	Otherwise, unsupported

	else
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNKNOWN_KEY_ID, sID), Msg);
		}
	}

void CSetKeySession::OnSuccess (void)

//	OnSuccess
//
//	Successfully wrote the key to Arc.keys.

	{
	//	Since we were successful, store the key in our store

	m_pEngine->InsertKey((const CString &)GetKeyPath(), GetData());

	//	Return the key that we just stored to the user

	SendMessageReply(MSG_REPLY_DATA, GetData());
	}
