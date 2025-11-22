//	MsgSignData.cpp
//
//	MsgSignData
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(TABLE_ARC_KEYS,					"Arc.keys")

DECLARE_CONST_STRING(ERR_CANT_ACCESS_KEY,				"Service not authorized to access key: %s.")

const int DEFAULT_KEY_SIZE_BYTES =						64;

class CSignDataSession : public CAeonInsertSession
	{
	public:
		CSignDataSession (CCryptosaurEngine *pEngine, const CString &sReplyAddr, const CString &sKeyName, CDatum dKey, CDatum dData) :
				CAeonInsertSession(sReplyAddr, TABLE_ARC_KEYS, sKeyName, dKey),
				m_pEngine(pEngine),
				m_dDataToSign(dData)
			{ }

	protected:
		virtual void OnSuccess (void);

	private:
		CCryptosaurEngine *m_pEngine;
		CDatum m_dDataToSign;
	};

void CCryptosaurEngine::MsgSignData (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSignData
//
//	Cryptosaur.signData {keyName} {data}

	{
	CStringView sKeyName = Msg.dPayload.GetElement(0);
	CDatum dData = Msg.dPayload.GetElement(1);

	//	Make sure that the keyname is appropriate to the security context

	if (sKeyName.IsEmpty() || (pSecurityCtx && !pSecurityCtx->IsNamespaceAccessible(sKeyName)))
		{
		SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_CANT_ACCESS_KEY, sKeyName), Msg);
		return;
		}
	
	//	Look for the key name in our cache. If found, then we sign the data and
	//	return the signature.

	CIPInteger Key;
	if (m_Keys.Find(sKeyName, &Key))
		{
		CDatum dSignature = CCryptosaurInterface::SignData(dData, Key);
		SendMessageReply(MSG_REPLY_DATA, dSignature, Msg);
		}

	//	Otherwise we start a session to store a key. We sign after the key has been
	//	successfully stored.

	else
		{
		cryptoRandom(DEFAULT_KEY_SIZE_BYTES, &Key);
		CDatum dKey;
		CDatum::CreateIPIntegerFromHandoff(Key, &dKey);

		StartSession(Msg, new CSignDataSession(
				this, 
				GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
				sKeyName,
				dKey,
				dData));
		}
	}

void CSignDataSession::OnSuccess (void)

//	OnSuccess
//
//	Key has been stored successfully

	{
	const CIPInteger &Key = GetData();

	//	Since we were successful, store the key in our store

	m_pEngine->InsertKey(GetKeyPath().AsStringView(), Key);

	//	Sign the data

	CDatum dSignature = CCryptosaurInterface::SignData(m_dDataToSign, Key);

	//	Return the signature

	SendMessageReply(MSG_REPLY_DATA, dSignature);
	}
