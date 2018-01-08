//	SuperFastHash.cpp
//
//	Based on Paul Hsieh's hash function:
//	http://wwww.azillionmonkeys.com/qed/hash.html
//
//	Copyright (c) 2017 by Kronosaur Productions. All Rights Reserved.

#include "stdafx.h"

DWORD HashBase::SuperFastHash (BYTE *pData, int iLen)

//	SuperFastHash
//
//	Hash function

	{
	DWORD dwHash = iLen;
	DWORD dwTemp;

	//	Short-circuit

	if (iLen <= 0 || pData == NULL)
		return 0;

	//	Process by DWORDs. iLen is now the number of complete DWORDs in the
	//	data, and iRemainder is the number of bytes left over.

	int iRemainder = iLen & 3;
	iLen >>= 2;

	//	Loop

	for (; iLen > 0; iLen--)
		{
		dwHash += Get16Bits(pData);
		dwTemp = (Get16Bits(pData + 2) << 11) ^ dwHash;
		dwHash = (dwHash << 16) ^ dwTemp;
		pData += 4;
		dwHash += dwHash >> 11;
		}

	//	Handle remainder

	switch (iRemainder)
		{
		case 3:
			dwHash += Get16Bits(pData);
			dwHash ^= dwHash << 16;
			dwHash ^= pData[2] << 18;
			dwHash += dwHash >> 11;
			break;

		case 2:
			dwHash += Get16Bits(pData);
			dwHash ^= dwHash << 11;
			dwHash += dwHash >> 17;
			break;

		case 1:
			dwHash += *pData;
			dwHash ^= dwHash << 10;
			dwHash += dwHash >> 1;
			break;
		}

	//	Force "avalanching" of final 127 bits

	dwHash ^= dwHash << 3;
	dwHash += dwHash >> 5;
	dwHash ^= dwHash << 4;
	dwHash += dwHash >> 17;
	dwHash ^= dwHash << 25;
	dwHash += dwHash >> 6;

	return dwHash;
	}
