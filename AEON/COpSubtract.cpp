//	COpSubtract.cpp
//
//	COpSubtract class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_OP_NOT_DEFINED,				"Operator undefined for %s - %s.");

CAEONOperatorTable COpSubtract::CreateTable ()

//	CreateTable
//
//	Returns a table of operator implementation functions.

	{
	//	By default for anything we don't handle we get nan

	CAEONOperatorTable Table(CAEONOp::ExecNaN);

	//	Array

	Table.SetOp(IDatatype::ARRAY, IDatatype::ARRAY, ExecArray_Array);
	Table.SetOp(IDatatype::ARRAY, IDatatype::BOOL, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::DATE_TIME, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::ENUM, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::FLOAT_64, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::INT_32, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::INT_64, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::INT_IP, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::NAN_CONST, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::STRING, ExecArray_Any);
	Table.SetOp(IDatatype::ARRAY, IDatatype::TIME_SPAN, ExecArray_Any);

	//	Bool

	Table.SetOp(IDatatype::BOOL, IDatatype::ARRAY, ExecAny_Array);

	//	DateTime

	Table.SetOp(IDatatype::DATE_TIME, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::DATE_TIME, IDatatype::DATE_TIME, ExecDateTime_DateTime);
	Table.SetOp(IDatatype::DATE_TIME, IDatatype::TIME_SPAN, ExecDateTime_TimeSpan);

	//	Double

	Table.SetOp(IDatatype::FLOAT_64, IDatatype::FLOAT_64, ExecDouble);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::ENUM, ExecDouble_Int32);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_32, ExecDouble_Int32);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_64, ExecDouble_Int64);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_IP, ExecDouble_IntIP);

	//	Enum

	Table.SetOp(IDatatype::ENUM, IDatatype::ENUM, ExecInt32);		//	Enum is stored just like Int32
	Table.SetOp(IDatatype::ENUM, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::ENUM, IDatatype::FLOAT_64, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_32, ExecInt32);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_64, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_IP, ExecInt32_Double);

	//	Expressions

	Table.SetOpLeft(IDatatype::EXPRESSION, ExecExpression_Any);
	Table.SetOpRight(IDatatype::EXPRESSION, ExecAny_Expression);
	Table.SetOp(IDatatype::EXPRESSION, IDatatype::EXPRESSION, ExecExpression_Expression);

	//	Int32

	Table.SetOp(IDatatype::INT_32, IDatatype::INT_32, ExecInt32);
	Table.SetOp(IDatatype::INT_32, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::INT_32, IDatatype::ENUM, ExecInt32);
	Table.SetOp(IDatatype::INT_32, IDatatype::FLOAT_64, ExecInt32_Double);
	Table.SetOp(IDatatype::INT_32, IDatatype::INT_64, ExecInt32_Int64);
	Table.SetOp(IDatatype::INT_32, IDatatype::INT_IP, ExecInt32_IntIP);

	//	Int64

	Table.SetOp(IDatatype::INT_64, IDatatype::INT_64, ExecInt64);
	Table.SetOp(IDatatype::INT_64, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::INT_64, IDatatype::ENUM, ExecInt64_Int32);
	Table.SetOp(IDatatype::INT_64, IDatatype::FLOAT_64, ExecInt64_Double);
	Table.SetOp(IDatatype::INT_64, IDatatype::INT_32, ExecInt64_Int32);
	Table.SetOp(IDatatype::INT_64, IDatatype::INT_IP, ExecInt64_IntIP);

	//	IntIP

	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_IP, ExecIntIP);
	Table.SetOp(IDatatype::INT_IP, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::INT_IP, IDatatype::ENUM, ExecIntIP_Int32);
	Table.SetOp(IDatatype::INT_IP, IDatatype::FLOAT_64, ExecIntIP_Double);
	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_32, ExecIntIP_Int32);
	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_64, ExecIntIP_Int64);

	//	NaN

	Table.SetOp(IDatatype::NAN_CONST, IDatatype::ARRAY, ExecAny_Array);

	//	Null

	Table.SetOpLeft(IDatatype::NULL_T, ExecNull_Any);
	Table.SetOp(IDatatype::NULL_T, IDatatype::ARRAY, ExecArray_Array);
	Table.SetOp(IDatatype::NULL_T, IDatatype::NULL_T, CAEONOp::ExecLeft);
	Table.SetOp(IDatatype::ARRAY, IDatatype::NULL_T, CAEONOp::ExecLeft);
	Table.SetOp(IDatatype::DATE_TIME, IDatatype::NULL_T, CAEONOp::ExecLeft);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::NULL_T, CAEONOp::ExecLeft);
	Table.SetOp(IDatatype::ENUM, IDatatype::NULL_T, CAEONOp::ExecLeft);
	Table.SetOp(IDatatype::INT_32, IDatatype::NULL_T, CAEONOp::ExecLeft);
	Table.SetOp(IDatatype::INT_64, IDatatype::NULL_T, CAEONOp::ExecLeft);
	Table.SetOp(IDatatype::INT_IP, IDatatype::NULL_T, CAEONOp::ExecLeft);

	//	String

	Table.SetOp(IDatatype::STRING, IDatatype::ARRAY, ExecAny_Array);

	//	TimeSpan

	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::TIME_SPAN, ExecTimeSpan);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::ARRAY, ExecAny_Array);

	//	Vectors

	Table.SetOp(IDatatype::VECTOR_2D_F64, IDatatype::VECTOR_2D_F64, ExecVector2D);
	Table.SetOp(IDatatype::VECTOR_3D_F64, IDatatype::VECTOR_3D_F64, ExecVector3D);

	//	Done

	return Table;
	}

