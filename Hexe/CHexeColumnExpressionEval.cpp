//	CHexeColumnExpressionEval.cpp
//
//	CHexeColumnExpressionEval class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_UNKNOWN_COLUMN,				"Unknown column: %s.");
DECLARE_CONST_STRING(ERR_NO_EXP,						"No expression.");

CHexeColumnExpressionEval::CHexeColumnExpressionEval (const CAEONExpression& Expr, CDatum dTable, const TArray<int>* pRows) :
		m_Expr(Expr),
		m_dTable(dTable),
		m_pRows(pRows)
	{
	MapColumns();
	}

CDatum CHexeColumnExpressionEval::EvalNode (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	switch (Node.iOp)
		{
		case CAEONExpression::EOp::Abs:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.MathAbs();
			}

		case CAEONExpression::EOp::Add:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return CAEONOp::Add(dLeft, dRight);
			}

		case CAEONExpression::EOp::And:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			if (dLeft.IsNil())
				return CDatum();

			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			if (dRight.IsNil())
				return CDatum();

			return dRight;
			}

		case CAEONExpression::EOp::All:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return All(ColExpr);
			}

		case CAEONExpression::EOp::Any:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return Any(ColExpr);
			}

		case CAEONExpression::EOp::Array:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return Array(ColExpr);
			}

		case CAEONExpression::EOp::Average:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return Average(ColExpr);
			}

		case CAEONExpression::EOp::Ceil:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.MathCeil();
			}

		case CAEONExpression::EOp::Clean:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			if (dValue.GetBasicType() == CDatum::typeString)
				return CDatum(dValue.AsStringView().Clean());
			else
				return CDatum(dValue.AsString().Clean());
			}

		case CAEONExpression::EOp::Column:
			return Ctx.pTable->GetFieldValue(Ctx.iRow, m_Col[Node.iDataID].iIndex);

		case CAEONExpression::EOp::Count:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			if (ColExpr.iOp == CAEONExpression::EOp::Literal)
				{
				if (m_pRows)
					return m_pRows->GetCount();
				else
					return Ctx.pTable->GetRowCount();
				}
			else
				return Count(ColExpr);
			}

		case CAEONExpression::EOp::DateTime:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return CDatum::CreateDateTime(dValue);
			}

		case CAEONExpression::EOp::Deref:
			{
			CDatum dArray = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dIndex = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			CAEONTypeSystem TypeSystem;
			return dArray.GetElementAt(TypeSystem, dIndex);
			}

		case CAEONExpression::EOp::Divide:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return CAEONOp::Divide(dLeft, dRight);
			}

		case CAEONExpression::EOp::EqualTo:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return CHexeProcess::ExecuteIsEquivalent(dLeft, dRight);
			}

		case CAEONExpression::EOp::ErrorLiteral:
			return m_Expr.GetLiteral(Node.iDataID);

		case CAEONExpression::EOp::False:
			return CDatum(false);

		case CAEONExpression::EOp::Find:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dSearch = (Node.iRight != -1 ? EvalNode(Ctx, m_Expr.GetNode(Node.iRight)) : CDatum());

			int iPos;
			if (dValue.GetBasicType() == CDatum::typeString)
				iPos = strFindNoCase(dValue.AsStringView(), dSearch.AsString());
			else
				iPos = strFindNoCase(dValue.AsString(), dSearch.AsString());

			return (iPos == -1 ? CDatum() : CDatum(iPos));
			}

		case CAEONExpression::EOp::First:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return First(ColExpr);
			}

		case CAEONExpression::EOp::Floor:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.MathFloor();
			}

		case CAEONExpression::EOp::GreaterThan:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return (CHexeProcess::ExecuteCompare(dLeft, dRight) > 0);
			}

		case CAEONExpression::EOp::GreaterThanOrEqualTo:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return (CHexeProcess::ExecuteCompare(dLeft, dRight) >= 0);
			}

		case CAEONExpression::EOp::If:
			{
			CDatum dCondition = EvalNode(Ctx, m_Expr.GetNode(Node.iDataID));
			if (dCondition.IsNil())
				{
				const CAEONExpression::SNode& FalseExpr = m_Expr.GetNode(Node.iRight);
				return EvalNode(Ctx, FalseExpr);
				}
			else
				{
				const CAEONExpression::SNode& TrueExpr = m_Expr.GetNode(Node.iLeft);
				return EvalNode(Ctx, TrueExpr);
				}
			}

		case CAEONExpression::EOp::In:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return dRight.OpContains(dLeft);
			}

		case CAEONExpression::EOp::Integer:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.AsInteger();
			}

		case CAEONExpression::EOp::Left:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			int iCount = (Node.iRight != -1 ? (int)EvalNode(Ctx, m_Expr.GetNode(Node.iRight)) : 0);
			CString sAsString;
			CStringView sValue;
			if (dValue.GetBasicType() == CDatum::typeString)
				sValue = dValue.AsStringView();
			else
				{
				sAsString = dValue.AsString();
				sValue = sAsString;
				}

			int iLeft = ::Max(0, ::Min(iCount, sValue.GetLength()));
			if (iLeft == 0)
				return CDatum();
			else
				return CDatum(strSubString(sValue, 0, iLeft));
			}

		case CAEONExpression::EOp::Length:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			if (dValue.GetBasicType() == CDatum::typeString)
				return CDatum(dValue.AsStringView().GetLength());
			else
				return CDatum(dValue.AsString().GetLength());
			}

		case CAEONExpression::EOp::LessThan:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return (CHexeProcess::ExecuteCompare(dLeft, dRight) < 0);
			}

		case CAEONExpression::EOp::LessThanOrEqualTo:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return (CHexeProcess::ExecuteCompare(dLeft, dRight) <= 0);
			}

		case CAEONExpression::EOp::Literal:
			return m_Expr.GetLiteral(Node.iDataID);

		case CAEONExpression::EOp::Lowercase:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			if (dValue.GetBasicType() == CDatum::typeString)
				return CDatum(strToLower(dValue.AsStringView()));
			else
				return CDatum(strToLower(dValue.AsString()));
			}

		case CAEONExpression::EOp::Max:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return Max(ColExpr);
			}

		case CAEONExpression::EOp::Median:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return Median(ColExpr);
			}

		case CAEONExpression::EOp::Member:
			{
			CDatum dObj = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dField = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return dObj.GetProperty(dField.AsStringView());
			}

		case CAEONExpression::EOp::Min:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return Min(ColExpr);
			}

		case CAEONExpression::EOp::Mod:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return CAEONOp::Mod(dLeft, dRight);
			}

		case CAEONExpression::EOp::Multiply:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return CAEONOp::Multiply(dLeft, dRight);
			}

		case CAEONExpression::EOp::Negate:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return CAEONOp::Negate(dValue);
			}

		case CAEONExpression::EOp::Not:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return CDatum(dValue.IsNil());
			}

		case CAEONExpression::EOp::NotEqualTo:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return !CHexeProcess::ExecuteIsEquivalent(dLeft, dRight);
			}

		case CAEONExpression::EOp::Number:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.AsNumber(true);
			}

		case CAEONExpression::EOp::Or:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			if (!dLeft.IsNil())
				return dLeft;
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return dRight;
			}

		case CAEONExpression::EOp::Power:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return CAEONOp::Power(dLeft, dRight);
			}

		case CAEONExpression::EOp::Real:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.AsFloat(true);
			}

		case CAEONExpression::EOp::Right:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			int iCount = (Node.iRight != -1 ? (int)EvalNode(Ctx, m_Expr.GetNode(Node.iRight)) : 0);
			CString sAsString;
			CStringView sValue;
			if (dValue.GetBasicType() == CDatum::typeString)
				sValue = dValue.AsStringView();
			else
				{
				sAsString = dValue.AsString();
				sValue = sAsString;
				}

			int iRight = ::Max(0, ::Min(iCount, sValue.GetLength()));
			if (iRight == 0)
				return CDatum();
			else
				return CDatum(strSubString(sValue, sValue.GetLength() - iRight, iRight));
			}

		case CAEONExpression::EOp::Round:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.MathRound();
			}

		case CAEONExpression::EOp::Sign:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			return dValue.MathSign();
			}

		case CAEONExpression::EOp::Slice:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CString sAsString;
			CStringView sValue;
			if (dValue.GetBasicType() == CDatum::typeString)
				sValue = dValue.AsStringView();
			else
				{
				sAsString = dValue.AsString();
				sValue = sAsString;
				}

			CDatum dStart = (Node.iRight != -1 ? EvalNode(Ctx, m_Expr.GetNode(Node.iRight)) : CDatum());
			CDatum dEnd = (Node.iDataID != -1 ? EvalNode(Ctx, m_Expr.GetNode(Node.iDataID)) : CDatum(sValue.GetLength()));

			int iStart, iLen;
			CAEONOp::CalcSliceParams(dStart, dEnd, sValue.GetLength(), iStart, iLen);

			if (iLen <= 0)
				return CDatum();

			return CDatum(strSubString(sValue, iStart, iLen));
			}

		case CAEONExpression::EOp::String:
			{
			//	With formatting

			if (Node.iRight != -1)
				{
				CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
				CDatum dFormat = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
				return CDatum::CreateString(dValue, dFormat);
				}

			//	Just convert to string

			else
				{
				CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
				if (dValue.GetBasicType() == CDatum::typeString)
					return dValue;
				else
					return CDatum(dValue.AsString());
				}
			}

		case CAEONExpression::EOp::Subtract:
			{
			CDatum dLeft = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			CDatum dRight = EvalNode(Ctx, m_Expr.GetNode(Node.iRight));
			return CAEONOp::Subtract(dLeft, dRight);
			}

		case CAEONExpression::EOp::Sum:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return Sum(ColExpr);
			}

		case CAEONExpression::EOp::TimeSpan:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			if (dValue.GetBasicType() == CDatum::typeTimeSpan)
				return dValue;
			else
				return CDatum(dValue.AsTimeSpan());
			}

		case CAEONExpression::EOp::True:
			return CDatum(true);

		case CAEONExpression::EOp::UniqueArray:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return UniqueArray(ColExpr);
			}

		case CAEONExpression::EOp::UniqueCount:
			{
			const CAEONExpression::SNode& ColExpr = m_Expr.GetNode(Node.iLeft);
			return UniqueCount(ColExpr);
			}

		case CAEONExpression::EOp::Uppercase:
			{
			CDatum dValue = EvalNode(Ctx, m_Expr.GetNode(Node.iLeft));
			if (dValue.GetBasicType() == CDatum::typeString)
				return CDatum(strToUpper(dValue.AsStringView()));
			else
				return CDatum(strToUpper(dValue.AsString()));
			}

		default:
			return CDatum();
		}
	}

