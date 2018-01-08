//	Listeners.cpp
//
//	Handles listeners
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command")

DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command")

DECLARE_CONST_STRING(MSG_ESPER_DISCONNECT,				"Esper.disconnect")
DECLARE_CONST_STRING(MSG_ESPER_START_LISTENER,			"Esper.startListener")
DECLARE_CONST_STRING(MSG_ESPER_STOP_LISTENER,			"Esper.stopListener")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

DECLARE_CONST_STRING(ERR_INVALID_LISTENER,				"Invalid listener: %s.")
DECLARE_CONST_STRING(ERR_NO_SERVICES,					"No services to handle connection.")

void CHyperionEngine::ActivateListeners (void)

//	ActivateListeners
//
//	Attempts to bring each listener closer to its desired state

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Listeners.GetCount(); i++)
		{
		SListener *pListener = &m_Listeners[i];
		if (pListener->iStatus != pListener->iDesiredStatus)
			{
			switch (pListener->iDesiredStatus)
				{
				case statusListening:
					//	If we're stopped, then send start up the listener

					if (pListener->iStatus == statusStopped
							|| pListener->iStatus == statusStopRequested)
						{
						pListener->dwLastTicket = MakeCustomTicket();

						CComplexArray *pPayload = new CComplexArray;
						pPayload->Insert(pListener->sName);
						pPayload->Insert(pListener->sPort);
						pPayload->Insert(pListener->sProtocol);

						SendMessageCommand(ADDRESS_ESPER_COMMAND,
								MSG_ESPER_START_LISTENER,
								GenerateAddress(PORT_HYPERION_COMMAND),
								pListener->dwLastTicket,
								CDatum(pPayload));

						//	Remember that we requested this

						pListener->iStatus = statusListenRequested;
						}

					break;

				case statusStopped:
					//	If we're listening, send a stop request to Esper

					if (pListener->iStatus == statusListening
							|| pListener->iStatus == statusListenRequested)
						{
						pListener->dwLastTicket = MakeCustomTicket();

						CComplexArray *pPayload = new CComplexArray;
						pPayload->Insert(pListener->sName);

						SendMessageCommand(ADDRESS_ESPER_COMMAND,
								MSG_ESPER_STOP_LISTENER,
								GenerateAddress(PORT_HYPERION_COMMAND),
								pListener->dwLastTicket,
								CDatum(pPayload));

						//	Remember that we requested this

						pListener->iStatus = statusStopRequested;
						}

					break;
				}
			}
		}
	}

void CHyperionEngine::Disconnect (const CString &sSocket)

//	Disconnect
//
//	Disconnect the socket (this should only be called before we have a session)

	{
	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(sSocket);

	SendMessageCommand(ADDRESS_ESPER_COMMAND,
			MSG_ESPER_DISCONNECT,
			GenerateAddress(PORT_HYPERION_COMMAND),
			0,
			CDatum(pPayload));
	}

bool CHyperionEngine::FindListener (const CString &sName, int *retiIndex)

//	FindListener
//
//	Finds the listener by name.

	{
	return m_Listeners.FindPos(sName, retiIndex);
	}

void CHyperionEngine::MsgEsperOnConnect (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgEsperOnConnect
//
//	Esper.onConnect {socket} {name} {address}

	{
	CSmartLock Lock(m_cs);

	//	Get some variables

	CString sSocket = Msg.dPayload.GetElement(0).AsString();
	CString sListener = Msg.dPayload.GetElement(1);
	CString sNetAddress = Msg.dPayload.GetElement(2);

	//	We look up the name in the list of listeners. If no listener
	//	could be found, then we drop the session.

	int iIndex;
	if (!FindListener(sListener, &iIndex))
		{
		Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_LISTENER, sListener));
		Disconnect(sSocket);
		return;
		}

	SListener *pListener = &m_Listeners[iIndex];

	//	If the listener has no services, then we drop the session

	if (pListener->Services.GetCount() == 0)
		{
		Log(MSG_LOG_ERROR, ERR_NO_SERVICES);
		Disconnect(sSocket);
		return;
		}

	//	Ask one of the services (it doesn't matter which one)
	//	to create a session object for us.

	ISessionHandler *pSession = pListener->Services[0]->CreateSessionObject(this, sListener, sSocket, sNetAddress);

	//	We can unlock at this point

	Lock.Unlock();

	//	Let the session handle the conversation

	StartSession(Msg, pSession);
	}

void CHyperionEngine::MsgEsperOnListenerStarted (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgEsperOnListenerStarted
//
//	Esper.onListenerStarted {name}

	{
	CSmartLock Lock(m_cs);

	int iIndex;
	if (FindListener(Msg.dPayload.GetElement(0), &iIndex))
		{
		//	If this is not the latest ticket, then we ignore it

		if (Msg.dwTicket != m_Listeners[iIndex].dwLastTicket)
			return;

		//	Set the status

		m_Listeners[iIndex].iStatus = statusListening;
		m_Listeners[iIndex].dwLastTicket = 0;
		}
	}

void CHyperionEngine::MsgEsperOnListenerStopped (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgEsperOnListenerStopped
//
//	Esper.onListenerStopped {name}

	{
	CSmartLock Lock(m_cs);

	int iIndex;
	if (FindListener(Msg.dPayload.GetElement(0), &iIndex))
		{
		//	If this is not the latest ticket, then we ignore it

		if (Msg.dwTicket != m_Listeners[iIndex].dwLastTicket)
			return;

		//	Set the status

		m_Listeners[iIndex].iStatus = statusStopped;
		m_Listeners[iIndex].dwLastTicket = 0;
		}
	}
