//	MathRandom.cpp
//
//	Math functions and classes
//	Copyright (c) 2012 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>

#define	m		((unsigned long)2147483647)
#define	q		((unsigned long)44488)

#define	a		((unsigned int)48271)
#define	r		((unsigned int)3399)

DWORD g_Seed = 0;

int mathDoubleToIntStochastic (double rValue)

//	mathDoubleToIntStochastic
//
//	Converts a double into an integer and randomly rounds up or down with 
//	probability proportional to the fractional part.

	{
	double rInt = floor(rValue);
	double rFraction = rValue - rInt;

	if (mathRandomDouble() < rFraction)
		return (int)rInt + 1;
	else
		return (int)rInt;
	}

double mathLog (double rX, double rBase)

//	mathLog
//
//	Returns the logarithm of X in the given base.

	{
	if (rX == 0.0 || rBase <= 1.0)
		return 0.0;
	else
		return log(rX) / log(rBase);
	}

DWORD mathRandom (void)

//	mathRandom
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
	//	Seed it

	if (g_Seed == 0)
		{
		g_Seed = MAKELONG(rand() % 0x10000, rand() % 0x10000);
		g_Seed *= ::GetTickCount();
		}

	//	Random

	int lo, hi, test;

	hi = g_Seed / q;
	lo = g_Seed % q;

	test = a * lo - r * hi;

	if (test > 0)
		g_Seed = test;
	else
		g_Seed = test + m;

	//	Done

	return g_Seed;
	}

int mathRandom (int iFrom, int iTo)

//	mathRandom
//
//	Returns a random number between iFrom and iTo (inclusive)

	{
	DWORD dwRange = Abs(iTo - iFrom) + 1;
	if (dwRange == 0)
		return iFrom;

	return iFrom + (mathRandom() % dwRange);
	}

double mathRandom (double rFrom, double rTo)

//	mathRandom
//
//	Returns a random number >= rFrom and < rTo

	{
	double rRange = (rTo - rFrom);
	double rRandom = mathRandomDouble();
	return rFrom + (rRange * rRandom);
	}

double mathRandomMinusOneToOne (void)
	{
	DWORD dwValue = mathRandom();

	if (dwValue % 2)
		return ((dwValue >> 1) / 1073741824.0);
	else
		return ((dwValue >> 1) / -1073741824.0);
	}

double mathRandomGaussian (void)

//	mathRandomGaussian
//
//	Returns a random number with Gaussian distribution. The mean value is 0.0 
//	and the standard deviation is 1.0.
//
//	Uses the polar form of the Box-Muller transformation.
//
//	See: http://www.taygeta.com/random/gaussian.html

	{
	double x1, x2, w;

	do
		{
		x1 = mathRandomMinusOneToOne();
		x2 = mathRandomMinusOneToOne();
		w = x1 * x1 + x2 * x2;
		}
	while (w >= 1.0);

	w = sqrt((-2.0 * log(w)) / w);

	return x1 * w;
	}

void mathRandomUnique (int iFrom, int iTo, int iCount, TArray<int> *retResult)

//	mathRandomUnique
//
//	Returns iCount unique numbers from iFrom to iTo (inclusive).

	{
	int i, j;

	//	Short-circuit if we don't want any numbers.

	retResult->DeleteAll();
	if (iCount <= 0)
		return;

	//	Always from lowest to highest

	if (iTo < iFrom)
		Swap(iTo, iFrom);

	//	The range must be large enough to support the count, otherwise some
	//	numbers would not be unique.

	int iRange = (iTo - iFrom) + 1;
	if (iRange < iCount)
		return;

	//	If the range is small enough, then shuffle

	retResult->InsertEmpty(iCount);
	if (iRange < (iCount * iCount) / 2)
		{
		TArray<int> FullRange;
		FullRange.InsertEmpty(iRange);
		for (i = 0; i < iRange; i++)
			FullRange[i] = i;

		FullRange.Shuffle();

		for (i = 0; i < iCount; i++)
			retResult->GetAt(i) = FullRange[i];
		}

	//	Otherwise, we do an n x (n-1) algorithm

	else
		{
		for (i = 0; i < iCount; i++)
			{
			//	Look for a unique value

			while (true)
				{
				int iValue = mathRandom(iFrom, iTo);

				bool bFound = false;
				for (j = 0; j < i; j++)
					if (retResult->GetAt(j) == iValue)
						{
						bFound = true;
						break;
						}

				if (!bFound)
					{
					retResult->GetAt(i) = iValue;
					break;
					}
				}
			}
		}
	}

DWORD mathSeededRandom (DWORD *ioSeed)

//	mathSeededRandom
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
	//	Seed it

	if (*ioSeed == 0)
		{
		*ioSeed = MAKELONG(rand() % 0x10000, rand() % 0x10000);
		*ioSeed *= ::GetTickCount();
		}

	//	Random

	int lo, hi, test;

	hi = (*ioSeed) / q;
	lo = (*ioSeed) % q;

	test = a * lo - r * hi;

	if (test > 0)
		*ioSeed = test;
	else
		*ioSeed = test + m;

	//	Done

	return *ioSeed;
	}

int mathSeededRandom (DWORD *ioSeed, int iFrom, int iTo)

//	mathSeededRandom
//
//	Returns a random number between iFrom and iTo (inclusive)

	{
	DWORD dwRange = Abs(iTo - iFrom) + 1;
	if (dwRange == 0)
		return iFrom;

	return iFrom + (mathSeededRandom(ioSeed) % dwRange);
	}

void mathShuffledRange (int iFrom, int iTo, TArray<int> *retResult)

//	mathShuffledRange
//
//	Returns a shuffled array of values from iFrom to iTo.

	{
	int i;

	retResult->DeleteAll();

	//	Always from lowest to highest

	if (iTo < iFrom)
		Swap(iTo, iFrom);

	//	The range must be large enough to support the count, otherwise some
	//	numbers would not be unique.

	int iRange = (iTo - iFrom) + 1;
	
	//	Fill the array

	retResult->InsertEmpty(iRange);
	for (i = 0; i < iRange; i++)
		retResult->GetAt(i) = iFrom + i;

	//	Shuffle

	retResult->Shuffle();
	}

