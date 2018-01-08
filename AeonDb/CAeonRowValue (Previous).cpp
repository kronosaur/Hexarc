//	CAeonRowValue.cpp
//
//	CAeonRowValue class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	The row is stored as follows (depending on the dimension)
//
//	0D ROW
//	----------------------------------------------------------------------------
//	DWORD	total size of the row, excluding this variable
//	DWORD	size of datum serialization (excluding padding)
//	BYTEs	datum serialization
//	...
//	BYTEs	padding so row is DWORD-aligned
//
//	1D ROW
//	----------------------------------------------------------------------------
//	DWORD	total size of the row, excluding this variable
//	DWORD	number of sub-rows
//
//	for each sub-row
//	DWORD	offset to sub-row key
//	DWORD	offset to sub-row data
//	...
//
//	start of var block
//	DWORD	total size of var block
//	DWORD	size of item 0
//	BYTEs	item bytes (either a key or datum serialization)
//	...
//	BYTEs	padding so item is DWORD-aligned
//	
//	DWORD	size of item 1
//	...
//
//		Note: offsets are always from start of var block
//		Note: List of sub-rows is sorted by key
//
//	2D ROW
//	----------------------------------------------------------------------------
//	DWORD	total size of the row, excluding this variable
//	DWORD	number of sub-rows
//
//	for each sub-row
//	DWORD	offset to sub-row key
//	DWORD	offset to sub-row struct
//	...
//
//	start of var block
//	DWORD	total size of var block
//	some items are strings; others are sub-row structs
//	DWORD	size of item 0
//	DWORD	number of cells
//
//	for each cell
//	DWORD	offset to cell key
//	DWORD	offset to cell data
//	...
//
//	DWORD	size of item 1

#include "stdafx.h"

const int DEFAULT_VAR_BLOCK_SIZE =					4096;
const int DEFAULT_ROW_COUNT =						16;

CAeonRowValue::CAeonRowValue (void) : m_iDimCount(-1),
		m_dwFixedBlockAlloc(0),
		m_pFixedBlock(NULL),
		m_dwVarBlockAlloc(0),
		m_pVarBlock(NULL)

//	CAeonRowValue constructor

	{
	}

CAeonRowValue::~CAeonRowValue (void)

//	CAeonRowValue destructor

	{
	CleanUp();
	}

DWORD CAeonRowValue::AllocVarBlock (void *pSource, int iLen, bool bIsString)

//	AllocVarBlock
//
//	Allocates a new item entry

	{
	//	Compute some sizes. If this is a string, then we increment
	//	to account for NULL terminator.

	int iActualLen = (bIsString ? iLen + 1 : iLen);
	DWORD dwSizeUp = AlignUp((DWORD)iActualLen, (DWORD)sizeof(DWORD));
	DWORD dwSizeNeeded = sizeof(SItemHeader) + dwSizeUp;

	DWORD dwCurrentSize = GetVarBlockSize();

	//	Do we need to reallocate?

	SItemHeader *pHeader = (SItemHeader *)m_pVarBlock;
	if (dwCurrentSize + dwSizeNeeded > m_dwVarBlockAlloc)
		{
		DWORD dwNewVarBlockAlloc = AlignUp(m_dwVarBlockAlloc + dwSizeNeeded, (DWORD)DEFAULT_VAR_BLOCK_SIZE);
		void *pNewVarBlock = new char [dwNewVarBlockAlloc];
		utlMemCopy(m_pVarBlock, pNewVarBlock, m_dwVarBlockAlloc);

		if (m_pVarBlock && m_dwVarBlockAlloc)
			delete m_pVarBlock;
		m_pVarBlock = pNewVarBlock;
		m_dwVarBlockAlloc = dwNewVarBlockAlloc;

		pHeader = (SItemHeader *)m_pVarBlock;
		}

	//	Initialize the new item

	SItemHeader *pNew = GetItemInVarBlock(dwCurrentSize);
	pNew->dwSize = iLen;	//	Does not include NULL terminator

	//	Copy the buffer

	utlMemCopy(pSource, &pNew[1], iActualLen);

	//	Increment the sizes, etc.

	pHeader->dwSize += dwSizeNeeded;

	//	Done

	return dwCurrentSize;
	}

