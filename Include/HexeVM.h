//	HexeVM.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2011 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

typedef DWORD OPCODE;

//	OpCodes --------------------------------------------------------------------
//
//	A code block consists of an array of DWORDs. Each DWORD is either an opcode
//	or data.
//
//	An opcode uses the high-order byte as the opcode. The remaining bits (24)
//	are used as data, if necessary.
//
//	Data DWORDs are either a CDatum or an opcode-specific type (e.g., a jump
//	offset).

enum EOpCodes
	{
	opNoOp =				0x00000000,		//	No operation

	opDefine =				0x01000000,		//	Defines variable in current environment
	opPushIntShort =		0x02000000,		//	Pushes operand as int
	opPushStr =				0x03000000,		//	Pushes operand as string block
	opPushStrNull =			0x04000000,		//	Pushes CDatum("")
	opPushGlobal =			0x05000000,		//	Pushes the global variable
	opMakeFunc =			0x06000000,
	opEnterEnv =			0x07000000,
	opDefineArg =			0x08000000,
	opExitEnv =				0x09000000,
	opReturn =				0x0a000000,
	opPushLocal =			0x0b000000,
	opMakeEnv =				0x0c000000,
	opCall =				0x0d000000,
	opPushInt =				0x0e000000,		//	Pushes operand as int
	opAdd =					0x0f000000,
	opDivide =				0x10000000,
	opMultiply =			0x11000000,
	opSubtract =			0x12000000,
	opJump =				0x13000000,
	opJumpIfNil =			0x14000000,
	opPushNil =				0x15000000,		//	Pushes CDatum()
	opPushTrue =			0x16000000,		//	Pushes CDatum(true)
	opIsEqual =				0x17000000,
	opIsLess =				0x18000000,
	opIsGreater =			0x19000000,
	opIsLessOrEqual =		0x1a000000,
	opIsGreaterOrEqual =	0x1b000000,
	opMakeArray =			0x1c000000,
	opPop =					0x1d000000,
	opHexarcMsg =			0x1e000000,
	opMakeApplyEnv =		0x1f000000,
	opMakePrimitive =		0x20000000,
	opPushDatum =			0x21000000,
	opError =				0x22000000,
	opSetLocal =			0x23000000,
	opSetGlobal =			0x24000000,
	opPopLocal =			0x25000000,
	opMakeBlockEnv =		0x26000000,
	opNot =					0x27000000,
	opSetLocalItem =		0x28000000,
	opSetGlobalItem =		0x29000000,
	opMakeStruct =			0x2a000000,
	opPushLocalLength =		0x2b000000,
	opPushLocalItem =		0x2c000000,
	opAppendLocalItem =		0x2d000000,
	opIncLocalInt =			0x2e000000,
	opMakeFlagsFromArray =	0x2f000000,
	opMapResult =			0x30000000,
	opJumpIfNilNoPop =		0x31000000,
	opJumpIfNotNilNoPop =	0x32000000,
	opIsNotEqual =			0x33000000,
	opPushArrayItem =		0x34000000,
	opSetArrayItem =		0x35000000,
	opPower =				0x36000000,
	opMod =					0x37000000,
	opCompareStep =			0x38000000,
	opIncStep =				0x39000000,
	opPushStructItem =		0x3a000000,
	opSetStructItem =		0x3b000000,

	opHalt =				0xff000000,

	opCodeCount =			256,
	};

//	CHexeCodeIntermediate ------------------------------------------------------

class CHexeCodeIntermediate
	{
	public:
		int CreateCodeBlock (void);
		int CreateDatumBlock (CDatum dDatum);
		CDatum CreateOutput (int iBlock) const;
		int GetCodeBlockPos (int iBlock) const { return m_CodeBlocks[iBlock].GetPos(); }
		void RewriteShortOpCode (int iBlock, int iPos, OPCODE opCode, DWORD dwOperand = 0);
		void WriteShortOpCode (int iBlock, OPCODE opCode, DWORD dwOperand = 0);
		void WriteLongOpCode (int iBlock, OPCODE opCode, DWORD dwData);
//		void WriteLongOpCode (int iBlock, OPCODE opCode, CDatum dData);

		const CBuffer &GetCodeBlock (int iIndex) const { return m_CodeBlocks[iIndex]; }
		int GetCodeBlockCount (void) const { return m_CodeBlocks.GetCount(); }
		CDatum GetDatumBlock (int iIndex) const { return m_DatumBlocks[iIndex]; }
		int GetDatumBlockCount (void) const { return m_DatumBlocks.GetCount(); }

	private:
		TArray<CBuffer> m_CodeBlocks;
		TArray<CDatum> m_DatumBlocks;
	};

