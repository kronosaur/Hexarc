//	CWebSocketProtocol.cpp
//
//	CWebSocketProtocol class
//	Copyright (c) 2024 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(HEADER_CONNECTION,					"Connection");
DECLARE_CONST_STRING(HEADER_DATE,						"Date");
DECLARE_CONST_STRING(HEADER_SERVER,						"Server");
DECLARE_CONST_STRING(HEADER_SEC_WEBSOCKET_KEY,			"Sec-WebSocket-Key");
DECLARE_CONST_STRING(HEADER_SEC_WEBSOCKET_ACCEPT,		"Sec-WebSocket-Accept");
DECLARE_CONST_STRING(HEADER_UPGRADE,					"Upgrade");

DECLARE_CONST_STRING(STR_SWITCHING_PROTOCOLS,			"Switching Protocols");
DECLARE_CONST_STRING(STR_UPGRADE,						"Upgrade");
DECLARE_CONST_STRING(STR_WEBSOCKET,						"websocket");

DECLARE_CONST_STRING(WEB_SOCKET_GUID,					"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

DECLARE_CONST_STRING(ERR_INVALID_CONTINUATION,			"Parsed continuation frame without initial frame.");

void CWebSocketProtocol::AppendFrame (bool bFinal, EOpCode iOpCode, CString sData)

//	AppendFrame
//
//	Appends a frame to the list of messages.

	{
	//	If this is a ping or pong message, we need to insert it BEFORE any 
	//	partial frame.

	if (iOpCode == EOpCode::Ping || iOpCode == EOpCode::Pong || iOpCode == EOpCode::Close)
		{
		//	Look backwards for the last non-final frame

		int iPos = m_Messages.GetCount();
		while (iPos > 0 && !m_Messages[iPos - 1].bComplete)
			iPos--;

		SMessage* pFrame = m_Messages.InsertAt(iPos);
		pFrame->bComplete = true;
		pFrame->iOpCode = iOpCode;
		pFrame->sData = std::move(sData);
		}
	else if (iOpCode == EOpCode::Continuation)
		{
		//	The last frame must be partial

		if (m_Messages.GetCount() == 0 || m_Messages[m_Messages.GetCount() - 1].bComplete)
			{
			SMessage* pFrame = m_Messages.Insert();
			pFrame->bComplete = true;
			pFrame->iOpCode = EOpCode::Error;
			pFrame->sData = ERR_INVALID_CONTINUATION;
			return;
			}

		SMessage* pFrame = &m_Messages[m_Messages.GetCount() - 1];
		pFrame->bComplete = bFinal;
		pFrame->sData += sData;
		}
	else if (iOpCode != EOpCode::Invalid)
		{
		SMessage* pFrame = m_Messages.Insert();
		pFrame->bComplete = bFinal;
		pFrame->iOpCode = iOpCode;
		pFrame->sData = std::move(sData);
		}
	}

CHTTPMessage CWebSocketProtocol::ComposeAcceptResponse (CStringView sKey, CStringView sServerHeader)

//	ComposeAcceptResponse
//
//	Composes a message responding to a WebSocket upgrade request.

	{
	//	Compute the WebSocket accept key

	CString sAcceptKey = CString(sKey);
	sAcceptKey += WEB_SOCKET_GUID;

	CBuffer AcceptKeyData(sAcceptKey);
	CIPInteger AcceptKey;
	cryptoCreateDigest(AcceptKeyData, &AcceptKey);

	//	We have a WebSocket request. Send a response

	CHTTPMessage Response;
	Response.InitResponse(http_SWITCHING_PROTOCOLS, STR_SWITCHING_PROTOCOLS);
	if (!sServerHeader.IsEmpty())
		Response.AddHeader(HEADER_SERVER, sServerHeader);
	Response.AddHeader(HEADER_DATE, CDateTime(CDateTime::Now));
	Response.AddHeader(HEADER_UPGRADE, STR_WEBSOCKET);
	Response.AddHeader(HEADER_CONNECTION, STR_UPGRADE);
	Response.AddHeader(HEADER_SEC_WEBSOCKET_ACCEPT, AcceptKey.AsBase64());

	return Response;
	}

CString CWebSocketProtocol::EncodeFrame (bool bFinal, EOpCode iOpCode, CStringView sData)

//	EncodeFrame
//
//	Encodes a single frame.

	{
	CStringBuffer Output;

	//	First byte is the header

	BYTE bHeader = 0;
	if (bFinal)
		bHeader |= 0x80;

	bHeader |= (BYTE)iOpCode;
	Output.WriteChar((char)bHeader);

	//	Second byte is the mask and payload length
	//	NOTE: No mask because this is a server frame. Use EncodeFrameMasked for
	//	client frames.

	if (sData.GetLength() < 126)
		{
		Output.WriteChar((char)(BYTE)sData.GetLength());
		}
	else if (sData.GetLength() < 0x10000)
		{
		Output.WriteChar(126);
		Output.WriteChar((char)(BYTE)(sData.GetLength() / 256));
		Output.WriteChar((char)(BYTE)(sData.GetLength() % 256));
		}
	else
		{
		Output.WriteChar(127);
		Output.WriteChar(0);
		Output.WriteChar(0);
		Output.WriteChar(0);
		Output.WriteChar(0);
		Output.WriteChar((char)(BYTE)(sData.GetLength() / 0x1000000));
		Output.WriteChar((char)(BYTE)(sData.GetLength() / 0x10000));
		Output.WriteChar((char)(BYTE)(sData.GetLength() / 0x100));
		Output.WriteChar((char)(BYTE)(sData.GetLength() % 0x100));
		}

	//	Payload

	Output.Write(sData);

	return CString(std::move(Output));
	}

CWebSocketProtocol::EOpCode CWebSocketProtocol::GetNextFrame (CString& retsData)

//	GetNextFrame
//
//	Returns the next valid frame.

	{
	if (m_Messages.GetCount() == 0 || !m_Messages[0].bComplete)
		return EOpCode::Invalid;
	else
		{
		retsData = std::move(m_Messages[0].sData);
		EOpCode iOpCode = m_Messages[0].iOpCode;

		m_Messages.Delete(0);

		return iOpCode;
		}
	}

void CWebSocketProtocol::OnRead (const IMemoryBlock& Buffer)

//	OnRead
//
//	We've read some data, so parse as many frames as we can and leave any
//	remaining data in the buffer.

	{
	const BYTE *pPos = NULL;
	const BYTE* pEnd = NULL;

	if (m_Buffer.GetLength() > 0)
		{
		m_Buffer.Seek(0, true);
		m_Buffer.Write(Buffer);
		pPos = (BYTE*)m_Buffer.GetPointer();
		pEnd = pPos + m_Buffer.GetLength();
		}
	else
		{
		pPos = (BYTE*)Buffer.GetPointer();
		pEnd = pPos + Buffer.GetLength();
		}

	//	Parse as many frames as we can

	while (true)
		{
		int iBytesParsed = ParseFrame(pPos, pEnd);
		if (iBytesParsed == 0)
			break;

		pPos += iBytesParsed;
		}

	//	Any extra data, we keep in the buffer

	if (pPos < pEnd)
		{
		CBuffer Remainder(pPos, (pEnd - pPos));
		m_Buffer = std::move(Remainder);
		}
	else
		m_Buffer = CBuffer();
	}

int CWebSocketProtocol::ParseFrame (const BYTE* pPos, const BYTE* pPosEnd)

//	ParseFrame
//
//	Parses a frame. If we have a complete frame, then we add it to m_Messages
//	and return the number of bytes we consumed. Otherwise, we return 0 and wait
//	for the next call.

	{
	//	If we don't have enough data to read the header, then we're done.

	if (pPosEnd - pPos < 2)
		return 0;

	//	First byte is the header

	const BYTE* pStart = pPos;
	BYTE bHeader = *pPos;
	bool bFinal = ((bHeader & 0x80) ? true : false);
	BYTE iOpCode = (bHeader & 0x0F);

	//	Second byte is the mask and payload length

	BYTE bMaskAndLength = *(pPos + 1);
	bool bMasked = ((bMaskAndLength & 0x80) ? true : false);
	DWORD dwPayloadLength = (bMaskAndLength & 0x7F);

	//	We need to read more bytes to get the payload length

	int iBytesToRead = 0;
	if (dwPayloadLength == 126)
		iBytesToRead = 2;
	else if (dwPayloadLength == 127)
		iBytesToRead = 8;

	if (pPosEnd - pPos < 2 + iBytesToRead)
		return 0;

	//	Now we parse the payload length

	if (dwPayloadLength == 126)
		{
		dwPayloadLength = (DWORD)pPos[2] * 256 + (DWORD)pPos[3];
		pPos += 2 + iBytesToRead;
		}
	else if (dwPayloadLength == 127)
		{
		DWORDLONG dwPayloadLength64 = (DWORDLONG)pPos[2] * 0x100000000000000 + (DWORDLONG)pPos[3] * 0x1000000000000 + (DWORDLONG)pPos[4] * 0x10000000000 + (DWORDLONG)pPos[5] * 0x100000000 + (DWORDLONG)pPos[6] * 0x1000000 + (DWORDLONG)pPos[7] * 0x10000 + (DWORDLONG)pPos[8] * 0x100 + (DWORDLONG)pPos[9];
		if (dwPayloadLength64 > 0x80000000)
			//	Not supported for now--break up into multiple frames
			throw CException(errFail);

		dwPayloadLength = (DWORD)dwPayloadLength64;
		pPos += 2 + iBytesToRead;
		}
	else
		pPos += 2;

	//	Now we need to read the mask

	BYTE Mask[4];
	if (bMasked)
		{
		if (pPosEnd - pPos < 4)
			return 0;

		Mask[0] = pPos[0];
		Mask[1] = pPos[1];
		Mask[2] = pPos[2];
		Mask[3] = pPos[3];
		pPos += 4;
		}

	//	Now we need to read the payload

	if (pPosEnd - pPos < (int)dwPayloadLength)
		return 0;

	//	Unmask the payload

	CStringBuffer Payload;
	if (bMasked)
		{
		for (int i = 0; i < (int)dwPayloadLength; i++)
			Payload.WriteChar(pPos[i] ^ Mask[i % 4]);
		}
	else
		{
		Payload.Write(pPos, dwPayloadLength);
		}

	//	Add the frame to the message

	AppendFrame(bFinal, (EOpCode)iOpCode, CString(std::move(Payload)));

	//	Done

	return (int)(pPos - pStart) + (int)dwPayloadLength;
	}
