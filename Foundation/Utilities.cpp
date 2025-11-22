//	Utilities.cpp
//
//	Utility functions
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "psapi.h"

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

CString sysGetCurrentThreadName ()

//	sysGetCurrentThreadName
//
//	Returns the name of the current thread, if it has one.

	{
	PTSTR pResult;
	if (SUCCEEDED(::GetThreadDescription(::GetCurrentThread(), &pResult)) && pResult)
		return CString(pResult);
	else
		return CString();
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

bool sysGetProcessMemoryInfo (SProcessMemoryInfo& retInfo)

//	sysGetProcessMemoryInfo
//
//	Returns memory info stats

	{
	PROCESS_MEMORY_COUNTERS_EX MemCounters;
	MemCounters.cb = sizeof(MemCounters);
	if (!::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&MemCounters, sizeof(MemCounters)))
		return false;

	retInfo.dwlWorkingSetSize = MemCounters.WorkingSetSize;
	retInfo.dwlCommittedSize = MemCounters.PrivateUsage;
	
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

DWORD sysGetSecondsElapsed (DWORDLONG dwTick, DWORDLONG *retdwNow)

//	sysGetSecondsElapsed
//
//	Returns the number of seconds elapsed (rounded)

	{
	DWORDLONG dwElapsed = sysGetTicksElapsed(dwTick, retdwNow);
	return (DWORD)((dwElapsed + 500) / 1000);
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
#if defined(_MSC_VER)
	// MSVC lowers this to __stosb/ERMSB when profitable.
	__stosb(reinterpret_cast<unsigned char*>(pDest), Value, Count);
#else
	// GCC/Clang also recognize this as a builtin and emit the right thing.
	std::memset(pDest, Value, Count);
#endif
	}

