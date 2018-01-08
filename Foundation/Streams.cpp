//	Streams.cpp
//
//	Stream functions
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int BUFFER_SIZE =									65536;

CNullStream NULL_STREAM;

//	IByteStream ----------------------------------------------------------------

char IByteStream::ReadChar (void)

//	ReadChar
//
//	Reads a single character

	{
	char chChar;
	int iRead = Read(&chChar, 1);
	return (iRead > 0 ? chChar : '\0');
	}

int IByteStream::Write (IByteStream &Stream, int iLength)

//	Write
//
//	Writes from the given stream.

	{
	char *pBuffer = new char [BUFFER_SIZE];

	try
		{
		int iLeftToWrite = iLength;

		while (iLeftToWrite > 0)
			{
			int iWrite = Min(BUFFER_SIZE, iLeftToWrite);

			Stream.Read(pBuffer, iWrite);
			Write(pBuffer, iWrite);

			iLeftToWrite -= iWrite;
			}
		}
	catch (...)
		{
		delete [] pBuffer;
		throw;
		}

	//	Done

	delete [] pBuffer;
	return iLength;
	}

int IByteStream::WriteChar (char chChar, int iCount)

//	WriteChar
//
//	Writes a series of repeating characters

	{
	switch (iCount)
		{
		case 0:
			return 0;

		case 1:
			return Write(&chChar, 1);

		default:
			{
			if (iCount <= 0)
				return 0;
			else if (iCount < 256)
				{
				char szBuffer[256];
				char *pPos = szBuffer;
				char *pPosEnd = pPos + iCount;
				while (pPos < pPosEnd)
					*pPos++ = chChar;
				return Write(szBuffer, iCount);
				}
			else
				{
				char *pBuffer = new char [iCount];
				char *pPos = pBuffer;
				char *pPosEnd = pPos + iCount;
				while (pPos < pPosEnd)
					*pPos++ = chChar;
				int iLength = Write(pBuffer, iCount);
				delete [] pBuffer;
				return iLength;
				}
			}
		}
	}

int IByteStream::WriteWithProgress (IByteStream &Stream, int iLength, IProgressEvents *pProgress)

//  WriteWithProgress
//
//  Writes the given stream to our stream and calls out progress updates.

    {
    if (iLength <= 0)
        return 0;

    if (pProgress)
        pProgress->OnProgressStart();

    int iBufferSize = Min(iLength, Max(BUFFER_SIZE, iLength / 1000));
	char *pBuffer = new char [iBufferSize];

	try
		{
		int iLeftToWrite = iLength;

		while (iLeftToWrite > 0)
			{
			int iWrite = Min(BUFFER_SIZE, iLeftToWrite);

			Stream.Read(pBuffer, iWrite);
			Write(pBuffer, iWrite);

			iLeftToWrite -= iWrite;

            //  Progress

            if (pProgress)
                pProgress->OnProgress(100 * (iLength - iLeftToWrite) / iLength);
			}
		}
	catch (...)
		{
		delete [] pBuffer;
		throw;
		}

    if (pProgress)
        pProgress->OnProgressDone();

	//	Done

	delete [] pBuffer;
	return iLength;
    }
