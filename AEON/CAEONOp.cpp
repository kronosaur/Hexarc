//	CAEONOp.cpp
//
//	CAEONOp class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_ARRAY_TOO_BIG,					"Arrays are limited to  %s elements.");
DECLARE_CONST_STRING(ERR_INVALID_AXIS,					"Arrays only have 1 dimension.");

IAEONOperatorCtx IAEONOperatorCtx::Default;

CAEONOperatorTable CAEONOp::m_Add = COpAdd::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompEqual = COpCompEqual::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompGreaterThan = COpCompGreaterThan::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompGreaterThanOrEqual = COpCompGreaterThanOrEqual::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompIdentical = COpCompIdentical::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompLessThan = COpCompLessThan::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompLessThanOrEqual = COpCompLessThanOrEqual::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompNotEqual = COpCompNotEqual::CreateTable();
CAEONOperatorTableNew CAEONOp::m_CompNotIdentical = COpCompNotIdentical::CreateTable();
CAEONOperatorTableNew CAEONOp::m_Concatenate = COpConcatenate::CreateTable();
CAEONOperatorTable CAEONOp::m_Divide = COpDivide::CreateTable();
CAEONOperatorTable CAEONOp::m_Mod = COpMod::CreateTable();
CAEONOperatorTable CAEONOp::m_Multiply = COpMultiply::CreateTable();
CAEONUnaryOpTable CAEONOp::m_Negate = COpNegate::CreateTable();
CAEONOperatorTable CAEONOp::m_Power = COpPower::CreateTable();
CAEONOperatorTable CAEONOp::m_Subtract = COpSubtract::CreateTable();

CDatum CAEONOp::CalcAddType (CDatum dLeftType, CDatum dRightType)
	{
	return COpAdd::CalcType(dLeftType, dRightType);
	}

CDatum CAEONOp::CalcCompType (CDatum dLeftType, CDatum dRightType)
	{
	const IDatatype& LeftType = dLeftType;
	const IDatatype& RightType = dRightType;

	//	Expressions are special

	if (!LeftType.IsNullType() && LeftType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);
	else if (!RightType.IsNullType() && RightType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);

	//	Always always comparisons to Any

	else if (LeftType.IsAny() || RightType.IsAny())
		return CAEONTypes::Get(IDatatype::BOOL);

	//	Numbers can always be compared

	else if (LeftType.IsA(IDatatype::NUMBER) && RightType.IsA(IDatatype::NUMBER))
		return CAEONTypes::Get(IDatatype::BOOL);

	//	Collections can always be compared

	else if (LeftType.IsA(IDatatype::INDEXED) && RightType.IsA(IDatatype::INDEXED))
		return CAEONTypes::Get(IDatatype::BOOL);

	//	If is-a relationship, then we can compare

	else if (LeftType.IsA(RightType) || RightType.IsA(LeftType))
		return CAEONTypes::Get(IDatatype::BOOL);

	//	Otherwise, error

	else
		return CAEONTypes::Get(IDatatype::ERROR_T);
	}

CDatum CAEONOp::CalcConcatType (CDatum dLeftType, CDatum dRightType)
	{
	return COpConcatenate::CalcType(dLeftType, dRightType);
	}

CDatum CAEONOp::CalcDivideType (CDatum dLeftType, CDatum dRightType)
	{
	return COpDivide::CalcType(dLeftType, dRightType);
	}

