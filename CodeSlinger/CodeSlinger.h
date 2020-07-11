//	CodeSlinger.h
//
//	CodeSlinger
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CCodeSlingerEngine : public TSimpleEngine<CCodeSlingerEngine>
	{
	public:
		CCodeSlingerEngine (void);
		CCodeSlingerEngine (const CCodeSlingerEngine &Src) = delete;
		CCodeSlingerEngine (CCodeSlingerEngine &&Src) = delete;

		virtual ~CCodeSlingerEngine (void);

		CCodeSlingerEngine &operator= (const CCodeSlingerEngine &Src) = delete;
		CCodeSlingerEngine &operator= (CCodeSlingerEngine &&Src) = delete;

		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:

		//	TSimpleEngine override

		virtual void OnBoot (void) override;
		virtual void OnMarkEx (void) override;
		virtual void OnStartRunning (void) override;
		virtual void OnStopRunning (void) override;

	private:

		//	Message handlers
//		void MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CCriticalSection m_cs;
		bool m_bMachineStarted = false;				//	TRUE if we've received Exarch.onMachineStart message
		bool m_bReady = false;						//	TRUE if we are serving requests
	};
