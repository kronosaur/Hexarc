//	CCryptoDigest.cpp
//
//	CCryptoDigest class
//	Copyright (c) 2015 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void CCryptoDigest::AddData (void *pData, int iLength)

//	AddData
//
//	Adds data to the hash

	{
	if (!m_bInitialized)
		{
		SHAInit(&m_Ctx);
		m_bInitialized = true;
		}

	SHAUpdate(&m_Ctx, (BYTE *)pData, iLength);
	}

void CCryptoDigest::AddData (IMemoryBlock &Data)

//	AddData
//
//	Adds data to the hash

	{
	AddData(Data.GetPointer(), Data.GetLength());
	}

void CCryptoDigest::CalcDigest (CIPInteger *retDigest)

//	CalcDigest
//
//	Returns the digest. Any additional data will be for a new hash

	{
	if (!m_bInitialized)
		{
		ASSERT(false);
		return;
		}

	DIGEST Buffer;
	SHAFinal((BYTE *)Buffer, &m_Ctx);

	//	NOTE: InitFromBytes expects big-endian order, which is what SHAFinal
	//	outputs.

	retDigest->InitFromBytes(CBuffer(Buffer, sizeof(Buffer), false));
	}

void CCryptoDigest::CalcDigest (BYTE *retDigest)

//	CalcDigest
//
//	Returns the digest

	{
	if (!m_bInitialized)
		{
		ASSERT(false);
		return;
		}

	SHAFinal(retDigest, &m_Ctx);
	}
