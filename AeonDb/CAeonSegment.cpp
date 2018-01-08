//	CAeonSegment.cpp
//
//	CAeonSegment class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	A segment file looks like this: (SSegmentHeader)
//
//	DWORD		Signature 'AEOS'
//	DWORD		Version
//	DWORD		Sequence high
//	DWORD		Sequence low
//	DWORD		No. of entries in index (including last key)
//	DWORD		Offset to index region (from start of file)
//	DWORD		Size of index region (including var items)
//	DWORD		Total rows in segment
//	DWORD		ViewID
//	DWORD[7]	Spare
//	----------------------- blocks
//	for each block (SBlockHeader)
//	DWORD		Size of block 0
//	DWORD		Number of rows in block
//	
//	for each row (SBlockIndexEntry)
//	DWORD		offset to row key (from start of block)
//	DWORD		offset to row data (from start of block)
//	Optional SBlockIndexEntryExtra
//	
//	for each row
//	DWORD		row key size
//	BYTEs		padded to DWORD-align
//	...
//	DWORD		row data size
//	BYTEs		padded to DWORD-align
//	----------------------- index entries
//	for each block... (SIndexEntry)
//	DWORD		offset to key for block 0 (from start of index)
//	DWORD		offset to block 0 (from start of file)
//	DWORD		size of block 0
//	DWORD		No. of rows in block
//	...
//	
//	for each block...
//	DWORD		size of key for block 0
//	BYTEs		padded to DWORD-align
//	...

#include "stdafx.h"

const DWORD INPROGRESS_SIGNATURE = 'XXXX';
const DWORD SIGNATURE = 'SOEA';		//	'AEOS' backwards because of little-endianness
const DWORD CURRENT_VERSION = 1;
const int BLOCK_CACHE_SIZE = 64;

DECLARE_CONST_STRING(FIELD_BLOCK_INDEX,					"blockIndex")
DECLARE_CONST_STRING(FIELD_FILE_SIZE,					"fileSize")
DECLARE_CONST_STRING(FIELD_FILESPEC,					"filespec")
DECLARE_CONST_STRING(FIELD_KEY,							"key")
DECLARE_CONST_STRING(FIELD_MARKED_FOR_DELETE,			"markedForDelete")
DECLARE_CONST_STRING(FIELD_OFFSET_X,					"offset")
DECLARE_CONST_STRING(FIELD_ROW_COUNT,					"rowCount")
DECLARE_CONST_STRING(FIELD_SEQ,							"sequence")
DECLARE_CONST_STRING(FIELD_SIZE,						"size")

DECLARE_CONST_STRING(ERR_MUST_HAVE_ROWS,				"Unable to create a segment with no rows.")
DECLARE_CONST_STRING(ERR_CANT_CREATE_FILE,				"Unable to create segment file: %s.")
DECLARE_CONST_STRING(ERR_CANT_READ_SEGMENT,				"Unable to read segment: %s.")
DECLARE_CONST_STRING(ERR_CANT_WRITE_FILE,				"Unable to write segment: %s.")

DECLARE_CONST_STRING(ERR_GET_BLOCK_BY_ROW_POS,			"Crash in GetBlockByRowPos.")
DECLARE_CONST_STRING(ERR_LOAD_BLOCK,					"Crash in LoadBlock.")
DECLARE_CONST_STRING(ERR_ROW_VALUE,						"Crash loading row value.")
DECLARE_CONST_STRING(ERR_GET_DATA,						"Crash in GetData.")

CAeonSegment::CAeonSegment (void) : 
		m_Seq(0),
		m_dwDesiredBlockSize(64 * 1024),
		m_pHeader(NULL),
		m_pIndex(NULL),
		m_bMarkedForDelete(false)

//	CAeonSegment constructor

	{
	}

CAeonSegment::~CAeonSegment (void)

//	CAeonSegment destructor

	{
	if (m_pHeader)
		delete m_pHeader;

	if (m_pIndex)
		delete [] (char *)m_pIndex;

	if (m_bMarkedForDelete)
		Delete();
	}

bool CAeonSegment::BlockFindData (SBlockHeader *pBlock, const CString &sKey, CDatum *retData, SEQUENCENUMBER *retRowID)

