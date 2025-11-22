//	OpsImpl.h
//
//	Defines operations
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

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

