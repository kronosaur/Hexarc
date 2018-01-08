//	CHexeCodeIntermediate.cpp
//
//	CHexeCodeIntermeidate class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

int CHexeCodeIntermediate::CreateCodeBlock (void)

//	CreateCodeBlock
//
//	Allocates a new code block

	{
	int iID = m_CodeBlocks.GetCount();
	m_CodeBlocks.Insert();
	return iID;
	}

int CHexeCodeIntermediate::CreateDatumBlock (CDatum dDatum)

//	CreateDatumBlock
//
//	Allocates a new datum block

	{
	int i;

	//	If this is a string see if we've already added the same string

	if (dDatum.GetBasicType() == CDatum::typeString)
		{
		for (i = 0; i < m_DatumBlocks.GetCount(); i++)
			if (strEquals(dDatum, m_DatumBlocks[i]))
				return i;
		}

	//	Add a new datum

	int iID = m_DatumBlocks.GetCount();
	m_DatumBlocks.Insert(dDatum);
	return iID;
	}

void CHexeCodeIntermediate::RewriteShortOpCode (int iBlock, int iPos, OPCODE opCode, DWORD dwOperand)

//	RewriteShortOpCode
//
//	Writes to a previously written position

	{
	int iOldPos = m_CodeBlocks[iBlock].GetPos();
	m_CodeBlocks[iBlock].Seek(iPos);
	WriteShortOpCode(iBlock, opCode, dwOperand);
	m_CodeBlocks[iBlock].Seek(iOldPos);
	}

void CHexeCodeIntermediate::WriteShortOpCode (int iBlock, OPCODE opCode, DWORD dwOperand)

//	WriteShortOpCode
//
//	Writes a short opcode to the given block

	{
	DWORD dwOpCode = (opCode | GetOperand(dwOperand));
	m_CodeBlocks[iBlock].Write(&dwOpCode, sizeof(DWORD));
	}

void CHexeCodeIntermediate::WriteLongOpCode (int iBlock, OPCODE opCode, DWORD dwData)

//	WriteLongOpCode
//
//	Writes a long opcode to the given block

	{
	DWORD dwOpCode = opCode;
	m_CodeBlocks[iBlock].Write(&dwOpCode, sizeof(DWORD));
	m_CodeBlocks[iBlock].Write(&dwData, sizeof(DWORD));
	}
