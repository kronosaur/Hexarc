//	COpCompNotEqual.cpp
//
//	COpCompNotEqual class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CAEONOperatorTableNew COpCompNotEqual::CreateTable ()

//	CreateTable
//
//	Returns a table of operator implementation functions.

	{
	//	By default for anything we don't handle we treat the two operands as
	//	scalars and combine them into an array.

	CAEONOperatorTableNew Table(ExecAny_Any);

	//	Expressions

	Table.SetOpLeft(IDatatype::EXPRESSION, ExecExpression_Any);
	Table.SetOpRight(IDatatype::EXPRESSION, ExecAny_Expression);
	Table.SetOp(IDatatype::EXPRESSION, IDatatype::EXPRESSION, ExecExpression_Expression);

	//	Done

	return Table;
	}

CDatum COpCompNotEqual::ExecAny_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::NotEqualTo, dLeft, dRight.AsExpression());
	}

CDatum COpCompNotEqual::ExecExpression_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::NotEqualTo, dLeft.AsExpression(), dRight);
	}

CDatum COpCompNotEqual::ExecExpression_Expression (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CAEONExpression::CreateBinaryOp(CAEONExpression::EOp::NotEqualTo, dLeft.AsExpression(), dRight.AsExpression());
	}