void CAeonRowValue::CleanUp (void)

//	CleanUp
//
//	Deletes all allocations

	{
	if (m_pFixedBlock && m_dwFixedBlockAlloc)
		{
		delete m_pFixedBlock;
		m_pFixedBlock = NULL;
		m_dwFixedBlockAlloc = 0;
		}

	if (m_pVarBlock && m_dwVarBlockAlloc)
		{
		delete m_pVarBlock;
		m_pVarBlock = NULL;
		m_dwVarBlockAlloc = 0;
		}
	}

void CAeonRowValue::Copy (const CAeonRowValue &Src)

//	Copy
//
//	Assumes that we are clean

	{
	ASSERT(Src.m_iDimCount != -1);

	m_iDimCount = Src.m_iDimCount;
	m_Dims = Src.m_Dims;

	if (Src.m_pFixedBlock)
		{
		m_dwFixedBlockAlloc = Src.m_dwFixedBlockAlloc;
		m_pFixedBlock = new char [m_dwFixedBlockAlloc];
		utlMemCopy(Src.m_pFixedBlock, m_pFixedBlock, GetFixedBlockSize());
		}
	else
		{
		m_pFixedBlock = NULL;
		m_dwFixedBlockAlloc = 0;
		}

	if (Src.m_pVarBlock)
		{
		m_dwVarBlockAlloc = Src.m_dwVarBlockAlloc;
		m_pVarBlock = new char [m_dwVarBlockAlloc];
		utlMemCopy(Src.m_pVarBlock, m_pVarBlock, GetVarBlockSize());
		}
	else
		{
		m_pVarBlock = NULL;
		m_dwVarBlockAlloc = 0;
		}
	}

CAeonRowValue::SCellEntry *CAeonRowValue::FindCellEntry (SCellEntry *pIndex, int iIndexCount, const SDimensionDesc &Desc, const CString &sKey, int *retiPos)

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

		CStringBuffer sKeyTry = ItemToKey(GetItemInVarBlock(pIndex[iTry].dwKeyOffset));
		int iCompare = IOrderedRowSet::CompareKeys(Desc, sKey, sKeyTry);
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

bool CAeonRowValue::FindValue (const SDimensionPath &Path, CDatum *retData)

//	FindValue
//
//	Looks for the value

	{
	if (Path.iDimCount != m_iDimCount + 1)
		return false;

	switch (m_iDimCount)
		{
		case 0:
			if (retData)
				*retData = GetValue();
			return true;

		case 1:
			{
			SCellEntry *pCell = Find1DCellEntry(Path.sKey[1]);
			if (pCell == NULL)
				return false;

			if (retData)
				*retData = ItemToValue(GetItemInVarBlock(pCell->dwValueOffset));
			return true;
			}

		case 2:
			{
			//	LATER
			ASSERT(false);
			return false;
			}

		default:
			//	Should never get here.
			ASSERT(false);
			return false;
		}
	}

bool CAeonRowValue::FindValue (const CString &sKey, CDatum *retData)

//	FindValue
//
//	Looks for data in a 1D row

	{
	ASSERT(m_iDimCount == 1);

	SCellEntry *pCell = Find1DCellEntry(sKey);
	if (pCell == NULL)
		return false;

	*retData = ItemToValue(GetItemInVarBlock(pCell->dwValueOffset));
	return true;
	}

int CAeonRowValue::GetCount (void)

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

DWORD CAeonRowValue::GetFixedBlockSize (void)

