//	CEventProcessingThread.cpp
//
//	CEventProcessingThread class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null");

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(STR_THREAD_NAME,					"EventProcessing");

CEventProcessingThread::CEventProcessingThread (IArchonProcessCtx& ProcessCtx, CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent) :
		TThread(STR_THREAD_NAME),
		m_ProcessCtx(ProcessCtx),
		m_pRunEvent(&RunEvent),
		m_pPauseEvent(&PauseEvent),
		m_pQuitEvent(&QuitEvent)

//	CEventProcessingThread constructor

	{
	m_PausedEvent.Create();
	m_RefreshEvent.Create();

	m_PausedEvent.Set();
	}

void CEventProcessingThread::AddEvent (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket)

//	AddEvent
//
//	Adds an event to wait on.

	{
	CSmartLock Lock(m_cs);

	SEvent *pNewEvent = m_NewEvents.Insert();
	pNewEvent->sName = sName;
	pNewEvent->pObj = &Event;
	pNewEvent->pClient = pPort;
	pNewEvent->sMsg = sMsg;
	pNewEvent->dwTicket = dwTicket;

	//	Set the refresh event so that our thread resets

	m_RefreshEvent.Set();
	}

void CEventProcessingThread::Run (void)

//	Run
//
//	Thread runs

	{
	static constexpr int MAX_CRASH_COUNT = 10;
	int iCrashCount = 0;

	m_PausedEvent.Reset();

	//	Keep looping until we're asked to quit

	while (true)
		{
		try
			{
			int iMaxEvents = Min(m_Events.GetCount(), MAXIMUM_WAIT_OBJECTS - 3);

			//	Wait for the events

			CWaitArray Wait;
			for (int i = 0; i < iMaxEvents; i++)
				Wait.Insert(*m_Events[i].pObj);

			//	Wait for the quit event and the work event

			int STOP_EVENT = Wait.Insert(*m_pPauseEvent);
			int QUIT_EVENT = Wait.Insert(*m_pQuitEvent);
			int REFRESH_EVENT = Wait.Insert(m_RefreshEvent);

			int iEvent = Wait.WaitForAny();
			if (iEvent < 0)
				{
				::Sleep(1000);
				continue;
				}
			else if (iEvent == QUIT_EVENT)
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

				m_PausedEvent.Reset();
				iCrashCount = 0;
				}
			else if (iEvent == REFRESH_EVENT)
				{
				CSmartLock Lock(m_cs);

				//	Move all the events from the new list to the working list

				for (int i = 0; i < m_NewEvents.GetCount(); i++)
					m_Events.Insert(m_NewEvents[i]);

				m_NewEvents.DeleteAll();
				m_RefreshEvent.Reset();
				}

			//	One of our events has fired, so we need to send a message

			else
				{
				//	NOTE: We don't have to lock here because the only place that
				//	touches m_Events is this thread.

				SEvent *pEvent = &m_Events[iEvent];

				SArchonMessage Msg;
				Msg.sMsg = pEvent->sMsg;
				Msg.sReplyAddr = (pEvent->dwTicket == 0 ? ADDR_NULL : NULL_STR);
				Msg.dwTicket = pEvent->dwTicket;

				CComplexArray *pPayload = new CComplexArray;
				pPayload->Insert(CDatum(pEvent->sName));

				Msg.dPayload = CDatum(pPayload);

				pEvent->pClient->SendMessage(Msg);

				//	Done with the event

				m_Events.Delete(iEvent);
				}
			}
		catch (...)
			{
			if (++iCrashCount > MAX_CRASH_COUNT)
				throw;

			m_ProcessCtx.Log(MSG_LOG_ERROR, CString("CRASH: In CEventProcessingThread."));
			}
		}
	}