bool CHexeColumnExpressionEval::All (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			if (EvalNode(Ctx, Node).IsNil())
				return false;
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			if (EvalNode(Ctx, Node).IsNil())
				return false;
			}
		}

	return true;
	}

bool CHexeColumnExpressionEval::Any (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			if (!EvalNode(Ctx, Node).IsNil())
				return true;
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			if (!EvalNode(Ctx, Node).IsNil())
				return true;
			}
		}

	return false;
	}

void CHexeColumnExpressionEval::AppendValues (const CAEONExpression::SNode& Node, CDatum dColumn) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			dColumn.Append(EvalNode(Ctx, Node));
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			dColumn.Append(EvalNode(Ctx, Node));
			}
		}
	}

CDatum CHexeColumnExpressionEval::Array (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	CDatum dResult(CDatum::typeArray);

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			dResult.Append(dValue);
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			dResult.Append(dValue);
			}
		}

	return dResult;
	}

CDatum CHexeColumnExpressionEval::Average (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return CDatum();

	if (Node.iOp == CAEONExpression::EOp::Column)
		return AverageOfColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return AverageOfExpr(Ctx, Node);
	}

CDatum CHexeColumnExpressionEval::AverageOfColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	//	Welford's incremental mean. This is much faster because we don't 
	//	accumulate a big number.

	double mean = 0.0;
	int iCount = 0;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			iCount++;
			mean += ((double)dValue - mean) / (double)iCount;
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			iCount++;
			mean += ((double)dValue - mean) / (double)iCount;
			}
		}

	if (iCount == 0)
		return CDatum();

	return CDatum(mean);
	}

