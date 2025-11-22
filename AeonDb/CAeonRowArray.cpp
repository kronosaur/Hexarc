//	CAeonRowArray.cpp
//
//	CAeonRowArray class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CAeonRowArray::DeleteAll (void)

//	DeleteAll
//
//	Deletes all rows
	
	{
	CSmartLock Lock(m_cs);

	m_Order.DeleteAll();
	m_Rows.DeleteAll();
	m_dwMemoryUsed = 0;
	m_dwChanges = 0;
	}

bool CAeonRowArray::FindData (const CRowKey &Key, CDatum *retData, SEQUENCENUMBER *retRowID)

//	FindData
//
//	Returns the data at the given path. Returns TRUE if found.

	{
	CSmartLock Lock(m_cs);

	//	Find the row

	int iIndexPos;
	if (!FindKey(Key, &iIndexPos))
		return false;

	if (retData)
		*retData = m_Rows[m_Order[iIndexPos]].Value.GetValue();

	if (retRowID)
		*retRowID = m_Rows[m_Order[iIndexPos]].RowID;

	return true;
	}

bool CAeonRowArray::FindKey (const CRowKey &Key, int *retiPos)

//	FindKey
//
//	Looks for the key in the array. If found, we return TRUE.
//	If required, we return the position at which the key should be inserted.

	{
	CSmartLock Lock(m_cs);

	int iCount = GetCount();
	int iMin = 0;
	int iMax = iCount;
	int iTry = iMax / 2;

	while (true)
		{
		if (iMax <= iMin)
			{
			if (retiPos)
				*retiPos = iMin;
			return false;
			}

		int iCompare = CRowKey::Compare(m_Dims, Key, CRowKey(m_Dims, GetKey(iTry)));
		if (iCompare == 0)
			{
			if (retiPos)
				*retiPos = iTry;
			return true;
			}
		else if (iCompare == -1)
			{
			iMin = iTry + 1;
			iTry = iMin + (iMax - iMin) / 2;
			}
		else if (iCompare == 1)
			{
			iMax = iTry;
			iTry = iMin + (iMax - iMin) / 2;
			}
		}

	return false;
	}

CDatum CAeonRowArray::GetData (int iIndex)

//	GetData
//
//	Gets the data at the index

	{
	CSmartLock Lock(m_cs);

	return m_Rows[m_Order[iIndex]].Value.GetValue();
	}

bool CAeonRowArray::GetRow (int iIndex, CRowKey *retKey, CDatum *retData, SEQUENCENUMBER *retRowID)

//	GetRow
//
//	Returns the row by index

	{
	CSmartLock Lock(m_cs);

	SEntry *pEntry = &m_Rows[m_Order[iIndex]];

	if (retKey)
		CRowKey::CreateFromEncodedKey(m_Dims, pEntry->sKey, retKey);

	if (retData)
		*retData = pEntry->Value.GetValue();

	if (retRowID)
		*retRowID = pEntry->RowID;

	return true;
	}

DWORD CAeonRowArray::GetRowSize (int iIndex)

//	GetRowSize
//
//	Returns the serialized size of the given row

	{
	return m_Rows[m_Order[iIndex]].Value.GetSerializedSize();
	}

void CAeonRowArray::Init (const CTableDimensions &Dims)

//	Init
//
//	Initializes the dimensions of the array

	{
	ASSERT(!IsInitialized());
	ASSERT(Dims.GetCount() > 0);

	m_Dims = Dims;
	}

bool CAeonRowArray::Insert (const CRowKey &Key, CDatum dData, SEQUENCENUMBER RowID)

//	Insert
//
//	Inserts or updates a row.

	{
	CSmartLock Lock(m_cs);

	SEntry *pEntry;
	int iIndexPos;

	//	If not found, we need to insert a new row

	if (!FindKey(Key, &iIndexPos))
		{
		int iValuePos = m_Rows.GetCount();
		pEntry = m_Rows.Insert();
		m_Order.Insert(iValuePos, iIndexPos);

		//	Only set RowID if this is a new row

		pEntry->RowID = RowID;
		}

	//	If it is found, we just overwrite it

	else
		{
		pEntry = &m_Rows[m_Order[iIndexPos]];

		//	If the old value is nil then we treat this as a new row

		if (pEntry->Value.IsNil())
			pEntry->RowID = RowID;

		//	Decrement the memory used since we're about to overwrite this

		m_dwMemoryUsed -= CAeonRowValue::GetSerializedKeySize(pEntry->sKey) + pEntry->Value.GetSerializedSize();
		}

	//	Set the key

	pEntry->sKey = Key.AsEncodedString();

	//	Insert the data to the row

	pEntry->Value.SetValue(dData);

	//	Keep track of memory used

	m_dwChanges++;
	m_dwMemoryUsed += CAeonRowValue::GetSerializedKeySize(pEntry->sKey) + pEntry->Value.GetSerializedSize();

	//	Done

	return true;
	}

void CAeonRowArray::WriteData (IByteStream &Stream, int iIndex, DWORD *retdwSize, SEQUENCENUMBER *retRowID)

//	WriteData
//
//	Writes the row value to the stream.

	{
	CSmartLock Lock(m_cs);
	
	//	Write out the value

	CAeonRowValue &Value = m_Rows[m_Order[iIndex]].Value;
	Value.Serialize(Stream);

	//	Return size

	if (retdwSize)
		*retdwSize = Value.GetSerializedSize();

	//	Return rowID

	if (retRowID)
		*retRowID = m_Rows[m_Order[iIndex]].RowID;
	}
