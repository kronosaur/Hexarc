//	COpNegate.cpp
//
//	COpNegate class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_OP_NOT_DEFINED,				"Operator undefined for %s - %s.");

CAEONUnaryOpTable COpNegate::CreateTable ()

//	CreateTable
//
//	Returns a table of operator implementation functions.

	{
	//	By default for anything we don't handle we get nan

	CAEONUnaryOpTable Table(CAEONOp::ExecUnaryNaN);

	Table.SetOp(IDatatype::ARRAY, ExecArray);
	Table.SetOp(IDatatype::ENUM, ExecInt32);
	Table.SetOp(IDatatype::EXPRESSION, ExecExpression);
	Table.SetOp(IDatatype::FLOAT_64, ExecDouble);
	Table.SetOp(IDatatype::INT_32, ExecInt32);
	Table.SetOp(IDatatype::INT_64, ExecInt64);
	Table.SetOp(IDatatype::INT_IP, ExecIntIP);
	Table.SetOp(IDatatype::NULL_T, CAEONOp::ExecUnaryNull);
	Table.SetOp(IDatatype::TIME_SPAN, ExecTimeSpan);
	Table.SetOp(IDatatype::VECTOR_2D_F64, ExecVector2D);
	Table.SetOp(IDatatype::VECTOR_3D_F64, ExecVector3D);

	//	Done

	return Table;
	}

CDatum COpNegate::CalcType (CDatum dType)
	{
	const IDatatype& Type = dType;

	if (Type.IsNullType())
		{
		return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (Type.IsNullable())
		{
		CDatum dNewType = CalcType(Type.GetVariantType());
		return CAEONTypeSystem::CreateNullableType(NULL_STR, dNewType);
		}

	else if (Type.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);

	else if (Type.IsA(IDatatype::ARRAY))
		{
		CDatum dElementType = Type.GetElementType();
		return CAEONTypeSystem::CreateAnonymousArray(NULL_STR, CalcType(dElementType));
		}
	else if (Type.IsA(IDatatype::TIME_SPAN) || Type.IsA(IDatatype::VECTOR_2D_F64) || Type.IsA(IDatatype::VECTOR_3D_F64))
		{
		return dType;
		}
	else if (Type.IsA(IDatatype::INT_32))
		{
		//	Can't always fit a negative value in an int32.
		return CAEONTypes::Get(IDatatype::INTEGER);
		}
	else if (Type.IsA(IDatatype::NUMBER) || Type.IsA(IDatatype::TIME_SPAN))
		{
		return dType;
		}
	else
		{
		return CAEONTypes::Get(IDatatype::ANY);
		}
	}

CDatum COpNegate::ExecArray (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	return dValue.MathNegateElements();
	}

CDatum COpNegate::ExecDouble (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	return CDatum(-dValue.raw_GetDouble());
	}

CDatum COpNegate::ExecExpression (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateUnaryOp(CAEONExpression::EOp::Negate, dValue.AsExpression());
	}

CDatum COpNegate::ExecInt32 (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	int iValue = dValue.raw_GetInt32();
	if (iValue == INT_MIN)
		return CDatum(CIPInteger(-(LONGLONG)iValue));
	else
		return CDatum(-iValue);
	}

CDatum COpNegate::ExecInt64 (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	return CDatum(-dValue.AsIPInteger());
	}

CDatum COpNegate::ExecIntIP (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	return CDatum(-(const CIPInteger&)dValue);
	}

CDatum COpNegate::ExecTimeSpan (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	const CTimeSpan& Value = dValue;
	return CDatum(CTimeSpan(Value.Days(), Value.Seconds(), !Value.IsNegative()));
	}

CDatum COpNegate::ExecVector2D (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	return CDatum(-(const CVector2D&)dValue);
	}

CDatum COpNegate::ExecVector3D (CDatum dValue, IAEONOperatorCtx& Ctx)
	{
	return CDatum(-(const CVector3D&)dValue);
	}
