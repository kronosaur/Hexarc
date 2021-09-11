//	CEsperAMP1ConnectionIn.cpp
//
//	CEsperAMP1ConnectionIn class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	AMP1 PROTOCOL
//
//	AMP/1.00 {keyword} {data-length} CRLF
//	{data} CRLF
//
//	Each request has five elements, separated by whitespace. Each element is 
//	described below:
//
//	keyword: This is a token consisting of A-Z, a-z, 0-9, -, _, and $. It 
//		identifies the command being issued.
//
//	data-length: This is the length of the data element, in bytes.
//
//	data: The data element is a serialized Aeon datum. The contents of the data 
//		depend on the command.

#include "stdafx.h"

DECLARE_CONST_STRING(AMP1_AUTH,							"AUTH");
DECLARE_CONST_STRING(AMP1_REPLY_OK,						"AMP/1.00 OK\r\n");

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address");
DECLARE_CONST_STRING(FIELD_AUTH_NAME,					"authName");
DECLARE_CONST_STRING(FIELD_AUTH_KEY,					"authKey");

DECLARE_CONST_STRING(MSG_ESPER_ON_AMP1,					"Esper.onAMP1");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(PROTOCOL_AMP1,						"AMP/1.00");

DECLARE_CONST_STRING(ERR_INVALID_MESSAGE,				"[%x] Cannot understand AMP1 message: %s");
DECLARE_CONST_STRING(ERR_READ_START_FAILED,				"[%x] Unable to initiate read from AMP1 connection: %s");
DECLARE_CONST_STRING(ERR_READ_OP_FAILED,				"[%x] Unable to read from AMP1 connection.");
DECLARE_CONST_STRING(ERR_WRITE_OP_FAILED,				"[%x] Unable to write to AMP1 connection.");
DECLARE_CONST_STRING(ERR_INVALID_PROTOCOL,				"Unknown protocol: %s.");
DECLARE_CONST_STRING(ERR_INVALID_HEADER,				"Invalid header.");
DECLARE_CONST_STRING(ERR_INVALID_DATA_LEN,				"Invalid data length: %s.");
DECLARE_CONST_STRING(ERR_INVALID_AUTH,					"Invalid machine authentication from %s.");

CEsperAMP1ConnectionIn::CEsperAMP1ConnectionIn (CEsperConnectionManager &Manager, const CString &sClientAddr, SOCKET hSocket) : CEsperConnection(hSocket),
		m_Manager(Manager),
		m_sClientAddr(sClientAddr),
		m_iState(stateNone)

//	CEsperAMP1ConnectionIn constructor

	{
	}

void CEsperAMP1ConnectionIn::AccumulateStatus (SStatus *ioStatus)

//	AccumulateStatus
//
//	Return status

	{
	ioStatus->iTotalObjects++;

	switch (m_iState)
		{
		case stateReadingHeader:
			ioStatus->iIdle++;
			break;

		case stateReadingHeaderContinues:
		case stateReadingData:
			ioStatus->iWaitingForRead++;
			break;

		case stateWriting:
			ioStatus->iWaitingForWrite++;
			break;

		default:
			ioStatus->iIdle++;
			break;
		}
	}

void CEsperAMP1ConnectionIn::ClearBusy (void)

//	ClearBusy
//
//	We're not in use

	{
	//	We don't take direct orders from clients (read/write), so we don't have 
	//	to do anything here.
	}

bool CEsperAMP1ConnectionIn::GetHeader (const IMemoryBlock &Data, CString *retsCommand, DWORD *retdwDataLen, char **retpPartialData, DWORD *retdwPartialDataLen)

//	GetHeader
//
//	Parses Data and sees if we have a complete and valid header. There are three
//	possible results:
//
//	1.	We have a complete and valid header. In that case, we return TRUE and 
//		the command, data length, and partial data variables are initialized.
//
//	2.	We don't have a full header yet. In that case, we return FALSE, and
//		retsCommand is empty.
//
//	3.	We have an invalid header. In that case we return FALSE, and retsCommand
//		is the human-readable error.

	{
	//	Initialize return command to NULL so that we can return when we detect
	//	that we need more data.

	*retsCommand = NULL_STR;

	//	Start parsing

	char *pPos = Data.GetPointer();
	char *pPosEnd = pPos + Data.GetLength();

	//	Parse the protocol and version

	CString sVersion;
	if (!ParseHeaderWord(pPos, pPosEnd, &sVersion, &pPos))
		return false;

	//	Parse the command

	CString sCommand;
	if (!ParseHeaderWord(pPos, pPosEnd, &sCommand, &pPos))
		return false;

	//	Parse the data length

	CString sDataLen;
	if (!ParseHeaderWord(pPos, pPosEnd, &sDataLen, &pPos))
		return false;

	//	Must be a number

	bool bFailed;
	DWORD dwDataLen = strToInt(sDataLen, 0, &bFailed);
	if (bFailed)
		{
		*retsCommand = strPattern(ERR_INVALID_DATA_LEN, sDataLen);
		return false;
		}

	//	We must end in CRLF

	while (pPos < pPosEnd && (*pPos == ' ' || *pPos == '\t'))
		pPos++;

	if (pPos == pPosEnd)
		return false;
	
	if (*pPos != '\r')
		{
		*retsCommand = ERR_INVALID_HEADER;
		return false;
		}

	pPos++;
	if (pPos == pPosEnd)
		return false;
	
	if (*pPos != '\n')
		{
		*retsCommand = ERR_INVALID_HEADER;
		return false;
		}

	pPos++;

	//	We now have a valid and complete header and pPos points to the start 
	//	of the data.

	*retsCommand = sCommand;
	*retdwDataLen = dwDataLen;
	*retpPartialData = pPos;
	*retdwPartialDataLen = (int)(pPosEnd - pPos);

	return true;
	}

