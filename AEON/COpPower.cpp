//	COpPower.cpp
//
//	COpPower class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const int COpPower::MAX_BASE_FOR_EXP[MAX_EXP_FOR_INT32 + 1] = 
	{
		0,					//	x^0
		0,					//	x^1
		46340,				//	x^2
		1290,				//	x^3
		215,				//	x^4
		73,					//	x^5
		35,					//	x^6
		21,					//	x^7
		14, 				//	x^8
		10, 				//	x^9
		8, 					//	x^10
		7, 					//	x^11
		5, 					//	x^12
		5, 					//	x^13
		4, 					//	x^14
		4, 					//	x^15
		3, 					//	x^16
		3, 					//	x^17
		3, 					//	x^18
		3, 					//	x^19
		2, 					//	x^20
		2, 					//	x^21
		2, 					//	x^22
		2, 					//	x^23
		2, 					//	x^24
		2, 					//	x^25
		2, 					//	x^26
		2, 					//	x^27
		2, 					//	x^28
		2, 					//	x^29
		2, 					//	x^30
	};

CAEONOperatorTable COpPower::CreateTable ()

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

	Table.SetOpLeft(IDatatype::NULL_T, CAEONOp::ExecNaN);
	Table.SetOpRight(IDatatype::NULL_T, CAEONOp::ExecNaN);
	Table.SetOp(IDatatype::NULL_T, IDatatype::ARRAY, ExecAny_Array);
	Table.SetOp(IDatatype::ARRAY, IDatatype::NULL_T, ExecArray_Any);

	//	String

	Table.SetOp(IDatatype::STRING, IDatatype::ARRAY, ExecAny_Array);

	//	TimeSpan

	Table.SetOp(IDatatype::TIME_SPAN, IDatatype::ARRAY, ExecAny_Array);

	//	Done

	return Table;
	}

CDatum COpPower::CalcType (CDatum dLeftType, CDatum dRightType)
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
	else
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return CalcType(dRightType, dLeftType);
		else
			return CAEONTypes::Get(IDatatype::ANY);
		}
	}

CDatum COpPower::ExecAny_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return dRight.MathExpToElements(dLeft);
	}

CDatum COpPower::ExecArray_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return dLeft.MathExpElementsTo(dRight);
	}

CDatum COpPower::ExecArray_Array (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
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

		dResult.Append(CAEONOp::Power(dLeftElement, dRightElement));
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

CDatum COpPower::ExecDouble (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow(dLeft.raw_GetDouble(), dRight.raw_GetDouble()));
	}

CDatum COpPower::ExecDouble_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow(dLeft.raw_GetDouble(), (double)dRight));
	}

CDatum COpPower::ExecDouble_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow(dLeft.raw_GetDouble(), (double)dRight.raw_GetInt32()));
	}

CDatum COpPower::ExecDouble_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow(dLeft.raw_GetDouble(), (double)dRight));
	}

CDatum COpPower::ExecDouble_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow(dLeft.raw_GetDouble(), (double)dRight));
	}

CDatum COpPower::ExecAny_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Power, dLeft, dRight.AsExpression());
	}

CDatum COpPower::ExecExpression_Any (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Power, dLeft.AsExpression(), dRight);
	}

CDatum COpPower::ExecExpression_Expression (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::Power, dLeft.AsExpression(), dRight.AsExpression());
	}

CDatum COpPower::ExecInt_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow((double)dLeft, dRight.raw_GetDouble()));
	}

CDatum COpPower::ExecInt_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecPower_Int32((int)dLeft, dRight.raw_GetInt32());
	}

CDatum COpPower::ExecInt_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CIPInteger((int)dLeft).Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecInt_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(CIPInteger((int)dLeft).Power((const CIPInteger&)dRight));
	}

CDatum COpPower::ExecInt32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecPower_Int32(dLeft.raw_GetInt32(), dRight.raw_GetInt32());
	}

CDatum COpPower::ExecInt32_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow((double)dLeft.raw_GetInt32(), dRight.raw_GetDouble()));
	}

CDatum COpPower::ExecInt32_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return ExecPower_Int32(dLeft.raw_GetInt32(), (int)dRight);
	}

CDatum COpPower::ExecInt32_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger().Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecInt32_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger().Power((const CIPInteger&)dRight));
	}

CDatum COpPower::ExecInt64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger().Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecInt64_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow((double)dLeft, dRight.raw_GetDouble()));
	}

CDatum COpPower::ExecInt64_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger().Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecInt64_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger().Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecInt64_IntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(dLeft.AsIPInteger().Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecIntIP (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(((const CIPInteger&)dLeft).Power((const CIPInteger&)dRight));
	}

CDatum COpPower::ExecIntIP_Double (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(pow((double)dLeft, dRight.raw_GetDouble()));
	}

CDatum COpPower::ExecIntIP_Int (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(((const CIPInteger&)dLeft).Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecIntIP_Int32 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(((const CIPInteger&)dLeft).Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecIntIP_Int64 (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx)
	{
	return CDatum(((const CIPInteger&)dLeft).Power(dRight.AsIPInteger()));
	}

CDatum COpPower::ExecPower_Int32 (int iLeft, int iRight)
	{
	if (iRight == 0)
		return CDatum(1);

	else if (iRight < 0)
		return pow((double)iLeft, (double)iRight);

	else if (iRight == 1)
		return CDatum(iLeft);

	else if (iRight <= MAX_EXP_FOR_INT32 && iLeft <= MAX_BASE_FOR_EXP[iRight])
		{
		//	LATER: Do an integer algorithm.
		return (int)pow((double)iLeft, (double)iRight);
		}
	else
		{
		CIPInteger IPBase(iLeft);
		CIPInteger IPExp(iRight);
		return IPBase.Power(IPExp);
		}
	}
