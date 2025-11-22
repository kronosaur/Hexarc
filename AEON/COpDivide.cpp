//	COpDivide.cpp
//
//	COpDivide class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CAEONOperatorTable COpDivide::CreateTable ()

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

	//	Double

	Table.SetOp(IDatatype::FLOAT_64, IDatatype::FLOAT_64, ExecDouble);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::ENUM, ExecDouble_Int32);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_32, ExecDouble_Int32);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_64, ExecDouble_Int64);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_IP, ExecDouble_IntIP);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::TIME_SPAN, ExecNumber_TimeSpan);

	//	Enum

	Table.SetOp(IDatatype::ENUM, IDatatype::ENUM, ExecInt32);		//	Enum is stored just like Int32
	Table.SetOp(IDatatype::ENUM, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::ENUM, IDatatype::FLOAT_64, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_32, ExecInt32);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_64, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_IP, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::TIME_SPAN, ExecNumber_TimeSpan);

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
	Table.SetOp(IDatatype::INT_32, IDatatype::TIME_SPAN, ExecNumber_TimeSpan);

	//	Int64

	Table.SetOp(IDatatype::INT_64, IDatatype::INT_64, ExecInt64);
	Table.SetOp(IDatatype::INT_64, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::INT_64, IDatatype::ENUM, ExecInt64_Int32);
	Table.SetOp(IDatatype::INT_64, IDatatype::FLOAT_64, ExecInt64_Double);
	Table.SetOp(IDatatype::INT_64, IDatatype::INT_32, ExecInt64_Int32);
	Table.SetOp(IDatatype::INT_64, IDatatype::INT_IP, ExecInt64_IntIP);
	Table.SetOp(IDatatype::INT_64, IDatatype::TIME_SPAN, ExecNumber_TimeSpan);

	//	IntIP

	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_IP, ExecIntIP);
	Table.SetOp(IDatatype::INT_IP, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::INT_IP, IDatatype::ENUM, ExecIntIP_Int32);
	Table.SetOp(IDatatype::INT_IP, IDatatype::FLOAT_64, ExecIntIP_Double);
	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_32, ExecIntIP_Int32);
	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_64, ExecIntIP_Int64);
	Table.SetOp(IDatatype::INT_IP, IDatatype::TIME_SPAN, ExecNumber_TimeSpan);

	//	NaN

	Table.SetOp(IDatatype::NAN_CONST, IDatatype::ARRAY, ExecAny_Array);

	//	Null

	Table.SetOpLeft(IDatatype::NULL_T, CAEONOp::ExecNaN);
	Table.SetOpRight(IDatatype::NULL_T, CAEONOp::ExecNaN);
	Table.SetOp(IDatatype::NULL_T, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::ARRAY, IDatatype::NULL_T, ExecArray_Any);

	//	String

	Table.SetOp(IDatatype::STRING, IDatatype::ARRAY, ExecAny_Array);

	//	TimeSpan

	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::FLOAT_64, ExecTimeSpan_Number);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::INT_32, ExecTimeSpan_Number);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::INT_64, ExecTimeSpan_Number);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::INT_IP, ExecTimeSpan_Number);

	//	Vector

	Table.SetOp(IDatatype::VECTOR_2D_F64, IDatatype::FLOAT_64, ExecVector2D_Number);
	Table.SetOp(IDatatype::VECTOR_2D_F64, IDatatype::INT_32, ExecVector2D_Number);
	Table.SetOp(IDatatype::VECTOR_2D_F64, IDatatype::INT_64, ExecVector2D_Number);
	Table.SetOp(IDatatype::VECTOR_2D_F64, IDatatype::INT_IP, ExecVector2D_Number);

	Table.SetOp(IDatatype::VECTOR_3D_F64, IDatatype::FLOAT_64, ExecVector3D_Number);
	Table.SetOp(IDatatype::VECTOR_3D_F64, IDatatype::INT_32, ExecVector3D_Number);
	Table.SetOp(IDatatype::VECTOR_3D_F64, IDatatype::INT_64, ExecVector3D_Number);
	Table.SetOp(IDatatype::VECTOR_3D_F64, IDatatype::INT_IP, ExecVector3D_Number);

	//	Done

	return Table;
	}

CDatum COpDivide::CalcType (CDatum dLeftType, CDatum dRightType)
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
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::TIME_SPAN))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::TIME_SPAN);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::INT_IP) || LeftType.IsA(IDatatype::INT_64))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::NUMBER);
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
			return CAEONTypes::Get(IDatatype::NUMBER);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::NUMBER);
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
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::FLOAT_64);
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
		if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::VECTOR_2D_F64);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::VECTOR_3D_F64))
		{
		if (RightType.IsA(IDatatype::NUMBER))
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

CDatum COpDivide::ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return dRight.MathDivideByElements(dLeft);
	}

CDatum COpDivide::ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return dLeft.MathDivideElementsBy(dRight);
	}

