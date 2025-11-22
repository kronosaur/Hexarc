//	COpAdd.cpp
//
//	COpAdd class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_OP_NOT_DEFINED,				"Operator undefined for %s + %s.");
DECLARE_CONST_STRING(ERR_STRING_TOO_BIG,				"Strings may not be larger than %s bytes.");

CAEONOperatorTable COpAdd::CreateTable ()

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
	Table.SetOp(IDatatype::BOOL, IDatatype::STRING, ExecAny_String);

	//	DateTime

	Table.SetOp(IDatatype::DATE_TIME, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::DATE_TIME, IDatatype::STRING, ExecAny_String);
	Table.SetOp(IDatatype::DATE_TIME, IDatatype::TIME_SPAN, ExecDateTime_TimeSpan);

	//	Double

	Table.SetOp(IDatatype::FLOAT_64, IDatatype::FLOAT_64, ExecDouble);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::ENUM, ExecDouble_Int32);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_32, ExecDouble_Int32);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_64, ExecDouble_Int64);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_IP, ExecDouble_IntIP);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::STRING, ExecAny_String);

	//	Enum

	Table.SetOp(IDatatype::ENUM, IDatatype::ENUM, ExecInt32);		//	Enum is stored just like Int32
	Table.SetOp(IDatatype::ENUM, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::ENUM, IDatatype::FLOAT_64, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_32, ExecInt32);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_64, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::INT_IP, ExecInt32_Double);
	Table.SetOp(IDatatype::ENUM, IDatatype::STRING, ExecAny_String);

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
	Table.SetOp(IDatatype::INT_32, IDatatype::STRING, ExecAny_String);

	//	Int64

	Table.SetOp(IDatatype::INT_64, IDatatype::INT_64, ExecInt64);
	Table.SetOp(IDatatype::INT_64, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::INT_64, IDatatype::ENUM, ExecInt64_Int32);
	Table.SetOp(IDatatype::INT_64, IDatatype::FLOAT_64, ExecInt64_Double);
	Table.SetOp(IDatatype::INT_64, IDatatype::INT_32, ExecInt64_Int32);
	Table.SetOp(IDatatype::INT_64, IDatatype::INT_IP, ExecInt64_IntIP);
	Table.SetOp(IDatatype::INT_64, IDatatype::STRING, ExecAny_String);

	//	IntIP

	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_IP, ExecIntIP);
	Table.SetOp(IDatatype::INT_IP, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::INT_IP, IDatatype::ENUM, ExecIntIP_Int32);
	Table.SetOp(IDatatype::INT_IP, IDatatype::FLOAT_64, ExecIntIP_Double);
	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_32, ExecIntIP_Int32);
	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_64, ExecIntIP_Int64);
	Table.SetOp(IDatatype::INT_IP, IDatatype::STRING, ExecAny_String);

	//	NaN

	Table.SetOp(IDatatype::NAN_CONST, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::NAN_CONST, IDatatype::STRING, ExecAny_String);

	//	Null

	Table.SetOpLeft(IDatatype::NULL_T, CAEONOp::ExecRight);
	Table.SetOpRight(IDatatype::NULL_T, CAEONOp::ExecLeft);

	//	String

	Table.SetOp(IDatatype::STRING, IDatatype::STRING, ExecString);
	Table.SetOp(IDatatype::STRING, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::STRING, IDatatype::BOOL, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::DATE_TIME, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::ENUM, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::FLOAT_64, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::INT_32, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::INT_64, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::INT_IP, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::NAN_CONST, ExecString_Any);
	Table.SetOp(IDatatype::STRING, IDatatype::TIME_SPAN, ExecString_Any);

	//	TimeSpan

	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::TIME_SPAN, ExecTimeSpan);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::DATE_TIME, ExecTimeSpan_DateTime);
	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::STRING, ExecAny_String);

	//	Vectors

	Table.SetOp(IDatatype::VECTOR_2D_F64, IDatatype::VECTOR_2D_F64, ExecVector2D);
	Table.SetOp(IDatatype::VECTOR_3D_F64, IDatatype::VECTOR_3D_F64, ExecVector3D);

	//	Done

	return Table;
	}

CDatum COpAdd::CalcType (CDatum dLeftType, CDatum dRightType)
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
			return CAEONTypes::Get(IDatatype::STRING);
		}
	else if (LeftType.IsA(IDatatype::DATE_TIME))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::DATE_TIME))
			return CAEONTypes::Get(IDatatype::ANY);
		else if (RightType.IsA(IDatatype::TIME_SPAN))
			return CAEONTypes::Get(IDatatype::DATE_TIME);
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::TIME_SPAN))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::TIME_SPAN))
			return CAEONTypes::Get(IDatatype::TIME_SPAN);
		else if (RightType.IsA(IDatatype::DATE_TIME))
			return CAEONTypes::Get(IDatatype::DATE_TIME);
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
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
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
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
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
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
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
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
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::NUMBER))
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else if (RightType.IsA(IDatatype::FLOAT_64))
			return CAEONTypes::Get(IDatatype::FLOAT_64);
		else if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::FLOAT);
		else if (RightType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
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
		else if (RightType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::STRING);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	}

