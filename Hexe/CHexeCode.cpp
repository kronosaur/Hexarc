//	CHexeCode.cpp
//
//	CHexeCode class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	FORMAT
//
//	The m_Code buffer is an array of DWORDs divided into blocks. Each block looks
//	like this:
//
//	DWORD	blockHeader
//	data...
//
//	The 4 high-order bits in the blockHeader encode the block type. The rest of the bits
//	encode the length of the block (including the block header) in bytes.
//
//	The data that follows depends on the type:
//
//	blockCode:
//
//		DWORD	blockHeader
//		opCodes...
//
//	blockString:
//
//		DWORD	blockHeader
//		DWORD	string length (negative, because it is literal)
//		chars
//		'\0'
//		padding to DWORD
//
//	blockDatum:
//		[Same as string, serialized encoding of a CDatum]

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_INVALID_LITERAL,				"Invalid literal in code block: %s.")

DECLARE_CONST_STRING(TYPENAME_HEXECODE,					"hexeCode")
const CString &CHexeCode::StaticGetTypename (void) { return TYPENAME_HEXECODE; }

void CHexeCode::Create (const CHexeCodeIntermediate &Intermediate, int iEntryPoint, CDatum *retdEntryPoint)

//	Create
//
//	Creates a code buffer and returns an entry point

	{
	int i;

	//	Allocate a new code object

	CHexeCode *pCodeObj = new CHexeCode;
	CDatum dCodeObj(pCodeObj);
	CBuffer &Dest = pCodeObj->m_Code;

	//	Keep track of offsets

	TArray<int> CodeOffsets;
	TArray<int> DataOffsets;

	//	Write out all code blocks first

	for (i = 0; i < Intermediate.GetCodeBlockCount(); i++)
		{
		const CBuffer &Source = Intermediate.GetCodeBlock(i);

		//	Remember the offset of the block. We add the size of
		//	the block header.

		int iOffset = Dest.GetPos() + sizeof(BLOCKHEADER);
		CodeOffsets.Insert(iOffset);

		//	Write the block header

		BLOCKHEADER dwHeader = ComposeHeader(blockCode, Source.GetLength() + sizeof(BLOCKHEADER));
		Dest.Write(&dwHeader, sizeof(BLOCKHEADER));

		//	Write the code

		Dest.Write(Source.GetPointer(), Source.GetLength());
		}

	//	Write out the datums

	for (i = 0; i < Intermediate.GetDatumBlockCount(); i++)
		{
		CDatum dSource = Intermediate.GetDatumBlock(i);
		CString sSource;
		BlockTypes iBlockType;

		//	If this is a string, then we store it as a string

		if (dSource.GetBasicType() == CDatum::typeString)
			{
			sSource = dSource;
			iBlockType = blockString;
			}

		//	Otherwise we serialize as a datum

		else
			{
			CStringBuffer Stream;
			dSource.Serialize(CDatum::formatAEONScript, Stream);
			sSource.TakeHandoff(Stream);
			iBlockType = blockDatum;
			}

		//	Write the block header

		DWORD dwStringSizeAligned = AlignUp(sSource.GetLength() + 1, (int)sizeof(DWORD));
		BLOCKHEADER dwHeader = ComposeHeader(iBlockType, sizeof(BLOCKHEADER) + sizeof(DWORD) + dwStringSizeAligned);
		Dest.Write(&dwHeader, sizeof(BLOCKHEADER));

		//	Write the string length (negative because it is not allocated)

		int iLen = -sSource.GetLength();
		Dest.Write(&iLen, sizeof(DWORD));

		//	Remember the offset

		DataOffsets.Insert(Dest.GetPos());

		//	Write the rest of the string

		Dest.Write(sSource.GetParsePointer(), sSource.GetLength());

		//	Write termination

		Dest.Write("\0\0\0\0\0", dwStringSizeAligned - sSource.GetLength());
		}

	//	Now that we have all the offset, we fix up the code buffers with the correct offset

	for (i = 0; i < CodeOffsets.GetCount(); i++)
		{
		DWORD *pBlockStart = (DWORD *)(Dest.GetPointer() + CodeOffsets[i] - sizeof(DWORD));
		DWORD *pPos = (DWORD *)(Dest.GetPointer() + CodeOffsets[i]);
		DWORD *pPosEnd = (DWORD *)((char *)pBlockStart + GetBlockSize(*pBlockStart));

		while (pPos < pPosEnd)
			{
			SOpCodeInfo *pInfo = g_OpCodeDb.GetInfo(*pPos);

			//	Fix up offsets

			if (pInfo->iOperand == operandCodeOffset)
				*pPos = MakeOpCode(pInfo->dwOpCode, CodeOffsets[GetOperand(*pPos)]);
			else if (pInfo->iOperand == operandStringOffset || pInfo->iOperand == operandDatumOffset)
				*pPos = MakeOpCode(pInfo->dwOpCode, DataOffsets[GetOperand(*pPos)]);

			//	Next op code

			pPos = g_OpCodeDb.Advance(pPos);
			}
		}

	//	Create a function that points to the given offset

	CHexeFunction::Create(dCodeObj, CodeOffsets[iEntryPoint], CDatum(), CDatum(), retdEntryPoint);
	}