CDatum COpSubtract::CalcType (CDatum dLeftType, CDatum dRightType)
	{
	const IDatatype& LeftType = dLeftType;
	const IDatatype& RightType = dRightType;

	if (LeftType.IsNullType())
		{
		if (RightType.IsNullType())
			return CAEONTypes::Get(IDatatype::ANY);
		else
			return dRightType;
		}
	else if (LeftType.IsNullable() && RightType.IsNullable())
		{
		CDatum dNewType = CalcType(LeftType.GetVariantType(), RightType.GetVariantType());
		return CAEONTypeSystem::CreateNullableType(NULL_STR, dNewType);
		}
	else if (LeftType.IsNullable())
		return CalcType(LeftType.GetVariantType(), dRightType);
	else if (RightType.IsNullable())
		return CalcType(dLeftType, RightType.GetVariantType());

	else if (LeftType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);
	else if (!RightType.IsNullType() && RightType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);

	else if (LeftType.IsA(IDatatype::ARRAY))
		{
		CDatum dLeftElementType = LeftType.GetElementType();
		const IDatatype& LeftElementType = dLeftElementType;

		if (RightType.IsA(IDatatype::ARRAY))
			{
			CDatum dRightElementType = RightType.GetElementType();
			const IDatatype& RightElementType = dRightElementType;

			CDatum dResultType = CalcType(dLeftElementType, dRightElementType);
			return CAEONTypeSystem::CreateAnonymousArray(NULL_STR, dResultType);
			}
		else
			{
			CDatum dResultType = CalcType(dLeftElementType, dRightType);
			return CAEONTypeSystem::CreateAnonymousArray(NULL_STR, dResultType);
			}
		}
	else if (LeftType.IsA(IDatatype::STRING))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::DATE_TIME))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::DATE_TIME))
			return CAEONTypes::Get(IDatatype::ANY);
		else if (RightType.IsA(IDatatype::TIME_SPAN))
			return CAEONTypes::Get(IDatatype::DATE_TIME);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::TIME_SPAN))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::TIME_SPAN))
			return CAEONTypes::Get(IDatatype::TIME_SPAN);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::INT_IP) || LeftType.IsA(IDatatype::INT_64))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::INT_IP);
		else if (RightType.IsA(IDatatype::FLOAT_64))
			return CAEONTypes::Get(IDatatype::FLOAT_64);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::INTEGER))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::INT_IP))
			return CAEONTypes::Get(IDatatype::INT_IP);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::INTEGER);
		else if (RightType.IsA(IDatatype::FLOAT_64))
			return CAEONTypes::Get(IDatatype::FLOAT_64);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::FLOAT_64))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::FLOAT_64))
			return CAEONTypes::Get(IDatatype::FLOAT_64);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::FLOAT_64);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::FLOAT))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::NUMBER))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::FLOAT_64))
			return CAEONTypes::Get(IDatatype::FLOAT_64);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::VECTOR_2D_F64))
		{
		if (RightType.IsA(IDatatype::VECTOR_2D_F64))
			return CAEONTypes::Get(IDatatype::VECTOR_2D_F64);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::VECTOR_3D_F64))
		{
		if (RightType.IsA(IDatatype::VECTOR_3D_F64))
			return CAEONTypes::Get(IDatatype::VECTOR_3D_F64);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	}

