//	CHexDecoder.cpp
//
//	CHexDecoder class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CHexDecoder::CHexDecoder (const IByteStream &Input, DWORD dwFlags) :
		m_Stream(const_cast<IByteStream&>(Input))

//	CHexDecoder constructor

	{
	}

int CHexDecoder::GetStreamLength (void)

//	GetStreamLength
//
//	Returns the length of the (decoded) stream

	{
	int iLength = m_Stream.GetStreamLength();
	return iLength / 2;
	}

int CHexDecoder::Read (void *pData, int iLength)

//	Read
//
//	Read the buffer

	{
	BYTE *pOutput = (BYTE *)pData;
	BYTE *pOutputEnd = pOutput + iLength;

	//	Keep reading from the stream

	while (pOutput < pOutputEnd)
		{
		//	Read 2 characters and decode them into 1 bytes

		int iH = CharToValue(m_Stream.ReadChar());
		if (iH == -1)
			throw CException(errFail);

		int iL = CharToValue(m_Stream.ReadChar());
		if (iL == -1)
			throw CException(errFail);

		*pOutput++ = (BYTE)((iH << 4) | iL);
		}

	//	Done

	return iLength;
	}
