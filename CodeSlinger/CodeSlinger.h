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

		bool IsVerbose (void) const { return m_bVerbose; }
		void SetAeonInitialized (bool bInitialized = true) { m_bAeonInitialized = bInitialized; }

		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:

		//	TSimpleEngine override

		virtual void OnBoot (void) override;
		virtual void OnMarkEx (void) override;
		virtual void OnStartRunning (void) override;
		virtual void OnStopRunning (void) override;

	private:
		static constexpr int DEFAULT_THREADS_PER_MACHINE = 4;
		static constexpr int INITIAL_THREADS = DEFAULT_THREADS_PER_MACHINE + 2;

		//	Message handlers
		void MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
//		void MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgMandelbrot (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgMandelbrotTask (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CCriticalSection m_cs;
		bool m_bMachineStarted = false;				//	TRUE if we've received Exarch.onMachineStart message
		bool m_bAeonInitialized = false;			//	TRUE if AeonDB has initialized
		bool m_bReady = false;						//	TRUE if we are serving requests
		bool m_bVerbose = true;						//	TRUE if we're in verbose mode (log extra information)
	};
