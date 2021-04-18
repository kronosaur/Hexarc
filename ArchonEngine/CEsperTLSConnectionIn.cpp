//	CEsperTLSConnectionIn.cpp
//
//	CEsperTLSConnectionIn class
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_TLS
#endif

DECLARE_CONST_STRING(ERR_SSL_INVALID_STATE,				"SSL invalid state.")
DECLARE_CONST_STRING(ERR_SSL_ERROR,						"[%x] SSL error: %s")
DECLARE_CONST_STRING(ERR_READ_OP_FAILED,				"[%x] Unable to read from SSL connection.")
DECLARE_CONST_STRING(ERR_WRITE_OP_FAILED,				"[%x] Unable to write to SSL connection.")

const DWORDLONG ACTIVE_TIME_THRESHOLD =					5 * 60 * 1000;

CEsperTLSConnectionIn::CEsperTLSConnectionIn (CEsperConnectionManager &Manager, const CString &sListener, const CString &sNetworkAddress, CSSLCtx &SSLCtx, const SArchonMessage &Msg, SOCKET hSocket) : CEsperConnection(hSocket),
		m_Manager(Manager),
		m_sListener(sListener),
		m_sNetworkAddress(sNetworkAddress),
		m_iState(stateNone),
		m_iSSLSavedState(stateNone),
		m_Msg(Msg),
		m_pSSL(NULL)

//	CEsperTLSConnectionIn constructor

	{
	m_pSSL = new CSSLAsyncEngine(&SSLCtx);
	}

CEsperTLSConnectionIn::~CEsperTLSConnectionIn (void)

//	CEsperTLSConnectionIn destructor

	{
	if (m_pSSL)
		delete m_pSSL;
	}

void CEsperTLSConnectionIn::AccumulateStatus (SStatus *ioStatus)

//	AccumulateStatus
//
//	Return status

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

bool CEsperTLSConnectionIn::BeginRead (const SArchonMessage &Msg, CString *retsError)

//	BeginRead
//
//	Client asks us to read data. When we've read something, we'll reply with
//	Esper.onRead.

	{
	ASSERT(m_pSSL);
	ASSERT(m_iState == stateBusy);
	if (m_pSSL == NULL || m_iState != stateBusy)
		{
		if (retsError) *retsError = ERR_SSL_INVALID_STATE;
		return false;
		}

	//	Remember the message to reply to

	m_Msg = Msg;

	//	Receive

#ifdef DEBUG_TLS
	printf("BeginRead\n");
#endif

	m_pSSL->Receive();
	m_iSSLSavedState = stateReplyOnRead;
	return OpProcessSSL(retsError);
	}

bool CEsperTLSConnectionIn::BeginWrite (const SArchonMessage &Msg, const CString &sData, CString *retsError)

//	BeginWrite
//
//	Client asks us to write data

	{
	ASSERT(m_pSSL);
	ASSERT(m_iState == stateBusy);
	if (m_pSSL == NULL || m_iState != stateBusy)
		{
		if (retsError) *retsError = ERR_SSL_INVALID_STATE;
		return false;
		}

	//	Remember the message to reply to

	m_Msg = Msg;

	//	Send

#ifdef DEBUG_TLS
	printf("BeginWrite\n");
#endif

	CStringBuffer Buffer(sData);
	m_pSSL->Send(Buffer);
	m_iSSLSavedState = stateReplyOnWrite;
	m_dwBytesToSend = sData.GetLength();
	return OpProcessSSL(retsError);
	}

void CEsperTLSConnectionIn::ClearBusy (void)

//	ClearBusy
//
//	We're not in use.

	{
	if (m_iState != stateBusy
			|| IsDeleted())
		return;

	m_iState = stateNone;
	}

void CEsperTLSConnectionIn::OnConnect (void)

//	OnConnect
//
//	Someone connected to us

	{
	m_Manager.LogTrace(strPattern("[%x] TLS Connection OnConnect", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));

	m_pSSL->Accept();

	m_iSSLSavedState = stateReplyOnConnect;
	OpProcessSSL();
	}

void CEsperTLSConnectionIn::OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred)

//	OnSocketOperationComplete
//
//	Success

	{
	CEsperStats &Stats = m_Manager.GetStats();

	switch (m_iState)
		{
		case stateWaitToReceiveSSLData:
			{
#ifdef DEBUG_TLS
			printf("received SSL data\n");
			printf("DATA: %s\n", GetBuffer()->GetPointer());
#endif

			m_pSSL->ProcessReceiveData(*GetBuffer());
			OpProcessSSL();
			break;
			}

		case stateWaitToSendSSLData:
			{
#ifdef DEBUG_TLS
			printf("sent SSL data\n");
#endif

			OpProcessSSL();
			break;
			}

		case stateWaitToSendSSLDataThenReceive:
			{
#ifdef DEBUG_TLS
			printf("sent SSL data, now receiving\n");
#endif

			OpRead(stateWaitToReceiveSSLData);
			break;
			}

		case stateWaitToSendSSLDataThenReady:
			{
#ifdef DEBUG_TLS
			printf("send SSL data, now ready\n");
#endif

			OnSSLOperationComplete();
			break;
			}

		default:
			ASSERT(false);
			break;
		}

	//	Report stats

	switch (iOp)
		{
		case opRead:
			Stats.IncStat(CEsperStats::statBytesRead, dwBytesTransferred);
			break;

		case opWrite:
			Stats.IncStat(CEsperStats::statBytesWritten, dwBytesTransferred);
			break;
		}
	}