CDatum CHexeColumnExpressionEval::AverageOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	//	Welford's incremental mean. This is much faster because we don't 
	//	accumulate a big number.

	double mean = 0.0;
	int iCount = 0;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			iCount++;
			mean += ((double)dValue - mean) / (double)iCount;
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			iCount++;
			mean += ((double)dValue - mean) / (double)iCount;
			}
		}

	if (iCount == 0)
		return CDatum();

	return CDatum(mean);
	}

CDatum CHexeColumnExpressionEval::Column (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);


	if (Node.iOp == CAEONExpression::EOp::Column)
		{
		CAEONTypeSystem Types;
		return Ctx.pTable->GetCol(Types, m_Expr.GetColumnID(Node.iDataID));
		}
	else
		{
		CDatum dResult(CDatum::typeArray);
		if (m_pRows)
			{
			dResult.GrowToFit(m_pRows->GetCount());
			for (int i = 0; i < m_pRows->GetCount(); i++)
				{
				Ctx.iRow = m_pRows->GetAt(i);
				dResult.Append(EvalNode(Ctx, Node));
				}
			}
		else
			{
			dResult.GrowToFit(Ctx.pTable->GetRowCount());
			for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
				{
				dResult.Append(EvalNode(Ctx, Node));
				}
			}

		return dResult;
		}
	}

