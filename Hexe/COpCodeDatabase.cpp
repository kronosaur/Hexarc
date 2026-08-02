//	COpCodeDatabase.cpp
//
//	COpCodeDatabase class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(OP_ADD,							"add");
DECLARE_CONST_STRING(OP_ADD2,							"add2");
DECLARE_CONST_STRING(OP_ADD_INT,						"addInt");
DECLARE_CONST_STRING(OP_ADD_LOCAL_L0_INT16,				"addLocalL0Int16");
DECLARE_CONST_STRING(OP_APPEND_TO_ARRAY,				"appendToArray");
DECLARE_CONST_STRING(OP_CALL,							"call");
DECLARE_CONST_STRING(OP_CALL_DIRECT,					"callDirect");
DECLARE_CONST_STRING(OP_CALL_DIRECT_NO_CLOSURE,			"callDirectNoClosure");
DECLARE_CONST_STRING(OP_CALL_DIRECT_SELF_NO_CLOSURE,	"callDirectSelfNoClosure");
DECLARE_CONST_STRING(OP_CALL_FRAME,						"callFrame");
DECLARE_CONST_STRING(OP_CALL_FRAME_SELF,				"callFrameSelf");
DECLARE_CONST_STRING(OP_CALL_LIB,						"callLib");
DECLARE_CONST_STRING(OP_COMPARE_FOR_EACH,				"compareForEach");
DECLARE_CONST_STRING(OP_COMPARE_STEP,					"compareStep");
DECLARE_CONST_STRING(OP_CONCAT,							"concat");
DECLARE_CONST_STRING(OP_DEBUG_BREAK,					"debugBreak");
DECLARE_CONST_STRING(OP_DEFINE,							"define");
DECLARE_CONST_STRING(OP_DEFINE_ARG,						"defineArg");
DECLARE_CONST_STRING(OP_DEFINE_ARG_FROM_CODE,			"defineArgFromCode");
DECLARE_CONST_STRING(OP_DEFINE_NEXT_ARG,				"defineNextArg");
DECLARE_CONST_STRING(OP_DIVIDE,							"divide");
DECLARE_CONST_STRING(OP_DIVIDE2,						"divide2");
DECLARE_CONST_STRING(OP_ENTER_ENV,						"enterEnv");
DECLARE_CONST_STRING(OP_ENTER_STACK_FRAME,				"enterStackFrame");
DECLARE_CONST_STRING(OP_ERROR,							"error");
DECLARE_CONST_STRING(OP_EXIT_ENV,						"exitEnv");
DECLARE_CONST_STRING(OP_EXIT_ENV_AND_JUMP_IF_NIL,		"exitEnvAndJumpIfNil");
DECLARE_CONST_STRING(OP_EXIT_ENV_AND_JUMP_IF_GREATER_INT,	"exitEnvAndJumpIfGreaterInt");
DECLARE_CONST_STRING(OP_EXIT_ENV_AND_JUMP_IF_GREATER_OR_EQUAL_INT,	"exitEnvAndJumpIfGreaterOrEqualInt");
DECLARE_CONST_STRING(OP_EXIT_ENV_AND_JUMP_IF_LOCAL_GREATER_INT,	"exitEnvAndJumpIfLocalGreaterInt");
DECLARE_CONST_STRING(OP_EXIT_ENV_AND_RETURN,			"exitEnvAndReturn");
DECLARE_CONST_STRING(OP_HALT,							"halt");
DECLARE_CONST_STRING(OP_HEXARC_MSG,						"hexarcMsg");
DECLARE_CONST_STRING(OP_INC,							"inc");
DECLARE_CONST_STRING(OP_INC_FOR_EACH,					"incForEach");
DECLARE_CONST_STRING(OP_INC_LOCAL_L0,					"incLocalL0");
DECLARE_CONST_STRING(OP_INC_LOCAL_INT,					"incLocalInt");
DECLARE_CONST_STRING(OP_INC_STEP,						"incStep");
DECLARE_CONST_STRING(OP_INIT_FOR_EACH,					"initForEach");
DECLARE_CONST_STRING(OP_IS_EQUAL,						"isEqual");
DECLARE_CONST_STRING(OP_IS_EQUAL_INT,					"isEqualInt");
DECLARE_CONST_STRING(OP_IS_EQUAL_MULTI,					"isEqualMulti");
DECLARE_CONST_STRING(OP_IS_GREATER,						"isGreater");
DECLARE_CONST_STRING(OP_IS_GREATER_INT,					"isGreaterInt");
DECLARE_CONST_STRING(OP_IS_GREATER_MULTI,				"isGreaterMulti");
DECLARE_CONST_STRING(OP_IS_GREATER_OR_EQUAL,			"isGreaterOrEqual");
DECLARE_CONST_STRING(OP_IS_GREATER_OR_EQUAL_INT,		"isGreaterOrEqualInt");
DECLARE_CONST_STRING(OP_IS_GREATER_OR_EQUAL_MULTI,		"isGreaterOrEqualMulti");
DECLARE_CONST_STRING(OP_IS_IDENTICAL,					"isIdentical");
DECLARE_CONST_STRING(OP_IS_IN,							"isIn");
DECLARE_CONST_STRING(OP_IS_LESS,						"isLess");
DECLARE_CONST_STRING(OP_IS_LESS_INT,					"isLessInt");
DECLARE_CONST_STRING(OP_IS_LESS_MULTI,					"isLessMulti");
DECLARE_CONST_STRING(OP_IS_LESS_OR_EQUAL,				"isLessOrEqual");
DECLARE_CONST_STRING(OP_IS_LESS_OR_EQUAL_INT,			"isLessOrEqualInt");
DECLARE_CONST_STRING(OP_IS_LESS_OR_EQUAL_MULTI,			"isLessOrEqualMulti");
DECLARE_CONST_STRING(OP_IS_NOT_EQUAL,					"isNotEqual");
DECLARE_CONST_STRING(OP_IS_NOT_EQUAL_INT,				"isNotEqualInt");
DECLARE_CONST_STRING(OP_IS_NOT_EQUAL_MULTI,				"isNotEqualMulti");
DECLARE_CONST_STRING(OP_IS_NOT_IDENTICAL,				"isNotIdentical");
DECLARE_CONST_STRING(OP_IS_NOT_IN,						"isNotIn");
DECLARE_CONST_STRING(OP_JUMP,							"jump");
DECLARE_CONST_STRING(OP_JUMP_IF_NIL,					"jumpIfNil");
DECLARE_CONST_STRING(OP_JUMP_IF_NIL_NO_POP,				"jumpIfNilNoPop");
DECLARE_CONST_STRING(OP_JUMP_IF_NOT_NIL_NO_POP,			"jumpIfNotNilNoPop");
DECLARE_CONST_STRING(OP_LOOP_INC_AND_JUMP,				"loopIncAndJump");
DECLARE_CONST_STRING(OP_LOOP_INC_LOCAL_AND_JUMP,		"loopIncLocalAndJump");
DECLARE_CONST_STRING(OP_MAKE_APPLY_ENV,					"makeApplyEnv");
DECLARE_CONST_STRING(OP_MAKE_ARRAY,						"makeArray");
DECLARE_CONST_STRING(OP_MAKE_AS_TYPE,					"makeAsType");
DECLARE_CONST_STRING(OP_MAKE_AS_TYPE_CONS,				"makeAsTypeCons");
DECLARE_CONST_STRING(OP_MAKE_BLOCK_ENV,					"makeBlockEnv");
DECLARE_CONST_STRING(OP_MAKE_DATATYPE,					"makeDatatype");
DECLARE_CONST_STRING(OP_MAKE_EMPTY_ARRAY,				"makeEmptyArray");
DECLARE_CONST_STRING(OP_MAKE_EMPTY_ARRAY_AS_TYPE,		"makeEmptyArrayAsType");
DECLARE_CONST_STRING(OP_MAKE_EMPTY_STRUCT,				"makeEmptyStruct");
DECLARE_CONST_STRING(OP_MAKE_ENV,						"makeEnv");
DECLARE_CONST_STRING(OP_MAKE_EXPR,						"makeExpr");
DECLARE_CONST_STRING(OP_MAKE_EXPR_IF,					"makeExprIf");
DECLARE_CONST_STRING(OP_MAKE_EXPR_NOT,					"makeExprNot");
DECLARE_CONST_STRING(OP_MAKE_EXPR_OR,					"makeExprOr");
DECLARE_CONST_STRING(OP_MAKE_FLAGS_FROM_ARRAY,			"makeFlagsFromArray");
DECLARE_CONST_STRING(OP_MAKE_FUNC,						"makeFunc");
DECLARE_CONST_STRING(OP_MAKE_FUNC2,						"makeFunc2");
DECLARE_CONST_STRING(OP_MAKE_LOCAL_ENV,					"makeLocalEnv");
DECLARE_CONST_STRING(OP_MAKE_MAP_COL_EXPR,				"makeMapColExpr");
DECLARE_CONST_STRING(OP_MAKE_METHOD_ENV,				"makeMethodEnv");
DECLARE_CONST_STRING(OP_MAKE_OBJECT,					"makeObject");
DECLARE_CONST_STRING(OP_MAKE_OBJECT_DIRECT,				"makeObjectDirect");
DECLARE_CONST_STRING(OP_MAKE_OBJECT_DIRECT_UNCHECKED,	"makeObjectDirectUnchecked");
DECLARE_CONST_STRING(OP_MAKE_PRIMITIVE,					"makePrimitive");
DECLARE_CONST_STRING(OP_MAKE_RANGE,						"makeRange");
DECLARE_CONST_STRING(OP_MAKE_SPREAD,					"makeSpread");
DECLARE_CONST_STRING(OP_MAKE_STRUCT,					"makeStruct");
DECLARE_CONST_STRING(OP_MAKE_TENSOR,					"makeTensor");
DECLARE_CONST_STRING(OP_MAKE_TENSOR_TYPE,				"makeTensorType");
DECLARE_CONST_STRING(OP_MAP_RESULT,						"mapResult");
DECLARE_CONST_STRING(OP_MOD,							"mod");
DECLARE_CONST_STRING(OP_MULTIPLY,						"multiply");
DECLARE_CONST_STRING(OP_MULTIPLY2,						"multiply2");
DECLARE_CONST_STRING(OP_MUTATE_ARRAY_ITEM_ADD,			"mutateArrayItemAdd");
DECLARE_CONST_STRING(OP_MUTATE_ARRAY_ITEM_CONCAT,		"mutateArrayItemConcat");
DECLARE_CONST_STRING(OP_MUTATE_ARRAY_ITEM_DIVIDE,		"mutateArrayItemDivide");
DECLARE_CONST_STRING(OP_MUTATE_ARRAY_ITEM_MOD,			"mutateArrayItemMod");
DECLARE_CONST_STRING(OP_MUTATE_ARRAY_ITEM_MULTIPLY,		"mutateArrayItemMultiply");
DECLARE_CONST_STRING(OP_MUTATE_ARRAY_ITEM_POWER,		"mutateArrayItemPower");
DECLARE_CONST_STRING(OP_MUTATE_ARRAY_ITEM_SUBTRACT,		"mutateArrayItemSubtract");
DECLARE_CONST_STRING(OP_MUTATE_GLOBAL_ADD,				"mutateGlobalAdd");
DECLARE_CONST_STRING(OP_MUTATE_GLOBAL_CONCAT,			"mutateGlobalConcat");
DECLARE_CONST_STRING(OP_MUTATE_GLOBAL_DIVIDE,			"mutateGlobalDivide");
DECLARE_CONST_STRING(OP_MUTATE_GLOBAL_MOD,				"mutateGlobalMod");
DECLARE_CONST_STRING(OP_MUTATE_GLOBAL_MULTIPLY,			"mutateGlobalMultiply");
DECLARE_CONST_STRING(OP_MUTATE_GLOBAL_POWER,			"mutateGlobalPower");
DECLARE_CONST_STRING(OP_MUTATE_GLOBAL_SUBTRACT,			"mutateGlobalSubtract");
DECLARE_CONST_STRING(OP_MUTATE_LOCAL_ADD,				"mutateLocalAdd");
DECLARE_CONST_STRING(OP_MUTATE_LOCAL_CONCAT,			"mutateLocalConcat");
DECLARE_CONST_STRING(OP_MUTATE_LOCAL_DIVIDE,			"mutateLocalDivide");
DECLARE_CONST_STRING(OP_MUTATE_LOCAL_MOD,				"mutateLocalMod");
DECLARE_CONST_STRING(OP_MUTATE_LOCAL_MULTIPLY,			"mutateLocalMultiply");
DECLARE_CONST_STRING(OP_MUTATE_LOCAL_POWER,				"mutateLocalPower");
DECLARE_CONST_STRING(OP_MUTATE_LOCAL_SUBTRACT,			"mutateLocalSubtract");
DECLARE_CONST_STRING(OP_MUTATE_OBJECT_ITEM_ADD,			"mutateObjectItemAdd");
DECLARE_CONST_STRING(OP_MUTATE_OBJECT_ITEM_CONCAT,		"mutateObjectItemConcat");
DECLARE_CONST_STRING(OP_MUTATE_OBJECT_ITEM_DIVIDE,		"mutateObjectItemDivide");
DECLARE_CONST_STRING(OP_MUTATE_OBJECT_ITEM_MOD,			"mutateObjectItemMod");
DECLARE_CONST_STRING(OP_MUTATE_OBJECT_ITEM_MULTIPLY,	"mutateObjectItemMultiply");
DECLARE_CONST_STRING(OP_MUTATE_OBJECT_ITEM_POWER,		"mutateObjectItemPower");
DECLARE_CONST_STRING(OP_MUTATE_OBJECT_ITEM_SUBTRACT,	"mutateObjectItemSubtract");
DECLARE_CONST_STRING(OP_MUTATE_TENSOR_ITEM_ADD,			"mutateTensorItemAdd");
DECLARE_CONST_STRING(OP_MUTATE_TENSOR_ITEM_CONCAT,		"mutateTensorItemConcat");
DECLARE_CONST_STRING(OP_MUTATE_TENSOR_ITEM_DIVIDE,		"mutateTensorItemDivide");
DECLARE_CONST_STRING(OP_MUTATE_TENSOR_ITEM_MOD,			"mutateTensorItemMod");
DECLARE_CONST_STRING(OP_MUTATE_TENSOR_ITEM_MULTIPLY,	"mutateTensorItemMultiply");
DECLARE_CONST_STRING(OP_MUTATE_TENSOR_ITEM_POWER,		"mutateTensorItemPower");
DECLARE_CONST_STRING(OP_MUTATE_TENSOR_ITEM_SUBTRACT,	"mutateTensorItemSubtract");
DECLARE_CONST_STRING(OP_NEGATE,							"negate");
DECLARE_CONST_STRING(OP_NEW_OBJECT,						"newObject");
DECLARE_CONST_STRING(OP_NO_OP,							"noOp");
DECLARE_CONST_STRING(OP_NOT,							"not");
DECLARE_CONST_STRING(OP_POP,							"pop");
DECLARE_CONST_STRING(OP_POP_DEEP,						"popDeep");
DECLARE_CONST_STRING(OP_POP_LOCAL,						"popLocal");
DECLARE_CONST_STRING(OP_POP_LOCAL_L0,					"popLocalL0");
DECLARE_CONST_STRING(OP_POWER,							"power");
DECLARE_CONST_STRING(OP_PUSH_ARRAY_ITEM,				"pushArrayItem");
DECLARE_CONST_STRING(OP_PUSH_ARRAY_ITEM_I,				"pushArrayItemI");
DECLARE_CONST_STRING(OP_PUSH_CORE_TYPE,					"pushCoreType");
DECLARE_CONST_STRING(OP_PUSH_DATUM,						"pushDatum");
DECLARE_CONST_STRING(OP_PUSH_FALSE,						"pushFalse");
DECLARE_CONST_STRING(OP_PUSH_FRAME_ARG,					"pushFrameArg");
DECLARE_CONST_STRING(OP_PUSH_GLOBAL,					"pushGlobal");
DECLARE_CONST_STRING(OP_PUSH_INIT_FOR_EACH,				"pushInitForEach");
DECLARE_CONST_STRING(OP_PUSH_INT,						"pushInt");
DECLARE_CONST_STRING(OP_PUSH_INT_SHORT,					"pushIntShort");
DECLARE_CONST_STRING(OP_PUSH_LITERAL,					"pushLiteral");
DECLARE_CONST_STRING(OP_PUSH_LOCAL,						"pushLocal");
DECLARE_CONST_STRING(OP_PUSH_LOCAL_L0,					"pushLocalL0");
DECLARE_CONST_STRING(OP_PUSH_LOCAL_ITEM,				"pushLocalItem");
DECLARE_CONST_STRING(OP_PUSH_LOCAL_LENGTH,				"pushLocalLength");
DECLARE_CONST_STRING(OP_PUSH_NAN,						"pushNaN");
DECLARE_CONST_STRING(OP_PUSH_NIL,						"pushNil");
DECLARE_CONST_STRING(OP_PUSH_OBJECT_ITEM,				"pushObjectItem");
DECLARE_CONST_STRING(OP_PUSH_OBJECT_METHOD,				"pushObjectMethod");
DECLARE_CONST_STRING(OP_PUSH_RECORD_SLOT_I,				"pushRecordSlotI");
DECLARE_CONST_STRING(OP_PUSH_STR,						"pushStr");
DECLARE_CONST_STRING(OP_PUSH_STR_NULL,					"pushStrNull");
DECLARE_CONST_STRING(OP_PUSH_TENSOR_ITEM,				"pushTensorItem");
DECLARE_CONST_STRING(OP_PUSH_TENSOR_ITEM_I,				"pushTensorItemI");
DECLARE_CONST_STRING(OP_PUSH_TRUE,						"pushTrue");
DECLARE_CONST_STRING(OP_PUSH_TYPE,						"pushType");
DECLARE_CONST_STRING(OP_RETURN,							"return");
DECLARE_CONST_STRING(OP_RETURN_FRAME,					"returnFrame");
DECLARE_CONST_STRING(OP_SET_ARRAY_ITEM,					"setArrayItem");
DECLARE_CONST_STRING(OP_SET_ARRAY_ITEM_I,				"setArrayItemI");
DECLARE_CONST_STRING(OP_SET_FOR_EACH_ITEM,				"setForEachItem");
DECLARE_CONST_STRING(OP_SET_FRAME_LOCAL,				"setFrameLocal");
DECLARE_CONST_STRING(OP_SET_GLOBAL,						"setGlobal");
DECLARE_CONST_STRING(OP_SET_GLOBAL_ITEM,				"setGlobalItem");
DECLARE_CONST_STRING(OP_SET_LOCAL,						"setLocal");
DECLARE_CONST_STRING(OP_SET_LOCAL_L0,					"setLocalL0");
DECLARE_CONST_STRING(OP_SET_LOCAL_ITEM,					"setLocalItem");
DECLARE_CONST_STRING(OP_SET_OBJECT_ITEM,				"setObjectItem");
DECLARE_CONST_STRING(OP_SET_OBJECT_ITEM2,				"setObjectItem2");
DECLARE_CONST_STRING(OP_SET_TENSOR_ITEM,				"setTensorItem");
DECLARE_CONST_STRING(OP_SET_TENSOR_ITEM_I,				"setTensorItemI");
DECLARE_CONST_STRING(OP_SUBTRACT,						"subtract");
DECLARE_CONST_STRING(OP_SUBTRACT2,						"subtract2");
DECLARE_CONST_STRING(OP_SUBTRACT_INT,					"subtractInt");

