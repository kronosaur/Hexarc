//	FoundationCompression.h
//
//	Foundation header file
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

enum ECompressionTypes
	{
	compressionNone =						0,
	compressionZlib =						1,	//	Zlib format
	compressionGzip =						2,	//	Gzip format
	};

void compCompress (IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer);
void compDecompress (IMemoryBlock &Data, ECompressionTypes iType, IMemoryBlock *retBuffer);