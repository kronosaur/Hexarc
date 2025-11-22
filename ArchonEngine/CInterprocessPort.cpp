//	CInterprocessPort.cpp
//
//	CInterprocessPort class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CLASS,						"class");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");

DECLARE_CONST_STRING(STR_INTERPROCESS_PORT,				"CInterprocessPort");

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

CDatum CInterprocessPort::GetPortStatus (void) const

//	GetPortStatus
//
//	Returns a struct with well-known fields for status.

	{
	CDatum dStatus = CDatum(CDatum::typeStruct);
	dStatus.SetElement(FIELD_CLASS, STR_INTERPROCESS_PORT);
	dStatus.SetElement(FIELD_STATUS, NULL_STR);
	return dStatus;
	}

bool CInterprocessPort::SendMessage (const SArchonMessage &Msg)

//	SendMessage
//
//	Sends a message to this foreign port

	{
	ASSERT(m_pQueue);

	SArchonEnvelope Env;
	Env.sAddr = m_sAddr;
	Env.Msg = Msg;

	CString sError;
	if (!m_pQueue->Enqueue(Env, &sError))
		{
		//	If we fail to send, log an error (we do not log errors trying to
		//	report errors because we might recurse infinitely.)

		if (!strEquals(Env.Msg.sMsg, MSG_LOG_ERROR))
			m_pProcess->Log(MSG_LOG_ERROR, sError);

		return false;
		}

	return true;
	}