//	BlockFindData
//
//	Finds the data at the given path

	{
	int iRow;
	if (!BlockFindRow(pBlock, sKey, &iRow))
		return false;

	//	Construct a row value

	CAeonRowValue RowValue;
	RowValue.Init(BlockGetValue(pBlock, iRow, retRowID));

	//	Done

	if (retData)
		*retData = RowValue.GetValue();

	return true;
	}

bool CAeonSegment::BlockFindRow (SBlockHeader *pBlock, const CString &sKey, int *retiPos)

//	BlockFindRow
//
//	Finds the position of the given row in the block.

	{
	int iCount = pBlock->dwRowCount;
	int iMin = 0;
	int iMax = iCount;
	int iTry = iMax / 2;

	while (true)
		{
		if (iMax <= iMin)
			{
			*retiPos = iMin;
			return false;
			}

		//	Get the block

		CString sTryKey = BlockGetKey(pBlock, iTry);
		int iCompare = CRowKey::Compare(m_Dims, CRowKey(m_Dims, sKey), CRowKey(m_Dims, sTryKey));

		if (iCompare == 0)
			{
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

CAeonSegment::SBlockIndexEntry *CAeonSegment::BlockGetIndexEntry (SBlockHeader *pBlock, int iPos)

//	BlockGetIndexEntry
//
//	Returns a pointer to the given block index entry (iPos is relative to the block).

	{
	BYTE *pPos = (BYTE *)pBlock;

	//	Skip header

	pPos += sizeof(SBlockHeader);

	//	Get the correct row

	int iEntrySize = GetBlockIndexEntrySize();
	pPos += iEntrySize * iPos;

	//	Done

	return (SBlockIndexEntry *)pPos;
	}

CString CAeonSegment::BlockGetKey (SBlockHeader *pBlock, int iPos)

//	BlockGetKey
//
//	Returns the key at the given position

	{
	SBlockIndexEntry *pEntry = BlockGetIndexEntry(pBlock, iPos);
	//	We advance by a DWORD because we point to the string (not the length)
	CStringBuffer Key(((char *)pBlock) + pEntry->dwKeyOffset + sizeof(DWORD));
	return Key;
	}

void *CAeonSegment::BlockGetValue (SBlockHeader *pBlock, int iPos, SEQUENCENUMBER *retRowID)

//	BlockGetValue
//
//	Returns a pointer to the stored value

	{
	SBlockIndexEntry *pEntry = BlockGetIndexEntry(pBlock, iPos);

	//	RowID, if requested

	if (retRowID)
		{
		ASSERT(HasRowID());

		SBlockIndexEntryExtra *pExtra = GetBlockIndexEntryExtra(pEntry);
		*retRowID = pExtra->RowID;
		}

	//	Done

	return ((char *)pBlock) + pEntry->dwValueOffset;
	}

bool CAeonSegment::Create (DWORD dwViewID, const CTableDimensions &Dims, SEQUENCENUMBER Seq, CRowIterator &Rows, const CString &sFilespec, DWORD dwFlags, CString *retsError)

//	Create
//
//	Creates a segment from a row iterator

	{
	int i;

	//	Make sure we have at least one row

	Rows.Reset();
	if (!Rows.HasMore())
		{
		*retsError = ERR_MUST_HAVE_ROWS;
		return false;
		}

	//	Initialize some stuff

	m_sFilespec = sFilespec;
	m_Seq = Seq;
	m_Dims = Dims;

	//	File to write

	CFile SegFile;

	//	Keep track of the blocks that we write out

	struct SIndexData
		{
		CString sKey;
		DWORD dwBlockOffset;
		DWORD dwBlockSize;
		DWORD dwRowCount;
		};

	TArray<SIndexData> NewBlocks;

	//	Open the file

	if (!SegFile.Create(m_sFilespec, CFile::FLAG_CREATE_ALWAYS))
		{
		*retsError = strPattern(ERR_CANT_CREATE_FILE, m_sFilespec);
		return false;
		}

	//	Allocate a block of memory for the header and index

	m_pHeader = new SSegmentHeader;

	//	Write out the header with an in-progress signature (we don't care yet about
	//	the other fields).

	utlMemSet(m_pHeader, sizeof(SSegmentHeader), 0);
	m_pHeader->dwSignature = INPROGRESS_SIGNATURE;
	m_pHeader->dwFlags = dwFlags;

	try
		{
		SegFile.Write(m_pHeader, sizeof(SSegmentHeader));
		}
	catch (...)
		{
		delete m_pHeader;
		m_pHeader = NULL;
		SegFile.Close();
		fileDelete(m_sFilespec);

		*retsError = strPattern(ERR_CANT_CREATE_FILE, m_sFilespec);
		return false;
		}

	//	Compute the size of each block index entry (this changes depending on
	//	the create flags).

	int iBlockIndexEntrySize = GetBlockIndexEntrySize();

	//	Now loop over every row in order and write out blocks

	while (Rows.HasMore())
		{
		bool bLastBlock = false;
		CString sLastKey;

		//	Start a new block

		SIndexData *pBlock = NewBlocks.Insert();
		pBlock->dwBlockOffset = SegFile.GetStreamLength();
		pBlock->dwBlockSize = 0;
		pBlock->dwRowCount = 0;

		//	Loop over writes to figure out how many rows will fit in this block.
		//	We use a copy of the iterator because we don't want to advance the
		//	real one yet.

		CRowIterator MeasureRows = Rows;
		while (MeasureRows.HasMore() && pBlock->dwBlockSize <= m_dwDesiredBlockSize)
			{
			//	If this is the first row, get the key so that we can remember
			//	the first key in the block.

			if (pBlock->dwRowCount == 0)
				pBlock->sKey = MeasureRows.GetKey();

			//	Increment the block size counter

			pBlock->dwRowCount++;
			pBlock->dwBlockSize += MeasureRows.GetNextRowSize();

			//	If we have no more rows then this will be the last block.
			//	Remember the last key.

			if (!MeasureRows.HasMore())
				bLastBlock = true;
			}

		//	Now that we know how many rows in the block we can calculate the
		//	size of the block header + index

		DWORD dwBlockHeaderSize = sizeof(SBlockHeader);
		DWORD dwBlockIndexSize = (pBlock->dwRowCount * iBlockIndexEntrySize);
		pBlock->dwBlockSize += dwBlockHeaderSize + dwBlockIndexSize;

		//	We're going to write to a temporary buffer first.

		CBuffer BlockBuffer;
		DWORD dwRowIndexPos = dwBlockHeaderSize;
		DWORD dwRowDataPos = dwBlockHeaderSize + dwBlockIndexSize;

		//	Loop over each row in the block writing out both the row index entry
		//	and the row data (at two different positions in the buffer).

		DWORD dwRowsLeft = pBlock->dwRowCount;
		while (Rows.HasMore() && dwRowsLeft > 0)
			{
			DWORD dwKeySize;
			DWORD dwDataSize;

			//	If this is the last row of the last block then get the last
			//	key.

			if (bLastBlock && dwRowsLeft == 1)
				sLastKey = Rows.GetKey();

			//	Sometimes we need the RowID

			SBlockIndexEntryExtra Extra;

			//	Write the row getting back the serialized size of the key
			//	and the data.
			//
			//	NOTE: We get the RowID here. The function relies on the fact
			//	that we don't ask for the RowID if the segment doesn't store it.

			BlockBuffer.Seek(dwRowDataPos);
			Rows.WriteNextRow(BlockBuffer, &dwKeySize, &dwDataSize, (HasRowID() ? &Extra.RowID : NULL));

			//	Write the row index

			SBlockIndexEntry RowIndex;
			RowIndex.dwKeyOffset = dwRowDataPos;
			RowIndex.dwValueOffset = dwRowDataPos + dwKeySize;

			BlockBuffer.Seek(dwRowIndexPos);
			BlockBuffer.Write(&RowIndex, sizeof(RowIndex));

			//	If we store RowIDs, write it out now

			if (HasRowID())
				BlockBuffer.Write(&Extra, sizeof(Extra));

			//	Increment the write positions

			dwRowIndexPos += iBlockIndexEntrySize;
			dwRowDataPos += dwKeySize + dwDataSize;

			//	Next

			dwRowsLeft--;
			}

		//	Now we can finish the block header

		SBlockHeader BlockHeader;
		BlockHeader.dwSize = dwRowDataPos;
		BlockHeader.dwRowCount = pBlock->dwRowCount;
		ASSERT(BlockHeader.dwSize == pBlock->dwBlockSize);

		BlockBuffer.Seek(0);
		BlockBuffer.Write(&BlockHeader, sizeof(BlockHeader));

		//	Now write out the block

		try
			{
			SegFile.Write(BlockBuffer);
			}
		catch (...)
			{
			delete m_pHeader;
			m_pHeader = NULL;
			SegFile.Close();
			fileDelete(m_sFilespec);

			*retsError = strPattern(ERR_CANT_WRITE_FILE, m_sFilespec);
			return false;
			}

		//	Keep incrementing the total rows

		m_pHeader->dwRowCount += pBlock->dwRowCount;

		//	If this was the last block, remember the last key

		if (bLastBlock)
			{
			SIndexData *pBlock = NewBlocks.Insert();
			pBlock->sKey = sLastKey;
			pBlock->dwBlockOffset = SegFile.GetStreamLength();
			pBlock->dwBlockSize = 0;
			pBlock->dwRowCount = 0;

			ASSERT(!Rows.HasMore());
			}
		}

	//	Now we need to generate the index. We write it out to a temporary
	//	buffer.

	CBuffer SegIndexBuffer;
	DWORD dwEntryPos = 0;
	DWORD dwDataPos = NewBlocks.GetCount() * sizeof(SIndexEntry);

	for (i = 0; i < NewBlocks.GetCount(); i++)
		{
		SIndexEntry Entry;
		Entry.dwKeyOffset = dwDataPos;
		Entry.dwBlockOffset = NewBlocks[i].dwBlockOffset;
		Entry.dwBlockSize = NewBlocks[i].dwBlockSize;
		Entry.dwRowCount = NewBlocks[i].dwRowCount;

		SegIndexBuffer.Seek(dwEntryPos);
		SegIndexBuffer.Write(&Entry, sizeof(Entry));

		//	Now write the key

		DWORD dwKeySize;
		SegIndexBuffer.Seek(dwDataPos);
		CAeonRowValue::SerializeKey(SegIndexBuffer, NewBlocks[i].sKey, &dwKeySize);

		//	Advance

		dwEntryPos += sizeof(Entry);
		dwDataPos += dwKeySize;
		}

	//	Write to the file

	try
		{
		SegFile.Write(SegIndexBuffer);
		}
	catch (...)
		{
		delete m_pHeader;
		m_pHeader = NULL;
		SegFile.Close();
		fileDelete(m_sFilespec);

		*retsError = strPattern(ERR_CANT_WRITE_FILE, m_sFilespec);
		return false;
		}

	//	Now we can go back and fill in the header properly

	m_pHeader->dwSignature = SIGNATURE;
	m_pHeader->dwVersion = CURRENT_VERSION;
	m_pHeader->Seq = Seq;
	m_pHeader->dwIndexCount = NewBlocks.GetCount();
	m_pHeader->dwIndexOffset = NewBlocks[NewBlocks.GetCount() - 1].dwBlockOffset;
	m_pHeader->dwIndexSize = SegIndexBuffer.GetLength();
	m_pHeader->dwViewID = dwViewID;

	try
		{
		SegFile.Seek(0);
		SegFile.Write(m_pHeader, sizeof(SSegmentHeader));
		}
	catch (...)
		{
		delete m_pHeader;
		m_pHeader = NULL;
		SegFile.Close();
		fileDelete(m_sFilespec);

		*retsError = strPattern(ERR_CANT_WRITE_FILE, m_sFilespec);
		return false;
		}

	//	Done writing the segment file

	SegFile.Close();

	//	Open the block cache

	if (!m_Blocks.Init(m_sFilespec, BLOCK_CACHE_SIZE))
		{
		delete m_pHeader;
		m_pHeader = NULL;
		SegFile.Close();
		fileDelete(m_sFilespec);

		*retsError = strPattern("Unable to open segment file: %s", m_sFilespec);
		return false;
		}

	//	Keep the index (because we keep the segment open)

	m_pIndex = (SIndexEntry *)SegIndexBuffer.GetHandoff();

#ifdef DEBUG_SEGMENTS
	for (i = 0; i < GetIndexCount(); i++)
		{
		SIndexEntry *pIndex = GetIndexEntry(i);
		CString sKey = IndexGetKey(pIndex);
		printf("Block: %d Key: %s\n", i, (LPSTR)CAeonTable::GetDimensionPath(m_Dims, sKey).AsString());

		if (pIndex->dwBlockSize > 0 && GetIndexCount() < 4)
			{
			SBlockHeader *pBlock;
			if (!m_Blocks.LoadBlock(pIndex->dwBlockOffset, pIndex->dwBlockSize, (void **)&pBlock))
				printf("Unable to load block.\n");

			for (int j = 0; j < (int)pIndex->dwRowCount; j++)
				{
				CString sKey = BlockGetKey(pBlock, j);
				printf("Row: %d Key: %s\n", j, (LPSTR)CAeonTable::GetDimensionPath(m_Dims, sKey).AsString());
				}

			m_Blocks.UnloadBlock(pIndex->dwBlockOffset);
			}

		printf("\n");
		}
#endif

	//	Done

	return true;
	}

CDatum CAeonSegment::DebugDump (void) const

//	DebugDump
//
//	Returns a structure as follows:
//
//	blockIndex: An array of structures:
//		key: The key for this block
//		offset: Offset of the block in the file
//		rowCount: The number of rows in the block
//		size: The size of the block in bytes
//	filespec: Filespec of the segment file
//	markedForDelete: True if marked for delete
//	seq: Sequence number

	{
	int i;

	CComplexStruct *pData = new CComplexStruct;

	pData->SetElement(FIELD_FILESPEC, m_sFilespec);
	pData->SetElement(FIELD_SEQ, m_Seq);
	if (m_bMarkedForDelete)
		pData->SetElement(FIELD_MARKED_FOR_DELETE, CDatum(CDatum::constTrue));

	pData->SetElement(FIELD_FILE_SIZE, fileGetSize(m_sFilespec));

	CComplexArray *pBlockIndex = new CComplexArray;
	for (i = 0; i < GetIndexCount(); i++)
		{
		CComplexStruct *pEntry = new CComplexStruct;
		const SIndexEntry *pIndex = GetIndexEntry(i);
		CString sKey = IndexGetKey(pIndex);
		CRowKey Key(m_Dims, sKey);

		pEntry->SetElement(FIELD_KEY, Key.AsDatum(m_Dims));
		pEntry->SetElement(FIELD_OFFSET_X, (int)pIndex->dwBlockOffset);
		pEntry->SetElement(FIELD_ROW_COUNT, (int)pIndex->dwRowCount);
		pEntry->SetElement(FIELD_SIZE, (int)pIndex->dwBlockSize);

		pBlockIndex->Append(CDatum(pEntry));
		}

	pData->SetElement(FIELD_BLOCK_INDEX, CDatum(pBlockIndex));

	return CDatum(pData);
	}

bool CAeonSegment::Delete (void)

//	Delete
//
//	Deletes the segment file

	{
	//	Terminate the block cache so that the file is closed

	m_Blocks.Term();

	//	Delete the file

	return fileDelete(m_sFilespec);
	}

bool CAeonSegment::FindData (const CRowKey &Key, CDatum *retData, SEQUENCENUMBER *retRowID)

//	FindData
//
//	Finds the data at the given path

	{
	//	Find the block that contains the key

	SIndexEntry *pEntry = GetBlockByKey(Key.AsEncodedString());
	if (pEntry == NULL)
		return false;

	//	Load the block (we throw on error)

	SBlockHeader *pBlock;
	m_Blocks.LoadBlock(pEntry->dwBlockOffset, pEntry->dwBlockSize, (void **)&pBlock);

	//	Find the key in the block

	bool bFound = BlockFindData(pBlock, Key.AsEncodedString(), retData, retRowID);

	//	Done

	m_Blocks.UnloadBlock(pEntry->dwBlockOffset);
	return bFound;
	}

bool CAeonSegment::FindKey (const CRowKey &Key, int *retiIndex)

//	FindKey
//
//	Finds the give key

	{
	int i;

	//	Find the block that contains the key

	int iBlockIndex;
	SIndexEntry *pEntry = GetBlockByKey(Key.AsEncodedString(), &iBlockIndex);
	if (pEntry == NULL)
		{
		if (retiIndex)
			*retiIndex = (iBlockIndex == 0 ? 0 : GetCount());

		return false;
		}

	//	Load the block (throw on error)

	SBlockHeader *pBlock;
	m_Blocks.LoadBlock(pEntry->dwBlockOffset, pEntry->dwBlockSize, (void **)&pBlock);

	//	Find the key in the block

	int iBlockPos;
	bool bFound = BlockFindRow(pBlock, Key.AsEncodedString(), &iBlockPos);

	//	Convert the position to a segment position

	for (i = 0; i < iBlockIndex; i++)
		iBlockPos += m_pIndex[i].dwRowCount;

	//	Done

	if (retiIndex)
		*retiIndex = iBlockPos;

	m_Blocks.UnloadBlock(pEntry->dwBlockOffset);
	return bFound;
	}

CAeonSegment::SIndexEntry *CAeonSegment::GetBlockByKey (const CString &sKey, int *retiPos)

//	GetBlockByKey
//
//	Returns the block index entry for the block that contains the given key.
//	(or NULL).

	{
	int iCount = GetIndexCount();
	int iMin = 0;
	int iMax = iCount;
	int iTry = iMax / 2;

	while (true)
		{
		if (iMax <= iMin)
			{
			//	If we're in one of the blocks, then return it

			if (iMin > 0 && iMin < iCount)
				{
				if (retiPos)
					*retiPos = iMin - 1;
				return GetIndexEntry(iMin - 1);
				}

			//	Otherwise, it is outside the range

			else
				{
				if (retiPos)
					{
					if (iMin <= 0)
						*retiPos = 0;
					else
						*retiPos = GetIndexCount();
					}

				return NULL;
				}
			}

		//	Get the block

		SIndexEntry *pTry = GetIndexEntry(iTry);
		CString sTryKey = IndexGetKey(pTry);
		int iCompare = CRowKey::Compare(m_Dims, CRowKey(m_Dims, sKey), CRowKey(m_Dims, sTryKey));

		if (iCompare == 0)
			{
			//	If this is the last block, then we return the second-to-last
			//	(This is because the last index entry represents the last key of the last block).

			if (iTry == iCount - 1)
				{
				if (retiPos)
					*retiPos = iTry - 1;
				return GetIndexEntry(iTry - 1);
				}
			else
				{
				if (retiPos)
					*retiPos = iTry;
				return pTry;
				}
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

CAeonSegment::SIndexEntry *CAeonSegment::GetBlockByRowPos (int iIndex, int *retiRowInBlock)

//	GetBlockByRowPos
//
//	Returns the block index entry for the block that contains the
//	given row by position.

	{
	int i;

	//	Find the block that has the given row

	for (i = 0; i < (int)m_pHeader->dwIndexCount; i++)
		{
		if (iIndex < (int)m_pIndex[i].dwRowCount)
			{
			if (retiRowInBlock)
				*retiRowInBlock = iIndex;

			return &m_pIndex[i];
			}

		iIndex -= m_pIndex[i].dwRowCount;
		}

	return NULL;
	}

CDatum CAeonSegment::GetData (int iIndex)

//	GetData
//
//	Returns the data at the given index

	{
	CSmartLock Lock(m_cs);

	bool bDiagGetBlockByPos = false;
	bool bDiagLoadBlock = false;
	bool bDiagRowValue = false;

	try
		{
		//	Figure out which block we should load

		int iRowPos;
		SIndexEntry *pEntry = GetBlockByRowPos(iIndex, &iRowPos);
		if (pEntry == NULL)
			//	LATER: Log something
			return CDatum();

		bDiagGetBlockByPos = true;

		//	Load the block (throw on error)

		SBlockHeader *pBlock;
		m_Blocks.LoadBlock(pEntry->dwBlockOffset, pEntry->dwBlockSize, (void **)&pBlock);
		bDiagLoadBlock = true;

		//	Construct a row value

		CAeonRowValue RowValue;
		RowValue.Init(BlockGetValue(pBlock, iRowPos));
		CDatum dValue = RowValue.GetValue();
		bDiagRowValue = true;

		//	Unload

		m_Blocks.UnloadBlock(pEntry->dwBlockOffset);

		//	Done

		return dValue;
		}
	catch (CFileException e)
		{
		throw;
		}
	catch (...)
		{
		if (!bDiagGetBlockByPos)
			throw CException(errProgrammerError, ERR_GET_BLOCK_BY_ROW_POS);
		else if (!bDiagLoadBlock)
			throw CException(errProgrammerError, ERR_LOAD_BLOCK);
		else if (!bDiagRowValue)
			throw CException(errProgrammerError, ERR_ROW_VALUE);
		else
			throw CException(errProgrammerError, ERR_GET_DATA);
		}
	}

bool CAeonSegment::GetInfo (const CString &sFilespec, SInfo *retInfo)

//	GetInfo
//
//	Gets information on the segment. Returns FALSE if error.

	{
	//	Load the header

	CFile File;
	SSegmentHeader Header;

	//	Open file

	if (!File.Create(sFilespec, CFile::FLAG_OPEN_READ_ONLY))
		return false;

	//	Read

	try
		{
		File.Read(&Header, sizeof(SSegmentHeader));
		}
	catch (...)
		{
		return false;
		}

	//	Make sure we have the right signature

	if (Header.dwSignature != SIGNATURE)
		return false;

	//	Get some info

	retInfo->Seq = Header.Seq;
	retInfo->dwRowCount = Header.dwRowCount;

	return true;
	}

CString CAeonSegment::GetKey (int iIndex)

//	GetKey
//
//	Returns the key at the given index

	{
	CSmartLock Lock(m_cs);

	//	Figure out which block we should load

	int iRowPos;
	SIndexEntry *pEntry = GetBlockByRowPos(iIndex, &iRowPos);
	if (pEntry == NULL)
		//	LATER: Log something
		return NULL_STR;

	//	If the row position is 0, then it means that it is one
	//	of the keys in the block index, so we can return that
	//	without loading the whole block.

	if (iRowPos == 0)
		return IndexGetKey(pEntry);

	//	Load the block (throw on error)

	SBlockHeader *pBlock;
	m_Blocks.LoadBlock(pEntry->dwBlockOffset, pEntry->dwBlockSize, (void **)&pBlock);

	//	Get the key from the block

	CString sKey = BlockGetKey(pBlock, iRowPos);

	//	Done

	m_Blocks.UnloadBlock(pEntry->dwBlockOffset);
	return sKey;
	}

bool CAeonSegment::GetRow (int iIndex, CRowKey *retKey, CDatum *retData, SEQUENCENUMBER *retRowID)

//	GetRow
//
//	Returns the row by index

	{
	CSmartLock Lock(m_cs);

	//	Figure out which block we should load

	int iRowPos;
	SIndexEntry *pEntry = GetBlockByRowPos(iIndex, &iRowPos);
	if (pEntry == NULL)
		//	LATER: Log something
		return false;

	//	Load the block (throw on error)

	SBlockHeader *pBlock;
	m_Blocks.LoadBlock(pEntry->dwBlockOffset, pEntry->dwBlockSize, (void **)&pBlock);

	//	Get the key

	if (retKey)
		{
		CString sKey = BlockGetKey(pBlock, iRowPos);
		CRowKey::CreateFromEncodedKey(m_Dims, sKey, retKey);
		}

	//	Get the data (and rowID)

	CAeonRowValue RowValue;
	RowValue.Init(BlockGetValue(pBlock, iRowPos, retRowID));

	if (retData)
		*retData = RowValue.GetValue();
	
	//	Unload

	m_Blocks.UnloadBlock(pEntry->dwBlockOffset);

	//	Done

	return true;
	}

DWORD CAeonSegment::GetRowSize (int iIndex)

//	GetRowSize
//
//	Returns the serialized size of the given row

	{
	CSmartLock Lock(m_cs);

	//	Figure out which block we should load

	int iRowPos;
	SIndexEntry *pEntry = GetBlockByRowPos(iIndex, &iRowPos);
	if (pEntry == NULL)
		//	LATER: Log something
		return 0;

	//	Load the block (throw on error)

	SBlockHeader *pBlock;
	m_Blocks.LoadBlock(pEntry->dwBlockOffset, pEntry->dwBlockSize, (void **)&pBlock);

	//	Construct a row value

	CAeonRowValue RowValue;
	RowValue.Init(BlockGetValue(pBlock, iRowPos));
	DWORD dwSize = RowValue.GetSerializedSize();;

	//	Unload

	m_Blocks.UnloadBlock(pEntry->dwBlockOffset);

	//	Done

	return dwSize;
	}

CString CAeonSegment::IndexGetKey (const SIndexEntry *pIndex) const

//	IndexGetKey
//
//	Returns the key for this index entry

	{
	//	We advance by a DWORD because we point to the string (not the length)
	CStringBuffer Key(((char *)m_pIndex) + pIndex->dwKeyOffset + sizeof(DWORD));
	return Key;
	}

bool CAeonSegment::Open (const CString &sFilespec, CString *retsError)

//	Open
//
//	Opens a segment and loads the index

	{
	//	Open file

	CFile File;
	if (!File.Create(sFilespec, CFile::FLAG_OPEN_READ_ONLY))
		{
		*retsError = strPattern("Unable to open segment file: %s", sFilespec);
		return false;
		}

	//	Load the header and index

	try
		{
		//	Read the header

		m_pHeader = new SSegmentHeader;

		File.Read(m_pHeader, sizeof(SSegmentHeader));

		//	Make sure we have the right signature

		if (m_pHeader->dwSignature != SIGNATURE)
			throw 1;

		//	Now that we know the size of the index, allocate some memory

		m_pIndex = (SIndexEntry *)new char [m_pHeader->dwIndexSize];

		//	Read the index

		File.Seek(m_pHeader->dwIndexOffset);
		File.Read(m_pIndex, m_pHeader->dwIndexSize);
		}
	catch (...)
		{
		if (m_pHeader)
			{
			delete m_pHeader;
			m_pHeader = NULL;
			}

		if (m_pIndex)
			{
			delete [] (char *)m_pIndex;
			m_pIndex = NULL;
			}

		*retsError = strPattern("Unable to open segment file: %s", sFilespec);
		return false;
		}

	//	Init some stuff

	m_sFilespec = sFilespec;
	m_Seq = m_pHeader->Seq;

	//	NOTE: We rely on our caller to set m_Dims when it figures out what view
	//	we belong to.

	//	Init the cache

	if (!m_Blocks.Init(sFilespec, BLOCK_CACHE_SIZE))
		{
		delete m_pHeader;
		m_pHeader = NULL;
		delete [] (char *)m_pIndex;
		m_pIndex = NULL;
		*retsError = strPattern("Unable to initialize block cache");
		return false;
		}

	//	Done

	return true;
	}

void CAeonSegment::WriteData (IByteStream &Stream, int iIndex, DWORD *retdwSize, SEQUENCENUMBER *retRowID)

//	WriteData
//
//	Writes the row value to the stream.

	{
	CSmartLock Lock(m_cs);

	//	Figure out which block we should load

	int iRowPos;
	SIndexEntry *pEntry = GetBlockByRowPos(iIndex, &iRowPos);
	if (pEntry == NULL)
		//	LATER: Log something
		return;

	//	Load the block (throw on error)

	SBlockHeader *pBlock;
	m_Blocks.LoadBlock(pEntry->dwBlockOffset, pEntry->dwBlockSize, (void **)&pBlock);

	//	Construct a row value

	CAeonRowValue RowValue;
	RowValue.Init(BlockGetValue(pBlock, iRowPos, retRowID));
	RowValue.Serialize(Stream);

	//	Get the size

	if (retdwSize)
		*retdwSize = RowValue.GetSerializedSize();

	//	Unload

	m_Blocks.UnloadBlock(pEntry->dwBlockOffset);
	}

