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

enum class EPromiseResult
	{
	OK,						//	Pass reply to next promise or return to client
	Wait,					//	Wait for message reply and call at fnProcess when done.
	Error,					//	Return error to client
	};

struct SArchonPromise
	{
	std::function<EPromiseResult(ISessionHandler &Session, const SArchonMessage &Msg, SArchonMessage &retReply)> fnStart = NULL;
	std::function<EPromiseResult(ISessionHandler &Session, const SArchonMessage &Msg, SArchonMessage &retReply)> fnProcess = NULL;
	std::function<void()> fnMark = NULL;

	TUniquePtr<SArchonPromise> pThen = NULL;
	};

class CPromiseSession : public ISessionHandler
	{
	public:
		CPromiseSession () { }
		CPromiseSession (TUniquePtr<SArchonPromise> &&pCode);

		void Then (TUniquePtr<SArchonPromise> &&pCode);

	protected:

		//	ISessionHandler virtuals

		virtual void OnMark (void) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;

	private:

		bool HandleReply (EPromiseResult iResult, const SArchonMessage &Reply);

		TUniquePtr<SArchonPromise> m_pCode;
		SArchonPromise *m_pCurrent = NULL;
	};

class CPromiseImpl
	{
	public:
		static TUniquePtr<SArchonPromise> AeonCreateTable (const CString &sTableName, const CString &sTableDesc, const CString &sReplyAddr);
	};
