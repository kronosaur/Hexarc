//	CProgramExecThread.cpp
//
//	CProgramExecThread class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

void CProgramExecThread::Run ()

//	Run
//
//	Run thread.

	{
	try
		{
		//	Process Events

		while (true)
			{
			try
				{
				//	Wait for either work to do, pause event, or shutdown event.

				CWaitArray Events;
				const int HAS_WORK = Events.Insert(m_ProgramSet.GetWorkEvent());
				const int SIGNAL_PAUSE = Events.Insert(m_SignalPause);
				const int SIGNAL_SHUTDOWN = Events.Insert(m_SignalShutdown);

				const int iEvent = Events.WaitForAny();
				if (iEvent == SIGNAL_PAUSE)
					{
					//	We're paused

					m_SignalPause.Reset();
					m_SignalResume.Reset();
					m_Paused.Set();

					//	Wait for resume signal

					CWaitArray ResumeEvents;
					const int SIGNAL_RESUME = Events.Insert(m_SignalResume);
					const int SIGNAL_SHUTDOWN = Events.Insert(m_SignalShutdown);

					int iResumeEvent = ResumeEvents.WaitForAny();
					if (iResumeEvent == SIGNAL_SHUTDOWN)
						//	Done
						return;
						
					//	Continue
					}
				else if (iEvent == SIGNAL_SHUTDOWN)
					{
					//	Done
					return;
					}
				else if (iEvent == HAS_WORK)
					{
					SRunResult Result = m_ProgramSet.Run();
					if (Result.iRunCode == CHexeProcess::runAsyncRequest)
						{
						m_Engine.MakeAsyncRequest(Result);
						}
					}

				//	This should never happen.

				else
					{
					throw CException(errFail);
					}
				}
			catch (...)
				{
				m_ArchonCtx.Log(MSG_LOG_ERROR, CString("CRASH: Processing CodeSlinger program."));
				}
			}
		}
	catch (...)
		{
		m_ArchonCtx.Log(MSG_LOG_ERROR, CString("CRASH: CodeSlinger exec thread."));
		}
	}
