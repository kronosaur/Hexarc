//	Compression.cpp
//
//	Compression functions and classes
//	Copyright (c) 2013 by Kronosaur Production, LLC. All Rights Reserved.

#include "stdafx.h"

#define ZLIB_WINAPI
#include "..\zlib-1.2.7\zlib.h"

const DWORD BUFFER_SIZE =					1024 * 1024;

void Deflate (IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer);
void NullCopy (IMemoryBlock &Data, IMemoryBlock *retBuffer);
void NullCopy (IMemoryBlock &Data, IByteStream &Result);
void NullCopy (IByteStream &Data, IByteStream &Result);

void compCompress (IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer)

//	compCompress
//
//	Compresses Data into Result using iType compression mechanism.

	{
	switch (iType)
		{
		case compressionNone:
			NullCopy(Data, retBuffer);
			break;

		case compressionGzip:
		case compressionZlib:
			Deflate(Data, iType, retBuffer);
			break;

		default:
			throw CException(errFail);
		}
	}

void Deflate (IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer)
	{
	//	Initialize library

	z_stream zcpr;
	utlMemSet(&zcpr, sizeof(zcpr));

	if (iType == compressionZlib)
		deflateInit(&zcpr, Z_DEFAULT_COMPRESSION);
	else if (iType == compressionGzip)
		deflateInit2(&zcpr,
				Z_DEFAULT_COMPRESSION,
				Z_DEFLATED,
				0x1f,		//	Bit 0x10 means gzip header
				8,
				Z_DEFAULT_STRATEGY);
	else
		{
		ASSERT(false);
		return;
		}

	//	Compress

	DWORD dwSourceLeft = Data.GetLength();
	BYTE *pSource = (BYTE *)Data.GetPointer();

	DWORD dwDestBufferSize = Min(dwSourceLeft, BUFFER_SIZE);
	retBuffer->SetLength(dwDestBufferSize);
	BYTE *pDest = (BYTE *)retBuffer->GetPointer();

	//	The entire source is available

	zcpr.next_in = pSource;
	zcpr.avail_in = dwSourceLeft;

	int ret;
	while (true)
		{
		zcpr.next_out = pDest;
		zcpr.avail_out = dwDestBufferSize;

		ret = deflate(&zcpr, Z_FINISH);

		//	If we need more output buffer, allocate more.

		if (ret == Z_OK)
			{
			ASSERT(zcpr.avail_out == 0);

			DWORD dwCurrent = retBuffer->GetLength();
			dwDestBufferSize = BUFFER_SIZE;
			retBuffer->SetLength(dwCurrent + dwDestBufferSize);
			pDest = (BYTE *)(retBuffer->GetPointer() + dwCurrent);

			continue;
			}

		//	If we finished compression, then we're done

		else if (ret == Z_STREAM_END)
			{
			retBuffer->SetLength(zcpr.total_out);
			break;
			}

		//	Otherwise, this is an error

		else
			throw CException(errFail);
		}

	//	Done

	deflateEnd(&zcpr);
	}

void NullCopy (IMemoryBlock &Data, IMemoryBlock *retBuffer)
	{
	retBuffer->SetLength(Data.GetLength());
	utlMemCopy(Data.GetPointer(), retBuffer->GetPointer(), Data.GetLength());
	}

void NullCopy (IMemoryBlock &Data, IByteStream &Result)
	{
	Result.Write(Data.GetPointer(), Data.GetLength());
	}

void NullCopy (IByteStream &Data, IByteStream &Result)
	{
	DWORD dwBytesLeft = Data.GetStreamLength();
	if (dwBytesLeft == 0)
		return;

	DWORD dwBufferSize = Min(dwBytesLeft, BUFFER_SIZE);
	CBuffer Buffer(dwBufferSize);

	while (dwBytesLeft > 0)
		{
		DWORD dwToRead = Min(dwBytesLeft, dwBufferSize);

		Data.Read(Buffer.GetPointer(), dwToRead);
		Result.Write(Buffer.GetPointer(), dwToRead);

		dwBytesLeft -= dwToRead;
		}
	}
