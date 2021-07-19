//	CEsperAMP1ConnectionOut.cpp
//
//	CEsperAMP1ConnectionOut class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	See CEsperAMP1ConnectionIn for a description of the AMP1 protocol.

#include "stdafx.h"

DECLARE_CONST_STRING(AMP1_AUTH,							"AUTH");
DECLARE_CONST_STRING(AMP1_LEAVE,						"LEAVE");

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address");
DECLARE_CONST_STRING(FIELD_AUTH_KEY,					"authKey");
DECLARE_CONST_STRING(FIELD_AUTH_NAME,					"authName");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_OK,							"OK");

DECLARE_CONST_STRING(PROTOCOL_AMP1,						"AMP/1.00");

DECLARE_CONST_STRING(STR_RESERVED_LENGTH,				"ABCDEFGHIJKLMNOP");

DECLARE_CONST_STRING(ERR_LOST_CONNECTION,				"AMP1 connection lost.");
DECLARE_CONST_STRING(ERR_DATA_TOO_BIG,					"Data too big to serialize.");
DECLARE_CONST_STRING(ERR_INVALID_AMP1_KEYWORD,			"Invalid AMP1 keyword: %s.");
DECLARE_CONST_STRING(ERR_INVALID_STATE,					"Invalid state for CEsperAMP1ConnectionOut: %x.");
DECLARE_CONST_STRING(ERR_CANNOT_CONNECT,				"Unable to connect to %s on port %d: %s");

CEsperAMP1ConnectionOut::CEsperAMP1ConnectionOut (CEsperConnectionManager &Manager, const CString &sHostConnection, const CString &sAddress, DWORD dwPort) : 
		CEsperConnection(sAddress, dwPort),
		m_Manager(Manager),
		m_sHostConnection(sHostConnection)
		
//	CEsperAMP1ConnectionOut constructor

	{
	}

CEsperAMP1ConnectionOut::~CEsperAMP1ConnectionOut (void)

//	CEsperAMP1ConnectionOut destructor

	{
	}

void CEsperAMP1ConnectionOut::AccumulateResult (TArray<CString> &Result)

//	AccumulateResult
//
//	Appends the result of the last operation.

	{
	if (!m_sLastResult.IsEmpty())
		Result.Insert(strPattern("[%x] %s", CEsperInterface::ConnectionToFriendlyID(GetID()), m_sLastResult));
	}

void CEsperAMP1ConnectionOut::AccumulateStatus (SStatus *ioStatus)

//	AccumulateStatus
//
//	Accumulates connection stats

	{
	ioStatus->iTotalObjects++;

	switch (m_iState)
		{
		case stateWaitForConnect:
		case stateWaitForAuthAck:
		case stateWaitForRequestAck:
			ioStatus->iWaitingForWrite++;
			break;

		case stateWaitForAuthResponse:
		case stateWaitForResponse:
			ioStatus->iWaitingForRead++;
			break;

		default:
			ioStatus->iIdle++;
			break;
		}
	}

bool CEsperAMP1ConnectionOut::BeginAMP1Request (const SArchonMessage &Msg, const SAMP1Request &Request, CString *retsError)

