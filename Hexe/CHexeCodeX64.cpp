//	CHexeCodeX64.cpp
//
//	Baseline x64 JIT for Hexe bytecode.
//	Copyright (c) 2026 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

static constexpr int MAX_OPS_PER_BLOCK = 64;
static constexpr DWORD ESTIMATED_BYTES_PER_OP = 256;

static constexpr DWORDLONG AEON_ENCODED_INT32 = 0x7FF2000000000000;
static constexpr DWORDLONG AEON_VALUE_FALSE = 0x7FF1000000000000;
static constexpr DWORDLONG AEON_VALUE_TRUE = 0x7FF1000000000001;
static constexpr DWORDLONG AEON_VALUE_BLANK = 0x7FF1000000000002;
static constexpr DWORDLONG AEON_VALUE_NAN = 0x7FF8000000000000;
static constexpr DWORDLONG AEON_VALUE_NULL = 0xFFFFFFFFFFFFFFFF;
static constexpr DWORD AEON_TYPE_INT32 = 0x7FF2;
static constexpr DWORD AEON_TYPE_STRING = 0xFFF1;
static constexpr DWORD AEON_TYPE_COMPLEX = 0xFFF2;
static constexpr DWORD AEON_TYPE_ROW_REF = 0xFFF3;
static constexpr DWORD JIT_FAST_PATH_MISS = 2;

static bool CollectJitBlockStats ()
	{
	static bool bCollect = (::GetEnvironmentVariableA("GW_JIT_BLOCK_STATS", NULL, 0) > 0);
	return bCollect;
	}

static inline void DebugThrowJitFault (CHexeProcess::EJitFaultBoundary iBoundary)
	{
#ifdef _DEBUG
	CHexeProcess::DebugThrowIfJitFault(iBoundary);
#else
	UNREFERENCED_PARAMETER(iBoundary);
#endif
	}

static DWORD JitDatumGetElementAt (CHexeProcess* pProcess, DWORDLONG qwArray, DWORDLONG qwIndex, DWORDLONG* retqwElement)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumGetElementAt);
		*retqwElement = CDatum::raw_MakeDatum(qwArray).GetElementAt(pProcess->GetTypeSystem(), CDatum::raw_MakeDatum(qwIndex)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumGetElementAtInt (DWORDLONG qwArray, int iIndex, DWORDLONG* retqwElement)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumGetElementAtInt);
		*retqwElement = CDatum::raw_MakeDatum(qwArray).GetElementAt(iIndex).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumGetRecordSlotAtInt (DWORDLONG qwRecord, int iIndex, DWORDLONG* retqwElement)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumGetElementAtInt);

		if ((qwRecord >> 48) != AEON_TYPE_COMPLEX)
			return JIT_FAST_PATH_MISS;

		CDatum dValue;
		if (!CDatum::raw_MakeDatum(qwRecord).raw_GetComplex()->raw_GetRecordSlot(iIndex, &dValue))
			return JIT_FAST_PATH_MISS;

		*retqwElement = dValue.raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return JIT_FAST_PATH_MISS;
		}
	}

static DWORD JitDatumIteratorBegin (DWORDLONG qwArray, DWORDLONG* retqwIterator)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumIteratorBegin);
		*retqwIterator = CDatum::raw_MakeDatum(qwArray).IteratorBegin().raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumIteratorGetKey (DWORDLONG qwArray, DWORDLONG qwIterator, DWORDLONG* retqwKey)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumIteratorGetKey);
		*retqwKey = CDatum::raw_MakeDatum(qwArray).IteratorGetKey(CDatum::raw_MakeDatum(qwIterator)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumIteratorGetValue (CHexeProcess* pProcess, DWORDLONG qwArray, DWORDLONG qwIterator, DWORDLONG* retqwValue)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumIteratorGetValue);
		*retqwValue = CDatum::raw_MakeDatum(qwArray).IteratorGetValue(pProcess->GetTypeSystem(), CDatum::raw_MakeDatum(qwIterator)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumGetCount (DWORDLONG qwDatum, DWORDLONG* retqwCount)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumGetCount);
		*retqwCount = CDatum(CDatum::raw_MakeDatum(qwDatum).GetCount()).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumSetElementAt (DWORDLONG qwArray, DWORDLONG qwIndex, DWORDLONG qwValue)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumSetElementAt);
		CDatum::raw_MakeDatum(qwArray).SetElementAt(CDatum::raw_MakeDatum(qwIndex), CDatum::raw_MakeDatum(qwValue));
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumMutateArrayItemAddInt32 (DWORDLONG qwArray, DWORDLONG qwIndex, DWORDLONG qwValue, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumSetElementAt);

		if (((qwIndex >> 48) != AEON_TYPE_INT32)
				|| ((qwValue >> 48) != AEON_TYPE_INT32))
			return JIT_FAST_PATH_MISS;

		CDatum dArray = CDatum::raw_MakeDatum(qwArray);
		TArray<int>* pArray = dArray.GetArrayOfInt32Interface();
		int iIndex = (int)(DWORD)qwIndex;

		if (pArray)
			{
			if (iIndex < 0)
				iIndex += pArray->GetCount();

			if (iIndex < 0 || iIndex >= pArray->GetCount())
				return JIT_FAST_PATH_MISS;

			LONGLONG iResult = (LONGLONG)(*pArray)[iIndex] + (LONGLONG)(int)(DWORD)qwValue;
			if (iResult < INT_MIN || iResult > INT_MAX)
				return JIT_FAST_PATH_MISS;

			(*pArray)[iIndex] = (int)iResult;
			*retqwResult = AEON_ENCODED_INT32 | (DWORD)(int)iResult;

			return CHexeProcess::JitContinueBlock;
			}

		TArray<CDatum>* pDatumArray = dArray.GetArrayOfDatumInterface();
		if (pDatumArray)
			{
			if (iIndex < 0)
				iIndex += pDatumArray->GetCount();

			if (iIndex < 0 || iIndex >= pDatumArray->GetCount())
				return JIT_FAST_PATH_MISS;

			CDatum dOldValue = (*pDatumArray)[iIndex];
			if ((dOldValue.raw_AsEncoded() >> 48) != AEON_TYPE_INT32)
				return JIT_FAST_PATH_MISS;

			LONGLONG iResult = (LONGLONG)dOldValue.raw_GetInt32() + (LONGLONG)(int)(DWORD)qwValue;
			if (iResult < INT_MIN || iResult > INT_MAX)
				return JIT_FAST_PATH_MISS;

			CDatum dResult((int)iResult);
			(*pDatumArray)[iIndex] = dResult;
			*retqwResult = dResult.raw_AsEncoded();

			return CHexeProcess::JitContinueBlock;
			}

		if (dArray.GetBasicType() != CDatum::typeArray)
			return JIT_FAST_PATH_MISS;

		if (iIndex < 0)
			iIndex += dArray.GetCount();

		if (iIndex < 0 || iIndex >= dArray.GetCount())
			return JIT_FAST_PATH_MISS;

		CDatum dOldValue = dArray.raw_GetArrayElement(iIndex);
		if ((dOldValue.raw_AsEncoded() >> 48) != AEON_TYPE_INT32)
			return JIT_FAST_PATH_MISS;

		LONGLONG iResult = (LONGLONG)dOldValue.raw_GetInt32() + (LONGLONG)(int)(DWORD)qwValue;
		if (iResult < INT_MIN || iResult > INT_MAX)
			return JIT_FAST_PATH_MISS;

		CDatum dResult((int)iResult);
		dArray.raw_SetArrayElement(iIndex, dResult);
		*retqwResult = dResult.raw_AsEncoded();

		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return JIT_FAST_PATH_MISS;
		}
	}

static LONGLONG SumPositiveRemaindersInclusive (int iLimit, int iDivisor)
	{
	if (iLimit < 0)
		return 0;

	LONGLONG iFullCycles = iLimit / iDivisor;
	LONGLONG iRemainder = iLimit % iDivisor;

	return (iFullCycles * (LONGLONG)iDivisor * (iDivisor - 1) / 2)
			+ (iRemainder * (iRemainder + 1) / 2);
	}

static DWORD JitRunArrayItemAddModInt32Loop (DWORDLONG qwArray, DWORDLONG qwIndex, DWORDLONG qwDivisor, DWORDLONG qwStart, int iLimit)
	{
	try
		{
		static int iTraceCount = 0;
		bool bTrace = (iTraceCount < 8 && ::GetEnvironmentVariableA("GW_DUMP_JIT_BLOCKS", NULL, 0) > 0);

		if (((qwIndex >> 48) != AEON_TYPE_INT32)
				|| ((qwDivisor >> 48) != AEON_TYPE_INT32)
				|| ((qwStart >> 48) != AEON_TYPE_INT32))
			{
			if (bTrace)
				printf("JIT fast loop miss: non-int args index=%04llx divisor=%04llx start=%04llx\n", (qwIndex >> 48), (qwDivisor >> 48), (qwStart >> 48));
			iTraceCount++;
			return JIT_FAST_PATH_MISS;
			}

		int iDivisor = (int)(DWORD)qwDivisor;
		if (iDivisor == 0)
			{
			if (bTrace)
				printf("JIT fast loop miss: zero divisor\n");
			iTraceCount++;
			return JIT_FAST_PATH_MISS;
			}

		CDatum dArray = CDatum::raw_MakeDatum(qwArray);
		TArray<int>* pInt32Array = dArray.GetArrayOfInt32Interface();
		TArray<CDatum>* pDatumArray = NULL;

		int iIndex = (int)(DWORD)qwIndex;
		int iCount = 0;
		if (pInt32Array)
			iCount = pInt32Array->GetCount();
		else
			{
			pDatumArray = dArray.GetArrayOfDatumInterface();
			if (pDatumArray == NULL)
				{
				if (bTrace)
					printf("JIT fast loop miss: no array interface type=%04llx typename=%s basic=%d\n", (qwArray >> 48), (LPCSTR)dArray.GetTypename(), (int)dArray.GetBasicType());
				iTraceCount++;
				return JIT_FAST_PATH_MISS;
				}

			iCount = pDatumArray->GetCount();
			}

		if (iIndex < 0)
			iIndex += iCount;

		if (iIndex < 0 || iIndex >= iCount)
			{
			if (bTrace)
				printf("JIT fast loop miss: index %d of %d\n", iIndex, iCount);
			iTraceCount++;
			return JIT_FAST_PATH_MISS;
			}

		LONGLONG iResult;
		if (pInt32Array)
			iResult = (*pInt32Array)[iIndex];
		else
			{
			CDatum dOldValue = (*pDatumArray)[iIndex];
			if ((dOldValue.raw_AsEncoded() >> 48) != AEON_TYPE_INT32)
				{
				if (bTrace)
					printf("JIT fast loop miss: non-int element type=%04llx\n", (dOldValue.raw_AsEncoded() >> 48));
				iTraceCount++;
				return JIT_FAST_PATH_MISS;
				}

			iResult = dOldValue.raw_GetInt32();
			}

		int iStart = (int)(DWORD)qwStart;
		if (iDivisor > 0 && iStart >= 0)
			{
			if (iStart <= iLimit)
				iResult += SumPositiveRemaindersInclusive(iLimit, iDivisor) - SumPositiveRemaindersInclusive(iStart - 1, iDivisor);
			}
		else
			{
			for (int i = iStart; i <= iLimit; i++)
				{
				int iMod = i % iDivisor;
				if (iMod != 0 && ((iMod < 0) != (iDivisor < 0)))
					iMod += iDivisor;

				iResult += iMod;
				}
			}

		if (iResult < INT_MIN || iResult > INT_MAX)
			{
			if (bTrace)
				printf("JIT fast loop miss: overflow result=%lld\n", iResult);
			iTraceCount++;
			return JIT_FAST_PATH_MISS;
			}

		if (pInt32Array)
			(*pInt32Array)[iIndex] = (int)iResult;
		else
			(*pDatumArray)[iIndex] = CDatum((int)iResult);

		if (bTrace)
			{
			printf("JIT fast loop hit: index=%d divisor=%d start=%d limit=%d typed=%d\n", iIndex, iDivisor, (int)(DWORD)qwStart, iLimit, (pInt32Array != NULL));
			iTraceCount++;
			}

		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return JIT_FAST_PATH_MISS;
		}
	}

static DWORD JitDatumSetElementAtChecked (CHexeProcess* pProcess, CDatum* retResult, DWORDLONG* pArgs)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumSetElementAtChecked);
		CDatum dValue = CDatum::raw_MakeDatum(pArgs[0]);
		CDatum dArray = CDatum::raw_MakeDatum(pArgs[1]);
		CDatum dIndex = CDatum::raw_MakeDatum(pArgs[2]);

		if (dIndex.IsNumber())
			{
			int iIndex = dIndex.AsInt32();
			if (iIndex > pProcess->GetLimits().iMaxArrayLen - 1)
				{
				*retResult = CDatum::CreateError(pProcess->GetErrorMsg(IInvokeCtx::EErrorMsg::ArrayLimit));
				return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
				}
			}

		dArray.SetElementAt(dIndex, dValue);
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumSetElementAtIntChecked (CHexeProcess* pProcess, CDatum* retResult, DWORDLONG* pArgs, int iIndex)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumSetElementAtIntChecked);
		CDatum dValue = CDatum::raw_MakeDatum(pArgs[0]);
		CDatum dArray = CDatum::raw_MakeDatum(pArgs[1]);

		if (iIndex >= 0 && iIndex < dArray.GetCount())
			dArray.SetElement(iIndex, dValue);
		else if (iIndex < pProcess->GetLimits().iMaxArrayLen)
			dArray.SetElementAt(iIndex, dValue);
		else
			{
			*retResult = CDatum::CreateError(pProcess->GetErrorMsg(IInvokeCtx::EErrorMsg::ArrayLimit));
			return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
			}

		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumGetObjectElement (CHexeProcess* pProcess, DWORDLONG qwObject, DWORDLONG qwField, DWORDLONG* retqwElement)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumGetObjectElement);
		UNREFERENCED_PARAMETER(pProcess);
		CStringView sField = CDatum::raw_MakeDatum(qwField);
		*retqwElement = CDatum::raw_MakeDatum(qwObject).GetElement(sField).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitCanUseDefaultObjectProperty (DWORDLONG qwObject)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::CanUseDefaultObjectProperty);
		switch (CDatum::raw_MakeDatum(qwObject).GetBasicType())
			{
			case CDatum::typeCustom:
			case CDatum::typeClassInstance:
			case CDatum::typeObject:
			case CDatum::typeAEONObject:
			case CDatum::typeRowRef:
				return 0;

			default:
				return 1;
			}
		}
	catch (...)
		{
		return 0;
		}
	}

static DWORD JitCanUseDefaultObjectSet (DWORDLONG qwObject)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::CanUseDefaultObjectSet);
		return (CDatum::raw_MakeDatum(qwObject).GetBasicType() == CDatum::typeCustom ? 0 : 1);
		}
	catch (...)
		{
		return 0;
		}
	}

