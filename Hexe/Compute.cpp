//	Compute.cpp
//
//	Compute
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "intrin.h"

static constexpr DWORDLONG STOP_CHECK_COUNT =			0x4000;
static constexpr int LOOP_INDEX = 0;

DECLARE_CONST_STRING(FIELD_CONSTRUCTOR,					"constructor");
DECLARE_CONST_STRING(FIELD_LENGTH,						"length");
DECLARE_CONST_STRING(FIELD_MSG,							"msg");
DECLARE_CONST_STRING(FIELD_PAYLOAD,						"payload");
DECLARE_CONST_STRING(FIELD_RESULT,						"result");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");

DECLARE_CONST_STRING(TYPE_DIALOG_DEBUG_SIM,				"dialogDebugSim");
DECLARE_CONST_STRING(TYPE_EVENT_HANDLER_CALL,			"eventHandlerCall");

DECLARE_CONST_STRING(TYPENAME_HEXE_FUNCTION,			"hexeFunction");

DECLARE_CONST_STRING(ERR_COLON_EXPECTED,				"':' expected: %s.");
DECLARE_CONST_STRING(ERR_DIVISION_BY_ZERO,				"Divide by zero error.");
DECLARE_CONST_STRING(ERR_INVALID_KEY,					"Invalid key: %s.");
DECLARE_CONST_STRING(ERR_INVALID_OP_CODE,				"Invalid opcode.");
DECLARE_CONST_STRING(ERR_INVALID_PRIMITIVE_SUB,			"Invalid primitive subroutine: not a function.");
DECLARE_CONST_STRING(ERR_NOT_A_FUNCTION,				"Not a function: %s.");
DECLARE_CONST_STRING(ERR_OUT_OF_MEMORY,					"Out of memory.");
DECLARE_CONST_STRING(ERR_NOT_ARRAY_OR_STRUCT,			"Unable to set item: not an array or structure.");
DECLARE_CONST_STRING(ERR_UNBOUND_VARIABLE,				"Undefined identifier: %s.");
DECLARE_CONST_STRING(ERR_EXECUTION_TOOK_TOO_LONG,		"Execution took too long.");
DECLARE_CONST_STRING(ERR_DUPLICATE_VARIABLE,			"Duplicate definition: %s.");
DECLARE_CONST_STRING(ERR_INVALID_OPERAND_COUNT,			"Invalid operand count.");
DECLARE_CONST_STRING(ERR_CANT_SET_FUNCTION_MEMBER,		"Function objects are read-only.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_OP,				"Unsupported opcode.");
DECLARE_CONST_STRING(ERR_MEMBER_FUNCTION_NOT_FOUND,		"Object does not have member function: %s.");
DECLARE_CONST_STRING(ERR_EXCEEDED_ARRAY_LIMITS,			"Arrays cannot have more than %s elements.");
DECLARE_CONST_STRING(ERR_EXCEEDED_STRING_LIMITS,		"Strings cannot be more than %s bytes.");
DECLARE_CONST_STRING(ERR_DATATYPE_EXPECTED,				"Datatype expected: %s.");
DECLARE_CONST_STRING(ERR_TOO_MANY_ARGS,					"Too many arguments for function: %d.");
DECLARE_CONST_STRING(ERR_INVALID_CALL_TYPE,				"Invalid call type.");
DECLARE_CONST_STRING(ERR_EXCEEDED_RECURSION_LIMITS,		"Exceeded recursion limits.");
DECLARE_CONST_STRING(ERR_NEGATIVE_ARRAY_SIZE,			"Array size cannot be negative: %d.");
DECLARE_CONST_STRING(ERR_INVALID_ARRAY_DIMENSION,		"Invalid array dimension: %s.");

#ifdef DEBUG

inline void DebugCheck (bool bExpr) { if (!bExpr) throw CException(errFail); }

#else

inline void DebugCheck (bool bExpr) { }

#endif

CHexeProcess::ERun CHexeProcess::Execute (CDatum *retResult)

//	Execute
//
//	Main VM loop.

	{
	//	Set abort time

	if (m_Limits.iMaxExecutionTimeSec > 0)
		m_dwAbortTime = ::sysGetTickCount64() + ((DWORDLONG)1000 * m_Limits.iMaxExecutionTimeSec);
	else
		m_dwAbortTime = 0;

	//	Run

	if (m_bEnableHistogram)
		return ExecuteWithHistogram(retResult);

	else
		{
		DWORD dwStopCheck = STOP_CHECK_COUNT;

		while (true)
			{
			//	Execute opcode

			ERun iResult = (this->*m_INSTRUCTION[*m_pIP >> 24])(*retResult);

			//	If error or halt, then we're done

			if (iResult != ERun::Continue)
				{
				m_dwComputes += STOP_CHECK_COUNT - dwStopCheck;
				return iResult;
				}

			//	Check to make sure we're not in an infinite loop.

			if (--dwStopCheck == 0)
				{
				m_dwComputes += STOP_CHECK_COUNT;

				if (m_dwAbortTime && ::sysGetTickCount64() >= m_dwAbortTime)
					return ERun::StopCheck;

				if (m_pComputeProgress)
					{
					m_pComputeProgress->OnCompute(m_dwComputes, m_dwLibraryTime);
					if (m_pComputeProgress->OnAbortCheck())
						return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, *retResult);
					}

				CSmartLock Lock(m_cs);
				if (m_bSignalPause)
					{
					m_bSignalPause = false;
					return ERun::StopCheck;
					}

				dwStopCheck = STOP_CHECK_COUNT;
				}
			}
		}
	}

CHexeProcess::ERun CHexeProcess::ExecuteWithHistogram (CDatum *retResult)
	{
	while (true)
		{
		DWORDLONG dwStart = ::sysGetTickCount64();
		DWORD dwOpCode = *m_pIP;

		ERun iResult = (this->*m_INSTRUCTION[*m_pIP >> 24])(*retResult);

		m_Histogram.AddEntry(dwOpCode, (DWORD)::sysGetTicksElapsed(dwStart));

		if (iResult != ERun::Continue)
			{
			return iResult;
			}

		//	Track computes

		m_dwComputes++;

		//	Check to make sure we're not in an infinite loop.

		if ((m_dwComputes % STOP_CHECK_COUNT) == 0)
			{
			if (m_dwAbortTime && ::sysGetTickCount64() >= m_dwAbortTime)
				return ERun::StopCheck;

			if (m_pComputeProgress)
				{
				m_pComputeProgress->OnCompute(m_dwComputes, m_dwLibraryTime);
				if (m_pComputeProgress->OnAbortCheck())
					return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, *retResult);
				}

			CSmartLock Lock(m_cs);
			if (m_bSignalPause)
				{
				m_bSignalPause = false;
				return ERun::StopCheck;
				}
			}
		}
	}

