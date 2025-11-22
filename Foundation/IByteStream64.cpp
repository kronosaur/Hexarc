//	IByteStream64.cpp
//
//	IByteStream64 class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const DWORD BUFFER_SIZE =					65536;

char IByteStream64::ReadChar (void)

//	ReadChar
//
//	Reads a single character

	{
	char chChar;
	DWORDLONG dwRead = ReadTry(&chChar, 1);
	if (dwRead != 1)
		return '\0';

	return chChar;
	}

void IByteStream64::Write (IMemoryBlock &Block)

//	Write

	{
	Write(Block.GetPointer(), Block.GetLength());
	}

void IByteStream64::Write (IByteStream &Stream, int iLength)

//	Write

	{
	char *pBuffer = new char [BUFFER_SIZE];

	try
		{
		DWORD dwLeftToWrite = Max(0, iLength);

		while (dwLeftToWrite > 0)
			{
			DWORD dwWrite = Min(BUFFER_SIZE, dwLeftToWrite);

			Stream.Read(pBuffer, dwWrite);
			Write(pBuffer, dwWrite);

			dwLeftToWrite -= dwWrite;
			}
		}
	catch (...)
		{
		delete [] pBuffer;
		throw;
		}

	//	Done

	delete [] pBuffer;
	}

void IByteStream64::Write (IByteStream64 &Stream, DWORDLONG dwLength)

//	Write

	{
	char *pBuffer = new char [BUFFER_SIZE];

	try
		{
		DWORDLONG dwLeftToWrite = dwLength;

		while (dwLeftToWrite > 0)
			{
			DWORDLONG dwWrite = Min((DWORDLONG)BUFFER_SIZE, dwLeftToWrite);

			Stream.Read(pBuffer, dwWrite);
			Write(pBuffer, dwWrite);

			dwLeftToWrite -= dwWrite;
			}
		}
	catch (...)
		{
		delete [] pBuffer;
		throw;
		}

	//	Done

	delete [] pBuffer;
	}

void IByteStream64::WriteChar (char chChar, int iCount)

//	WriteChar
//
//	Writes a series of repeating characters

	{
	switch (iCount)
		{
		case 0:
			break;

		case 1:
			Write(&chChar, 1);
			break;

		default:
			{
			if (iCount <= 0)
				return;
			else if (iCount < 256)
				{
				char szBuffer[256];
				char *pPos = szBuffer;
				char *pPosEnd = pPos + iCount;
				while (pPos < pPosEnd)
					*pPos++ = chChar;

				Write(szBuffer, iCount);
				}
			else
				{
				char *pBuffer = new char [iCount];

				char *pPos = pBuffer;
				char *pPosEnd = pPos + iCount;
				while (pPos < pPosEnd)
					*pPos++ = chChar;

				try
					{
					Write(pBuffer, iCount);
					}
				catch (...)
					{
					delete [] pBuffer;
					throw;
					}

				delete [] pBuffer;
				}
			}
		}
	}

void IByteStream64::WriteStreamDefault (IByteStream &Stream, int iLength)

//	WriteStreamDefault
//
//	Writes from the given stream.

	{
	char *pBuffer = new char [BUFFER_SIZE];

	try
		{
		int iLeftToWrite = iLength;

		while (iLeftToWrite > 0)
			{
			int iWrite = Min((int)BUFFER_SIZE, iLeftToWrite);

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
	}

void IByteStream64::WriteWithProgress (IByteStream &Stream, int iLength, IProgressEvents *pProgress)

//  WriteWithProgress
//
//  Writes the given stream to our stream and calls out progress updates.

    {
    if (iLength <= 0)
        return;

    if (pProgress)
        pProgress->OnProgressStart();

    int iBufferSize = Min(iLength, Max((int)BUFFER_SIZE, iLength / 1000));
	char *pBuffer = new char [iBufferSize];

	try
		{
		int iLeftToWrite = iLength;

		while (iLeftToWrite > 0)
			{
			int iWrite = Min((int)BUFFER_SIZE, iLeftToWrite);

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
    }
