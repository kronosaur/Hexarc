//	OpsImpl.h
//
//	Defines operations
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once
class COpCompHelpers
	{
	public:

		static bool IsIntFloatPair (DWORD dwLeftType, DWORD dwRightType)
			{
			return (IsIntegerType(dwLeftType) && dwRightType == IDatatype::FLOAT_64)
				|| (dwLeftType == IDatatype::FLOAT_64 && IsIntegerType(dwRightType));
			}

		static bool TryCompareIntFloat (CDatum dLeft, CDatum dRight, int& retCompare)
			{
			DWORD dwLeftType = dLeft.GetBasicDatatype();
			DWORD dwRightType = dRight.GetBasicDatatype();
			if (!IsIntFloatPair(dwLeftType, dwRightType))
				return false;

			double rValue = 0.0;
			CIPInteger iValue;
			const bool bFloatOnLeft = (dwLeftType == IDatatype::FLOAT_64);
			if (bFloatOnLeft)
				{
				rValue = (double)dLeft;
				iValue = IntegerFromDatum(dRight);
				}
			else
				{
				rValue = (double)dRight;
				iValue = IntegerFromDatum(dLeft);
				}

			if (!std::isfinite(rValue))
				return false;

			CIPInteger iFromFloat;
			if (TryGetExactIntegerFromDouble(rValue, iFromFloat))
				{
				const int iCompare = iFromFloat.Compare(iValue);
				retCompare = (bFloatOnLeft ? iCompare : -iCompare);
				return true;
				}

			CIPInteger iFloor;
			if (!TryGetExactIntegerFromDouble(std::floor(rValue), iFloor))
				return false;

			const int iCompare = (iFloor.Compare(iValue) >= 0 ? 1 : -1);
			retCompare = (bFloatOnLeft ? iCompare : -iCompare);
			return true;
			}


	private:

		union SDoubleBits
			{
			double rValue;
			DWORDLONG dwBits;
			};

		static bool IsIntegerType (DWORD dwType)
			{
			return (dwType == IDatatype::INT_32
					|| dwType == IDatatype::INT_64
					|| dwType == IDatatype::INT_IP);
			}

		static CIPInteger IntegerFromDatum (CDatum dValue)
			{
			switch (dValue.GetBasicDatatype())
				{
				case IDatatype::INT_32:
					return CIPInteger((int)dValue);

				case IDatatype::INT_64:
					return CIPInteger((DWORDLONG)dValue);

				case IDatatype::INT_IP:
					return (const CIPInteger&)dValue;

				default:
					return CIPInteger(0);
				}
			}

		static bool TryGetExactIntegerFromDouble (double rValue, CIPInteger& retValue)
			{
			if (!std::isfinite(rValue))
				return false;
			else if (rValue == 0.0)
				{
				retValue = CIPInteger(0);
				return true;
				}

			SDoubleBits Bits;
			Bits.rValue = rValue;

			const DWORDLONG dwExponentBits = ((Bits.dwBits >> 52) & 0x7ff);
			if (dwExponentBits == 0x7ff)
				return false;
			else if (dwExponentBits == 0)
				return false;

			const bool bNegative = ((Bits.dwBits >> 63) != 0);
			const int iExponent = (int)dwExponentBits - 1023;
			const DWORDLONG dwSignificand = ((DWORDLONG)1 << 52) | (Bits.dwBits & ((((DWORDLONG)1) << 52) - 1));

			CIPInteger Result;
			if (iExponent < 0)
				return false;
			else if (iExponent >= 52)
				{
				Result = CIPInteger(dwSignificand);
				Result <<= (size_t)(iExponent - 52);
				}
			else
				{
				const int iShift = 52 - iExponent;
				const DWORDLONG dwMask = ((((DWORDLONG)1) << iShift) - 1);
				if ((dwSignificand & dwMask) != 0)
					return false;

				Result = CIPInteger(dwSignificand >> iShift);
				}

			if (bNegative && !Result.IsZero())
				Result.SetNegative(true);

			retValue = Result;
			return true;
			}
	};

