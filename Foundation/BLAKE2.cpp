//	BLAKE2.cpp
//
//	Crypto functions
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "BLAKE2/blake2.h"

CStringBuffer cryptoBLAKE2 (const IMemoryBlock& Data, int iPos, int iLength)

//	cryptoBLAKE2
//
//	Computes the BLAKE2 hash of the given data.

	{
	CStringBuffer Result(BLAKE2B_OUTBYTES);

	const char* input = (const char*)Data.GetPointer() + iPos;
	if (iLength == -1)
		iLength = Data.GetLength() - iPos;

	if (blake2b(Result.GetPointer(), Result.GetLength(), input, iLength, NULL, 0) != 0)
		return CStringBuffer();

	return Result;
	}
