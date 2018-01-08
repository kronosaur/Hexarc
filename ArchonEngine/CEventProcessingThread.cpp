//	CEventProcessingThread.cpp
//
//	CEventProcessingThread class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null")

CEventProcessingThread::CEventProcessingThread (CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent) :
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
	try
		{
		int i;

		m_PausedEvent.Reset();

		//	Keep looping until we're asked to quit

		while (true)
			{
			int iMaxEvents = Min(m_Events.GetCount(), MAXIMUM_WAIT_OBJECTS - 3);

			//	Wait for the events

			CWaitArray Wait;
			for (i = 0; i < iMaxEvents; i++)
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
				if (iEvent == STOP_EVENT)
					return;
				else
					m_PausedEvent.Reset();
				}
			else if (iEvent == REFRESH_EVENT)
				{
				CSmartLock Lock(m_cs);

				//	Move all the events from the new list to the working list

				for (i = 0; i < m_NewEvents.GetCount(); i++)
					m_Events.Insert(m_NewEvents[i]);

				m_NewEvents.DeleteAll();
				m_RefreshEvent.Reset();
				}

			//	One of our events has fired, so we need to send a message

			else
				{
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
		}
	catch (...)
		{

		}
	}
