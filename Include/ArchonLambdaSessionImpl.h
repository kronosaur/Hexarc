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
