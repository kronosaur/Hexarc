//	AeonFS.h
//
//	AeonFS Archon Implementation
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CAeonFSEngine : public TSimpleEngine<CAeonFSEngine>
	{
	public:
		CAeonFSEngine (void);
		virtual ~CAeonFSEngine (void) { }

		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:
		//	TSimpleEngine override
		virtual void OnBoot (void) override;
		virtual void OnMarkEx (void) override;
		virtual void OnStartRunning (void) override;
		virtual void OnStopRunning (void) override;

	private:
		void MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CCriticalSection m_cs;
	};
