//	CInterprocessMessageThread.cpp
//
//	CInterprocessMessageThread class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int DEFAULT_PROCESSING_CHUNK =					10;
const int MAX_QUEUE_SIZE =								256;

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

DECLARE_CONST_STRING(ERR_CANT_DESERIALIZE,				"Unable to deserialize message: %s.")
DECLARE_CONST_STRING(ERR_CANT_BIND,						"Unable to bind to address: %s.")
DECLARE_CONST_STRING(ERR_DESERIALIZE_TIME_WARNING,		"Deserialized message.")

CInterprocessMessageThread::CInterprocessMessageThread (void) :
		m_pProcess(NULL),
		m_iProcessingChunk(DEFAULT_PROCESSING_CHUNK),
		m_pRunEvent(NULL),
		m_pPauseEvent(NULL),
		m_pQuitEvent(NULL)

//	CInterprocessMessageThread constructor

	{
	m_PausedEvent.Create();
	m_PausedEvent.Set();
	}

void CInterprocessMessageThread::Boot (IArchonProcessCtx *pProcess, const CString &sName, CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent)

//	Boot
//
//	Boot the thread

	{
	m_pProcess = pProcess;
	m_sName = sName;
	m_pRunEvent = &RunEvent;
	m_pPauseEvent = &PauseEvent;
	m_pQuitEvent = &QuitEvent;

	//	Start the queue

	if (!m_Queue.Create(pProcess, pProcess->GetMachineName(), pProcess->GetModuleName(), MAX_QUEUE_SIZE))
		{
		//	If we could not create the queue then it means that someone else 
		//	already has it open. This can happen if our process got restarted
		//	while the rest of the arcology kept a handle to the queue.

		if (!m_Queue.Open(pProcess, pProcess->GetMachineName(), pProcess->GetModuleName()))
			{
			//	LATER: Log error or handle error
			NULL;
			}
		}
	}

void CInterprocessMessageThread::Run (void)

//	Run
//
//	Run thread

	{
	static constexpr int MAX_CRASH_COUNT = 10;
	int iCrashCount = 0;

	m_PausedEvent.Reset();

	//	Keep looping until we're asked to quit

	while (true)
		{
		try
			{
			CWaitArray Wait;

			//	Wait for the quit event and the work event

			int STOP_EVENT = Wait.Insert(*m_pPauseEvent);
			int QUIT_EVENT = Wait.Insert(*m_pQuitEvent);
			int WORK_EVENT = Wait.Insert(m_Queue.GetEvent());

			int iEvent = Wait.WaitForAny();
			if (iEvent == QUIT_EVENT)
				return;
			else if (iEvent == STOP_EVENT)
				{
				m_PausedEvent.Set();

				CWaitArray Wait;
				int QUIT_EVENT = Wait.Insert(*m_pQuitEvent);
				int RUN_EVENT = Wait.Insert(*m_pRunEvent);

				int iEvent = Wait.WaitForAny();
				if (iEvent == QUIT_EVENT)
					return;
				else
					m_PausedEvent.Reset();

				iCrashCount = 0;
				}
			else if (iEvent == WORK_EVENT)
				{
				TArray<CString> List;
				if (m_Queue.Dequeue(m_iProcessingChunk, &List))
					{
					for (int i = 0; i < List.GetCount(); i++)
						{
						CArchonTimer Timer;

						SArchonEnvelope Env;
						if (!CInterprocessMessageQueue::DeserializeMessage(List[i], &Env))
							{
							m_pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_DESERIALIZE, List[i]));
							continue;
							}

						Timer.LogTime(m_pProcess, ERR_DESERIALIZE_TIME_WARNING);

						//	LATER: Check to see if this is bound for another machine
						//	(In which case, we should send to Exarch for transport)

						m_pProcess->SendMessage(Env.sAddr, Env.Msg);
						}
					}
				}
			}
		catch (...)
			{
			if (++iCrashCount > MAX_CRASH_COUNT)
				throw;

			m_pProcess->Log(MSG_LOG_ERROR, CString("CRASH: Interprocess Message Thread."));
			}
		}
	}
