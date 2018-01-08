//	Hyperion.h
//
//	Hyperion Engine Implementation
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#pragma once

//	Services

class IHyperionService
	{
	public:
		virtual ~IHyperionService (void) { }
	};

//	CHyperionEngine ------------------------------------------------------------

class CHyperionEngine : public TSimpleEngine<CHyperionEngine>
	{
	public:
		CHyperionEngine (void);
		virtual ~CHyperionEngine (void);

		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:
		virtual void OnBoot (void);
		virtual void OnMarkEx (void);
		virtual void OnStartRunning (void);
		virtual void OnStopRunning (void);

	private:
		//	Messages
		void MsgAeonOnStart (const SArchonMessage &Msg);
	};
