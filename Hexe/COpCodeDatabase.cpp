//	COpCodeDatabase.cpp
//
//	COpCodeDatabase class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(OP_ADD,							"add")
DECLARE_CONST_STRING(OP_ADD2,							"add2")
DECLARE_CONST_STRING(OP_APPEND_LOCAL_ITEM,				"appendLocalItem")
DECLARE_CONST_STRING(OP_CALL,							"call")
DECLARE_CONST_STRING(OP_DEFINE,							"define")
DECLARE_CONST_STRING(OP_DEFINE_ARG,						"defineArg")
DECLARE_CONST_STRING(OP_DEFINE_ARG_FROM_CODE,			"defineArgFromCode")
DECLARE_CONST_STRING(OP_DEFINE_NEXT_ARG,				"defineNextArg")
DECLARE_CONST_STRING(OP_DIVIDE,							"divide")
DECLARE_CONST_STRING(OP_DIVIDE2,						"divide2")
DECLARE_CONST_STRING(OP_ENTER_ENV,						"enterEnv")
DECLARE_CONST_STRING(OP_ERROR,							"error")
DECLARE_CONST_STRING(OP_EXIT_ENV,						"exitEnv")
DECLARE_CONST_STRING(OP_HALT,							"halt")
DECLARE_CONST_STRING(OP_HEXARC_MSG,						"hexarcMsg")
DECLARE_CONST_STRING(OP_INC_LOCAL_L0,					"incLocalL0")
DECLARE_CONST_STRING(OP_INC_LOCAL_INT,					"incLocalInt")
DECLARE_CONST_STRING(OP_IS_EQUAL,						"isEqual")
DECLARE_CONST_STRING(OP_IS_GREATER,						"isGreater")
DECLARE_CONST_STRING(OP_IS_GREATER_OR_EQUAL,			"isGreaterOrEqual")
DECLARE_CONST_STRING(OP_IS_IDENTICAL,					"isIdentical")
DECLARE_CONST_STRING(OP_IS_LESS,						"isLess")
DECLARE_CONST_STRING(OP_IS_LESS_OR_EQUAL,				"isLessOrEqual")
DECLARE_CONST_STRING(OP_IS_NOT_EQUAL,					"isNotEqual")
DECLARE_CONST_STRING(OP_IS_NOT_IDENTICAL,				"isNotIdentical")
DECLARE_CONST_STRING(OP_JUMP,							"jump")
DECLARE_CONST_STRING(OP_JUMP_IF_NIL,					"jumpIfNil")
DECLARE_CONST_STRING(OP_JUMP_IF_NIL_NO_POP,				"jumpIfNilNoPop")
DECLARE_CONST_STRING(OP_JUMP_IF_NOT_NIL_NO_POP,			"jumpIfNotNilNoPop")
DECLARE_CONST_STRING(OP_MAKE_APPLY_ENV,					"makeApplyEnv")
DECLARE_CONST_STRING(OP_MAKE_ARRAY,						"makeArray")
DECLARE_CONST_STRING(OP_MAKE_BLOCK_ENV,					"makeBlockEnv")
DECLARE_CONST_STRING(OP_MAKE_ENV,						"makeEnv")
DECLARE_CONST_STRING(OP_MAKE_FLAGS_FROM_ARRAY,			"makeFlagsFromArray")
DECLARE_CONST_STRING(OP_MAKE_FUNC,						"makeFunc")
DECLARE_CONST_STRING(OP_MAKE_FUNC2,						"makeFunc2")
DECLARE_CONST_STRING(OP_MAKE_LOCAL_ENV,					"makeLocalEnv")
DECLARE_CONST_STRING(OP_MAKE_PRIMITIVE,					"makePrimitive")
DECLARE_CONST_STRING(OP_MAKE_STRUCT,					"makeStruct")
DECLARE_CONST_STRING(OP_MAP_RESULT,						"mapResult")
DECLARE_CONST_STRING(OP_MULTIPLY,						"multiply")
DECLARE_CONST_STRING(OP_MULTIPLY2,						"multiply2")
DECLARE_CONST_STRING(OP_NO_OP,							"noOp")
DECLARE_CONST_STRING(OP_NOT,							"not")
DECLARE_CONST_STRING(OP_POP,							"pop")
DECLARE_CONST_STRING(OP_POP_LOCAL,						"popLocal")
DECLARE_CONST_STRING(OP_POP_LOCAL_L0,					"popLocalL0")
DECLARE_CONST_STRING(OP_PUSH_DATUM,						"pushDatum")
DECLARE_CONST_STRING(OP_PUSH_GLOBAL,					"pushGlobal")
DECLARE_CONST_STRING(OP_PUSH_INT,						"pushInt")
DECLARE_CONST_STRING(OP_PUSH_INT_SHORT,					"pushIntShort")
DECLARE_CONST_STRING(OP_PUSH_LITERAL,					"pushLiteral")
DECLARE_CONST_STRING(OP_PUSH_LOCAL,						"pushLocal")
DECLARE_CONST_STRING(OP_PUSH_LOCAL_L0,					"pushLocalL0")
DECLARE_CONST_STRING(OP_PUSH_LOCAL_ITEM,				"pushLocalItem")
DECLARE_CONST_STRING(OP_PUSH_LOCAL_LENGTH,				"pushLocalLength")
DECLARE_CONST_STRING(OP_PUSH_NIL,						"pushNil")
DECLARE_CONST_STRING(OP_PUSH_STR,						"pushStr")
DECLARE_CONST_STRING(OP_PUSH_STR_NULL,					"pushStrNull")
DECLARE_CONST_STRING(OP_PUSH_TRUE,						"pushTrue")
DECLARE_CONST_STRING(OP_RETURN,							"return")
DECLARE_CONST_STRING(OP_SET_GLOBAL,						"setGlobal")
DECLARE_CONST_STRING(OP_SET_GLOBAL_ITEM,				"setGlobalItem")
DECLARE_CONST_STRING(OP_SET_LOCAL,						"setLocal")
DECLARE_CONST_STRING(OP_SET_LOCAL_L0,					"setLocalL0")
DECLARE_CONST_STRING(OP_SET_LOCAL_ITEM,					"setLocalItem")
DECLARE_CONST_STRING(OP_SUBTRACT,						"subtract")
DECLARE_CONST_STRING(OP_SUBTRACT2,						"subtract2")

