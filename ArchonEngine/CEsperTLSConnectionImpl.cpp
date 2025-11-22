//	CEsperTLSConnectionImpl.cpp
//
//	CEsperTLSConnectionImpl class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_TLS
#endif

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(ERR_SSL_INVALID_STATE,				"SSL invalid state.");
DECLARE_CONST_STRING(ERR_SSL_ERROR,						"[%x] SSL error: %s");
DECLARE_CONST_STRING(ERR_READ_OP_FAILED,				"[%x] Unable to read from SSL connection.");
DECLARE_CONST_STRING(ERR_WRITE_OP_FAILED,				"[%x] Unable to write to SSL connection.");
DECLARE_CONST_STRING(ERR_UNEXPECTED_SSL_CONNECT,		"[%x] Unexpected SSL connect state.");
DECLARE_CONST_STRING(ERR_UNEXPECTED_SSL_READ,			"[%x] Unexpected SSL read state.");
DECLARE_CONST_STRING(ERR_UNEXPECTED_SSL_WRITE,			"[%x] Unexpected SSL write state.");

const DWORDLONG ACTIVE_TIME_THRESHOLD =					5 * 60 * 1000;

CEsperTLSConnectionImpl::CEsperTLSConnectionImpl (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLCtx& SSLCtx, SOCKET hSocket) : CEsperConnection(hSocket),
		m_Manager(Manager),
		m_sListener(sListener),
		m_sNetworkAddress(sNetworkAddress),
		m_pSSL(NULL)

//	CEsperTLSConnectionImpl constructor

	{
	m_pSSL = new CSSLAsyncEngine(&SSLCtx);
	}

CEsperTLSConnectionImpl::CEsperTLSConnectionImpl (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLAsyncEngine* pSSL, SOCKET hSocket) : CEsperConnection(hSocket),
		m_Manager(Manager),
		m_sListener(sListener),
		m_sNetworkAddress(sNetworkAddress),
		m_pSSL(pSSL)

//	CEsperTLSConnectionImpl constructor

	{
	}

CEsperTLSConnectionImpl::~CEsperTLSConnectionImpl (void)

//	CEsperTLSConnectionImpl destructor

	{
	if (m_pSSL)
		delete m_pSSL;
	}

void CEsperTLSConnectionImpl::AccumulateStatus (SStatus *ioStatus)

//	AccumulateStatus
//
//	Return status

	{
	CSmartLock Lock(m_cs);

	ioStatus->iTotalObjects++;

	if (m_bReadRequested)
		ioStatus->iWaitingForRead++;

	if (m_bWriteRequested)
		ioStatus->iWaitingForWrite++;

	//	Idle?

	if (!m_bReadRequested && !m_bWriteRequested)
		ioStatus->iIdle++;

	//	Active?

	if (sysGetTicksElapsed(GetLastActivityTime()) <= ACTIVE_TIME_THRESHOLD)
		ioStatus->iActive++;
	}

bool CEsperTLSConnectionImpl::BeginRead (const SArchonMessage &Msg, CString *retsError)

//	BeginRead
//
//	Client asks us to read data. When we've read something, we'll reply with
//	Esper.onRead.

	{
	CSmartLock Lock(m_cs);

	ASSERT(m_pSSL);
	if (m_pSSL == NULL || !m_bReadRequested)
		{
		if (retsError) *retsError = ERR_SSL_INVALID_STATE;
		return false;
		}

	//	Receive

#ifdef DEBUG_TLS
	printf("BeginRead\n");
#endif

	DebugPerfReset();

	OnTLSBeginRead(Msg);
	m_pSSL->Receive();
	return OpProcessSSL(retsError);
	}

bool CEsperTLSConnectionImpl::BeginWrite (const SArchonMessage &Msg, const CString &sData, CString *retsError)

//	BeginWrite
//
//	Client asks us to write data

	{
	CSmartLock Lock(m_cs);

	ASSERT(m_pSSL);
	if (m_pSSL == NULL || !m_bWriteRequested)
		{
		if (retsError) *retsError = ERR_SSL_INVALID_STATE;
		return false;
		}

	//	Send

#ifdef DEBUG_TLS
	printf("BeginWrite\n");
#endif

	DebugPerfReset();
	DebugPerfInit();

	OnTLSBeginWrite(Msg);
	CStringBuffer Buffer(sData);
	m_pSSL->Send(Buffer);
	m_dwBytesToSend = sData.GetLength();
	return OpProcessSSL(retsError);
	}