class COpAdd
	{
	public:

		static CAEONOperatorTable CreateTable ();
		static CDatum CalcType (CDatum dLeftType, CDatum dRightType);

	private:

		static constexpr int LARGE_STRING_THRESHOLD = 1024;

		static CDatum ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecAny_String (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDateTime_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecError (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecString (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecString_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan_DateTime (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector2D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector3D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);

		static CDatum ExecConcatString (const CString& sLeft, const CString& sRight, IAEONOperatorCtx& Ctx);
	};

class COpCompEqual
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(dLeft.OpIsEqual(dRight)); }
		static CDatum ExecDouble (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt32 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt64 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecIntIP (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecNumber_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpCompGreaterThan
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(dLeft.OpCompare(dRight) > 0); }
		static CDatum ExecDouble (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt32 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt64 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecIntIP (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecNumber_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpCompGreaterThanOrEqual
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(dLeft.OpCompare(dRight) >= 0); }
		static CDatum ExecDouble (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt32 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt64 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecIntIP (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecNumber_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpCompIdentical
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(dLeft.OpIsIdentical(dRight)); }
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpCompLessThan
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(dLeft.OpCompare(dRight) < 0); }
		static CDatum ExecDouble (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt32 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt64 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecIntIP (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecNumber_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpCompLessThanOrEqual
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(dLeft.OpCompare(dRight) <= 0); }
		static CDatum ExecDouble (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt32 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt64 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecIntIP (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecNumber_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpCompNotEqual
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(!dLeft.OpIsEqual(dRight)); }
		static CDatum ExecDouble (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt32 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecInt64 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecIntIP (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecNumber_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpCompNotIdentical
	{
	public:

		static CAEONOperatorTableNew CreateTable ();

	private:

		static CDatum ExecAny_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return CDatum(!dLeft.OpIsIdentical(dRight)); }
		static CDatum ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
	};

class COpConcatenate
	{
	public:

		static CAEONOperatorTableNew CreateTable ();
		static CDatum CalcType (CDatum dLeftType, CDatum dRightType);

	private:

		static CDatum ExecAny_Null (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecArray_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecArray_Object (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecArray_Scalar (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecClassInstance_ClassInstance (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecError (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecNull_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecObject_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecObject_Object (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecObject_String (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecScalar_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecScalar_Scalar (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecString_Object (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecStruct_Struct (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecStruct_Table (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecTable_Struct (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum ExecTable_Table (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);

		static CDatum CalcDatatypeArray_Scalar (CDatum dArrayType, CDatum dScalarType);
	};

class COpDivide
	{
	public:

		static CAEONOperatorTable CreateTable ();
		static CDatum CalcType (CDatum dLeftType, CDatum dRightType);

	private:

		static CDatum ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecNumber_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector2D_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector3D_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);

		static CDatum ExecDivide_Double (double rLeft, double rRight) {	return (rRight == 0.0 ? CDatum::CreateNaN() : CDatum(rLeft / rRight)); }
		static CDatum ExecDivide_Int32 (int iLeft, int iRight);
		static CDatum ExecDivide_Int64 (DWORDLONG dwLeft, DWORDLONG dwRight);
		static CDatum ExecDivide_IntIP (const CIPInteger& Left, const CIPInteger& Right);
	};

class COpMod
	{
	public:

		static CAEONOperatorTable CreateTable ();
		static CDatum CalcType (CDatum dLeftType, CDatum dRightType);

	private:

		static CDatum ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecNumber_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);

		static CDatum ExecMod_Double (double rLeft, double rRight);
		static CDatum ExecMod_Int32 (int iLeft, int iRight);
		static CDatum ExecMod_Int64 (DWORDLONG dwLeft, DWORDLONG dwRight);
		static CDatum ExecMod_IntIP (const CIPInteger& Left, const CIPInteger& Right);
	};

class COpMultiply
	{
	public:

		static CAEONOperatorTable CreateTable ();
		static CDatum CalcType (CDatum dLeftType, CDatum dRightType);

	private:

		static CDatum ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecError (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecNumber_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecNumber_Vector2D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector2D_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecNumber_Vector3D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector3D_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
	};

class COpNegate
	{
	public:

		static CAEONUnaryOpTable CreateTable ();
		static CDatum CalcType (CDatum dType);

	private:

		static CDatum ExecArray (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32 (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64 (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector2D (CDatum dValue, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector3D (CDatum dValue, IAEONOperatorCtx& Ctx);
	};

class COpPower
	{
	public:

		static CAEONOperatorTable CreateTable ();
		static CDatum CalcType (CDatum dLeftType, CDatum dRightType);

	private:

		static CDatum ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);

		static CDatum ExecPower_Int32 (int iLeft, int iRight);

		static constexpr int MAX_EXP_FOR_INT32 = 30;
		static const int MAX_BASE_FOR_EXP[MAX_EXP_FOR_INT32 + 1];
	};

class COpSubtract
	{
	public:

		static CAEONOperatorTable CreateTable ();
		static CDatum CalcType (CDatum dLeftType, CDatum dRightType);

	private:

		static CDatum ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDateTime_DateTime (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDateTime_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecError (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecNull_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecTimeSpan_DateTime (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector2D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
		static CDatum ExecVector3D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);
	};

