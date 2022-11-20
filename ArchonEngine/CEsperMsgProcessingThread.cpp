//	CEsperMsgProcessingThread.cpp
//
//	CEsperMsgProcessingThread class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CRASH,							"Crash in CEsperMsgProcessingThread.")

CEsperMsgProcessingThread::CEsperMsgProcessingThread (CEsperEngine &Engine) :
		m_Engine(Engine),
		m_iState(processingUnknown),
		m_dwLastStateChange(0)

//	CEsperMsgProcessingThread constructor

	{
	m_PausedEvent.Create();
	}

EProcessingState CEsperMsgProcessingThread::GetState (DWORD *retdwDuration)

//	GetState
//
//	Returns the current state of the engine and how long it has been in that
//	state.
//
//	NOTE: For efficiency we don't lock anything when returning this, so there
//	is a small chance that the state will not match the duration.

	{
	if (retdwDuration)
		*retdwDuration = ::GetTickCount() - m_dwLastStateChange;

	return m_iState;
	}

void CEsperMsgProcessingThread::Run (void)

//	Run
//
//	Processing thread

	{
	try
		{
		//	Keep looping until we're asked to quit

		while (true)
			{
			SetState(processingWaiting);

			//	Wait for the quit event and the work event

			CWaitArray Wait;
			int STOP_EVENT = Wait.Insert(m_Engine.GetPauseEvent());
			int QUIT_EVENT = Wait.Insert(m_Engine.GetQuitEvent());
			int WORK_EVENT = Wait.Insert(m_Engine.GetQueueEvent());

			int iEvent = Wait.WaitForAny();
			if (iEvent == QUIT_EVENT)
				{
				SetState(processingExit);
				return;
				}
			else if (iEvent == STOP_EVENT)
				{
				SetState(processingStopped);
				m_PausedEvent.Set();

				CWaitArray Wait;
				int QUIT_EVENT = Wait.Insert(m_Engine.GetQuitEvent());
				int RUN_EVENT = Wait.Insert(m_Engine.GetRunEvent());

				int iEvent = Wait.WaitForAny();
				if (iEvent == QUIT_EVENT)
					{
					SetState(processingExit);
					return;
					}
				else
					m_PausedEvent.Reset();
				}
			else if (iEvent == WORK_EVENT)
				{
				SetState(processingMessages);

				CArchonMessageList MsgList;
				if (m_Engine.Dequeue(1, &MsgList))
					{
					for (int i = 0; i < MsgList.GetCount(); i++)
						m_Engine.ProcessMessage(MsgList[i]);
					}
				}
			}
		}
	catch (...)
		{
		m_Engine.GetProcessCtx()->LogBlackBox(ERR_CRASH);
		throw;
		}
	}

