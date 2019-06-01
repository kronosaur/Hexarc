//	CBufferedIO.cpp
//
//	CBufferedIO class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CBufferedIO::CBufferedIO (IByteStream &Stream, int iBufferSize) :
		m_Stream(Stream),
		m_iBufferAlloc(iBufferSize),
		m_iBufferStart(0),
		m_iBufferPos(0),
		m_iBufferLen(0),
		m_pBuffer(NULL),
		m_bReadBuffer(false),
        m_bWriteBuffer(false)

//	CBufferedIO constructor

	{
	if (m_iBufferAlloc > 0)
		m_pBuffer = new char [m_iBufferAlloc];
	}

CBufferedIO::~CBufferedIO (void)

//	CBufferedIO destructor

	{
	Flush();
	if (m_pBuffer)
		delete [] m_pBuffer;
	}

void CBufferedIO::Flush (void)

//	Close
//
//	Flush data

	{
	if (m_bWriteBuffer)
		{
		m_Stream.Write(m_pBuffer, m_iBufferLen);
		m_bWriteBuffer = false;
		}

    m_bReadBuffer = false;
	}

int CBufferedIO::Read (void *pData, int iLength)

//	Read
//
//	Reads

	{
	char *pDest = (char *)pData;
	int iDestLeft = iLength;

	while (iDestLeft > 0)
		{
		//	If we have data in our buffer, read data from there.

		if (m_bReadBuffer && m_iBufferPos < m_iBufferLen)
			{
			int iWrite = Min(iDestLeft, m_iBufferLen - m_iBufferPos);
			utlMemCopy(m_pBuffer + m_iBufferPos, pDest, iWrite);

			m_iBufferPos += iWrite;
			pDest += iWrite;
			iDestLeft -= iWrite;
			}

		//	Otherwise, we need to read from the source stream

		else
			{
			Flush();

			//	Read as much as we can.

			m_iBufferStart = m_Stream.GetPos();
            m_iBufferLen = Min(m_iBufferAlloc, m_Stream.GetStreamLength() - m_iBufferStart);
            m_Stream.Read(m_pBuffer, m_iBufferLen);
			m_iBufferPos = 0;

			//	If we don't have more, then we're at the end of the stream.

			if (m_iBufferLen == 0)
				return (int)(pDest - (char *)pData);

			//	Otherwise, continue reading

            m_bReadBuffer = true;
			continue;
			}
		}

	//	Done

	return iLength;
	}

void CBufferedIO::Seek (int iPos, bool bFromEnd)

//	Seek
//
//	Seek to the given position

	{
	Flush();

	//	Seek

	m_Stream.Seek(iPos, bFromEnd);
	}

int CBufferedIO::Write (const void *pData, int iLength)

//	Write
//
//	Writes

	{
	char *pDest = (char *)pData;
	int iDestLeft = iLength;

    if (m_bReadBuffer)
        Flush();
	
	while (iDestLeft > 0)
		{
		//	If we can write to our buffer, do it.

		if (m_iBufferPos < m_iBufferAlloc)
			{
			int iWrite = Min(iDestLeft, m_iBufferAlloc - m_iBufferPos);
			utlMemCopy(pDest, m_pBuffer + m_iBufferPos, iWrite);

			m_iBufferPos += iWrite;
			pDest += iWrite;
			iDestLeft -= iWrite;
			m_iBufferLen = Max(m_iBufferPos, m_iBufferLen);
			m_bWriteBuffer = true;
			}

		//	Otherwise, write out the buffer to the stream

		else
			{
			int iWritten = m_Stream.Write(m_pBuffer, m_iBufferAlloc);
			if (iWritten != m_iBufferAlloc)
				return (int)(pDest - (char *)pData);

			m_iBufferPos = 0;
			m_iBufferLen = 0;
            m_bWriteBuffer = false;
			continue;
			}
		}

	//	Done

	return iLength;
	}
