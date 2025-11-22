//	CAMP1Protocol.cpp
//
//	CAMP1Protocol class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(AMP1_AUTH0,						"AUTH0");

DECLARE_CONST_STRING(PROTOCOL_AMP1,						"AMP/1.00");

DECLARE_CONST_STRING(ERR_INVALID_DATA_LEN,				"Invalid data length: %s.");
DECLARE_CONST_STRING(ERR_INVALID_HEADER,				"Invalid header.");
DECLARE_CONST_STRING(ERR_INVALID_PROTOCOL,				"Not AMP1 Protocol.");

bool CAMP1Protocol::GetHeader (const IMemoryBlock& Data, CString* retsCommand, DWORD* retdwDataLen, const char** retpPartialData, DWORD* retdwPartialDataLen)

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

	if (retsCommand) *retsCommand = NULL_STR;

	//	Start parsing

	const char *pPos = Data.GetPointer();
	const char *pPosEnd = pPos + Data.GetLength();

	//	Parse the protocol and version

	CString sVersion;
	if (!ParseHeaderWord(pPos, pPosEnd, &sVersion, &pPos))
		return false;	//	Need more buffer

	if (!strEquals(sVersion, PROTOCOL_AMP1))
		{
		if (retsCommand) *retsCommand = ERR_INVALID_PROTOCOL;
		return false;
		}

	//	Parse the command

	CString sCommand;
	if (!ParseHeaderWord(pPos, pPosEnd, &sCommand, &pPos))
		return false;	//	Need more buffer

	//	If not the end of the message, then we expect a data length

	DWORD dwDataLen;
	if (*pPos != '\r')
		{
		//	Parse the data length

		CString sDataLen;
		if (!ParseHeaderWord(pPos, pPosEnd, &sDataLen, &pPos))
			return false;	//	Need more buffer

		//	Must be a number

		bool bFailed;
		dwDataLen = strToInt(sDataLen, 0, &bFailed);
		if (bFailed)
			{
			if (retsCommand) *retsCommand = strPattern(ERR_INVALID_DATA_LEN, sDataLen);
			return false;
			}

		//	We must end in CRLF

		while (pPos < pPosEnd && (*pPos == ' ' || *pPos == '\t'))
			pPos++;

		if (pPos == pPosEnd)
			return false;	//	Need more buffer
		}
	else
		{
		dwDataLen = 0;
		}
	
	if (*pPos != '\r')
		{
		if (retsCommand) *retsCommand = ERR_INVALID_HEADER;
		return false;
		}

	pPos++;
	if (pPos == pPosEnd)
		return false;	//	Need more buffer
	
	if (*pPos != '\n')
		{
		if (retsCommand) *retsCommand = ERR_INVALID_HEADER;
		return false;
		}

	pPos++;

	//	We now have a valid and complete header and pPos points to the start 
	//	of the data.

	if (retsCommand) *retsCommand = sCommand;
	if (retdwDataLen) *retdwDataLen = dwDataLen;
	if (retpPartialData) *retpPartialData = pPos;
	if (retdwPartialDataLen) *retdwPartialDataLen = (int)(pPosEnd - pPos);

	return true;
	}

CString CAMP1Protocol::MakeAUTH0Message (CStringView sMachineName, const CIPInteger& SecretKey)

//	MakeAUTH0Message
//
//	Makes an AUTH0 message

	{
	CStringBuffer Msg;
	Msg.Write(sMachineName);
	Msg.WriteChar(' ');
	Msg.Write(SecretKey.AsString());

	return MakeMessage(AMP1_AUTH0, Msg);
	}

CString CAMP1Protocol::MakeMessage (CStringView sMsg, const IMemoryBlock& Data)

//	MakeMessage
//
//	Makes a complete AMP1 message

	{
	CStringBuffer Msg;

	Msg.Write(PROTOCOL_AMP1);
	Msg.WriteChar(' ');
	Msg.Write(sMsg);

	if (Data.GetLength() > 0)
		{
		Msg.WriteChar(' ');
		Msg.Write(strFromInt(Data.GetLength()));
		Msg.WriteChar('\r');
		Msg.WriteChar('\n');
		Msg.Write(Data);
		}
	else
		{
		Msg.WriteChar('\r');
		Msg.WriteChar('\n');
		}

	return CString::CreateFromHandoff(Msg);
	}

bool CAMP1Protocol::ParseHeaderWord (const char* pPos, const char* pPosEnd, CString* retsWord, const char** retpPos)

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

	const char *pStart = pPos;
	while (pPos < pPosEnd && !strIsWhitespace(pPos))
		pPos++;

	if (pPos == pPosEnd)
		return false;

	if (retsWord) *retsWord = CString(pStart, pPos - pStart);
	if (retpPos) *retpPos = pPos;

	return true;
	}



