//	COpCompGreaterThan.cpp
//
//	COpCompGreaterThan class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CAEONOperatorTableNew COpCompGreaterThan::CreateTable ()

//	CreateTable
//
//	Returns a table of operator implementation functions.

	{
	//	By default for anything we don't handle we treat the two operands as
	//	scalars and combine them into an array.

	CAEONOperatorTableNew Table(ExecAny_Any);
	//	Same-type numeric comparisons (fast path)

	Table.SetOp(IDatatype::FLOAT_64, IDatatype::FLOAT_64, ExecDouble);
	Table.SetOp(IDatatype::INT_32, IDatatype::INT_32, ExecInt32);
	Table.SetOp(IDatatype::INT_64, IDatatype::INT_64, ExecInt64);
	Table.SetOp(IDatatype::INT_IP, IDatatype::INT_IP, ExecIntIP);

	//	Mixed integer/Float64 comparisons (precision-safe)

	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_32, ExecNumber_Any);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_64, ExecNumber_Any);
	Table.SetOp(IDatatype::FLOAT_64, IDatatype::INT_IP, ExecNumber_Any);
	Table.SetOp(IDatatype::INT_32, IDatatype::FLOAT_64, ExecNumber_Any);
	Table.SetOp(IDatatype::INT_64, IDatatype::FLOAT_64, ExecNumber_Any);
	Table.SetOp(IDatatype::INT_IP, IDatatype::FLOAT_64, ExecNumber_Any);
	//	Expressions

	Table.SetOpLeft(IDatatype::EXPRESSION, ExecExpression_Any);
	Table.SetOpRight(IDatatype::EXPRESSION, ExecAny_Expression);
	Table.SetOp(IDatatype::EXPRESSION, IDatatype::EXPRESSION, ExecExpression_Expression);

	//	Done

	return Table;
	}


CDatum COpCompGreaterThan::ExecDouble (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CDatum(dLeft.raw_GetDouble() > dRight.raw_GetDouble());
	}

CDatum COpCompGreaterThan::ExecInt32 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CDatum((int)dLeft > (int)dRight);
	}

CDatum COpCompGreaterThan::ExecInt64 (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CDatum((DWORDLONG)dLeft > (DWORDLONG)dRight);
	}

CDatum COpCompGreaterThan::ExecIntIP (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CDatum(((const CIPInteger&)dLeft).Compare((const CIPInteger&)dRight) > 0);
	}
CDatum COpCompGreaterThan::ExecNumber_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	int iCompare;
	if (COpCompHelpers::TryCompareIntFloat(dLeft, dRight, iCompare))
		return CDatum(iCompare > 0);
	else
		return CDatum(dLeft.OpCompare(dRight) > 0);
	}

CDatum COpCompGreaterThan::ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::GreaterThan, dLeft, dRight.AsExpression());
	}

CDatum COpCompGreaterThan::ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::GreaterThan, dLeft.AsExpression(), dRight);
	}

CDatum COpCompGreaterThan::ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::GreaterThan, dLeft.AsExpression(), dRight.AsExpression());
	}