CDatum CAEONOp::CalcInType (CDatum dLeftType, CDatum dRightType)
	{
	const IDatatype& LeftType = dLeftType;
	const IDatatype& RightType = dRightType;

	//	Expressions are special

	if (!LeftType.IsNullType() && LeftType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);
	else if (!RightType.IsNullType() && RightType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);

	//	If the right-side is Any, then we accept anything

	else if (RightType.IsAny())
		return CAEONTypes::Get(IDatatype::BOOL);

	//	Null Types are always false, but we allow them.

	else if (LeftType.IsNullType() || RightType.IsNullType())
		return CAEONTypes::Get(IDatatype::BOOL);

	//	If the right-side is a number, then nothing is valid

	else if (RightType.IsA(IDatatype::NUMBER))
		return CAEONTypes::Get(IDatatype::ERROR_T);

	//	If the right-side is a string, then we accept strings

	else if (RightType.IsA(IDatatype::STRING))
		{
		if (LeftType.IsAny() || LeftType.IsA(IDatatype::STRING))
			return CAEONTypes::Get(IDatatype::BOOL);
		else
			return CAEONTypes::Get(IDatatype::ERROR_T);
		}

	//	If the right-side is a range, then we accept numbers

	else if (RightType.IsA(IDatatype::RANGE))
		{
		if (LeftType.IsAny() || LeftType.IsA(IDatatype::NUMBER))
			return CAEONTypes::Get(IDatatype::BOOL);
		else
			return CAEONTypes::Get(IDatatype::ERROR_T);
		}

	//	Otherwise, always return a boolean type.

	else
		return CAEONTypes::Get(IDatatype::BOOL);
	}

CDatum CAEONOp::CalcLogicalAndType (CDatum dLeftType, CDatum dRightType)
	{
	const IDatatype& LeftType = dLeftType;
	const IDatatype& RightType = dRightType;

	if (RightType.IsNullType())
		return CAEONTypes::Get(IDatatype::ANY);
	else if (!LeftType.IsNullType() && LeftType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);
	else if (RightType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);
	else
		return dRightType;
	}

CDatum CAEONOp::CalcLogicalOrType (CDatum dLeftType, CDatum dRightType)
	{
	const IDatatype& LeftType = dLeftType;
	const IDatatype& RightType = dRightType;

	if (LeftType == RightType)
		return dLeftType;

	else if (!LeftType.IsNullType() && LeftType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);
	else if (!RightType.IsNullType() && RightType.IsA(IDatatype::EXPRESSION))
		return CAEONTypes::Get(IDatatype::EXPRESSION);

	//	If the left-type is nullable and the right-type cannot be null, then we
	//	just combine the non-nullable parts.

	else if (LeftType.IsNullable() && !RightType.CanBeNull())
		return CalcLogicalOrType(LeftType.GetVariantType(), dRightType);

	//	If the left-type cannot be null and the right-type is nullable, then we
	//	just combine the non-nullable parts.

	else if (!LeftType.CanBeNull() && RightType.IsNullable())
		return CalcLogicalOrType(dLeftType, RightType.GetVariantType());

	//	If the left type is just null, the result is the right type.

	else if (LeftType.IsNullType())
		return dRightType;

	//	If the right type is just null, the result is the left type.

	else if (RightType.IsNullType())
		return dLeftType;

	//	Keep checking.

	if (LeftType.IsA(RightType))
		return dRightType;
	else if (RightType.IsA(LeftType))
		return dLeftType;

	//	In general we try to find the most specific type that is a superset of
	//	both types.

	else if (LeftType.IsA(CAEONTypeSystem::GetCoreType(IDatatype::ARRAY))
			&& RightType.IsA(CAEONTypeSystem::GetCoreType(IDatatype::ARRAY)))
		{
		const IDatatype& LeftItemType = LeftType.GetElementType();
		const IDatatype& RightItemType = RightType.GetElementType();
		
		if (LeftItemType == RightItemType)
			return dLeftType;
		else
			return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY);
		}
	else if (LeftType.IsA(IDatatype::INT_32))
		{
		if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::INTEGER);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::INT_IP) || LeftType.IsA(IDatatype::INT_64))
		{
		if (RightType.IsA(IDatatype::INT_32))
			return CAEONTypes::Get(IDatatype::INTEGER);
		else if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::INTEGER))
		{
		if (RightType.IsA(IDatatype::FLOAT))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else if (LeftType.IsA(IDatatype::FLOAT))
		{
		if (RightType.IsA(IDatatype::INTEGER))
			return CAEONTypes::Get(IDatatype::NUMBER);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	else
		{
		return CAEONTypes::Get(IDatatype::ANY);
		}
	}

CDatum CAEONOp::CalcModType (CDatum dLeftType, CDatum dRightType)
	{
	return COpMod::CalcType(dLeftType, dRightType);
	}

CDatum CAEONOp::CalcMultiplyType (CDatum dLeftType, CDatum dRightType)
	{
	return COpMultiply::CalcType(dLeftType, dRightType);
	}

CDatum CAEONOp::CalcNegateType (CDatum dType)
	{
	return COpNegate::CalcType(dType);
	}

CDatum CAEONOp::CalcPowerType (CDatum dLeftType, CDatum dRightType)
	{
	return COpPower::CalcType(dLeftType, dRightType);
	}

void CAEONOp::CalcSliceParams (CDatum dStart, CDatum dEnd, int iLength, int& retStart, int& retLen)

//	CalcSliceParams
//
//	We use the same semantics as JavaScript slice:
//	https://developer.mozilla.org/en-US/docs/web/javascript/reference/global_objects/array/slice

	{
	if (dStart.IsNil())
		{
		retStart = 0;
		retLen = 0;
		}
	else if (dStart.IsIdenticalToTrue())
		{
		retStart = 0;
		retLen = iLength;
		}
	else
		{
		int iStart = (int)dStart;
		int iEnd = (dEnd.IsNil() ? iLength : (int)dEnd);

		if (iStart < 0)
			iStart = Max(0, iStart + iLength);

		if (iEnd < 0)
			iEnd = Max(0, iEnd + iLength);

		iEnd = Min(iEnd, iLength);

		if (iEnd <= iStart || iStart >= iLength)
			{
			retStart = 0;
			retLen = 0;
			}
		else
			{
			retStart = iStart;
			retLen = iEnd - iStart;
			}
		}
	}

CDatum CAEONOp::CalcSubtractType (CDatum dLeftType, CDatum dRightType)
	{
	return COpSubtract::CalcType(dLeftType, dRightType);
	}

CDatum CAEONOp::ErrorTooBig (IInvokeCtx& Ctx)
	{
	return CDatum::CreateError(strPattern(ERR_ARRAY_TOO_BIG, strFormatInteger(Ctx.GetLimits().iMaxArrayLen, -1, FORMAT_THOUSAND_SEPARATOR)));
	}

CDatum CAEONOp::ExecConcatenateArray_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight, int iAxis)