//	BeginAMP1Request
//
//	Starts a request

	{
	CString sError;

	//	Prepare the request.

	m_Msg = Msg;
	m_sLastResult = NULL_STR;

	if (!SerializeAMP1Request(Request.sCommand, Request.dData, m_RequestBuffer, &sError))
		{
		m_sLastResult = sError;
		if (retsError) *retsError = sError;
		return false;
		}

	//	Allow reconnect in case of error.

	m_bReconnect = true;
	m_bResetBuffer = true;

	//	If this is a AMP1_LEAVE message, then delete the connection.

	m_bDeleteWhenDone = strEquals(Request.sCommand, AMP1_LEAVE);

	//	If we're not yet connected, we need to connect

	if (m_iState == stateDisconnectedBusy)
		{
		//	Get some options

		m_sAddress = Request.sAddress;
		m_dwPort = Request.dwPort;
		m_sAuthName = Request.sSenderName;
		m_AuthKey = Request.SenderKey;

		//	Connect. Even if this call fails, it handles replying to the client,
		//	so it is OK to always return TRUE.

		OpConnect();
		}

	//	Otherwise, send

	else if (m_iState == stateConnectedBusy)
		{
		OpSendRequest();
		}

	//	Otherwise, error

	else
		{
		m_sLastResult = strPattern(ERR_INVALID_STATE, (DWORD)m_iState);
		if (retsError) *retsError = m_sLastResult;
		return false;
		}

#ifdef DEBUG_AMP1
	printf("[%x] Sending %s to %s. %d bytes\n", CEsperInterface::ConnectionToFriendlyID(GetID()), (LPSTR)Request.sCommand, (LPSTR)m_sAuthName, m_RequestBuffer.GetLength());
#endif

	//	Success

	return true;
	}

void CEsperAMP1ConnectionOut::ClearBusy (void)

//	SetBusy
//
//	Clears the busy mode flag. Callers must guarantee synchronization (e.g., via manager lock).

	{
	if (IsDeleted())
		return;

	switch (m_iState)
		{
		case stateConnectedBusy:
			m_iState = stateConnected;
			break;

		case stateDisconnectedBusy:
			m_iState = stateDisconnected;
			break;
		}
	}

CDatum CEsperAMP1ConnectionOut::GetProperty (const CString &sProperty) const

//	GetProperty
//
//	Returns properties.

	{
	if (strEquals(sProperty, FIELD_ADDRESS))
		return CDatum(m_sAddress);
	else
		return CDatum();
	}

void CEsperAMP1ConnectionOut::OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred)

//	OnSocketOperationComplete
//
//	Success

	{
	switch (m_iState)
		{
		case stateWaitForConnect:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Connected to %s:%d.", CEsperInterface::ConnectionToFriendlyID(GetID()), m_sAddress, m_dwPort));
#endif
			//	Since we succeeded in connecting on this request, we don't 
			//	bother trying to reconnect.

			m_bReconnect = false;

			//	If we have an auth name, then authenticate the connection

			if (!m_sAuthName.IsEmpty())
				OpSendAuth();

			//	Otherwise, just send the request

			else
				OpSendRequest();

			break;
			}

		case stateWaitForAuthAck:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Request sent %d bytes.", CEsperInterface::ConnectionToFriendlyID(GetID()), dwBytesTransferred));
#endif
			//	Start reading

			OpRead(stateWaitForAuthResponse);
			break;
			}

		case stateWaitForAuthResponse:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Received %d bytes.", CEsperInterface::ConnectionToFriendlyID(GetID()), GetBuffer()->GetLength()));
#endif
			//	Now send the actual request

			OpSendRequest();
			break;
			}

		case stateWaitForRequestAck:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Request sent %d bytes.", CEsperInterface::ConnectionToFriendlyID(GetID()), dwBytesTransferred));
#endif
			//	Start reading

			OpRead();
			break;
			}

		case stateWaitForResponse:
			{
#ifdef DEBUG_SOCKET_OPS
			m_Manager.LogTrace(strPattern("[%x] Received %d bytes.", CEsperInterface::ConnectionToFriendlyID(GetID()), GetBuffer()->GetLength()));
#endif
			//	Process message
			//	LATER: Parse the response and see if it's an error.

			m_iState = stateConnected;
			m_sLastResult = MSG_OK;
			m_Manager.SendMessageReply(MSG_OK, CDatum(), m_Msg);

			if (m_bDeleteWhenDone)
				m_Manager.DeleteConnection(CDatum(GetID()));
			break;
			}
		}
	}

void CEsperAMP1ConnectionOut::OnSocketOperationFailed (EOperations iOp)

