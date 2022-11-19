//	ArchonPromiseSessionImpl.h
//
//	Session Implementations
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by ArchonEngine.h

#pragma once

#include <functional>

template <class CTX> struct TArchonPromise
	{
	using ArchonPromise = TArchonPromise<CTX>;

	std::function<EPromiseResult(ISessionHandler &Session, CTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)> fnStart = NULL;
	std::function<EPromiseResult(ISessionHandler &Session, CTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)> fnProcess = NULL;
	std::function<void()> fnMark = NULL;

	TUniquePtr<ArchonPromise> pThen = NULL;
	};

struct SDefaultSessionCtx
	{
	void Mark () { }
	};

template <class CTX = SDefaultSessionCtx> class TPromiseSession : public ISessionHandler
	{
	public:

		using ArchonPromise = TArchonPromise<CTX>;

		TPromiseSession () { }

		TPromiseSession (CTX &&Ctx) : m_Ctx(std::move(Ctx)) { }

		TPromiseSession (const CTX &Ctx) : m_Ctx(Ctx) { }

		TPromiseSession (TUniquePtr<ArchonPromise> &&pCode) : m_pCode(std::move(pCode))
			{
			}

		CTX &Ctx () { return m_Ctx; }
		const CTX &Ctx () const { return m_Ctx; }
		const CTX &Ctx_Const () { return m_Ctx; }

		static TUniquePtr<TPromiseSession<CTX>> Make ()
			{
			return TUniquePtr<TPromiseSession<CTX>>(new TPromiseSession<CTX>);
			}

		static TUniquePtr<TPromiseSession<CTX>> Make (CTX &&Ctx)
			{
			return TUniquePtr<TPromiseSession<CTX>>(new TPromiseSession<CTX>(std::move(Ctx)));
			}

		void Then (TUniquePtr<ArchonPromise> &&pCode)
			{
			if (m_pCode)
				{
				ArchonPromise *pLast = m_pCode;
				while (pLast->pThen)
					pLast = pLast->pThen;

				pLast->pThen = std::move(pCode);
				}
			else
				{
				m_pCode = std::move(pCode);
				}
			}
			
		void Then (std::function<EPromiseResult(ISessionHandler &Session, CTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)> fnStart,
				std::function<EPromiseResult(ISessionHandler &Session, CTX &Ctx, const SArchonMessage &Msg, SArchonMessage &retReply)> fnProcess = NULL)
			{
			Then(TUniquePtr<ArchonPromise>(
					new ArchonPromise({
						fnStart, 
						fnProcess,
						NULL,
						NULL
					})
				));
			}

		void Then_AeonCreateTable (const CString &sTableName, const CString &sTableDesc, const CString &sReplyAddr)
			{
			Then(TUniquePtr<ArchonPromise>(
				new ArchonPromise({

					//	Create table

					[sTableName, sTableDesc, sReplyAddr](auto &Session, auto &Ctx, const auto &Msg, auto &retReply) 
						{
						static const CString ADDR_AEON_COMMAND("Aeon.command");
						static const CString MSG_AEON_CREATE_TABLE("Aeon.createTable");
						static constexpr DWORD MESSAGE_TIMEOUT = 3000 * 1000;

						CDatum dTableDesc;
						CStringBuffer Stream(sTableDesc);
						CDatum::Deserialize(CDatum::EFormat::AEONScript, Stream, &dTableDesc);

						CDatum dPayload(CDatum::typeArray);
						dPayload.Append(dTableDesc);

						Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, Session.GenerateAddress(sReplyAddr), dPayload, MESSAGE_TIMEOUT);

						return EPromiseResult::WaitForResponse;
						},

					//	Handle reply from Aeon

					[sTableName, sTableDesc, sReplyAddr](auto &Session, auto &Ctx, const auto &Msg, auto &retReply) 
						{
						static const CString MSG_ERROR_ALREADY_EXISTS("Error.alreadyExists");
						static const CString MSG_LOG_ERROR("Log.error");
						static const CString ERR_UNABLE_TO_CREATE_TABLE("Unable to create %s table: %s.");

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
				})));
			}

		void Then_AeonGetValue (const CString &sTableName, CDatum dKey, const CString &sReplyAddr)
			{
			Then(TUniquePtr<ArchonPromise>(
				new ArchonPromise({

					//	Aeon.getValue

					[sTableName, dKey, sReplyAddr](auto &Session, auto &Ctx, const auto &Msg, auto &retReply) 
						{
						static const CString ADDR_AEON_COMMAND("Aeon.command");
						static const CString MSG_AEON_GET_VALUE("Aeon.getValue");
						static constexpr DWORD MESSAGE_TIMEOUT = 3000 * 1000;

						CDatum dPayload(CDatum::typeArray);
						dPayload.Append(sTableName);
						dPayload.Append(dKey);

						Session.SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_GET_VALUE, Session.GenerateAddress(sReplyAddr), dPayload, MESSAGE_TIMEOUT);

						return EPromiseResult::WaitForResponse;
						},

					//	We pass the result of Aeon.getValue to the next promise.
					//	If Aeon.getValue returns an error, then we exit the session, but
					//	that's OK because errors only happen on catastrophes. If the 
					//	value is not found, we return nil.

					NULL,

					//	Mark data in use.
					//
					//	NOTE: We can't pass-by-reference because this function will exit
					//	before the promise resolves. We need mutable to remove the 
					//	const.

					[dKey]() mutable
						{
						dKey.Mark();
						},

					//	No Then

					NULL
				})));
			}

	protected:

		//	ISessionHandler virtuals

		virtual void OnMark (void) override
			{
			m_Ctx.Mark();

			ArchonPromise *pPromise = m_pCode;
			while (pPromise)
				{
				if (pPromise->fnMark)
					pPromise->fnMark();

				pPromise = pPromise->pThen;
				}
			}

		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override
			{
			//	Start with the first one.

			m_pCurrent = m_pCode;
			if (!m_pCurrent || !m_pCurrent->fnStart)
				{
				SendMessageReply(CString("OK"));
				return false;
				}

			//	Start

			SArchonMessage Reply;
			auto iResult = m_pCurrent->fnStart(*this, m_Ctx, Msg, Reply);

			return HandleReply(iResult, Reply);
			}

		virtual bool OnProcessMessage (const SArchonMessage &Msg) override
			{
			if (m_pCurrent->fnProcess)
				{
				SArchonMessage Reply;
				auto iResult = m_pCurrent->fnProcess(*this, m_Ctx, Msg, Reply);

				return HandleReply(iResult, Reply);
				}
			else if (IsError(Msg))
				{
				SendMessageReplyError(Msg.sMsg, Msg.dPayload);
				return false;
				}
			else
				{
				return HandleReply(EPromiseResult::OK, Msg);
				}
			}

		bool HandleReply (EPromiseResult iResult, const SArchonMessage &Reply)
			{
			switch (iResult)
				{
				//	Success!

				case EPromiseResult::EndSession:
					{
					SendMessageReply(Reply.sMsg, Reply.dPayload);
					return false;
					}

				//	Next

				case EPromiseResult::OK:
					m_pCurrent = m_pCurrent->pThen;
					if (!m_pCurrent || !m_pCurrent->fnStart)
						{
						SendMessageReply(Reply.sMsg, Reply.dPayload);
						return false;
						}
					else
						{
						SArchonMessage NextReply;
						auto iNextResult = m_pCurrent->fnStart(*this, m_Ctx, Reply, NextReply);

						return HandleReply(iNextResult, NextReply);
						}

				//	Repeat

				case EPromiseResult::Repeat:
					if (!m_pCurrent || !m_pCurrent->fnStart)
						{
						SendMessageReply(Reply.sMsg, Reply.dPayload);
						return false;
						}
					else
						{
						SArchonMessage NextReply;
						auto iNextResult = m_pCurrent->fnStart(*this, m_Ctx, Reply, NextReply);

						return HandleReply(iNextResult, NextReply);
						}

				//	Wait for reply

				case EPromiseResult::WaitForResponse:
					return true;

				//	Error

				case EPromiseResult::Error:
					SendMessageReplyError(Reply.sMsg, Reply.dPayload);
					return false;

				default:
					throw CException(errFail);
				}
			}

		TUniquePtr<ArchonPromise> m_pCode;
		ArchonPromise *m_pCurrent = NULL;
		CTX m_Ctx;
	};