void CHexeProcess::InitInstructionTable ()
	{
	m_INSTRUCTION[(DWORD)opNoOp >> 24] = &CHexeProcess::ExecuteNoOp;

	m_INSTRUCTION[(DWORD)opAdd >> 24] = &CHexeProcess::ExecuteAdd;
	m_INSTRUCTION[(DWORD)opAdd2 >> 24] = &CHexeProcess::ExecuteAdd2;
	m_INSTRUCTION[(DWORD)opAddInt >> 24] = &CHexeProcess::ExecuteAddInt;
	m_INSTRUCTION[(DWORD)opAppendToArray >> 24] = &CHexeProcess::ExecuteAppendToArray;
	m_INSTRUCTION[(DWORD)opCall >> 24] = &CHexeProcess::ExecuteCall;
	m_INSTRUCTION[(DWORD)opCallLib >> 24] = &CHexeProcess::ExecuteCallLib;
	m_INSTRUCTION[(DWORD)opCompareForEach >> 24] = &CHexeProcess::ExecuteCompareForEach;
	m_INSTRUCTION[(DWORD)opCompareStep >> 24] = &CHexeProcess::ExecuteCompareStep;
	m_INSTRUCTION[(DWORD)opConcat >> 24] = &CHexeProcess::ExecuteConcat;
	m_INSTRUCTION[(DWORD)opDebugBreak >> 24] = &CHexeProcess::ExecuteDebugBreak;
	m_INSTRUCTION[(DWORD)opDefine >> 24] = &CHexeProcess::ExecuteDefine;
	m_INSTRUCTION[(DWORD)opDefineArg >> 24] = &CHexeProcess::ExecuteDefineArg;
	m_INSTRUCTION[(DWORD)opDefineArgFromCode >> 24] = &CHexeProcess::ExecuteDefineArgFromCode;
	m_INSTRUCTION[(DWORD)opDivide >> 24] = &CHexeProcess::ExecuteDivide;
	m_INSTRUCTION[(DWORD)opDivide2 >> 24] = &CHexeProcess::ExecuteDivide2;
	m_INSTRUCTION[(DWORD)opEnterEnv >> 24] = &CHexeProcess::ExecuteEnterEnv;
	m_INSTRUCTION[(DWORD)opError >> 24] = &CHexeProcess::ExecuteError;
	m_INSTRUCTION[(DWORD)opExitEnv >> 24] = &CHexeProcess::ExecuteExitEnv;
	m_INSTRUCTION[(DWORD)opExitEnvAndJumpIfGreaterInt >> 24] = &CHexeProcess::ExecuteExitEnvAndJumpIfGreaterInt;
	m_INSTRUCTION[(DWORD)opExitEnvAndJumpIfGreaterOrEqualInt >> 24] = &CHexeProcess::ExecuteExitEnvAndJumpIfGreaterOrEqualInt;
	m_INSTRUCTION[(DWORD)opExitEnvAndJumpIfNil >> 24] = &CHexeProcess::ExecuteExitEnvAndJumpIfNil;
	m_INSTRUCTION[(DWORD)opExitEnvAndReturn >> 24] = &CHexeProcess::ExecuteExitEnvAndReturn;
	m_INSTRUCTION[(DWORD)opHalt >> 24] = &CHexeProcess::ExecuteHalt;
	m_INSTRUCTION[(DWORD)opHexarcMsg >> 24] = &CHexeProcess::ExecuteHexarcMsg;
	m_INSTRUCTION[(DWORD)opInc >> 24] = &CHexeProcess::ExecuteInc;
	m_INSTRUCTION[(DWORD)opIncForEach >> 24] = &CHexeProcess::ExecuteIncForEach;
	m_INSTRUCTION[(DWORD)opIncLocalInt >> 24] = &CHexeProcess::ExecuteIncLocalInt;
	m_INSTRUCTION[(DWORD)opIncLocalL0 >> 24] = &CHexeProcess::ExecuteIncLocalL0;
	m_INSTRUCTION[(DWORD)opIncStep >> 24] = &CHexeProcess::ExecuteIncStep;
	m_INSTRUCTION[(DWORD)opInitForEach >> 24] = &CHexeProcess::ExecuteInitForEach;
	m_INSTRUCTION[(DWORD)opIsEqual >> 24] = &CHexeProcess::ExecuteIsEqual;
	m_INSTRUCTION[(DWORD)opIsEqualInt >> 24] = &CHexeProcess::ExecuteIsEqualInt;
	m_INSTRUCTION[(DWORD)opIsEqualMulti >> 24] = &CHexeProcess::ExecuteIsEqualMulti;
	m_INSTRUCTION[(DWORD)opIsGreater >> 24] = &CHexeProcess::ExecuteIsGreater;
	m_INSTRUCTION[(DWORD)opIsGreaterInt >> 24] = &CHexeProcess::ExecuteIsGreaterInt;
	m_INSTRUCTION[(DWORD)opIsGreaterMulti >> 24] = &CHexeProcess::ExecuteIsGreaterMulti;
	m_INSTRUCTION[(DWORD)opIsGreaterOrEqual >> 24] = &CHexeProcess::ExecuteIsGreaterOrEqual;
	m_INSTRUCTION[(DWORD)opIsGreaterOrEqualInt >> 24] = &CHexeProcess::ExecuteIsGreaterOrEqualInt;
	m_INSTRUCTION[(DWORD)opIsGreaterOrEqualMulti >> 24] = &CHexeProcess::ExecuteIsGreaterOrEqualMulti;
	m_INSTRUCTION[(DWORD)opIsIdentical >> 24] = &CHexeProcess::ExecuteIsIdentical;
	m_INSTRUCTION[(DWORD)opIsIn >> 24] = &CHexeProcess::ExecuteIsIn;
	m_INSTRUCTION[(DWORD)opIsLess >> 24] = &CHexeProcess::ExecuteIsLess;
	m_INSTRUCTION[(DWORD)opIsLessInt >> 24] = &CHexeProcess::ExecuteIsLessInt;
	m_INSTRUCTION[(DWORD)opIsLessMulti >> 24] = &CHexeProcess::ExecuteIsLessMulti;
	m_INSTRUCTION[(DWORD)opIsLessOrEqual >> 24] = &CHexeProcess::ExecuteIsLessOrEqual;
	m_INSTRUCTION[(DWORD)opIsLessOrEqualInt >> 24] = &CHexeProcess::ExecuteIsLessOrEqualInt;
	m_INSTRUCTION[(DWORD)opIsLessOrEqualMulti >> 24] = &CHexeProcess::ExecuteIsLessOrEqualMulti;
	m_INSTRUCTION[(DWORD)opIsNotEqual >> 24] = &CHexeProcess::ExecuteIsNotEqual;
	m_INSTRUCTION[(DWORD)opIsNotEqualInt >> 24] = &CHexeProcess::ExecuteIsNotEqualInt;
	m_INSTRUCTION[(DWORD)opIsNotEqualMulti >> 24] = &CHexeProcess::ExecuteIsNotEqualMulti;
	m_INSTRUCTION[(DWORD)opIsNotIdentical >> 24] = &CHexeProcess::ExecuteIsNotIdentical;
	m_INSTRUCTION[(DWORD)opJump >> 24] = &CHexeProcess::ExecuteJump;
	m_INSTRUCTION[(DWORD)opJumpIfNil >> 24] = &CHexeProcess::ExecuteJumpIfNil;
	m_INSTRUCTION[(DWORD)opJumpIfNilNoPop >> 24] = &CHexeProcess::ExecuteJumpIfNilNoPop;
	m_INSTRUCTION[(DWORD)opJumpIfNotNilNoPop >> 24] = &CHexeProcess::ExecuteJumpIfNotNilNoPop;
	m_INSTRUCTION[(DWORD)opLoopIncAndJump >> 24] = &CHexeProcess::ExecuteLoopIncAndJump;
	m_INSTRUCTION[(DWORD)opMakeApplyEnv >> 24] = &CHexeProcess::ExecuteMakeApplyEnv;
	m_INSTRUCTION[(DWORD)opMakeArray >> 24] = &CHexeProcess::ExecuteMakeArray;
	m_INSTRUCTION[(DWORD)opMakeRange >> 24] = &CHexeProcess::ExecuteMakeRange;
	m_INSTRUCTION[(DWORD)opMakeAsType >> 24] = &CHexeProcess::ExecuteMakeAsType;
	m_INSTRUCTION[(DWORD)opMakeAsTypeCons >> 24] = &CHexeProcess::ExecuteMakeAsTypeCons;
	m_INSTRUCTION[(DWORD)opMakeBlockEnv >> 24] = &CHexeProcess::ExecuteMakeBlockEnv;
	m_INSTRUCTION[(DWORD)opMakeDatatype >> 24] = &CHexeProcess::ExecuteMakeDatatype;
	m_INSTRUCTION[(DWORD)opMakeEmptyArray >> 24] = &CHexeProcess::ExecuteMakeEmptyArray;
	m_INSTRUCTION[(DWORD)opMakeEmptyArrayAsType >> 24] = &CHexeProcess::ExecuteMakeEmptyArrayAsType;
	m_INSTRUCTION[(DWORD)opMakeEmptyStruct >> 24] = &CHexeProcess::ExecuteMakeEmptyStruct;
	m_INSTRUCTION[(DWORD)opMakeEnv >> 24] = &CHexeProcess::ExecuteMakeEnv;
	m_INSTRUCTION[(DWORD)opMakeExpr >> 24] = &CHexeProcess::ExecuteMakeExpr;
	m_INSTRUCTION[(DWORD)opMakeExprIf >> 24] = &CHexeProcess::ExecuteMakeExprIf;
	m_INSTRUCTION[(DWORD)opMakeFlagsFromArray >> 24] = &CHexeProcess::ExecuteMakeFlagsFromArray;
	m_INSTRUCTION[(DWORD)opMakeFunc >> 24] = &CHexeProcess::ExecuteMakeFunc;
	m_INSTRUCTION[(DWORD)opMakeFunc2 >> 24] = &CHexeProcess::ExecuteMakeFunc2;
	m_INSTRUCTION[(DWORD)opMakeLocalEnv >> 24] = &CHexeProcess::ExecuteMakeLocalEnv;
	m_INSTRUCTION[(DWORD)opMakeMapColExpr >> 24] = &CHexeProcess::ExecuteMakeMapColExpr;
	m_INSTRUCTION[(DWORD)opMakeMethodEnv >> 24] = &CHexeProcess::ExecuteMakeMethodEnv;
	m_INSTRUCTION[(DWORD)opMakeObject >> 24] = &CHexeProcess::ExecuteMakeObject;
	m_INSTRUCTION[(DWORD)opMakePrimitive >> 24] = &CHexeProcess::ExecuteMakePrimitive;
	m_INSTRUCTION[(DWORD)opMakeSpread >> 24] = &CHexeProcess::ExecuteMakeSpread;
	m_INSTRUCTION[(DWORD)opMakeStruct >> 24] = &CHexeProcess::ExecuteMakeStruct;
	m_INSTRUCTION[(DWORD)opMakeTensor >> 24] = &CHexeProcess::ExecuteMakeTensor;
	m_INSTRUCTION[(DWORD)opMakeTensorType >> 24] = &CHexeProcess::ExecuteMakeTensorType;
	m_INSTRUCTION[(DWORD)opMapResult >> 24] = &CHexeProcess::ExecuteMapResult;
	m_INSTRUCTION[(DWORD)opMod >> 24] = &CHexeProcess::ExecuteMod;
	m_INSTRUCTION[(DWORD)opMultiply >> 24] = &CHexeProcess::ExecuteMultiply;
	m_INSTRUCTION[(DWORD)opMultiply2 >> 24] = &CHexeProcess::ExecuteMultiply2;
	m_INSTRUCTION[(DWORD)opMutateArrayItemAdd >> 24] = &CHexeProcess::ExecuteMutateArrayItemAdd;
	m_INSTRUCTION[(DWORD)opMutateArrayItemConcat >> 24] = &CHexeProcess::ExecuteMutateArrayItemConcat;
	m_INSTRUCTION[(DWORD)opMutateArrayItemDivide >> 24] = &CHexeProcess::ExecuteMutateArrayItemDivide;
	m_INSTRUCTION[(DWORD)opMutateArrayItemMod >> 24] = &CHexeProcess::ExecuteMutateArrayItemMod;
	m_INSTRUCTION[(DWORD)opMutateArrayItemMultiply >> 24] = &CHexeProcess::ExecuteMutateArrayItemMultiply;
	m_INSTRUCTION[(DWORD)opMutateArrayItemPower >> 24] = &CHexeProcess::ExecuteMutateArrayItemPower;
	m_INSTRUCTION[(DWORD)opMutateArrayItemSubtract >> 24] = &CHexeProcess::ExecuteMutateArrayItemSubtract;
	m_INSTRUCTION[(DWORD)opMutateGlobalAdd >> 24] = &CHexeProcess::ExecuteMutateGlobalAdd;
	m_INSTRUCTION[(DWORD)opMutateGlobalConcat >> 24] = &CHexeProcess::ExecuteMutateGlobalConcat;
	m_INSTRUCTION[(DWORD)opMutateGlobalDivide >> 24] = &CHexeProcess::ExecuteMutateGlobalDivide;
	m_INSTRUCTION[(DWORD)opMutateGlobalMod >> 24] = &CHexeProcess::ExecuteMutateGlobalMod;
	m_INSTRUCTION[(DWORD)opMutateGlobalMultiply >> 24] = &CHexeProcess::ExecuteMutateGlobalMultiply;
	m_INSTRUCTION[(DWORD)opMutateGlobalPower >> 24] = &CHexeProcess::ExecuteMutateGlobalPower;
	m_INSTRUCTION[(DWORD)opMutateGlobalSubtract >> 24] = &CHexeProcess::ExecuteMutateGlobalSubtract;
	m_INSTRUCTION[(DWORD)opMutateLocalAdd >> 24] = &CHexeProcess::ExecuteMutateLocalAdd;
	m_INSTRUCTION[(DWORD)opMutateLocalConcat >> 24] = &CHexeProcess::ExecuteMutateLocalConcat;
	m_INSTRUCTION[(DWORD)opMutateLocalDivide >> 24] = &CHexeProcess::ExecuteMutateLocalDivide;
	m_INSTRUCTION[(DWORD)opMutateLocalMod >> 24] = &CHexeProcess::ExecuteMutateLocalMod;
	m_INSTRUCTION[(DWORD)opMutateLocalMultiply >> 24] = &CHexeProcess::ExecuteMutateLocalMultiply;
	m_INSTRUCTION[(DWORD)opMutateLocalPower >> 24] = &CHexeProcess::ExecuteMutateLocalPower;
	m_INSTRUCTION[(DWORD)opMutateLocalSubtract >> 24] = &CHexeProcess::ExecuteMutateLocalSubtract;
	m_INSTRUCTION[(DWORD)opMutateObjectItemAdd >> 24] = &CHexeProcess::ExecuteMutateObjectItemAdd;
	m_INSTRUCTION[(DWORD)opMutateObjectItemConcat >> 24] = &CHexeProcess::ExecuteMutateObjectItemConcat;
	m_INSTRUCTION[(DWORD)opMutateObjectItemDivide >> 24] = &CHexeProcess::ExecuteMutateObjectItemDivide;
	m_INSTRUCTION[(DWORD)opMutateObjectItemMod >> 24] = &CHexeProcess::ExecuteMutateObjectItemMod;
	m_INSTRUCTION[(DWORD)opMutateObjectItemMultiply >> 24] = &CHexeProcess::ExecuteMutateObjectItemMultiply;
	m_INSTRUCTION[(DWORD)opMutateObjectItemPower >> 24] = &CHexeProcess::ExecuteMutateObjectItemPower;
	m_INSTRUCTION[(DWORD)opMutateObjectItemSubtract >> 24] = &CHexeProcess::ExecuteMutateObjectItemSubtract;
	m_INSTRUCTION[(DWORD)opMutateTensorItemAdd >> 24] = &CHexeProcess::ExecuteMutateTensorItemAdd;
	m_INSTRUCTION[(DWORD)opMutateTensorItemConcat >> 24] = &CHexeProcess::ExecuteMutateTensorItemConcat;
	m_INSTRUCTION[(DWORD)opMutateTensorItemDivide >> 24] = &CHexeProcess::ExecuteMutateTensorItemDivide;
	m_INSTRUCTION[(DWORD)opMutateTensorItemMod >> 24] = &CHexeProcess::ExecuteMutateTensorItemMod;
	m_INSTRUCTION[(DWORD)opMutateTensorItemMultiply >> 24] = &CHexeProcess::ExecuteMutateTensorItemMultiply;
	m_INSTRUCTION[(DWORD)opMutateTensorItemPower >> 24] = &CHexeProcess::ExecuteMutateTensorItemPower;
	m_INSTRUCTION[(DWORD)opMutateTensorItemSubtract >> 24] = &CHexeProcess::ExecuteMutateTensorItemSubtract;
	m_INSTRUCTION[(DWORD)opNegate >> 24] = &CHexeProcess::ExecuteNegate;
	m_INSTRUCTION[(DWORD)opNewObject >> 24] = &CHexeProcess::ExecuteNewObject;
	m_INSTRUCTION[(DWORD)opNot >> 24] = &CHexeProcess::ExecuteNot;
	m_INSTRUCTION[(DWORD)opPop >> 24] = &CHexeProcess::ExecutePop;
	m_INSTRUCTION[(DWORD)opPopDeep >> 24] = &CHexeProcess::ExecutePopDeep;
	m_INSTRUCTION[(DWORD)opPopLocal >> 24] = &CHexeProcess::ExecutePopLocal;
	m_INSTRUCTION[(DWORD)opPopLocalL0 >> 24] = &CHexeProcess::ExecutePopLocalL0;
	m_INSTRUCTION[(DWORD)opPower >> 24] = &CHexeProcess::ExecutePower;
	m_INSTRUCTION[(DWORD)opPushArrayItem >> 24] = &CHexeProcess::ExecutePushArrayItem;
	m_INSTRUCTION[(DWORD)opPushArrayItemI >> 24] = &CHexeProcess::ExecutePushArrayItemI;
	m_INSTRUCTION[(DWORD)opPushCoreType >> 24] = &CHexeProcess::ExecutePushCoreType;
	m_INSTRUCTION[(DWORD)opPushDatum >> 24] = &CHexeProcess::ExecutePushDatum;
	m_INSTRUCTION[(DWORD)opPushFalse >> 24] = &CHexeProcess::ExecutePushFalse;
	m_INSTRUCTION[(DWORD)opPushGlobal >> 24] = &CHexeProcess::ExecutePushGlobal;
	m_INSTRUCTION[(DWORD)opPushInitForEach >> 24] = &CHexeProcess::ExecutePushInitForEach;
	m_INSTRUCTION[(DWORD)opPushInt >> 24] = &CHexeProcess::ExecutePushInt;
	m_INSTRUCTION[(DWORD)opPushIntShort >> 24] = &CHexeProcess::ExecutePushIntShort;
	m_INSTRUCTION[(DWORD)opPushLiteral >> 24] = &CHexeProcess::ExecutePushLiteral;
	m_INSTRUCTION[(DWORD)opPushLocal >> 24] = &CHexeProcess::ExecutePushLocal;
	m_INSTRUCTION[(DWORD)opPushLocalItem >> 24] = &CHexeProcess::ExecutePushLocalItem;
	m_INSTRUCTION[(DWORD)opPushLocalL0 >> 24] = &CHexeProcess::ExecutePushLocalL0;
	m_INSTRUCTION[(DWORD)opPushLocalLength >> 24] = &CHexeProcess::ExecutePushLocalLength;
	m_INSTRUCTION[(DWORD)opPushNaN >> 24] = &CHexeProcess::ExecutePushNaN;
	m_INSTRUCTION[(DWORD)opPushNil >> 24] = &CHexeProcess::ExecutePushNil;
	m_INSTRUCTION[(DWORD)opPushObjectItem >> 24] = &CHexeProcess::ExecutePushObjectItem;
	m_INSTRUCTION[(DWORD)opPushObjectMethod >> 24] = &CHexeProcess::ExecutePushObjectMethod;
	m_INSTRUCTION[(DWORD)opPushStr >> 24] = &CHexeProcess::ExecutePushStr;
	m_INSTRUCTION[(DWORD)opPushStrNull >> 24] = &CHexeProcess::ExecutePushStrNull;
	m_INSTRUCTION[(DWORD)opPushTensorItem >> 24] = &CHexeProcess::ExecutePushTensorItem;
	m_INSTRUCTION[(DWORD)opPushTensorItemI >> 24] = &CHexeProcess::ExecutePushTensorItemI;
	m_INSTRUCTION[(DWORD)opPushTrue >> 24] = &CHexeProcess::ExecutePushTrue;
	m_INSTRUCTION[(DWORD)opPushType >> 24] = &CHexeProcess::ExecutePushType;
	m_INSTRUCTION[(DWORD)opReturn >> 24] = &CHexeProcess::ExecuteReturn;
	m_INSTRUCTION[(DWORD)opSetArrayItem >> 24] = &CHexeProcess::ExecuteSetArrayItem;
	m_INSTRUCTION[(DWORD)opSetArrayItemI >> 24] = &CHexeProcess::ExecuteSetArrayItemI;
	m_INSTRUCTION[(DWORD)opSetForEachItem >> 24] = &CHexeProcess::ExecuteSetForEachItem;
	m_INSTRUCTION[(DWORD)opSetGlobal >> 24] = &CHexeProcess::ExecuteSetGlobal;
	m_INSTRUCTION[(DWORD)opSetGlobalItem >> 24] = &CHexeProcess::ExecuteSetGlobalItem;
	m_INSTRUCTION[(DWORD)opSetLocal >> 24] = &CHexeProcess::ExecuteSetLocal;
	m_INSTRUCTION[(DWORD)opSetLocalItem >> 24] = &CHexeProcess::ExecuteSetLocalItem;
	m_INSTRUCTION[(DWORD)opSetLocalL0 >> 24] = &CHexeProcess::ExecuteSetLocalL0;
	m_INSTRUCTION[(DWORD)opSetObjectItem >> 24] = &CHexeProcess::ExecuteSetObjectItem;
	m_INSTRUCTION[(DWORD)opSetObjectItem2 >> 24] = &CHexeProcess::ExecuteSetObjectItem2;
	m_INSTRUCTION[(DWORD)opSetTensorItem >> 24] = &CHexeProcess::ExecuteSetTensorItem;
	m_INSTRUCTION[(DWORD)opSetTensorItemI >> 24] = &CHexeProcess::ExecuteSetTensorItemI;
	m_INSTRUCTION[(DWORD)opSubtract >> 24] = &CHexeProcess::ExecuteSubtract;
	m_INSTRUCTION[(DWORD)opSubtract2 >> 24] = &CHexeProcess::ExecuteSubtract2;
	m_INSTRUCTION[(DWORD)opSubtractInt >> 24] = &CHexeProcess::ExecuteSubtractInt;
	}

