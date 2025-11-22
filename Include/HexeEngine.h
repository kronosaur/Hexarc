//	HexeEngine.h
//
//	HexeEngine Implementation
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Hexe.h"

//	CHexeEngine ------------------------------------------------------------

class CHexeEngine : public TSimpleEngine<CHexeEngine>
	{
	public:
		CHexeEngine (void);
		virtual ~CHexeEngine (void);

		//	TSimpleEngine
		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:
		virtual void OnBoot (void);
		virtual void OnMarkEx (void);
		virtual void OnStartRunning (void);
		virtual void OnStopRunning (void);

	private:
		//	Messages
		void MsgHTTP (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRun (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CCriticalSection m_cs;
	};
