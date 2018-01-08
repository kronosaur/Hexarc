//	CRowIterator.cpp
//
//	CRowIterator class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

CRowIterator::~CRowIterator (void)

//	CRowIterator destructor

	{
	CleanUp();
	}

void CRowIterator::AddSegment (IOrderedRowSet *pSegment)

//	AddSegment
//
//	Adds a segment to the iterator

	{
	ASSERT(pSegment != NULL);

	SEntry *pEntry = m_Data.Insert();
	pEntry->pSegment = pSegment;
	pEntry->pSegment->AddRef();
	pEntry->iPosCursor = -1;
	pEntry->iCount = pSegment->GetCount();

	ASSERT(pEntry->iCount > 0);
	}

void CRowIterator::Advance (void)

//	Advance
//
//	Advance to the next row

	{
	int i;

	//	If we're starting at the beginning, initialize all the cursors
	//	to point at the beginning

	if (m_iEntryCursor == -1)
		{
		for (i = 0; i < m_Data.GetCount(); i++)
			{
			SEntry *pEntry = &m_Data[i];
			pEntry->iPosCursor = 0;
			pEntry->sKey = pEntry->pSegment->GetKey(pEntry->iPosCursor);
			}
		}

	//	Otherwise, advance the current entry to the next row

	else
		{
		CString sKey = m_Data[m_iEntryCursor].sKey;

		//	Increment all entries that have this key

		for (i = 0; i < m_Data.GetCount(); i++)
			{
			SEntry *pEntry = &m_Data[i];

			if (i == m_iEntryCursor
					|| strEquals(sKey, pEntry->sKey))
				{
				pEntry->iPosCursor++;
				if (pEntry->iPosCursor < pEntry->iCount)
					pEntry->sKey = pEntry->pSegment->GetKey(pEntry->iPosCursor);
				else
					pEntry->sKey = NULL_STR;
				}
			}
		}

	//	Look for the smallest key

	int iBestEntry = -1;
	CString *pBestKey = NULL;

	for (i = 0; i < m_Data.GetCount(); i++)
		{
		SEntry *pEntry = &m_Data[i];
		if (!pEntry->sKey.IsEmpty()
				&& (pBestKey == NULL || CRowKey::Compare(m_Dims, CRowKey(m_Dims, pEntry->sKey), CRowKey(m_Dims, *pBestKey)) == 1))
			{
			pBestKey = &pEntry->sKey;
			iBestEntry = i;
			}
		}

	//	Done

	m_iEntryCursor = iBestEntry;
	}

void CRowIterator::AdvanceUntilDifferent (const CRowKey &PartialKey)

//	AdvanceUntilDifferent
//
//	Advances one row until we don't match the partial key or until we reach the
//	end of the array.

	{
	while (HasMore() && CRowKey::ComparePartial(m_Dims, PartialKey, CRowKey(m_Dims, m_Data[m_iEntryCursor].sKey)))
		{
		if (m_bIncludeNil)
			Advance();
		else
			AdvanceUntilNonNil();
		}
	}

void CRowIterator::AdvanceUntilNonNil (void)

//	AdvanceUntilNonNil
//
//	Advances until the row is not nil

	{
	do
		{
		Advance();
		}
	while (HasMore() && m_Data[m_iEntryCursor].pSegment->GetData(m_Data[m_iEntryCursor].iPosCursor).IsNil());
	}

void CRowIterator::AdvanceWithLimits (void)

//	AdvanceWithLimits
//
//	Advances one row taking into account the limits.
//

	{
	int i;

	//	Advance one row. If we hit the end, then we're done

	if (m_bIncludeNil)
		Advance();
	else
		AdvanceUntilNonNil();

	if (!HasLimits() || !HasMore())
		return;

	//	Compare the new row with the previous row (stored in m_Limits) to see
	//	which key does not match.

	int iDimToInc;
	CompareToLimit(&iDimToInc);

	//	Keep decrementing from right to left. This is a recursive function
	//	that decrements one limit and see if it needs to decrement others.

	int iDimLeft;
	if (!DecrementLimit(iDimToInc, &iDimLeft))
		{
		//	If we've hit our limit then we're done.

		m_iEntryCursor = -1;
		return;
		}

	//	Advance until we find the next key in this dimension.

	AdvanceUntilDifferent(m_Limits[iDimLeft].LimitKey);
	if (!HasMore())
		return;

	//	Reset the limits and key

	for (i = iDimLeft; i < m_Dims.GetCount(); i++)
		{
		CRowKey::CreateFromEncodedKeyPartial(m_Dims, i + 1, m_Data[m_iEntryCursor].sKey, &m_Limits[i].LimitKey);

		if (i > iDimLeft)
			m_Limits[i].iLeft = m_Limits[i].iLimit;
		}
	}

