//	CEsperTLSConnectionIn.cpp
//
//	CEsperTLSConnectionIn class
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");

CEsperTLSConnectionIn::CEsperTLSConnectionIn (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLCtx& SSLCtx, const SArchonMessage& Msg, SOCKET hSocket) 
		: CEsperTLSConnectionImpl(Manager, sListener, sNetworkAddress, SSLCtx, hSocket),
		m_Msg(Msg)

//	CEsperTLSConnectionIn constructor

	{
	}

#ifdef DEBUG_MARK_CRASH

CEsperTLSConnectionIn::~CEsperTLSConnectionIn ()
	{
	m_Manager.Log(MSG_LOG_DEBUG, strPattern("CEsperTLSConnectionIn destructor %08x%08x", HIDWORD((DWORDLONG)this), LODWORD((DWORDLONG)this)));
	}

#endif

void CEsperTLSConnectionIn::OnTLSConnect ()
	{
	m_Manager.SendMessageReplyOnConnect(CDatum(GetID()), m_sListener, m_sNetworkAddress, m_Msg);
	}

void CEsperTLSConnectionIn::OnTLSDisconnect ()
	{
	m_Manager.SendMessageReplyDisconnect(m_Msg);
	}

void CEsperTLSConnectionIn::OnTLSRead (CString&& sData)
	{
	m_Manager.SendMessageReplyOnRead(CDatum(GetID()), sData, m_Msg);
	}

void CEsperTLSConnectionIn::OnTLSWriteComplete (DWORD dwBytesWritten)
	{
	m_Manager.SendMessageReplyOnWrite(CDatum(GetID()), dwBytesWritten, m_Msg);
	}