CDatum COpSubtract::ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return dRight.MathSubtractElementsFrom(dLeft);
	}

CDatum COpSubtract::ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return dLeft.MathSubtractFromElements(dRight);
	}

CDatum COpSubtract::ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	int iCount = Min(dLeft.GetCount(), dRight.GetCount());
	int iResultCount = Max(dLeft.GetCount(), dRight.GetCount());

	//	LATER: If both arrays are typed, then we should try to create an 
	//	appropriate result array.

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(iResultCount);

	for (int i = 0; i < iCount; i++)
		{
		CDatum dLeftElement = dLeft.GetElement(i);
		CDatum dRightElement = dRight.GetElement(i);

		dResult.Append(CAEONOp::Subtract(dLeftElement, dRightElement));
		}

	if (dLeft.GetCount() > dRight.GetCount())
		{
		for (int i = iCount; i < iResultCount; i++)
			dResult.Append(dLeft.GetElement(i));
		}
	else if (dRight.GetCount() > dLeft.GetCount())
		{
		for (int i = iCount; i < iResultCount; i++)
			dResult.Append(CAEONOp::Subtract(CDatum(), dRight.GetElement(i)));
		}

	return dResult;
	}

CDatum COpSubtract::ExecDateTime_DateTime (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(timeSpan(dRight, dLeft));
	}

CDatum COpSubtract::ExecDateTime_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CDateTime(timeSubtractTime(dLeft, dRight)));
	}

CDatum COpSubtract::ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() - dRight.raw_GetDouble());
	}

CDatum COpSubtract::ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() - (double)dRight);
	}

CDatum COpSubtract::ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Subtract, dLeft, dRight.AsExpression());
	}

CDatum COpSubtract::ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Subtract, dLeft.AsExpression(), dRight);
	}

CDatum COpSubtract::ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Subtract, dLeft.AsExpression(), dRight.AsExpression());
	}

CDatum COpSubtract::ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() - (double)dRight.raw_GetInt32());
	}

CDatum COpSubtract::ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() - (double)dRight);
	}

CDatum COpSubtract::ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() - (double)dRight);
	}

CDatum COpSubtract::ExecError (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum::CreateError(strPattern(ERR_OP_NOT_DEFINED, ((const IDatatype&)dLeft.GetDatatype()).GetName(), ((const IDatatype&)dRight.GetDatatype()).GetName()));
	}

CDatum COpSubtract::ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft - dRight.raw_GetDouble());
	}

CDatum COpSubtract::ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	LONGLONG iResult = (int)dLeft - (LONGLONG)dRight.raw_GetInt32();
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		return CDatum((int)iResult);
	else
		return CDatum(CIPInteger(iResult));
	}

CDatum COpSubtract::ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CIPInteger((int)dLeft) - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CIPInteger((int)dLeft) - (const CIPInteger&)dRight);
	}

CDatum COpSubtract::ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	LONGLONG iResult = (LONGLONG)dLeft.raw_GetInt32() - (LONGLONG)dRight.raw_GetInt32();
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		return CDatum((int)iResult);
	else
		return CDatum(CIPInteger(iResult));
	}

CDatum COpSubtract::ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft.raw_GetInt32() - dRight.raw_GetDouble());
	}

CDatum COpSubtract::ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	LONGLONG iResult = (LONGLONG)dLeft.raw_GetInt32() - (LONGLONG)(int)dRight;
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		return CDatum((int)iResult);
	else
		return CDatum(CIPInteger(iResult));
	}

CDatum COpSubtract::ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() - (const CIPInteger&)dRight);
	}

CDatum COpSubtract::ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft - dRight.raw_GetDouble());
	}

CDatum COpSubtract::ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft - (const CIPInteger&)dRight);
	}

CDatum COpSubtract::ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft - dRight.raw_GetDouble());
	}

CDatum COpSubtract::ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft - dRight.AsIPInteger());
	}

CDatum COpSubtract::ExecNull_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONOp::Subtract(CDatum((int)0), dRight, Ctx);
	}

CDatum COpSubtract::ExecTimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CTimeSpan::Subtract(dLeft, dRight));
	}

CDatum COpSubtract::ExecTimeSpan_DateTime (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CDateTime(timeSubtractTime(dRight, dLeft)));
	}

CDatum COpSubtract::ExecVector2D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CVector2D&)dLeft - (const CVector2D&)dRight);
	}

CDatum COpSubtract::ExecVector3D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CVector3D&)dLeft - (const CVector3D&)dRight);
	}