int CHexeColumnExpressionEval::Count (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	int iCount = 0;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			if (!EvalNode(Ctx, Node).IsNil())
				{
				iCount++;
				}
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			if (!EvalNode(Ctx, Node).IsNil())
				{
				iCount++;
				}
			}
		}

	return iCount;
	}

CDatum CHexeColumnExpressionEval::Eval () const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (m_pRows && m_pRows->GetCount() > 0)
		Ctx.iRow = m_pRows->GetAt(0);
	else
		Ctx.iRow = 0;

	return EvalNode(Ctx, m_Expr.GetRootNode());
	}

TArray<CDatum> CHexeColumnExpressionEval::EvalColumn (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	TArray<CDatum> Result;
	if (m_pRows)
		{
		Result.InsertEmpty(m_pRows->GetCount());
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			Result[i] = EvalNode(Ctx, Node);
			}
		}
	else
		{
		Result.InsertEmpty(Ctx.pTable->GetRowCount());
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			Result[Ctx.iRow] = EvalNode(Ctx, Node);
			}
		}

	return Result;
	}

CDatum CHexeColumnExpressionEval::EvalRow (int iRow) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	Ctx.iRow = iRow;

	return EvalNode(Ctx, m_Expr.GetRootNode());
	}

TArray<int> CHexeColumnExpressionEval::FilterRows (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	//	If we have a column comparison against a literal, then we can optimize.

	if (IsColumnCompare(Node))
		{
		int iColIndex = m_Col[m_Expr.GetNode(Node.iLeft).iDataID].iIndex;
		CDatum dValue = m_Expr.GetLiteral(m_Expr.GetNode(Node.iRight).iDataID);

		switch (Node.iOp)
			{
			case CAEONExpression::EOp::EqualTo:
				return FilterRowsColumnEqLit(Ctx, iColIndex, dValue);

			case CAEONExpression::EOp::GreaterThan:
				return FilterRowsColumnGreaterLit(Ctx, iColIndex, dValue);

			case CAEONExpression::EOp::GreaterThanOrEqualTo:
				return FilterRowsColumnGreaterEqLit(Ctx, iColIndex, dValue);

			case CAEONExpression::EOp::LessThan:
				return FilterRowsColumnLesserLit(Ctx, iColIndex, dValue);

			case CAEONExpression::EOp::LessThanOrEqualTo:
				return FilterRowsColumnLesserEqLit(Ctx, iColIndex, dValue);

			case CAEONExpression::EOp::NotEqualTo:
				return FilterRowsColumnNeqLit(Ctx, iColIndex, dValue);

			default:
				return FilterRowsExpr(Ctx, Node);
			}
		}
	else
		{
		return FilterRowsExpr(Ctx, Node);
		}
	}

TArray<int> CHexeColumnExpressionEval::FilterRowsColumnEqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const
	{
	TArray<int> Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (CHexeProcess::ExecuteIsEquivalent(dColValue, dLiteral))
				Result.Insert(m_pRows->GetAt(i));
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (CHexeProcess::ExecuteIsEquivalent(dColValue, dLiteral))
				Result.Insert(iRow);
			}
		}

	return Result;
	}

TArray<int> CHexeColumnExpressionEval::FilterRowsColumnGreaterLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const
	{
	TArray<int> Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) > 0)
				Result.Insert(m_pRows->GetAt(i));
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) > 0)
				Result.Insert(iRow);
			}
		}

	return Result;
	}

