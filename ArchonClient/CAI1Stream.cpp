//	CAI1Stream.cpp
//
//	CAI1Stream class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_EOM_TOKEN,						"|")
DECLARE_CONST_STRING(STR_HEADER_AI1_00,					"AI/1.00")

DECLARE_CONST_STRING(AI1_INVALID_MESSAGE,				"invalidMessage")

const int MAX_MESSAGE_SIZE =							1024 * 1024;

void CAI1Stream::GetNext (CString *retsCommand, CDatum *retdPayload)

//	GetNext
//
//	Returns the next message

	{
	*retsCommand = m_sCachedCommand;
	*retdPayload = m_dCachedPayload;

	m_sCachedCommand = NULL_STR;
	m_dCachedPayload = CDatum();
	}

bool CAI1Stream::HasMore (bool *retbOverflow)

//	HasMore
//
//	Returns TRUE if there are more messages

	{
	if (retbOverflow)
		*retbOverflow = false;

	//	If we have a message cached then we clearly have more

	if (!m_sCachedCommand.IsEmpty())
		return true;

	//	Otherwise, see if we have a full message

	int iPos = m_Buffer.Scan(STR_EOM_TOKEN);
	if (iPos == -1)
		{
		//	If we exceeded our max size in the buffer without finding an EOM, then 
		//	it probably means that somebody is trying to send us garbage. We discard
		//	everything.

		int iLength = m_Buffer.GetStreamLength();
		if (iLength > MAX_MESSAGE_SIZE)
			{
			if (retbOverflow)
				*retbOverflow = true;

			m_Buffer.Read(NULL, iLength);
			}

		return false;
		}

	//	Since we bothered to scan the buffer, just cache the message (so we
	//	don't have to scan again).

	ParseMessage(iPos + STR_EOM_TOKEN.GetLength(), &m_sCachedCommand, &m_dCachedPayload);
	return true;
	}

void CAI1Stream::ParseMessage (int iLength, CString *retsCommand, CDatum *retdPayload)

//	ReadMessage
//
//	Reads a message from the buffer

	{
	char *pLine = new char [iLength];
	m_Buffer.Read(pLine, iLength);

	char *pPos = pLine;
	char *pEndPos = pPos + iLength - STR_EOM_TOKEN.GetLength();

	//	Look for the header

	char *pStart = pPos;
	while (pPos < pEndPos && *pPos != ' ')
		pPos++;

	CString sHeader(pStart, pPos - pStart);
	if (!strEquals(sHeader, STR_HEADER_AI1_00) || pPos == pEndPos)
		{
		*retsCommand = AI1_INVALID_MESSAGE;
		*retdPayload = CDatum();
		return;
		}

	pPos++;

	//	Look for the command

	pStart = pPos;
	while (pPos < pEndPos && *pPos != ' ')
		pPos++;

	CString sCommand(pStart, pPos - pStart);
	if (sCommand.IsEmpty())
		{
		*retsCommand = AI1_INVALID_MESSAGE;
		*retdPayload = CDatum();
		return;
		}

	if (pPos < pEndPos)
		pPos++;

	//	Look for the payload

	CBuffer Payload(pPos, (int)(pEndPos - pPos), false);

	//	The remaining datums are the payload

	CComplexArray *pArray = new CComplexArray;
	CDatum dItem;
	while (Payload.HasMore() && CDatum::Deserialize(CDatum::formatAEONScript, Payload, &dItem))
		pArray->Insert(dItem);

	//	Done

	*retsCommand = sCommand;
	*retdPayload = CDatum(pArray);
	}

void CAI1Stream::ReadFromSocket (CSocket &theSocket)

//	ReadFromSocket
//
//	Reads from the socket until there is at least one message

	{
	while (!HasMore())
		{
		char szBuffer[4096];
		int iLength = theSocket.Read(szBuffer, sizeof(szBuffer));
		if (iLength == 0)
			return;

		m_Buffer.Write(szBuffer, iLength);
		}
	}

void CAI1Stream::WriteToEsper (const CString &sCommand, CDatum dPayload, CDatum *retdData)

//	WriteToEsper
//
//	Writes a message to a data object suitable for Esper.write

	{
	int i;
	CStringBuffer Output;

	Output.Write(STR_HEADER_AI1_00);
	Output.Write(" ", 1);
	Output.Write(sCommand);

	for (i = 0; i < dPayload.GetCount(); i++)
		{
		Output.Write(" ", 1);
		dPayload.GetElement(i).Serialize(CDatum::formatAEONScript, Output);
		}

	Output.Write(STR_EOM_TOKEN);

	//	Done

	CDatum::CreateStringFromHandoff(Output, retdData);
	}