//	GetFixedBlockSize
//
//	Returns the current size of the fixed block

	{
	switch (m_iDimCount)
		{
		case -1:
			return 0;

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

int CAeonRowValue::GetSerializedSize (void)

//	GetSerializedSize
//
//	Returns the serialized size of the value

	{
	return GetFixedBlockSize() + GetVarBlockSize();
	}

CDatum CAeonRowValue::GetValue (void)

//	GetValue
//
//	Returns the value of a 0D row

	{
	switch (m_iDimCount)
		{
		case 0:
			{
			SItemHeader *pValue = Get0DItem();
			return ItemToValue(pValue);
			}

		default:
			//	LATER?
			//	May want to deprecate (or delete) multi-valued rows
			//	(and instead implement dimensions above the row level).
			ASSERT(false);
			return CDatum();
		}
	}

CDatum CAeonRowValue::GetValue (int iIndex)

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

CDatum CAeonRowValue::GetValue (const CString &sKey)

//	GetValue
//
//	For 1D rows we return the value at the given key

	{
	CDatum dValue;
	if (!FindValue(sKey, &dValue))
		return CDatum();

	return dValue;
	}

CDatum CAeonRowValue::GetValue (const CString &sKeyX, const CString &sKeyY)

//	GetValue
//
//	For 2D rows we return the value of the given cell

	{
	ASSERT(m_iDimCount == 2);

	//	LATER
	ASSERT(false);

	return CDatum();
	}

DWORD CAeonRowValue::GetVarBlockSize (void)

//	GetVarBlockSize
//
//	Returns the size of the var block

	{
	if (m_pVarBlock == NULL)
		return 0;

	SItemHeader *pHeader = (SItemHeader *)m_pVarBlock;
	return sizeof(SItemHeader) + pHeader->dwSize;
	}

void CAeonRowValue::Init (const CTableDimensions &Dims, void *pValue)

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

	//	If buffers are passed in, use those

	if (pValue)
		{
		m_pFixedBlock = pValue;
		m_dwFixedBlockAlloc = 0;

		//	The variable block follows

		m_pVarBlock = ((char *)m_pFixedBlock) + GetFixedBlockSize();
		m_dwVarBlockAlloc = 0;

		return;
		}

	//	Otherwise, initialize with allocated buffers

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

void CAeonRowValue::Insert (const CString &sKey, CDatum dValue)

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

		pCell->dwKeyOffset = AllocVarBlock(sKey.GetParsePointer(), sKey.GetLength(), true);
		}

	//	Serialize the datum

	CMemoryBuffer Buffer(4096);
	dValue.Serialize(CDatum::formatAEONScript, Buffer);

	//	Add the value

	pCell->dwValueOffset = AllocVarBlock(Buffer.GetPointer(), Buffer.GetLength());
	}

void CAeonRowValue::Insert (const CString &sKeyX, const CString &sKeyY, CDatum dValue)

//	Insert
//
//	For 2D rows, inserts a new cell at the given keys
//	(If a cell already exists, we replace it)

	{
	//	LATER
	ASSERT(false);
	}

CAeonRowValue::SCellEntry *CAeonRowValue::InsertSubRow (int iPos)

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

		if (m_pFixedBlock && m_dwFixedBlockAlloc)
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

CDatum CAeonRowValue::ItemToValue (SItemHeader *pItem)

//	ItemToValue
//
//	Deserialize a value from the given item

	{
	CMemoryBuffer Buffer(&pItem[1], pItem->dwSize);

	CDatum dValue;
	if (!CDatum::Deserialize(CDatum::formatAEONScript, Buffer, &dValue))
		//	LATER: Need to handle this better
		throw CException(errFail);

	return dValue;
	}

void CAeonRowValue::Load (void)

//	Load
//
//	Loads a row from the current stream position

	{
	}

void CAeonRowValue::Save (void)

//	Save
//
//	Saves a row to the current stream position

	{
	}

void CAeonRowValue::Serialize (IByteStream &Stream)

//	Serialize
//
//	Serialize the row (we always write out aligned to DWORD)

	{
	if (m_pFixedBlock)
		Stream.Write(m_pFixedBlock, GetFixedBlockSize());

	if (m_pVarBlock)
		Stream.Write(m_pVarBlock, GetVarBlockSize());
	}

void CAeonRowValue::SetValue (CDatum dValue)

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

	if (m_pFixedBlock && m_dwFixedBlockAlloc)
		delete m_pFixedBlock;
	m_pFixedBlock = pNewFixedBlock;
	m_dwFixedBlockAlloc = dwNewFixedBlockAlloc;
	}