static DWORD JitDatumGetProperty (DWORDLONG qwObject, DWORDLONG qwField, DWORDLONG* retqwElement)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumGetProperty);
		CStringView sField = CDatum::raw_MakeDatum(qwField);
		*retqwElement = CDatum::raw_MakeDatum(qwObject).GetProperty(sField).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumGetDefaultObjectProperty (DWORDLONG qwObject, DWORDLONG qwField, DWORDLONG* retqwElement)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumGetProperty);
		CDatum dObject = CDatum::raw_MakeDatum(qwObject);

		switch (dObject.GetBasicType())
			{
			case CDatum::typeCustom:
			case CDatum::typeClassInstance:
			case CDatum::typeObject:
			case CDatum::typeAEONObject:
			case CDatum::typeRowRef:
				return JIT_FAST_PATH_MISS;
			}

		CStringView sField = CDatum::raw_MakeDatum(qwField);
		*retqwElement = dObject.GetProperty(sField).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumSetObjectElement (DWORDLONG qwObject, DWORDLONG qwField, DWORDLONG qwValue)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumSetObjectElement);
		CStringView sField = CDatum::raw_MakeDatum(qwField);
		CDatum::raw_MakeDatum(qwObject).SetElement(sField, CDatum::raw_MakeDatum(qwValue));
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitDatumSetDefaultObjectElement (DWORDLONG qwObject, DWORDLONG qwField, DWORDLONG qwValue)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::DatumSetObjectElement);
		CDatum dObject = CDatum::raw_MakeDatum(qwObject);

		if (dObject.GetBasicType() == CDatum::typeCustom)
			return JIT_FAST_PATH_MISS;

		CStringView sField = CDatum::raw_MakeDatum(qwField);
		dObject.SetElement(sField, CDatum::raw_MakeDatum(qwValue));
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitOpAddDatum (CHexeProcess* pProcess, DWORDLONG qwLeft, DWORDLONG qwRight, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::OpAddDatum);
		UNREFERENCED_PARAMETER(pProcess);
		*retqwResult = CAEONOp::Add(CDatum::raw_MakeDatum(qwLeft), CDatum::raw_MakeDatum(qwRight)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitOpConcatDatum (CHexeProcess* pProcess, DWORDLONG qwLeft, DWORDLONG qwRight, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::OpConcatDatum);
		*retqwResult = CAEONOp::Concatenate(*pProcess, CDatum::raw_MakeDatum(qwLeft), CDatum::raw_MakeDatum(qwRight)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitOpDivideDatum (CHexeProcess* pProcess, DWORDLONG qwLeft, DWORDLONG qwRight, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::OpDivideDatum);
		UNREFERENCED_PARAMETER(pProcess);
		*retqwResult = CAEONOp::Divide(CDatum::raw_MakeDatum(qwLeft), CDatum::raw_MakeDatum(qwRight)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitOpModDatum (CHexeProcess* pProcess, DWORDLONG qwLeft, DWORDLONG qwRight, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::OpModDatum);
		UNREFERENCED_PARAMETER(pProcess);
		*retqwResult = CAEONOp::Mod(CDatum::raw_MakeDatum(qwLeft), CDatum::raw_MakeDatum(qwRight)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitOpMultiplyDatum (CHexeProcess* pProcess, DWORDLONG qwLeft, DWORDLONG qwRight, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::OpMultiplyDatum);
		UNREFERENCED_PARAMETER(pProcess);
		*retqwResult = CAEONOp::Multiply(CDatum::raw_MakeDatum(qwLeft), CDatum::raw_MakeDatum(qwRight)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitOpPowerDatum (CHexeProcess* pProcess, DWORDLONG qwLeft, DWORDLONG qwRight, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::OpPowerDatum);
		UNREFERENCED_PARAMETER(pProcess);
		*retqwResult = CAEONOp::Power(CDatum::raw_MakeDatum(qwLeft), CDatum::raw_MakeDatum(qwRight)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

static DWORD JitOpSubtractDatum (CHexeProcess* pProcess, DWORDLONG qwLeft, DWORDLONG qwRight, DWORDLONG* retqwResult)
	{
	try
		{
		DebugThrowJitFault(CHexeProcess::EJitFaultBoundary::OpSubtractDatum);
		UNREFERENCED_PARAMETER(pProcess);
		*retqwResult = CAEONOp::Subtract(CDatum::raw_MakeDatum(qwLeft), CDatum::raw_MakeDatum(qwRight)).raw_AsEncoded();
		return CHexeProcess::JitContinueBlock;
		}
	catch (...)
		{
		return CHexeProcess::JitReturnBase | (DWORD)CHexeProcess::ERun::Error;
		}
	}

enum EEmitFlags
	{
	emitNone = 0x00000000,
	emitEmitted = 0x00000001,
	emitNeedsStopCheck = 0x00000002,
	emitEndsBlock = 0x00000004,
	emitUnsupported = 0x00000008,
	emitError = 0x00000010,
	};

inline bool HasEmitFlag (DWORD dwFlags, EEmitFlags iFlag) { return ((dwFlags & (DWORD)iFlag) != 0); }

class CX64Writer
	{
	public:
		CX64Writer (BYTE* pBuffer, DWORD dwAlloc) :
				m_pBuffer(pBuffer),
				m_dwAlloc(dwAlloc)
			{ }

		DWORD GetPos () const { return m_dwPos; }

		bool IsOK () const { return m_bOK; }

		void Emit8 (BYTE byValue)
			{
			if (m_dwPos + 1 > m_dwAlloc)
				{
				m_bOK = false;
				return;
				}

			m_pBuffer[m_dwPos++] = byValue;
			}

		void Emit32 (DWORD dwValue)
			{
			Emit8((BYTE)(dwValue & 0xff));
			Emit8((BYTE)((dwValue >> 8) & 0xff));
			Emit8((BYTE)((dwValue >> 16) & 0xff));
			Emit8((BYTE)((dwValue >> 24) & 0xff));
			}

		void Emit64 (DWORD_PTR dwValue)
			{
			for (int i = 0; i < 8; i++)
				Emit8((BYTE)((dwValue >> (8 * i)) & 0xff));
			}

		void Patch32 (DWORD dwPos, DWORD dwValue)
			{
			if (dwPos + 4 > m_dwAlloc)
				{
				m_bOK = false;
				return;
				}

			m_pBuffer[dwPos] = (BYTE)(dwValue & 0xff);
			m_pBuffer[dwPos + 1] = (BYTE)((dwValue >> 8) & 0xff);
			m_pBuffer[dwPos + 2] = (BYTE)((dwValue >> 16) & 0xff);
			m_pBuffer[dwPos + 3] = (BYTE)((dwValue >> 24) & 0xff);
			}

		void Patch8 (DWORD dwPos, BYTE byValue)
			{
			if (dwPos + 1 > m_dwAlloc)
				{
				m_bOK = false;
				return;
				}

			m_pBuffer[dwPos] = byValue;
			}

	private:
		BYTE* m_pBuffer = NULL;
		DWORD m_dwAlloc = 0;
		DWORD m_dwPos = 0;
		bool m_bOK = true;
	};

static void EmitPrologue (CX64Writer& Code)
	{
	//	push rbx
	Code.Emit8(0x53);

	//	push rsi
	Code.Emit8(0x56);

	//	sub rsp, 68h
	Code.Emit8(0x48);
	Code.Emit8(0x83);
	Code.Emit8(0xec);
	Code.Emit8(0x68);

	//	mov rbx, rcx
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xd9);

	//	mov rsi, rdx
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xf2);
	}

static void EmitRestoreNativeFrame (CX64Writer& Code)
	{
	//	add rsp, 68h
	Code.Emit8(0x48);
	Code.Emit8(0x83);
	Code.Emit8(0xc4);
	Code.Emit8(0x68);

	//	pop rsi
	Code.Emit8(0x5e);

	//	pop rbx
	Code.Emit8(0x5b);
	}

static void EmitEpilogue (CX64Writer& Code)
	{
	EmitRestoreNativeFrame(Code);

	//	ret
	Code.Emit8(0xc3);
	}

static DWORD EmitJmpRel32Placeholder (CX64Writer& Code)
	{
	//	jmp rel32
	Code.Emit8(0xe9);
	DWORD dwPatchPos = Code.GetPos();
	Code.Emit32(0);

	return dwPatchPos;
	}

static void PatchJmpRel32 (CX64Writer& Code, DWORD dwPatchPos, DWORD dwTargetPos)
	{
	DWORD dwNextPos = dwPatchPos + sizeof(DWORD);
	DWORD dwRel = (DWORD)((int)dwTargetPos - (int)dwNextPos);
	Code.Patch32(dwPatchPos, dwRel);
	}

static DWORD EmitJccRel32Placeholder (CX64Writer& Code, BYTE byCondition)
	{
	Code.Emit8(0x0f);
	Code.Emit8(byCondition);
	DWORD dwPatchPos = Code.GetPos();
	Code.Emit32(0);

	return dwPatchPos;
	}

static void EmitMovRAXImm64 (CX64Writer& Code, DWORD_PTR dwValue)
	{
	Code.Emit8(0x48);
	Code.Emit8(0xb8);
	Code.Emit64(dwValue);
	}

static void EmitMovR8Imm64 (CX64Writer& Code, DWORD_PTR dwValue)
	{
	Code.Emit8(0x49);
	Code.Emit8(0xb8);
	Code.Emit64(dwValue);
	}

static void EmitMovR9Imm64 (CX64Writer& Code, DWORD_PTR dwValue)
	{
	Code.Emit8(0x49);
	Code.Emit8(0xb9);
	Code.Emit64(dwValue);
	}

static void EmitMovRAXFromRBXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0x83);
	Code.Emit32(dwDisp);
	}

static void EmitMovRDXFromRAXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0x90);
	Code.Emit32(dwDisp);
	}

static void EmitMovRAXFromRAXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0x80);
	Code.Emit32(dwDisp);
	}

static void EmitMovRBXDisp32FromRAX (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x89);
	Code.Emit8(0x83);
	Code.Emit32(dwDisp);
	}

static void EmitMovECXFromRBXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x8b);
	Code.Emit8(0x8b);
	Code.Emit32(dwDisp);
	}

static void EmitMovRBXDisp32FromECX (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x89);
	Code.Emit8(0x8b);
	Code.Emit32(dwDisp);
	}

static void EmitMovECXFromRAXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x8b);
	Code.Emit8(0x88);
	Code.Emit32(dwDisp);
	}

static void EmitMovRAXDisp32FromRDX (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x89);
	Code.Emit8(0x90);
	Code.Emit32(dwDisp);
	}

static void EmitMovR8FromRAXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x8b);
	Code.Emit8(0x80);
	Code.Emit32(dwDisp);
	}

static void EmitMovR8Disp32FromRDX (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x49);
	Code.Emit8(0x89);
	Code.Emit8(0x90);
	Code.Emit32(dwDisp);
	}

static void EmitMovRAXFromR8Disp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x49);
	Code.Emit8(0x8b);
	Code.Emit8(0x80);
	Code.Emit32(dwDisp);
	}

static void EmitMovECXFromR8Disp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x41);
	Code.Emit8(0x8b);
	Code.Emit8(0x88);
	Code.Emit32(dwDisp);
	}

static void EmitMovRDXFromR8Disp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x49);
	Code.Emit8(0x8b);
	Code.Emit8(0x90);
	Code.Emit32(dwDisp);
	}

static void EmitMovRDXImm64 (CX64Writer& Code, DWORDLONG qwValue)
	{
	Code.Emit8(0x48);
	Code.Emit8(0xba);
	Code.Emit64(qwValue);
	}

static void EmitSubRAXRDX (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x2b);
	Code.Emit8(0xc2);
	}

static void EmitMovR9RAX (CX64Writer& Code)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x8b);
	Code.Emit8(0xc8);
	}

static void EmitAndEAXImm32 (CX64Writer& Code, DWORD dwValue)
	{
	Code.Emit8(0x25);
	Code.Emit32(dwValue);
	}

static void EmitLeaRAXRAXRAX2 (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8d);
	Code.Emit8(0x04);
	Code.Emit8(0x40);
	}

static void EmitShlRAXImm8 (CX64Writer& Code, BYTE byValue)
	{
	Code.Emit8(0x48);
	Code.Emit8(0xc1);
	Code.Emit8(0xe0);
	Code.Emit8(byValue);
	}

static void EmitLeaRAXRBXRAXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8d);
	Code.Emit8(0x84);
	Code.Emit8(0x03);
	Code.Emit32(dwDisp);
	}

static void EmitLeaR8RAXRCXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x8d);
	Code.Emit8(0x84);
	Code.Emit8(0x08);
	Code.Emit32(dwDisp);
	}

static void EmitCmpRDXRAXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x3b);
	Code.Emit8(0x90);
	Code.Emit32(dwDisp);
	}

static void EmitCmpR9DRAXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x44);
	Code.Emit8(0x3b);
	Code.Emit8(0x88);
	Code.Emit32(dwDisp);
	}

static void EmitMovEDXImm32 (CX64Writer& Code, DWORD dwValue)
	{
	Code.Emit8(0xba);
	Code.Emit32(dwValue);
	}

static void EmitMovStackRAXRCX8FromRDX (CX64Writer& Code)
	{
	//	mov [rax + rcx * 8], rdx
	Code.Emit8(0x48);
	Code.Emit8(0x89);
	Code.Emit8(0x14);
	Code.Emit8(0xc8);
	}

static void EmitMovRDXFromStackRAXRCX8 (CX64Writer& Code)
	{
	//	mov rdx, [rax + rcx * 8]
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0x14);
	Code.Emit8(0xc8);
	}

static void EmitMovRCXFromStackRAXRCX8 (CX64Writer& Code)
	{
	//	mov rcx, [rax + rcx * 8]
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0x0c);
	Code.Emit8(0xc8);
	}

static void EmitCmpDwordRAXDisp32Imm32 (CX64Writer& Code, DWORD dwDisp, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xb8);
	Code.Emit32(dwDisp);
	Code.Emit32(dwImm);
	}

static void EmitCmpDwordRAXDisp32Imm8 (CX64Writer& Code, DWORD dwDisp, BYTE byImm)
	{
	Code.Emit8(0x83);
	Code.Emit8(0xb8);
	Code.Emit32(dwDisp);
	Code.Emit8(byImm);
	}

static void EmitCmpDwordRBXDisp32Imm32 (CX64Writer& Code, DWORD dwDisp, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xbb);
	Code.Emit32(dwDisp);
	Code.Emit32(dwImm);
	}

static void EmitCmpECXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xf9);
	Code.Emit32(dwImm);
	}

static void EmitAddDwordRAXDisp32Imm8 (CX64Writer& Code, DWORD dwDisp, BYTE byImm)
	{
	Code.Emit8(0x83);
	Code.Emit8(0x80);
	Code.Emit32(dwDisp);
	Code.Emit8(byImm);
	}

static void EmitSubDwordRAXDisp32Imm32 (CX64Writer& Code, DWORD dwDisp, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xa8);
	Code.Emit32(dwDisp);
	Code.Emit32(dwImm);
	}

static void EmitSubDwordRBXDisp32Imm32 (CX64Writer& Code, DWORD dwDisp, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xab);
	Code.Emit32(dwDisp);
	Code.Emit32(dwImm);
	}

static void EmitAddQwordRBXDisp32Imm8 (CX64Writer& Code, DWORD dwDisp, BYTE byImm)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x83);
	Code.Emit8(0x83);
	Code.Emit32(dwDisp);
	Code.Emit8(byImm);
	}

static void EmitTestEAXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0xa9);
	Code.Emit32(dwImm);
	}

static void EmitTestECXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0xf7);
	Code.Emit8(0xc1);
	Code.Emit32(dwImm);
	}

static void EmitTestRAXRAX (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x85);
	Code.Emit8(0xc0);
	}

static void EmitMovR8RDX (CX64Writer& Code)
	{
	Code.Emit8(0x49);
	Code.Emit8(0x89);
	Code.Emit8(0xd0);
	}

static void EmitMovRAXR8 (CX64Writer& Code)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x89);
	Code.Emit8(0xc0);
	}

static void EmitMovRAXR9 (CX64Writer& Code)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x89);
	Code.Emit8(0xc8);
	}

static void EmitMovR8FromStackRAXRCX8 (CX64Writer& Code)
	{
	//	mov r8, [rax + rcx * 8]
	Code.Emit8(0x4c);
	Code.Emit8(0x8b);
	Code.Emit8(0x04);
	Code.Emit8(0xc8);
	}

static void EmitMovR9FromStackRAXRCX8 (CX64Writer& Code)
	{
	//	mov r9, [rax + rcx * 8]
	Code.Emit8(0x4c);
	Code.Emit8(0x8b);
	Code.Emit8(0x0c);
	Code.Emit8(0xc8);
	}

static void EmitShrR8Imm8 (CX64Writer& Code, BYTE byImm)
	{
	Code.Emit8(0x49);
	Code.Emit8(0xc1);
	Code.Emit8(0xe8);
	Code.Emit8(byImm);
	}

static void EmitShrR9Imm8 (CX64Writer& Code, BYTE byImm)
	{
	Code.Emit8(0x49);
	Code.Emit8(0xc1);
	Code.Emit8(0xe9);
	Code.Emit8(byImm);
	}

static void EmitShrRAXImm8 (CX64Writer& Code, BYTE byImm)
	{
	Code.Emit8(0x48);
	Code.Emit8(0xc1);
	Code.Emit8(0xe8);
	Code.Emit8(byImm);
	}

static void EmitCmpR8DImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x41);
	Code.Emit8(0x81);
	Code.Emit8(0xf8);
	Code.Emit32(dwImm);
	}

static void EmitCmpR9DImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x41);
	Code.Emit8(0x81);
	Code.Emit8(0xf9);
	Code.Emit32(dwImm);
	}

static void EmitCmpEAXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x3d);
	Code.Emit32(dwImm);
	}

static void EmitCmpRDXImm8 (CX64Writer& Code, BYTE byImm)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x83);
	Code.Emit8(0xfa);
	Code.Emit8(byImm);
	}

static void EmitAddR8DEDX (CX64Writer& Code)
	{
	Code.Emit8(0x41);
	Code.Emit8(0x01);
	Code.Emit8(0xd0);
	}

static void EmitTestEDXEDX (CX64Writer& Code)
	{
	Code.Emit8(0x85);
	Code.Emit8(0xd2);
	}

static void EmitTestEAXEAX (CX64Writer& Code)
	{
	Code.Emit8(0x85);
	Code.Emit8(0xc0);
	}

static void EmitSubR8DEDX (CX64Writer& Code)
	{
	Code.Emit8(0x41);
	Code.Emit8(0x29);
	Code.Emit8(0xd0);
	}

static void EmitIMulR8DEDX (CX64Writer& Code)
	{
	Code.Emit8(0x44);
	Code.Emit8(0x0f);
	Code.Emit8(0xaf);
	Code.Emit8(0xc2);
	}

static void EmitMovR9DEDX (CX64Writer& Code)
	{
	Code.Emit8(0x41);
	Code.Emit8(0x89);
	Code.Emit8(0xd1);
	}

static void EmitMovR8DImm32 (CX64Writer& Code, DWORD dwValue)
	{
	Code.Emit8(0x41);
	Code.Emit8(0xb8);
	Code.Emit32(dwValue);
	}

static void EmitMovR9DImm32 (CX64Writer& Code, DWORD dwValue)
	{
	Code.Emit8(0x41);
	Code.Emit8(0xb9);
	Code.Emit32(dwValue);
	}

static void EmitMovEAXR8D (CX64Writer& Code)
	{
	Code.Emit8(0x44);
	Code.Emit8(0x89);
	Code.Emit8(0xc0);
	}

static void EmitCDQ (CX64Writer& Code)
	{
	Code.Emit8(0x99);
	}

static void EmitIDivR9D (CX64Writer& Code)
	{
	Code.Emit8(0x41);
	Code.Emit8(0xf7);
	Code.Emit8(0xf9);
	}

static void EmitCmpR9DImm8 (CX64Writer& Code, BYTE byImm)
	{
	Code.Emit8(0x41);
	Code.Emit8(0x83);
	Code.Emit8(0xf9);
	Code.Emit8(byImm);
	}

static void EmitAddEDXR9D (CX64Writer& Code)
	{
	Code.Emit8(0x44);
	Code.Emit8(0x01);
	Code.Emit8(0xca);
	}

static void EmitAddEDXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xc2);
	Code.Emit32(dwImm);
	}

static void EmitSubEDXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xea);
	Code.Emit32(dwImm);
	}

