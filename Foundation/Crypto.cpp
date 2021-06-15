//	Crypto.cpp
//
//	Crypto functions
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

//	These are the sets that we use to generate random passwords
//	Capital Os and zeros have been removed to avoid confusion.
char g_Alpha_set[] = "abcdefghijklmnopqrstuvwxyz";
char g_AlphaCode24_set[] = "ABCDEFGHJKLMNPQRSTUVWXYZ";
char g_AlphaCode32_set[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
char g_AlphaMixed_set[] = "ABCDEFGHIJKLMNPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char g_Numbers_set[] = "123456789";
char g_Symbols_set[] = "!@#$%&+=_?:~";

const int MAX_SET_SIZE = (sizeof(g_AlphaMixed_set) + sizeof(g_Numbers_set) + sizeof(g_Symbols_set));

void cryptoRandom (int iCount, CIPInteger *retx)

//	cryptoRandom
//
//	Generate iCount BYTEs of randomness

	{
	//	Edge cases

	if (iCount == 0)
		*retx = CIPInteger();

	//	Generate bytes

	CBuffer Chaos(iCount);

	int iLeft = iCount;
	while (iLeft)
		{
		//	rand_s is Window's cryptographic random number generator.
		//	NOTE: rand_s IS thread-safe, since it just calls an internal OS
		//	function.

		unsigned int dwRnd;
		rand_s(&dwRnd);

		int iWrite = Min(iLeft, (int)sizeof(DWORD));
		Chaos.Write(&dwRnd, iWrite);

		iLeft -= iWrite;
		}

	//	Initialize. Note that CIPInteger expects the byte stream to be in big-
	//	endian order, but since random is random it doesn't matter that we're
	//	backwards.

	retx->InitFromBytes(Chaos);
	}

CString cryptoRandomCode (int iChars, DWORD dwFlags)

//	cryptoRandomCode
//
//	Generates a random code of the given number of characters from the following
//	set:
//
//	A-Z (except I and O)
//	2-9

	{
	if (iChars <= 0)
		return NULL_STR;

	//	Get the set

	char *pSet = g_AlphaCode32_set;
	int iSetSize = sizeof(g_AlphaCode32_set) - 1;

	//	Now generate a password of the appropriate number of characters

	CString sCode(iChars);
	char *pPos = sCode.GetParsePointer();
	char *pPosEnd = pPos + iChars;
	while (pPos < pPosEnd)
		{
		unsigned int dwRnd;
		rand_s(&dwRnd);
		*pPos++ = pSet[dwRnd % iSetSize];
		}

	//	Done

	return sCode;
	}

CString cryptoRandomCodeBlock (int iChars)

//	cryptoRandomCodeBlock
//
//	Returns a random code of the given number of characters. One of the 
//	characters (randomly) is a number from 2-9. The remaining are uppercase
//	characters from A-Z, except I and 0.

	{
	if (iChars <= 0)
		return NULL_STR;

	const char *pSet = g_AlphaCode24_set;
	const int iSetSize = sizeof(g_AlphaCode24_set) - 1;

	//	Figure out the position of the number.

	unsigned int dwRnd;
	rand_s(&dwRnd);
	const int iNumberPos = dwRnd % iChars;

	//	Generate the characters.

	CString sCode(iChars);
	char *pPos = sCode.GetParsePointer();
	char *pPosEnd = pPos + iChars;
	char *pPosNumber = pPos + iNumberPos;
	while (pPos < pPosEnd)
		{
		if (pPos == pPosNumber)
			{
			rand_s(&dwRnd);
			*pPos++ = '2' + (dwRnd % 8);
			}
		else
			{
			rand_s(&dwRnd);
			*pPos++ = pSet[dwRnd % iSetSize];
			}
		}

	//	Done

	return sCode;
	}

CString cryptoRandomUserPassword (int iChars, DWORD dwFlags)

//	cryptoRandomUserPassword
//
//	Generates a random password for a user (e.g., when reseting a password).

	{
	if (iChars <= 0)
		return NULL_STR;

	//	Put together a large string of possible characters based on the flags.

	CString sSet(MAX_SET_SIZE);
	char *pSet = sSet.GetParsePointer();
	char *pPos = pSet;

	if (dwFlags & CRYPTOPASS_MIXED_CASE)
		{
		char *pSrc = g_AlphaMixed_set;
		char *pSrcEnd = pSrc + sizeof(g_AlphaMixed_set) - 1;
		while (pSrc < pSrcEnd)
			*pPos++ = *pSrc++;
		}
	else
		{
		char *pSrc = g_Alpha_set;
		char *pSrcEnd = pSrc + sizeof(g_Alpha_set) - 1;
		while (pSrc < pSrcEnd)
			*pPos++ = *pSrc++;
		}

	if (dwFlags & CRYPTOPASS_NUMBERS)
		{
		char *pSrc = g_Numbers_set;
		char *pSrcEnd = pSrc + sizeof(g_Numbers_set) - 1;
		while (pSrc < pSrcEnd)
			*pPos++ = *pSrc++;
		}

	if (dwFlags & CRYPTOPASS_SYMBOLS)
		{
		char *pSrc = g_Symbols_set;
		char *pSrcEnd = pSrc + sizeof(g_Symbols_set) - 1;
		while (pSrc < pSrcEnd)
			*pPos++ = *pSrc++;
		}

	int iSetSize = (int)(pPos - pSet);

	//	Now generate a password of the appropriate number of characters

	CString sPassword(iChars);
	pPos = sPassword.GetParsePointer();
	char *pPosEnd = pPos + iChars;
	while (pPos < pPosEnd)
		{
		unsigned int dwRnd;
		rand_s(&dwRnd);
		*pPos++ = pSet[dwRnd % iSetSize];
		}

	//	Done

	return sPassword;
	}
