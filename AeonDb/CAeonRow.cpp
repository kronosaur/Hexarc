//	CAeonRow.cpp
//
//	CAeonRow class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int DEFAULT_VAR_BLOCK_SIZE =					4096;
const int DEFAULT_ROW_COUNT =						16;

CAeonRow::CAeonRow (void) : m_iDimCount(-1),
		m_dwFixedBlockAlloc(0),
		m_pFixedBlock(NULL),
		m_dwVarBlockAlloc(0),
		m_pVarBlock(NULL)

//	CAeonRow constructor

	{
	}

CAeonRow::~CAeonRow (void)

//	CAeonRow destructor

	{
	if (m_pFixedBlock)
		delete m_pFixedBlock;

	if (m_pVarBlock)
		delete m_pVarBlock;
	}

DWORD CAeonRow::AllocVarBlock (void *pSource, int iLen)

//	AllocVarBlock
//
//	Allocates a new item entry

	{
	DWORD dwSizeUp = AlignUp((DWORD)iLen, (DWORD)sizeof(DWORD));

	//	Do we need to reallocate?

	SItemHeader *pHeader = (SItemHeader *)m_pVarBlock;
	if (sizeof(SItemHeader) + pHeader->dwSize + sizeof(SItemHeader) + dwSizeUp > m_dwVarBlockAlloc)
		{
		DWORD dwNewVarBlockAlloc = AlignUp(m_dwVarBlockAlloc + (DWORD)sizeof(SItemHeader) + dwSizeUp, (DWORD)DEFAULT_VAR_BLOCK_SIZE);
		void *pNewVarBlock = new char [dwNewVarBlockAlloc];
		utlMemCopy(m_pVarBlock, pNewVarBlock, m_dwVarBlockAlloc);

		delete m_pVarBlock;
		m_pVarBlock = pNewVarBlock;
		m_dwVarBlockAlloc = dwNewVarBlockAlloc;

		pHeader = (SItemHeader *)m_pVarBlock;
		}

	//	Compute the offset

	DWORD dwPos = sizeof(SItemHeader) + pHeader->dwSize;

	//	Initialize the new item

	SItemHeader *pNew = GetItemInVarBlock(dwPos);
	pNew->dwSize = iLen;

	//	Copy the buffer

	utlMemCopy(pSource, &pNew[1], iLen);

	//	Increment the sizes, etc.

	pHeader->dwSize += sizeof(SItemHeader) + dwSizeUp;

	//	Done

	return dwPos;
	}

int CAeonRow::CompareKeys (const SDimensionDesc &Desc, const CString &sKey1, const CString &sKey2)

//	CompareKeys
//
//	Compares two keys

	{
	switch (Desc.iKeyType)
		{
		case keyInt32:
			{
			int *pKey1 = (int *)(LPSTR)sKey1;
			int *pKey2 = (int *)(LPSTR)sKey2;
			if (*pKey1 > *pKey2)
				return Desc.iSort;
			else if (*pKey1 < *pKey2)
				return -Desc.iSort;
			else
				return 0;
			}

		case keyUTF8:
			return Desc.iSort * KeyCompare(sKey1, sKey2);

		default:
			ASSERT(false);
			return 0;
		}
	}

CAeonRow::SCellEntry *CAeonRow::FindCellEntry (SCellEntry *pIndex, int iIndexCount, const SDimensionDesc &Desc, const CString &sKey, int *retiPos)