void CRowIterator::CleanUp (void)

//	CleanUp
//
//	Clean up data

	{
	int i;

	for (i = 0; i < m_Data.GetCount(); i++)
		m_Data[i].pSegment->Release();

	m_Data.DeleteAll();
	}

bool CRowIterator::CompareToLimit (int *retiDiffKey)

//	CompareToLimit
//
//	Compares the current row with the limit key. retiDiffKey is the index of the
//	dimension that does not match.
//
//	Returns TRUE if all dimensions except the last match. Returns FALSE if we
//	store before the last dimension.

	{
	int i;

	//	Check limits based on the number of dimension.
	//
	//	NOTE: The last dimension is always different, so we don't consider it.

	int iDimCount = m_Limits.GetCount();
	for (i = 0; i < iDimCount - 1; i++)
		{
		if (!CRowKey::ComparePartial(m_Dims, m_Limits[i].LimitKey, CRowKey(m_Dims, m_Data[m_iEntryCursor].sKey)))
			{
			*retiDiffKey = i;
			return false;
			}
		}

	//	If we get this far then we know that we still match all dimensions
	//	except for the last one, which never matches.

	*retiDiffKey = iDimCount - 1;
	return true;
	}

void CRowIterator::Copy (const CRowIterator &Src)

//	Copy
//
//	Copy from another.

	{
	int i;

	m_Dims = Src.m_Dims;
	m_Data = Src.m_Data;
	m_iEntryCursor = Src.m_iEntryCursor;

	for (i = 0; i < m_Data.GetCount(); i++)
		m_Data[i].pSegment->AddRef();

	m_bIncludeNil = Src.m_bIncludeNil;
	m_Limits = Src.m_Limits;
	}

bool CRowIterator::DecrementLimit (int iDimIndex, int *retiDimLeft)

//	DecrementLimit
//
//	This is a recursive function that decrements limits from right
//	to left starting with iDimIndex.
//
//	retiDimLeft is set to the smallest dimension with a non-zero limit
//	remaining.
//
//	We return TRUE we we still have non-zero limits; FALSE otherwise.

	{
	if (m_Limits[iDimIndex].iLeft != -1 && --(m_Limits[iDimIndex].iLeft) == 0)
		{
		//	If we're at the first key then we've reached out limit

		if (iDimIndex == 0)
			return false;

		//	Decrement the next smallest key

		else
			return DecrementLimit(iDimIndex - 1, retiDimLeft);
		}
	else
		{
		*retiDimLeft = iDimIndex;
		return true;
		}
	}

bool CRowIterator::Init (const CTableDimensions &Dims)

//	Init
//
//	Initializes

	{
	CleanUp();

	m_Dims = Dims;
	m_iEntryCursor = -1;
	return true;
	}

CString CRowIterator::GetKey (void)

//	GetKey
//
//	Returns the current key

	{
	return m_Data[m_iEntryCursor].sKey;
	}

CString CRowIterator::GetNextKey (void)

//	GetNextKey
//
//	Returns the next key

	{
	CString sKey = m_Data[m_iEntryCursor].sKey;
	AdvanceWithLimits();
	return sKey;
	}

void CRowIterator::GetNextKey (CRowKey *retKey)

//	GetNextKey
//
//	Returns the next key

	{
	CRowKey::CreateFromEncodedKey(m_Dims, m_Data[m_iEntryCursor].sKey, retKey);
	AdvanceWithLimits();
	}

CString CRowIterator::GetNextRow (CDatum *retdData)

//	GetNextRow
//
//	Returns the key and the data for the current row
//	and advances the pointer

	{
	CString sKey = m_Data[m_iEntryCursor].sKey;
	if (retdData)
		*retdData = m_Data[m_iEntryCursor].pSegment->GetData(m_Data[m_iEntryCursor].iPosCursor);
	
	AdvanceWithLimits();
	return sKey;
	}

void CRowIterator::GetNextRow (CRowKey *retKey, CDatum *retdData, SEQUENCENUMBER *retRowID)

//	GetNextRow
//
//	Returns the key and the data for the current row
//	and advances the pointer

	{
	m_Data[m_iEntryCursor].pSegment->GetRow(m_Data[m_iEntryCursor].iPosCursor, retKey, retdData, retRowID);
	AdvanceWithLimits();
	}

DWORD CRowIterator::GetNextRowSize (void)