TArray<int> CHexeColumnExpressionEval::FilterRowsColumnGreaterEqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const
	{
	TArray<int> Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) >= 0)
				Result.Insert(m_pRows->GetAt(i));
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) >= 0)
				Result.Insert(iRow);
			}
		}

	return Result;
	}

TArray<int> CHexeColumnExpressionEval::FilterRowsColumnLesserLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const
	{
	TArray<int> Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) < 0)
				Result.Insert(m_pRows->GetAt(i));
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) < 0)
				Result.Insert(iRow);
			}
		}

	return Result;
	}

TArray<int> CHexeColumnExpressionEval::FilterRowsColumnLesserEqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const
	{
	TArray<int> Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) <= 0)
				Result.Insert(m_pRows->GetAt(i));
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (CHexeProcess::ExecuteCompare(dColValue, dLiteral) <= 0)
				Result.Insert(iRow);
			}
		}

	return Result;
	}

TArray<int> CHexeColumnExpressionEval::FilterRowsColumnNeqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const
	{
	TArray<int> Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (!CHexeProcess::ExecuteIsEquivalent(dColValue, dLiteral))
				Result.Insert(m_pRows->GetAt(i));
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dColValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (!CHexeProcess::ExecuteIsEquivalent(dColValue, dLiteral))
				Result.Insert(iRow);
			}
		}

	return Result;
	}

TArray<int> CHexeColumnExpressionEval::FilterRowsExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	TArray<int> Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			if (!EvalNode(Ctx, Node).IsNil())
				{
				Result.Insert(Ctx.iRow);
				}
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			if (!EvalNode(Ctx, Node).IsNil())
				{
				Result.Insert(Ctx.iRow);
				}
			}
		}

	return Result;
	}

CDatum CHexeColumnExpressionEval::First (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			return dValue;
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			return dValue;
			}
		}

	return CDatum();
	}

TSortMap<CDatum, TArray<int>, CKeyCompareEquivalent<CDatum>> CHexeColumnExpressionEval::GroupRows (const CAEONExpression::SNode& Node, DWORD dwFlags) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	TSortMap<CDatum, TArray<int>, CKeyCompareEquivalent<CDatum>> Result;

	for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
		{
		CDatum dGroup = EvalNode(Ctx, Node);
		if (!dGroup.IsNil() || (dwFlags & FLAG_ALLOW_NULL))
			{
			bool bNew;
			TArray<int> *pGroup = Result.SetAt(dGroup, &bNew);
			pGroup->Insert(Ctx.iRow);
			}
		}

	return Result;
	}

bool CHexeColumnExpressionEval::IsColumnCompare (const CAEONExpression::SNode& Node) const
	{
	switch (Node.iOp)
		{
		case CAEONExpression::EOp::EqualTo:
		case CAEONExpression::EOp::NotEqualTo:
		case CAEONExpression::EOp::LessThan:
		case CAEONExpression::EOp::LessThanOrEqualTo:
		case CAEONExpression::EOp::GreaterThan:
		case CAEONExpression::EOp::GreaterThanOrEqualTo:
			return (m_Expr.GetNode(Node.iLeft).iOp == CAEONExpression::EOp::Column) && (m_Expr.GetNode(Node.iRight).iOp == CAEONExpression::EOp::Literal);

		default:
			return false;
		}
	}

bool CHexeColumnExpressionEval::IsError (CString* retsError) const
	{
	if (m_Expr.IsEmpty())
		{
		if (retsError)
			*retsError = ERR_NO_EXP;

		return true;
		}

	//	If the root node is an error, then we return that.

	else if (m_Expr.GetRootNode().iOp == CAEONExpression::EOp::ErrorLiteral)
		{
		if (retsError)
			*retsError = m_Expr.GetLiteral(m_Expr.GetRootNode().iDataID).AsString();

		return true;
		}

	//	Otherwise, make sure all the column references are valid.

	else
		{
		for (int i = 0; i < m_Col.GetCount(); i++)
			{
			if (m_Col[i].iIndex == -1)
				{
				if (retsError)
					*retsError = strPattern(ERR_UNKNOWN_COLUMN, m_Expr.GetColumnID(i));

				return true;
				}
			}

		//	No error.

		return false;
		}
	}

CAEONTableGroupIndex CHexeColumnExpressionEval::MakeGroupIndex () const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	const CAEONExpression::SNode& Node = m_Expr.GetRootNode();

	if (Node.iOp == CAEONExpression::EOp::Column)
		return MakeGroupIndexByColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return MakeGroupIndexExpr(Ctx, Node);
	}

