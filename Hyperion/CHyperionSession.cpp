//	CHyperionSession.cpp
//
//	CHyperionSession class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CONNECT_ADDR,				"connectAddr")
DECLARE_CONST_STRING(FIELD_SOCKET,						"socket")

DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")

CHyperionSession::CHyperionSession (CHyperionEngine *pEngine, const CString &sListener, const CString &sProtocol, CDatum dSocket, const CString &sNetAddress) :
		m_pEngine(pEngine),
		m_sListener(sListener),
		m_sProtocol(sProtocol),
		m_dSocket(dSocket),
		m_sNetAddress(sNetAddress)

//	CHyperionSession constructor

	{
	}

void CHyperionSession::DebugLog (const CString &sLog)

//	DebugLog
//
//	Logs a line from a service

	{
	GetProcessCtx()->Log(MSG_LOG_INFO, strPattern("[%x] %s", CEsperInterface::ConnectionToFriendlyID(m_dSocket), sLog));
	}

CString CHyperionSession::GetDebugInfo (void) const

//	GetDebugInfo
//
//	Returns info about this session.

	{
	return strPattern("[%x] Unknown session.", CEsperInterface::ConnectionToFriendlyID(m_dSocket));
	}

void CHyperionSession::OnGetStatusReport (CComplexStruct *pStatus) const

//	GetStatusReport
//
//	Returns a struct with status information about the session.

	{
	//	Add our information

	pStatus->SetElement(FIELD_SOCKET, CEsperInterface::ConnectionToFriendlyID(m_dSocket));
	pStatus->SetElement(FIELD_CONNECT_ADDR, m_sNetAddress);

	//	Let our subclasses add status

	OnGetHyperionStatusReport(pStatus);
	}
