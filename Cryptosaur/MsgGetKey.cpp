//	MsgGetKey.cpp
//
//	MsgGetKey
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(KEYS_TABLE_NAME,					"Arc.keys")

DECLARE_CONST_STRING(ERR_CANT_ACCESS_KEY,				"Service not authorized to access key: %s.")

const int DEFAULT_KEY_SIZE_BYTES =						64;

class CCreateKeySession : public CAeonInsertSession
	{
	public:
		CCreateKeySession (CCryptosaurEngine *pEngine, const CString &sReplyAddr, const CString &sTableName, CDatum dKeyPath, CDatum dData) :
				CAeonInsertSession(sReplyAddr, sTableName, dKeyPath, dData),
				m_pEngine(pEngine)
			{ }

	protected:
		virtual void OnSuccess (void);

	private:
		CCryptosaurEngine *m_pEngine;
	};

void CCryptosaurEngine::MsgGetKey (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetKey
//
//	Cryptosaur.getKey {keyName}

	{
	CStringView sKeyName = Msg.dPayload.GetElement(0);

	//	Make sure that the keyname is appropriate to the security context

	if (sKeyName.IsEmpty() || (pSecurityCtx && !pSecurityCtx->IsNamespaceAccessible(sKeyName)))
		{
		SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_CANT_ACCESS_KEY, sKeyName), Msg);
		return;
		}
	
	//	Look for the key name in our cache. If we find it, return it.

	CSmartLock Lock(m_cs);

	//	For known external keys, we look up in the cache, but don't create a key
	//	if not found.

	if (IsKnownExternalKey(sKeyName))
		{
		CString *pValue = m_ExternalKeys.GetAt(sKeyName);
		if (!pValue)
			SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);
		else
			SendMessageReply(MSG_REPLY_DATA, CDatum(*pValue), Msg);

		return;
		}

	//	Otherwise, we look for the key in our cache

	CIPInteger Key;
	if (m_Keys.Find(sKeyName, &Key))
		{
		CDatum dKey;
		CDatum::CreateIPIntegerFromHandoff(Key, &dKey);

		SendMessageReply(MSG_REPLY_DATA, dKey, Msg);
		return;
		}

	Lock.Unlock();

	//	Otherwise, we need to create a new key, store it in Arc.keys and then
	//	return it to the user.

	cryptoRandom(DEFAULT_KEY_SIZE_BYTES, &Key);
	CDatum dKey;
	CDatum::CreateIPIntegerFromHandoff(Key, &dKey);

	StartSession(Msg, new CCreateKeySession(
			this, 
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			KEYS_TABLE_NAME,
			sKeyName,
			dKey));
	}

void CCreateKeySession::OnSuccess (void)

//	OnSuccess
//
//	Successfully wrote the key to Arc.keys.

	{
	//	Since we were successful, store the key in our store

	m_pEngine->InsertKey(GetKeyPath().AsStringView(), (const CIPInteger &)GetData());

	//	Return the key that we just stored to the user

	SendMessageReply(MSG_REPLY_DATA, GetData());
	}
