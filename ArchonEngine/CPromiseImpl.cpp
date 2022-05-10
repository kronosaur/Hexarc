//	CPromiseImpl.cpp
//
//	CPromiseImpl class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command");

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable");
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_TABLE,		"Unable to create %s table: %s");

static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;

TUniquePtr<SArchonPromise> CPromiseImpl::AeonCreateTable (const CString &sTableName, const CString &sTableDesc, const CString &sReplyAddr)

//	AeonCreateTable
//
//	Makes sure the given table exists; creates it if not.

	{
	return TUniquePtr<SArchonPromise>(
		new SArchonPromise({

			//	Create table

			[sTableName, sTableDesc, sReplyAddr](ISessionHandler &Session, const SArchonMessage &Msg, SArchonMessage &retReply) 
				{
				CDatum dTableDesc;
				CStringBuffer Stream(sTableDesc);
				CDatum::Deserialize(CDatum::EFormat::AEONScript, Stream, &dTableDesc);

				CDatum dPayload(CDatum::typeArray);
				dPayload.Append(dTableDesc);

				Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, Session.GenerateAddress(sReplyAddr), dPayload, MESSAGE_TIMEOUT);

				return EPromiseResult::Wait;
				},

			//	Handle reply from Aeon

			[sTableName, sTableDesc, sReplyAddr](ISessionHandler &Session, const SArchonMessage &Msg, SArchonMessage &retReply) 
				{
				//	If the table already exists, then just continue.

				if (strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
					return EPromiseResult::OK;

				//	Otherwise, log the error, but continue

				else if (Session.IsError(Msg))
					{
					Session.GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_TABLE, sTableName, Msg.dPayload.AsString()));
					return EPromiseResult::OK;
					}

				//	No error

				else
					return EPromiseResult::OK;
				},

			//	Nothing to mark

			NULL,

			//	No Then

			NULL
		}));
	}