void CHexeCode::CreateFunctionCall (const CString &sFunction, const TArray<CDatum> &Args, CDatum *retdEntryPoint)

//	CreateFunctionCall
//
//	Creates a CHexeCode block that makes a function call

	{
	int i;

	CHexeCodeIntermediate CodeBlocks;
	int iBlock = CodeBlocks.CreateCodeBlock();

	//	Compile the function name (which must be a global function)

	int iSymbol = CodeBlocks.CreateDatumBlock(sFunction);

	//	Add the lookup opcode

	CodeBlocks.WriteShortOpCode(iBlock, opPushGlobal, iSymbol);

	//	Push all the arguments

	for (i = 0; i < Args.GetCount(); i++)
		{
		int iDataBlock = CodeBlocks.CreateDatumBlock(Args[i]);
		if (Args[i].GetBasicType() == CDatum::typeString)
			CodeBlocks.WriteShortOpCode(iBlock, opPushStr, iDataBlock);
		else
			CodeBlocks.WriteShortOpCode(iBlock, opPushDatum, iDataBlock);
		}

	//	Create the environment

	CodeBlocks.WriteShortOpCode(iBlock, opMakeEnv, Args.GetCount());

	//	Call

	CodeBlocks.WriteShortOpCode(iBlock, opCall);

	//	Done

	CodeBlocks.WriteShortOpCode(iBlock, opHalt);

	//	Now create the function

	Create(CodeBlocks, iBlock, retdEntryPoint);
	}

void CHexeCode::CreateFunctionCall (int iArgCount, CDatum *retdEntryPoint)

//	CreateFunctionCall
//
//	Creates a function call assuming that the args and function are already
//	on the stack.

	{
	CHexeCodeIntermediate CodeBlocks;
	int iBlock = CodeBlocks.CreateCodeBlock();

	//	Create the environment

	CodeBlocks.WriteShortOpCode(iBlock, opMakeEnv, iArgCount);

	//	Call

	CodeBlocks.WriteShortOpCode(iBlock, opCall);

	//	Done

	CodeBlocks.WriteShortOpCode(iBlock, opHalt);

	//	Now create the function

	Create(CodeBlocks, iBlock, retdEntryPoint);
	}

void CHexeCode::CreateInvokeCall (const TArray<CDatum> &Args, CDatum *retdEntryPoint)

//	CreateInvokeCall
//
//	Creates a CHexeCode block that makes an invoke call

	{
	int i;

	CHexeCodeIntermediate CodeBlocks;
	int iBlock = CodeBlocks.CreateCodeBlock();

	//	Push the invoke code

	CodeBlocks.WriteShortOpCode(iBlock, opMakePrimitive, CDatum::primitiveInvoke);

	//	Push all the arguments

	for (i = 0; i < Args.GetCount(); i++)
		{
		int iDataBlock = CodeBlocks.CreateDatumBlock(Args[i]);
		if (Args[i].GetBasicType() == CDatum::typeString)
			CodeBlocks.WriteShortOpCode(iBlock, opPushStr, iDataBlock);
		else
			CodeBlocks.WriteShortOpCode(iBlock, opPushDatum, iDataBlock);
		}

	//	Create the environment

	CodeBlocks.WriteShortOpCode(iBlock, opMakeEnv, Args.GetCount());

	//	Call

	CodeBlocks.WriteShortOpCode(iBlock, opCall);

	//	Done

	CodeBlocks.WriteShortOpCode(iBlock, opReturn);

	//	Now create the function

	Create(CodeBlocks, iBlock, retdEntryPoint);
	}

CDatum CHexeCode::GetDatum (int iOffset)

//	GetDatum
//
//	Returns the datum at the offset

	{
	//	This is a string literal so we don't allocate anything

	CString sData(m_Code.GetPointer() + iOffset, -1, true);

	//	Deserialize

	CStringBuffer Data(sData);
	CDatum dDatum;
	if (!CDatum::Deserialize(CDatum::formatAEONScript, Data, &dDatum))
		CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_LITERAL, sData), &dDatum);

	//	Done

	return dDatum;
	}

int CHexeCode::GetOperandInt (DWORD dwCode)
	{
	DWORD dwOperand = GetOperand(dwCode);
	if (dwOperand & 0x00800000)
		return (int)(dwOperand | 0xff000000);
	else
		return (int)dwOperand;
	}

bool CHexeCode::OnDeserialize (CDatum::ESerializationFormats iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	DWORD dwLength;
	Stream.Read(&dwLength, sizeof(DWORD));

	m_Code.SetLength(dwLength);

	if (dwLength > 0)
		Stream.Read(m_Code.GetPointer(), m_Code.GetLength());

	return true;
	}

void CHexeCode::OnSerialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	DWORD dwLength = m_Code.GetLength();

	Stream.Write(&dwLength, sizeof(DWORD));
	if (dwLength)
		Stream.Write(m_Code.GetPointer(), m_Code.GetLength());
	}