void CEsperTLSConnectionIn::OnSocketOperationFailed (EOperations iOp)

//	OnSocketOperationFailed
//
//	Failure

	{
	switch (m_iState)
		{
		case stateReplyOnRead:
		case stateReplyOnWrite:
		case stateWaitToReceiveSSLData:
		case stateWaitToSendSSLData:
		case stateWaitToSendSSLDataThenReceive:
		case stateWaitToSendSSLDataThenReady:
			m_Manager.LogTrace(strPattern("[%x] Disconnect on failure", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			m_Manager.SendMessageReplyDisconnect(m_Msg);
			m_Manager.DeleteConnection(CDatum(GetID()));
			break;

		default:
			ASSERT(false);
			break;
		}
	}

void CEsperTLSConnectionIn::OnSSLOperationComplete (void)

//	OnSSLOperationComplete
//
//	SSL operation finished.

	{
	switch (m_iSSLSavedState)
		{
		case stateReplyOnConnect:
#ifdef DEBUG_TLS
			printf("Success: OnConnect\n");
#endif

			//	Done with SSL stuff; wait for callers to read or write.

			m_iState = stateNone;
			m_Manager.SendMessageReplyOnConnect(CDatum(GetID()), m_sListener, m_sNetworkAddress, m_Msg);
			break;

		case stateReplyOnRead:
			{
#ifdef DEBUG_TLS
			printf("Success: OnRead\n");
#endif

			m_iState = stateNone;
			const IMemoryBlock &Data = m_pSSL->GetBuffer();
			CString sData(Data.GetPointer(), Data.GetLength());
			m_Manager.SendMessageReplyOnRead(CDatum(GetID()), sData, m_Msg);
			break;
			}

		case stateReplyOnWrite:
#ifdef DEBUG_TLS
			printf("Success: OnWrite\n");
#endif
			m_iState = stateNone;
			m_Manager.SendMessageReplyOnWrite(CDatum(GetID()), m_dwBytesToSend, m_Msg);
			break;

		default:
			ASSERT(false);
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Unhandled state.", CEsperInterface::ConnectionToFriendlyID(GetID())));
#endif
			break;
		}
	}

bool CEsperTLSConnectionIn::OpProcessSSL (CString *retsError)

//	OpProcessSSL
//
//	Process SSL until we are ready.

	{
	m_Manager.LogTrace(strPattern("[%x] TLS Process SSL state: %d", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), m_pSSL->GetInternalState()));

	CString sError;
	switch (m_pSSL->Process(&sError))
		{
		case CSSLAsyncEngine::resReady:
			OnSSLOperationComplete();
			return true;

		case CSSLAsyncEngine::resReceiveData:
			{
			if (m_pSSL->ProcessHasDataToSend())
				{
				CStringBuffer Buffer;
				m_pSSL->ProcessSendData(Buffer);
				OpWrite(Buffer, stateWaitToSendSSLDataThenReceive);
				}
			else
				OpRead(stateWaitToReceiveSSLData);

			return true;
			}

		case CSSLAsyncEngine::resSendData:
			{
			CStringBuffer Buffer;
			m_pSSL->ProcessSendData(Buffer);
			OpWrite(Buffer, stateWaitToSendSSLData);
			return true;
			}

		case CSSLAsyncEngine::resError:
			m_iState = stateNone;
			if (!IsDeleted())
				{
				m_Manager.LogTrace(strPattern(ERR_SSL_ERROR, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), sError));
				m_Manager.SendMessageReplyDisconnect(m_Msg);
				m_Manager.DeleteConnection(CDatum(GetID()));
				}

			if (retsError) *retsError = sError;
			return false;

		default:
			ASSERT(false);
			return false;
		}
	}

bool CEsperTLSConnectionIn::OpRead (EStates iNewState)

//	OpRead
//
//	Initiates a read operation.

	{
	CString sError;

#ifdef DEBUG_TLS
	printf("OpRead\n");
#endif

	m_iState = iNewState;
	if (!IIOCPEntry::BeginRead(&sError))
		{
		if (!IsDeleted())
			{
			m_Manager.LogTrace(strPattern(ERR_READ_OP_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), sError));
			m_Manager.SendMessageReplyDisconnect(m_Msg);
			m_Manager.DeleteConnection(CDatum(GetID()));
			}
		return false;
		}

	return true;
	}

bool CEsperTLSConnectionIn::OpWrite (const CString &sData, EStates iNewState)

//	OpWrite
//
//	Initiates a write operation

	{
	CString sError;

#ifdef DEBUG_TLS
	printf("OpWrite\n");
	printf("DATA: %s\n", sData.GetParsePointer());
#endif

	m_iState = iNewState;
	if (!IIOCPEntry::BeginWrite(sData, &sError))
		{
		if (!IsDeleted())
			{
			m_Manager.LogTrace(strPattern(ERR_WRITE_OP_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			m_Manager.SendMessageReplyDisconnect(m_Msg);
			m_Manager.DeleteConnection(CDatum(GetID()));
			}
		return false;
		}

	return true;
	}

bool CEsperTLSConnectionIn::SetBusy (void)

//	SetBusy
//
//	Begin use of connection

	{
	if (m_iState != stateNone
			|| IsDeleted())
		return false;

	m_iState = stateBusy;
	return true;
	}

bool CEsperTLSConnectionIn::SetProperty (const CString &sProperty, CDatum dValue)

//	SetProperty
//
//	Sets a property

	{
	//	We don't have any special properties.

	return false;
	}