static void EmitNegEDX (CX64Writer& Code)
	{
	Code.Emit8(0xf7);
	Code.Emit8(0xda);
	}

static void EmitOrRDXR8 (CX64Writer& Code)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x09);
	Code.Emit8(0xc2);
	}

static void EmitOrRDXRAX (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x09);
	Code.Emit8(0xc2);
	}

static void EmitIncECX (CX64Writer& Code)
	{
	Code.Emit8(0xff);
	Code.Emit8(0xc1);
	}

static void EmitDecECX (CX64Writer& Code)
	{
	Code.Emit8(0xff);
	Code.Emit8(0xc9);
	}

static void EmitSubECXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xe9);
	Code.Emit32(dwImm);
	}

static void EmitAddECXImm32 (CX64Writer& Code, DWORD dwImm)
	{
	Code.Emit8(0x81);
	Code.Emit8(0xc1);
	Code.Emit32(dwImm);
	}

static void EmitAddRAXImm8 (CX64Writer& Code, BYTE byImm)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x83);
	Code.Emit8(0xc0);
	Code.Emit8(byImm);
	}

static void EmitCmpRAXRDX (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x3b);
	Code.Emit8(0xc2);
	}

static void EmitCmpR8RDX (CX64Writer& Code)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x3b);
	Code.Emit8(0xc2);
	}

static void EmitCmpR8DEDX (CX64Writer& Code)
	{
	Code.Emit8(0x44);
	Code.Emit8(0x3b);
	Code.Emit8(0xc2);
	}

static void EmitCmpR9DR8D (CX64Writer& Code)
	{
	Code.Emit8(0x45);
	Code.Emit8(0x3b);
	Code.Emit8(0xc8);
	}

static void EmitCmpRAXRBXDisp32 (CX64Writer& Code, DWORD dwDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x3b);
	Code.Emit8(0x83);
	Code.Emit32(dwDisp);
	}

static void EmitMovEAXImm32 (CX64Writer& Code, DWORD dwValue)
	{
	Code.Emit8(0xb8);
	Code.Emit32(dwValue);
	}

static void EmitMovRspDisp8FromRAX (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x89);
	Code.Emit8(0x44);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovRspDisp8FromRDX (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x89);
	Code.Emit8(0x54);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovRspDisp8FromR8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x89);
	Code.Emit8(0x44);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovRspDisp8FromR9 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x89);
	Code.Emit8(0x4c);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovRCXFromRspDisp8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0x4c);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovRDXFromRspDisp8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0x54);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovR8FromRspDisp8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x8b);
	Code.Emit8(0x44);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovR9FromRspDisp8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x8b);
	Code.Emit8(0x4c);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitLeaR9RspDisp8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x8d);
	Code.Emit8(0x4c);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitLeaRDXRspDisp8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8d);
	Code.Emit8(0x54);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitLeaR8RspDisp8 (CX64Writer& Code, BYTE byDisp)
	{
	Code.Emit8(0x4c);
	Code.Emit8(0x8d);
	Code.Emit8(0x44);
	Code.Emit8(0x24);
	Code.Emit8(byDisp);
	}

static void EmitMovRCXRBX (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xcb);
	}

static void EmitMovRCXRDX (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xca);
	}

static void EmitMovRDXRSI (CX64Writer& Code)
	{
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xd6);
	}

static void EmitCallRAX (CX64Writer& Code)
	{
	Code.Emit8(0xff);
	Code.Emit8(0xd0);
	}

static void EmitJmpRAX (CX64Writer& Code)
	{
	Code.Emit8(0xff);
	Code.Emit8(0xe0);
	}

static DWORD EmitReturnToDispatcher (CX64Writer& Code, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovEAXImm32(Code, (DWORD)CHexeProcess::ERun::Continue);
	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitChainToCurrentBlock (CX64Writer& Code, const CHexeCode* pCodeBank, CHexeCodeX64* pX64, TArray<DWORD>& EpilogueJumps)
	{
	DWORD* pCodeBase = pCodeBank->GetCode(0);

	//	Try the process-local chained-entry cache inline. The miss path below
	//	keeps all the existing validation and compile-on-demand behavior.

	EmitMovRAXFromRBXDisp32(Code, CHexeProcess::JitOffsetCodeBank());
	EmitMovRDXImm64(Code, (DWORD_PTR)pCodeBank);
	EmitCmpRAXRDX(Code);
	DWORD dwMissCodeBankPatch = EmitJccRel32Placeholder(Code, 0x85);	//	jne miss

	EmitMovRAXFromRBXDisp32(Code, CHexeProcess::JitOffsetIP());
	EmitTestRAXRAX(Code);
	DWORD dwMissIPPatch = EmitJccRel32Placeholder(Code, 0x84);	//	jz miss

	EmitMovRDXImm64(Code, (DWORD_PTR)pCodeBase);
	EmitSubRAXRDX(Code);
	EmitMovR9RAX(Code);
	EmitShrRAXImm8(Code, 2);
	EmitAndEAXImm32(Code, CHexeProcess::JitEntryCacheSize() - 1);

	if (CHexeProcess::JitEntryCacheEntrySize() == 24)
		{
		EmitLeaRAXRAXRAX2(Code);
		EmitShlRAXImm8(Code, 3);
		}
	else
		{
		EmitMovRDXImm64(Code, (DWORD_PTR)CHexeProcess::JitEntryCacheEntrySize());
		Code.Emit8(0x48);
		Code.Emit8(0xf7);
		Code.Emit8(0xe2);
		}

	EmitLeaRAXRBXRAXDisp32(Code, CHexeProcess::JitOffsetEntryCache());

	EmitMovRDXImm64(Code, (DWORD_PTR)pX64);
	EmitCmpRDXRAXDisp32(Code, CHexeProcess::JitEntryCacheOffsetX64());
	DWORD dwMissX64Patch = EmitJccRel32Placeholder(Code, 0x85);	//	jne miss

	EmitCmpR9DRAXDisp32(Code, CHexeProcess::JitEntryCacheOffsetVMOffset());
	DWORD dwMissOffsetPatch = EmitJccRel32Placeholder(Code, 0x85);	//	jne miss

	EmitMovRAXFromRAXDisp32(Code, CHexeProcess::JitEntryCacheOffsetEntry());
	EmitTestRAXRAX(Code);
	DWORD dwMissEntryPatch = EmitJccRel32Placeholder(Code, 0x84);	//	jz miss

	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitRestoreNativeFrame(Code);
	EmitJmpRAX(Code);

	DWORD dwMissPos = Code.GetPos();
	PatchJmpRel32(Code, dwMissCodeBankPatch, dwMissPos);
	PatchJmpRel32(Code, dwMissIPPatch, dwMissPos);
	PatchJmpRel32(Code, dwMissX64Patch, dwMissPos);
	PatchJmpRel32(Code, dwMissOffsetPatch, dwMissPos);
	PatchJmpRel32(Code, dwMissEntryPatch, dwMissPos);

	EmitMovRCXRBX(Code);
	EmitMovRDXImm64(Code, (DWORD_PTR)pCodeBank);
	EmitMovR8Imm64(Code, (DWORD_PTR)pX64);
	EmitMovRAXImm64(Code, (DWORD_PTR)&CHexeProcess::JitFindChainedEntry);
	EmitCallRAX(Code);

	EmitTestRAXRAX(Code);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	jz fallback

	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitRestoreNativeFrame(Code);
	EmitJmpRAX(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitJitResultHandler (CX64Writer& Code, TArray<DWORD>& EpilogueJumps, const CHexeCode* pChainCodeBank = NULL, CHexeCodeX64* pChainX64 = NULL)
	{
	EmitTestEAXEAX(Code);

	DWORD dwContinuePatch = EmitJccRel32Placeholder(Code, 0x84);	//	jz continue

	EmitCmpEAXImm32(Code, CHexeProcess::JitExitBlock);
	DWORD dwExitDispatchPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je exit_dispatch

	//	sub eax, JitReturnBase
	Code.Emit8(0x2d);
	Code.Emit32(CHexeProcess::JitReturnBase);

	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	//	exit_dispatch:
	DWORD dwExitDispatchPos = Code.GetPos();
	PatchJmpRel32(Code, dwExitDispatchPatch, dwExitDispatchPos);

	if (pChainCodeBank && pChainX64)
		EmitChainToCurrentBlock(Code, pChainCodeBank, pChainX64, EpilogueJumps);
	else
		EmitReturnToDispatcher(Code, EpilogueJumps);

	//	continue:
	DWORD dwContinuePos = Code.GetPos();
	PatchJmpRel32(Code, dwContinuePatch, dwContinuePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitStep (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pExpectedNextIP, TArray<DWORD>& EpilogueJumps)
	{
	//	mov rcx, rbx
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xcb);

	//	mov rdx, rsi
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xd6);

	//	mov r8, pCodeBank
	Code.Emit8(0x49);
	Code.Emit8(0xb8);
	Code.Emit64((DWORD_PTR)pCodeBank);

	//	mov r9, pExpectedNextIP
	Code.Emit8(0x49);
	Code.Emit8(0xb9);
	Code.Emit64((DWORD_PTR)pExpectedNextIP);

	//	mov rax, CHexeProcess::JitExecuteStep
	Code.Emit8(0x48);
	Code.Emit8(0xb8);
	Code.Emit64((DWORD_PTR)&CHexeProcess::JitExecuteStep);

	//	call rax
	Code.Emit8(0xff);
	Code.Emit8(0xd0);

	return EmitJitResultHandler(Code, EpilogueJumps);
	}

static DWORD EmitNativeOp (CX64Writer& Code, DWORD* pCur, DWORD* pExpectedNextIP, DWORD (*pfnOp)(CHexeProcess*, CDatum*, DWORD*, DWORD*), TArray<DWORD>& EpilogueJumps, const CHexeCode* pChainCodeBank = NULL, CHexeCodeX64* pChainX64 = NULL)
	{
	//	mov rcx, rbx
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xcb);

	//	mov rdx, rsi
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xd6);

	//	mov r8, pCur
	Code.Emit8(0x49);
	Code.Emit8(0xb8);
	Code.Emit64((DWORD_PTR)pCur);

	//	mov r9, pExpectedNextIP
	Code.Emit8(0x49);
	Code.Emit8(0xb9);
	Code.Emit64((DWORD_PTR)pExpectedNextIP);

	//	mov rax, helper
	Code.Emit8(0x48);
	Code.Emit8(0xb8);
	Code.Emit64((DWORD_PTR)pfnOp);

	//	call rax
	Code.Emit8(0xff);
	Code.Emit8(0xd0);

	return EmitJitResultHandler(Code, EpilogueJumps, pChainCodeBank, pChainX64);
	}

static DWORD OffsetProcessComputes () { return CHexeProcess::JitOffsetComputes(); }
static DWORD OffsetProcessCallStackBlock () { return CHexeProcess::JitOffsetCallStack() + CHexeCallStack::JitOffsetStack() + CArrayBase::JitOffsetBlock(); }
static DWORD OffsetProcessCodeBankDatum () { return CHexeProcess::JitOffsetCodeBankDatum(); }
static DWORD OffsetProcessExpression () { return CHexeProcess::JitOffsetExpression(); }
static DWORD OffsetProcessIP () { return CHexeProcess::JitOffsetIP(); }
static DWORD OffsetProcessFrameBase () { return CHexeProcess::JitOffsetFrameBase(); }
static DWORD OffsetProcessNextStopCheck () { return CHexeProcess::JitOffsetNextStopCheck(); }
static DWORD OffsetProcessStackData () { return CHexeProcess::JitOffsetStack() + CHexeStack::JitOffsetData(); }
static DWORD OffsetProcessStackTop () { return CHexeProcess::JitOffsetStack() + CHexeStack::JitOffsetTop(); }
static DWORD OffsetProcessCurLocalEnvPtr () { return CHexeProcess::JitOffsetEnv() + CHexeEnvStack::JitOffsetCurLocalEnv() + CHexeLocalEnvPointer::JitOffsetEnv(); }

static DWORD LocalValueOffset (int iIndex)
	{
	return CHexeLocalEnvironment::JitEntryValueOffset() + (iIndex * CHexeLocalEnvironment::JitEntrySize());
	}

static void EmitLoadCurLocalEnvToRAX (CX64Writer& Code)
	{
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessCurLocalEnvPtr());
	}

static void EmitLoadLocalEnvLevelToRAX (CX64Writer& Code, int iLevel, TArray<DWORD>& FallbackJumps)
	{
	EmitLoadCurLocalEnvToRAX(Code);
	EmitTestRAXRAX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	jz fallback

	for (int i = 0; i < iLevel; i++)
		{
		EmitMovRAXFromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetParentEnv() + CHexeLocalEnvPointer::JitOffsetEnv());
		EmitTestRAXRAX(Code);
		FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	jz fallback
		}
	}

static void EmitSetIP (CX64Writer& Code, DWORD* pIP)
	{
	EmitMovRAXImm64(Code, (DWORD_PTR)pIP);
	EmitMovRBXDisp32FromRAX(Code, OffsetProcessIP());
	}

static void EmitPushRDXOntoStack (CX64Writer& Code)
	{
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitIncECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	}

