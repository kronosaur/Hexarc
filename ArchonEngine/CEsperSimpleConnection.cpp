//	CEsperSimpleConnection.cpp
//
//	CEsperSimpleConnection class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")

DECLARE_CONST_STRING(ERR_CONNECTION_LOST,				"Simple connection lost.")

const DWORD ACTIVE_TIME_THRESHOLD =						5 * 60 * 1000;

CEsperSimpleConnection::CEsperSimpleConnection (CEsperConnectionManager &Manager, SOCKET hSocket) : CEsperConnection(hSocket),
		m_Manager(Manager),
		m_iState(stateNone)

//	CEsperConnection constructor

	{
	}

void CEsperSimpleConnection::AccumulateStatus (CEsperConnection::SStatus *ioStatus)

//	AccumulateStatus
//
//	Updates status counters

	{
	ioStatus->iTotalObjects++;

	switch (m_iState)
		{
		case stateReplyOnRead:
			ioStatus->iWaitingForRead++;
			break;

		case stateReplyOnWrite:
			ioStatus->iWaitingForWrite++;
			break;

		default:
			ioStatus->iIdle++;
			break;
		}

	//	Active?

	if (sysGetTicksElapsed(GetCurrentOpStartTime()) <= ACTIVE_TIME_THRESHOLD)
		ioStatus->iActive++;
	}

bool CEsperSimpleConnection::BeginRead (const SArchonMessage &Msg, CString *retsError)

//	BeginRead
//
//	Start a read operation and reply to the message when complete.

	{
	//	Remember the message to reply to.

	m_iState = stateReplyOnRead;
	m_Msg = Msg;

	//	Let our subclass handle it.
	
	return IIOCPEntry::BeginRead(retsError);
	}

bool CEsperSimpleConnection::BeginWrite (const SArchonMessage &Msg, const CString &sData, CString *retsError)

//	BeginWrite
//
//	Start a write operation and reply to the message when complete.

	{
	//	Remember the message to reply to.

	m_iState = stateReplyOnWrite;
	m_Msg = Msg;

	//	Let our subclass handle it.
	
	return IIOCPEntry::BeginWrite(sData, retsError);
	}

void CEsperSimpleConnection::ClearBusy (void)

//	ClearBusy
//
//	Clears busy flag

	{
	if (m_iState != stateBusy
			|| IsDeleted())
		return;

	m_iState = stateNone;
	}

void CEsperSimpleConnection::OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred)

//	OnSocketOperationComplete
//
//	Success!

	{
	//	Get stats now, in case things get destroyed.

	CEsperStats &Stats = m_Manager.GetStats();

	switch (m_iState)
		{
		case stateReplyOnRead:
			m_iState = stateNone;

#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Read %d bytes", CEsperInterface::ConnectionToFriendlyID(GetID()), dwBytesTransferred));
#endif
			m_Manager.SendMessageReplyOnRead(CDatum(GetID()), CString(GetBuffer()->GetPointer(), dwBytesTransferred), m_Msg);
			Stats.IncStat(CEsperStats::statBytesRead, dwBytesTransferred);
			break;

		case stateReplyOnWrite:
			m_iState = stateNone;

#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Wrote %d bytes", CEsperInterface::ConnectionToFriendlyID(GetID()), dwBytesTransferred));
#endif
			m_Manager.SendMessageReplyOnWrite(CDatum(GetID()), dwBytesTransferred, m_Msg);
			Stats.IncStat(CEsperStats::statBytesWritten, dwBytesTransferred);
			break;

		default:
			ASSERT(false);
		}
	}

void CEsperSimpleConnection::OnSocketOperationFailed (EOperations iOp)

//	OnSocketOperationFailed
//
//	If the operation failed, then we return a failure message to the client, and
//	disconnect.

	{
	switch (m_iState)
		{
		case stateReplyOnRead:
		case stateReplyOnWrite:
			m_Manager.LogTrace(strPattern("[%x] Disconnect on failure", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			m_Manager.SendMessageReplyDisconnect(m_Msg);
			m_Manager.DeleteConnection(CDatum(GetID()));
			break;

		default:
			ASSERT(false);
		}
	}

bool CEsperSimpleConnection::SetBusy (void)

//	SetBusy
//
//	This manages access to the connection from multiple threads. A thread should
//	call this before calling BeginRead, etc. to indicate that the connection is
//	about to be used (and to prevent other threads from using it at the same 
//	time).
//
//	If we return TRUE, then the connection is ready for use. BeginXXX must be
//	called (or else the connection will never become free).
//
//	If we return FALSE, then it means some other thread has grabbed this 
//	connection.
//
//	NOTE: Connection manager uses its own lock to control access to this 
//	operation. We do this so we don't have to keep a critical section for each
//	connection.
//
//	We break this up into two steps (SetBusy followed by BeginXXX) because we
//	don't want to lock connection manager during BeginXXX (because it might be
//	expensive).

	{
	if (m_iState != stateNone
			|| IsDeleted())
		return false;

	m_iState = stateBusy;
	return true;
	}