//	GetNextRowSize
//
//	Returns the serialized size of the row and advances the pointer.

	{
	DWORD dwSize = CAeonRowValue::GetSerializedKeySize(m_Data[m_iEntryCursor].sKey)
			+ m_Data[m_iEntryCursor].pSegment->GetRowSize(m_Data[m_iEntryCursor].iPosCursor);
	
	AdvanceWithLimits();
	return dwSize;
	}

void CRowIterator::GetRow (CRowKey *retKey, CDatum *retdData, SEQUENCENUMBER *retRowID)

//	GetRow
//
//	Gets the current key and data

	{
	m_Data[m_iEntryCursor].pSegment->GetRow(m_Data[m_iEntryCursor].iPosCursor, retKey, retdData, retRowID);
	}

bool CRowIterator::HasMore (void)

//	HasMore
//
//	Returns TRUE if there are more rows

	{
	return (m_iEntryCursor != -1);
	}

void CRowIterator::Reset (void)

//	Reset
//
//	Initializes to the beginning

	{
	m_iEntryCursor = -1;
	if (m_bIncludeNil)
		Advance();
	else
		AdvanceUntilNonNil();
	}

bool CRowIterator::SelectKey (const CRowKey &Key)

//	SelectKey
//
//	Places the cursor as close as possible to the given key.
//	Returns TRUE if the key was found.

	{
	int i;

	int iFound = -1;
	for (i = 0; i < m_Data.GetCount(); i++)
		{
		SEntry *pEntry = &m_Data[i];

		if (pEntry->pSegment->FindKey(Key, &pEntry->iPosCursor))
			{
			if (iFound == -1)
				iFound = i;
			}

		if (pEntry->iPosCursor < pEntry->iCount)
			pEntry->sKey = pEntry->pSegment->GetKey(pEntry->iPosCursor);
		else
			pEntry->sKey = NULL_STR;
		}

	//	If we found it, then the key is in the right place

	if (iFound != -1)
		{
		m_iEntryCursor = iFound;
		return true;
		}

	//	Otherwise, pick the lowest key

	else
		{
		int iBestEntry = -1;
		CString *pBestKey = NULL;

		for (i = 0; i < m_Data.GetCount(); i++)
			{
			SEntry *pEntry = &m_Data[i];
			if (!pEntry->sKey.IsEmpty()
					&& (pBestKey == NULL || CRowKey::Compare(m_Dims, CRowKey(m_Dims, pEntry->sKey), CRowKey(m_Dims, *pBestKey)) == 1))
				{
				pBestKey = &pEntry->sKey;
				iBestEntry = i;
				}
			}

		//	Done

		m_iEntryCursor = iBestEntry;

		//	If this is a partial key then see if we at least match all 
		//	parts of the partial key. If so, then we consider it a match.

		if (pBestKey && Key.GetCount() < m_Dims.GetCount())
			return CRowKey::ComparePartial(m_Dims, Key, CRowKey(m_Dims, *pBestKey));
		else
			return false;
		}
	}

void CRowIterator::SetLimits (const TArray<int> &Limits)

//	SetLimits
//
//	Sets iteration limits for each dimension

	{
	int i;

	if (!HasMore())
		return;

	m_Limits.DeleteAll();
	m_Limits.InsertEmpty(m_Dims.GetCount());

	for (i = 0; i < m_Dims.GetCount(); i++)
		{
		if (i < Limits.GetCount() && Limits[i] > 0)
			m_Limits[i].iLimit = Limits[i];
		else
			m_Limits[i].iLimit = -1;

		m_Limits[i].iLeft = m_Limits[i].iLimit;
		CRowKey::CreateFromEncodedKeyPartial(m_Dims, i + 1, m_Data[m_iEntryCursor].sKey, &m_Limits[i].LimitKey);
		}
	}

void CRowIterator::WriteNextRow (IByteStream &Stream, DWORD *retdwKeySize, DWORD *retdwDataSize, SEQUENCENUMBER *retRowID)

//	WriteNextRow
//
//	Serializes the current row to the stream and advances to the next row.
//
//	DWORD		row key size
//	BYTEs		padded to DWORD-align
//	...
//	DWORD		row data size
//	BYTEs		padded to DWORD-align

	{
	CAeonRowValue::SerializeKey(Stream, m_Data[m_iEntryCursor].sKey, retdwKeySize);
	m_Data[m_iEntryCursor].pSegment->WriteData(Stream, m_Data[m_iEntryCursor].iPosCursor, retdwDataSize, retRowID);

	if (m_bIncludeNil)
		Advance();
	else
		AdvanceUntilNonNil();
	}