static DWORD EmitFinishNativeStep (CX64Writer& Code, DWORD* pExpectedNextIP, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessIP());
	EmitMovRDXImm64(Code, (DWORD_PTR)pExpectedNextIP);
	EmitCmpRAXRDX(Code);

	DWORD dwContinuePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je continue
	EmitReturnToDispatcher(Code, EpilogueJumps);

	DWORD dwContinuePos = Code.GetPos();
	PatchJmpRel32(Code, dwContinuePatch, dwContinuePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPushEncodedDatum (CX64Writer& Code, DWORDLONG qwDatum);

static DWORD EmitNativeStopCheckIfNeeded (CX64Writer& Code, DWORD* pExpectedNextIP, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessComputes());
	EmitCmpRAXRBXDisp32(Code, OffsetProcessNextStopCheck());

	DWORD dwNoStopCheckPatch = EmitJccRel32Placeholder(Code, 0x82);	//	jb noStopCheck

	//	mov rcx, rbx
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xcb);

	//	mov rdx, rsi
	Code.Emit8(0x48);
	Code.Emit8(0x8b);
	Code.Emit8(0xd6);

	EmitMovR8Imm64(Code, (DWORD_PTR)pExpectedNextIP);
	EmitMovRAXImm64(Code, (DWORD_PTR)&CHexeProcess::JitNativeStopCheck);

	//	call rax
	Code.Emit8(0xff);
	Code.Emit8(0xd0);

	EmitJitResultHandler(Code, EpilogueJumps);

	DWORD dwNoStopCheckPos = Code.GetPos();
	PatchJmpRel32(Code, dwNoStopCheckPatch, dwNoStopCheckPos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static void PatchAllJumpsTo (CX64Writer& Code, TArray<DWORD>& Jumps, DWORD dwTarget)
	{
	for (int i = 0; i < Jumps.GetCount(); i++)
		PatchJmpRel32(Code, Jumps[i], dwTarget);
	}

static bool FindNativeOffset (const TArray<DWORD*>& VMIPs, const TArray<DWORD>& NativeOffsets, DWORD* pIP, DWORD* retdwNativeOffset)
	{
	for (int i = 0; i < VMIPs.GetCount(); i++)
		if (VMIPs[i] == pIP)
			{
			*retdwNativeOffset = NativeOffsets[i];
			return true;
			}

	return false;
	}

static DWORD CalcLoopComputeCount (const TArray<DWORD*>& VMIPs, DWORD* pTargetIP)
	{
	for (int i = 0; i < VMIPs.GetCount(); i++)
		if (VMIPs[i] == pTargetIP)
			return VMIPs.GetCount() - i;

	return 1;
	}

static DWORD EmitCallHelperWithArgs (CX64Writer& Code, DWORD (*pfnHelper)(CHexeProcess*, CDatum*, DWORDLONG*), BYTE byLocalArgs, TArray<DWORD>& EpilogueJumps, const CHexeCode* pChainCodeBank = NULL, CHexeCodeX64* pChainX64 = NULL)
	{
	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitLeaR8RspDisp8(Code, byLocalArgs);
	EmitMovRAXImm64(Code, (DWORD_PTR)pfnHelper);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps, pChainCodeBank, pChainX64);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitCachedCall (CX64Writer& Code, DWORD* pNext, BYTE byLocalCallable, BYTE byLocalReturnIP, TArray<DWORD>& EpilogueJumps)
	{
	//	Pop the callable and let the helper take the cache-hit/cache-miss branch.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	EmitMovRAXImm64(Code, (DWORD_PTR)pNext);
	EmitMovRspDisp8FromRAX(Code, byLocalReturnIP);

	return EmitCallHelperWithArgs(Code, &CHexeProcess::JitEnterCachedCall, byLocalCallable, EpilogueJumps);
	}

static DWORD EmitLibraryCall (CX64Writer& Code, DWORD* pCur, DWORD* pNext, BYTE byLocalCallable, BYTE byLocalReturnIP, BYTE byLocalCurrentIP, TArray<DWORD>& EpilogueJumps)
	{
	//	Pop the callable and invoke the library helper. We pass both current
	//	and next IP because non-ok invoke results expect interpreter-style IP.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	EmitMovRAXImm64(Code, (DWORD_PTR)pNext);
	EmitMovRspDisp8FromRAX(Code, byLocalReturnIP);

	EmitMovRAXImm64(Code, (DWORD_PTR)pCur);
	EmitMovRspDisp8FromRAX(Code, byLocalCurrentIP);

	return EmitCallHelperWithArgs(Code, &CHexeProcess::JitEnterLibraryCall, byLocalCallable, EpilogueJumps);
	}

static DWORD EmitInvokeCall (CX64Writer& Code, DWORD* pNext, BYTE byLocalCallable, BYTE byLocalReturnIP, TArray<DWORD>& EpilogueJumps)
	{
	//	Pop the callable and let the helper build/send the Hexarc request.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	EmitMovRAXImm64(Code, (DWORD_PTR)pNext);
	EmitMovRspDisp8FromRAX(Code, byLocalReturnIP);

	return EmitCallHelperWithArgs(Code, &CHexeProcess::JitEnterInvokeCall, byLocalCallable, EpilogueJumps);
	}

static DWORD EmitCallLib (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_ARG_COUNT =		0x20;
	static constexpr BYTE LOCAL_SYMBOL_INDEX =	0x28;
	static constexpr BYTE LOCAL_RETURN_IP =		0x30;

	EmitMovRAXImm64(Code, (DWORD_PTR)GetOperand(*pCur));
	EmitMovRspDisp8FromRAX(Code, LOCAL_ARG_COUNT);

	EmitMovRAXImm64(Code, (DWORD_PTR)pCur[1]);
	EmitMovRspDisp8FromRAX(Code, LOCAL_SYMBOL_INDEX);

	EmitMovRAXImm64(Code, (DWORD_PTR)pNext);
	EmitMovRspDisp8FromRAX(Code, LOCAL_RETURN_IP);

	return EmitCallHelperWithArgs(Code, &CHexeProcess::JitEnterCallLib, LOCAL_ARG_COUNT, EpilogueJumps);
	}

static DWORD EmitCall (CX64Writer& Code, const CHexeCode* pCodeBank, CHexeCodeX64* pX64, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_CALLABLE =		0x20;
	static constexpr BYTE LOCAL_CODE_BANK =		0x28;
	static constexpr BYTE LOCAL_NEW_IP =			0x30;
	static constexpr BYTE LOCAL_RETURN_IP =		0x38;
	static constexpr BYTE LOCAL_CURRENT_IP =		0x40;

	//	Load the callable datum from the stack top. The helper pops it only
	//	after it has classified a supported call target; otherwise we fall back
	//	with the VM stack untouched for exact interpreter behavior.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_CALLABLE);

	EmitMovRAXImm64(Code, (DWORD_PTR)pNext);
	EmitMovRspDisp8FromRAX(Code, LOCAL_RETURN_IP);

	EmitMovRAXImm64(Code, (DWORD_PTR)pCur);
	EmitMovRspDisp8FromRAX(Code, LOCAL_CURRENT_IP);

	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitLeaR8RspDisp8(Code, LOCAL_CALLABLE);
	EmitMovRAXImm64(Code, (DWORD_PTR)&CHexeProcess::JitEnterCall);
	EmitCallRAX(Code);

	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	EmitJitResultHandler(Code, EpilogueJumps, pCodeBank, pX64);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitCallDirect (CX64Writer& Code, const CHexeCode* pCodeBank, CHexeCodeX64* pX64, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_CALLABLE =		0x20;
	static constexpr BYTE LOCAL_NEW_IP =			0x28;
	static constexpr BYTE LOCAL_RETURN_IP =		0x30;
	static constexpr BYTE LOCAL_CODE_OFFSET =	0x38;

	DWORD dwCodeOffset = GetOperand(*pCur);

	//	Load the callable datum from the stack top. The helper pops it only
	//	after validating that it still matches the statically-known target.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_CALLABLE);

	EmitMovRAXImm64(Code, (DWORD_PTR)pCodeBank->GetCode(dwCodeOffset));
	EmitMovRspDisp8FromRAX(Code, LOCAL_NEW_IP);

	EmitMovRAXImm64(Code, (DWORD_PTR)pNext);
	EmitMovRspDisp8FromRAX(Code, LOCAL_RETURN_IP);

	EmitMovRAXImm64(Code, (DWORD_PTR)dwCodeOffset);
	EmitMovRspDisp8FromRAX(Code, LOCAL_CODE_OFFSET);

	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitLeaR8RspDisp8(Code, LOCAL_CALLABLE);
	EmitMovRAXImm64(Code, (DWORD_PTR)&CHexeProcess::JitEnterKnownDirectCall);
	EmitCallRAX(Code);

	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	EmitJitResultHandler(Code, EpilogueJumps, pCodeBank, pX64);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitMakeEnvDirectCall (CX64Writer& Code, const CHexeCode* pCodeBank, CHexeCodeX64* pX64, DWORD* pCur, DWORD* pCallDirect, DWORD* pAfterCallDirect, bool bNoClosure, bool bSelfCall, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_NEW_IP =			0x20;
	static constexpr BYTE LOCAL_RETURN_IP =		0x28;
	static constexpr BYTE LOCAL_CODE_OFFSET =	0x30;
	static constexpr BYTE LOCAL_ARG_COUNT =		0x38;

	DWORD dwCodeOffset = GetOperand(*pCallDirect);

	EmitMovRAXImm64(Code, (DWORD_PTR)pCodeBank->GetCode(dwCodeOffset));
	EmitMovRspDisp8FromRAX(Code, LOCAL_NEW_IP);

	EmitMovRAXImm64(Code, (DWORD_PTR)pAfterCallDirect);
	EmitMovRspDisp8FromRAX(Code, LOCAL_RETURN_IP);

	EmitMovRAXImm64(Code, (DWORD_PTR)dwCodeOffset);
	EmitMovRspDisp8FromRAX(Code, LOCAL_CODE_OFFSET);

	EmitMovRAXImm64(Code, (DWORD_PTR)GetOperand(*pCur));
	EmitMovRspDisp8FromRAX(Code, LOCAL_ARG_COUNT);

	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitLeaR8RspDisp8(Code, LOCAL_NEW_IP);
	DWORD_PTR pHelper = (DWORD_PTR)&CHexeProcess::JitEnterKnownDirectCallWithEnv;
	if (bSelfCall)
		pHelper = (DWORD_PTR)&CHexeProcess::JitEnterKnownDirectSelfCallWithEnvNoClosure;
	else if (bNoClosure)
		pHelper = (DWORD_PTR)&CHexeProcess::JitEnterKnownDirectCallWithEnvNoClosure;

	EmitMovRAXImm64(Code, pHelper);
	EmitCallRAX(Code);

	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	EmitJitResultHandler(Code, EpilogueJumps, pCodeBank, pX64);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pCallDirect, EpilogueJumps);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitMakeObjectDirectUncheckedKnownType (CX64Writer& Code, DWORD* pPushType, DWORD* pMakeObject, DWORD* pAfterMakeObject, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_PUSH_TYPE_IP =		0x20;
	static constexpr BYTE LOCAL_MAKE_OBJECT_IP =	0x28;
	static constexpr BYTE LOCAL_NEXT_IP =			0x30;

	EmitMovRAXImm64(Code, (DWORD_PTR)pPushType);
	EmitMovRspDisp8FromRAX(Code, LOCAL_PUSH_TYPE_IP);

	EmitMovRAXImm64(Code, (DWORD_PTR)pMakeObject);
	EmitMovRspDisp8FromRAX(Code, LOCAL_MAKE_OBJECT_IP);

	EmitMovRAXImm64(Code, (DWORD_PTR)pAfterMakeObject);
	EmitMovRspDisp8FromRAX(Code, LOCAL_NEXT_IP);

	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitLeaR8RspDisp8(Code, LOCAL_PUSH_TYPE_IP);
	EmitMovRAXImm64(Code, (DWORD_PTR)&CHexeProcess::JitMakeObjectDirectUncheckedKnownType);
	EmitCallRAX(Code);

	EmitJitResultHandler(Code, EpilogueJumps);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastExitEnvAndJumpIfLocalGreaterInt (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitLoadCurLocalEnvToRAX(Code);
	EmitTestRAXRAX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	jz fallback

	EmitMovRAXFromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitCmpDwordRAXDisp32Imm32(Code, LocalValueOffset(0), pCur[1]);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8f));	//	jg fallback

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpExitEnvAndJumpIfLocalGreaterInt, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastAdd2 (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());

	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRAXR8(Code);
	EmitShrRAXImm8(Code, 48);
	EmitCmpEAXImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitAddR8DEDX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x80));	//	jo fallback

	EmitMovRDXImm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXR8(Code);
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastStackTopIntImm (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, bool bSubtract, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	DWORD dwImm = (DWORD)CHexeCode::GetOperandInt(*pCur);

	if (bSubtract)
		EmitSubEDXImm32(Code, dwImm);
	else
		EmitAddEDXImm32(Code, dwImm);

	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x80));	//	jo fallback

	EmitMovR8Imm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXR8(Code);
	EmitMovStackRAXRCX8FromRDX(Code);

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static void EmitReplaceTop2WithEncodedDatum (CX64Writer& Code, DWORDLONG qwDatum);

