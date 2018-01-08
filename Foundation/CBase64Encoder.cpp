//	CBase64Encoder.cpp
//
//	CBase64Encoder class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

BYTE g_Table64[64] =
	{	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/' };

CBase64Encoder::CBase64Encoder (IByteStream *pOutput, DWORD dwFlags) : 
		m_pStream(pOutput),
		m_iBufferLen(0)

//	CBase64Encoder constructor

	{
	}

void CBase64Encoder::Close (void)

//	Close
//
//	Closes the stream. Note that the filter will not write out the last few
//	bytes until Close is called.

	{
	//	If we've got data in the buffer then we need to write it out

	if (m_iBufferLen == 1)
		{
		BYTE pOutput[4];

		pOutput[0] = g_Table64[m_chBuffer[0] >> 2];
		pOutput[1] = g_Table64[((m_chBuffer[0] & 0x03) << 4)];
		pOutput[2] = '=';
		pOutput[3] = '=';

		m_pStream->Write(pOutput, 4);

		m_iBufferLen = 0;
		}
	else if (m_iBufferLen == 2)
		{
		BYTE pOutput[4];

		pOutput[0] = g_Table64[m_chBuffer[0] >> 2];
		pOutput[1] = g_Table64[((m_chBuffer[0] & 0x03) << 4) | (m_chBuffer[1] >> 4)];
		pOutput[2] = g_Table64[((m_chBuffer[1] & 0x0f) << 2)];
		pOutput[3] = '=';

		m_pStream->Write(pOutput, 4);

		m_iBufferLen = 0;
		}
	}

int CBase64Encoder::Write (void *pData, int iLength)

//	Write
//
//	Writes out binary data and encodes it into base64.

	{
	BYTE *pInput = (BYTE *)pData;
	BYTE *pInputEnd = pInput + iLength;

	//	Edge-cases

	if (pInput == pInputEnd)
		return 0;

	//	If we've got some data in the buffer, add to it until we have a complete
	//	triplet.

	if (m_iBufferLen > 0)
		{
		if (m_iBufferLen == 1)
			{
			m_chBuffer[1] = (pInput == NULL ? 0 : *pInput++);
			m_iBufferLen++;
			if (pInput == pInputEnd)
				return 1;

			m_chBuffer[2] = (pInput == NULL ? 0 : *pInput++);
			m_iBufferLen++;
			}
		else if (m_iBufferLen == 2)
			{
			m_chBuffer[2] = (pInput == NULL ? 0 : *pInput++);
			m_iBufferLen++;
			}

		//	We have a full buffer, so write it out

		int iWritten = WriteTriplet(m_chBuffer);
		m_iBufferLen = 0;

		if (iWritten != 3)
			return iWritten;
		}

	//	Write out the rest of the data in triplets

	while (pInput + 3 <= pInputEnd)
		{
		if (WriteTriplet(pInput) != 3)
			return (int)(pInput - (BYTE *)pData);

		pInput += 3;
		}

	//	Add the remainder to the buffer

	while (pInput < pInputEnd)
		m_chBuffer[m_iBufferLen++] = *pInput++;

	//	Done

	return iLength;
	}

int CBase64Encoder::WriteTriplet (void *pData)

//	WriteTripler
//
//	Writes 3 bytes encoded in base64 to 4 bytes.

	{
	BYTE *pInput = (BYTE *)pData;
	BYTE pOutput[4];

	pOutput[0] = g_Table64[pInput[0] >> 2];
	pOutput[1] = g_Table64[((pInput[0] & 0x03) << 4) | (pInput[1] >> 4)];
	pOutput[2] = g_Table64[((pInput[1] & 0x0f) << 2) | (pInput[2] >> 6)];
	pOutput[3] = g_Table64[pInput[2] & 0x3f];

	int iWritten = m_pStream->Write(pOutput, 4);
	return (iWritten == 4 ? 3 : 0);
	}
