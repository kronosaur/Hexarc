//	CDatumBlock.cpp
//
//	CDatumBlock class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

static TAllocatorGC<DWORD> g_IntAlloc;
static TAllocatorGC<double> g_DoubleAlloc;

void CDatumBlock::ClearMark (void)

//	ClearMark
//
//	Clear the in-use mark

	{
	//	Clear the mark on the datum itself

	m_pNext = (CDatumBlock *)(((DWORD_PTR)m_pNext) & ~AEON_MARK_MASK);

	//	No need to clear m_pNext or nested datum blocks because
	//	this function is always called from CCortex::Sweep, which
	//	iterates over all datum blocks.
	}

double CDatumBlock::GetDouble (void) const

//	GetDouble
//
//	Returns double if NUMBER_DOUBLE

	{
	return g_DoubleAlloc.Get(GetNumberIndex());
	}

int CDatumBlock::GetExtInteger (void) const

//	GetExtInteger
//
//	Returns integer if NUMBER_32BIT

	{
	return (int)g_IntAlloc.Get(GetNumberIndex());
	}

CDatum::Types CDatumBlock::GetType (void) const

//	GetType
//
//	Returns the type

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return CDatum::typeString;

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData & AEON_NUMBER_MASK)
						{
						case AEON_CONST_NIL:
							return CDatum::typeNil;

						case AEON_CONST_TRUE:
							return CDatum::typeTrue;

						default:
							ASSERT(false);
							return CDatum::typeUnknown;
						}
					}

				case AEON_NUMBER_28BIT:
				case AEON_NUMBER_32BIT:
					return CDatum::typeInteger;

				case AEON_NUMBER_DOUBLE:
					return CDatum::typeDouble;

				default:
					ASSERT(false);
					return CDatum::typeUnknown;
				}

		case AEON_TYPE_LIST:
			return CDatum::typeList;

		case AEON_TYPE_COMPLEX:
			return GetComplex()->GetType();

		default:
			ASSERT(false);
			return CDatum::typeUnknown;
		}
	}

void CDatumBlock::Mark (void)

//	Mark
//
//	Mark as being in-use. We need to mark both the datum and
//	the object that we point to (if any).

	{
	//	We mark the datum by setting the low bit on the m_pNext pointer.
	//	Note that our callers must be aware of this (and call GetNextInSweep
	//	instead of GetNext).

	m_pNext = (CDatumBlock *)(((DWORD_PTR)m_pNext) | AEON_MARK_MASK);

	//	Depending on our type, we mark recursively

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_LIST:
			GetList()->Mark();
			break;

		case AEON_TYPE_COMPLEX:
			//	LATER: Mark object
			break;
		}

	//	Mark m_pNext objects

	CDatumBlock *pNext = GetNextInSweep();
	while (pNext)
		{
		pNext->Mark();
		pNext = pNext->GetNextInSweep();
		}
	}

CDatumBlock *CDatumBlock::SetDouble (double rValue)

//	SetDouble
//
//	Sets a double

	{
	ASSERT(m_dwData == AEON_CONST_FREE);

	DWORD dwID = g_DoubleAlloc.New(rValue);
	m_dwData = (dwID << 4) | AEON_NUMBER_DOUBLE;
	return this;
	}

void CDatumBlock::SetFree (void)

//	SetFree
//
//	Frees any associated objects and marks the datum as free

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_NUMBER:
			{
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_32BIT:
					g_IntAlloc.Delete(GetNumberIndex());
					break;

				case AEON_NUMBER_DOUBLE:
					g_IntAlloc.Delete(GetNumberIndex());
					break;
				}
			break;
			}

		case AEON_TYPE_STRING:
			CString *pString = (CString *)&m_dwData;
			*pString = CString();
			break;

		case AEON_TYPE_LIST:
			//	No need to do anything since we're pointing to a
			//	CDatumBlock (which will be freed as a datum)
			break;

		case AEON_TYPE_COMPLEX:
			//	LATER: Delete object
			break;
		}

	m_dwData = AEON_CONST_FREE;
	}

CDatumBlock *CDatumBlock::SetInteger (int iValue)

//	SetInteger
//
//	Sets an integer

	{
	ASSERT(m_dwData == AEON_CONST_FREE);

	DWORD dwValue = (DWORD)iValue;

	//	If we fit in 28-bit range, then store in m_dwData

	if (dwValue >= AEON_MIN_28BIT || dwValue <= AEON_MAX_28BIT)
		m_dwData = ((dwValue << 4) | AEON_NUMBER_28BIT);

	//	Otherwise, we need to store the number elsewhere

	else
		{
		DWORD dwID = g_IntAlloc.New(dwValue);
		m_dwData = (dwID << 4) | AEON_NUMBER_32BIT;
		}

	return this;
	}

CDatumBlock *CDatumBlock::SetList (CDatumBlock *pHead)

//	SetList
//
//	Sets a list

	{
	ASSERT(m_dwData == AEON_CONST_FREE);

	m_dwData = ((DWORD_PTR)pHead) | AEON_TYPE_LIST;

	return this;
	}

CDatumBlock *CDatumBlock::SetString (const CString &sString)

//	SetString
//
//	Sets a string

	{
	ASSERT(m_dwData == AEON_CONST_FREE);

	//	Convert m_dwData into a CString

	m_dwData = 0;
	CString *pString = (CString *)&m_dwData;

	//	Copy the input string and take a handoff

	CString sNewString(sString);
	pString->TakeHandoff(sNewString);

	//	No need to mark the type (because TYPE_STRING is 0)

	ASSERT(AEON_TYPE_STRING == 0x00);

	//	Done

	return this;
	}