//	ExecConcatenateArray_Array
//
//	Default implementation of concatenating one array to another. This only 
//	works for 1D arrays.

	{
	if (iAxis != 0)
		return CDatum::CreateError(ERR_INVALID_AXIS);

	//	Handle 0-length arrays

	int iLeftCount = dLeft.GetCount();
	if (iLeftCount == 0)
		return dRight;

	int iRightCount = dRight.GetCount();
	if (iRightCount == 0)
		return dLeft;

	//	Make sure we do not exceed the maximum size

	int iResultCount = iLeftCount + iRightCount;
	if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
		return ErrorTooBig(Ctx);

	//	If we're combining two arrays, the result type should be appropriate 
	//	to the operands.

	CDatum dLeftType = dLeft.GetDatatype();
	CDatum dRightType = dRight.GetDatatype();
	CDatum dNewDatatype = CAEONTypes::GetCompatibleType(dLeftType, dRightType);

	//	Create a new array

	CDatum dResult = CDatum::CreateArrayAsType(dNewDatatype);
	dResult.GrowToFit(iResultCount);

	for (int i = 0; i < dLeft.GetCount(); i++)
		dResult.Append(dLeft.GetElement(i));

	for (int i = 0; i < dRight.GetCount(); i++)
		dResult.Append(dRight.GetElement(i));

	return dResult;
	}

CDatum CAEONOp::In (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	//	If either left or right is an expression, then we return an expression.

	if (dLeft.GetBasicType() == CDatum::typeExpression || dRight.GetBasicType() == CDatum::typeExpression)
		{
		if (dLeft.GetBasicType() != CDatum::typeExpression)
			dLeft = CAEONExpression::CreateLiteral(dLeft);

		if (dRight.GetBasicType() != CDatum::typeExpression)
			dRight = CAEONExpression::CreateLiteral(dRight);

		return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::In, dLeft.AsExpression(), dRight.AsExpression());
		}

	//	Otherwise, normal.

	else
		return CDatum(dRight.OpContains(dLeft));
	}