//	OnSocketOperationFailed
//
//	Failure

	{
	switch (m_iState)
		{
		case stateWaitForAuthAck:
		case stateWaitForAuthResponse:
		case stateWaitForRequestAck:
		case stateWaitForResponse:
			OpTransmissionFailed(strPattern("[%x] AMP1 socket operation failed. State = %d", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), (int)m_iState));
			break;

		default:
			m_Manager.LogTrace(strPattern("[%x] Disconnect on failure", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			m_sLastResult = ERR_LOST_CONNECTION;
			m_Manager.SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_LOST_CONNECTION, m_Msg);
			m_Manager.DeleteConnection(CDatum(GetID()));
			break;
		}
	}

bool CEsperAMP1ConnectionOut::OpConnect (bool bReconnect)

//	OpConnect
//
//	Connects to host

	{
	CString sError;

#ifdef DEBUG_SOCKET_OPS
	if (bReconnect)
		m_Manager.LogTrace(strPattern("[%x] Reconnecting to %s:%d.", CEsperInterface::ConnectionToFriendlyID(GetID()), m_sAddress, m_dwPort));
	else
		m_Manager.LogTrace(strPattern("[%x] Connecting to %s:%d.", CEsperInterface::ConnectionToFriendlyID(GetID()), m_sAddress, m_dwPort));
#endif

	//	Reset the socket, if necessary

	if (bReconnect)
		{
		m_bReconnect = false;
		m_Manager.ResetConnection(CDatum(GetID()));
		}

	//	Begin a connection

	m_iState = stateWaitForConnect;
	if (!IIOCPEntry::BeginConnection(m_sAddress, m_dwPort, &sError))
		{
		//	Reset the connection for next time.

		m_iState = stateDisconnected;
		m_Manager.ResetConnection(CDatum(GetID()));

#ifdef DEBUG_SOCKET_OPS
		m_Manager.LogTrace(strPattern("[%x] Connect failed.", CEsperInterface::ConnectionToFriendlyID(GetID())));
#endif
		m_sLastResult = strPattern(ERR_CANNOT_CONNECT, m_sAddress, m_dwPort, sError);
		m_Manager.SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, m_sLastResult, m_Msg);
		return false;
		}

	//	Done

	return true;
	}

bool CEsperAMP1ConnectionOut::OpRead (EStates iNewState)

//	OpRead
//
//	Initiates a read operation.

	{
	CString sError;

	m_iState = iNewState;
	if (!IIOCPEntry::BeginRead(&sError))
		return OpTransmissionFailed(strPattern("[%x] AMP1 read operation failed. State = %d", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), (int)m_iState));

	return true;
	}

bool CEsperAMP1ConnectionOut::OpSendAuth (void)

//	OpSendAuth
//
//	Sends an AUTH command.

	{
	//	Generate the AUTH request

	CComplexStruct *pData = new CComplexStruct;
	pData->SetElement(FIELD_AUTH_NAME, m_sAuthName);
	pData->SetElement(FIELD_AUTH_KEY, m_AuthKey);

#ifdef DEBUG_AMP1
	printf("[%x] Sending AUTH to %s\n", CEsperInterface::ConnectionToFriendlyID(GetID()), (LPSTR)m_sAuthName);
#endif

	CStringBuffer Request;
	CString sError;
	if (!SerializeAMP1Request(AMP1_AUTH, CDatum(pData), Request, &sError))
		return false;

	//	Write it out

	OpWrite(Request, stateWaitForAuthAck);

	//	Done

	return true;
	}

bool CEsperAMP1ConnectionOut::OpSendRequest (void)

//	OpSendRequest
//
//	Makes an AMP1 request.

	{
	OpWrite(m_RequestBuffer, stateWaitForRequestAck);

	//	Done

	return true;
	}

bool CEsperAMP1ConnectionOut::OpTransmissionFailed (const CString &sError)

