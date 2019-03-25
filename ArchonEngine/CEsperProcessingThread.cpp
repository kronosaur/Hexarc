//	CEsperProcessingThread.cpp
//
//	CEsperProcessingThread class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(IOCP_SOCKET_OP,					"IOCP.socketOp")

DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

DECLARE_CONST_STRING(WORKER_SIGNAL_SHUTDOWN,			"Worker.shutdown")
DECLARE_CONST_STRING(WORKER_SIGNAL_PAUSE,				"Worker.pause")

class CPauseThreadEvent : public IIOCPEntry
	{
	public:
		CPauseThreadEvent (CEsperProcessingThread *pThread) : IIOCPEntry(WORKER_SIGNAL_PAUSE),
				m_pThread(pThread)
			{ }

	protected:

		//	IIOCPEntry overrides

		virtual void OnProcess (void) {	m_pThread->Stop(); }

	private:
		CEsperProcessingThread *m_pThread;
	};

void CEsperProcessingThread::Mark (void)

//	Mark
//
//	Mark all AEON data in use

	{
	}

void CEsperProcessingThread::Run (void)

//	Run
//
//	Runs

	{
	try
		{
		//	Create a couple of special entries for stopping the thread
		//	and quitting the app.

		m_pPauseSignal = new CPauseThreadEvent(this);

		//	Process Events

		while (true)
			{
			try
				{
				//	If ProcessConnections returns FALSE, then it means we want
				//	to quit processing.

				if (!m_pEngine->ProcessConnections())
					break;
				}
			catch (...)
				{
				m_pEngine->Log(MSG_LOG_ERROR, CString("CRASH: Processing IO completion port."));
				}
			}

		//	Done

		delete m_pPauseSignal;
		m_pPauseSignal = NULL;
		}
	catch (...)
		{
		m_pEngine->Log(MSG_LOG_ERROR, CString("CRASH: Esper processing thread."));
		}
	}

void CEsperProcessingThread::Stop (void)

//	Stop
//
//	Stop the thread while we garbage collect

	{
	//	LATER: We need a new way to do this. We should do this like stopping the
	//	thread, because currently we can't guarantee that the thread running 
	//	this code will be the one that owns the m_PausedEvent.

	m_PausedEvent.Set();

	CWaitArray Events;
	int iQUIT_EVENT = Events.Insert(m_pEngine->GetQuitEvent());
	int iRUN_EVENT = Events.Insert(m_pEngine->GetRunEvent());

	int iEvent = Events.WaitForAny();
	if (iEvent == iQUIT_EVENT)
		{
		m_bQuit = true;
		return;
		}
	else
		m_PausedEvent.Reset();
	}