CAEONTableGroupIndex CHexeColumnExpressionEval::MakeGroupIndexByColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	struct DatumHash {
		std::size_t operator()(const CDatum& d) const noexcept { return d.Hash(); }
	};

	struct DatumEqual {
		bool operator()(const CDatum& a, const CDatum& b) const noexcept { return a.OpIsIdentical(b); }
	};

	std::unordered_map<CDatum, int, DatumHash, DatumEqual> Map;
	TArray<TArray<int>> Index;
	TSortMap<CDatum, int> Sorted;
	int iRowCount = Ctx.pTable->GetRowCount();

	for (int iRow = 0; iRow < iRowCount; iRow++)
		{
		CDatum dGroup = Ctx.pTable->GetFieldValue(iRow, iColIndex);
		if (dGroup.IsNil())
			continue;

		auto [it, inserted] = Map.try_emplace(dGroup);
		if (inserted)
			{
			it->second = Index.GetCount();
			Sorted.Insert(dGroup, it->second);
			Index.InsertEmpty();
			Index[it->second].GrowToFit(::Min(50000, iRowCount / 20));
			}

		Index[it->second].Insert(iRow);
		}

	TArray<TArray<int>> SortedIndex;
	SortedIndex.InsertEmpty(Index.GetCount());
	for (int i = 0; i < Sorted.GetCount(); i++)
		SortedIndex[i] = std::move(Index[Sorted[i]]);

	return CAEONTableGroupIndex(std::move(SortedIndex));
	}

CAEONTableGroupIndex CHexeColumnExpressionEval::MakeGroupIndexExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	TSortMap<CDatum, TArray<int>> Map;

	for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
		{
		CDatum dGroup = EvalNode(Ctx, m_Expr.GetRootNode());
		if (dGroup.IsNil())
			continue;

		TArray<int>* pGroup = Map.SetAt(dGroup);
		pGroup->Insert(Ctx.iRow);
		}

	TArray<TArray<int>> Index;
	for (int i = 0; i < Map.GetCount(); i++)
		Index.Insert(std::move(Map[i]));

	return CAEONTableGroupIndex(std::move(Index));
	}

void CHexeColumnExpressionEval::Mark ()
	{
	m_dTable.Mark();

	for (int i = 0; i < m_Col.GetCount(); i++)
		m_Col[i].dType.Mark();
	}

CDatum CHexeColumnExpressionEval::Map (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	CDatum dResult(CDatum::typeArray);

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsIdenticalToNil())
				continue;

			dResult.Append(dValue);
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsIdenticalToNil())
				continue;

			dResult.Append(dValue);
			}
		}

	return dResult;
	}

void CHexeColumnExpressionEval::MapColumns ()
	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	m_Col.DeleteAll();
	m_Col.InsertEmpty(m_Expr.GetColumnCount());

	for (int i = 0; i < m_Expr.GetColumnCount(); i++)
		{
		CStringView sColID = m_Expr.GetColumnID(i);
		if (!pTable->FindCol(sColID, &m_Col[i].iIndex))
			m_Col[i].iIndex = -1;

		auto ColDesc = pTable->GetColDesc(m_dTable, m_Col[i].iIndex);
		m_Col[i].dType = ColDesc.dType;
		}
	}

CDatum CHexeColumnExpressionEval::Max (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return CDatum();

	if (Node.iOp == CAEONExpression::EOp::Column)
		return MaxOfColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return MaxOfExpr(Ctx, Node);
	}

CDatum CHexeColumnExpressionEval::MaxOfColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	CDatum dMax;
	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return CDatum();

		dMax = Ctx.pTable->GetFieldValue(m_pRows->GetAt(0), iColIndex);

		for (int i = 1; i < m_pRows->GetCount(); i++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				dMax = dNewValue;
			}
		}
	else
		{
		dMax = Ctx.pTable->GetFieldValue(0, iColIndex);

		for (int iRow = 1; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				dMax = dNewValue;
			}
		}

	return dMax;
	}

