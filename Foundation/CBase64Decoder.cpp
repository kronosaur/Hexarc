//	CBase64Decoder.cpp
//
//	CBase64Decoder class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CBase64Decoder::CBase64Decoder (IByteStream *pInput, DWORD dwFlags) :
		m_pStream(pInput),
		m_iBufferPos(3)

//	CBase64Decoder constructor

	{
	}

BYTE CBase64Decoder::CharToByte (char chChar)

//	CharToByte
//
//	Converts from a character to a base64 number

	{
	if (chChar >= 'A' && chChar <= 'Z')
		return (chChar - 'A');
	else if (chChar >= 'a' && chChar <= 'z')
		return 26 + (chChar - 'a');
	else if (chChar >= '0' && chChar <= '9')
		return 52 + (chChar - '0');
	else if (chChar == '+')
		return 62;
	else if (chChar == '/')
		return 63;

	//	Base64url

	else if (chChar == '-')
		return 62;
	else if (chChar == '_')
		return 63;

	else
		return 0xff;
	}

int CBase64Decoder::GetStreamLength (void)

//	GetStreamLength
//
//	Returns the length of the (decoded) stream

	{
	//	To calculate the length of the decoded stream we need to exclude any 
	//	padding bytes at the end.

	int iPos = m_pStream->GetPos();

	int iLength = m_pStream->GetStreamLength();
	m_pStream->Seek(2, true);
	if (m_pStream->ReadChar() == '=') iLength--;
	if (m_pStream->ReadChar() == '=') iLength--;
	m_pStream->Seek(iPos);

	return (iLength * 3) / 4;
	}

int CBase64Decoder::Read (void *pData, int iLength)

//	Read
//
//	Read the buffer

	{
	BYTE *pOutput = (BYTE *)pData;
	BYTE *pOutputEnd = pOutput + iLength;

	//	Keep reading from the stream

	while (pOutput < pOutputEnd)
		{
		//	Read 4 characters and decode them into 3 bytes

		if (m_iBufferPos == 3)
			{
			char pInputChar[4];
			int iRead = m_pStream->Read(pInputChar, 4);
			if (iRead != 4)
				return (int)(pOutput - (BYTE *)pData);

			BYTE pInput[4];
			pInput[0] = CharToByte(pInputChar[0]);
			pInput[1] = CharToByte(pInputChar[1]);
			pInput[2] = CharToByte(pInputChar[2]);
			pInput[3] = CharToByte(pInputChar[3]);

			//	Convert to 3 bytes of binary

			m_chBuffer[0] = (pInput[0] << 2) | (pInput[1] >> 4);
			m_chBuffer[1] = (pInput[1] << 4) | (pInput[2] >> 2);
			m_chBuffer[2] = (pInput[2] << 6) | pInput[3];

			m_iBufferPos = 0;
			}

		//	Read from buffer

		*pOutput++ = m_chBuffer[m_iBufferPos++];
		}

	//	Done

	return iLength;
	}