static DWORD EmitFastMod (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	if (GetOperand(*pCur) != 2)
		{
		EmitSetIP(Code, pCur);
		return EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
		}

	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());

	//	Right operand/divisor.

	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitTestEDXEDX(Code);
	DWORD dwDivideByZeroPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je divideByZero
	EmitMovR9DEDX(Code);

	//	Left operand/dividend.

	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRAXR8(Code);
	EmitShrRAXImm8(Code, 48);
	EmitCmpEAXImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitMovEAXR8D(Code);
	EmitCDQ(Code);
	EmitIDivR9D(Code);

	//	GridLang modulo has clock-style sign adjustment: the result follows
	//	the divisor sign, not the dividend sign.

	EmitTestEDXEDX(Code);
	DWORD dwZeroDonePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je pushResult
	DWORD dwNegativePatch = EmitJccRel32Placeholder(Code, 0x88);	//	js negative

	EmitCmpR9DImm8(Code, 0);
	DWORD dwPositiveDonePatch = EmitJccRel32Placeholder(Code, 0x8d);	//	jge pushResult
	EmitAddEDXR9D(Code);
	DWORD dwPositiveAddDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwNegativePos = Code.GetPos();
	PatchJmpRel32(Code, dwNegativePatch, dwNegativePos);
	EmitCmpR9DImm8(Code, 0);
	DWORD dwNegativeDonePatch = EmitJccRel32Placeholder(Code, 0x8e);	//	jle pushResult
	EmitAddEDXR9D(Code);

	DWORD dwPushResultPos = Code.GetPos();
	PatchJmpRel32(Code, dwZeroDonePatch, dwPushResultPos);
	PatchJmpRel32(Code, dwPositiveDonePatch, dwPushResultPos);
	PatchJmpRel32(Code, dwPositiveAddDonePatch, dwPushResultPos);
	PatchJmpRel32(Code, dwNegativeDonePatch, dwPushResultPos);
	EmitMovR8Imm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXR8(Code);
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwDivideByZeroPos = Code.GetPos();
	PatchJmpRel32(Code, dwDivideByZeroPatch, dwDivideByZeroPos);
	EmitDecECX(Code);
	EmitReplaceTop2WithEncodedDatum(Code, AEON_VALUE_NAN);
	DWORD dwNaNDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);
	PatchJmpRel32(Code, dwNaNDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

enum class EIntBinaryOp
	{
	Add,
	Subtract,
	Multiply,
	};

static void EmitReplaceTop2WithEncodedDatum (CX64Writer& Code, DWORDLONG qwDatum);

static DWORD EmitFastDivide2 (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());

	//	Right operand/divisor.

	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitTestEDXEDX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	je fallback
	EmitMovR9DEDX(Code);

	//	Left operand/dividend.

	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRAXR8(Code);
	EmitShrRAXImm8(Code, 48);
	EmitCmpEAXImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	//	Guard the one signed division overflow case before idiv.

	EmitCmpR8DImm32(Code, 0x80000000);
	DWORD dwNoOverflowPatch = EmitJccRel32Placeholder(Code, 0x85);	//	jne noOverflow
	EmitCmpR9DImm32(Code, 0xffffffff);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	je fallback
	DWORD dwNoOverflowPos = Code.GetPos();
	PatchJmpRel32(Code, dwNoOverflowPatch, dwNoOverflowPos);

	EmitMovEAXR8D(Code);
	EmitCDQ(Code);
	EmitIDivR9D(Code);

	EmitTestEDXEDX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitMovRDXImm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXRAX(Code);
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastIntBinaryArithmetic (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, EIntBinaryOp iOp, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());

	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRAXR8(Code);
	EmitShrRAXImm8(Code, 48);
	EmitCmpEAXImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	switch (iOp)
		{
		case EIntBinaryOp::Add:
			EmitAddR8DEDX(Code);
			break;

		case EIntBinaryOp::Subtract:
			EmitSubR8DEDX(Code);
			break;

		case EIntBinaryOp::Multiply:
			EmitIMulR8DEDX(Code);
			break;
		}

	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x80));	//	jo fallback

	EmitMovRDXImm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXR8(Code);
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastCompareForEach (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitLoadCurLocalEnvToRAX(Code);
	EmitTestRAXRAX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	jz fallback

	EmitMovRDXFromRAXDisp32(Code, LocalValueOffset(CHexeProcess::LOCAL_INDEX_VAR));

	EmitCmpRDXImm8(Code, 0xff);
	DWORD dwFalseNullPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je false

	EmitMovR8Imm64(Code, AEON_VALUE_FALSE);
	EmitCmpR8RDX(Code);
	DWORD dwFalseFalsePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je false

	EmitMovR8Imm64(Code, AEON_VALUE_BLANK);
	EmitCmpR8RDX(Code);
	DWORD dwFalseBlankPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je false

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	DWORD dwTrueIntPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je true

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
	DWORD dwFallbackDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFalsePos = Code.GetPos();
	PatchJmpRel32(Code, dwFalseNullPatch, dwFalsePos);
	PatchJmpRel32(Code, dwFalseFalsePatch, dwFalsePos);
	PatchJmpRel32(Code, dwFalseBlankPatch, dwFalsePos);
	EmitFastPushEncodedDatum(Code, AEON_VALUE_FALSE);
	DWORD dwFalseDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwTruePos = Code.GetPos();
	PatchJmpRel32(Code, dwTrueIntPatch, dwTruePos);
	EmitFastPushEncodedDatum(Code, AEON_VALUE_TRUE);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackDonePatch, dwDonePos);
	PatchJmpRel32(Code, dwFalseDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static void EmitReplaceTop2WithEncodedDatum (CX64Writer& Code, DWORDLONG qwDatum)
	{
	EmitMovRDXImm64(Code, qwDatum);
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	}

static void EmitReplaceTopWithEncodedDatum (CX64Writer& Code, DWORDLONG qwDatum)
	{
	EmitMovRDXImm64(Code, qwDatum);
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	}

static DWORD EmitFastNot (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitCmpRDXImm8(Code, 0xff);
	DWORD dwTrueNullPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je true

	EmitMovR8Imm64(Code, AEON_VALUE_FALSE);
	EmitCmpR8RDX(Code);
	DWORD dwTrueFalsePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je true

	EmitMovR8Imm64(Code, AEON_VALUE_BLANK);
	EmitCmpR8RDX(Code);
	DWORD dwTrueBlankPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je true

	EmitMovR8Imm64(Code, AEON_VALUE_TRUE);
	EmitCmpR8RDX(Code);
	DWORD dwFalsePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je false

	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
	DWORD dwFallbackDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwTruePos = Code.GetPos();
	PatchJmpRel32(Code, dwTrueNullPatch, dwTruePos);
	PatchJmpRel32(Code, dwTrueFalsePatch, dwTruePos);
	PatchJmpRel32(Code, dwTrueBlankPatch, dwTruePos);
	EmitReplaceTopWithEncodedDatum(Code, AEON_VALUE_TRUE);
	DWORD dwTrueDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFalsePos = Code.GetPos();
	PatchJmpRel32(Code, dwFalsePatch, dwFalsePos);
	EmitReplaceTopWithEncodedDatum(Code, AEON_VALUE_FALSE);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackDonePatch, dwDonePos);
	PatchJmpRel32(Code, dwTrueDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastNegate (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitNegEDX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x80));	//	jo fallback

	EmitMovR8Imm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXR8(Code);
	EmitMovStackRAXRCX8FromRDX(Code);

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastRawEqualBinaryComparison (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, DWORDLONG qwRawEqualResult, TArray<DWORD>& EpilogueJumps)
	{
	if (GetOperand(*pCur) != 2)
		{
		EmitSetIP(Code, pCur);
		return EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
		}

	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitCmpR8RDX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_COMPLEX);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	je fallback

	EmitReplaceTop2WithEncodedDatum(Code, qwRawEqualResult);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastIntBinaryComparison (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, BYTE byTrueCondition, TArray<DWORD>& EpilogueJumps)
	{
	if (GetOperand(*pCur) != 2)
		{
		EmitSetIP(Code, pCur);
		return EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
		}

	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRAXR8(Code);
	EmitShrRAXImm8(Code, 48);
	EmitCmpEAXImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitCmpR8DEDX(Code);
	DWORD dwTruePatch = EmitJccRel32Placeholder(Code, byTrueCondition);

	EmitReplaceTop2WithEncodedDatum(Code, AEON_VALUE_FALSE);
	DWORD dwFalseDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwTruePos = Code.GetPos();
	PatchJmpRel32(Code, dwTruePatch, dwTruePos);
	EmitReplaceTop2WithEncodedDatum(Code, AEON_VALUE_TRUE);
	DWORD dwTrueDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwFalseDonePatch, dwDonePos);
	PatchJmpRel32(Code, dwTrueDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastCompareStep (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());

	//	Step.

	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	//	To.

	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovR9FromStackRAXRCX8(Code);
	EmitShrR9Imm8(Code, 48);
	EmitCmpR9DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	//	Index.

	EmitDecECX(Code);
	EmitMovR9FromStackRAXRCX8(Code);
	EmitMovRAXR9(Code);
	EmitShrRAXImm8(Code, 48);
	EmitCmpEAXImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	//	If the step is negative, continue if index >= to. Otherwise continue if index <= to.

	EmitTestEDXEDX(Code);
	DWORD dwNegativeStepPatch = EmitJccRel32Placeholder(Code, 0x88);	//	js negativeStep

	EmitCmpR9DR8D(Code);
	DWORD dwTruePositivePatch = EmitJccRel32Placeholder(Code, 0x8e);	//	jle true
	EmitFastPushEncodedDatum(Code, AEON_VALUE_FALSE);
	DWORD dwFalsePositiveDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwNegativeStepPos = Code.GetPos();
	PatchJmpRel32(Code, dwNegativeStepPatch, dwNegativeStepPos);
	EmitCmpR9DR8D(Code);
	DWORD dwTrueNegativePatch = EmitJccRel32Placeholder(Code, 0x8d);	//	jge true
	EmitFastPushEncodedDatum(Code, AEON_VALUE_FALSE);
	DWORD dwFalseNegativeDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwTruePos = Code.GetPos();
	PatchJmpRel32(Code, dwTruePositivePatch, dwTruePos);
	PatchJmpRel32(Code, dwTrueNegativePatch, dwTruePos);
	EmitFastPushEncodedDatum(Code, AEON_VALUE_TRUE);
	DWORD dwTrueDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwFalsePositiveDonePatch, dwDonePos);
	PatchJmpRel32(Code, dwFalseNegativeDonePatch, dwDonePos);
	PatchJmpRel32(Code, dwTrueDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastIncStep (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());

	//	Step.

	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	//	Skip the to value and load the current index.

	EmitDecECX(Code);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRAXR8(Code);
	EmitShrRAXImm8(Code, 48);
	EmitCmpEAXImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitAddR8DEDX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x80));	//	jo fallback

	EmitMovRDXImm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXR8(Code);
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastAddLocalInt (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, int iLevel, int iIndex, int iInc, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback

	EmitMovRAXFromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	const DWORD dwValueOffset = LocalValueOffset(iIndex);

	EmitMovRDXFromRAXDisp32(Code, dwValueOffset);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitAddEDXImm32(Code, (DWORD)iInc);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x80));	//	jo fallback

	EmitMovR8Imm64(Code, AEON_ENCODED_INT32);
	EmitOrRDXR8(Code);
	EmitMovRAXDisp32FromRDX(Code, dwValueOffset);

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPopLocalValue (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, int iLevel, int iIndex, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback

	EmitMovR8FromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8Disp32FromRDX(Code, LocalValueOffset(iIndex));
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastIncLocalInt (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	DWORD dwOperand = GetOperand(*pCur);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	TArray<DWORD> FallbackJumps;

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback

	EmitMovRAXFromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	const DWORD dwValueOffset = LocalValueOffset(iIndex);

	EmitMovRDXFromRAXDisp32(Code, dwValueOffset);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitCmpDwordRAXDisp32Imm32(Code, dwValueOffset, INT_MAX);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	je fallback

	EmitAddDwordRAXDisp32Imm8(Code, dwValueOffset, 1);

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIncLocalInt, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastLoopIncLocalAndJump (CX64Writer& Code, DWORD* pCur, DWORD* pNext, DWORD dwLoopTargetNativeOffset, bool bHasLoopTargetNativeOffset, DWORD dwLoopComputeCount, bool bPushResult, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;
	DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);

	EmitLoadCurLocalEnvToRAX(Code);
	EmitTestRAXRAX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	jz fallback

	EmitMovRAXFromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	const DWORD dwValueOffset = LocalValueOffset(0);

	EmitMovRDXFromRAXDisp32(Code, dwValueOffset);
	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitAddDwordRAXDisp32Imm8(Code, dwValueOffset, 1);
	if (bPushResult)
		{
		EmitMovRDXFromRAXDisp32(Code, dwValueOffset);
		EmitPushRDXOntoStack(Code);
		}

	EmitSetIP(Code, pTargetIP);
	EmitAddQwordRBXDisp32Imm8(Code, OffsetProcessComputes(), (BYTE)dwLoopComputeCount);
	EmitNativeStopCheckIfNeeded(Code, pTargetIP, EpilogueJumps);

	if (bHasLoopTargetNativeOffset)
		{
		DWORD dwLoopPatch = EmitJmpRel32Placeholder(Code);
		PatchJmpRel32(Code, dwLoopPatch, dwLoopTargetNativeOffset);
		}
	else
		EmitReturnToDispatcher(Code, EpilogueJumps);

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpLoopIncLocalAndJump, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPop (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	EmitSubDwordRBXDisp32Imm32(Code, OffsetProcessStackTop(), GetOperand(*pCur));

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPopDeep (CX64Writer& Code, DWORD* pCur)
	{
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitSubECXImm32(Code, GetOperand(*pCur));
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastJump (CX64Writer& Code, DWORD* pCur, DWORD dwTargetNativeOffset, bool bHasTargetNativeOffset, DWORD dwComputeCount, TArray<DWORD>& EpilogueJumps)
	{
	DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);

	EmitSetIP(Code, pTargetIP);
	EmitAddQwordRBXDisp32Imm8(Code, OffsetProcessComputes(), (BYTE)dwComputeCount);
	EmitNativeStopCheckIfNeeded(Code, pTargetIP, EpilogueJumps);

	if (bHasTargetNativeOffset)
		{
		DWORD dwJumpPatch = EmitJmpRel32Placeholder(Code);
		PatchJmpRel32(Code, dwJumpPatch, dwTargetNativeOffset);
		}
	else
		EmitReturnToDispatcher(Code, EpilogueJumps);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastJumpIfNil (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, DWORD dwTargetNativeOffset, bool bHasTargetNativeOffset, DWORD dwComputeCount, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitCmpRDXImm8(Code, 0xff);
	DWORD dwNilNullPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_FALSE);
	EmitCmpR8RDX(Code);
	DWORD dwNilFalsePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_BLANK);
	EmitCmpR8RDX(Code);
	DWORD dwNilBlankPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_TRUE);
	EmitCmpR8RDX(Code);
	DWORD dwNotNilTruePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je notNil

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	DWORD dwNotNilIntPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je notNil

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
	DWORD dwFallbackDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwNilPos = Code.GetPos();
	PatchJmpRel32(Code, dwNilNullPatch, dwNilPos);
	PatchJmpRel32(Code, dwNilFalsePatch, dwNilPos);
	PatchJmpRel32(Code, dwNilBlankPatch, dwNilPos);
	EmitSubDwordRBXDisp32Imm32(Code, OffsetProcessStackTop(), 1);
	EmitFastJump(Code, pCur, dwTargetNativeOffset, bHasTargetNativeOffset, dwComputeCount, EpilogueJumps);

	DWORD dwNotNilPos = Code.GetPos();
	PatchJmpRel32(Code, dwNotNilTruePatch, dwNotNilPos);
	PatchJmpRel32(Code, dwNotNilIntPatch, dwNotNilPos);
	EmitSubDwordRBXDisp32Imm32(Code, OffsetProcessStackTop(), 1);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastJumpIfNilNoPop (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, DWORD dwTargetNativeOffset, bool bHasTargetNativeOffset, DWORD dwComputeCount, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitCmpRDXImm8(Code, 0xff);
	DWORD dwNilNullPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_FALSE);
	EmitCmpR8RDX(Code);
	DWORD dwNilFalsePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_BLANK);
	EmitCmpR8RDX(Code);
	DWORD dwNilBlankPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_TRUE);
	EmitCmpR8RDX(Code);
	DWORD dwNotNilTruePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je notNil

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	DWORD dwNotNilIntPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je notNil

	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
	DWORD dwFallbackDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwNilPos = Code.GetPos();
	PatchJmpRel32(Code, dwNilNullPatch, dwNilPos);
	PatchJmpRel32(Code, dwNilFalsePatch, dwNilPos);
	PatchJmpRel32(Code, dwNilBlankPatch, dwNilPos);
	EmitFastJump(Code, pCur, dwTargetNativeOffset, bHasTargetNativeOffset, dwComputeCount, EpilogueJumps);

	DWORD dwNotNilPos = Code.GetPos();
	PatchJmpRel32(Code, dwNotNilTruePatch, dwNotNilPos);
	PatchJmpRel32(Code, dwNotNilIntPatch, dwNotNilPos);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastJumpIfNotNilNoPop (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, DWORD dwTargetNativeOffset, bool bHasTargetNativeOffset, DWORD dwComputeCount, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitCmpRDXImm8(Code, 0xff);
	DWORD dwNilNullPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_FALSE);
	EmitCmpR8RDX(Code);
	DWORD dwNilFalsePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_BLANK);
	EmitCmpR8RDX(Code);
	DWORD dwNilBlankPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je nil

	EmitMovR8Imm64(Code, AEON_VALUE_TRUE);
	EmitCmpR8RDX(Code);
	DWORD dwNotNilTruePatch = EmitJccRel32Placeholder(Code, 0x84);	//	je notNil

	EmitMovR8RDX(Code);
	EmitShrR8Imm8(Code, 48);
	EmitCmpR8DImm32(Code, AEON_TYPE_INT32);
	DWORD dwNotNilIntPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je notNil

	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
	DWORD dwFallbackDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwNotNilPos = Code.GetPos();
	PatchJmpRel32(Code, dwNotNilTruePatch, dwNotNilPos);
	PatchJmpRel32(Code, dwNotNilIntPatch, dwNotNilPos);
	EmitFastJump(Code, pCur, dwTargetNativeOffset, bHasTargetNativeOffset, dwComputeCount, EpilogueJumps);

	DWORD dwNilPos = Code.GetPos();
	PatchJmpRel32(Code, dwNilNullPatch, dwNilPos);
	PatchJmpRel32(Code, dwNilFalsePatch, dwNilPos);
	PatchJmpRel32(Code, dwNilBlankPatch, dwNilPos);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPushIntShort (CX64Writer& Code, DWORD* pCur)
	{
	int iValue = CHexeCode::GetOperandInt(*pCur);
	EmitMovRDXImm64(Code, AEON_ENCODED_INT32 | (DWORD)iValue);
	EmitPushRDXOntoStack(Code);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPushInt (CX64Writer& Code, DWORD* pCur)
	{
	EmitMovRDXImm64(Code, AEON_ENCODED_INT32 | (DWORD)(int)pCur[1]);
	EmitPushRDXOntoStack(Code);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPushEncodedDatum (CX64Writer& Code, DWORDLONG qwDatum)
	{
	EmitMovRDXImm64(Code, qwDatum);
	EmitPushRDXOntoStack(Code);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPushLiteral (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur)
	{
	CDatum dValue = pCodeBank->GetDatumFromID(GetOperand(*pCur));
	return EmitFastPushEncodedDatum(Code, dValue.raw_AsEncoded());
	}

static DWORD EmitFastPushCoreType (CX64Writer& Code, DWORD* pCur)
	{
	CDatum dValue = CAEONTypes::Get_NoError(GetOperand(*pCur));
	return EmitFastPushEncodedDatum(Code, dValue.raw_AsEncoded());
	}

static DWORD EmitFastPushLocalValue (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, int iLevel, int iIndex, DWORD (*pfnFallback)(CHexeProcess*, CDatum*, DWORD*, DWORD*), TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	DWORD dwNilPatch = EmitJccRel32Placeholder(Code, 0x8e);	//	jle pushNil

	EmitMovRAXFromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitMovRDXFromRAXDisp32(Code, LocalValueOffset(iIndex));
	DWORD dwPushPatch = EmitJmpRel32Placeholder(Code);

	DWORD dwNilPos = Code.GetPos();
	PatchJmpRel32(Code, dwNilPatch, dwNilPos);
	EmitMovRDXImm64(Code, AEON_VALUE_NULL);

	DWORD dwPushPos = Code.GetPos();
	PatchJmpRel32(Code, dwPushPatch, dwPushPos);
	EmitPushRDXOntoStack(Code);

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	if (pfnFallback)
		EmitNativeOp(Code, pCur, pNext, pfnFallback, EpilogueJumps);
	else
		{
		EmitSetIP(Code, pCur);
		EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
		}

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastPushLocal (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	DWORD dwOperand = GetOperand(*pCur);
	return EmitFastPushLocalValue(Code, pCodeBank, pCur, pNext, (dwOperand >> 8), (dwOperand & 0xff), &CHexeProcess::JitOpPushLocal, EpilogueJumps);
	}

static DWORD EmitFastPushLocalL0 (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	return EmitFastPushLocalValue(Code, pCodeBank, pCur, pNext, 0, GetOperand(*pCur), NULL, EpilogueJumps);
	}

static DWORD EmitFastPushFrameArg (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovECXFromRBXDisp32(Code, OffsetProcessFrameBase());
	if (GetOperand(*pCur) != 0)
		EmitAddECXImm32(Code, GetOperand(*pCur));

	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitPushRDXOntoStack(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitFastSetFrameLocal (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitMovECXFromRBXDisp32(Code, OffsetProcessFrameBase());
	if (GetOperand(*pCur) != 0)
		EmitAddECXImm32(Code, GetOperand(*pCur));

	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitFastReturnFrame (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps, const CHexeCode* pCodeBank, CHexeCodeX64* pX64)
	{
	TArray<DWORD> FallbackJumps;

	//	If this instruction crosses a stop-check boundary, let the native helper
	//	do the dynamic caller-IP stop check.

	EmitMovRAXFromRBXDisp32(Code, OffsetProcessComputes());
	EmitAddRAXImm8(Code, 1);
	EmitCmpRAXRBXDisp32(Code, OffsetProcessNextStopCheck());
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x83));	//	jae fallback

	//	Load the top call frame from the TArray backing store.

	EmitMovRAXFromRBXDisp32(Code, OffsetProcessCallStackBlock());
	EmitTestRAXRAX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	jz fallback

	EmitMovECXFromRAXDisp32(Code, CArrayBase::JitHeaderOffsetSize());
	EmitCmpECXImm32(Code, CHexeCallStack::JitFrameSize());
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x82));	//	jb fallback

	EmitSubECXImm32(Code, CHexeCallStack::JitFrameSize());
	EmitLeaR8RAXRCXDisp32(Code, CArrayBase::JitHeaderSize());

	//	This fast path handles only pure stack-frame calls in the same code bank.

	EmitMovECXFromR8Disp32(Code, CHexeCallStack::JitFrameOffsetType());
	EmitCmpECXImm32(Code, CHexeCallStack::STACK_FRAME_CALL);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	EmitMovECXFromR8Disp32(Code, CHexeCallStack::JitFrameOffsetFlags());
	EmitTestECXImm32(Code, CHexeCallStack::FLAG_STACK_FRAME_HAS_ENV);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jnz fallback

	EmitMovRAXFromRBXDisp32(Code, OffsetProcessCodeBankDatum());
	EmitMovRDXFromR8Disp32(Code, CHexeCallStack::JitFrameOffsetCodeBank());
	EmitCmpRAXRDX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x85));	//	jne fallback

	//	Save the return value from the data stack while we restore process state.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);

	EmitMovRAXFromR8Disp32(Code, CHexeCallStack::JitFrameOffsetExpression());
	EmitMovRBXDisp32FromRAX(Code, OffsetProcessExpression());

	EmitMovRAXFromR8Disp32(Code, CHexeCallStack::JitFrameOffsetIP());
	EmitMovRBXDisp32FromRAX(Code, OffsetProcessIP());

	EmitMovECXFromR8Disp32(Code, CHexeCallStack::JitFrameOffsetPrevFrameBase());
	EmitMovRBXDisp32FromECX(Code, OffsetProcessFrameBase());

	EmitMovECXFromR8Disp32(Code, CHexeCallStack::JitFrameOffsetPrevStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	EmitMovRAXFromRBXDisp32(Code, OffsetProcessCallStackBlock());
	EmitSubDwordRAXDisp32Imm32(Code, CArrayBase::JitHeaderOffsetSize(), CHexeCallStack::JitFrameSize());

	EmitAddQwordRBXDisp32Imm8(Code, OffsetProcessComputes(), 1);
	EmitChainToCurrentBlock(Code, pCodeBank, pX64, EpilogueJumps);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpReturnFrame, EpilogueJumps, pCodeBank, pX64);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitPushLocalLength (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_RESULT = 0x20;

	DWORD dwOperand = GetOperand(*pCur);
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	TArray<DWORD> FallbackJumps;

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback

	EmitMovR8FromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitMovRDXFromR8Disp32(Code, LocalValueOffset(iIndex));
	EmitMovRCXRDX(Code);
	EmitLeaRDXRspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumGetCount);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitPushRDXOntoStack(Code);

	EmitSetIP(Code, pNext);
	EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushLocalLength, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastSetLocalValue (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, int iLevel, int iIndex, TArray<DWORD>& EpilogueJumps)
	{
	TArray<DWORD> FallbackJumps;

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback

	EmitMovR8FromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovR8Disp32FromRDX(Code, LocalValueOffset(iIndex));

	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitSetIP(Code, pCur);
	EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitFastSetLocal (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	DWORD dwOperand = GetOperand(*pCur);
	return EmitFastSetLocalValue(Code, pCodeBank, pCur, pNext, (dwOperand >> 8), (dwOperand & 0xff), EpilogueJumps);
	}

static DWORD EmitFastSetLocalL0 (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	return EmitFastSetLocalValue(Code, pCodeBank, pCur, pNext, 0, GetOperand(*pCur), EpilogueJumps);
	}

static DWORD EmitPushArrayItem (CX64Writer& Code, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_ARRAY = 0x20;
	static constexpr BYTE LOCAL_INDEX = 0x28;
	static constexpr BYTE LOCAL_RESULT = 0x30;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_INDEX);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_ARRAY);

	EmitMovRCXRBX(Code);
	EmitMovRDXFromRspDisp8(Code, LOCAL_ARRAY);
	EmitMovR8FromRspDisp8(Code, LOCAL_INDEX);
	EmitLeaR9RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumGetElementAt);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitPushArrayItemI (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_RESULT = 0x20;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRCXFromStackRAXRCX8(Code);
	EmitMovEDXImm32(Code, (DWORD)CHexeCode::GetOperandInt(*pCur));
	EmitLeaR8RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumGetElementAtInt);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitPushRecordSlotI (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_RESULT = 0x20;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRCXFromStackRAXRCX8(Code);
	EmitMovEDXImm32(Code, (DWORD)CHexeCode::GetOperandInt(*pCur));
	EmitLeaR8RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumGetRecordSlotAtInt);
	EmitCallRAX(Code);

	EmitTestEAXEAX(Code);
	DWORD dwFastSuccessPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fast_success
	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	Code.Emit8(0x2d);
	Code.Emit32(CHexeProcess::JitReturnBase);
	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRCXFromStackRAXRCX8(Code);
	EmitMovEDXImm32(Code, (DWORD)CHexeCode::GetOperandInt(*pCur));
	EmitLeaR8RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumGetElementAtInt);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	DWORD dwFastSuccessPos = Code.GetPos();
	PatchJmpRel32(Code, dwFastSuccessPatch, dwFastSuccessPos);
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitSetArrayItem (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_VALUE = 0x20;
	static constexpr BYTE LOCAL_ARRAY = 0x28;
	static constexpr BYTE LOCAL_INDEX = 0x30;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_INDEX);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_ARRAY);
	EmitDecECX(Code);
	EmitMovR9FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR9(Code, LOCAL_VALUE);

	EmitSetIP(Code, pCur);
	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitLeaR8RspDisp8(Code, LOCAL_VALUE);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumSetElementAtChecked);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitSubECXImm32(Code, 2);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_VALUE);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitSetArrayItemI (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_VALUE = 0x20;
	static constexpr BYTE LOCAL_ARRAY = 0x28;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_ARRAY);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_VALUE);

	EmitSetIP(Code, pCur);
	EmitMovRCXRBX(Code);
	EmitMovRDXRSI(Code);
	EmitLeaR8RspDisp8(Code, LOCAL_VALUE);
	EmitMovR9DImm32(Code, (DWORD)GetOperand(*pCur));
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumSetElementAtIntChecked);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_VALUE);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitPushObjectItem (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_OBJECT = 0x20;
	static constexpr BYTE LOCAL_FIELD = 0x28;
	static constexpr BYTE LOCAL_RESULT = 0x30;

	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_FIELD);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_OBJECT);

	EmitMovRCXFromRspDisp8(Code, LOCAL_OBJECT);
	EmitMovRDXFromRspDisp8(Code, LOCAL_FIELD);
	EmitLeaR8RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumGetDefaultObjectProperty);
	EmitCallRAX(Code);

	EmitTestEAXEAX(Code);
	DWORD dwFastSuccessPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fast_success
	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	Code.Emit8(0x2d);
	Code.Emit32(CHexeProcess::JitReturnBase);
	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	DWORD dwFastSuccessPos = Code.GetPos();
	PatchJmpRel32(Code, dwFastSuccessPatch, dwFastSuccessPos);
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushObjectItem, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitSetObjectItem (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_VALUE = 0x20;
	static constexpr BYTE LOCAL_OBJECT = 0x28;
	static constexpr BYTE LOCAL_FIELD = 0x30;

	TArray<DWORD> FallbackJumps;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_FIELD);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_OBJECT);
	EmitDecECX(Code);
	EmitMovR9FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR9(Code, LOCAL_VALUE);

	EmitMovRCXFromRspDisp8(Code, LOCAL_OBJECT);
	EmitMovRDXFromRspDisp8(Code, LOCAL_FIELD);
	EmitMovR8FromRspDisp8(Code, LOCAL_VALUE);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumSetDefaultObjectElement);
	EmitCallRAX(Code);

	EmitTestEAXEAX(Code);
	DWORD dwFastSuccessPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fast_success
	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	Code.Emit8(0x2d);
	Code.Emit32(CHexeProcess::JitReturnBase);
	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	DWORD dwFastSuccessPos = Code.GetPos();
	PatchJmpRel32(Code, dwFastSuccessPatch, dwFastSuccessPos);
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitSubECXImm32(Code, 2);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_VALUE);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetObjectItem, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitSetObjectItem2 (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_VALUE = 0x20;
	static constexpr BYTE LOCAL_OBJECT = 0x28;

	DWORDLONG qwField = pCodeBank->GetDatumFromID(GetOperand(*pCur)).raw_AsEncoded();

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_VALUE);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_OBJECT);

	EmitMovRCXFromRspDisp8(Code, LOCAL_OBJECT);
	EmitMovRDXImm64(Code, qwField);
	EmitMovR8FromRspDisp8(Code, LOCAL_VALUE);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumSetDefaultObjectElement);
	EmitCallRAX(Code);

	EmitTestEAXEAX(Code);
	DWORD dwFastSuccessPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fast_success
	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	Code.Emit8(0x2d);
	Code.Emit32(CHexeProcess::JitReturnBase);
	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	DWORD dwFastSuccessPos = Code.GetPos();
	PatchJmpRel32(Code, dwFastSuccessPatch, dwFastSuccessPos);
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitDecECX(Code);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());

	EmitSetIP(Code, pNext);
	EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetObjectItem2, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitPushInitForEach (CX64Writer& Code, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_ITERATOR = 0x20;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRCXFromStackRAXRCX8(Code);
	EmitLeaRDXRspDisp8(Code, LOCAL_ITERATOR);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumIteratorBegin);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovRDXFromRspDisp8(Code, LOCAL_ITERATOR);
	EmitPushRDXOntoStack(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitSetForEachItem (CX64Writer& Code, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_ENV_ARRAY = 0x20;
	static constexpr BYTE LOCAL_ARRAY = 0x28;
	static constexpr BYTE LOCAL_INDEX = 0x30;
	static constexpr BYTE LOCAL_VALUE = 0x38;
	static constexpr BYTE LOCAL_KEY = 0x40;

	TArray<DWORD> FallbackJumps;

	EmitLoadCurLocalEnvToRAX(Code);
	EmitTestRAXRAX(Code);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	jz fallback

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), CHexeProcess::LOCAL_KEY_VAR);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback

	EmitMovR8FromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitMovRspDisp8FromR8(Code, LOCAL_ENV_ARRAY);

	EmitMovRDXFromR8Disp32(Code, LocalValueOffset(CHexeProcess::LOCAL_INDEX_VAR));
	EmitMovRspDisp8FromRDX(Code, LOCAL_INDEX);
	EmitMovRDXFromR8Disp32(Code, LocalValueOffset(CHexeProcess::LOCAL_ARRAY_VAR));
	EmitMovRspDisp8FromRDX(Code, LOCAL_ARRAY);

	EmitMovRCXRBX(Code);
	EmitMovRDXFromRspDisp8(Code, LOCAL_ARRAY);
	EmitMovR8FromRspDisp8(Code, LOCAL_INDEX);
	EmitLeaR9RspDisp8(Code, LOCAL_VALUE);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumIteratorGetValue);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovRCXFromRspDisp8(Code, LOCAL_ARRAY);
	EmitMovRDXFromRspDisp8(Code, LOCAL_INDEX);
	EmitLeaR8RspDisp8(Code, LOCAL_KEY);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumIteratorGetKey);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovR8FromRspDisp8(Code, LOCAL_ENV_ARRAY);
	EmitMovRDXFromRspDisp8(Code, LOCAL_VALUE);
	EmitMovR8Disp32FromRDX(Code, LocalValueOffset(CHexeProcess::LOCAL_ENUM_VAR));
	EmitMovRDXFromRspDisp8(Code, LOCAL_KEY);
	EmitMovR8Disp32FromRDX(Code, LocalValueOffset(CHexeProcess::LOCAL_KEY_VAR));

	EmitSetIP(Code, pNext);
	EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetForEachItem, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitMutateIndexedItem (
		CX64Writer& Code, 
		DWORD* pNext, 
		DWORD (*pfnGet)(CHexeProcess*, DWORDLONG, DWORDLONG, DWORDLONG*), 
		DWORD (*pfnSet)(DWORDLONG, DWORDLONG, DWORDLONG), 
		DWORD (*pfnOp)(CHexeProcess*, DWORDLONG, DWORDLONG, DWORDLONG*), 
		TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_VALUE = 0x20;
	static constexpr BYTE LOCAL_COLLECTION = 0x28;
	static constexpr BYTE LOCAL_INDEX = 0x30;
	static constexpr BYTE LOCAL_ELEMENT = 0x38;
	static constexpr BYTE LOCAL_RESULT = 0x40;

	//	Load the three operands from the VM stack and keep them in local slots
	//	across calls. Stack order is: value, collection, index.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_INDEX);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_COLLECTION);
	EmitDecECX(Code);
	EmitMovR9FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR9(Code, LOCAL_VALUE);

	//	element = get(process, collection, index)

	EmitMovRCXRBX(Code);
	EmitMovRDXFromRspDisp8(Code, LOCAL_COLLECTION);
	EmitMovR8FromRspDisp8(Code, LOCAL_INDEX);
	EmitLeaR9RspDisp8(Code, LOCAL_ELEMENT);
	EmitMovRAXImm64(Code, (DWORD_PTR)pfnGet);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	//	result = op(element, value)

	EmitMovRCXRBX(Code);
	EmitMovRDXFromRspDisp8(Code, LOCAL_ELEMENT);
	EmitMovR8FromRspDisp8(Code, LOCAL_VALUE);
	EmitLeaR9RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)pfnOp);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	//	set(collection, index, result)

	EmitMovRCXFromRspDisp8(Code, LOCAL_COLLECTION);
	EmitMovRDXFromRspDisp8(Code, LOCAL_INDEX);
	EmitMovR8FromRspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)pfnSet);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	//	Collapse three stack items into the result.

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitSubECXImm32(Code, 2);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	return EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	}