CDatum CHexeColumnExpressionEval::MaxOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	CDatum dMax;
	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return CDatum();

		Ctx.iRow = m_pRows->GetAt(0);
		dMax = EvalNode(Ctx, Node);

		for (int i = 1; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dNewValue = EvalNode(Ctx, Node);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				dMax = dNewValue;
			}
		}
	else
		{
		Ctx.iRow = 0;
		dMax = EvalNode(Ctx, Node);

		for (Ctx.iRow = 1; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dNewValue = EvalNode(Ctx, Node);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				dMax = dNewValue;
			}
		}

	return dMax;
	}

int CHexeColumnExpressionEval::MaxRow (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return -1;

	if (Node.iOp == CAEONExpression::EOp::Column)
		return MaxRowOfColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return MaxRowOfExpr(Ctx, Node);
	}

int CHexeColumnExpressionEval::MaxRowOfColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	CDatum dMax;
	int iMaxRow = -1;

	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return -1;

		dMax = Ctx.pTable->GetFieldValue(m_pRows->GetAt(0), iColIndex);
		iMaxRow = m_pRows->GetAt(0);

		for (int i = 1; i < m_pRows->GetCount(); i++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				{
				dMax = dNewValue;
				iMaxRow = m_pRows->GetAt(i);
				}
			}
		}
	else
		{
		dMax = Ctx.pTable->GetFieldValue(0, iColIndex);
		iMaxRow = 0;

		for (int iRow = 1; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				{
				dMax = dNewValue;
				iMaxRow = iRow;
				}
			}
		}

	return (dMax.IsIdenticalToNil() ? -1 : iMaxRow);
	}

int CHexeColumnExpressionEval::MaxRowOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	CDatum dMax;
	int iMaxRow = -1;

	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return -1;

		Ctx.iRow = m_pRows->GetAt(0);
		dMax = EvalNode(Ctx, Node);
		iMaxRow = Ctx.iRow;

		for (int i = 1; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dNewValue = EvalNode(Ctx, Node);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				{
				dMax = dNewValue;
				iMaxRow = Ctx.iRow;
				}
			}
		}
	else
		{
		Ctx.iRow = 0;
		dMax = EvalNode(Ctx, Node);
		iMaxRow = 0;

		for (Ctx.iRow = 1; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dNewValue = EvalNode(Ctx, Node);
			int iCompare = dNewValue.OpCompare(dMax);
			if (iCompare == 1)
				{
				dMax = dNewValue;
				iMaxRow = Ctx.iRow;
				}
			}
		}

	return (dMax.IsIdenticalToNil() ? -1 : iMaxRow);
	}

CDatum CHexeColumnExpressionEval::Median (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return CDatum();

	if (Node.iOp == CAEONExpression::EOp::Column)
		return MedianOfColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return MedianOfExpr(Ctx, Node);
	}

CDatum CHexeColumnExpressionEval::MedianOfColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	TArray<CDatum> Values;

	if (m_pRows)
		{
		Values.GrowToFit(m_pRows->GetCount());
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Values.Insert(dValue);
			}
		}
	else
		{
		Values.GrowToFit(Ctx.pTable->GetRowCount());
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Values.Insert(dValue);
			}
		}

	return CComplexArray::CalcMedian(Values);
	}

CDatum CHexeColumnExpressionEval::MedianOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	TArray<CDatum> Values;

	if (m_pRows)
		{
		Values.GrowToFit(m_pRows->GetCount());
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Values.Insert(dValue);
			}
		}
	else
		{
		Values.GrowToFit(Ctx.pTable->GetRowCount());
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Values.Insert(dValue);
			}
		}

	return CComplexArray::CalcMedian(Values);
	}

CDatum CHexeColumnExpressionEval::Min (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return CDatum();

	if (Node.iOp == CAEONExpression::EOp::Column)
		return MinOfColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return MinOfExpr(Ctx, Node);
	}

CDatum CHexeColumnExpressionEval::MinOfColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	CDatum dMin;
	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return CDatum();

		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || dMin.IsIdenticalToNil())
				dMin = dNewValue;
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || dMin.IsIdenticalToNil())
				dMin = dNewValue;
			}
		}

	return dMin;
	}

CDatum CHexeColumnExpressionEval::MinOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	CDatum dMin;

	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return CDatum();

		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dNewValue = EvalNode(Ctx, Node);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || dMin.IsIdenticalToNil())
				dMin = dNewValue;
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dNewValue = EvalNode(Ctx, Node);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || dMin.IsIdenticalToNil())
				dMin = dNewValue;
			}
		}

	return dMin;
	}

