//	CGLVMCodeGenerator.cpp
//
//	CGLVMCodeGenerator Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CGLVMCodeGenerator::EnterCodeBlock ()
	{
	if (m_iBlock == -1)
		throw CException(errFail);

	m_SavedBlocks.Insert(m_iBlock);
	m_iBlock = m_Code.CreateCodeBlock();
	}

void CGLVMCodeGenerator::ExitCodeBlock ()
	{
	if (m_SavedBlocks.GetCount() == 0)
		throw CException(errFail);

	m_iBlock = m_SavedBlocks[m_SavedBlocks.GetCount() - 1];
	m_SavedBlocks.Delete(m_SavedBlocks.GetCount() - 1);
	}

void CGLVMCodeGenerator::Init ()

//	Init
//
//	Initialize the generator. Must be called before anything else.

	{
	m_Code = CHexeCodeIntermediate();
	m_iBlock = m_Code.CreateCodeBlock();
	}

CDatum CGLVMCodeGenerator::CreateOutput ()

//	CreateOutput
//
//	Creates the output.

	{
	return m_Code.CreateOutput(m_iBlock);
	}

int CGLVMCodeGenerator::CreateDataBlock (CDatum dValue)
	{
	return m_Code.CreateDatumBlock(dValue);
	}

void CGLVMCodeGenerator::WriteLongOpCode (OPCODE opCode, DWORD dwData)
	{
	m_Code.WriteLongOpCode(m_iBlock, opCode, dwData);
	}

void CGLVMCodeGenerator::WriteShortOpCode (OPCODE opCode, DWORD dwOperand)
	{
	m_Code.WriteShortOpCode(m_iBlock, opCode, dwOperand);
	}
