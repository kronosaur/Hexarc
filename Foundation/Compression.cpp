//	Compression.cpp
//
//	Compression functions and classes
//	Copyright (c) 2013 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#define ZLIB_WINAPI
#include "../zlib-1.2.7/zlib.h"

const DWORD BUFFER_SIZE =					1024 * 1024;

void Deflate (const IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer);
void Inflate (const IMemoryBlock &Data, ECompressionTypes iType, IByteStream64 &Output);
void NullCopy (const IMemoryBlock &Data, IMemoryBlock *retBuffer);
void NullCopy (const IMemoryBlock &Data, IByteStream &Result);
void NullCopy (IByteStream &Data, IByteStream &Result);

void compCompress (const IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer)

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
		case compressionZipFile:
		case compressionZlib:
			Deflate(Data, iType, retBuffer);
			break;

		default:
			throw CException(errFail);
		}
	}

void compDecompress (const IMemoryBlock &Data, ECompressionTypes iType, IByteStream64 &Output)

//	compDecompress
//
//	Decompresses Data into result buffer.

	{
	switch (iType)
		{
		case compressionNone:
			Output.Write(Data.GetPointer(), Data.GetLength());
			break;

		case compressionGzip:
		case compressionZipFile:
		case compressionZlib:
			Inflate(Data, iType, Output);
			break;

		default:
			throw CException(errFail);
		}
	}

void Deflate (const IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer)
	{
	DWORD dwSourceLeft = Data.GetLength();
	BYTE *pSource = (BYTE *)Data.GetPointer();

	//	Initialize library

	z_stream zcpr;
	utlMemSet(&zcpr, sizeof(zcpr));

	//	The entire source is available

	zcpr.next_in = pSource;
	zcpr.avail_in = dwSourceLeft;

	if (iType == compressionZlib)
		deflateInit(&zcpr, Z_DEFAULT_COMPRESSION);
	else if (iType == compressionZipFile)
		deflateInit2(&zcpr,
				Z_DEFAULT_COMPRESSION,
				Z_DEFLATED,
				-MAX_WBITS,	//	No header
				8,			//	Memory-use level 0-9 (8 is a good default)
				Z_DEFAULT_STRATEGY);
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

	DWORD dwDestBufferSize = Min(Max((DWORD)100, dwSourceLeft), BUFFER_SIZE);
	retBuffer->SetLength(dwDestBufferSize);
	BYTE *pDest = (BYTE *)retBuffer->GetPointer();

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

void Inflate (const IMemoryBlock &Data, ECompressionTypes iType, IByteStream64 &Output)
	{
	//	Initialize library

	z_stream zcpr;
	utlMemSet(&zcpr, sizeof(zcpr), '\0');

	if (iType == compressionZlib)
		inflateInit(&zcpr);
	else if (iType == compressionGzip)
		inflateInit2(&zcpr,
				0x1f);		//	Bit 0x10 means gzip header
	else if (iType == compressionZipFile)
		inflateInit2(&zcpr, -MAX_WBITS);
	else
		throw CException(errFail);

	//	Allocate a buffer to compress to

	BYTE *pBuffer = new BYTE [BUFFER_SIZE];

	//	Compress

	DWORD dwSourceLeft = Data.GetLength();
	BYTE *pSource = (BYTE *)Data.GetPointer();

	//	The entire source is available

	zcpr.next_in = pSource;
	zcpr.avail_in = dwSourceLeft;

	int ret;
	int iWritten = 0;

	while (true)
		{
		zcpr.next_out = pBuffer;
		zcpr.avail_out = BUFFER_SIZE;

		ret = inflate(&zcpr, Z_FINISH);

		//	If error, then fail

		if (ret == Z_STREAM_ERROR || ret == Z_MEM_ERROR || ret == Z_DATA_ERROR
				|| (ret == Z_BUF_ERROR && zcpr.avail_in == 0))
			{
			inflateEnd(&zcpr);
			delete [] pBuffer;
			throw CException(errFail);
			}

		//	Write out the compressed data

		int iChunk = zcpr.total_out - iWritten;
		iWritten += iChunk;

		try
			{
			Output.Write(pBuffer, iChunk);
			}
		catch (...)
			{
			inflateEnd(&zcpr);
			delete [] pBuffer;
			throw CException(errFail);
			}

		//	If we need more output buffer, then continue

		if (ret == Z_BUF_ERROR
				|| (ret == Z_OK && zcpr.avail_out == 0))
			continue;

		//	Otherwise, we're done.

		else
			break;
		}

	//	Done

	inflateEnd(&zcpr);
	delete [] pBuffer;
	}

void NullCopy (const IMemoryBlock &Data, IMemoryBlock *retBuffer)
	{
	retBuffer->SetLength(Data.GetLength());
	utlMemCopy(Data.GetPointer(), retBuffer->GetPointer(), Data.GetLength());
	}

void NullCopy (const IMemoryBlock &Data, IByteStream &Result)
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
