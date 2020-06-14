//	CAeonRowValue.cpp
//
//	CAeonRowValue class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	The row is stored as follows
//
//	DWORD	total size of the row, excluding this variable
//	DWORD	size of datum serialization (excluding padding)
//	BYTEs	datum serialization
//	...
//	BYTEs	padding so row is DWORD-aligned

#include "stdafx.h"

const int DEFAULT_VAR_BLOCK_SIZE =					4096;
const int DEFAULT_ROW_COUNT =						16;

CAeonRowValue::~CAeonRowValue (void)

//	CAeonRowValue destructor

	{
	CleanUp();
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
	}

void CAeonRowValue::Copy (const CAeonRowValue &Src)

//	Copy
//
//	Assumes that we are clean

	{
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
	}

bool CAeonRowValue::FindValue (const CRowKey &Path, CDatum *retData)

//	FindValue
//
//	Looks for the value

	{
	if (retData)
		*retData = GetValue();
	return true;
	}

DWORD CAeonRowValue::GetFixedBlockSize (void)

//	GetFixedBlockSize
//
//	Returns the current size of the fixed block

	{
	SItemHeader *pHeader = (SItemHeader *)m_pFixedBlock;
	return sizeof(SItemHeader) + pHeader->dwSize;
	}

int CAeonRowValue::GetSerializedSize (void)

//	GetSerializedSize
//
//	Returns the serialized size of the value

	{
	return GetFixedBlockSize();
	}

CDatum CAeonRowValue::GetValue (void)

//	GetValue
//
//	Returns the value of a 0D row

	{
	SItemHeader *pValue = Get0DItem();
	return ItemToValue(pValue);
	}

void CAeonRowValue::Init (void *pValue)

//	Init
//
//	Initializes a new row

	{
	ASSERT(m_pFixedBlock == NULL);

	//	If buffers are passed in, use those

	if (pValue)
		{
		m_pFixedBlock = pValue;
		m_dwFixedBlockAlloc = 0;
		}

	//	Otherwise, initialize with allocated buffers

	else
		{
		m_dwFixedBlockAlloc = sizeof(SItemHeader) + sizeof(SItemHeader);
		m_pFixedBlock = new char [m_dwFixedBlockAlloc];

		SItemHeader *pHeader = (SItemHeader *)m_pFixedBlock;
		pHeader->dwSize = sizeof(SItemHeader);

		SItemHeader *pItem = (SItemHeader *)(&pHeader[1]);
		pItem->dwSize = 0;
		}
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

bool CAeonRowValue::IsNil (void)

//	IsNil
//
//	Returns TRUE if the value is nil

	{
	SItemHeader *pValue = Get0DItem();

	//	We expect the characters "nil"

	if (pValue->dwSize != 3)
		return false;

	char *pPos = (char *)(&pValue[1]);
	return (pPos[0] == 'n' && pPos[1] == 'i' && pPos[2] == 'l');
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
	}

DWORD CAeonRowValue::GetSerializedKeySize (const CString &sKey)

//	GetSerializedKeySize
//
//	Computes the serialized size of a key.

	{
	return AlignUp((int)sizeof(DWORD) + sKey.GetLength() + 1, (int)sizeof(DWORD)); 
	}

void CAeonRowValue::SerializeKey (IByteStream &Stream, const CString &sKey, DWORD *retdwSize)

//	SerializeKey
//
//	Serializes a key and returns the serialized size.
//
//	DWORD	key size
//	BYTES[]	key (padded to DWORD align)

	{
	DWORD dwKeySize = sKey.GetLength();

	//	Write out the key length

	Stream.Write(&dwKeySize, sizeof(DWORD));

	//	Write out the key (add 1 for NULL termination)

	Stream.Write(sKey.GetParsePointer(), dwKeySize + 1);

	//	Save padding

	DWORD dwZero = 0;
	DWORD dwAlignedKeySize = AlignUp(dwKeySize + 1, (DWORD)sizeof(DWORD));
	Stream.Write(&dwZero, dwAlignedKeySize - (dwKeySize + 1));

	//	Done

	if (retdwSize)
		*retdwSize = sizeof(DWORD) + dwAlignedKeySize;
	}

void CAeonRowValue::SetValue (CDatum dValue)

//	SetValue
//
//	Sets the value of a 0D row

	{
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