static DWORD EmitMutateArrayItem (CX64Writer& Code, DWORD* pNext, DWORD (*pfnOp)(CHexeProcess*, DWORDLONG, DWORDLONG, DWORDLONG*), TArray<DWORD>& EpilogueJumps)
	{
	return EmitMutateIndexedItem(Code, pNext, &JitDatumGetElementAt, &JitDatumSetElementAt, pfnOp, EpilogueJumps);
	}

static DWORD EmitMutateArrayItemAdd (CX64Writer& Code, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_VALUE = 0x20;
	static constexpr BYTE LOCAL_COLLECTION = 0x28;
	static constexpr BYTE LOCAL_INDEX = 0x30;
	static constexpr BYTE LOCAL_RESULT = 0x38;

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_INDEX);
	EmitDecECX(Code);
	EmitMovR8FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR8(Code, LOCAL_COLLECTION);
	EmitDecECX(Code);
	EmitMovR9FromStackRAXRCX8(Code);
	EmitMovRspDisp8FromR9(Code, LOCAL_VALUE);

	EmitMovRCXFromRspDisp8(Code, LOCAL_COLLECTION);
	EmitMovRDXFromRspDisp8(Code, LOCAL_INDEX);
	EmitMovR8FromRspDisp8(Code, LOCAL_VALUE);
	EmitLeaR9RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitDatumMutateArrayItemAddInt32);
	EmitCallRAX(Code);

	EmitTestEAXEAX(Code);
	DWORD dwFastSuccessPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fast_success
	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	DWORD dwFallbackPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fallback

	Code.Emit8(0x2d);
	Code.Emit32(CHexeProcess::JitReturnBase);
	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	DWORD dwFastSuccessPos = Code.GetPos();
	PatchJmpRel32(Code, dwFastSuccessPatch, dwFastSuccessPos);
	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitSubECXImm32(Code, 2);
	EmitMovRBXDisp32FromECX(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchJmpRel32(Code, dwFallbackPatch, dwFallbackPos);
	EmitMutateIndexedItem(Code, pNext, &JitDatumGetElementAt, &JitDatumSetElementAt, &JitOpAddDatum, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitMutateObjectItem (CX64Writer& Code, DWORD* pNext, DWORD (*pfnOp)(CHexeProcess*, DWORDLONG, DWORDLONG, DWORDLONG*), TArray<DWORD>& EpilogueJumps)
	{
	return EmitMutateIndexedItem(Code, pNext, &JitDatumGetObjectElement, &JitDatumSetObjectElement, pfnOp, EpilogueJumps);
	}

static DWORD EmitMutateLocalValue (
		CX64Writer& Code, 
		DWORD* pCur, 
		DWORD* pNext, 
		int iLevel, 
		int iIndex, 
		DWORD (*pfnOp)(CHexeProcess*, DWORDLONG, DWORDLONG, DWORDLONG*), 
		DWORD (*pfnFallback)(CHexeProcess*, CDatum*, DWORD*, DWORD*),
		TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_VALUE = 0x20;
	static constexpr BYTE LOCAL_LEFT = 0x28;
	static constexpr BYTE LOCAL_RESULT = 0x30;
	static constexpr BYTE LOCAL_ARRAY = 0x38;

	TArray<DWORD> FallbackJumps;

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);

	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback

	EmitMovR8FromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitMovRspDisp8FromR8(Code, LOCAL_ARRAY);

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovRDXFromStackRAXRCX8(Code);
	EmitMovRspDisp8FromRDX(Code, LOCAL_VALUE);

	EmitMovRDXFromR8Disp32(Code, LocalValueOffset(iIndex));
	EmitMovRspDisp8FromRDX(Code, LOCAL_LEFT);

	EmitMovRCXRBX(Code);
	EmitMovRDXFromRspDisp8(Code, LOCAL_LEFT);
	EmitMovR8FromRspDisp8(Code, LOCAL_VALUE);
	EmitLeaR9RspDisp8(Code, LOCAL_RESULT);
	EmitMovRAXImm64(Code, (DWORD_PTR)pfnOp);
	EmitCallRAX(Code);
	EmitJitResultHandler(Code, EpilogueJumps);

	EmitMovR8FromRspDisp8(Code, LOCAL_ARRAY);
	EmitMovRDXFromRspDisp8(Code, LOCAL_RESULT);
	EmitMovR8Disp32FromRDX(Code, LocalValueOffset(iIndex));

	EmitMovECXFromRBXDisp32(Code, OffsetProcessStackTop());
	EmitMovRAXFromRBXDisp32(Code, OffsetProcessStackData());
	EmitMovStackRAXRCX8FromRDX(Code);

	EmitSetIP(Code, pNext);
	EmitFinishNativeStep(Code, pNext, EpilogueJumps);
	DWORD dwDonePatch = EmitJmpRel32Placeholder(Code);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	EmitNativeOp(Code, pCur, pNext, pfnFallback, EpilogueJumps);

	DWORD dwDonePos = Code.GetPos();
	PatchJmpRel32(Code, dwDonePatch, dwDonePos);

	return (Code.IsOK() ? emitEmitted : emitError);
	}

static DWORD EmitMutateLocal (CX64Writer& Code, DWORD* pCur, DWORD* pNext, DWORD (*pfnOp)(CHexeProcess*, DWORDLONG, DWORDLONG, DWORDLONG*), DWORD (*pfnFallback)(CHexeProcess*, CDatum*, DWORD*, DWORD*), TArray<DWORD>& EpilogueJumps)
	{
	DWORD dwOperand = GetOperand(*pCur);
	return EmitMutateLocalValue(Code, pCur, pNext, (dwOperand >> 8), (dwOperand & 0xff), pfnOp, pfnFallback, EpilogueJumps);
	}

static void EmitLoadLocalOperandToRspSlot (CX64Writer& Code, DWORD dwOperand, BYTE bySlot, TArray<DWORD>& FallbackJumps)
	{
	int iLevel = (dwOperand >> 8);
	int iIndex = (dwOperand & 0xff);

	EmitLoadLocalEnvLevelToRAX(Code, iLevel, FallbackJumps);
	EmitCmpDwordRAXDisp32Imm32(Code, CHexeLocalEnvironment::JitOffsetArgCount(), iIndex);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x8e));	//	jle fallback
	EmitMovRAXFromRAXDisp32(Code, CHexeLocalEnvironment::JitOffsetArray());
	EmitMovRDXFromRAXDisp32(Code, LocalValueOffset(iIndex));
	EmitMovRspDisp8FromRDX(Code, bySlot);
	}

