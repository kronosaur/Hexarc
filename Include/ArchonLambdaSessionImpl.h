//	ArchonLambdaSessionImpl.h
//
//	Session Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by ArchonEngine.h

#pragma once

#include <functional>

template<typename DATACTX> struct SArchonSessionHandler
	{
	std::function<bool(ISessionHandler &Session, DATACTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)> fnOnStart = NULL;
	std::function<bool(ISessionHandler &Session, DATACTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)> fnOnSuccess = NULL;
	std::function<bool(ISessionHandler &Session, DATACTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)> fnOnError = NULL;
	};

template<typename DATACTX> class CLambdaSession : public ISessionHandler
	{
	public:
		CLambdaSession (const DATACTX &Ctx = DATACTX()) : m_Ctx(Ctx)
			{ }

		void InsertHandler (const SArchonSessionHandler<DATACTX> &Handler)
			{
			auto *pEntry = m_Handlers.Insert();
			*pEntry = Handler;
			}

		void CmdCreateTable (const CString &sTableName, const CString &sTableDesc, const CString &sReplyAddr)
			{
			static const CString ADDR_AEON_COMMAND					("Aeon.command");
			static const CString MSG_AEON_CREATE_TABLE				("Aeon.createTable");
			static const CString MSG_ERROR_ALREADY_EXISTS			("Error.alreadyExists");
			static const CString MSG_LOG_ERROR						("Log.error");

			static const CString ERR_UNABLE_TO_CREATE_TABLE			("Unable to create %s table: %s");

			InsertHandler({

				[sTableDesc, sReplyAddr](ISessionHandler &Session, DATACTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)
					{
					CDatum dTableDesc;
					CStringBuffer Stream(sTableDesc);
					CDatum::Deserialize(CDatum::formatAEONScript, Stream, &dTableDesc);

					CDatum dPayload(CDatum::typeArray);
					dPayload.Append(dTableDesc);

					Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, Session.GenerateAddress(sReplyAddr), dPayload, MESSAGE_TIMEOUT);

					return true;
					},

					//	OnSuccess

					NULL,

					//	OnError

				[sTableName](ISessionHandler &Session, DATACTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)
					{
					//	If the table already exists, then just continue.

					if (strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
						return false;

					//	Otherwise, log the error.

					Session.GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_TABLE, sTableName, Msg.dPayload.AsString()));
					return false;
					},
				});
			}
		
	protected:

		//	ISessionHandler virtuals

		virtual void OnMark (void) override
			{
			m_Ctx.Mark();
			}

		virtual bool OnProcessMessage (const SArchonMessage &Msg) override
			{
			if (IsError(Msg))
				{
				if (m_Handlers[m_iPos].fnOnError)
					{
					SArchonMessage Reply;
					if (m_Handlers[m_iPos].fnOnError(*this, m_Ctx, Msg, Reply))
						return true;

					return HandleReply(Reply);
					}
				else
					{
					SendMessageReplyError(Msg.sMsg, Msg.dPayload);
					return false;
					}
				}
			else
				{
				if (m_Handlers[m_iPos].fnOnSuccess)
					{
					SArchonMessage Reply;
					if (m_Handlers[m_iPos].fnOnSuccess(*this, m_Ctx, Msg, Reply))
						return true;

					return HandleReply(Reply);
					}
				else
					{
					return HandleReply(SArchonMessage());
					}
				}
			}

		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override
			{
			if (m_Handlers.GetCount() == 0)
				//	LATER: Reply with error?
				return false;

			SArchonMessage Reply;
			if (m_Handlers[m_iPos].fnOnStart(*this, m_Ctx, Msg, Reply))
				return true;

			return HandleReply(Reply);
			}

	private:
		static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;


		bool HandleReply (const SArchonMessage &Reply)
			{
			m_iPos++;
			if (m_iPos < m_Handlers.GetCount())
				{
				SArchonMessage NewReply;
				if (m_Handlers[m_iPos].fnOnStart(*this, m_Ctx, Reply, NewReply))
					return true;

				return HandleReply(NewReply);
				}
			else
				return false;
			}

		DATACTX m_Ctx;
		TArray<SArchonSessionHandler<DATACTX>> m_Handlers;
		int m_iPos = 0;
	};