static SOpCodeInfo OPCODE_INFO[] = 
	{
	SOpCodeInfo(opNoOp,				OP_NO_OP,				operandNone	),

	//	Comparisons
	SOpCodeInfo(opIsEqual,			OP_IS_EQUAL,			operandIntShort ),
	SOpCodeInfo(opIsEqual,			OP_IS_IDENTICAL,		operandIntShort ),
	SOpCodeInfo(opIsLess,			OP_IS_LESS,				operandIntShort ),
	SOpCodeInfo(opIsGreater,		OP_IS_GREATER,			operandIntShort ),
	SOpCodeInfo(opIsLessOrEqual,	OP_IS_LESS_OR_EQUAL,	operandIntShort ),
	SOpCodeInfo(opIsGreaterOrEqual,	OP_IS_GREATER_OR_EQUAL,	operandIntShort ),
	SOpCodeInfo(opIsNotEqual,		OP_IS_NOT_EQUAL,		operandIntShort ),
	SOpCodeInfo(opIsNotIdentical,	OP_IS_NOT_IDENTICAL,	operandIntShort ),
	SOpCodeInfo(opNot,				OP_NOT,					operandNone ),

	//	Control flow
	SOpCodeInfo(opJump,				OP_JUMP,				operandIntShort ),
	SOpCodeInfo(opJumpIfNil,		OP_JUMP_IF_NIL,			operandIntShort ),
	SOpCodeInfo(opJumpIfNilNoPop,	OP_JUMP_IF_NIL_NO_POP,	operandIntShort ),
	SOpCodeInfo(opJumpIfNotNilNoPop,OP_JUMP_IF_NOT_NIL_NO_POP,	operandIntShort ),
	SOpCodeInfo(opPop,				OP_POP,					operandIntShort ),
	SOpCodeInfo(opHexarcMsg,		OP_HEXARC_MSG,			operandNone ),
	SOpCodeInfo(opMakeBlockEnv,		OP_MAKE_BLOCK_ENV,		operandNone ),
	SOpCodeInfo(opMakeLocalEnv,		OP_MAKE_LOCAL_ENV,		operandIntShort ),

	//	Function calls
	SOpCodeInfo(opMakeFunc,			OP_MAKE_FUNC,			operandCodeOffset ),
	SOpCodeInfo(opMakeFunc2,		OP_MAKE_FUNC2,			operandCodeOffset ),
	SOpCodeInfo(opMakeEnv,			OP_MAKE_ENV,			operandIntShort ),
	SOpCodeInfo(opMakeApplyEnv,		OP_MAKE_APPLY_ENV,		operandIntShort ),
	SOpCodeInfo(opCall,				OP_CALL,				operandNone ),
	SOpCodeInfo(opEnterEnv,			OP_ENTER_ENV,			operandNone ),
	SOpCodeInfo(opDefineArg,		OP_DEFINE_ARG,			operandStringOffset ),
	SOpCodeInfo(opDefineArgFromCode,OP_DEFINE_ARG_FROM_CODE,operandIntShort ),
	SOpCodeInfo(opDefineNextArg,	OP_DEFINE_NEXT_ARG,		operandNone ),
	SOpCodeInfo(opExitEnv,			OP_EXIT_ENV,			operandNone ),
	SOpCodeInfo(opReturn,			OP_RETURN,				operandNone ),
	SOpCodeInfo(opMakePrimitive,	OP_MAKE_PRIMITIVE,		operandIntShort ),

	//	Local environment
	SOpCodeInfo(opIncLocalL0,		OP_INC_LOCAL_L0,		operandIntShort ),
	SOpCodeInfo(opPopLocal,			OP_POP_LOCAL,			operandIntShort ),
	SOpCodeInfo(opPopLocalL0,		OP_POP_LOCAL_L0,		operandIntShort ),
	SOpCodeInfo(opPushLocal,		OP_PUSH_LOCAL,			operandIntShort ),
	SOpCodeInfo(opPushLocalL0,		OP_PUSH_LOCAL_L0,		operandIntShort ),
	SOpCodeInfo(opSetLocal,			OP_SET_LOCAL,			operandIntShort ),
	SOpCodeInfo(opSetLocalL0,		OP_SET_LOCAL_L0,		operandIntShort ),

	//	Global environment
	SOpCodeInfo(opDefine,			OP_DEFINE,				operandStringOffset ),
	SOpCodeInfo(opPushGlobal,		OP_PUSH_GLOBAL,			operandStringOffset ),
	SOpCodeInfo(opSetGlobal,		OP_SET_GLOBAL,			operandStringOffset ),

	//	Lists & Structs
	SOpCodeInfo(opAppendLocalItem,	OP_APPEND_LOCAL_ITEM,	operandIntShort ),
	SOpCodeInfo(opMakeArray,		OP_MAKE_ARRAY,			operandIntShort ),
	SOpCodeInfo(opMakeStruct,		OP_MAKE_STRUCT,			operandIntShort ),
	SOpCodeInfo(opPushLocalItem,	OP_PUSH_LOCAL_ITEM,		operandIntShort ),
	SOpCodeInfo(opPushLocalLength,	OP_PUSH_LOCAL_LENGTH,	operandIntShort ),
	SOpCodeInfo(opSetGlobalItem,	OP_SET_GLOBAL_ITEM,		operandStringOffset ),
	SOpCodeInfo(opSetLocalItem,		OP_SET_LOCAL_ITEM,		operandIntShort ),

	//	Literals
	SOpCodeInfo(opPushDatum,		OP_PUSH_DATUM,			operandStringOffset ),
	SOpCodeInfo(opPushNil,			OP_PUSH_NIL,			operandNone ),
	SOpCodeInfo(opPushTrue,			OP_PUSH_TRUE,			operandNone ),
	SOpCodeInfo(opPushInt,			OP_PUSH_INT,			operandInt ),
	SOpCodeInfo(opPushIntShort,		OP_PUSH_INT_SHORT,		operandIntShort ),
	SOpCodeInfo(opPushStr,			OP_PUSH_STR,			operandStringOffset ),
	SOpCodeInfo(opPushStrNull,		OP_PUSH_STR_NULL,		operandNone ),
	SOpCodeInfo(opPushLiteral,		OP_PUSH_LITERAL,		operandIntShort ),

	//	Math
	SOpCodeInfo(opAdd,				OP_ADD,					operandIntShort ),
	SOpCodeInfo(opAdd2,				OP_ADD2,				operandNone ),
	SOpCodeInfo(opDivide,			OP_DIVIDE,				operandIntShort ),
	SOpCodeInfo(opDivide2,			OP_DIVIDE2,				operandNone ),
	SOpCodeInfo(opIncLocalInt,		OP_INC_LOCAL_INT,		operandIntShort ),
	SOpCodeInfo(opMultiply,			OP_MULTIPLY,			operandIntShort ),
	SOpCodeInfo(opMultiply2,		OP_MULTIPLY2,			operandNone ),
	SOpCodeInfo(opSubtract,			OP_SUBTRACT,			operandIntShort ),
	SOpCodeInfo(opSubtract2,		OP_SUBTRACT2,			operandNone ),

	//	Miscellaneous
	SOpCodeInfo(opError,			OP_ERROR,				operandIntShort ),
	SOpCodeInfo(opHalt,				OP_HALT,				operandNone ),
	SOpCodeInfo(opMakeFlagsFromArray,	OP_MAKE_FLAGS_FROM_ARRAY,	operandNone ),
	SOpCodeInfo(opMapResult,		OP_MAP_RESULT,			operandIntShort ),
	};

static int OPCODE_INFO_COUNT = SIZEOF_STATIC_ARRAY(OPCODE_INFO);

COpCodeDatabase g_OpCodeDb;

COpCodeDatabase::COpCodeDatabase (void)

//	COpCodeDatabase constructor

	{
	int i;

	//	Initialize all to noOp

	for (i = 0; i < opCodeCount; i++)
		m_Info[i] = &OPCODE_INFO[0];

	//	Add all known opcodes

	for (i = 0; i < OPCODE_INFO_COUNT; i++)
		{
		SOpCodeInfo *pInfo = &OPCODE_INFO[i];
		m_Info[pInfo->dwOpCode >> 24] = pInfo;
		}
	}

DWORD *COpCodeDatabase::Advance (DWORD *pPos)

//	Advance
//
//	Advances the instruction pointer to the next opcode

	{
	SOpCodeInfo *pInfo = GetInfo(*pPos);

	switch (pInfo->iOperand)
		{
		case operandNone:
		case operandIntShort:
		case operandCodeOffset:
		case operandStringOffset:
		case operandDatumOffset:
			return pPos + 1;

		case operandInt:
			return pPos + 2;

		default:
			ASSERT(false);
			return pPos + 1;
		}
	}
