//	Utilities.cpp
//
//	Utility functions
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

DWORD utlAdler32 (IMemoryBlock &Data)

//	utlAdler32
//
//	Computes Adler-32 checksum.
//	See: http://en.wikipedia.org/wiki/Adler-32
//
//	Adapted fomr Mark Adler's zlib
//  Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler

	{
	BYTE *pPos = (BYTE *)Data.GetPointer();
	DWORD dwLen = Data.GetLength();
    if (dwLen == 0)
		return 1;

    DWORD s1 = 1;
    DWORD s2 = 0;

    while (dwLen > 0)
		{
        int k = (dwLen < NMAX ? dwLen : NMAX);
        dwLen -= k;

        while (k >= 16)
			{
            DO16(pPos);
			pPos += 16;
            k -= 16;
			}

        if (k != 0)
			do 
				{
				s1 += *pPos++;
				s2 += s1;
				}
			while (--k);

        s1 %= BASE;
        s2 %= BASE;
		}

    return (s2 << 16) | s1;
	}

CString sysGetDNSName (void)

//	sysGetDNSName
//
//	Returns fully qualified DNS name for this machine

	{
	TCHAR szBuffer16[1024];
	DWORD dwSize = 1024;
	::GetComputerNameEx(ComputerNameDnsFullyQualified, szBuffer16, &dwSize);
	return CString(szBuffer16, dwSize);
	}

bool sysGetMemoryInfo (SSystemMemoryInfo *retInfo)

//	sysGetMemoryInfo
//
//	Returns memory info stats
//	See also: http://msdn.microsoft.com/en-us/library/aa965225

	{
	MEMORYSTATUSEX MemStatus;
	MemStatus.dwLength = sizeof(MemStatus);
	if (!::GlobalMemoryStatusEx(&MemStatus))
		return false;

	retInfo->dwlTotalPhysicalMemory = MemStatus.ullTotalPhys;
	retInfo->dwlAvailPhysicalMemory = MemStatus.ullAvailPhys;

	return true;
	}

CString sysGetOSErrorText (DWORD dwError)

//	sysGetOSErrorText
//
//	Returns text for a standard operating system error

	{
	LPTSTR pTemp = NULL;

	int iLen = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			NULL,
			dwError,
			LANG_NEUTRAL,
			(LPTSTR)&pTemp,
			0,
			NULL);
	if (iLen == 0)
		return CString();

	//	Convert to UTF-8

	CString sText = CString(pTemp, iLen);

	//	Free buffer

	::LocalFree((HLOCAL)pTemp);

	//	Done

	return sText;
	}

DWORD sysGetTicksElapsed (DWORD dwTick, DWORD *retdwNow)

//	sysGetTicksElapsed
//
//	Returns the number of milliseconds since the given tick count

	{
	DWORD dwNow = sysGetTickCount();
	if (retdwNow)
		*retdwNow = dwNow;

	if (dwNow < dwTick)
		return (0xffffffff - dwTick) + dwNow + 1;
	else
		return dwNow - dwTick;
	}

DWORDLONG sysGetTicksElapsed (DWORDLONG dwTick, DWORDLONG *retdwNow)

//	sysGetTicksElapsed
//
//	Returns the number of milliseconds since the given tick count

	{
	DWORDLONG dwNow = sysGetTickCount64();
	if (retdwNow)
		*retdwNow = dwNow;

	return dwNow - dwTick;
	}

bool sysIsBigEndian (void)

//	sysIsBigEndian
//
//	Returns TRUE if we're running on a big-endian architecture

	{
	return ((*(unsigned short *) ("#S") >> 8) == '#');
	}

int utlBitsSet (DWORD dwValue)

//	utlBitsSet
//
//	Returns the number of bits set in dwValue.
//
//	See: http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
//	See: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel

	{
	dwValue = dwValue - ((dwValue >> 1) & 0x55555555);
	dwValue = (dwValue & 0x33333333) + ((dwValue >> 2) & 0x33333333);
	DWORD dwCount = ((dwValue + (dwValue >> 4) & 0x0f0f0f0f) * 0x01010101) >> 24;

	return dwCount;
	}

bool utlMemCompare (void *pSource, void *pDest, int iCount)

//	utlMemCompare
//
//	Compare two blocks of memory for equality

	{
	char *pS = (char *)pSource;
	char *pD = (char *)pDest;

	for (; iCount > 0; iCount--)
		if (*pD++ != *pS++)
			return false;

	return true;
	}

void utlMemCopy (const void *pSource, void *pDest, DWORD dwCount)

//	utlMemCopy
//
//	Copies a block of memory of Count bytes from pSource to pDest.
//	
//	Inputs:
//		pSource: Pointer to source memory block
//		pDest: Pointer to destination memory block
//		dwCount: Number of bytes to copy

	{
	const char *pS = (const char *)pSource;
	char *pD = (char *)pDest;
	const char *pSEnd = pS + dwCount;

	while (pS < pSEnd)
		*pD++ = *pS++;
	}

void utlMemCopy (const void *pSource, void *pDest, DWORDLONG dwCount)

//	utlMemCopy
//
//	Copies a block of memory of Count bytes from pSource to pDest.
//	
//	Inputs:
//		pSource: Pointer to source memory block
//		pDest: Pointer to destination memory block
//		dwCount: Number of bytes to copy

	{
	const char *pS = (const char *)pSource;
	char *pD = (char *)pDest;
	const char *pSEnd = pS + dwCount;

	while (pS < pSEnd)
		*pD++ = *pS++;
	}

void utlMemReverse (void *pSource, void *pDest, int iCount)

//	ultMemReverse
//
//	Reverse the bytes in pSource and store then in pDest.

	{
	ASSERT(pSource);
	ASSERT(pDest);
	ASSERT(pSource != pDest);

	BYTE *pDst = (BYTE *)pDest;
	BYTE *pDstEnd = (BYTE *)pDest + iCount;
	BYTE *pSrc = (BYTE *)pSource + iCount - 1;

	while (pDst < pDstEnd)
		*pDst++ = *pSrc--;
	}

void utlMemSet (void *pDest, int Count, char Value)

//	utlMemSet
//
//	Initializes a block of memory to the given value.
//	
//	Inputs:
//		pDest: Pointer to block of memory to initialize
//		Count: Length of block in bytes
//		Value: Value to initialize to

	{
	char *pPos = (char *)pDest;
	char *pEndPos = pPos + Count;
	DWORD dwValue;
	DWORD *pdwPos;

	//	Store the initial unaligned piece

	while (pPos < pEndPos && ((DWORD_PTR)pPos % sizeof(DWORD)))
		*pPos++ = Value;

	//	Store the aligned piece

	dwValue = ((BYTE)Value) << 24 | ((BYTE)Value) << 16 | ((BYTE)Value) << 8 | (BYTE)Value;
	pdwPos = (DWORD *)pPos;
	while (pdwPos < (DWORD *)(pEndPos - (sizeof(DWORD) - 1)))
		*pdwPos++ = dwValue;

	//	Store the ending unaligned piece

	pPos = (char *)pdwPos;
	while (pPos < pEndPos)
		*pPos++ = Value;
	}

