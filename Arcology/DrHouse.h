//	DrHouse.h
//
//	DrHouse Engine Implementation
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#pragma once

#include "BlackBoxReader.h"

//	CDrHouseEngine ------------------------------------------------------------

class CDrHouseEngine : public TSimpleEngine<CDrHouseEngine>
	{
	public:
		CDrHouseEngine (void);
		virtual ~CDrHouseEngine (void);

		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:
		virtual void OnBoot (void) override;
		virtual void OnMarkEx (void) override { m_BlackBoxProcessor.Mark(); }
		virtual void OnStartRunning (void) override;
		virtual void OnStopRunning (void) override;

	private:
		//	Message processing
		void MsgCreateLogSearch (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCreateTestTable (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetLogSearch (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgPortCacheDump (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgProcessLogSearch (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgUnitTest (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CBlackBoxProcessor m_BlackBoxProcessor;
	};
