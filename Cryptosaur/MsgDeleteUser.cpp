//	DeleteUser.cpp
//
//	User creation routines
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command")
DECLARE_CONST_STRING(ADDR_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_OK,							"OK");

DECLARE_CONST_STRING(USERS_TABLE_NAME,					"Arc.users")

const DWORD MESSAGE_TIMEOUT =							30 * 1000;

void CCryptosaurEngine::MsgDeleteUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgDeleteUser
//
//	Cryptosaur.deleteUser {username}

	{
	struct SMsgDeleteUser
		{
		void Mark () { }

		CString sUserID;
		};

	//	Must be admin service

	if (!ValidateMessage(Msg, pSecurityCtx, true))
		return;

	//	Get some parameters

	SMsgDeleteUser Ctx;
	Ctx.sUserID = Msg.dPayload.GetElement(0).AsString();

	//	Now start our run

	auto pSession = TPromiseSession<SMsgDeleteUser>::Make(std::move(Ctx));

	pSession->Then(
		[this](ISessionHandler &Session, auto &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)
			{
			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(USERS_TABLE_NAME);
			dPayload.Append(strToLower(Ctx.sUserID));
			dPayload.Append(CDatum());

			Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_INSERT, Session.GenerateAddress(ADDR_CRYPTOSAUR_COMMAND), dPayload, MESSAGE_TIMEOUT);
			return EPromiseResult::WaitForResponse;
			},
		
		[this](auto& Session, auto& Ctx, const SArchonMessage& Msg, SArchonMessage& retReply)
			{
			if (IsError(Msg))
				return ReturnError(Msg.sMsg, Msg.dPayload, retReply);

			return ReturnEndSession(MSG_OK, CDatum(), retReply);
			});

	StartSession(Msg, std::move(pSession));
	}

