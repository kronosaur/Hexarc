//	CLargeSet.cpp
//
//	CLargeSet class
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const int BITS_PER_DWORD =						32;

CLargeSet::CLargeSet (int iSize)

//	CLargeSet constructor

	{
	if (iSize != -1)
		{
		int iCount = AlignUp(iSize, BITS_PER_DWORD) / BITS_PER_DWORD;
		m_Set.InsertEmpty(iCount);
		for (int i = 0; i < iCount; i++)
			m_Set[i] = 0;
		}
	}

void CLargeSet::Clear (DWORD dwValue)

//	Clear
//
//	Remove the value from the set

	{
	DWORD dwPos = dwValue / BITS_PER_DWORD;
	DWORD dwBit = dwValue % BITS_PER_DWORD;

	if (dwPos >= (DWORD)m_Set.GetCount())
		return;

	m_Set[dwPos] = (m_Set[dwPos] & ~(1 << dwBit));
	}

void CLargeSet::Clear (const CLargeSet &Src)

//	Clear
//
//	Clear the values in the given set

	{
	int i;

	int iCount = Min(Src.m_Set.GetCount(), m_Set.GetCount());
	for (i = 0; i < iCount; i++)
		{
		DWORD dwToClear = Src.m_Set[i];
		if (dwToClear)
			m_Set[i] &= ~dwToClear;
		}
	}

void CLargeSet::ClearAll (void)

//	ClearAll
//
//	Removes all bits

	{
	m_Set.DeleteAll();
	}

int CLargeSet::GetCount (void) const

//	GetCount
//
//	Returns the count of items in the set.

	{
	int i;

	int iCount = 0;
	for (i = 0; i < m_Set.GetCount(); i++)
		{
		DWORD dwBits = m_Set[i];
		if (dwBits)
			iCount += utlBitsSet(dwBits);
		}

	return iCount;
	}

DWORD CLargeSet::GetNext (SLargeSetEnumerator &i) const

//	GetNext
//
//	Get the next bit

	{
	//	Get the current

	DWORD dwCurrent = (i.dwDWORDIndex * BITS_PER_DWORD) + i.dwBitIndex;

	//	Advance until we find the next bit or until we hit the end

	while (true)
		{
		i.dwBitIndex++;
		i.dwBitMask = i.dwBitMask << 1;

		if (i.dwBitIndex == BITS_PER_DWORD)
			{
			//	Advance DWORDs until we hit the next non-zero one or until we 
			//	reach the end of the array.

			do
				{
				i.dwDWORDIndex++;

				if (i.dwDWORDIndex >= (DWORD)m_Set.GetCount())
					{
					i.dwDWORDIndex = 0xffffffff;
					return dwCurrent;
					}
				}
			while (m_Set[i.dwDWORDIndex] == 0);

			//	Otherwise, start looking for the next bit

			i.dwBitIndex = 0;
			i.dwBitMask = 1;
			}

		//	If this bit is set, then we're done

		if ((m_Set[i.dwDWORDIndex] & i.dwBitMask))
			return dwCurrent;

		//	Otherwise, loop
		}
	}

void CLargeSet::GetValues (TArray<DWORD> *retResult)

//	GetValues
//
//	Returns all values

	{
	int i;

	//	We estimate the number of values by counting the number of DWORDs that
	//	are non-zero. This helps us to size the result array properly.

	int iEstimate = 0;
	for (i = 0; i < m_Set.GetCount(); i++)
		if (m_Set[i])
			iEstimate++;

	//	Size the resulting array

	retResult->DeleteAll();
	retResult->GrowToFit(iEstimate * BITS_PER_DWORD);

	//	Get all bits

	for (i = 0; i < m_Set.GetCount(); i++)
		{
		DWORD dwSet = m_Set[i];
		if (dwSet)
			{
			DWORD dwBit = 1;
			DWORD dwCount = 0;
			while (dwCount < BITS_PER_DWORD)
				{
				if (dwSet & dwBit)
					retResult->Insert(i * BITS_PER_DWORD + dwCount);

				dwBit = dwBit << 1;
				dwCount++;
				}
			}
		}
	}

bool CLargeSet::HasMore (SLargeSetEnumerator &i) const

//	HasMore
//
//	Returns TRUE if we have more (if GetNext will succeed)

	{
	return (i.dwDWORDIndex != 0xffffffff);
	}

bool CLargeSet::IsEmpty (void) const

//	IsEmpty
//
//	Returns TRUE if empty

	{
	int i;

	for (i = 0; i < m_Set.GetCount(); i++)
		if (m_Set[i])
			return false;

	return true;
	}

bool CLargeSet::IsSet (DWORD dwValue) const

//	IsSet
//
//	Returns TRUE if the value is in the set

	{
	DWORD dwPos = dwValue / BITS_PER_DWORD;
	DWORD dwBit = dwValue % BITS_PER_DWORD;

	if (dwPos >= (DWORD)m_Set.GetCount())
		return false;

	return ((m_Set[dwPos] & (1 << dwBit)) ? true : false);
	}

void CLargeSet::Reset (SLargeSetEnumerator &i) const

//	Reset
//
//	Resets to first element

	{
	i.dwDWORDIndex = 0xffffffff;
	i.dwBitIndex = BITS_PER_DWORD - 1;
	i.dwBitMask = 0;

	GetNext(i);
	}

void CLargeSet::Set (DWORD dwValue)

//	Set
//
//	Add the value to the set

	{
	DWORD dwPos = dwValue / BITS_PER_DWORD;
	DWORD dwBit = dwValue % BITS_PER_DWORD;
	
	int iOldCount = m_Set.GetCount();
	if (dwPos >= (DWORD)iOldCount)
		{
		m_Set.InsertEmpty(dwPos - iOldCount + 1);
		for (int i = iOldCount; i < m_Set.GetCount(); i++)
			m_Set[i] = 0;
		}

	m_Set[dwPos] = (m_Set[dwPos] | (1 << dwBit));
	}

void CLargeSet::Set (const CLargeSet &Set)

//	Set
//
//	Add the two sets

	{
	int i;

	//	Increase the size of our set, if necessary

	int iOldCount = m_Set.GetCount();
	int iNewCount = Set.m_Set.GetCount();
	if (iOldCount < iNewCount)
		{
		int iAddCount = iNewCount - iOldCount;
		m_Set.InsertEmpty(iAddCount);

		for (i = iOldCount; i < iNewCount; i++)
			m_Set[i] = Set.m_Set[i];
		}

	//	Combine the overlap

	int iCount = Min(iOldCount, iNewCount);
	for (i = 0; i < iCount; i++)
		m_Set[i] |= Set.m_Set[i];
	}