CDatum CEsperAMP1ConnectionIn::GetProperty (const CString &sProperty) const

//	GetProperty
//
//	Returns properties.

	{
	if (strEquals(sProperty, FIELD_ADDRESS))
		return CDatum(m_sClientAddr);
	else
		return CDatum();
	}

void CEsperAMP1ConnectionIn::OnConnect (void)

//	OnConnect
//
//	Someone connected to us.

	{
	if (m_iState != stateNone)
		{
		ASSERT(false);
		return;
		}

	//	Reset our state

	m_sMachineName = NULL_STR;

	//	When someone connects, start reading

	OpReadRequest(stateReadingHeader);
	}

void CEsperAMP1ConnectionIn::OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred)

//	OnSocketOperationComplete
//
//	Operation complete

	{
	CString sError;
	CEsperStats &Stats = m_Manager.GetStats();

	switch (m_iState)
		{
		case stateReadingHeader:
			{
			IMemoryBlock *pData = GetBuffer();

			//	See if we have the entire header. If we don't then we need to 
			//	keep reading.

			char *pPartialData;
			DWORD dwPartialDataLen;
			if (!GetHeader(*pData, &m_sCommand, &m_dwDataLen, &pPartialData, &dwPartialDataLen))
				{
				//	Error

				if (!m_sCommand.IsEmpty())
					{
					m_Manager.LogTrace(strPattern(ERR_INVALID_MESSAGE, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), m_sCommand));
					m_Manager.DeleteConnection(CDatum(GetID()));
					}

				//	Need more data

				else
					{
					m_Data.SetLength(0);
					m_Data.Seek(0);
					m_Data.Write(*pData);

					OpReadRequest(stateReadingHeaderContinues);
					}
				}

			//	The header is complete. See if we have enough data.

			else if (dwPartialDataLen >= (int)m_dwDataLen)
				{
				CBuffer Buffer(pPartialData, dwPartialDataLen);
				OpSendAMPMessage(m_sCommand, Buffer);
				}

			//	Otherwise, keep reading

			else
				{
				m_Data.SetLength(0);
				m_Data.Seek(0);
				m_Data.Write(pPartialData, dwPartialDataLen);

				OpReadRequest(stateReadingData);
				}

			break;
			}

		case stateReadingHeaderContinues:
			{
			//	Append the new data to what we read before

			m_Data.Write(*GetBuffer());

			//	See if we have the entire header. If we don't then we need to 
			//	keep reading.

			char *pPartialData;
			DWORD dwPartialDataLen;
			if (!GetHeader(m_Data, &m_sCommand, &m_dwDataLen, &pPartialData, &dwPartialDataLen))
				{
				//	Error

				if (!m_sCommand.IsEmpty())
					{
					m_Manager.LogTrace(strPattern(ERR_INVALID_MESSAGE, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), m_sCommand));
					m_Manager.DeleteConnection(CDatum(GetID()));
					}

				//	Need more data

				else
					{
					OpReadRequest(stateReadingHeaderContinues);
					}
				}

			//	The header is complete. See if we have enough data.

			else if (dwPartialDataLen >= (int)m_dwDataLen)
				{
				CBuffer Buffer(pPartialData, dwPartialDataLen);
				OpSendAMPMessage(m_sCommand, Buffer);
				}

			//	Otherwise, keep reading

			else
				{
				CBuffer Data;
				Data.Write(pPartialData, dwPartialDataLen);
				m_Data.TakeHandoff(Data);
				m_Data.Seek(0, true);

				OpReadRequest(stateReadingData);
				}

			break;
			}

		case stateReadingData:
			{
			m_Data.Write(*GetBuffer());

			if (m_Data.GetLength() >= (int)m_dwDataLen)
				{
				m_Data.Seek(0);
				OpSendAMPMessage(m_sCommand, m_Data);
				}
			else
				OpReadRequest(stateReadingData);

			break;
			}

		case stateWriting:
			{
			//	Writing successful, read more

			OpReadRequest(stateReadingHeader);
			break;
			}
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