CDatum COpAdd::ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	//	Add operator is not commutative when adding strings.
	return dRight.MathAddElementsTo(dLeft);
	}

CDatum COpAdd::ExecAny_String (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecConcatString(dLeft.AsString(), dRight.AsStringView(), Ctx);
	}

CDatum COpAdd::ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return dLeft.MathAddToElements(dRight);
	}

CDatum COpAdd::ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
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

		dResult.Append(CAEONOp::Add(dLeftElement, dRightElement));
		}

	if (dLeft.GetCount() > dRight.GetCount())
		{
		for (int i = iCount; i < iResultCount; i++)
			dResult.Append(dLeft.GetElement(i));
		}
	else if (dRight.GetCount() > dLeft.GetCount())
		{
		for (int i = iCount; i < iResultCount; i++)
			dResult.Append(dRight.GetElement(i));
		}

	return dResult;
	}

CDatum COpAdd::ExecDateTime_TimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CDateTime(timeAddTime(dLeft, dRight)));
	}

CDatum COpAdd::ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() + dRight.raw_GetDouble());
	}

CDatum COpAdd::ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() + (double)dRight);
	}

CDatum COpAdd::ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() + (double)dRight.raw_GetInt32());
	}

CDatum COpAdd::ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() + (double)dRight);
	}

CDatum COpAdd::ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.raw_GetDouble() + (double)dRight);
	}

CDatum COpAdd::ExecError (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum::CreateError(strPattern(ERR_OP_NOT_DEFINED, ((const IDatatype&)dLeft.GetDatatype()).GetName(), ((const IDatatype&)dRight.GetDatatype()).GetName()));
	}

CDatum COpAdd::ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Add, dLeft, dRight.AsExpression());
	}

CDatum COpAdd::ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Add, dLeft.AsExpression(), dRight);
	}

CDatum COpAdd::ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Add, dLeft.AsExpression(), dRight.AsExpression());
	}

CDatum COpAdd::ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft + dRight.raw_GetDouble());
	}

CDatum COpAdd::ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	LONGLONG iResult = (int)dLeft + (LONGLONG)dRight.raw_GetInt32();
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		return CDatum((int)iResult);
	else
		return CDatum(CIPInteger(iResult));
	}

CDatum COpAdd::ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CIPInteger((int)dLeft) + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CIPInteger((int)dLeft) + (const CIPInteger&)dRight);
	}

CDatum COpAdd::ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	LONGLONG iResult = (LONGLONG)dLeft.raw_GetInt32() + (LONGLONG)dRight.raw_GetInt32();
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		return CDatum((int)iResult);
	else
		return CDatum(CIPInteger(iResult));
	}

CDatum COpAdd::ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft.raw_GetInt32() + dRight.raw_GetDouble());
	}

CDatum COpAdd::ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	LONGLONG iResult = (LONGLONG)dLeft.raw_GetInt32() + (LONGLONG)(int)dRight;
	if (iResult >= INT_MIN && iResult <= INT_MAX)
		return CDatum((int)iResult);
	else
		return CDatum(CIPInteger(iResult));
	}

CDatum COpAdd::ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() + (const CIPInteger&)dRight);
	}

CDatum COpAdd::ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft + dRight.raw_GetDouble());
	}

CDatum COpAdd::ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger() + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft + (const CIPInteger&)dRight);
	}

CDatum COpAdd::ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((double)dLeft + dRight.raw_GetDouble());
	}

CDatum COpAdd::ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CIPInteger&)dLeft + dRight.AsIPInteger());
	}

CDatum COpAdd::ExecString (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecConcatString(dLeft.AsStringView(), dRight.AsStringView(), Ctx);
	}

CDatum COpAdd::ExecString_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecConcatString(dLeft.AsStringView(), dRight.AsString(), Ctx);
	}

CDatum COpAdd::ExecTimeSpan (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CTimeSpan::Add(dLeft, dRight));
	}

CDatum COpAdd::ExecTimeSpan_DateTime (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CDateTime(timeAddTime(dRight, dLeft)));
	}

CDatum COpAdd::ExecVector2D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CVector2D&)dLeft + (const CVector2D&)dRight);
	}

CDatum COpAdd::ExecVector3D (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum((const CVector3D&)dLeft + (const CVector3D&)dRight);
	}

CDatum COpAdd::ExecConcatString (const CString& sLeft, const CString& sRight, IAEONOperatorCtx& Ctx)
	{
	if (sLeft.GetLength() + sRight.GetLength() < LARGE_STRING_THRESHOLD)
		return CDatum(CString::Concatenate(sLeft, sRight));
	else if (sLeft.GetLength() + sRight.GetLength() <= Ctx.GetMaxStringSize())
		return CDatum(CString::Concatenate(sLeft, sRight));
	else
		return CDatum::CreateError(strPattern(ERR_STRING_TOO_BIG, strFormatInteger((int)Ctx.GetMaxStringSize(), -1, FORMAT_THOUSAND_SEPARATOR)));
	}