int CHexeColumnExpressionEval::MinRow (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return -1;

	if (Node.iOp == CAEONExpression::EOp::Column)
		return MinRowOfColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return MinRowOfExpr(Ctx, Node);
	}

int CHexeColumnExpressionEval::MinRowOfColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	CDatum dMin;
	int iMinRow = -1;

	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return -1;

		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || iMinRow == -1)
				{
				dMin = dNewValue;
				iMinRow = m_pRows->GetAt(i);
				}
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dNewValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || iMinRow == -1)
				{
				dMin = dNewValue;
				iMinRow = iRow;
				}
			}
		}

	return iMinRow;
	}

int CHexeColumnExpressionEval::MinRowOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	CDatum dMin;
	int iMinRow = -1;

	if (m_pRows)
		{
		if (m_pRows->GetCount() == 0)
			return -1;

		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dNewValue = EvalNode(Ctx, Node);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || iMinRow == -1)
				{
				dMin = dNewValue;
				iMinRow = Ctx.iRow;
				}
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dNewValue = EvalNode(Ctx, Node);
			if (dNewValue.IsNil())
				continue;

			int iCompare = dNewValue.OpCompare(dMin);
			if (iCompare == -1 || iMinRow == -1)
				{
				dMin = dNewValue;
				iMinRow = Ctx.iRow;
				}
			}
		}

	return iMinRow;
	}

CDatum CHexeColumnExpressionEval::Sum (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return CDatum();

	if (Node.iOp == CAEONExpression::EOp::Column)
		return SumOfColumn(Ctx, m_Col[Node.iDataID].iIndex);
	else
		return SumOfExpr(Ctx, Node);
	}

CDatum CHexeColumnExpressionEval::SumOfColumn (SEvalCtx& Ctx, int iColIndex) const
	{
	CNumberValue Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			CDatum dValue = Ctx.pTable->GetFieldValue(m_pRows->GetAt(i), iColIndex);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Result.Add(dValue);
			if (!Result.IsValidNumber())
				return CDatum::CreateNaN();
			}
		}
	else
		{
		for (int iRow = 0; iRow < Ctx.pTable->GetRowCount(); iRow++)
			{
			CDatum dValue = Ctx.pTable->GetFieldValue(iRow, iColIndex);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Result.Add(dValue);
			if (!Result.IsValidNumber())
				return CDatum::CreateNaN();
			}
		}

	return Result.GetDatum();
	}

CDatum CHexeColumnExpressionEval::SumOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const
	{
	CNumberValue Result;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Result.Add(dValue);
			if (!Result.IsValidNumber())
				return CDatum::CreateNaN();
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			if (!dValue.CanSum())
				return CDatum::CreateNaN();

			Result.Add(dValue);
			if (!Result.IsValidNumber())
				return CDatum::CreateNaN();
			}
		}

	return Result.GetDatum();
	}

CDatum CHexeColumnExpressionEval::UniqueArray (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	TSortMap<CDatum, bool, CKeyCompareEquivalent<CDatum>> Unique;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			Unique.SetAt(dValue, true);
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			Unique.SetAt(dValue, true);
			}
		}

	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < Unique.GetCount(); i++)
		{
		dResult.Append(Unique.GetKey(i));
		}

	return dResult;
	}

int CHexeColumnExpressionEval::UniqueCount (const CAEONExpression::SNode& Node) const
	{
	SEvalCtx Ctx;
	Ctx.pTable = m_dTable.GetTableInterface();
	if (!Ctx.pTable)
		throw CException(errFail);

	if (Ctx.pTable->GetRowCount() == 0)
		return 0;

	TSortMap<CDatum, bool, CKeyCompareEquivalent<CDatum>> Unique;

	if (m_pRows)
		{
		for (int i = 0; i < m_pRows->GetCount(); i++)
			{
			Ctx.iRow = m_pRows->GetAt(i);
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			Unique.SetAt(dValue, true);
			}
		}
	else
		{
		for (Ctx.iRow = 0; Ctx.iRow < Ctx.pTable->GetRowCount(); Ctx.iRow++)
			{
			CDatum dValue = EvalNode(Ctx, Node);
			if (dValue.IsNil())
				continue;

			Unique.SetAt(dValue, true);
			}
		}

	return Unique.GetCount();
	}