void CEsperTLSConnectionImpl::DeleteConnection ()

//	DeleteConnection
//
//	Delete this connection.

	{
	CSmartLock Lock(m_cs);

	m_bReadRequested = false;
	m_bWriteRequested = false;
	m_iReadState = stateNone;
	m_iWriteState = stateNone;

	SetMarkedForDelete();
	}

void CEsperTLSConnectionImpl::OnConnect (void)

//	OnConnect
//
//	Someone connected to us

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG_TLS_CONNECTION
	m_Manager.LogTrace(strPattern("[%x] TLS Connection OnConnect", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
#endif

	m_pSSL->Accept();

	m_bConnectedRequested = true;
	OpProcessSSL();
	}

void CEsperTLSConnectionImpl::OnSocketOperationComplete (EOperation iOp, DWORD dwBytesTransferred)

//	OnSocketOperationComplete
//
//	Success

	{
	CSmartLock Lock(m_cs);

	CEsperStats &Stats = m_Manager.GetStats();

	DebugPerfInit();
	DebugPerfData(dwBytesTransferred);

	//	Report stats

	switch (iOp)
		{
		case EOperation::read:
			{
			m_bReadSubmitted = false;
			switch (m_iReadState)
				{
				case stateWaitToReceiveSSLData:
					{
#ifdef DEBUG_TLS
					printf("received SSL data\n");
					printf("DATA: %s\n", GetReadBuffer()->GetPointer());
#endif

					m_iReadState = stateNone;
					m_pSSL->ProcessReceiveData(*GetReadBuffer());
					OpProcessSSL();
					break;
					}

				default:
					break;
				}

			Stats.IncStat(CEsperStats::statBytesRead, dwBytesTransferred);
			break;
			}

		case EOperation::write:
			{
			m_bWriteSubmitted = false;
			switch (m_iWriteState)
				{
				case stateWaitToSendSSLData:
					{
#ifdef DEBUG_TLS
					printf("sent SSL data\n");
#endif

					m_iWriteState = stateNone;
					OpProcessSSL();
					break;
					}

				case stateWaitToSendSSLDataThenReceive:
					{
#ifdef DEBUG_TLS
					printf("sent SSL data, now receiving\n");
#endif

					m_iWriteState = stateNone;
					OpRead(stateWaitToReceiveSSLData);
					break;
					}

				default:
					break;
				}

			Stats.IncStat(CEsperStats::statBytesWritten, dwBytesTransferred);
			break;
			}

		default:
			break;
		}
	}

void CEsperTLSConnectionImpl::OnSocketOperationFailed (EOperation iOp, CStringView sError)

//	OnSocketOperationFailed
//
//	Failure

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG_TLS_CONNECTION
	m_Manager.LogTrace(strPattern("[%x] Disconnect on failure", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
#endif

	OnTLSDisconnect();
	DeleteConnection();
	}

void CEsperTLSConnectionImpl::OnSSLOperationComplete (EOperation iOp)

//	OnSSLOperationComplete
//
//	SSL operation finished.

	{
	switch (iOp)
		{
		case EOperation::connect:
			m_iWriteState = stateNone;
			if (m_bConnectedRequested)
				{
				m_bConnectedRequested = false;
				OnTLSConnect();
				}
			else
				m_Manager.Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_SSL_CONNECT, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			break;

		case EOperation::read:
			m_iReadState = stateNone;
			if (m_bReadRequested)
				{
				m_bReadRequested = false;
				OnTLSRead(m_pSSL->GetBufferHandoff());
				}
			else
				m_Manager.Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_SSL_READ, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			DebugPerfReport();
			break;

		case EOperation::write:
			m_iWriteState = stateNone;
			if (m_bWriteRequested)
				{
				m_bWriteRequested = false;
				OnTLSWriteComplete(m_dwBytesToSend);
				OpProcessSSL();
				}
			else
				m_Manager.Log(MSG_LOG_ERROR, strPattern(ERR_UNEXPECTED_SSL_WRITE, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			DebugPerfReport();
			break;

		default:
			ASSERT(false);
			break;
		}
	}

bool CEsperTLSConnectionImpl::OpProcessSSL (CString *retsError)

//	OpProcessSSL
//
//	Process SSL until we are ready.

	{
#ifdef DEBUG_TLS_CONNECTION
	m_Manager.LogTrace(strPattern("[%x] TLS Process SSL state: %d", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), m_pSSL->GetInternalState()));
#endif

	CString sError;
	switch (m_pSSL->Process(&sError))
		{
		case CSSLAsyncEngine::resReadyConnect:
			OnSSLOperationComplete(EOperation::connect);
			return true;

		case CSSLAsyncEngine::resReadyRead:
			OnSSLOperationComplete(EOperation::read);
			return true;

		case CSSLAsyncEngine::resReadyWrite:
			OnSSLOperationComplete(EOperation::write);
			return true;

		case CSSLAsyncEngine::resReadyIdle:
//			OnSSLOperationComplete(EOperation::connect);
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
			m_iReadState = stateNone;
			m_iWriteState = stateNone;
			if (!IsDeleted())
				{
#ifdef DEBUG_TLS_CONNECTION
				m_Manager.LogTrace(strPattern(ERR_SSL_ERROR, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), sError));
#endif
				OnTLSDisconnect();
				DeleteConnection();
				}

			if (retsError) *retsError = sError;
			return false;

		default:
			ASSERT(false);
			return false;
		}
	}

bool CEsperTLSConnectionImpl::OpRead (EStates iNewState)

//	OpRead
//
//	Initiates a read operation.

	{
	CString sError;

	//	If we've already got an outstanding read, then we don't need to do anything.

	if (m_bReadSubmitted)
		return true;

#ifdef DEBUG_TLS
	printf("OpRead\n");
#endif

	m_iReadState = iNewState;
	m_bReadSubmitted = true;
	if (!IIOCPEntry::BeginRead(&sError))
		{
		m_bReadSubmitted = false;
		if (!IsDeleted())
			{
#ifdef DEBUG_TLS_CONNECTION
			m_Manager.LogTrace(strPattern(ERR_READ_OP_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), sError));
#endif
			OnTLSDisconnect();
			DeleteConnection();
			}
		return false;
		}

	return true;
	}

bool CEsperTLSConnectionImpl::OpWrite (const CString &sData, EStates iNewState)

//	OpWrite
//
//	Initiates a write operation

	{
	CString sError;

#ifdef DEBUG_TLS
	printf("OpWrite\n");
	printf("DATA: %s\n", sData.GetParsePointer());
#endif

	m_iWriteState = iNewState;
	m_bWriteSubmitted = true;
	if (!IIOCPEntry::BeginWrite(sData, &sError))
		{
		m_bWriteSubmitted = false;
		if (!IsDeleted())
			{
#ifdef DEBUG_TLS_CONNECTION
			m_Manager.LogTrace(strPattern(ERR_WRITE_OP_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
#endif
			OnTLSDisconnect();
			DeleteConnection();
			}
		return false;
		}

	return true;
	}

bool CEsperTLSConnectionImpl::SetBusy (EOperation iOperation)

//	SetBusy
//
//	Begin use of connection

	{
	CSmartLock Lock(m_cs);

	if (IsDeleted())
		return false;
	else if (iOperation == EOperation::read)
		{
		if (m_bReadRequested || m_iReadState != stateNone)
			return false;

		m_bReadRequested = true;
		return true;
		}
	else if (iOperation == EOperation::write)
		{
		if (m_bWriteRequested || m_iWriteState != stateNone)
			return false;

		m_bWriteRequested = true;
		return true;
		}
	else
		{
		m_iWriteState = stateBusy;
		return true;
		}
	}

bool CEsperTLSConnectionImpl::SetProperty (const CString &sProperty, CDatum dValue)

//	SetProperty
//
//	Sets a property

	{
	//	We don't have any special properties.

	return false;
	}

CEsperConnection* CEsperTLSConnectionImpl::UpgradeWebSocket ()

//	UpgradeWebSocket
//
//	Returns a new connection that is a WebSocket connection, and transfer any
//	resources (including the socket) to the new connection.

	{
	CSmartLock Lock(m_cs);

	//	Take ownership of the socket and SSL structure

	SOCKET hSocket = GetSocketHandoff();
	if (hSocket == INVALID_SOCKET)
		return NULL;

	CSSLAsyncEngine* pSSL = m_pSSL;
	m_pSSL = NULL;

	//	Create a WebSocket connection

	return new CEsperWSSConnectionIn(m_Manager, m_sListener, m_sNetworkAddress, pSSL, hSocket);
	}