static SOpCodeInfo OPCODE_INFO[] = 
	{
	SOpCodeInfo(opNoOp,				OP_NO_OP,				operandNone	),

	//	Comparisons
	SOpCodeInfo(opIsEqual,			OP_IS_EQUAL,			operandIntShort ),
	SOpCodeInfo(opIsEqualInt,		OP_IS_EQUAL_INT,		operandIntShort ),
	SOpCodeInfo(opIsEqualMulti,		OP_IS_EQUAL_MULTI,		operandIntShort ),
	SOpCodeInfo(opIsIdentical,		OP_IS_IDENTICAL,		operandIntShort ),
	SOpCodeInfo(opIsIn,				OP_IS_IN,				operandIntShort ),
	SOpCodeInfo(opIsLess,			OP_IS_LESS,				operandIntShort ),
	SOpCodeInfo(opIsLessInt,		OP_IS_LESS_INT,			operandIntShort ),
	SOpCodeInfo(opIsLessMulti,		OP_IS_LESS_MULTI,		operandIntShort ),
	SOpCodeInfo(opIsGreater,		OP_IS_GREATER,			operandIntShort ),
	SOpCodeInfo(opIsGreaterInt,		OP_IS_GREATER_INT,		operandIntShort ),
	SOpCodeInfo(opIsGreaterMulti,	OP_IS_GREATER_MULTI,	operandIntShort ),
	SOpCodeInfo(opIsLessOrEqual,	OP_IS_LESS_OR_EQUAL,	operandIntShort ),
	SOpCodeInfo(opIsLessOrEqualInt,	OP_IS_LESS_OR_EQUAL_INT,	operandIntShort ),
	SOpCodeInfo(opIsLessOrEqualMulti,	OP_IS_LESS_OR_EQUAL_MULTI,	operandIntShort ),
	SOpCodeInfo(opIsGreaterOrEqual,	OP_IS_GREATER_OR_EQUAL,	operandIntShort ),
	SOpCodeInfo(opIsGreaterOrEqualInt,	OP_IS_GREATER_OR_EQUAL_INT,	operandIntShort ),
	SOpCodeInfo(opIsGreaterOrEqualMulti,	OP_IS_GREATER_OR_EQUAL_MULTI,	operandIntShort ),
	SOpCodeInfo(opIsNotEqual,		OP_IS_NOT_EQUAL,		operandIntShort ),
	SOpCodeInfo(opIsNotEqualInt,	OP_IS_NOT_EQUAL_INT,	operandIntShort ),
	SOpCodeInfo(opIsNotEqualMulti,	OP_IS_NOT_EQUAL_MULTI,	operandIntShort ),
	SOpCodeInfo(opIsNotIdentical,	OP_IS_NOT_IDENTICAL,	operandIntShort ),
	SOpCodeInfo(opIsNotIn,			OP_IS_NOT_IN,			operandIntShort ),
	SOpCodeInfo(opNot,				OP_NOT,					operandNone ),

	//	Control flow
	SOpCodeInfo(opJump,				OP_JUMP,				operandIntShort ),
	SOpCodeInfo(opJumpIfNil,		OP_JUMP_IF_NIL,			operandIntShort ),
	SOpCodeInfo(opJumpIfNilNoPop,	OP_JUMP_IF_NIL_NO_POP,	operandIntShort ),
	SOpCodeInfo(opJumpIfNotNilNoPop,OP_JUMP_IF_NOT_NIL_NO_POP,	operandIntShort ),
	SOpCodeInfo(opPop,				OP_POP,					operandIntShort ),
	SOpCodeInfo(opPopDeep,			OP_POP_DEEP,			operandIntShort ),
	SOpCodeInfo(opHexarcMsg,		OP_HEXARC_MSG,			operandNone ),
	SOpCodeInfo(opDebugBreak,		OP_DEBUG_BREAK,			operandNone ),
	SOpCodeInfo(opMakeBlockEnv,		OP_MAKE_BLOCK_ENV,		operandNone ),
	SOpCodeInfo(opMakeLocalEnv,		OP_MAKE_LOCAL_ENV,		operandIntShort ),
	SOpCodeInfo(opExitEnvAndJumpIfNil,	OP_EXIT_ENV_AND_JUMP_IF_NIL,	operandIntShort ),
	SOpCodeInfo(opExitEnvAndJumpIfGreaterInt,	OP_EXIT_ENV_AND_JUMP_IF_GREATER_INT,	operandIntShort ),
	SOpCodeInfo(opExitEnvAndJumpIfGreaterOrEqualInt,	OP_EXIT_ENV_AND_JUMP_IF_GREATER_OR_EQUAL_INT,	operandIntShort ),
	SOpCodeInfo(opExitEnvAndJumpIfLocalGreaterInt,	OP_EXIT_ENV_AND_JUMP_IF_LOCAL_GREATER_INT,	operandInt ),
	SOpCodeInfo(opLoopIncAndJump,	OP_LOOP_INC_AND_JUMP,	operandIntShort ),
	SOpCodeInfo(opLoopIncLocalAndJump,	OP_LOOP_INC_LOCAL_AND_JUMP,	operandIntShort ),

	//	Function calls
	SOpCodeInfo(opMakeFunc,			OP_MAKE_FUNC,			operandCodeOffset ),
	SOpCodeInfo(opMakeFunc2,		OP_MAKE_FUNC2,			operandCodeOffset ),
	SOpCodeInfo(opMakeEnv,			OP_MAKE_ENV,			operandIntShort ),
	SOpCodeInfo(opMakeApplyEnv,		OP_MAKE_APPLY_ENV,		operandIntShort ),
	SOpCodeInfo(opMakeMethodEnv,	OP_MAKE_METHOD_ENV,		operandIntShort ),
	SOpCodeInfo(opCall,				OP_CALL,				operandNone ),
	SOpCodeInfo(opCallDirect,		OP_CALL_DIRECT,			operandCodeOffset ),
	SOpCodeInfo(opCallDirectNoClosure,	OP_CALL_DIRECT_NO_CLOSURE,	operandCodeOffset ),
	SOpCodeInfo(opCallDirectSelfNoClosure,	OP_CALL_DIRECT_SELF_NO_CLOSURE,	operandCodeOffset ),
	SOpCodeInfo(opCallFrame,		OP_CALL_FRAME,			operandArgCountCodeOffset ),
	SOpCodeInfo(opCallFrameSelf,	OP_CALL_FRAME_SELF,		operandArgCountCodeOffset ),
	SOpCodeInfo(opCallLib,			OP_CALL_LIB,			operandLibCall ),
	SOpCodeInfo(opEnterEnv,			OP_ENTER_ENV,			operandNone ),
	SOpCodeInfo(opEnterStackFrame,	OP_ENTER_STACK_FRAME,	operandIntShort ),
	SOpCodeInfo(opDefineArg,		OP_DEFINE_ARG,			operandStringOffset ),
	SOpCodeInfo(opDefineArgFromCode,OP_DEFINE_ARG_FROM_CODE,operandIntShort ),
	SOpCodeInfo(opExitEnv,			OP_EXIT_ENV,			operandNone ),
	SOpCodeInfo(opReturn,			OP_RETURN,				operandNone ),
	SOpCodeInfo(opExitEnvAndReturn,	OP_EXIT_ENV_AND_RETURN,	operandNone ),
	SOpCodeInfo(opReturnFrame,		OP_RETURN_FRAME,		operandNone ),
	SOpCodeInfo(opMakePrimitive,	OP_MAKE_PRIMITIVE,		operandIntShort ),
	SOpCodeInfo(opMakeSpread,		OP_MAKE_SPREAD,			operandNone ),

	//	Local environment
	SOpCodeInfo(opAddLocalL0Int16,	OP_ADD_LOCAL_L0_INT16,	operandIntShort2 ),
	SOpCodeInfo(opIncLocalL0,		OP_INC_LOCAL_L0,		operandIntShort ),
	SOpCodeInfo(opPopLocal,			OP_POP_LOCAL,			operandIntShort ),
	SOpCodeInfo(opPopLocalL0,		OP_POP_LOCAL_L0,		operandIntShort ),
	SOpCodeInfo(opPushFrameArg,		OP_PUSH_FRAME_ARG,		operandIntShort ),
	SOpCodeInfo(opPushLocal,		OP_PUSH_LOCAL,			operandIntShort ),
	SOpCodeInfo(opPushLocalL0,		OP_PUSH_LOCAL_L0,		operandIntShort ),
	SOpCodeInfo(opSetFrameLocal,	OP_SET_FRAME_LOCAL,		operandIntShort ),
	SOpCodeInfo(opSetLocal,			OP_SET_LOCAL,			operandIntShort ),
	SOpCodeInfo(opSetLocalL0,		OP_SET_LOCAL_L0,		operandIntShort ),

	//	Global environment
	SOpCodeInfo(opDefine,			OP_DEFINE,				operandStringOffset ),
	SOpCodeInfo(opPushGlobal,		OP_PUSH_GLOBAL,			operandStringOffset ),
	SOpCodeInfo(opSetGlobal,		OP_SET_GLOBAL,			operandStringOffset ),

	//	Lists & Structs
	SOpCodeInfo(opAppendToArray,	OP_APPEND_TO_ARRAY,		operandIntShort ),
	SOpCodeInfo(opMakeArray,		OP_MAKE_ARRAY,			operandIntShort ),
	SOpCodeInfo(opMakeAsType,		OP_MAKE_AS_TYPE,		operandNone ),
	SOpCodeInfo(opMakeAsTypeCons,	OP_MAKE_AS_TYPE_CONS,	operandNone ),
	SOpCodeInfo(opMakeDatatype,		OP_MAKE_DATATYPE,		operandNone ),
	SOpCodeInfo(opMakeEmptyArray,	OP_MAKE_EMPTY_ARRAY,	operandIntShort ),
	SOpCodeInfo(opMakeEmptyArrayAsType,	OP_MAKE_EMPTY_ARRAY_AS_TYPE,	operandIntShort ),
	SOpCodeInfo(opMakeEmptyStruct,	OP_MAKE_EMPTY_STRUCT,	operandIntShort ),
	SOpCodeInfo(opMakeExpr,			OP_MAKE_EXPR,			operandIntShort2 ),
	SOpCodeInfo(opMakeExprIf,		OP_MAKE_EXPR_IF,		operandIntShort ),
	SOpCodeInfo(opMakeMapColExpr,	OP_MAKE_MAP_COL_EXPR,	operandNone ),
	SOpCodeInfo(opMakeRange,		OP_MAKE_RANGE,			operandIntShort ),
	SOpCodeInfo(opMakeObject,		OP_MAKE_OBJECT,			operandIntShort ),
	SOpCodeInfo(opMakeObjectDirect,	OP_MAKE_OBJECT_DIRECT,	operandIntShort ),
	SOpCodeInfo(opMakeObjectDirectUnchecked,	OP_MAKE_OBJECT_DIRECT_UNCHECKED,	operandIntShort ),
	SOpCodeInfo(opNewObject,		OP_NEW_OBJECT,			operandIntShort ),
	SOpCodeInfo(opMakeStruct,		OP_MAKE_STRUCT,			operandIntShort ),
	SOpCodeInfo(opMakeTensor,		OP_MAKE_TENSOR,			operandIntShort ),
	SOpCodeInfo(opMakeTensorType,	OP_MAKE_TENSOR_TYPE,	operandIntShort ),
	SOpCodeInfo(opPushTensorItem,	OP_PUSH_TENSOR_ITEM,	operandIntShort ),
	SOpCodeInfo(opPushTensorItemI,	OP_PUSH_TENSOR_ITEM_I,	operandIntShort ),
	SOpCodeInfo(opPushArrayItem,	OP_PUSH_ARRAY_ITEM,		operandIntShort ),
	SOpCodeInfo(opPushArrayItemI,	OP_PUSH_ARRAY_ITEM_I,	operandIntShort ),
	SOpCodeInfo(opPushObjectItem,	OP_PUSH_OBJECT_ITEM,	operandIntShort ),
	SOpCodeInfo(opPushObjectMethod,	OP_PUSH_OBJECT_METHOD,	operandIntShort ),
	SOpCodeInfo(opPushRecordSlotI,	OP_PUSH_RECORD_SLOT_I,	operandIntShort ),
	SOpCodeInfo(opPushLocalItem,	OP_PUSH_LOCAL_ITEM,		operandIntShort ),
	SOpCodeInfo(opPushLocalLength,	OP_PUSH_LOCAL_LENGTH,	operandIntShort ),
	SOpCodeInfo(opSetArrayItem,		OP_SET_ARRAY_ITEM,		operandIntShort ),
	SOpCodeInfo(opSetArrayItemI,	OP_SET_ARRAY_ITEM_I,	operandIntShort ),
	SOpCodeInfo(opSetTensorItem,	OP_SET_TENSOR_ITEM,		operandIntShort ),
	SOpCodeInfo(opSetTensorItemI,	OP_SET_TENSOR_ITEM_I,	operandIntShort ),
	SOpCodeInfo(opSetGlobalItem,	OP_SET_GLOBAL_ITEM,		operandStringOffset ),
	SOpCodeInfo(opSetLocalItem,		OP_SET_LOCAL_ITEM,		operandIntShort ),
	SOpCodeInfo(opSetObjectItem,	OP_SET_OBJECT_ITEM,		operandNone ),
	SOpCodeInfo(opSetObjectItem2,	OP_SET_OBJECT_ITEM2,	operandIntShort ),

	//	Literals
	SOpCodeInfo(opPushDatum,		OP_PUSH_DATUM,			operandStringOffset ),
	SOpCodeInfo(opPushNil,			OP_PUSH_NIL,			operandNone ),
	SOpCodeInfo(opPushFalse,		OP_PUSH_FALSE,			operandNone ),
	SOpCodeInfo(opPushTrue,			OP_PUSH_TRUE,			operandNone ),
	SOpCodeInfo(opPushNaN,			OP_PUSH_NAN,			operandNone ),
	SOpCodeInfo(opPushInt,			OP_PUSH_INT,			operandInt ),
	SOpCodeInfo(opPushIntShort,		OP_PUSH_INT_SHORT,		operandIntShort ),
	SOpCodeInfo(opPushStr,			OP_PUSH_STR,			operandStringOffset ),
	SOpCodeInfo(opPushStrNull,		OP_PUSH_STR_NULL,		operandNone ),
	SOpCodeInfo(opPushLiteral,		OP_PUSH_LITERAL,		operandIntShort ),
	SOpCodeInfo(opPushType,			OP_PUSH_TYPE,			operandStringOffset ),
	SOpCodeInfo(opPushCoreType,		OP_PUSH_CORE_TYPE,		operandIntShort ),

	//	Math
	SOpCodeInfo(opAdd,				OP_ADD,					operandIntShort ),
	SOpCodeInfo(opAdd2,				OP_ADD2,				operandNone ),
	SOpCodeInfo(opAddInt,			OP_ADD_INT,				operandIntShort ),
	SOpCodeInfo(opDivide,			OP_DIVIDE,				operandIntShort ),
	SOpCodeInfo(opDivide2,			OP_DIVIDE2,				operandNone ),
	SOpCodeInfo(opIncLocalInt,		OP_INC_LOCAL_INT,		operandIntShort ),
	SOpCodeInfo(opMod,				OP_MOD,					operandIntShort ),
	SOpCodeInfo(opMultiply,			OP_MULTIPLY,			operandIntShort ),
	SOpCodeInfo(opMultiply2,		OP_MULTIPLY2,			operandNone ),
	SOpCodeInfo(opNegate,			OP_NEGATE,				operandNone ),
	SOpCodeInfo(opPower,			OP_POWER,				operandIntShort ),
	SOpCodeInfo(opSubtract,			OP_SUBTRACT,			operandIntShort ),
	SOpCodeInfo(opSubtract2,		OP_SUBTRACT2,			operandNone ),
	SOpCodeInfo(opSubtractInt,		OP_SUBTRACT_INT,		operandIntShort ),
	SOpCodeInfo(opInc,				OP_INC,					operandIntShort ),
	SOpCodeInfo(opIncStep,			OP_INC_STEP,			operandNone ),

	//	Mutations

	SOpCodeInfo(opMutateArrayItemAdd,		OP_MUTATE_ARRAY_ITEM_ADD,		operandNone ),
	SOpCodeInfo(opMutateArrayItemConcat,	OP_MUTATE_ARRAY_ITEM_CONCAT,	operandNone ),
	SOpCodeInfo(opMutateArrayItemDivide,	OP_MUTATE_ARRAY_ITEM_DIVIDE,	operandNone ),
	SOpCodeInfo(opMutateArrayItemMod,		OP_MUTATE_ARRAY_ITEM_MOD,		operandNone ),
	SOpCodeInfo(opMutateArrayItemMultiply,	OP_MUTATE_ARRAY_ITEM_MULTIPLY,	operandNone ),
	SOpCodeInfo(opMutateArrayItemPower,		OP_MUTATE_ARRAY_ITEM_POWER,		operandNone ),
	SOpCodeInfo(opMutateArrayItemSubtract,	OP_MUTATE_ARRAY_ITEM_SUBTRACT,	operandNone ),
	SOpCodeInfo(opMutateObjectItemAdd,		OP_MUTATE_OBJECT_ITEM_ADD,		operandNone ),
	SOpCodeInfo(opMutateObjectItemConcat,	OP_MUTATE_OBJECT_ITEM_CONCAT,	operandNone ),
	SOpCodeInfo(opMutateObjectItemDivide,	OP_MUTATE_OBJECT_ITEM_DIVIDE,	operandNone ),
	SOpCodeInfo(opMutateObjectItemMod,		OP_MUTATE_OBJECT_ITEM_MOD,		operandNone ),
	SOpCodeInfo(opMutateObjectItemMultiply,	OP_MUTATE_OBJECT_ITEM_MULTIPLY,	operandNone ),
	SOpCodeInfo(opMutateObjectItemPower,	OP_MUTATE_OBJECT_ITEM_POWER,	operandNone ),
	SOpCodeInfo(opMutateObjectItemSubtract,	OP_MUTATE_OBJECT_ITEM_SUBTRACT,	operandNone ),
	SOpCodeInfo(opMutateGlobalAdd,			OP_MUTATE_GLOBAL_ADD,			operandStringOffset ),
	SOpCodeInfo(opMutateGlobalConcat,		OP_MUTATE_GLOBAL_CONCAT,		operandStringOffset ),
	SOpCodeInfo(opMutateGlobalDivide,		OP_MUTATE_GLOBAL_DIVIDE,		operandStringOffset ),
	SOpCodeInfo(opMutateGlobalMod,			OP_MUTATE_GLOBAL_MOD,			operandStringOffset ),
	SOpCodeInfo(opMutateGlobalMultiply,		OP_MUTATE_GLOBAL_MULTIPLY,		operandStringOffset ),
	SOpCodeInfo(opMutateGlobalPower,		OP_MUTATE_GLOBAL_POWER,			operandStringOffset ),
	SOpCodeInfo(opMutateGlobalSubtract,		OP_MUTATE_GLOBAL_SUBTRACT,		operandStringOffset ),
	SOpCodeInfo(opMutateLocalAdd,			OP_MUTATE_LOCAL_ADD,			operandIntShort ),
	SOpCodeInfo(opMutateLocalConcat,		OP_MUTATE_LOCAL_CONCAT,			operandIntShort ),
	SOpCodeInfo(opMutateLocalDivide,		OP_MUTATE_LOCAL_DIVIDE,			operandIntShort ),
	SOpCodeInfo(opMutateLocalMod,			OP_MUTATE_LOCAL_MOD,			operandIntShort ),
	SOpCodeInfo(opMutateLocalMultiply,		OP_MUTATE_LOCAL_MULTIPLY,		operandIntShort ),
	SOpCodeInfo(opMutateLocalPower,			OP_MUTATE_LOCAL_POWER,			operandIntShort ),
	SOpCodeInfo(opMutateLocalSubtract,		OP_MUTATE_LOCAL_SUBTRACT,		operandIntShort ),
	SOpCodeInfo(opMutateTensorItemAdd,		OP_MUTATE_TENSOR_ITEM_ADD,		operandIntShort ),
	SOpCodeInfo(opMutateTensorItemConcat,	OP_MUTATE_TENSOR_ITEM_CONCAT,	operandIntShort ),
	SOpCodeInfo(opMutateTensorItemDivide,	OP_MUTATE_TENSOR_ITEM_DIVIDE,	operandIntShort ),
	SOpCodeInfo(opMutateTensorItemMod,		OP_MUTATE_TENSOR_ITEM_MOD,		operandIntShort ),
	SOpCodeInfo(opMutateTensorItemMultiply,	OP_MUTATE_TENSOR_ITEM_MULTIPLY,	operandIntShort ),
	SOpCodeInfo(opMutateTensorItemPower,	OP_MUTATE_TENSOR_ITEM_POWER,	operandIntShort ),
	SOpCodeInfo(opMutateTensorItemSubtract,	OP_MUTATE_TENSOR_ITEM_SUBTRACT,	operandIntShort ),

	//	For Each

	SOpCodeInfo(opCompareForEach,	OP_COMPARE_FOR_EACH,	operandNone ),
	SOpCodeInfo(opCompareStep,		OP_COMPARE_STEP,		operandNone ),
	SOpCodeInfo(opSetForEachItem,	OP_SET_FOR_EACH_ITEM,	operandNone ),
	SOpCodeInfo(opIncForEach,		OP_INC_FOR_EACH,		operandIntShort ),
	SOpCodeInfo(opInitForEach,		OP_INIT_FOR_EACH,		operandNone ),
	SOpCodeInfo(opPushInitForEach,	OP_PUSH_INIT_FOR_EACH,	operandNone ),

	//	Miscellaneous
	SOpCodeInfo(opError,			OP_ERROR,				operandIntShort ),
	SOpCodeInfo(opHalt,				OP_HALT,				operandNone ),
	SOpCodeInfo(opMakeFlagsFromArray,	OP_MAKE_FLAGS_FROM_ARRAY,	operandNone ),
	SOpCodeInfo(opMapResult,		OP_MAP_RESULT,			operandIntShort ),
	SOpCodeInfo(opConcat,			OP_CONCAT,				operandNone ),
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
		case operandIntShort2:
		case operandCodeOffset:
		case operandStringOffset:
		case operandDatumOffset:
			return pPos + 1;

		case operandInt:
		case operandLibCall:
		case operandArgCountCodeOffset:
			return pPos + 2;

		default:
			ASSERT(false);
			return pPos + 1;
		}
	}