static DWORD EmitFastArrayItemAddModLoop (CX64Writer& Code, const CHexeCode* pCodeBank, DWORD* pCur, DWORD* pNext, TArray<DWORD>& EpilogueJumps)
	{
	static constexpr BYTE LOCAL_ARRAY = 0x28;
	static constexpr BYTE LOCAL_INDEX = 0x30;
	static constexpr BYTE LOCAL_DIVISOR = 0x38;
	static constexpr BYTE LOCAL_START = 0x40;

	DWORD* pBlockEnd = pCodeBank->GetCodeBlockEnd(pCur);
	if (pBlockEnd == NULL
			|| pCur + 10 > pBlockEnd
			|| pCodeBank->GetCodeOffset(pCur) < (int)(3 * sizeof(DWORD))
			|| pCodeBank->GetCodeBlockEnd(pCur - 3) != pBlockEnd)
		return EmitFastPushLocalL0(Code, pCodeBank, pCur, pNext, EpilogueJumps);

	if (GetOpCode(pCur[0]) != opPushLocalL0 || GetOperand(pCur[0]) != 0
			|| GetOpCode(pCur[1]) != opPushLocal
			|| GetOpCode(pCur[2]) != opMod || GetOperand(pCur[2]) != 2
			|| GetOpCode(pCur[3]) != opPushLocal
			|| GetOpCode(pCur[4]) != opPushLocal
			|| GetOpCode(pCur[5]) != opMutateArrayItemAdd
			|| GetOpCode(pCur[6]) != opPop || GetOperand(pCur[6]) != 1
			|| GetOpCode(pCur[7]) != opPushLocalL0 || GetOperand(pCur[7]) != 0
			|| GetOpCode(pCur[8]) != opInc || GetOperand(pCur[8]) != 1
			|| GetOpCode(pCur[9]) != opJump
			|| GetOpCode(pCur[-3]) != opPushIntShort)
		return EmitFastPushLocalL0(Code, pCodeBank, pCur, pNext, EpilogueJumps);

	DWORD* pLoopTargetIP = pCur - 4;
	if (pCur + 9 + CHexeCode::GetOperandInt(pCur[9]) != pLoopTargetIP)
		return EmitFastPushLocalL0(Code, pCodeBank, pCur, pNext, EpilogueJumps);

	int iLimit = CHexeCode::GetOperandInt(pCur[-3]);
	if (::GetEnvironmentVariableA("GW_DUMP_JIT_BLOCKS", NULL, 0) > 0)
		printf("JIT fast loop pattern emitted: limit=%d\n", iLimit);

	TArray<DWORD> FallbackJumps;

	EmitLoadLocalOperandToRspSlot(Code, GetOperand(pCur[3]), LOCAL_ARRAY, FallbackJumps);
	EmitLoadLocalOperandToRspSlot(Code, GetOperand(pCur[4]), LOCAL_INDEX, FallbackJumps);
	EmitLoadLocalOperandToRspSlot(Code, GetOperand(pCur[1]), LOCAL_DIVISOR, FallbackJumps);
	EmitLoadLocalOperandToRspSlot(Code, GetOperand(pCur[0]), LOCAL_START, FallbackJumps);

	EmitMovRCXFromRspDisp8(Code, LOCAL_ARRAY);
	EmitMovRDXFromRspDisp8(Code, LOCAL_INDEX);
	EmitMovR8FromRspDisp8(Code, LOCAL_DIVISOR);
	EmitMovR9FromRspDisp8(Code, LOCAL_START);
	EmitMovRAXImm64(Code, (DWORD_PTR)iLimit);
	EmitMovRspDisp8FromRAX(Code, 0x20);
	EmitMovRAXImm64(Code, (DWORD_PTR)&JitRunArrayItemAddModInt32Loop);
	EmitCallRAX(Code);

	EmitTestEAXEAX(Code);
	DWORD dwFastSuccessPatch = EmitJccRel32Placeholder(Code, 0x84);	//	je fast_success
	EmitCmpEAXImm32(Code, JIT_FAST_PATH_MISS);
	FallbackJumps.Insert(EmitJccRel32Placeholder(Code, 0x84));	//	je fallback

	Code.Emit8(0x2d);
	Code.Emit32(CHexeProcess::JitReturnBase);
	EpilogueJumps.Insert(EmitJmpRel32Placeholder(Code));

	DWORD dwFastSuccessPos = Code.GetPos();
	PatchJmpRel32(Code, dwFastSuccessPatch, dwFastSuccessPos);
	EmitMovRDXImm64(Code, AEON_ENCODED_INT32 | (DWORD)(iLimit + 1));
	EmitPushRDXOntoStack(Code);
	EmitSetIP(Code, pLoopTargetIP);
	EmitReturnToDispatcher(Code, EpilogueJumps);

	DWORD dwFallbackPos = Code.GetPos();
	PatchAllJumpsTo(Code, FallbackJumps, dwFallbackPos);
	return EmitFastPushLocalL0(Code, pCodeBank, pCur, pNext, EpilogueJumps);
	}

static DWORD EmitOpcode (CX64Writer& Code, const CHexeCode* pCodeBank, CHexeCodeX64* pX64, DWORD* pCur, DWORD*& pNext, DWORD* pEnd, const TArray<DWORD*>& VMIPs, const TArray<DWORD>& NativeOffsets, TArray<DWORD>& EpilogueJumps)
	{
	switch (GetOpCode(*pCur))
		{
		case opNoOp:
			return (Code.IsOK() ? emitEmitted : emitError);

		case opAdd:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpAdd, EpilogueJumps);

		case opAdd2:
			return EmitFastAdd2(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opDivide:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpDivide, EpilogueJumps);

		case opDivide2:
			return EmitFastDivide2(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opMultiply:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMultiply, EpilogueJumps);

		case opSubtract2:
			return EmitFastIntBinaryArithmetic(Code, pCodeBank, pCur, pNext, EIntBinaryOp::Subtract, EpilogueJumps);

		case opSubtract:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSubtract, EpilogueJumps);

		case opMultiply2:
			return EmitFastIntBinaryArithmetic(Code, pCodeBank, pCur, pNext, EIntBinaryOp::Multiply, EpilogueJumps);

		case opMod:
			return EmitFastMod(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opAddInt:
			return EmitFastStackTopIntImm(Code, pCodeBank, pCur, pNext, false, EpilogueJumps);

		case opSubtractInt:
			return EmitFastStackTopIntImm(Code, pCodeBank, pCur, pNext, true, EpilogueJumps);

		case opInc:
			return EmitFastStackTopIntImm(Code, pCodeBank, pCur, pNext, false, EpilogueJumps);

		case opIsEqual:
		case opIsIdentical:
			return EmitFastRawEqualBinaryComparison(Code, pCodeBank, pCur, pNext, AEON_VALUE_TRUE, EpilogueJumps);

		case opIsNotEqual:
		case opIsNotIdentical:
			return EmitFastRawEqualBinaryComparison(Code, pCodeBank, pCur, pNext, AEON_VALUE_FALSE, EpilogueJumps);

		case opIsEqualMulti:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsEqualMulti, EpilogueJumps);

		case opIsNotEqualMulti:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsNotEqualMulti, EpilogueJumps);

		case opIsLess:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsLess, EpilogueJumps);

		case opIsLessMulti:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsLessMulti, EpilogueJumps);

		case opIsGreater:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsGreater, EpilogueJumps);

		case opIsGreaterMulti:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsGreaterMulti, EpilogueJumps);

		case opIsLessOrEqual:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsLessOrEqual, EpilogueJumps);

		case opIsLessOrEqualMulti:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsLessOrEqualMulti, EpilogueJumps);

		case opIsGreaterOrEqual:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsGreaterOrEqual, EpilogueJumps);

		case opIsGreaterOrEqualMulti:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsGreaterOrEqualMulti, EpilogueJumps);

		case opIsIn:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsIn, EpilogueJumps);

		case opIsNotIn:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIsNotIn, EpilogueJumps);

		case opIsEqualInt:
			return EmitFastIntBinaryComparison(Code, pCodeBank, pCur, pNext, 0x84, EpilogueJumps);	//	je true

		case opIsNotEqualInt:
			return EmitFastIntBinaryComparison(Code, pCodeBank, pCur, pNext, 0x85, EpilogueJumps);	//	jne true

		case opIsLessInt:
			return EmitFastIntBinaryComparison(Code, pCodeBank, pCur, pNext, 0x8c, EpilogueJumps);	//	jl true

		case opIsGreaterInt:
			return EmitFastIntBinaryComparison(Code, pCodeBank, pCur, pNext, 0x8f, EpilogueJumps);	//	jg true

		case opIsLessOrEqualInt:
			return EmitFastIntBinaryComparison(Code, pCodeBank, pCur, pNext, 0x8e, EpilogueJumps);	//	jle true

		case opIsGreaterOrEqualInt:
			return EmitFastIntBinaryComparison(Code, pCodeBank, pCur, pNext, 0x8d, EpilogueJumps);	//	jge true

		case opCompareStep:
			return EmitFastCompareStep(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opCompareForEach:
			return EmitFastCompareForEach(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opAppendToArray:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpAppendToArray, EpilogueJumps);

		case opConcat:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpConcat, EpilogueJumps);

		case opCall:
			return EmitCall(Code, pCodeBank, pX64, pCur, pNext, EpilogueJumps) | emitEndsBlock;

		case opCallDirect:
			return EmitCallDirect(Code, pCodeBank, pX64, pCur, pNext, EpilogueJumps) | emitEndsBlock;

		case opCallDirectNoClosure:
			return EmitCallDirect(Code, pCodeBank, pX64, pCur, pNext, EpilogueJumps) | emitEndsBlock;

		case opCallDirectSelfNoClosure:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpCallDirectSelf, EpilogueJumps, pCodeBank, pX64) | emitEndsBlock;

		case opCallFrame:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpCallFrame, EpilogueJumps, pCodeBank, pX64) | emitEndsBlock;

		case opCallFrameSelf:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpCallFrameSelf, EpilogueJumps, pCodeBank, pX64) | emitEndsBlock;

		case opCallLib:
			return EmitCallLib(Code, pCur, pNext, EpilogueJumps) | emitEndsBlock;

		case opDebugBreak:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpDebugBreak, EpilogueJumps);

		case opDefine:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpDefine, EpilogueJumps);

		case opDefineArg:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpDefineArg, EpilogueJumps);

		case opDefineArgFromCode:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpDefineArgFromCode, EpilogueJumps);

		case opEnterEnv:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpEnterEnv, EpilogueJumps);

		case opEnterStackFrame:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpEnterStackFrame, EpilogueJumps);

		case opError:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpError, EpilogueJumps);

		case opExitEnv:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpExitEnv, EpilogueJumps);

		case opExitEnvAndReturn:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpExitEnvAndReturn, EpilogueJumps, pCodeBank, pX64) | emitEndsBlock;

		case opReturnFrame:
			return EmitFastReturnFrame(Code, pCur, pNext, EpilogueJumps, pCodeBank, pX64) | emitEndsBlock;

		case opExitEnvAndJumpIfGreaterInt:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpExitEnvAndJumpIfGreaterInt, EpilogueJumps);

		case opExitEnvAndJumpIfGreaterOrEqualInt:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpExitEnvAndJumpIfGreaterOrEqualInt, EpilogueJumps);

		case opExitEnvAndJumpIfNil:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpExitEnvAndJumpIfNil, EpilogueJumps);

		case opHexarcMsg:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpHexarcMsg, EpilogueJumps);

		case opIncForEach:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpIncForEach, EpilogueJumps);

		case opInitForEach:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpInitForEach, EpilogueJumps);

		case opIncStep:
			return EmitFastIncStep(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opNot:
			return EmitFastNot(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opNegate:
			return EmitFastNegate(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opPower:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPower, EpilogueJumps);

		case opPushIntShort:
			return EmitFastPushIntShort(Code, pCur);

		case opPushInt:
			return EmitFastPushInt(Code, pCur);

		case opPushLiteral:
			return EmitFastPushLiteral(Code, pCodeBank, pCur);

		//	GetString/GetDatum allocate values that must be rooted by the VM stack.
		//	Do not embed their encoded pointers as native constants.
		case opPushStr:
			EmitSetIP(Code, pCur);
			return EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

		case opPushDatum:
			EmitSetIP(Code, pCur);
			return EmitStep(Code, pCodeBank, pNext, EpilogueJumps);

		case opPushCoreType:
			return EmitFastPushCoreType(Code, pCur);

		case opPushType:
			if (pNext < pEnd && GetOpCode(*pNext) == opMakeObjectDirectUnchecked)
				{
				DWORD* pAfterMakeObject = g_OpCodeDb.Advance(pNext);
				if (pAfterMakeObject > pNext && pAfterMakeObject <= pEnd)
					{
					DWORD dwResult = EmitMakeObjectDirectUncheckedKnownType(Code, pCur, pNext, pAfterMakeObject, EpilogueJumps);
					if (!HasEmitFlag(dwResult, emitError))
						pNext = pAfterMakeObject;

					return dwResult;
					}
				}

			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushType, EpilogueJumps);

		case opPushNil:
			return EmitFastPushEncodedDatum(Code, AEON_VALUE_NULL);

		case opPushNaN:
			return EmitFastPushEncodedDatum(Code, AEON_VALUE_NAN);

		case opPushStrNull:
			return EmitFastPushEncodedDatum(Code, AEON_VALUE_BLANK);

		case opPushTrue:
			return EmitFastPushEncodedDatum(Code, AEON_VALUE_TRUE);

		case opPushFalse:
			return EmitFastPushEncodedDatum(Code, AEON_VALUE_FALSE);

		case opPushFrameArg:
			return EmitFastPushFrameArg(Code, pCur, pNext, EpilogueJumps);

		case opPushLocal:
			return EmitFastPushLocal(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opPushLocalL0:
			if (GetOperand(*pCur) == 0)
				return EmitFastArrayItemAddModLoop(Code, pCodeBank, pCur, pNext, EpilogueJumps);

			return EmitFastPushLocalL0(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opPushArrayItem:
			return EmitPushArrayItem(Code, pNext, EpilogueJumps);

		case opPushArrayItemI:
			return EmitPushArrayItemI(Code, pCur, pNext, EpilogueJumps);

		case opPushRecordSlotI:
			return EmitPushRecordSlotI(Code, pCur, pNext, EpilogueJumps);

		case opPushGlobal:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushGlobal, EpilogueJumps);

		case opPushInitForEach:
			return EmitPushInitForEach(Code, pNext, EpilogueJumps);

		case opPushLocalItem:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushLocalItem, EpilogueJumps);

		case opPushLocalLength:
			return EmitPushLocalLength(Code, pCur, pNext, EpilogueJumps);

		case opPushObjectItem:
			return EmitPushObjectItem(Code, pCur, pNext, EpilogueJumps);

		case opPushObjectMethod:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushObjectMethod, EpilogueJumps);

		case opPushTensorItem:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushTensorItem, EpilogueJumps);

		case opPushTensorItemI:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpPushTensorItemI, EpilogueJumps);

		case opSetLocalL0:
			return EmitFastSetLocalL0(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opSetLocal:
			return EmitFastSetLocal(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opSetObjectItem2:
			return EmitSetObjectItem2(Code, pCodeBank, pCur, pNext, EpilogueJumps);

		case opSetArrayItem:
			return EmitSetArrayItem(Code, pCur, pNext, EpilogueJumps);

		case opSetArrayItemI:
			return EmitSetArrayItemI(Code, pCur, pNext, EpilogueJumps);

		//	This helper returns multiple CDatum values across native calls; keep it
		//	on the rooted VM path until JIT spill slots are explicit GC roots.
		case opSetForEachItem:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetForEachItem, EpilogueJumps);

		case opSetFrameLocal:
			return EmitFastSetFrameLocal(Code, pCur, pNext, EpilogueJumps);

		case opSetGlobal:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetGlobal, EpilogueJumps);

		case opSetGlobalItem:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetGlobalItem, EpilogueJumps);

		case opSetLocalItem:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetLocalItem, EpilogueJumps);

		case opSetObjectItem:
			return EmitSetObjectItem(Code, pCur, pNext, EpilogueJumps);

		case opSetTensorItem:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetTensorItem, EpilogueJumps);

		case opSetTensorItemI:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpSetTensorItemI, EpilogueJumps);

		case opMakeArray:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeArray, EpilogueJumps);

		case opMakeApplyEnv:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeApplyEnv, EpilogueJumps);

		case opMakeAsType:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeAsType, EpilogueJumps);

		case opMakeAsTypeCons:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeAsTypeCons, EpilogueJumps);

		case opMakeBlockEnv:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeBlockEnv, EpilogueJumps);

		case opMakeDatatype:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeDatatype, EpilogueJumps);

		case opMakeEmptyArray:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeEmptyArray, EpilogueJumps);

		case opMakeEmptyArrayAsType:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeEmptyArrayAsType, EpilogueJumps);

		case opMakeEmptyStruct:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeEmptyStruct, EpilogueJumps);

		case opMakeEnv:
			if (pNext < pEnd && (GetOpCode(*pNext) == opCallDirect || GetOpCode(*pNext) == opCallDirectNoClosure || GetOpCode(*pNext) == opCallDirectSelfNoClosure))
				{
				DWORD* pAfterCallDirect = g_OpCodeDb.Advance(pNext);
				if (pAfterCallDirect > pNext && pAfterCallDirect <= pEnd)
					return EmitMakeEnvDirectCall(Code, pCodeBank, pX64, pCur, pNext, pAfterCallDirect, (GetOpCode(*pNext) == opCallDirectNoClosure || GetOpCode(*pNext) == opCallDirectSelfNoClosure), (GetOpCode(*pNext) == opCallDirectSelfNoClosure), EpilogueJumps) | emitEndsBlock;
				}

			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeEnv, EpilogueJumps);

		case opMakeExpr:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeExpr, EpilogueJumps);

		case opMakeExprIf:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeExprIf, EpilogueJumps);

		case opMakeFlagsFromArray:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeFlagsFromArray, EpilogueJumps);

		case opMakeFunc:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeFunc, EpilogueJumps);

		case opMakeFunc2:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeFunc2, EpilogueJumps);

		case opMakeLocalEnv:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeLocalEnv, EpilogueJumps);

		case opMakeMapColExpr:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeMapColExpr, EpilogueJumps);

		case opMakeMethodEnv:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeMethodEnv, EpilogueJumps);

		case opMakeRange:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeRange, EpilogueJumps);

		case opMakeObject:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeObject, EpilogueJumps);

		case opMakeObjectDirect:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeObjectDirect, EpilogueJumps);

		case opMakeObjectDirectUnchecked:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeObjectDirectUnchecked, EpilogueJumps);

		case opMakePrimitive:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakePrimitive, EpilogueJumps);

		case opMakeSpread:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeSpread, EpilogueJumps);

		case opMakeStruct:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeStruct, EpilogueJumps);

		case opMakeTensor:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeTensor, EpilogueJumps);

		case opMakeTensorType:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMakeTensorType, EpilogueJumps);

		case opMapResult:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMapResult, EpilogueJumps);

		case opNewObject:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpNewObject, EpilogueJumps);

		case opPopLocalL0:
			return EmitFastPopLocalValue(Code, pCodeBank, pCur, pNext, 0, GetOperand(*pCur), EpilogueJumps);

		case opPopLocal:
			{
			DWORD dwOperand = GetOperand(*pCur);
			return EmitFastPopLocalValue(Code, pCodeBank, pCur, pNext, (dwOperand >> 8), (dwOperand & 0xff), EpilogueJumps);
			}

		case opPop:
			return EmitFastPop(Code, pCur, pNext, EpilogueJumps);

		case opPopDeep:
			return EmitFastPopDeep(Code, pCur);

		case opJump:
			{
			DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);
			DWORD dwTargetNativeOffset = 0;
			bool bHasTargetNativeOffset = FindNativeOffset(VMIPs, NativeOffsets, pTargetIP, &dwTargetNativeOffset);
			DWORD dwComputeCount = (bHasTargetNativeOffset ? CalcLoopComputeCount(VMIPs, pTargetIP) : 1);
			return EmitFastJump(Code, pCur, dwTargetNativeOffset, bHasTargetNativeOffset, dwComputeCount, EpilogueJumps) | emitEndsBlock;
			}

		case opJumpIfNil:
			{
			DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);
			DWORD dwTargetNativeOffset = 0;
			bool bHasTargetNativeOffset = FindNativeOffset(VMIPs, NativeOffsets, pTargetIP, &dwTargetNativeOffset);
			DWORD dwComputeCount = (bHasTargetNativeOffset ? CalcLoopComputeCount(VMIPs, pTargetIP) : 1);
			return EmitFastJumpIfNil(Code, pCodeBank, pCur, pNext, dwTargetNativeOffset, bHasTargetNativeOffset, dwComputeCount, EpilogueJumps) | emitEndsBlock;
			}

		case opJumpIfNilNoPop:
			{
			DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);
			DWORD dwTargetNativeOffset = 0;
			bool bHasTargetNativeOffset = FindNativeOffset(VMIPs, NativeOffsets, pTargetIP, &dwTargetNativeOffset);
			DWORD dwComputeCount = (bHasTargetNativeOffset ? CalcLoopComputeCount(VMIPs, pTargetIP) : 1);
			return EmitFastJumpIfNilNoPop(Code, pCodeBank, pCur, pNext, dwTargetNativeOffset, bHasTargetNativeOffset, dwComputeCount, EpilogueJumps) | emitEndsBlock;
			}

		case opJumpIfNotNilNoPop:
			{
			DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);
			DWORD dwTargetNativeOffset = 0;
			bool bHasTargetNativeOffset = FindNativeOffset(VMIPs, NativeOffsets, pTargetIP, &dwTargetNativeOffset);
			DWORD dwComputeCount = (bHasTargetNativeOffset ? CalcLoopComputeCount(VMIPs, pTargetIP) : 1);
			return EmitFastJumpIfNotNilNoPop(Code, pCodeBank, pCur, pNext, dwTargetNativeOffset, bHasTargetNativeOffset, dwComputeCount, EpilogueJumps) | emitEndsBlock;
			}

		case opExitEnvAndJumpIfLocalGreaterInt:
			return EmitFastExitEnvAndJumpIfLocalGreaterInt(Code, pCur, pNext, EpilogueJumps);

		case opLoopIncLocalAndJump:
			{
			DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);
			DWORD dwLoopTargetNativeOffset = 0;
			bool bHasLoopTargetNativeOffset = FindNativeOffset(VMIPs, NativeOffsets, pTargetIP, &dwLoopTargetNativeOffset);
			DWORD dwLoopComputeCount = (bHasLoopTargetNativeOffset ? CalcLoopComputeCount(VMIPs, pTargetIP) : 1);

			return EmitFastLoopIncLocalAndJump(Code, pCur, pNext, dwLoopTargetNativeOffset, bHasLoopTargetNativeOffset, dwLoopComputeCount, false, EpilogueJumps) | emitEndsBlock;
			}

		case opLoopIncAndJump:
			{
			DWORD* pTargetIP = pCur + CHexeCode::GetOperandInt(*pCur);
			DWORD dwLoopTargetNativeOffset = 0;
			bool bHasLoopTargetNativeOffset = FindNativeOffset(VMIPs, NativeOffsets, pTargetIP, &dwLoopTargetNativeOffset);
			DWORD dwLoopComputeCount = (bHasLoopTargetNativeOffset ? CalcLoopComputeCount(VMIPs, pTargetIP) : 1);

			return EmitFastLoopIncLocalAndJump(Code, pCur, pNext, dwLoopTargetNativeOffset, bHasLoopTargetNativeOffset, dwLoopComputeCount, true, EpilogueJumps) | emitEndsBlock;
			}

		case opIncLocalInt:
			return EmitFastIncLocalInt(Code, pCur, pNext, EpilogueJumps);

		case opIncLocalL0:
			return EmitFastAddLocalInt(Code, pCodeBank, pCur, pNext, 0, GetOperand(*pCur), 1, EpilogueJumps);

		case opAddLocalL0Int16:
			{
			DWORD dwOperand = GetOperand(*pCur);
			int iIndex = (dwOperand >> 16) & 0xff;
			int iInc = ((dwOperand & 0x8000) ? (int)(dwOperand | 0xffff0000) : (int)(dwOperand & 0xffff));
			return EmitFastAddLocalInt(Code, pCodeBank, pCur, pNext, 0, iIndex, iInc, EpilogueJumps);
			}

		case opMutateArrayItemAdd:
			return EmitMutateArrayItemAdd(Code, pNext, EpilogueJumps);

		case opMutateArrayItemSubtract:
			return EmitMutateArrayItem(Code, pNext, &JitOpSubtractDatum, EpilogueJumps);

		case opMutateArrayItemMultiply:
			return EmitMutateArrayItem(Code, pNext, &JitOpMultiplyDatum, EpilogueJumps);

		case opMutateArrayItemDivide:
			return EmitMutateArrayItem(Code, pNext, &JitOpDivideDatum, EpilogueJumps);

		case opMutateArrayItemConcat:
			return EmitMutateArrayItem(Code, pNext, &JitOpConcatDatum, EpilogueJumps);

		case opMutateArrayItemMod:
			return EmitMutateArrayItem(Code, pNext, &JitOpModDatum, EpilogueJumps);

		case opMutateArrayItemPower:
			return EmitMutateArrayItem(Code, pNext, &JitOpPowerDatum, EpilogueJumps);

		case opMutateObjectItemAdd:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateObjectItemAdd, EpilogueJumps);

		case opMutateObjectItemSubtract:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateObjectItemSubtract, EpilogueJumps);

		case opMutateObjectItemMultiply:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateObjectItemMultiply, EpilogueJumps);

		case opMutateObjectItemDivide:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateObjectItemDivide, EpilogueJumps);

		case opMutateObjectItemConcat:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateObjectItemConcat, EpilogueJumps);

		case opMutateObjectItemMod:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateObjectItemMod, EpilogueJumps);

		case opMutateObjectItemPower:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateObjectItemPower, EpilogueJumps);

		case opMutateGlobalAdd:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateGlobalAdd, EpilogueJumps);

		case opMutateGlobalSubtract:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateGlobalSubtract, EpilogueJumps);

		case opMutateGlobalMultiply:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateGlobalMultiply, EpilogueJumps);

		case opMutateGlobalDivide:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateGlobalDivide, EpilogueJumps);

		case opMutateGlobalConcat:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateGlobalConcat, EpilogueJumps);

		case opMutateGlobalMod:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateGlobalMod, EpilogueJumps);

		case opMutateGlobalPower:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateGlobalPower, EpilogueJumps);

		case opMutateLocalAdd:
			return EmitMutateLocal(Code, pCur, pNext, &JitOpAddDatum, &CHexeProcess::JitOpMutateLocalAdd, EpilogueJumps);

		case opMutateLocalSubtract:
			return EmitMutateLocal(Code, pCur, pNext, &JitOpSubtractDatum, &CHexeProcess::JitOpMutateLocalSubtract, EpilogueJumps);

		case opMutateLocalMultiply:
			return EmitMutateLocal(Code, pCur, pNext, &JitOpMultiplyDatum, &CHexeProcess::JitOpMutateLocalMultiply, EpilogueJumps);

		case opMutateLocalDivide:
			return EmitMutateLocal(Code, pCur, pNext, &JitOpDivideDatum, &CHexeProcess::JitOpMutateLocalDivide, EpilogueJumps);

		case opMutateLocalConcat:
			return EmitMutateLocal(Code, pCur, pNext, &JitOpConcatDatum, &CHexeProcess::JitOpMutateLocalConcat, EpilogueJumps);

		case opMutateLocalMod:
			return EmitMutateLocal(Code, pCur, pNext, &JitOpModDatum, &CHexeProcess::JitOpMutateLocalMod, EpilogueJumps);

		case opMutateLocalPower:
			return EmitMutateLocal(Code, pCur, pNext, &JitOpPowerDatum, &CHexeProcess::JitOpMutateLocalPower, EpilogueJumps);

		case opMutateTensorItemAdd:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateTensorItemAdd, EpilogueJumps);

		case opMutateTensorItemSubtract:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateTensorItemSubtract, EpilogueJumps);

		case opMutateTensorItemMultiply:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateTensorItemMultiply, EpilogueJumps);

		case opMutateTensorItemDivide:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateTensorItemDivide, EpilogueJumps);

		case opMutateTensorItemConcat:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateTensorItemConcat, EpilogueJumps);

		case opMutateTensorItemMod:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateTensorItemMod, EpilogueJumps);

		case opMutateTensorItemPower:
			return EmitNativeOp(Code, pCur, pNext, &CHexeProcess::JitOpMutateTensorItemPower, EpilogueJumps);

		default:
			EmitSetIP(Code, pCur);
			return EmitStep(Code, pCodeBank, pNext, EpilogueJumps);
		}
	}

