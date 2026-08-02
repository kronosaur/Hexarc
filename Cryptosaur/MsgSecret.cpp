//	MsgSecret.cpp
//
//	Cryptosaur secret storage messages.
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(PREFIX_GRID_SECRET,				"Grid.")

DECLARE_CONST_STRING(TABLE_ARC_KEYS,					"Arc.keys")

DECLARE_CONST_STRING(ERR_CANT_ACCESS_SECRET,			"Service not authorized to access secret: %s.")
DECLARE_CONST_STRING(ERR_INVALID_SECRET_ID,				"Invalid secret ID: %s.")
DECLARE_CONST_STRING(ERR_SECRET_VALUE_REQUIRED,			"Secret value required.")

class CSetSecretSession : public CAeonInsertSession
	{
	public:
		CSetSecretSession (CCryptosaurEngine *pEngine, const CString &sReplyAddr, const CString &sTableName, CDatum dKeyPath, CDatum dData) :
				CAeonInsertSession(sReplyAddr, sTableName, dKeyPath, dData),
				m_pEngine(pEngine)
			{ }

	protected:
		virtual void OnSuccess (void) override;

	private:
		CCryptosaurEngine *m_pEngine;
	};

static bool IsValidSecretID (CStringView sID)
	{
	CString sClean = strClean(sID);
	if (sClean.IsEmpty() || !strEquals(sClean, CString(sID)))
		return false;

	if (!strStartsWith(sClean, PREFIX_GRID_SECRET))
		return false;

	const char *pPos = sClean.GetParsePointer();
	const char *pEnd = pPos + sClean.GetLength();
	while (pPos < pEnd)
		{
		if ((BYTE)*pPos < 0x20 || *pPos == '{' || *pPos == '}' || *pPos == '\\')
			return false;

		pPos++;
		}

	return true;
	}

void CCryptosaurEngine::MsgGetSecret (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetSecret
//
//	Cryptosaur.getSecret {secretID}

	{
	CString sSecretID = Msg.dPayload.GetElement(0).AsString();
	if (!IsValidSecretID(sSecretID))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_SECRET_ID, sSecretID), Msg);
		return;
		}

	if (pSecurityCtx && !pSecurityCtx->HasServiceRightArcAdmin() && !pSecurityCtx->IsNamespaceAccessible(sSecretID))
		{
		SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_CANT_ACCESS_SECRET, sSecretID), Msg);
		return;
		}

	CSmartLock Lock(m_cs);
	CString *pValue = m_ExternalKeys.GetAt(sSecretID);
	SendMessageReply(MSG_REPLY_DATA, (pValue ? CDatum(*pValue) : CDatum()), Msg);
	}

void CCryptosaurEngine::MsgSetSecret (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSetSecret
//
//	Cryptosaur.setSecret {secretID} {value}

	{
	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	CString sSecretID = Msg.dPayload.GetElement(0).AsString();
	if (!IsValidSecretID(sSecretID))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_SECRET_ID, sSecretID), Msg);
		return;
		}

	CDatum dValue = Msg.dPayload.GetElement(1);
	if (dValue.IsNil())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_SECRET_VALUE_REQUIRED, Msg);
		return;
		}

	StartSession(Msg, new CSetSecretSession(
			this,
			GenerateAddress(PORT_CRYPTOSAUR_COMMAND),
			TABLE_ARC_KEYS,
			sSecretID,
			CDatum(dValue.AsString())));
	}

void CSetSecretSession::OnSuccess (void)

//	OnSuccess
//
//	Successfully wrote the secret to Arc.keys.

	{
	m_pEngine->InsertKey(GetKeyPath().AsStringView(), GetData());
	SendMessageReply(MSG_REPLY_DATA, CDatum(true));
	}