void CEsperAMP1ConnectionIn::OnSocketOperationFailed (EOperations iOp)

//	OnSocketOperationFailed
//
//	Failure

	{
	switch (m_iState)
		{
		case stateReadingHeader:
		case stateReadingHeaderContinues:
		case stateReadingData:
			m_Manager.LogTrace(strPattern(ERR_READ_OP_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			m_Manager.DeleteConnection(CDatum(GetID()));
			break;

		case stateWriting:
			m_Manager.LogTrace(strPattern(ERR_WRITE_OP_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
			m_Manager.DeleteConnection(CDatum(GetID()));
			break;
		}
	}

void CEsperAMP1ConnectionIn::OpReadRequest (EStates iNewState)

//	OpReadRequest
//
//	Start reading an AMP1 message

	{
	m_iState = iNewState;

	//	Let our subclass handle it.
	
	CString sError;
	if (!IIOCPEntry::BeginRead(&sError))
		{
		m_Manager.LogTrace(strPattern(ERR_READ_START_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), sError));
		m_Manager.DeleteConnection(CDatum(GetID()));
		}
	}

void CEsperAMP1ConnectionIn::OpSendAMPMessage (const CString &sCommand, IMemoryBlock &Data)

//	OpSendAMPMessage
//
//	We have an AMP1 message, so send it to the client.

	{
	//	Deserialize

	CDatum dData;
	if (!CDatum::Deserialize(CDatum::EFormat::AEONScript, Data, &dData))
		dData = CDatum();

	//	If this is an AUTH command, then we need to check the key to see if we can 
	//	authenticate this client.

	bool bAuthFailure = false;
	if (strEquals(sCommand, AMP1_AUTH))
		{
		CString sName = dData.GetElement(FIELD_AUTH_NAME);
		CIPInteger Key = dData.GetElement(FIELD_AUTH_KEY);

		if (m_Manager.AuthenticateMachine(sName, Key))
			{
#ifdef DEBUG_AMP1
			printf("[%x] Connection from (%s) authenticated.", CEsperInterface::ConnectionToFriendlyID(CDatum(GetID())), (LPSTR)sName);
#endif

			m_sMachineName = sName;
			}
		else
			{
			bAuthFailure = true;
			m_Manager.Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_AUTH, sName));
			}
		}

	//	Compose a message for our client

	if (!bAuthFailure)
		{
		CComplexArray *pPayload = new CComplexArray;
		pPayload->Insert(sCommand);
		pPayload->Insert(dData);
		pPayload->Insert(GetID());

		//	If we have a machine name, then it means this connection is authenticated.

		if (!m_sMachineName.IsEmpty())
			pPayload->Insert(m_sMachineName);

		//	Send it

		m_Manager.SendMessageCommand(m_sClientAddr, MSG_ESPER_ON_AMP1, NULL_STR, 0, CDatum(pPayload));
		}

	//	Reply with acknowledgement.

	CString sReply = AMP1_REPLY_OK;
	CString sError;

	m_iState = stateWriting;
	if (!IIOCPEntry::BeginWrite(sReply, &sError))
		{
		m_Manager.LogTrace(strPattern(ERR_WRITE_OP_FAILED, CEsperInterface::ConnectionToFriendlyID(CDatum(GetID()))));
		m_Manager.DeleteConnection(CDatum(GetID()));
		}
	}

bool CEsperAMP1ConnectionIn::ParseHeaderWord (char *pPos, char *pPosEnd, CString *retsWord, char **retpPos)

//	ParseHeaderWord
//
//	Parses a word. We expect the word to terminate in whitespace. If we don't
//	have enough buffer, we return FALSE.

	{
	//	Skip leading whitespace

	while (pPos < pPosEnd && strIsWhitespace(pPos))
		pPos++;

	if (pPos == pPosEnd)
		return false;

	//	Parse the word

	char *pStart = pPos;
	while (pPos < pPosEnd && !strIsWhitespace(pPos))
		pPos++;

	if (pPos == pPosEnd)
		return false;

	*retsWord = CString(pStart, pPos - pStart);
	*retpPos = pPos;

	return true;
	}

bool CEsperAMP1ConnectionIn::SetBusy (void)

//	SetBusy
//
//	We're in use

	{
	//	We don't take direct orders from clients (read/write), so we don't have 
	//	to do anything here.

	return false;
	}

bool CEsperAMP1ConnectionIn::SetProperty (const CString &sProperty, CDatum dValue)

//	SetProperty
//
//	Sets the property

	{
	if (strEquals(sProperty, FIELD_AUTH_NAME))
		m_sMachineName = dValue;

	else
		return false;

	return true;
	}