CHexeCodeX64::~CHexeCodeX64 ()
	{
	PrintBlockStats();
	DeleteAll();
	}

bool CHexeCodeX64::CompileBlock (const CHexeCode& CodeBank, int iVMOffset)
	{
#ifndef _M_X64
	return false;
#else
	static bool bDumpJITBlocks = (::GetEnvironmentVariableA("GW_DUMP_JIT_BLOCKS", NULL, 0) > 0);

	if (FindBlock(iVMOffset))
		return true;

	DWORD* pIP = CodeBank.GetCode(iVMOffset);
	DWORD* pEnd = CodeBank.GetCodeBlockEnd(pIP);
	if (pEnd == NULL || pIP >= pEnd)
		return false;

	DWORD dwAllocSize = 64 + (MAX_OPS_PER_BLOCK * ESTIMATED_BYTES_PER_OP);
	BYTE* pMemory = (BYTE *)::VirtualAlloc(NULL, dwAllocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (pMemory == NULL)
		return false;

	CX64Writer Writer(pMemory, dwAllocSize);
	TArray<DWORD> EpilogueJumps;
	TArray<DWORD*> VMIPs;
	TArray<DWORD> NativeOffsets;

	EmitPrologue(Writer);

	DWORD* pCur = pIP;
	int iOps = 0;
	while (pCur < pEnd && iOps < MAX_OPS_PER_BLOCK)
		{
		DWORD* pNext = g_OpCodeDb.Advance(pCur);
		if (pNext <= pCur || pNext > pEnd)
			break;

		VMIPs.Insert(pCur);
		NativeOffsets.Insert(Writer.GetPos());

		if (bDumpJITBlocks)
			{
			SOpCodeInfo* pInfo = g_OpCodeDb.GetInfo(*pCur);
			printf("JIT block %d vm+%04d: %s operand=%d\n", iVMOffset, CodeBank.GetCodeOffset(pCur), (pInfo ? (LPCSTR)pInfo->sOpCode : "?"), (int)GetOperand(*pCur));
			}

		DWORD dwEmitFlags = EmitOpcode(Writer, &CodeBank, this, pCur, pNext, pEnd, VMIPs, NativeOffsets, EpilogueJumps);
		if (HasEmitFlag(dwEmitFlags, emitUnsupported))
			dwEmitFlags = EmitStep(Writer, &CodeBank, pNext, EpilogueJumps);

		if (HasEmitFlag(dwEmitFlags, emitError) || !HasEmitFlag(dwEmitFlags, emitEmitted))
			break;

		pCur = pNext;
		iOps++;

		if (HasEmitFlag(dwEmitFlags, emitNeedsStopCheck) || HasEmitFlag(dwEmitFlags, emitEndsBlock))
			break;
		}

	if (iOps == 0 || !Writer.IsOK())
		{
		::VirtualFree(pMemory, 0, MEM_RELEASE);
		return false;
		}

	//	Fall through to the dispatcher when the block reaches its compiled end.

	EmitSetIP(Writer, pCur);
	EmitAddQwordRBXDisp32Imm8(Writer, OffsetProcessComputes(), (BYTE)iOps);
	EmitNativeStopCheckIfNeeded(Writer, pCur, EpilogueJumps);

	EmitChainToCurrentBlock(Writer, &CodeBank, this, EpilogueJumps);

	DWORD dwEpiloguePos = Writer.GetPos();
	EmitEpilogue(Writer);

	if (!Writer.IsOK())
		{
		::VirtualFree(pMemory, 0, MEM_RELEASE);
		return false;
		}

	for (int i = 0; i < EpilogueJumps.GetCount(); i++)
		PatchJmpRel32(Writer, EpilogueJumps[i], dwEpiloguePos);

	DWORD dwOldProtect = 0;
	if (!::VirtualProtect(pMemory, dwAllocSize, PAGE_EXECUTE_READ, &dwOldProtect))
		{
		::VirtualFree(pMemory, 0, MEM_RELEASE);
		return false;
		}

	::FlushInstructionCache(::GetCurrentProcess(), pMemory, Writer.GetPos());

	SX64Block* pBlock = new SX64Block;
	pBlock->iStartVMOffset = iVMOffset;
	pBlock->iEndVMOffset = CodeBank.GetCodeOffset(pCur);
	pBlock->pNativeEntry = pMemory;
	pBlock->dwNativeSize = dwAllocSize;

	m_Blocks.Insert(pBlock);
	m_ByVMOffset.SetAt(iVMOffset, pBlock);

	return true;
#endif
	}

void CHexeCodeX64::DeleteAll ()
	{
	for (int i = 0; i < m_Blocks.GetCount(); i++)
		{
		SX64Block* pBlock = m_Blocks[i];
		if (pBlock)
			{
			if (pBlock->pNativeEntry)
				::VirtualFree(pBlock->pNativeEntry, 0, MEM_RELEASE);

			delete pBlock;
			}
		}

	m_Blocks.DeleteAll();
	m_ByVMOffset.DeleteAll();
	for (int i = 0; i < FIND_BLOCK_CACHE_SIZE; i++)
		m_FindBlockCache[i] = SFindBlockCacheEntry();
	}

CHexeCodeX64::SX64Block* CHexeCodeX64::FindBlock (int iVMOffset) const
	{
	SFindBlockCacheEntry& Cache = m_FindBlockCache[(iVMOffset >> 2) & (FIND_BLOCK_CACHE_SIZE - 1)];
	if (Cache.iVMOffset == iVMOffset && Cache.pBlock)
		{
		RecordFindBlock(iVMOffset, true);
		if (CollectJitBlockStats())
			m_dwFindBlockCacheHits++;
		return Cache.pBlock;
		}

	SX64Block** ppBlock = m_ByVMOffset.GetAt(iVMOffset);
	RecordFindBlock(iVMOffset, (ppBlock != NULL));
	if (ppBlock)
		{
		Cache.iVMOffset = iVMOffset;
		Cache.pBlock = *ppBlock;
		}

	return (ppBlock ? *ppBlock : NULL);
	}

void CHexeCodeX64::PrintBlockStats () const
	{
	if (!CollectJitBlockStats() || m_bPrintedBlockStats || m_dwFindBlockCalls == 0)
		return;

	m_bPrintedBlockStats = true;

	printf("JIT block stats: blocks=%d findBlock=%llu cacheHits=%llu mapHits=%llu misses=%llu\n",
			m_Blocks.GetCount(),
			m_dwFindBlockCalls,
			m_dwFindBlockCacheHits,
			m_dwFindBlockHits - m_dwFindBlockCacheHits,
			m_dwFindBlockCalls - m_dwFindBlockHits);

	for (int iRank = 0; iRank < 10; iRank++)
		{
		int iBest = -1;
		DWORDLONG dwBest = 0;

		for (int i = 0; i < m_FindBlockOffsets.GetCount(); i++)
			{
			DWORDLONG dwCount = m_FindBlockOffsets.GetValue(i);
			if (dwCount > dwBest)
				{
				dwBest = dwCount;
				iBest = i;
				}
			}

		if (iBest == -1 || dwBest == 0)
			break;

		printf("  vm+%04d: %llu lookups\n", m_FindBlockOffsets.GetKey(iBest), dwBest);
		m_FindBlockOffsets.GetValue(iBest) = 0;
		}
	}

void CHexeCodeX64::RecordFindBlock (int iVMOffset, bool bFound) const
	{
	if (!CollectJitBlockStats())
		return;

	m_dwFindBlockCalls++;
	if (bFound)
		m_dwFindBlockHits++;

	DWORDLONG* pCount = m_FindBlockOffsets.GetAt(iVMOffset);
	if (pCount)
		(*pCount)++;
	else
		m_FindBlockOffsets.SetAt(iVMOffset, (DWORDLONG)1);
	}

CHexeCodeX64::X64Entry CHexeCodeX64::FindOrCompileEntry (const CHexeCode& CodeBank, int iVMOffset)
	{
	CSmartLock Lock(m_cs);

	SX64Block* pBlock = FindBlock(iVMOffset);
	if (pBlock == NULL)
		{
		if (!CompileBlock(CodeBank, iVMOffset))
			return NULL;

		pBlock = FindBlock(iVMOffset);
		}

	return (pBlock ? (X64Entry)pBlock->pNativeEntry : NULL);
	}
