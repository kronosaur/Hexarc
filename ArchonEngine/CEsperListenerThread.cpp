//	CEsperListenerThread.cpp
//
//	CEsperListenerThread class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null")

DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_ESPER_CREATE_LISTENER,			"Esper.startListener")
DECLARE_CONST_STRING(MSG_ESPER_ON_CONNECT,				"Esper.onConnect")
DECLARE_CONST_STRING(MSG_ESPER_ON_LISTENER_STARTED,		"Esper.onListenerStarted")
DECLARE_CONST_STRING(MSG_ESPER_ON_LISTENER_STOPPED,		"Esper.onListenerStopped")
DECLARE_CONST_STRING(MSG_OK,							"OK")

DECLARE_CONST_STRING(ERR_UNABLE_TO_BIND,				"Esper unable to bind to client message port: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_SOCKET,		"Out of server resources.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_ACCEPT_SOCKET,		"Socket accept failed.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND_MESSAGE,		"Unable to send Esper message to engine.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_LISTEN_ON_PORT,		"Unable to listen on port %s.")

CEsperListenerThread::CEsperListenerThread (CEsperEngine *pEngine, CEsperConnection::ETypes iType, const SArchonMessage &Msg) :
		m_pEngine(pEngine),
		m_iType(iType),
		m_OriginalMsg(Msg)

//	CEsperListenerThread constructor

	{
	m_PausedEvent.Create();

	//	Parse the message

	m_sName = Msg.dPayload.GetElement(0);
	m_sService = Msg.dPayload.GetElement(1).AsString();
	m_sClientAddr = Msg.sReplyAddr;
	m_dwClientTicket = Msg.dwTicket;
	}

CEsperListenerThread::~CEsperListenerThread (void)

//	CEsperListenerThread destructor

	{
	}

void CEsperListenerThread::Run (void)

//	Run
//
//	Listener thread

	{
	try
		{
		//	Bind the client port

		m_pClient = m_pEngine->Bind(m_sClientAddr);
		if (m_pClient == NULL)
			{
			//	We can't report an error because we can't reach the client
			//	Log an error locally.
			m_pEngine->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_BIND, m_sClientAddr));
			m_pEngine->ReportShutdown(m_sName);
			return;
			}

		//	Create the socket

		if (!m_Listener.CreateListeners(strToInt(m_sService, 0, NULL)))
			{
			CString sError = strPattern(ERR_UNABLE_TO_LISTEN_ON_PORT, m_sService);
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
			m_pEngine->Log(MSG_LOG_ERROR, sError);
			m_pEngine->ReportShutdown(m_sName);
			return;
			}

		//	Report that we're listening

		SendMessageReplyText(MSG_ESPER_ON_LISTENER_STARTED, m_sName);
		m_pEngine->Log(MSG_LOG_INFO, strPattern("%s listening on port %s", m_sClientAddr, m_sService));

		//	Keep accepting clients until we're asked to quit

		while (true)
			{
			//	Since accept will block, we can say that we have stopped
			//	(But we need to make sure that we catch it when accept returns).

			m_PausedEvent.Set();

			//	Accept a new connection

			CSocket NewSocket;
			bool bSuccess = m_Listener.AcceptConnection(NewSocket);

			//	If we've been asked to stop, then stop here before we allocate any
			//	memory.
			//
			//	NOTE: This call only waits if we've received a connection while
			//	we've been asked to stop. Otherwise we never leave AcceptConnection.

			if (m_pEngine->GetPauseEvent().Wait(0))
				{
				CWaitArray Wait;
				int QUIT_EVENT = Wait.Insert(m_pEngine->GetQuitEvent());
				int RUN_EVENT = Wait.Insert(m_pEngine->GetRunEvent());

				int iEvent = Wait.WaitForAny();
				if (iEvent == QUIT_EVENT)
					break;
				}

			m_PausedEvent.Reset();

			//	If we've been asked to terminate, then exit

			if (m_bShutdown)
				break;

			//	If accept failed, then report the error and exit

			if (!bSuccess)
				{
				m_pEngine->Log(MSG_LOG_ERROR, ERR_UNABLE_TO_ACCEPT_SOCKET);
				break;
				}

			//	Let the engine deal with the connection

			m_pEngine->OnConnect(m_OriginalMsg, NewSocket, m_sName, m_iType);
			}

		//	Tell our parent that we're done running. This will free
		//	up the thread object, so make sure we don't do anything after this.

		SendMessageReplyText(MSG_ESPER_ON_LISTENER_STOPPED, m_sName);
		m_pEngine->Log(MSG_LOG_INFO, strPattern("Stopped listening on port %s", m_sService));
		m_pEngine->ReportShutdown(m_sName);
		}
	catch (...)
		{
		m_pEngine->Log(MSG_LOG_ERROR, CString("CRASH: Esper listener thread."));
		throw;
		}
	}

void CEsperListenerThread::SendMessageReplyError (const CString &sMsg, const CString &sText)

//	SendMessageReplyError
//
//	Sends an error to the client

	{
	ASSERT(m_pClient);

	SArchonMessage Msg;
	Msg.sMsg = sMsg;
	Msg.dwTicket = m_dwClientTicket;
	Msg.dPayload = CDatum(sText);

	m_pClient->SendMessage(Msg);
	}

void CEsperListenerThread::SendMessageReplyText (const CString &sMsg, const CString &sText)

//	SendMessageReplyText
//
//	Sends a reply

	{
	ASSERT(m_pClient);

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(sText);
	pPayload->Insert(MSG_ESPER_CREATE_LISTENER);

	SArchonMessage Msg;
	Msg.sMsg = sMsg;
	Msg.dwTicket = m_dwClientTicket;
	Msg.dPayload = CDatum(pPayload);

	m_pClient->SendMessage(Msg);
	}

void CEsperListenerThread::SignalShutdown (void)

//	SignalShutdown
//
//	Tells the thread to quit

	{
	m_bShutdown = true;

	//	We close the socket to force an error
	m_Listener.Close();
	}