CHexeProcess::ERun CHexeProcess::ExecuteAdd (CDatum& retResult)
	{
	//	NOTE: This is the compatible version of add, which is used
	//	by the HexeLisp interpreter. The GridLang version uses opAdd2.

	int iCount = GetOperand(*m_pIP);

	if (iCount == 2)
		{
		CDatum dB = m_Stack.Pop();
		CDatum dA = m_Stack.Pop();
		m_Stack.Push(ExecuteOpAddCompatible(dA, dB, m_bAddConcatenatesStrings));
		}
	else if (iCount == 0)
		m_Stack.Push(CDatum(0));
	else
		{
		CNumberValue Result(m_Stack.Pop());

		for (int i = 1; i < iCount; i++)
			Result.Add(m_Stack.Pop());

		m_Stack.Push(Result.GetDatum());
		}

	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteAdd2 (CDatum& retResult)
	{
	CDatum dB = m_Stack.Pop();
	CDatum dA = m_Stack.Pop();

	m_Stack.Push(CAEONOp::Add(dA, dB));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteAddInt (CDatum& retResult)
	{
	//	Param is guaranteed to be Int32
	LONGLONG iResult = (LONGLONG)m_Stack.Get().raw_GetInt32() + (LONGLONG)GetOperand(*m_pIP);
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		m_Stack.GetRef() = CDatum((int)iResult);
	else
		m_Stack.GetRef() = CDatum(CIPInteger(iResult));

	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteAppendToArray (CDatum& retResult)
	{
	//	We expect n elements on the stack and before that, the 
	//	array to append to. We pop all n elements and append them
	//	to the array (in reverse order, bottom-most first).
	//
	//	We leave the array on the stack.

	int iCount = GetOperand(*m_pIP);
	CDatum dValue = m_Stack.Get(iCount);
	ExecuteMakeArrayFromStack(dValue, iCount);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteCall (CDatum& retResult)
	{
	CDatum dNewExpression = m_Stack.Pop();
				
	//	If the function changes the instruction pointer, then set up 
	//	the call context here.

	DWORD *pNewIP;
	CDatum dNewCodeBank;
	CDatum::ECallType iCallType = dNewExpression.GetCallInfo(&dNewCodeBank, &pNewIP);

	//	Handle it

	switch (iCallType)
		{
		case CDatum::ECallType::Call:
			{
			if (m_CallStack.GetCount() >= m_Limits.iMaxStackDepth)
				return RuntimeError(ERR_EXCEEDED_RECURSION_LIMITS, retResult);

			m_CallStack.PushFunCall(m_dExpression, m_dCodeBank, ++m_pIP);
			m_dExpression = dNewExpression;
			SetCodeBank(dNewCodeBank);

			m_pIP = pNewIP;
			break;
			}

		case CDatum::ECallType::CachedCall:
			{
			//	See if the function has a cached value.

			SAEONInvokeResult Result;
			CDatum::InvokeResult iResult = dNewExpression.Invoke(this, m_Env.GetLocalEnv(), m_UserSecurity.GetExecutionRights(), Result);
			if (iResult == CDatum::InvokeResult::ok)
				{
				//	Restore the environment

				m_Env.PopFrame();

				//	Done

				m_Stack.Push(Result.dResult);
				m_pIP++;
				}

			//	Otherwise, execute the function as normal.

			else
				{
				if (m_CallStack.GetCount() >= m_Limits.iMaxStackDepth)
					return RuntimeError(ERR_EXCEEDED_RECURSION_LIMITS, retResult);

				m_CallStack.PushFunCall(m_dExpression, m_dCodeBank, ++m_pIP);
				m_dExpression = dNewExpression;
				SetCodeBank(dNewCodeBank);

				m_pIP = pNewIP;
				}

			break;
			}

		case CDatum::ECallType::Library:
			{
			DWORDLONG dwStart = ::sysGetTickCount64();

			SAEONInvokeResult Result;
			CDatum::InvokeResult iResult = dNewExpression.Invoke(this, m_Env.GetLocalEnv(), m_UserSecurity.GetExecutionRights(), Result);

			m_dwLibraryTime += ::sysGetTickCount64() - dwStart;

			if (iResult != CDatum::InvokeResult::ok)
				{
				ERun iRunResult = ExecuteHandleInvokeResult(iResult, dNewExpression, Result, &retResult);
				if (iRunResult == ERun::OK)
					break;

				return iRunResult;
				}

			//	Restore the environment

			m_Env.PopFrame();

			//	Done

			m_Stack.Push(Result.dResult);
			m_pIP++;
			break;
			}

		case CDatum::ECallType::Invoke:
			{
			//	The first argument is the message

			CDatum dMsg = m_Env.GetLocalEnv().GetElement(0);

			//	Make a payload out of the arguments

			CDatum dArray(CDatum::typeArray);
			for (int i = 1; i < m_Env.GetLocalEnv().GetCount(); i++)
				dArray.Append(m_Env.GetLocalEnv().GetElement(i));

			//	Restore the environment and advance IP

			m_Env.PopFrame();
			m_pIP++;

			//	Make sure we haven't exceeded our execution time.

			if (m_dwAbortTime && ::sysGetTickCount64() >= m_dwAbortTime)
				return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, retResult);

			//	Send the message

			CDatum dValue;
			if (!SendHexarcMessage(dMsg.AsStringView(), dArray, &dValue))
				{
				retResult = dValue;
				return ERun::Error;
				}

			//	Async request

			retResult = dValue;
			return ERun::AsyncRequest;
			}

		default:
			return RuntimeError(strPattern(ERR_NOT_A_FUNCTION, dNewExpression.AsString()), retResult);
		}

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteCallLib (CDatum& retResult)
	{
	//	The operand is the number of arguments and following instruction is the
	//	global variable ID.

	int iArgCount = GetOperand(*m_pIP);
	m_pIP++;
	int iIndex = (int)(*m_pIP);
	m_pIP++;

	DWORD dwID = m_GlobalEnvCache.GetID(iIndex);
	if (dwID == CHexeGlobalEnvCache::INVALID_ID)
		{
		if (!m_Env.GetGlobalEnv().FindSymbol(m_pCodeBank->GetStringLiteral(iIndex), &dwID))
			return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(iIndex)), retResult);

		m_GlobalEnvCache.SetID(iIndex, dwID);
		}

	CDatum dFunc = m_Env.GetGlobalEnv().GetAt(dwID);

	//	Compute the stack pointer. The top of the stack is the last argument,
	//	so we want to back up by iArgCount.

	CDatum *pTop = &m_Stack.GetRef();
	CDatum* pStart = pTop - iArgCount + 1;
	CHexeStackEnv LocalEnv(pStart, iArgCount);

	//	Invoke

	DWORDLONG dwStart = ::sysGetTickCount64();

	SAEONInvokeResult Result;
	CDatum::InvokeResult iResult = dFunc.InvokeLibrary(*this, LocalEnv, m_UserSecurity.GetExecutionRights(), Result);

	m_dwLibraryTime += ::sysGetTickCount64() - dwStart;

	//	No matter what, we pop the arguments

	m_Stack.Pop(iArgCount);

	//	Handle the result

	if (iResult != CDatum::InvokeResult::ok)
		{
		ERun iRunResult = ExecuteHandleInvokeResult(iResult, dFunc, Result, &retResult, FLAG_NO_ADVANCE | FLAG_NO_ENV);
		if (iRunResult == ERun::OK)
			return ERun::Continue;
		else
			return iRunResult;
		}

	//	Done

	m_Stack.Push(Result.dResult);

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteCompareForEach (CDatum& retResult)
	{
	//	In this scope we have three variables:
	//
	//	0: The list element (user-defined)
	//	1: The array index ($i)
	//	2: The array ($array)
	//
	//	We compare the array index against the length of the array.
	//	If less, then we push true; otherwise nil.

	CDatum dIndex = m_Env.GetLocalEnv().GetArgument(LOCAL_INDEX_VAR);
	m_Stack.Push(CDatum(!dIndex.IsNil()));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteCompareStep (CDatum& retResult)
	{
	//	The stack has (in reverse order)
	//
	//	0: step value
	//	1: to value
	//	2: loop index.
	//
	//	If step value is positive, then we check to see if the loop 
	//	value is less that or equal to to. Otherwise, we check to
	//	see if it is greater than or equal to to. We push the result
	//	(either true or nil) on the stack and leave the other values
	//	alone.

	CNumberValue Step(m_Stack.Get());
	CNumberValue To(m_Stack.Get(1));
	CNumberValue I(m_Stack.Get(2));
	int iCompare = I.Compare(To);

	if (Step.IsNegative())
		{
		//	Is loop index >= to? Then continue looping.

		if (iCompare >= 0)
			m_Stack.Push(CDatum(true));
		else
			m_Stack.Push(CDatum(false));
		}
	else
		{
		//	If loop index <= to? Then continue looping.

		if (iCompare <= 0)
			m_Stack.Push(CDatum(true));
		else
			m_Stack.Push(CDatum(false));
		}
				
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteConcat (CDatum& retResult)
	{
	CDatum dB = m_Stack.Pop();
	CDatum dA = m_Stack.Pop();

	CDatum dResult = CAEONOp::Concatenate(*this, dA, dB);
	if (dResult.IsError())
		{
		retResult = dResult;
		return ERun::Error;
		}

	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteDebugBreak (CDatum& retResult)
	{
#ifdef DEBUG
	::DebugBreak();
#endif
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteDefine (CDatum& retResult)
	{
	//	NOTE: We leave the value of the definition on the stack
	if (!m_Env.GetGlobalEnv().AddSymbol(m_pCodeBank->GetString(GetOperand(*m_pIP)), m_Stack.Get()))
		return RuntimeError(strPattern(ERR_DUPLICATE_VARIABLE, m_pCodeBank->GetString(GetOperand(*m_pIP))), retResult);

	m_pIP++;
	
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteDefineArg (CDatum& retResult)
	{
	m_Env.GetLocalEnv().SetNextArgKey(m_pCodeBank->GetString(GetOperand(*m_pIP)));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteDefineArgFromCode (CDatum& retResult)
	{
	m_Env.GetLocalEnv().SetNextArgKey(GetStringFromDataBlock(GetOperand(*m_pIP)));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteDefineNextArg (CDatum& retResult)
	{
	m_Env.GetLocalEnv().SetNextArgKey(NULL_STR);
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteDivide (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);

	if (iCount == 2)
		{
		CDatum dDivisor = m_Stack.Pop();
		CNumberValue Dividend(m_Stack.Pop());
		if (!Dividend.Divide(dDivisor))
			return RuntimeError(ERR_DIVISION_BY_ZERO, retResult);

		m_Stack.Push(Dividend.GetDatum());
		}
	else if (iCount == 1)
		{
		CDatum dDivisor = m_Stack.Pop();
		CNumberValue Dividend(1.0);
		if (!Dividend.Divide(dDivisor))
			return RuntimeError(ERR_DIVISION_BY_ZERO, retResult);

		m_Stack.Push(Dividend.GetDatum());
		}
	else if (iCount < 1)
		m_Stack.Push(0);
	else
		{
		CNumberValue Result(m_Stack.Pop());
		for (int i = 2; i < iCount; i++)
			Result.Multiply(m_Stack.Pop());

		if (!Result.DivideReversed(m_Stack.Pop()))
			return RuntimeError(ERR_DIVISION_BY_ZERO, retResult);

		m_Stack.Push(Result.GetDatum());
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteDivide2 (CDatum& retResult)
	{
	CDatum dDivisor = m_Stack.Pop();
	CDatum dDividend = m_Stack.Pop();

	if (dDivisor.GetBasicType() == CDatum::typeDouble && dDividend.GetBasicType() == CDatum::typeDouble)
		{
		double rDivisor = dDivisor;
		if (rDivisor == 0.0)
			m_Stack.Push(CDatum::CreateNaN());
		else
			m_Stack.Push((double)dDividend / rDivisor);
		}
	else
		m_Stack.Push(CAEONOp::Divide(dDividend, dDivisor));

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteEnterEnv (CDatum& retResult)
	{
	CHexeFunction *pFunction = CHexeFunction::Upconvert(m_dExpression);

	//	Set the global environment to match the function

	CHexeGlobalEnvironment *pGlobalEnv = pFunction->GetGlobalEnvPointer();
	if (pGlobalEnv)
		m_Env.SetGlobalEnv(pFunction->GetGlobalEnv(), pGlobalEnv);

	//	Set the local environment of the function

	m_Env.SetLocalEnvParent(pFunction->GetLocalEnv());
	m_Env.GetLocalEnv().ResetNextArg();

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteError (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);

	CDatum dErrorDesc = (iCount > 1 ? m_Stack.Pop() : CDatum());
	CDatum dErrorCode = (iCount > 0 ? m_Stack.Pop() : CDatum());
	retResult = CDatum::CreateError(dErrorDesc.AsString(), dErrorCode.AsString());

	return ERun::Error;
	}

CHexeProcess::ERun CHexeProcess::ExecuteExitEnv (CDatum& retResult)
	{
	m_Env.PopFrame();
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteExitEnvAndJumpIfGreaterInt (CDatum& retResult)
	{
	bool bJump = m_Stack.Get(1).raw_GetInt32() > m_Stack.Get().raw_GetInt32();
	m_Stack.Pop(2);

	if (bJump)
		{
		m_Env.PopFrame();
		m_pIP += CHexeCode::GetOperandInt(*m_pIP);
		}
	else
		m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteExitEnvAndJumpIfGreaterOrEqualInt (CDatum& retResult)
	{
	bool bJump = (int)m_Stack.Get(1) >= (int)m_Stack.Get();
	m_Stack.Pop(2);

	if (bJump)
		{
		m_Env.PopFrame();
		m_pIP += CHexeCode::GetOperandInt(*m_pIP);
		}
	else
		m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteExitEnvAndJumpIfNil (CDatum& retResult)
	{
	if (m_Stack.Pop().IsNil())
		{
		m_Env.PopFrame();
		m_pIP += CHexeCode::GetOperandInt(*m_pIP);
		}
	else
		m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteExitEnvAndReturn (CDatum& retResult)
	{
	//	If this function needs to be cached, then we cache it now.

	if (m_dExpression.GetCallInfo() == CDatum::ECallType::CachedCall)
		{
		CDatum dResult = m_Stack.Get();
		m_dExpression.CacheInvokeResult(m_Env.GetLocalEnv(), dResult);

		//	We need to mark this as copy-on-write; otherwise callers would be
		//	modifying our cached value.
		//	LATER: This should be handled inside CacheInvokeResult

		m_Stack.Set(dResult.Clone(CDatum::EClone::CopyOnWrite));
		}

	//	Exit environment.

	m_Env.PopFrame();

	//	Restore

	return ExecuteReturn(retResult);
	}

CHexeProcess::ERun CHexeProcess::ExecuteHalt (CDatum& retResult)
	{
	retResult = m_Stack.Pop();

#ifdef DEBUG_HISTOGRAM
	if (m_bEnableHistogram)
		{
		m_Histogram.DebugOutput();
		}
#endif
	return ERun::OK;
	}

CHexeProcess::ERun CHexeProcess::ExecuteHexarcMsg (CDatum& retResult)
	{
	CDatum dPayload = m_Stack.Pop();
	CDatum dMsg = m_Stack.Pop();

	m_pIP++;

	//	Make sure we haven't exceeded our execution time.

	if (m_dwAbortTime && ::sysGetTickCount64() >= m_dwAbortTime)
		return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, retResult);

	//	Send the message

	CDatum dValue;
	if (!SendHexarcMessage(dMsg.AsStringView(), dPayload, &dValue))
		{
		retResult = dValue;
		return ERun::Error;
		}

	//	Async request

	retResult = dValue;
	return ERun::AsyncRequest;
	}

CHexeProcess::ERun CHexeProcess::ExecuteInc (CDatum& retResult)
	{
	m_Stack.GetRef().MutateAdd(CHexeCode::GetOperandInt(*m_pIP));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIncForEach (CDatum& retResult)
	{
	CDatum dArray = m_Env.GetLocalEnv().GetArgument(LOCAL_ARRAY_VAR);
	CDatum dIndex = m_Env.GetLocalEnv().GetArgument(LOCAL_INDEX_VAR);
	m_Env.GetLocalEnv().SetArgumentValue(LOCAL_INDEX_VAR, dArray.IteratorNext(dIndex));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIncLocalInt (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	CDatum dValue = m_Env.GetLocalEnv().GetArgument((dwOperand >> 8), (dwOperand & 0xff));

	m_Env.GetLocalEnv().SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), CDatum((int)dValue + 1));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIncLocalL0 (CDatum& retResult)
	{
	m_Env.GetLocalEnv().IncArgumentValue(GetOperand(*m_pIP), 1);
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIncStep (CDatum& retResult)
	{
	//	Pop the step

	CDatum dStep = m_Stack.Pop();

	//	Pop the to value (which we no longer need)

	m_Stack.Pop();

	//	Pop the current loop index value

	CDatum dI(m_Stack.Pop());

	//	Increment

	m_Stack.Push(CAEONOp::Add(dI, dStep));

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteInitForEach (CDatum& retResult)
	{
	CDatum dArray = m_Env.GetLocalEnv().GetArgument(LOCAL_ARRAY_VAR);
	m_Env.GetLocalEnv().SetArgumentValue(LOCAL_INDEX_VAR, dArray.IteratorBegin());
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsEqual (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompEqual(*this, m_Stack.Get(1), m_Stack.Get(0));
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsEqualInt (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CDatum((int)m_Stack.Get() == (int)m_Stack.Get(1));
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsEqualMulti (CDatum& retResult)
	{
	bool bCondition;
	int iCount = GetOperand(*m_pIP);
	if (iCount == 0)
		bCondition = true;
	else
		{
		bCondition = true;
		CDatum dValue = m_Stack.Pop();

		for (int i = 1; i < iCount; i++)
			{
			CDatum dValue2 = m_Stack.Pop();
			if (bCondition && !ExecuteIsEquivalent(dValue, dValue2))
				bCondition = false;
			}
		}

	m_Stack.Push(CDatum(bCondition));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsGreater (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompGreaterThan(*this, m_Stack.Get(1), m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsGreaterInt (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CDatum((int)m_Stack.Get(1) > (int)m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsGreaterMulti (CDatum& retResult)
	{
	bool bCondition;

	int iCount = GetOperand(*m_pIP);
	if (iCount == 0)
		bCondition = false;
	else if (iCount == 1)
		bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) > 0);
	else
		{
		bCondition = true;
		CDatum dValue = m_Stack.Pop();

		for (int i = 1; i < iCount; i++)
			{
			CDatum dValue2 = m_Stack.Pop();
			if (bCondition && ExecuteCompare(dValue, dValue2) >= 0)
				bCondition = false;

			dValue = dValue2;
			}
		}

	m_Stack.Push(CDatum(bCondition));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsGreaterOrEqual (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompGreaterThanOrEqual(*this, m_Stack.Get(1), m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsGreaterOrEqualInt (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CDatum((int)m_Stack.Get(1) >= (int)m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsGreaterOrEqualMulti (CDatum& retResult)
	{
	bool bCondition;
	int iCount = GetOperand(*m_pIP);
	if (iCount == 0)
		bCondition = true;
	else if (iCount == 1)
		bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) >= 0);
	else
		{
		bCondition = true;
		CDatum dValue = m_Stack.Pop();

		for (int i = 1; i < iCount; i++)
			{
			CDatum dValue2 = m_Stack.Pop();
			if (bCondition && ExecuteCompare(dValue, dValue2) > 0)
				bCondition = false;

			dValue = dValue2;
			}
		}

	m_Stack.Push(CDatum(bCondition));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsIdentical (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompIdentical(*this, m_Stack.Get(1), m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsIn (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = m_Stack.Get(0).OpContains(m_Stack.Get(1));
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsLess (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompLessThan(*this, m_Stack.Get(1), m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsLessInt (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CDatum((int)m_Stack.Get(1) < (int)m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsLessMulti (CDatum& retResult)
	{
	bool bCondition;
	int iCount = GetOperand(*m_pIP);
	if (iCount == 0)
		bCondition = false;
	else if (iCount == 1)
		bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) < 0);
	else
		{
		bCondition = true;
		CDatum dValue = m_Stack.Pop();

		for (int i = 1; i < iCount; i++)
			{
			CDatum dValue2 = m_Stack.Pop();
			if (bCondition && ExecuteCompare(dValue, dValue2) <= 0)
				bCondition = false;

			dValue = dValue2;
			}
		}

	m_Stack.Push(CDatum(bCondition));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsLessOrEqual (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompLessThanOrEqual(*this, m_Stack.Get(1), m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsLessOrEqualInt (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CDatum((int)m_Stack.Get(1) <= (int)m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsLessOrEqualMulti (CDatum& retResult)
	{
	bool bCondition;
	int iCount = GetOperand(*m_pIP);
	if (iCount == 0)
		bCondition = true;
	else if (iCount == 1)
		bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) <= 0);
	else
		{
		bCondition = true;
		CDatum dValue = m_Stack.Pop();

		for (int i = 1; i < iCount; i++)
			{
			CDatum dValue2 = m_Stack.Pop();
			if (bCondition && ExecuteCompare(dValue, dValue2) < 0)
				bCondition = false;

			dValue = dValue2;
			}
		}

	m_Stack.Push(CDatum(bCondition));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsNotEqual (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompNotEqual(*this, m_Stack.Get(1), m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsNotEqualInt (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CDatum((int)m_Stack.Get() != (int)m_Stack.Get(1));
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsNotEqualMulti (CDatum& retResult)
	{
	bool bCondition;
	int iCount = GetOperand(*m_pIP);
	if (iCount == 0)
		bCondition = true;
	else
		{
		bCondition = true;
		CDatum dValue = m_Stack.Pop();

		for (int i = 1; i < iCount; i++)
			{
			CDatum dValue2 = m_Stack.Pop();
			if (bCondition && !ExecuteIsEquivalent(dValue, dValue2))
				bCondition = false;
			}
		}

	m_Stack.Push(CDatum(!bCondition));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteIsNotIdentical (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dValue = CAEONOp::CompNotIdentical(*this, m_Stack.Get(1), m_Stack.Get());
	m_Stack.Replace(dValue, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteJump (CDatum& retResult)
	{
	m_pIP += CHexeCode::GetOperandInt(*m_pIP);
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteJumpIfNil (CDatum& retResult)
	{
	if (m_Stack.Pop().IsNil())
		m_pIP += CHexeCode::GetOperandInt(*m_pIP);
	else
		m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteJumpIfNilNoPop (CDatum& retResult)
	{
	if (m_Stack.Get().IsNil())
		m_pIP += CHexeCode::GetOperandInt(*m_pIP);
	else
		m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteJumpIfNotNilNoPop (CDatum& retResult)
	{
	if (!m_Stack.Get().IsNil())
		m_pIP += CHexeCode::GetOperandInt(*m_pIP);
	else
		m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteLoopIncAndJump (CDatum& retResult)
	{
	m_Stack.Push(m_Env.GetLocalEnv().IncArgumentValueInt32(LOOP_INDEX, 1));
	m_pIP += CHexeCode::GetOperandInt(*m_pIP);

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeApplyEnv (CDatum& retResult)
	{
	m_Env.PushNewFrame();

	//	The top of the stack is a list of arguments

	CDatum dArgList = m_Stack.Pop();

	//	Add the remaining args to the environment

	int iFixedArgCount = GetOperand(*m_pIP) - 1;
	for (int i = 0; i < iFixedArgCount; i++)
		m_Env.GetLocalEnv().SetArgumentValue(0, (iFixedArgCount - 1) - i, m_Stack.Pop());

	//	Now add the args in the list

	for (int i = 0; i < dArgList.GetCount(); i++)
		m_Env.GetLocalEnv().SetArgumentValue(0, iFixedArgCount + i, dArgList.GetElement(i));

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeArray (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	if (iCount == 0)
		m_Stack.Push(CDatum(CDatum::typeArray));
	else
		{
		CDatum dArray(CDatum::typeArray);
		ExecuteMakeArrayFromStack(dArray, iCount);
		m_Stack.Push(dArray);
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeAsType (CDatum& retResult)
	{
	CDatum dType = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();
	CDatum dNewValue = CDatum::CreateAsType(dType, dValue);
	m_Stack.Push(dNewValue);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeAsTypeCons (CDatum& retResult)
	{
	CDatum dType = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();
	CDatum dNewValue = CDatum::CreateAsType(dType, dValue, true);
	m_Stack.Push(dNewValue);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeBlockEnv (CDatum& retResult)
	{
	m_Env.PushNewFrameAsChild();
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeDatatype (CDatum& retResult)
	{
	CDatum dType = m_Stack.Pop();
	m_Stack.Push(dType);
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeEmptyArray (CDatum& retResult)
	{
	CDatum dValue = CDatum(CDatum::typeArray);

	int iCount = GetOperand(*m_pIP);
	dValue.GrowToFit(iCount);
	m_Stack.Push(dValue);

	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeEmptyArrayAsType (CDatum& retResult)
	{
	CDatum dType = m_Stack.Pop();
	CDatum dArray = CDatum::CreateArrayAsType(dType);

	int iCount = GetOperand(*m_pIP);
	dArray.GrowToFit(iCount);
	m_Stack.Push(dArray);

	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeRange (CDatum& retResult)
	{
	//	The arguments are pushed on the stack in left-to-right order,
	//	which means the top of the stack if the right-most.

	CDatum dEnd = m_Stack.Pop();
	CDatum dStart = m_Stack.Pop();

	//	Create the range

	CDatum dRange = CDatum::CreateRange(dStart, dEnd, CDatum());
	if (dRange.IsError())
		return RuntimeError(dRange.AsString(), retResult);

	m_Stack.Push(dRange);

	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeEmptyStruct (CDatum& retResult)
	{
	CDatum dValue = CDatum(CDatum::typeStruct);

	int iCount = GetOperand(*m_pIP);
	dValue.GrowToFit(iCount);
	m_Stack.Push(dValue);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeEnv (CDatum& retResult)
	{
	m_Env.PushNewFrame();

	int iArgCount = GetOperand(*m_pIP);

	//	The arguments are pushed on the stack in left-to-right order,
	//	which means the top of the stack if the right-most.

	for (int i = iArgCount - 1; i >= 0; i--)
		m_Env.GetLocalEnv().AppendArgumentValue(m_Stack.Get(i));

	m_Stack.Pop(iArgCount);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeExpr (CDatum& retResult)
	{
	int iArgCount = (int)GetOperand2Arg(*m_pIP);
	CAEONExpression::EOp iOp = (CAEONExpression::EOp)GetOperand2Value(*m_pIP);

	CDatum dValue;
	if (iArgCount == 1)
		{
		CDatum dParam = m_Stack.Get();
		if (dParam.GetBasicType() != CDatum::typeExpression)
			dParam = CAEONExpression::CreateLiteral(dParam);

		dValue = CAEONExpression::CreateUnaryOp(iOp, dParam.AsExpression());
		}
	else if (iArgCount == 2)
		{
		CDatum dParam1 = m_Stack.Get(1);
		if (dParam1.GetBasicType() != CDatum::typeExpression)
			dParam1 = CAEONExpression::CreateLiteral(dParam1);

		CDatum dParam2 = m_Stack.Get();
		if (dParam2.GetBasicType() != CDatum::typeExpression)
			dParam2 = CAEONExpression::CreateLiteral(dParam2);

		dValue = CAEONExpression::CreateBinaryOp(iOp, dParam1.AsExpression(), dParam2.AsExpression());
		}
	else if (iArgCount == 3)
		{
		CDatum dParam1 = m_Stack.Get(2);
		if (dParam1.GetBasicType() != CDatum::typeExpression)
			dParam1 = CAEONExpression::CreateLiteral(dParam1);

		CDatum dParam2 = m_Stack.Get(1);
		if (dParam2.GetBasicType() != CDatum::typeExpression)
			dParam2 = CAEONExpression::CreateLiteral(dParam2);

		CDatum dParam3 = m_Stack.Get();
		if (dParam3.GetBasicType() != CDatum::typeExpression)
			dParam3 = CAEONExpression::CreateLiteral(dParam3);

		dValue = CAEONExpression::CreateTernaryOp(iOp, dParam1.AsExpression(), dParam2.AsExpression(), dParam3.AsExpression());
		}
	else
		throw CException(errFail);

	m_Stack.Replace(dValue, iArgCount);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeExprIf (CDatum& retResult)
	{
	int iArgCount = GetOperand(*m_pIP);

	CDatum dCondition = m_Stack.Get(0);
	CDatum dThen = m_Stack.Get(1);
	CDatum dElse = (iArgCount > 2 ? m_Stack.Get(2) : CDatum());

	CDatum dConditionExpr = (dCondition.GetBasicType() == CDatum::typeExpression ? dCondition : CAEONExpression::CreateLiteral(dCondition));
	CDatum dThenExpr = (dThen.GetBasicType() == CDatum::typeExpression ? dThen : CAEONExpression::CreateLiteral(dThen));
	CDatum dElseExpr = (dElse.GetBasicType() == CDatum::typeExpression ? dElse : CAEONExpression::CreateLiteral(dElse));

	CDatum dIfExpr = CAEONExpression::CreateIf(dConditionExpr.AsExpression(), dThenExpr.AsExpression(), dElseExpr.AsExpression());

	m_Stack.Replace(dIfExpr, iArgCount);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeFlagsFromArray (CDatum& retResult)
	{
	CDatum dMap = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dResult;
	if (!ExecuteMakeFlagsFromArray(dArray, dMap, &dResult))
		{
		retResult = dResult;
		return ERun::Error;
		}

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeFunc (CDatum& retResult)
	{
	CDatum dValue;
	CHexeFunction::Create(m_dCodeBank, GetOperand(*m_pIP), m_Env.GetGlobalEnvClosure(), m_Env.GetLocalEnvClosure(), &dValue);
	m_Stack.Push(dValue);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeFunc2 (CDatum& retResult)
	{
	CDatum dFuncType = m_Stack.Pop();
	CDatum dAttribs = m_Stack.Pop();
	m_Stack.Push(CHexeFunction::Create(m_dCodeBank, GetOperand(*m_pIP), m_Env.GetGlobalEnvClosure(), m_Env.GetLocalEnvClosure(), dAttribs, dFuncType));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeLocalEnv (CDatum& retResult)
	{
	m_Env.PushNewFrameAsChild(GetOperand(*m_pIP));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeMapColExpr (CDatum& retResult)
	{
	CDatum dType = m_Stack.Get(1);
	CDatum dExpressions = m_Stack.Get();

	CDatum dMap = CAEONMapColumnExpression::Create(dType, dExpressions);
	m_Stack.Replace(dMap, 2);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeMethodEnv (CDatum& retResult)
	{
	m_Env.PushNewFrame();

	int iArgCount = GetOperand(*m_pIP);

	//	Look for the this pointer. If we have a valid (non-nil) this
	//	pointer, then we include it as the first parameter. [But we
	//	make sure it is exactly nil; otherwise we might have an 
	//	empty table or array that is treated as nil.]

	CDatum dThis = m_Stack.Get(iArgCount);
	if (!dThis.IsIdenticalToNil())
		iArgCount++;

	//	The arguments are pushed on the stack in left-to-right order,
	//	which means the top of the stack is the right-most.

	for (int i = iArgCount - 1; i >= 0; i--)
		m_Env.GetLocalEnv().AppendArgumentValue(m_Stack.Get(i));

	m_Stack.Pop(iArgCount);

	//	Pop the this pointer, if necessary.

	if (dThis.IsIdenticalToNil())
		m_Stack.Pop();

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeObject (CDatum& retResult)
	{
	CDatum dType = m_Stack.Pop();
	CDatum dObj = CDatum::CreateObject(dType);

	int iCount = GetOperand(*m_pIP);
	if (iCount > 0)
		{
		for (int i = 0; i < iCount; i++)
			{
			CDatum dValue = m_Stack.Pop();
			CDatum dKey = m_Stack.Pop();

			dObj.SetElement(dKey.AsStringView(), dValue);
			}
		}

	m_Stack.Push(dObj);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakePrimitive (CDatum& retResult)
	{
	CDatum dValue;
	CHexePrimitive::Create((CDatum::ECallType)GetOperand(*m_pIP), &dValue);
	m_Stack.Push(dValue);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeSpread (CDatum& retResult)
	{
	CDatum::SAnnotation Annotation;
	Annotation.fSpread = true;

	CDatum dValue = CDatum::CreateAnnotated(m_Stack.Pop(), Annotation);
	m_Stack.Push(dValue);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeStruct (CDatum& retResult)
	{
	CComplexStruct* pStruct = new CComplexStruct;
	int iCount = GetOperand(*m_pIP);
	if (iCount > 0)
		{
		for (int i = 0; i < iCount; i++)
			{
			CDatum dValue = m_Stack.Pop();
			CDatum dKey = m_Stack.Pop();

			pStruct->SetElement(dKey.AsStringView(), dValue);
			}
		}

	m_Stack.Push(CDatum(pStruct));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeTensor (CDatum& retResult)
	{
	CDatum dType;
	ERun iRun = ExecuteMakeTensorTypeFromStack(dType);
	if (iRun != ERun::Continue)
		return iRun;

	CDatum dInitValue = m_Stack.Pop();

	CDatum dTensor = CDatum::CreateTensorAsType(dType, dInitValue);
	m_Stack.Push(dTensor);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeTensorTypeFromStack (CDatum& retResult)
	{
	CDatum dElementType = m_Stack.Pop();

	int iCount = GetOperand(*m_pIP);	//	Number of dimensions
	TArray<CDatum> Dims;
	Dims.InsertEmpty(iCount);
	LONGLONG iTotalElements = 1;
	for (int i = 0; i < iCount; i++)
		{
		CDatum dDim = m_Stack.Pop();

		int iMin;
		int iMax;
		int iDimSize;
		switch (dDim.GetBasicType())
			{
			//	If this is a range then we use it to create the dimension.

			case CDatum::typeRange:
				{
				const IAEONRange* pRange = dDim.GetRangeInterface();
				if (!pRange)
					throw CException(errFail);

				iMin = pRange->GetStart();
				iMax = pRange->GetEnd();
				iDimSize = iMax - iMin + 1;
				break;
				}

			//	If this is a datatype, see if it is a subrange

			case CDatum::typeDatatype:
				{
				const IDatatype& Type = dDim;
				if (Type.GetClass() == IDatatype::ECategory::Number)
					{
					auto NumberDesc = Type.GetNumberDesc();
					if (NumberDesc.bSubRange)
						{
						iMin = NumberDesc.iSubRangeMin;
						iMax = NumberDesc.iSubRangeMax;
						iDimSize = iMax - iMin + 1;
						}
					else
						{
						return RuntimeError(strPattern(ERR_INVALID_ARRAY_DIMENSION, dDim.AsString()), retResult);
						}
					}
				else
					return RuntimeError(strPattern(ERR_INVALID_ARRAY_DIMENSION, dDim.AsString()), retResult);

				break;
				}

			//	Otherwise, we just use the value as a dimension

			default:
				iMin = 0;
				iDimSize = (int)dDim;
				iMax = iDimSize - 1;
				break;
			}

		if (iDimSize < 0)
			return RuntimeError(strPattern(ERR_NEGATIVE_ARRAY_SIZE, iDimSize), retResult);

		iTotalElements *= iDimSize;
		if (iDimSize > m_Limits.iMaxArrayLen)
			return RuntimeError(strPattern(ERR_EXCEEDED_ARRAY_LIMITS, ::strFormatInteger(m_Limits.iMaxArrayLen, -1, FORMAT_THOUSAND_SEPARATOR)), retResult);

		Dims[i] = CAEONTypes::CreateInt32SubRange(NULL_STR, iMin, iMax);
		}

	retResult = m_Types.AddAnonymousTensor(dElementType, Dims);
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMakeTensorType (CDatum& retResult)
	{
	CDatum dType;
	ERun iRun = ExecuteMakeTensorTypeFromStack(dType);
	if (iRun != ERun::Continue)
		return iRun;

	m_Stack.Push(dType);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMapResult (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	DWORD dwFlags = (DWORD)(int)m_Stack.Pop();
	CDatum dOriginal = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();
	CDatum dArray = m_Env.GetLocalEnv().GetArgument((dwOperand >> 8), (dwOperand & 0xff));

	//	If we're excluding nil and the value is Nil, then nothing to do

	if ((dwFlags & FLAG_MAP_EXCLUDE_NIL) && dValue.IsNil())
		;

	//	Otherwise, append the value to the array

	else
		{
		if (dwFlags & FLAG_MAP_ORIGINAL)
			dValue = dOriginal;

		if (dArray.IsNil())
			{
			CDatum dArray(CDatum::typeArray);
			dArray.Append(dValue);
			m_Env.GetLocalEnv().SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), dArray);
			}
		else
			dArray.Append(dValue);
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMod (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dDivisor = m_Stack.Pop();
	CDatum dDividend = m_Stack.Pop();
	m_Stack.Push(CAEONOp::Mod(dDividend, dDivisor));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMultiply (CDatum& retResult)
	{
	//	NOTE: Compatible version for HexeLisp. It is not used by
	//	GridLang.

	int iCount = GetOperand(*m_pIP);

	if (iCount < 1)
		m_Stack.Push(0);
	else
		{
		CNumberValue Result(m_Stack.Pop());

		for (int i = 1; i < iCount; i++)
			Result.Multiply(m_Stack.Pop());

		m_Stack.Push(Result.GetDatum());
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMultiply2 (CDatum& retResult)
	{
	CDatum dB = m_Stack.Pop();
	CDatum dA = m_Stack.Pop();

	m_Stack.Push(CAEONOp::Multiply(dA, dB));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateArrayItemAdd (CDatum& retResult)
	{
	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	CDatum dElement = dArray.GetElementAt(m_Types, dIndex);
	CDatum dResult = CAEONOp::Add(dElement, dValue);
	dArray.SetElementAt(dIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateArrayItemConcat (CDatum& retResult)
	{
	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	CDatum dElement = dArray.GetElementAt(m_Types, dIndex);
	CDatum dResult = CAEONOp::Concatenate(*this, dElement, dValue);
	dArray.SetElementAt(dIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateArrayItemDivide (CDatum& retResult)
	{
	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	CDatum dElement = dArray.GetElementAt(m_Types, dIndex);
	CDatum dResult = CAEONOp::Divide(dElement, dValue);
	dArray.SetElementAt(dIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateArrayItemMod (CDatum& retResult)
	{
	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	CDatum dElement = dArray.GetElementAt(m_Types, dIndex);
	CDatum dResult = CAEONOp::Mod(dElement, dValue);
	dArray.SetElementAt(dIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateArrayItemMultiply (CDatum& retResult)
	{
	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	CDatum dElement = dArray.GetElementAt(m_Types, dIndex);
	CDatum dResult = CAEONOp::Multiply(dElement, dValue);
	dArray.SetElementAt(dIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateArrayItemPower (CDatum& retResult)
	{
	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	CDatum dElement = dArray.GetElementAt(m_Types, dIndex);
	CDatum dResult = CAEONOp::Power(dElement, dValue);
	dArray.SetElementAt(dIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateArrayItemSubtract (CDatum& retResult)
	{
	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	CDatum dElement = dArray.GetElementAt(m_Types, dIndex);
	CDatum dResult = CAEONOp::Subtract(dElement, dValue);
	dArray.SetElementAt(dIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateGlobalAdd (CDatum& retResult)
	{
	DWORD dwIndex;
	if (!m_Env.GetGlobalEnv().FindSymbol(GetStringFromDataBlock(GetOperand(*m_pIP)), &dwIndex))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, GetStringFromDataBlock(GetOperand(*m_pIP)).AsString()), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Add(m_Env.GetGlobalEnv().GetAt(dwIndex), dValue);
	m_Env.GetGlobalEnv().SetAt(dwIndex, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateGlobalConcat (CDatum& retResult)
	{
	DWORD dwIndex;
	if (!m_Env.GetGlobalEnv().FindSymbol(GetStringFromDataBlock(GetOperand(*m_pIP)), &dwIndex))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, GetStringFromDataBlock(GetOperand(*m_pIP)).AsString()), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Concatenate(*this, m_Env.GetGlobalEnv().GetAt(dwIndex), dValue);
	m_Env.GetGlobalEnv().SetAt(dwIndex, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateGlobalDivide (CDatum& retResult)
	{
	DWORD dwIndex;
	if (!m_Env.GetGlobalEnv().FindSymbol(GetStringFromDataBlock(GetOperand(*m_pIP)), &dwIndex))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, GetStringFromDataBlock(GetOperand(*m_pIP)).AsString()), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Divide(m_Env.GetGlobalEnv().GetAt(dwIndex), dValue);
	m_Env.GetGlobalEnv().SetAt(dwIndex, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateGlobalMod (CDatum& retResult)
	{
	DWORD dwIndex;
	if (!m_Env.GetGlobalEnv().FindSymbol(GetStringFromDataBlock(GetOperand(*m_pIP)), &dwIndex))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, GetStringFromDataBlock(GetOperand(*m_pIP)).AsString()), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Mod(m_Env.GetGlobalEnv().GetAt(dwIndex), dValue);
	m_Env.GetGlobalEnv().SetAt(dwIndex, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateGlobalMultiply (CDatum& retResult)
	{
	DWORD dwIndex;
	if (!m_Env.GetGlobalEnv().FindSymbol(GetStringFromDataBlock(GetOperand(*m_pIP)), &dwIndex))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, GetStringFromDataBlock(GetOperand(*m_pIP)).AsString()), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Multiply(m_Env.GetGlobalEnv().GetAt(dwIndex), dValue);
	m_Env.GetGlobalEnv().SetAt(dwIndex, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateGlobalPower (CDatum& retResult)
	{
	DWORD dwIndex;
	if (!m_Env.GetGlobalEnv().FindSymbol(GetStringFromDataBlock(GetOperand(*m_pIP)), &dwIndex))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, GetStringFromDataBlock(GetOperand(*m_pIP)).AsString()), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Power(m_Env.GetGlobalEnv().GetAt(dwIndex), dValue);
	m_Env.GetGlobalEnv().SetAt(dwIndex, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateGlobalSubtract (CDatum& retResult)
	{
	DWORD dwIndex;
	if (!m_Env.GetGlobalEnv().FindSymbol(GetStringFromDataBlock(GetOperand(*m_pIP)), &dwIndex))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, GetStringFromDataBlock(GetOperand(*m_pIP)).AsString()), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Subtract(m_Env.GetGlobalEnv().GetAt(dwIndex), dValue);
	m_Env.GetGlobalEnv().SetAt(dwIndex, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateLocalAdd (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	auto& Env = m_Env.GetLocalEnvAt(iLevel);
	m_Stack.Set(Env.OpAdd(iIndex, m_Stack.Get()));

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateLocalConcat (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	auto& Env = m_Env.GetLocalEnvAt(iLevel);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Concatenate(*this, Env.GetArgument(iIndex), dValue);
	Env.SetArgumentValue(iIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateLocalDivide (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	auto& Env = m_Env.GetLocalEnvAt(iLevel);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Divide(Env.GetArgument(iIndex), dValue);
	Env.SetArgumentValue(iIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateLocalMod (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	auto& Env = m_Env.GetLocalEnvAt(iLevel);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Mod(Env.GetArgument(iIndex), dValue);
	Env.SetArgumentValue(iIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateLocalMultiply (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	auto& Env = m_Env.GetLocalEnvAt(iLevel);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Multiply(Env.GetArgument(iIndex), dValue);
	Env.SetArgumentValue(iIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateLocalPower (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	auto& Env = m_Env.GetLocalEnvAt(iLevel);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Power(Env.GetArgument(iIndex), dValue);
	Env.SetArgumentValue(iIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateLocalSubtract (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	auto& Env = m_Env.GetLocalEnvAt(iLevel);

	CDatum dValue = m_Stack.Pop();
	CDatum dResult = CAEONOp::Subtract(Env.GetArgument(iIndex), dValue);
	Env.SetArgumentValue(iIndex, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateObjectItemAdd (CDatum& retResult)
	{
	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	CDatum dElement = dObject.GetElement(sField);
	CDatum dResult = CAEONOp::Add(dElement, dValue);
	dObject.SetElement(sField, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateObjectItemConcat (CDatum& retResult)
	{
	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	CDatum dElement = dObject.GetElement(sField);
	CDatum dResult = CAEONOp::Concatenate(*this, dElement, dValue);
	dObject.SetElement(sField, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateObjectItemDivide (CDatum& retResult)
	{
	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	CDatum dElement = dObject.GetElement(sField);
	CDatum dResult = CAEONOp::Divide(dElement, dValue);
	dObject.SetElement(sField, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateObjectItemMod (CDatum& retResult)
	{
	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	CDatum dElement = dObject.GetElement(sField);
	CDatum dResult = CAEONOp::Mod(dElement, dValue);
	dObject.SetElement(sField, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateObjectItemMultiply (CDatum& retResult)
	{
	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	CDatum dElement = dObject.GetElement(sField);
	CDatum dResult = CAEONOp::Multiply(dElement, dValue);
	dObject.SetElement(sField, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateObjectItemPower (CDatum& retResult)
	{
	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	CDatum dElement = dObject.GetElement(sField);
	CDatum dResult = CAEONOp::Power(dElement, dValue);
	dObject.SetElement(sField, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateObjectItemSubtract (CDatum& retResult)
	{
	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	CDatum dElement = dObject.GetElement(sField);
	CDatum dResult = CAEONOp::Subtract(dElement, dValue);
	dObject.SetElement(sField, dResult);

	m_Stack.Push(dResult);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateTensorItemAdd (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndex);
			CDatum dResult = CAEONOp::Add(dElement, dValue);
			dTensor.SetElementAt(dIndex, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 2:
			{
			int iIndex1 = m_Stack.Pop();
			int iIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt2DI(iIndex1, iIndex2);
			CDatum dResult = CAEONOp::Add(dElement, dValue);
			dTensor.SetElementAt2DI(iIndex1, iIndex2, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt3DI(dIndex1, dIndex2, dIndex3);
			CDatum dResult = CAEONOp::Add(dElement, dValue);
			dTensor.SetElementAt3DI(dIndex1, dIndex2, dIndex3, dResult);
			m_Stack.Push(dResult);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndices);
			CDatum dResult = CAEONOp::Add(dElement, dValue);
			dTensor.SetElementAt(dIndices, dResult);
			m_Stack.Push(dResult);
			break;
			}
		}

	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateTensorItemConcat (CDatum& retResult)
	{
	//	Not yet implemented
	throw CException(errFail);
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateTensorItemDivide (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndex);
			CDatum dResult = CAEONOp::Divide(dElement, dValue);
			dTensor.SetElementAt(dIndex, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 2:
			{
			int iIndex1 = m_Stack.Pop();
			int iIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt2DI(iIndex1, iIndex2);
			CDatum dResult = CAEONOp::Divide(dElement, dValue);
			dTensor.SetElementAt2DI(iIndex1, iIndex2, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt3DI(dIndex1, dIndex2, dIndex3);
			CDatum dResult = CAEONOp::Divide(dElement, dValue);
			dTensor.SetElementAt3DI(dIndex1, dIndex2, dIndex3, dResult);
			m_Stack.Push(dResult);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndices);
			CDatum dResult = CAEONOp::Divide(dElement, dValue);
			dTensor.SetElementAt(dIndices, dResult);
			m_Stack.Push(dResult);
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateTensorItemMod (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndex);
			CDatum dResult = CAEONOp::Mod(dElement, dValue);
			dTensor.SetElementAt(dIndex, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 2:
			{
			int iIndex1 = m_Stack.Pop();
			int iIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt2DI(iIndex1, iIndex2);
			CDatum dResult = CAEONOp::Mod(dElement, dValue);
			dTensor.SetElementAt2DI(iIndex1, iIndex2, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt3DI(dIndex1, dIndex2, dIndex3);
			CDatum dResult = CAEONOp::Mod(dElement, dValue);
			dTensor.SetElementAt3DI(dIndex1, dIndex2, dIndex3, dResult);
			m_Stack.Push(dResult);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndices);
			CDatum dResult = CAEONOp::Mod(dElement, dValue);
			dTensor.SetElementAt(dIndices, dResult);
			m_Stack.Push(dResult);
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateTensorItemMultiply (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndex);
			CDatum dResult = CAEONOp::Multiply(dElement, dValue);
			dTensor.SetElementAt(dIndex, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 2:
			{
			int iIndex1 = m_Stack.Pop();
			int iIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt2DI(iIndex1, iIndex2);
			CDatum dResult = CAEONOp::Multiply(dElement, dValue);
			dTensor.SetElementAt2DI(iIndex1, iIndex2, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt3DI(dIndex1, dIndex2, dIndex3);
			CDatum dResult = CAEONOp::Multiply(dElement, dValue);
			dTensor.SetElementAt3DI(dIndex1, dIndex2, dIndex3, dResult);
			m_Stack.Push(dResult);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndices);
			CDatum dResult = CAEONOp::Multiply(dElement, dValue);
			dTensor.SetElementAt(dIndices, dResult);
			m_Stack.Push(dResult);
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateTensorItemPower (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndex);
			CDatum dResult = CAEONOp::Power(dElement, dValue);
			dTensor.SetElementAt(dIndex, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 2:
			{
			int iIndex1 = m_Stack.Pop();
			int iIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt2DI(iIndex1, iIndex2);
			CDatum dResult = CAEONOp::Power(dElement, dValue);
			dTensor.SetElementAt2DI(iIndex1, iIndex2, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt3DI(dIndex1, dIndex2, dIndex3);
			CDatum dResult = CAEONOp::Power(dElement, dValue);
			dTensor.SetElementAt3DI(dIndex1, dIndex2, dIndex3, dResult);
			m_Stack.Push(dResult);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndices);
			CDatum dResult = CAEONOp::Power(dElement, dValue);
			dTensor.SetElementAt(dIndices, dResult);
			m_Stack.Push(dResult);
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteMutateTensorItemSubtract (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndex);
			CDatum dResult = CAEONOp::Subtract(dElement, dValue);
			dTensor.SetElementAt(dIndex, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 2:
			{
			int iIndex1 = m_Stack.Pop();
			int iIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt2DI(iIndex1, iIndex2);
			CDatum dResult = CAEONOp::Subtract(dElement, dValue);
			dTensor.SetElementAt2DI(iIndex1, iIndex2, dResult);
			m_Stack.Push(dResult);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt3DI(dIndex1, dIndex2, dIndex3);
			CDatum dResult = CAEONOp::Subtract(dElement, dValue);
			dTensor.SetElementAt3DI(dIndex1, dIndex2, dIndex3, dResult);
			m_Stack.Push(dResult);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			CDatum dElement = dTensor.GetElementAt(m_Types, dIndices);
			CDatum dResult = CAEONOp::Subtract(dElement, dValue);
			dTensor.SetElementAt(dIndices, dResult);
			m_Stack.Push(dResult);
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteNegate (CDatum& retResult)
	{
	m_Stack.Push(CAEONOp::Negate(m_Stack.Pop()));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteNewObject (CDatum& retdError)

//	ExecuteNewObject
//
//	Implement opNewObject.
//
//	We expect n arguments pushed on the stack, plus the object type at the top.
//	The operand is the number of arguments.

	{
	//	Get the number of arguments

	int iArgCount = GetOperand(*m_pIP);

	//	Get the type

	CDatum dType = m_Stack.Pop();
	if (dType.GetBasicType() != CDatum::typeDatatype)
		return RuntimeError(strPattern(ERR_DATATYPE_EXPECTED, dType.AsString()), retdError);

	//	Does the type have a constructor? If so, then we call it.

	const IDatatype& Datatype = dType;
	if (Datatype.FindMember(FIELD_CONSTRUCTOR) != -1)
		{
		static constexpr int MAX_ARGS = 16;

		//	Pop the arguments (but save them).

		if (iArgCount > MAX_ARGS)
			return RuntimeError(strPattern(ERR_TOO_MANY_ARGS, iArgCount), retdError);

		if (m_CallStack.GetCount() >= m_Limits.iMaxStackDepth)
			return RuntimeError(ERR_EXCEEDED_RECURSION_LIMITS, retdError);

		CDatum Args[MAX_ARGS];
		for (int i = 0; i < iArgCount; i++)
			Args[i] = m_Stack.Pop();

		//	Get the constructor method

		CString sFunctionName = CAEONTypes::MakeFullyQualifiedName(Datatype.GetFullyQualifiedName(), FIELD_CONSTRUCTOR);

		DWORD dwConstructorID;
		if (!m_Env.GetGlobalEnv().FindSymbol(sFunctionName, &dwConstructorID))
			return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, sFunctionName), retdError);

		CDatum dConstructor = m_Env.GetGlobalEnv().GetAt(dwConstructorID);

		//	Setup the environment for the call.

		m_Env.PushNewFrame();

		CDatum dObj = CDatum::CreateObject(dType);
		m_Env.GetLocalEnv().AppendArgumentValue(dObj);

		for (int i = iArgCount - 1; i >= 0; i--)
			m_Env.GetLocalEnv().AppendArgumentValue(Args[i]);

		//	Now call the constructor

		DWORD *pNewIP;
		CDatum dNewCodeBank;
		if (dConstructor.GetCallInfo(&dNewCodeBank, &pNewIP) != CDatum::ECallType::Call)
			throw CException(errFail);	//	We don't support library calls as constructors.

		//	Save the stack and advance the IP so that we return AFTER the 
		//	opNewObject instruction.

		m_CallStack.PushFunCall(m_dExpression, m_dCodeBank, ++m_pIP);
		m_dExpression = dConstructor;
		SetCodeBank(dNewCodeBank);

		m_pIP = pNewIP;
		}

	//	Otherwise, we create a new object

	else
		{
		//	We take the first argument (if any) as the initial value. But first 
		//	we need to pop off any extra arguments.

		if (iArgCount > 1)
			m_Stack.Pop(iArgCount - 1);

		CDatum dValue;
		if (iArgCount > 0)
			dValue = m_Stack.Pop();

		//	Create the object and push it on the stack

		CDatum dNewValue = CDatum::CreateAsType(dType, dValue);
		m_Stack.Push(dNewValue);
		m_pIP++;
		}

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteNot (CDatum& retResult)
	{
	if (m_Stack.Pop().IsNil())
		m_Stack.Push(true);
	else
		m_Stack.Push(false);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePop (CDatum& retResult)
	{
	m_Stack.Pop(GetOperand(*m_pIP));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePopDeep (CDatum& retResult)
	{
	CDatum dSaved = m_Stack.Pop();
	m_Stack.Pop(GetOperand(*m_pIP));
	m_Stack.Push(dSaved);
	m_pIP++;
	
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePopLocal (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	m_Env.GetLocalEnv().SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), m_Stack.Pop());
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePopLocalL0 (CDatum& retResult)
	{
	m_Env.GetLocalEnv().SetArgumentValue(GetOperand(*m_pIP), m_Stack.Pop());
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePower (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dPower = m_Stack.Pop();
	CDatum dBase = m_Stack.Pop();
	m_Stack.Push(CAEONOp::Power(dBase, dPower));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushArrayItem (CDatum& retResult)
	{
	DebugCheck(GetOperand(*m_pIP) == 2);

	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	m_Stack.Push(dArray.GetElementAt(m_Types, dIndex));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushArrayItemI (CDatum& retResult)
	{
	CDatum dArray = m_Stack.Pop();
	int iIndex = CHexeCode::GetOperandInt(*m_pIP);

	//	NOTE: GetElementAt does a range check.
	//	NOTE: We call GetElementAt instead of GetElement because we
	//	want strings to be treated as arrays of chars.

	m_Stack.Push(dArray.GetElementAt(iIndex));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushCoreType (CDatum& retResult)
	{
	DWORD dwCoreType = GetOperand(*m_pIP);
	m_Stack.Push(CAEONTypes::Get_NoError(dwCoreType));

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushDatum (CDatum& retResult)
	{
	m_Stack.Push(m_pCodeBank->GetDatum(GetOperand(*m_pIP)));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushGlobal (CDatum& retResult)
	{
	int iIndex = GetOperand(*m_pIP);
	DWORD dwID = m_GlobalEnvCache.GetID(iIndex);
	if (dwID == CHexeGlobalEnvCache::INVALID_ID)
		{
		if (!m_Env.GetGlobalEnv().FindSymbol(m_pCodeBank->GetStringLiteral(iIndex), &dwID))
			return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(iIndex)), retResult);

		m_GlobalEnvCache.SetID(iIndex, dwID);
		}

	m_Stack.Push(m_Env.GetGlobalEnv().GetAt(dwID));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushInitForEach (CDatum& retResult)
	{
	CDatum dArray = m_Stack.Get();
	m_Stack.Push(dArray.IteratorBegin());
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushInt (CDatum& retResult)
	{
	m_pIP++;
	m_Stack.Push(CDatum((int)*m_pIP));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushIntShort (CDatum& retResult)
	{
	m_Stack.Push(CDatum(CHexeCode::GetOperandInt(*m_pIP)));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushLiteral (CDatum& retResult)
	{
	m_Stack.Push(GetStringFromDataBlock(GetOperand(*m_pIP)));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushLocal (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	m_Stack.Push(m_Env.GetLocalEnvAt(iLevel).GetArgument(iIndex));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushLocalItem (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	int iIndex = (int)m_Stack.Pop();
	CDatum dList = m_Env.GetLocalEnv().GetArgument((dwOperand >> 8), (dwOperand & 0xff));

	//	For structs we push a tuple of key, value

	if (dList.GetBasicType() == CDatum::typeStruct)
		{
		CDatum dTuple(CDatum::typeArray);
		dTuple.Append(dList.GetKey(iIndex));
		dTuple.Append(dList.GetElement(this, iIndex));
		m_Stack.Push(dTuple);
		}

	//	Otherwise, just the value

	else
		m_Stack.Push(dList.GetElement(this, iIndex));

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushLocalL0 (CDatum& retResult)
	{
	m_Stack.Push(m_Env.GetLocalEnv().GetArgument(GetOperand(*m_pIP)));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushLocalLength (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	m_Stack.Push(m_Env.GetLocalEnv().GetArgument((dwOperand >> 8), (dwOperand & 0xff)).GetCount());
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushNaN (CDatum& retResult)
	{
	m_Stack.Push(CDatum::CreateNaN());
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushNil (CDatum& retResult)
	{
	m_Stack.Push(CDatum());
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushObjectItem (CDatum& retResult)
	{
	bool bNoIPInc = false;
	DebugCheck(GetOperand(*m_pIP) == 2);

	//	NOTE: We always expect this to be a string. Compilers should
	//	convert to string if necessary. Otherwise, performance will
	//	suffer.

	CDatum dField = m_Stack.Pop();
	CStringView sField = dField;
	CDatum dObject = m_Stack.Pop();

	switch (dObject.GetBasicType())
		{
		case CDatum::typeCustom:
			if (!ExecuteCustomMemberItem(dObject, sField, retResult))
				return ERun::Error;
			break;

		case CDatum::typeClassInstance:
		case CDatum::typeObject:
		case CDatum::typeAEONObject:
		case CDatum::typeRowRef:
			if (!ExecuteObjectMemberItem(dObject, sField, retResult, bNoIPInc))
				return ERun::Error;
			break;

		default:
			m_Stack.Push(dObject.GetProperty(sField));
			break;
		}

	if (!bNoIPInc)
		m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushObjectMethod (CDatum &retResult)

//	ExecutePushObjectMethod
//
//	Implement opPushObjectMethod

	{
	int iCount = GetOperand(*m_pIP);

	//	LATER: This should probably be an error.

	if (iCount != 2)
		{
		m_Stack.Push(CDatum());
		return ERun::Continue;
		}

	//	Get the object and method and figure out what to do based on the type of
	//	object on the stack.

	CString sField = m_Stack.Pop().AsString();
	CDatum dObject = m_Stack.Pop();

	switch (dObject.GetBasicType())
		{
		case CDatum::typeNil:
		case CDatum::typeBinary:
		case CDatum::typeDatatype:
		case CDatum::typeDateTime:
		case CDatum::typeString:
		case CDatum::typeArray:
		case CDatum::typeRange:
		case CDatum::typeTable:
		case CDatum::typeTensor:
		case CDatum::typeTextLines:
		case CDatum::typeVector2D:
		case CDatum::typeVector3D:
		case CDatum::typeAEONObject:
			{
			CDatum dMember = dObject.GetMethod(sField);
			if (dMember.IsNil() || !dMember.CanInvoke())
				{
				retResult = CDatum::CreateError(strPattern(ERR_MEMBER_FUNCTION_NOT_FOUND, sField));
				return ERun::Error;
				}

			m_Stack.Push(dMember);
			m_Stack.Push(dObject);
			break;
			}

		case CDatum::typeStruct:
			{
			CDatum dMember = dObject.GetProperty(sField);
			if (dMember.CanInvoke())
				{
				m_Stack.Push(dMember);

				//	Push a nil this pointer. In opMakeMethodEnv we 
				//	detect this and deal with it appropriately.

				m_Stack.Push(CDatum());
				break;
				}
			else
				{
				CDatum dMember = dObject.GetMethod(sField);
				if (dMember.IsNil() || !dMember.CanInvoke())
					{
					retResult = CDatum::CreateError(strPattern(ERR_MEMBER_FUNCTION_NOT_FOUND, sField));
					return ERun::Error;
					}

				m_Stack.Push(dMember);
				m_Stack.Push(dObject);
				}

			break;
			}

		case CDatum::typeClassInstance:
		case CDatum::typeObject:
		case CDatum::typeImage32:
		case CDatum::typeRowRef:
			{
			const IDatatype &Type = dObject.GetDatatype();
			if (Type.GetMemberCount() > 0)
				{
				auto iMemberType = Type.HasMember(sField);
				switch (iMemberType)
					{
					case IDatatype::EMemberType::InstanceMethod:
					case IDatatype::EMemberType::InstanceValue:
					case IDatatype::EMemberType::StaticMethod:
						{
						//	NOTE: Getting the type from the instance and combining 
						//	with the static method name is what gives us polymorphism.

						CString sFunctionName = CAEONTypes::MakeFullyQualifiedName(Type.GetFullyQualifiedName(), sField);

						DWORD dwID;
						if (!m_Env.GetGlobalEnv().FindSymbol(sFunctionName, &dwID))
							{
							retResult = CDatum::CreateError(strPattern(ERR_UNBOUND_VARIABLE, sFunctionName));
							return ERun::Error;
							}

						CDatum dValue = m_Env.GetGlobalEnv().GetAt(dwID);
						m_Stack.Push(dValue);

						//	We always push the this pointer.

						m_Stack.Push(dObject);
						break;
						}

					case IDatatype::EMemberType::DynamicMember:
					case IDatatype::EMemberType::InstanceKeyVar:
					case IDatatype::EMemberType::InstanceProperty:
					case IDatatype::EMemberType::InstanceReadOnlyProperty:
					case IDatatype::EMemberType::InstanceVar:
						{
						CDatum dFunc = dObject.GetElement(sField);
						if (dFunc.IsNil() || !dFunc.CanInvoke())
							{
							retResult = CDatum::CreateError(strPattern(ERR_MEMBER_FUNCTION_NOT_FOUND, sField));
							return ERun::Error;
							}

						m_Stack.Push(dFunc);

						//	Push nil because this is just the equivalent of a static function.

						m_Stack.Push(CDatum());
						break;
						}

					default:
						retResult = CDatum::CreateError(strPattern(ERR_MEMBER_FUNCTION_NOT_FOUND, sField));
						return ERun::Error;
					}
				}
			else
				{
				CDatum dMember = dObject.GetMethod(sField);
				if (dMember.IsNil() || !dMember.CanInvoke())
					{
					retResult = CDatum::CreateError(strPattern(ERR_MEMBER_FUNCTION_NOT_FOUND, sField));
					return ERun::Error;
					}

				m_Stack.Push(dMember);

				//	If we have no type defined, we assume it is 
				//	member function, so we pass the this pointer.
				//	We only need this because some built-in 
				//	objects don't have a type defined. In the future,
				//	we should define types for all objects.

				m_Stack.Push(dObject);
				}
			break;
			}

		default:
			CDatum dMember = dObject.GetElement(sField);
			if (dMember.IsNil() || !dMember.CanInvoke())
				{
				retResult = CDatum::CreateError(strPattern(ERR_MEMBER_FUNCTION_NOT_FOUND, sField));
				return ERun::Error;
				}

			m_Stack.Push(dMember);

			//	Push a nil this pointer. In opMakeMethodEnv we 
			//	detect this and deal with it appropriately.

			m_Stack.Push(CDatum());
			break;
		}

	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushStr (CDatum& retResult)
	{
	m_Stack.Push(m_pCodeBank->GetString(GetOperand(*m_pIP)));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushStrNull (CDatum& retResult)
	{
	m_Stack.Push(CDatum(NULL_STR));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushType (CDatum& retResult)
	{
	int iIndex = GetOperand(*m_pIP);
	DWORD dwID = m_GlobalEnvCache.GetID(iIndex);
	if (dwID == CHexeGlobalEnvCache::INVALID_ID)
		{
		dwID = m_Types.Atomize(m_pCodeBank->GetStringLiteral(iIndex));
		if (dwID == CHexeGlobalEnvCache::INVALID_ID)
			return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(iIndex)), retResult);

		m_GlobalEnvCache.SetID(iIndex, dwID);
		}

	m_Stack.Push(m_Types.Get(dwID));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushTensorItem (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			int iIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt(iIndex));
			break;
			}

		case 2:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt2DA(dIndex1, dIndex2));
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt3DA(dIndex1, dIndex2, dIndex3));
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt(m_Types, dIndices));
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushTensorItemI (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			int iIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt(iIndex));
			break;
			}

		case 2:
			{
			int iIndex1 = m_Stack.Pop();	//	Can't be PopInt() because we can't guarantee that it is an integer.
			int iIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt2DI(iIndex1, iIndex2));
			break;
			}

		case 3:
			{
			int iIndex1 = m_Stack.Pop();
			int iIndex2 = m_Stack.Pop();
			int iIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt3DI(iIndex1, iIndex2, iIndex3));
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			m_Stack.Push(dTensor.GetElementAt(m_Types, dIndices));
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushFalse (CDatum& retResult)
	{
	m_Stack.Push(CDatum(false));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecutePushTrue (CDatum& retResult)
	{
	m_Stack.Push(CDatum(true));
	m_pIP++;
	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteReturn (CDatum& retResult)
	{
	const CHexeCallStack::SFrame& Frame = m_CallStack.Top();
	m_dExpression = Frame.dExpression;
	m_pIP = Frame.pIP;
	SetCodeBank(Frame.dCodeBank);

	if (Frame.dwType == CHexeCallStack::FUNC_CALL)
		{
		//	Nothing more to do

		m_CallStack.Pop();
		return ERun::Continue;
		}
	else if (Frame.dwType == CHexeCallStack::SYSTEM_CALL)
		{
		ERun iRunResult = ExecuteReturnFromLibrary(Frame, retResult);
		m_CallStack.Pop();

		if (iRunResult != ERun::OK)
			return iRunResult;

		return ERun::Continue;
		}
	else
		throw CException(errFail);
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetArrayItem (CDatum& retResult)
	{
	//	LATER: Check for reference loops

	CDatum dIndex = m_Stack.Pop();
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();
	if (dIndex.IsNumber())
		{
		int iIndex = dIndex.AsInt32();
		if (iIndex > m_Limits.iMaxArrayLen - 1)
			return RuntimeError(strPattern(ERR_EXCEEDED_ARRAY_LIMITS, ::strFormatInteger(m_Limits.iMaxArrayLen, -1, FORMAT_THOUSAND_SEPARATOR)), retResult);
		}

	dArray.SetElementAt(dIndex, dValue);
	m_Stack.Push(dValue);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetArrayItemI (CDatum& retResult)
	{
	CDatum dArray = m_Stack.Pop();
	CDatum dValue = m_Stack.Get();
	int iIndex = GetOperand(*m_pIP);
	if (iIndex >= 0 && iIndex < dArray.GetCount())
		{
		//	LATER: Maybe add .SetElementRaw (or something) that sets without checking.
		//	[Otherwise we're range checking twice.]
		dArray.SetElement(iIndex, dValue);
		}
	else if (iIndex < m_Limits.iMaxArrayLen)
		{
		dArray.SetElementAt(iIndex, dValue);
		}
	else
		return RuntimeError(strPattern(ERR_EXCEEDED_ARRAY_LIMITS, ::strFormatInteger(m_Limits.iMaxArrayLen, -1, FORMAT_THOUSAND_SEPARATOR)), retResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetForEachItem (CDatum& retResult)
	{
	CDatum dIndex = m_Env.GetLocalEnv().GetArgument(LOCAL_INDEX_VAR);
	CDatum dArray = m_Env.GetLocalEnv().GetArgument(LOCAL_ARRAY_VAR);

	m_Env.GetLocalEnv().SetArgumentValue(LOCAL_ENUM_VAR, dArray.IteratorGetValue(m_Types, dIndex));
	m_Env.GetLocalEnv().SetArgumentValue(LOCAL_KEY_VAR, dArray.IteratorGetKey(dIndex));

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetGlobal (CDatum& retResult)
	{
	DWORD dwID;
	if (!m_Env.GetGlobalEnv().FindSymbol(m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP)), &dwID))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP))), retResult);

	m_Env.GetGlobalEnv().SetAt(dwID, m_Stack.Get());
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetGlobalItem (CDatum& retResult)
	{
	DWORD dwID;
	if (!m_Env.GetGlobalEnv().FindSymbol(m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP)), &dwID))
		return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP))), retResult);

	CDatum dValue = m_Stack.Pop();
	CDatum dKey = m_Stack.Pop();
	CDatum dResult;
	if (!ExecuteSetAt(m_Env.GetGlobalEnv().GetAt(dwID), dKey, dValue, &dResult))
		{
		retResult = dResult;
		return ERun::Error;
		}

	m_Env.GetGlobalEnv().SetAt(dwID, dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetLocal (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);
	m_Env.GetLocalEnv().SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), m_Stack.Get());
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetLocalItem (CDatum& retResult)
	{
	DWORD dwOperand = GetOperand(*m_pIP);

	CDatum dValue = m_Stack.Pop();
	CDatum dKey = m_Stack.Pop();
	CDatum dResult;
	if (!ExecuteSetAt(m_Env.GetLocalEnv().GetArgument((dwOperand >> 8), (dwOperand & 0xff)), dKey, dValue, &dResult))
		{
		retResult = dResult;
		return ERun::Error;
		}

	m_Env.GetLocalEnv().SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), dResult);
	m_Stack.Push(dResult);

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetLocalL0 (CDatum& retResult)
	{
	m_Env.GetLocalEnv().SetArgumentValue(GetOperand(*m_pIP), m_Stack.Get());
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetObjectItem (CDatum& retResult)
	{
	//	LATER: Check for reference loops
	//	LATER: Handle errors inside SetElement.

	CDatum dField = m_Stack.Pop();
	CDatum dObject = m_Stack.Pop();
	CDatum dValue = m_Stack.Pop();

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = dField;

	switch (dObject.GetBasicType())
		{
		case CDatum::typeCustom:
			if (!ExecuteSetCustomMemberItem(dObject, sField, dValue, retResult))
				return ERun::Error;
			break;

		default:
			dObject.SetElement(sField, dValue);
			break;
		}

	m_Stack.Push(dValue);
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetObjectItem2 (CDatum& retResult)
	{
	//	This is an optimized version of opSetObjectItem. We expect
	//	the object and the field value to be on the stack and the
	//	field name to be in the operand.
	//
	//	We leave the array object on the stack (because this 
	//	optimizes for setting multiple fields).

	//	LATER: Check for reference loops
	//	LATER: Handle errors inside SetElement.

	//	NOTE: For performance reasons, this should always be a 
	//	CDatum::typeString. Compilers should enforce this.

	CStringView sField = GetStringFromDataBlock(GetOperand(*m_pIP));
	CDatum dValue = m_Stack.Pop();
	CDatum dObject = m_Stack.Get();

	switch (dObject.GetBasicType())
		{
		case CDatum::typeCustom:
			if (!ExecuteSetCustomMemberItem(dObject, sField, dValue, retResult))
				return ERun::Error;
			break;

		default:
			dObject.SetElement(sField, dValue);
			break;
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetTensorItem (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt(dIndex, dValue);
			m_Stack.Push(dValue);
			break;
			}

		case 2:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt2DA(dIndex1, dIndex2, dValue);
			m_Stack.Push(dValue);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt3DA(dIndex1, dIndex2, dIndex3, dValue);
			m_Stack.Push(dValue);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt(dIndices, dValue);
			m_Stack.Push(dValue);
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSetTensorItemI (CDatum& retResult)
	{
	int iCount = GetOperand(*m_pIP);
	switch (iCount)
		{
		case 0:
			m_Stack.Pop();	//	Tensor
			m_Stack.Push(CDatum());
			break;

		case 1:
			{
			CDatum dIndex = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt(dIndex, dValue);
			m_Stack.Push(dValue);
			break;
			}

		case 2:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt2DI(dIndex1, dIndex2, dValue);
			m_Stack.Push(dValue);
			break;
			}

		case 3:
			{
			CDatum dIndex1 = m_Stack.Pop();
			CDatum dIndex2 = m_Stack.Pop();
			CDatum dIndex3 = m_Stack.Pop();
			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt3DI(dIndex1, dIndex2, dIndex3, dValue);
			m_Stack.Push(dValue);
			break;
			}

		default:
			{
			CDatum dIndices(CDatum::typeArray);
			for (int i = 0; i < iCount; i++)
				dIndices.Append(m_Stack.Pop());

			CDatum dTensor = m_Stack.Pop();
			CDatum dValue = m_Stack.Pop();
			dTensor.SetElementAt(dIndices, dValue);
			m_Stack.Push(dValue);
			break;
			}
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSubtract (CDatum& retResult)
	{
	//	This is the legacy version used by HexeLisp. Is is deprecated
	//	and should not be used by GridLang.

	int iCount = GetOperand(*m_pIP);

	if (iCount == 2)
		{
		CDatum dB = m_Stack.Pop();
		CDatum dA = m_Stack.Pop();

		m_Stack.Push(ExecuteOpSubtractCompatible(dA, dB));
		}
	else if (iCount == 0)
		m_Stack.Push(CDatum(0));

	else if (iCount == 1)
		{
		CNumberValue Result(CDatum(0));
		Result.Subtract(m_Stack.Pop());
		m_Stack.Push(Result.GetDatum());
		}
	else
		{
		CNumberValue Result(CDatum(0));

		for (int i = 0; i < iCount - 1; i++)
			Result.Subtract(m_Stack.Pop());

		Result.Add(m_Stack.Pop());
		m_Stack.Push(Result.GetDatum());
		}

	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSubtract2 (CDatum& retResult)
	{
	CDatum dB = m_Stack.Pop();
	CDatum dA = m_Stack.Pop();

	m_Stack.Push(CAEONOp::Subtract(dA, dB));
	m_pIP++;

	return ERun::Continue;
	}

CHexeProcess::ERun CHexeProcess::ExecuteSubtractInt (CDatum& retResult)
	{
	//	Param is guaranteed to be Int32
	LONGLONG iResult = (LONGLONG)m_Stack.Get().raw_GetInt32() - (LONGLONG)GetOperand(*m_pIP);
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		m_Stack.GetRef() = CDatum((int)iResult);
	else
		m_Stack.GetRef() = CDatum(CIPInteger(iResult));

	m_pIP++;

	return ERun::Continue;
	}

CDatum CHexeProcess::ExecuteBinaryOp (IInvokeCtx& Ctx, EOpCodes iOp, CDatum dLeft, CDatum dRight)

//	ExecuteBinaryOp
//
//	This is used by callers to duplicate the functionality of VM.

	{
	switch (iOp)
		{
		case opAdd:
		case opAdd2:
			return CAEONOp::Add(dLeft, dRight);

		case opConcat:
			return CAEONOp::Concatenate(Ctx, dLeft, dRight);
			
		case opDivide:
		case opDivide2:
			return CAEONOp::Divide(dLeft, dRight);

		case opIsEqual:
		case opIsEqualMulti:
			return CAEONOp::CompEqual(Ctx, dLeft, dRight);

		case opIsGreater:
		case opIsGreaterMulti:
			return CAEONOp::CompGreaterThan(Ctx, dLeft, dRight);

		case opIsGreaterOrEqual:
		case opIsGreaterOrEqualMulti:
			return CAEONOp::CompGreaterThanOrEqual(Ctx, dLeft, dRight);

		case opIsIdentical:
			return CAEONOp::CompIdentical(Ctx, dLeft, dRight);

		case opIsIn:
			return CAEONOp::In(Ctx, dLeft, dRight);

		case opIsLess:
		case opIsLessMulti:
			return CAEONOp::CompLessThan(Ctx, dLeft, dRight);

		case opIsLessOrEqual:
		case opIsLessOrEqualMulti:
			return CAEONOp::CompLessThanOrEqual(Ctx, dLeft, dRight);

		case opIsNotEqual:
		case opIsNotEqualMulti:
			return CAEONOp::CompNotEqual(Ctx, dLeft, dRight);

		case opIsNotIdentical:
			return CAEONOp::CompNotIdentical(Ctx, dLeft, dRight);

		case opMakeRange:
			return CDatum::CreateRange(dLeft, dRight, CDatum());

		case opMod:
			return CAEONOp::Mod(dLeft, dRight);

		case opMultiply:
		case opMultiply2:
			return CAEONOp::Multiply(dLeft, dRight);

		case opPower:
			return CAEONOp::Power(dLeft, dRight);

		case opSubtract:
		case opSubtract2:
			return CAEONOp::Subtract(dLeft, dRight);

		default:
			return CDatum::CreateError(ERR_UNSUPPORTED_OP);
		}
	}

bool CHexeProcess::ExecuteCustomMemberItem (CDatum dObject, const CString &sField, CDatum &retdResult)

//	ExecuteCustomMemberItem
//
//	Push an object member.

	{
	const CString &sTypename = dObject.GetTypename();

	if (strEquals(sTypename, TYPENAME_HEXE_FUNCTION))
		{
		m_Stack.Push(dObject.GetProperty(sField));
		}
	else
		m_Stack.Push(dObject.GetProperty(sField));

	return true;
	}

CHexeProcess::ERun CHexeProcess::ExecuteHandleInvokeResult (CDatum::InvokeResult iInvokeResult, CDatum dExpression, const SAEONInvokeResult& Result, CDatum *retResult, DWORD dwFlags)

//	ExecuteHandleInvokeResult
//
//	Process the result of a primitive invoke when FALSE is returned.
//	We return FALSE if an error should be returned.

	{
	switch (iInvokeResult)
		{
		case CDatum::InvokeResult::ok:
			*retResult = Result.dResult;
			return ERun::OK;

		case CDatum::InvokeResult::error:
			if (Result.dResult.IsNil())
				*retResult = CDatum::CreateError(strPattern(ERR_NOT_A_FUNCTION, dExpression.AsString()));
			else
				*retResult = Result.dResult;
			return ERun::Error;

		case CDatum::InvokeResult::functionCall:
			{
			CDatum dFunction = Result.dResult;
			CDatum dContext = Result.dContext;

			//	Validate function

			CDatum dNewCodeBank;
			DWORD *pNewIP;
			if (dFunction.GetCallInfo(&dNewCodeBank, &pNewIP) != CDatum::ECallType::Call)
				return RuntimeError(ERR_INVALID_PRIMITIVE_SUB, *retResult);

			if (m_CallStack.GetCount() >= m_Limits.iMaxStackDepth)
				return RuntimeError(ERR_EXCEEDED_RECURSION_LIMITS, *retResult);

			//	Save the library function in the call stack so we return to it 
			//	when we're done

			m_CallStack.PushSysCall(m_dExpression, m_dCodeBank, m_pIP, dExpression, dContext, dwFlags);

			//	Set up environment

			m_Env.PushNewFrame();

			int iArgCount = Result.Args.GetCount();
			for (int i = 0; i < iArgCount; i++)
				m_Env.GetLocalEnv().SetArgumentValue(0, i, Result.Args[i]);

			//	Make the call

			m_dExpression = dFunction;
			SetCodeBank(dNewCodeBank);

			m_pIP = pNewIP;

			//	Continue processing

			return ERun::OK;
			}

		//	If the primitive returns FALSE with an array then it means
		//	that we are calling a subroutine. The array has the following 
		//	elements:
		//
		//	0.	Function to call
		//	1.	Array of parameters to function
		//	2.	Context data for InvokeContinues

		case CDatum::InvokeResult::runFunction:
			{
			CDatum dFunction = Result.dResult.GetElement(0);
			CDatum dArgs = Result.dResult.GetElement(1);
			CDatum dContext = Result.dResult.GetElement(2);

			//	Validate function

			CDatum dNewCodeBank;
			DWORD *pNewIP;
			if (dFunction.GetCallInfo(&dNewCodeBank, &pNewIP) != CDatum::ECallType::Call)
				return RuntimeError(ERR_INVALID_PRIMITIVE_SUB, *retResult);

			if (m_CallStack.GetCount() >= m_Limits.iMaxStackDepth)
				return RuntimeError(ERR_EXCEEDED_RECURSION_LIMITS, *retResult);

			//	Save the library function in the call stack so we return to it 
			//	when we're done

			m_CallStack.PushSysCall(m_dExpression, m_dCodeBank, m_pIP, dExpression, dContext, dwFlags);

			//	Set up environment

			m_Env.PushNewFrame();

			int iArgCount = dArgs.GetCount();
			for (int i = 0; i < iArgCount; i++)
				m_Env.GetLocalEnv().SetArgumentValue(0, i, dArgs.GetElement(i));

			//	Make the call

			m_dExpression = dFunction;
			SetCodeBank(dNewCodeBank);

			m_pIP = pNewIP;

			//	Continue processing

			return ERun::OK;
			}

		//	Input Request

		case CDatum::InvokeResult::runInputRequest:
		case CDatum::InvokeResult::runInputRequestDebugSim:
			{
			//	Restore the environment and advance IP

			if (!(dwFlags & FLAG_NO_ENV))
				m_Env.PopFrame();

			if (!(dwFlags & FLAG_NO_ADVANCE))
				m_pIP++;

			//	If debugSim, then we return the desired dialog exit code.

			if (iInvokeResult == CDatum::InvokeResult::runInputRequestDebugSim)
				{
				*retResult = CDatum(CDatum::typeStruct);
				retResult->SetElement(FIELD_TYPE, TYPE_DIALOG_DEBUG_SIM);
				retResult->SetElement(FIELD_RESULT, Result.dResult);
				}
			else
				*retResult = CDatum();

			//	Block until we get input

			return ERun::InputRequest;
			}

		//	Convert to a Hexarc message send.

		case CDatum::InvokeResult::runInvoke:
			{
			CStringView sMsg = Result.dResult.GetElement(FIELD_MSG);
			CDatum dPayload = Result.dResult.GetElement(FIELD_PAYLOAD);

			//	Restore the environment and advance IP

			if (!(dwFlags & FLAG_NO_ENV))
				m_Env.PopFrame();

			if (!(dwFlags & FLAG_NO_ADVANCE))
				m_pIP++;

			//	Send message. NOTE: We call the version that does not check to
			//	see if the user has invoke rights. Since we've wrapped this 
			//	inside a library function, we assume the library is responsible
			//	for security.

			CDatum dValue;
			if (!SendHexarcMessageSafe(sMsg, dPayload, &dValue))
				{
				*retResult = dValue;
				return ERun::Error;
				}

			//	Async request

			*retResult = dValue;
			return ERun::AsyncRequest;
			}

		//	Otherwise, generic error (this should never happen).

		default:
			return RuntimeError(strPattern(ERR_NOT_A_FUNCTION, dExpression.AsString()), *retResult);
		}
	}

void CHexeProcess::ExecuteMakeArrayFromStack (CDatum dArray, int iElements)

//	ExecuteMakeArrayFromStack
//
//	Returns an array from the given elements on the stack.

	{
	dArray.GrowToFit(iElements);

	//	The elements got pushed on the stack in left-to-right order, so the
	//	last element is at the top of the stack.

	for (int i = iElements - 1; i >= 0; i--)
		{
		CDatum dElement = m_Stack.Get(i);

		//	If this is a spread function, then add its constituent elements.

		if (dElement.GetAnnotation().fSpread)
			{
			CDatum dSubArray = dElement.GetElement(0);

			//	NOTE: GrowToFit handles negative numbers correctly.

			dArray.GrowToFit(dSubArray.GetCount() - 1);
			for (int j = 0; j < dSubArray.GetCount(); j++)
				dArray.Append(dSubArray.GetElement(j));
			}
		else
			{
			dArray.Append(dElement);
			}
		}

	//	Pop the stack

	m_Stack.Pop(iElements);
	}

bool CHexeProcess::ExecuteMakeFlagsFromArray (CDatum dOptions, CDatum dMap, CDatum *retdResult)

//	ExecuteMakeFlagsFromArray
//
//	dOptions is an array of string options.
//	dMap is a mapping from strings to DWORDs.
//
//	We look up each string option in the map and OR together each of the flags
//	to generate a result.

	{
	int i;
	DWORD dwFlags = 0;

	for (i = 0; i < dOptions.GetCount(); i++)
		{
		DWORD dwFlag = (DWORD)(int)dMap.GetElement(dOptions.GetElement(i).AsString());
		if (dwFlag)
			dwFlags |= dwFlag;
		}

	*retdResult = CDatum((int)dwFlags);
	return true;
	}

bool CHexeProcess::ExecuteObjectMemberItem (CDatum dObject, const CString& sField, CDatum& retdResult, bool& retbNoIPInc)

//	ExecuteObjectMemberItem
//
//	Push an object member.

	{
	retbNoIPInc = false;

	const IDatatype &Type = dObject.GetDatatype();
	if (Type.GetMemberCount() > 0)
		{
		int iOrdinal;
		auto iMemberType = Type.HasMember(sField, NULL, &iOrdinal);
		switch (iMemberType)
			{
			case IDatatype::EMemberType::InstanceMethod:
			case IDatatype::EMemberType::StaticMethod:
				{
				CString sFunctionName = CAEONTypes::MakeFullyQualifiedName(Type.GetFullyQualifiedName(), sField);

				DWORD dwID;
				if (!m_Env.GetGlobalEnv().FindSymbol(sFunctionName, &dwID))
					{
					RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, sFunctionName), retdResult);
					return false;
					}

				CDatum dValue = m_Env.GetGlobalEnv().GetAt(dwID);
				m_Stack.Push(dValue);
				break;
				}

			//	For instance values, we need to invoke the method

			case IDatatype::EMemberType::InstanceValue:
				{
				CString sFunctionName = CAEONTypes::MakeFullyQualifiedName(Type.GetFullyQualifiedName(), sField);

				DWORD dwID;
				if (!m_Env.GetGlobalEnv().FindSymbol(sFunctionName, &dwID))
					{
					RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, sFunctionName), retdResult);
					return false;
					}

				CDatum dValue = m_Env.GetGlobalEnv().GetAt(dwID);
				m_Stack.Push(dValue);

				m_Env.PushNewFrame();

				//	Set the this pointer

				m_Env.GetLocalEnv().AppendArgumentValue(dObject);

				//	Call

				CDatum dNewExpression = m_Stack.Pop();
				
				//	If the function changes the instruction pointer, then set up 
				//	the call context here.

				DWORD *pNewIP;
				CDatum dNewCodeBank;
				CDatum::ECallType iCallType = dNewExpression.GetCallInfo(&dNewCodeBank, &pNewIP);

				//	Handle it

				switch (iCallType)
					{
					case CDatum::ECallType::Call:
						{
						if (m_CallStack.GetCount() > m_Limits.iMaxStackDepth)
							{
							RuntimeError(ERR_EXCEEDED_RECURSION_LIMITS, retdResult);
							return false;
							}

						m_CallStack.PushFunCall(m_dExpression, m_dCodeBank, ++m_pIP);
						m_dExpression = dNewExpression;
						SetCodeBank(dNewCodeBank);

						m_pIP = pNewIP;
						retbNoIPInc = true;
						break;
						}

					default:
						{
						RuntimeError(ERR_INVALID_CALL_TYPE, retdResult);
						return false;
						}
					}

				break;
				}

			case IDatatype::EMemberType::DynamicMember:
			case IDatatype::EMemberType::InstanceProperty:
			case IDatatype::EMemberType::InstanceReadOnlyProperty:
				m_Stack.Push(dObject.GetProperty(sField));
				break;

			case IDatatype::EMemberType::InstanceVar:
			case IDatatype::EMemberType::InstanceKeyVar:
				if (iOrdinal != -1)
					m_Stack.Push(dObject.GetElement(iOrdinal));
				else
					m_Stack.Push(dObject.GetProperty(sField));
				break;

			default:
				m_Stack.Push(CDatum());
				break;
			}
		}
	else
		{
		m_Stack.Push(dObject.GetProperty(sField));
		}

	return true;
	}

CDatum CHexeProcess::ExecuteOpAddCompatible (CDatum dLeft, CDatum dRight, bool bConcatenateStrings)

//	ExecuteOpAddCompatible
//
//	Binary operation

	{
	int iValue1;
	int iValue2;

	if (dLeft.IsNil())
		return dRight;
	else if (dRight.IsNil())
		return dLeft;
	else if (bConcatenateStrings
			&& (dLeft.GetBasicType() == CDatum::typeString && dRight.GetBasicType() == CDatum::typeString))
		{
		CStringView sA = dLeft;
		CStringView sB = dRight;
		CString sResult(sA.GetLength() + sB.GetLength());
		utlMemCopy(sA.GetParsePointer(), sResult.GetParsePointer(), sA.GetLength());
		utlMemCopy(sB.GetParsePointer(), sResult.GetParsePointer() + sA.GetLength(), sB.GetLength());
		return CDatum(std::move(sResult));
		}
	else if ((dLeft.GetNumberType(&iValue1) == CDatum::typeInteger32)
			&& (dRight.GetNumberType(&iValue2) == CDatum::typeInteger32))
		{
		LONGLONG iResult = (LONGLONG)iValue1 + (LONGLONG)iValue2;
		if (iResult >= INT_MIN && iResult <= INT_MAX)
			return CDatum((int)iResult);
		else
			{
			CNumberValue Result(dLeft);
			Result.ConvertToIPInteger();
			Result.Add(dRight);
			return Result.GetDatum();
			}
		}
	else if (bConcatenateStrings
			&& (dLeft.GetBasicType() == CDatum::typeString || dRight.GetBasicType() == CDatum::typeString))
		{
		CString sA = dLeft.AsString();
		CString sB = dRight.AsString();
		CString sResult(sA.GetLength() + sB.GetLength());
		utlMemCopy(sA.GetParsePointer(), sResult.GetParsePointer(), sA.GetLength());
		utlMemCopy(sB.GetParsePointer(), sResult.GetParsePointer() + sA.GetLength(), sB.GetLength());
		return CDatum(std::move(sResult));
		}
	else if (dLeft.GetBasicType() == CDatum::typeDateTime && dRight.GetBasicType() == CDatum::typeTimeSpan)
		{
		return CDatum(CDateTime(timeAddTime(dLeft, dRight)));
		}
	else if (dLeft.GetBasicType() == CDatum::typeTimeSpan && dRight.GetBasicType() == CDatum::typeDateTime)
		{
		return CDatum(CDateTime(timeAddTime(dRight, dLeft)));
		}
	else if (dLeft.GetBasicType() == CDatum::typeTimeSpan && dRight.GetBasicType() == CDatum::typeTimeSpan)
		{
		return CDatum(CTimeSpan::Add(dLeft, dRight));
		}
	else
		{
		CNumberValue Result(dLeft);
		Result.Add(dRight);
		return Result.GetDatum();
		}
	}

CDatum CHexeProcess::ExecuteOpDivideMod (CDatum dDividend, CDatum dDivisor)

//	ExecuteOpDivideMod
//
//	Returns a array of quotient and remainder.

	{
	CDatum::Types iDividendType = dDividend.GetBasicType();
	CDatum::Types iDivisorType = dDivisor.GetBasicType();

	if (iDividendType == CDatum::typeIntegerIP 
			|| iDividendType == CDatum::typeDouble
			|| iDivisorType == CDatum::typeIntegerIP
			|| iDivisorType == CDatum::typeDouble)
		{
		CIPInteger Dividend(dDividend.AsIPInteger());
		CIPInteger Divisor(dDivisor.AsIPInteger());

		CIPInteger Quotient;
		CIPInteger Remainder;
		if (!Dividend.DivideMod(Divisor, Quotient, Remainder))
			return CDatum::CreateNaN();

		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(2);
		dResult.Append(Quotient);
		dResult.Append(Remainder);

		return dResult;
		}
	else if (iDividendType == CDatum::typeInteger32 && iDivisorType == CDatum::typeInteger32)
		{
		int iDividend = (int)dDividend;
		int iDivisor = (int)dDivisor;
		if (iDivisor == 0)
			return CDatum::CreateNaN();

		int iQuotient = iDividend / iDivisor;
		int iRemainder = iDividend % iDivisor;

		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(2);
		dResult.Append(iQuotient);
		dResult.Append(iRemainder);

		return dResult;
		}
	else
		{
		return CDatum::CreateNaN();
		}
	}

CDatum CHexeProcess::ExecuteOpSubtractCompatible (CDatum dLeft, CDatum dRight)

//	ExecuteOpSubtractCompatible
//
//	Binary operation

	{
	CDatum::Types iLeftType = dLeft.GetBasicType();
	CDatum::Types iRightType = dRight.GetBasicType();

	int iValue1;
	int iValue2;

	if (iLeftType == CDatum::typeDouble && iRightType == CDatum::typeDouble)
		{
		return CDatum((double)dLeft - (double)dRight);
		}
	else if ((dLeft.GetNumberType(&iValue1) == CDatum::typeInteger32)
			&& (dRight.GetNumberType(&iValue2) == CDatum::typeInteger32))
		{
		LONGLONG iResult = (LONGLONG)iValue1 - (LONGLONG)iValue2;
		if (iResult >= INT_MIN && iResult <= INT_MAX)
			return CDatum((int)iResult);
		else
			{
			CNumberValue Result(dLeft);
			Result.ConvertToIPInteger();
			Result.Subtract(dRight);
			return Result.GetDatum();
			}
		}
	else if (iLeftType == CDatum::typeArray && iRightType == CDatum::typeArray)
		{
		int iCount = Min(dLeft.GetCount(), dRight.GetCount());
		CComplexArray *pArray = new CComplexArray(iCount);
		CDatum dResult(pArray);

		for (int i = 0; i < iCount; i++)
			{
			CDatum dLeftElement = dLeft.GetElement(i);
			CDatum dRightElement = dRight.GetElement(i);
			CDatum::Types iLeftType = dLeftElement.GetBasicType();

			if (dRightElement.GetBasicType() == iLeftType)
				{
				switch (iLeftType)
					{
					case CDatum::typeDouble:
						pArray->SetAt(i, CDatum((double)dLeftElement - (double)dRightElement));
						break;

					default:
						pArray->SetAt(i, ExecuteOpSubtractCompatible(dLeft.GetElement(i), dRight.GetElement(i)));
						break;
					}
				}
			else
				pArray->SetAt(i, ExecuteOpSubtractCompatible(dLeft.GetElement(i), dRight.GetElement(i)));
			}

		return dResult;
		}
	else if (iLeftType == CDatum::typeArray)
		{
		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(dLeft.GetCount());

		for (int i = 0; i < dLeft.GetCount(); i++)
			dResult.Append(ExecuteOpSubtractCompatible(dLeft.GetElement(i), dRight));

		return dResult;
		}
	else if (iRightType == CDatum::typeArray)
		{
		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(dRight.GetCount());

		for (int i = 0; i < dRight.GetCount(); i++)
			dResult.Append(ExecuteOpSubtractCompatible(dLeft, dRight.GetElement(i)));

		return dResult;
		}
	else if (iLeftType == CDatum::typeDateTime && iRightType == CDatum::typeDateTime)
		{
		return CDatum(timeSpan(dRight, dLeft));
		}
	else if (iLeftType == CDatum::typeDateTime && iRightType == CDatum::typeTimeSpan)
		{
		return CDatum(timeSubtractTime(dLeft, dRight));
		}
	else if (iLeftType == CDatum::typeTimeSpan && iRightType == CDatum::typeTimeSpan)
		{
		return CDatum(CTimeSpan::Subtract(dLeft, dRight));
		}
	else
		{
		CNumberValue Result(dLeft);
		Result.Subtract(dRight);
		return Result.GetDatum();
		}
	}

CHexeProcess::ERun CHexeProcess::ExecuteReturnFromLibrary (const CHexeCallStack::SFrame& Frame, CDatum& retResult)

//	ExecuteReturnFromLibrary
//
//	Implementation helper.

	{
	//	Is this an event handler return?

	if (strEquals(Frame.dContext.AsStringView(), TYPE_EVENT_HANDLER_CALL))
		{
		//	Must be in an event handler.

		if (m_iEventHandlerLevel <= 0)
			throw CException(errFail);

		//	Pop the result

		CDatum dEventHandlerResult = m_Stack.Pop();

		//	Continue
		//
		//	NOTE: We don't need to restore m_Env because the function 
		//	did that in its return code (call to opExitEnv).

		m_iEventHandlerLevel--;
		retResult = dEventHandlerResult;
		return ERun::EventHandlerDone;
		}

	//	Otherwise, we return to a library function.

	else
		{
		//	Pop the result

		CDatum dSubResult = m_Stack.Pop();

		//	Continue library invocation

		DWORDLONG dwStart = ::sysGetTickCount64();

		SAEONInvokeResult Result;
		CDatum dPrimitive = Frame.dPrimitive;
		CDatum::InvokeResult iResult = dPrimitive.InvokeContinues(this, Frame.dContext, dSubResult, Result);

		m_dwLibraryTime += ::sysGetTickCount64() - dwStart;

		if (iResult != CDatum::InvokeResult::ok)
			return ExecuteHandleInvokeResult(iResult, dPrimitive, Result, &retResult);

		if (!(Frame.dwFlags & FLAG_NO_ENV))
			m_Env.PopFrame();

		//	Done

		m_Stack.Push(Result.dResult);
		if (!(Frame.dwFlags & FLAG_NO_ADVANCE))
			m_pIP++;
		}

	return ERun::OK;
	}

bool CHexeProcess::ExecuteSetAt (CDatum dOriginal, CDatum dKey, CDatum dValue, CDatum *retdResult)

//	ExecuteSetAt
//
//	Sets ket and value

	{
	int iKey;
	CString sKey;

	//	If the key an integer or string?

	CDatum::Types iKeyType = dKey.GetBasicType();
	bool bKeyIsInteger = (iKeyType == CDatum::typeInteger32 || iKeyType == CDatum::typeDouble);
	if (bKeyIsInteger)
		{
		iKey = (int)dKey;
		if (iKey < 0)
			{
			*retdResult = CDatum::CreateError(strPattern(ERR_INVALID_KEY, dKey.AsString()));
			return false;
			}
		else if (iKey > m_Limits.iMaxArrayLen)
			{
			*retdResult = CDatum::CreateError(ERR_OUT_OF_MEMORY);
			return false;
			}
		}
	else
		{
		sKey = dKey.AsString();
		if (sKey.IsEmpty())
			{
			*retdResult = CDatum::CreateError(strPattern(ERR_INVALID_KEY, dKey.AsString()));
			return false;
			}
		}

	//	Set based on what type of value we have in the original variable.

	switch (dOriginal.GetBasicType())
		{
		//	If nil, then we create a new array or structure, depending on the
		//	key type.

		case CDatum::typeNil:
			{
			if (bKeyIsInteger)
				{
				CComplexArray *pArray = new CComplexArray;
				pArray->InsertEmpty(iKey + 1);
				pArray->SetElement(iKey, dValue);
				*retdResult = CDatum(pArray);
				return true;
				}
			else
				{
				CComplexStruct *pStruct = new CComplexStruct;
				pStruct->SetElement(sKey, dValue);
				*retdResult = CDatum(pStruct);
				return true;
				}
			}

		case CDatum::typeArray:
		case CDatum::typeTensor:
		case CDatum::typeTextLines:
			{
			if (!bKeyIsInteger)
				{
				*retdResult = CDatum::CreateError(strPattern(ERR_INVALID_KEY, dKey.AsString()));
				return false;
				}

			//	Change in place

			dOriginal.SetElement(iKey, dValue);
			*retdResult = dOriginal;
			return true;
			}

		case CDatum::typeStruct:
			{
			if (bKeyIsInteger)
				sKey = dKey.AsString();

			//	Change in place

			dOriginal.SetElement(sKey, dValue);
			*retdResult = dOriginal;
			return true;
			}

		case CDatum::typeCustom:
			{
			if (bKeyIsInteger)
				sKey = dKey.AsString();

			dOriginal.SetElement(sKey, dValue);
			*retdResult = dOriginal;
			return true;
			}

		default:
			*retdResult = CDatum::CreateError(ERR_NOT_ARRAY_OR_STRUCT);
			return false;
		}
	}

bool CHexeProcess::ExecuteSetCustomMemberItem (CDatum dObject, const CString &sField, CDatum dValue, CDatum &retdResult)

//	ExecuteSetCustomMemberItem
//
//	Sets a member item.

	{
	const CString &sTypename = dObject.GetTypename();

	if (strEquals(sTypename, TYPENAME_HEXE_FUNCTION))
		{
		RuntimeError(ERR_CANT_SET_FUNCTION_MEMBER, retdResult);
		return false;
		}
	else
		{
		dObject.SetElement(sField, dValue);
		}

	return true;
	}

CDatum CHexeProcess::ExecuteUnaryOp (EOpCodes iOp, CDatum dValue)

//	ExecuteUnaryOp
//
//	Duplicates VM functionality.

	{
	switch (iOp)
		{
		case opNegate:
			return CAEONOp::Negate(dValue);

		case opNot:
			{
			if (dValue.GetBasicType() == CDatum::typeExpression)
				return CAEONExpression::CreateUnaryOp(CAEONExpression::EOp::Not, dValue.AsExpression());
			else
				return CDatum(dValue.IsNil());
			}

		default:
			return CDatum::CreateError(ERR_UNSUPPORTED_OP);
		}
	}

CHexeProcess::ERun CHexeProcess::RuntimeError (const CString &sError, CDatum &retdResult)

//	CreateRuntimeError
//
//	Helper for returning an error from the compute engine.

	{
	retdResult = CDatum::CreateError(sError);
	return ERun::Error;
	}

void CHexeProcess::SignalPause (bool bPause)

//	SignalPause
//
//	This can be called to exit compute.

	{
	CSmartLock Lock(m_cs);
	m_bSignalPause = bPause;
	}
