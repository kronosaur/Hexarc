//	MsgSetKey.cpp
//
//	MsgSetKey
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPE_KEY_ENVELOPE,					"keyEnvelope")
DECLARE_CONST_STRING(TYPE_KEY_GOOGLE_OAUTH,					"keyEnvelope")

void CCryptosaurEngine::MsgSetKey (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSetKey
//
//	Cryptosaur.setKey {type} {data}

	{
#if 0
	CString sType = Msg.dPayload.GetElement(0);
	CDatum dData = Msg.dPayload.GetElement(1);

	//	Handle each type

	if (strEqualsNoCase(sType, TYPE_SSL_CERTIFICATE))
		{
		//	Generate a record for Arc.certificates.

		CDatum dKey;
		CDatum dRecord;
		CString sError;
		if (!LoadPEMCertAndKey(dData, &dKey, &dRecord, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			return;
			}

		//	Start a session to write the record to Arc.certificates. We will
		//	return MSG_OK on success.

		StartSession(Msg, new CSaveCertSession(*this, dKey, dRecord));
		}

	//	Otherwise, unknown type

	else
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNKNOWN_CERT_TYPE, sType), Msg);
		}
#endif
	}