//	FindCellEntry
//
//	Looks for the key in the given index and returns the cell.
//	Returns NULL if not found.

	{
	int iCount = iIndexCount;
	int iMin = 0;
	int iMax = iCount;
	int iTry = iMax / 2;

	while (true)
		{
		if (iMax <= iMin)
			{
			if (retiPos)
				*retiPos = iMin;
			return NULL;
			}

		CStringBuffer sKeyTry = ItemToKey(GetItemInVarBlock(pIndex->dwKeyOffset));
		int iCompare = CompareKeys(Desc, sKey, sKeyTry);
		if (iCompare == 0)
			{
			if (retiPos)
				*retiPos = iTry;
			return &pIndex[iTry];
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

	return NULL;
	}

int CAeonRow::GetCount (void)

//	GetCount
//
//	For 0D rows this always returns 1;
//	For 1D rows we return the number of sub-rows
//	For 2D rows we return the number of cells

	{
	switch (m_iDimCount)
		{
		case 0:
			return 1;

		case 1:
			return ((SArrayHeader *)m_pFixedBlock)->dwCount;

		case 2:
			//	LATER: count the number of cells
			ASSERT(false);
			return 0;

		default:
			ASSERT(false);
			return 0;
		}
	}

DWORD CAeonRow::GetFixedBlockSize (void)

//	GetFixedBlockSize
//
//	Returns the current size of the fixed block

	{
	switch (m_iDimCount)
		{
		case 0:
			{
			SItemHeader *pHeader = (SItemHeader *)m_pFixedBlock;
			return sizeof(SItemHeader) + pHeader->dwSize;
			}

		case 1:
		case 2:
			{
			SArrayHeader *pHeader = (SArrayHeader *)m_pFixedBlock;
			return sizeof(SArrayHeader) + pHeader->dwCount * sizeof(SCellEntry);
			}

		default:
			ASSERT(false);
			return 0;
		}
	}

CDatum CAeonRow::GetValue (void)

//	GetValue
//
//	Returns the value of a 0D row

	{
	ASSERT(m_iDimCount == 0);
	SItemHeader *pValue = Get0DItem();
	return ItemToValue(pValue);
	}

CDatum CAeonRow::GetValue (int iIndex)

//	GetValue
//
//	For 1D rows we return the value of the nth sub-row
//	For 2D rows we return the value of the nth cell

	{
	switch (m_iDimCount)
		{
		case 0:
			ASSERT(iIndex == 0);
			return GetValue();

		case 1:
			{
			ASSERT(iIndex < GetCount());
			SCellEntry *pCell = Get1DCellEntry(iIndex);
			return ItemToValue(GetItemInVarBlock(pCell->dwValueOffset));
			}

		case 2:
			//	LATER
			ASSERT(false);
			return CDatum();

		default:
			ASSERT(false);
			return CDatum();
		}
	}

CDatum CAeonRow::GetValue (const CString &sKey)

//	GetValue
//
//	For 1D rows we return the value at the given key

	{
	ASSERT(m_iDimCount == 1);

	SCellEntry *pCell = Find1DCellEntry(sKey);
	if (pCell == NULL)
		return CDatum();
	
	return CDatum();
	}

CDatum CAeonRow::GetValue (const CString &sKeyX, const CString &sKeyY)

//	GetValue
//
//	For 2D rows we return the value of the given cell

	{
	ASSERT(m_iDimCount == 2);

	//	LATER
	ASSERT(false);

	return CDatum();
	}

void CAeonRow::Init (const TArray<SDimensionDesc> &Dims)

//	Init
//
//	Initializes a new row

	{
	ASSERT(Dims.GetCount() <= 2);
	ASSERT(m_pFixedBlock == NULL);
	ASSERT(m_pVarBlock == NULL);

	//	Initialize dimensions

	m_iDimCount = Dims.GetCount();
	m_Dims = Dims;

	//	Initialize buffers

	switch (m_iDimCount)
		{
		case 0:
			{
			m_dwFixedBlockAlloc = sizeof(SItemHeader) + sizeof(SItemHeader);
			m_pFixedBlock = new char [m_dwFixedBlockAlloc];

			SItemHeader *pHeader = (SItemHeader *)m_pFixedBlock;
			pHeader->dwSize = sizeof(SItemHeader);

			SItemHeader *pItem = (SItemHeader *)(&pHeader[1]);
			pItem->dwSize = 0;
			break;
			}

		case 1:
			{
			m_dwFixedBlockAlloc = sizeof(SArrayHeader) + DEFAULT_ROW_COUNT * sizeof(SCellEntry);
			m_pFixedBlock = new char [m_dwFixedBlockAlloc];

			SArrayHeader *pHeader = (SArrayHeader *)m_pFixedBlock;
			pHeader->dwSize = sizeof(SArrayHeader) - sizeof(DWORD);
			pHeader->dwCount = 0;

			m_dwVarBlockAlloc = DEFAULT_VAR_BLOCK_SIZE;
			m_pVarBlock = new char [m_dwVarBlockAlloc];
			SItemHeader *pVarHeader = (SItemHeader *)m_pVarBlock;
			pVarHeader->dwSize = 0;
			break;
			}

		case 2:
			//	LATER
			ASSERT(false);
			break;

		default:
			ASSERT(false);
		}
	}

void CAeonRow::Insert (const CString &sKey, CDatum dValue)

//	Insert
//
//	For 1D rows, inserts a new sub-row at the given key
//	(If the key already exists, we replace it)

	{
	ASSERT(m_iDimCount == 1);

	//	First look for this key

	int iPos;
	SCellEntry *pCell = Find1DCellEntry(sKey, &iPos);
	
	//	If we could not find the cell then we need to insert a new one

	if (pCell == NULL)
		{
		pCell = InsertSubRow(iPos);

		//	Add the key

		pCell->dwKeyOffset = AllocVarBlock((LPSTR)sKey, sKey.GetLength());
		}

	//	Serialize the datum

	CMemoryBuffer Buffer(4096);
	dValue.Serialize(CDatum::formatAEONScript, Buffer);

	//	Add the value

	pCell->dwValueOffset = AllocVarBlock(Buffer.GetPointer(), Buffer.GetLength());
	}

void CAeonRow::Insert (const CString &sKeyX, const CString &sKeyY, CDatum dValue)

//	Insert
//
//	For 2D rows, inserts a new cell at the given keys
//	(If a cell already exists, we replace it)

	{
	//	LATER
	ASSERT(false);
	}

CAeonRow::SCellEntry *CAeonRow::InsertSubRow (int iPos)

//	InsertSubRow
//
//	Inserts a new cell entry at the given position

	{
	int i;

	ASSERT(m_iDimCount != 0);
	ASSERT(iPos >= 0 && iPos <= (int)((SArrayHeader *)m_pFixedBlock)->dwCount);

	//	Do we need to reallocate?

	DWORD dwSize = GetFixedBlockSize();
	if (dwSize + sizeof(SCellEntry) > m_dwFixedBlockAlloc)
		{
		SArrayHeader *pHeader = (SArrayHeader *)m_pFixedBlock;

		//	Allocate a new block

		DWORD dwNewFixedBlockAlloc = sizeof(SArrayHeader) + 2 * pHeader->dwCount * sizeof(SCellEntry);
		void *pNewFixedBlock = new char [dwNewFixedBlockAlloc];

		//	Copy data over (up to insertion point)

		char *pSource = (char *)m_pFixedBlock;
		char *pInsert = pSource + (sizeof(SArrayHeader) + iPos * sizeof(SCellEntry));
		char *pDest = (char *)pNewFixedBlock;
		
		while (pSource < pInsert)
			*pDest++ = *pSource++;

		//	Skip new insertion point

		pDest += sizeof(SCellEntry);

		//	Copy remaining

		char *pEnd = pSource + (sizeof(SArrayHeader) + pHeader->dwCount * sizeof(SCellEntry));

		while (pSource < pEnd)
			*pDest++ = *pSource++;

		//	Swap

		delete m_pFixedBlock;
		m_pFixedBlock = pNewFixedBlock;
		m_dwFixedBlockAlloc = dwNewFixedBlockAlloc;
		}

	//	Otherwise, expand the array

	else
		{
		SArrayHeader *pHeader = (SArrayHeader *)m_pFixedBlock;

		//	Move cells after the insertion point up

		for (i = pHeader->dwCount; i > iPos; i--)
			*Get1DCellEntry(i) = *Get1DCellEntry(i - 1);
		}

	//	Increment counts and sizes

	SArrayHeader *pHeader = (SArrayHeader *)m_pFixedBlock;
	pHeader->dwSize += sizeof(SCellEntry);
	pHeader->dwCount += 1;

	//	Done

	return Get1DCellEntry(iPos);
	}

CDatum CAeonRow::ItemToValue (SItemHeader *pItem)

//	ItemToValue
//
//	Deserialize a value from the given item

	{
	CMemoryBuffer Buffer(&pItem[1], pItem->dwSize);

	CDatum dValue;
	if (!CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &dValue))
		throw CException(errFail);

	return dValue;
	}

void CAeonRow::Load (void)

//	Load
//
//	Loads a row from the current stream position

	{
	}

void CAeonRow::Save (void)

//	Save
//
//	Saves a row to the current stream position

	{
	}

void CAeonRow::SetValue (CDatum dValue)

//	SetValue
//
//	Sets the value of a 0D row

	{
	ASSERT(m_iDimCount == 0);

	CMemoryBuffer Buffer(4096);
	dValue.Serialize(CDatum::formatAEONScript, Buffer);

	//	Allocate a new block

	DWORD dwSizeUp = AlignUp(Buffer.GetLength(), (int)sizeof(DWORD));
	DWORD dwNewFixedBlockAlloc = sizeof(SItemHeader) + sizeof(SItemHeader) + dwSizeUp;
	void *pNewFixedBlock = new char [dwNewFixedBlockAlloc];

	//	Init

	SItemHeader *pHeader = (SItemHeader *)pNewFixedBlock;
	pHeader->dwSize = sizeof(SItemHeader) + dwSizeUp;

	SItemHeader *pItem = (SItemHeader *)&pHeader[1];
	pItem->dwSize = Buffer.GetLength();

	utlMemCopy(Buffer.GetPointer(), &pItem[1], Buffer.GetLength());

	//	Replace

	delete m_pFixedBlock;
	m_pFixedBlock = pNewFixedBlock;
	m_dwFixedBlockAlloc = dwNewFixedBlockAlloc;
	}