CDatum COpDivide::ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
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

		dResult.Append(CAEONOp::Divide(dLeftElement, dRightElement));
		}

	if (dLeft.GetCount() > dRight.GetCount())
		{
		for (int i = iCount; i < iResultCount; i++)
			dResult.Append(CDatum::CreateNaN());
		}
	else if (dRight.GetCount() > dLeft.GetCount())
		{
		for (int i = iCount; i < iResultCount; i++)
			dResult.Append(CDatum::CreateNaN());
		}

	return dResult;
	}

CDatum COpDivide::ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double(dLeft.raw_GetDouble(), dRight.raw_GetDouble());
	}

CDatum COpDivide::ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double(dLeft.raw_GetDouble(), (double)dRight);
	}

CDatum COpDivide::ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double(dLeft.raw_GetDouble(), (double)dRight.raw_GetInt32());
	}

CDatum COpDivide::ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double(dLeft.raw_GetDouble(), (double)dRight);
	}

CDatum COpDivide::ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double(dLeft.raw_GetDouble(), (double)dRight);
	}

CDatum COpDivide::ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double((double)dLeft, dRight.raw_GetDouble());
	}

CDatum COpDivide::ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Divide, dLeft, dRight.AsExpression());
	}

CDatum COpDivide::ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Divide, dLeft.AsExpression(), dRight);
	}

CDatum COpDivide::ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Divide, dLeft.AsExpression(), dRight.AsExpression());
	}

CDatum COpDivide::ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int32((int)dLeft, dRight.raw_GetInt32());
	}

CDatum COpDivide::ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int64((DWORDLONG)dLeft, (DWORDLONG)dRight);
	}

CDatum COpDivide::ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_IntIP(CIPInteger((int)dLeft), (const CIPInteger&)dRight);
	}

CDatum COpDivide::ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int32(dLeft.raw_GetInt32(), dRight.raw_GetInt32());
	}

CDatum COpDivide::ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double((double)dLeft.raw_GetInt32(), dRight.raw_GetDouble());
	}

CDatum COpDivide::ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int32(dLeft.raw_GetInt32(), (int)dRight);
	}

CDatum COpDivide::ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int64((DWORDLONG)dLeft, (DWORDLONG)dRight);
	}

CDatum COpDivide::ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_IntIP(dLeft.AsIPInteger(), (const CIPInteger&)dRight);
	}

CDatum COpDivide::ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int64((DWORDLONG)dLeft, (DWORDLONG)dRight);
	}

CDatum COpDivide::ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double((double)dLeft, dRight.raw_GetDouble());
	}

CDatum COpDivide::ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int64((DWORDLONG)dLeft, (DWORDLONG)dRight);
	}

CDatum COpDivide::ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Int64((DWORDLONG)dLeft, (DWORDLONG)dRight);
	}

CDatum COpDivide::ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_IntIP(dLeft.AsIPInteger(), (const CIPInteger&)dRight);
	}

CDatum COpDivide::ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_IntIP((const CIPInteger&)dLeft, (const CIPInteger&)dRight);
	}

CDatum COpDivide::ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_Double((double)dLeft, dRight.raw_GetDouble());
	}

CDatum COpDivide::ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_IntIP((const CIPInteger&)dLeft, dRight.AsIPInteger());
	}

CDatum COpDivide::ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_IntIP((const CIPInteger&)dLeft, dRight.AsIPInteger());
	}

CDatum COpDivide::ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecDivide_IntIP((const CIPInteger&)dLeft, dRight.AsIPInteger());
	}

CDatum COpDivide::ExecNumber_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum::CreateNaN();
	}

CDatum COpDivide::ExecTimeSpan_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	double rValue = dRight;
	if (rValue == 0.0)
		return CDatum::CreateNaN();

	return CDatum(CTimeSpan::Divide(dLeft, rValue));
	}

CDatum COpDivide::ExecDivide_Int32 (int iLeft, int iRight)
	{
	if (iRight == 0)
		return CDatum::CreateNaN();
	else if ((iLeft % iRight) == 0)
		return CDatum(iLeft / iRight);
	else
		return CDatum((double)iLeft / (double)iRight);
	}

CDatum COpDivide::ExecDivide_Int64 (DWORDLONG dwLeft, DWORDLONG dwRight)
	{
	if (dwRight == 0)
		return CDatum::CreateNaN();
	else if ((dwLeft % dwRight) == 0)
		return CDatum(CIPInteger(dwLeft / dwRight));
	else
		return CDatum((double)dwLeft / (double)dwRight);
	}

CDatum COpDivide::ExecDivide_IntIP (const CIPInteger& Left, const CIPInteger& Right)
	{
	CIPInteger Quotient;
	CIPInteger Remainder;
	if (!Left.DivideMod(Right, Quotient, Remainder))
		return CDatum::CreateNaN();

	if (Remainder.IsZero())
		return CDatum(Quotient);
	else
		return CDatum(Left.AsDouble() / Right.AsDouble());
	}

CDatum COpDivide::ExecVector2D_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	double rValue = dRight;
	if (rValue == 0.0)
		return CDatum::CreateNaN();

	return CDatum((const CVector2D&)dLeft / rValue);
	}

CDatum COpDivide::ExecVector3D_Number (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	double rValue = dRight;
	if (rValue == 0.0)
		return CDatum::CreateNaN();

	return CDatum((const CVector3D&)dLeft / rValue);
	}