//	OpTransmissionFailed
//
//	Failed

	{
#ifdef DEBUG_SOCKET_OPS
	m_Manager.LogTrace(strPattern("[%x] Unable to communicate on socket: %s.", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), (LPSTR)sError));
#endif

	//	If we failed, try to reconnect.

	if (m_bReconnect)
		return OpConnect(true);

	//	Otherwise, we reply with an error

	else
		{
		m_sLastResult = sError;
		m_Manager.SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, m_Msg);

		//	Delete the connection; otherwise we'll keep getting called here.

#ifdef DEBUG_SOCKET_OPS
		m_Manager.LogTrace(strPattern("[%x] Deleting connection.", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
#endif
		m_Manager.DeleteConnection(CDatum(GetID()));
		return false;
		}
	}

bool CEsperAMP1ConnectionOut::OpWrite (const CString &sData, EStates iNewState)

//	OpWrite
//
//	Initiates a write operation

	{
	CString sError;

	m_iState = iNewState;
	if (!IIOCPEntry::BeginWrite(sData, &sError))
		return OpTransmissionFailed(strPattern("[%x] AMP1 write operation failed. State = %d", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), (int)m_iState));

	return true;
	}

bool CEsperAMP1ConnectionOut::SerializeAMP1Request (const CString &sCommand, CDatum dData, CStringBuffer &Stream, CString *retsError)

//	SerializeAMP1Request
//
//	Serializes the request

	{
	//	Make sure the command is valid for AMP1

	char *pPos = sCommand.GetParsePointer();
	char *pPosEnd = pPos + sCommand.GetLength();
	while (pPos < pPosEnd)
		{
		//	We accept A-Z, a-z, 0-9, -, _, and $

		if ((*pPos >= 'A' && *pPos <= 'Z')
				|| (*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= '0' && *pPos <= '9')
				|| *pPos == '-'
				|| *pPos == '_'
				|| *pPos == '$')
			pPos++;
		else
			{
			*retsError = strPattern(ERR_INVALID_AMP1_KEYWORD, sCommand);
			return false;
			}
		}

	//	Write

	Stream.Seek(0);
	Stream.Write(PROTOCOL_AMP1);
	Stream.Write(" ", 1);
	Stream.Write(sCommand);
	Stream.Write(" ", 1);

	//	Remember our current positions so we can come back and write the length.
	//	Reserve some space for the length.

	int iLengthPos = Stream.GetPos();
	Stream.Write(STR_RESERVED_LENGTH);

	//	End of header

	Stream.Write("\r\n", 2);

	//	Write the data (and keep track of how much we write)

	int iStartPos = Stream.GetPos();
	dData.Serialize(CDatum::formatAEONScript, Stream);
	int iDataLength = Stream.GetPos() - iStartPos;

	//	Write the end

	Stream.Write("\r\n", 2);

	//	Now go back and write the length

	CString sPattern = strPattern("%%%dd", STR_RESERVED_LENGTH.GetLength());
	CString sLength = strPattern(sPattern, iDataLength);
	if (sLength.GetLength() != STR_RESERVED_LENGTH.GetLength())
		{
		*retsError = ERR_DATA_TOO_BIG;
		return false;
		}

	int iEndPos = Stream.GetPos();
	Stream.Seek(iLengthPos);
	Stream.Write(sLength);

	//	Truncate at the end

	Stream.SetLength(iEndPos);

	//	Done

	return true;
	}

bool CEsperAMP1ConnectionOut::SetBusy (void)

//	SetBusy
//
//	Sets the connection to busy mode

	{
	if (IsDeleted())
		return false;

	switch (m_iState)
		{
		case stateConnected:
			m_iState = stateConnectedBusy;
			return true;

		case stateDisconnected:
			m_iState = stateDisconnectedBusy;
			return true;

		default:
			return false;
		}
	}
