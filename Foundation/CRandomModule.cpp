//	CRandomModule.cpp
//
//	CRandomModule class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CRandomModule::CRandomModule (DWORD dwSeed) :
		m_dwSeed(dwSeed ? dwSeed : InitSeed())

//	CRandomModule constructor

	{
	}

DWORD CRandomModule::InitSeed ()
	{
	return (DWORD)MAKELONG(rand() % 0x10000, rand() % 0x10000) * ::GetTickCount();
	}

DWORD CRandomModule::Random ()

//	Random
//
//	Returns a random 31-bit number: 0 to 2^31-1 (2147483647)
//
//	Based on code written by William S. England (Oct 1988) based
//	on:
//
//	Stephen K. Park and Keith W. Miller. RANDOM NUMBER GENERATORS:
//	GOOD ONES ARE HARD TO FIND. Communications of the ACM,
//	New York, NY.,October 1988 p.1192

	{
	//	Random

	int lo, hi, test;

	hi = m_dwSeed / Q;
	lo = m_dwSeed % Q;

	test = A * lo - R * hi;

	if (test > 0)
		m_dwSeed = test;
	else
		m_dwSeed = test + M;

	//	Done

	return m_dwSeed;
	}

double CRandomModule::RandomDouble ()

//	RandomDouble
//
//	Returns a random double between 0 and 1

	{
	return (Random() / 2147483648.0);
	}

int CRandomModule::RandomInRange (int iFrom, int iTo)

//	RandomInRange
//
//	Returns a random integer in the range iFrom to iTo (inclusive)

	{
	DWORD dwRange = Abs(iTo - iFrom) + 1;
	if (dwRange == 0)
		return iFrom;

	return (iFrom + (Random() % dwRange));
	}

double CRandomModule::RandomInRange (double rFrom, double rTo)

//	RandomInRange
//
//	Returns a random double in the range rFrom to rTo (inclusive)

	{
	return (rFrom + (RandomDouble() * (rTo - rFrom)));
	}

