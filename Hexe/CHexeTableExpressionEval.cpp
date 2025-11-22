//	CHexeTableExpressionEval.cpp
//
//	CHexeTableExpressionEval class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatum CHexeTableExpressionEval::Sort (CDatum dTable, CDatum dSortColumns)

//	Sort
//
//	Returns a new table sorted by the specified columns. The sort columns must
//	be column expressions, which we evaluate on the table to sort.

	{
	IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	If the table has groups, then we need to sort the rows within each group.

	const CAEONTableGroupDefinition& Groups = pTable->GetGroups();
	if (!Groups.IsEmpty())
		{
		IAEONTable::SSubset Subset;
		TArray<TArray<int>> NewIndex;

		const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();
		for (int i = 0; i < GroupIndex.GetCount(); i++)
			{
			TArray<int> SortedRows = SortRows(dTable, dSortColumns, &GroupIndex.GetGroupIndex(i));
			Subset.Rows.Insert(SortedRows);
			NewIndex.Insert(std::move(SortedRows));
			}

		Subset.GroupDef = Groups;
		Subset.GroupIndex = CAEONTableGroupIndex(std::move(NewIndex));

		CAEONTypeSystem TypeSystem;
		CDatum dResult;

		if (!IAEONTable::CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}

	//	Otherwise, we can just sort the entire table.

	else
		{
		IAEONTable::SSubset Subset;
		Subset.Rows = SortRows(dTable, dSortColumns);

		CAEONTypeSystem TypeSystem;
		CDatum dResult;

		if (!IAEONTable::CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}
	}

TArray<int> CHexeTableExpressionEval::SortRows (CDatum dTable, CDatum dSortColumns, const TArray<int>* pRows)
	{
	struct SRowMap
		{
		int iRow = 0;
		int iIndex = 0;		//	Index into SortValues.
		};

	IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	We generate the values for all the sort columns.

	TArray<int> Order;
	TArray<TArray<CDatum>> SortValues;

	for (int i = 0; i < dSortColumns.GetCount(); i++)
		{
		const CAEONExpression* pExpr = dSortColumns.GetElement(i).GetQueryInterface();
		if (!pExpr)
			continue;	//	Not a column expression, so we skip it.

		//	If this is a negation, then we flip the operator.

		const CAEONExpression::SNode* pSortOn = NULL;
		if (pExpr->GetRootNode().iOp == CAEONExpression::EOp::Negate)
			{
			pSortOn = &pExpr->GetNode(pExpr->GetRootNode().iLeft);
			Order.Insert(-1);
			}
		else
			{
			pSortOn = &pExpr->GetRootNode();
			Order.Insert(1);
			}

		//	Now generate the values.

		CHexeColumnExpressionEval Eval(*pExpr, dTable, pRows);
		SortValues.Insert(Eval.EvalColumn(*pSortOn));
		}

	if (SortValues.GetCount() == 0)
		return (pRows ? *pRows : TArray<int>());

	//	Now we generate a new sort order.

	TArray<SRowMap> SortedRows;
	if (pRows)
		{
		SortedRows.InsertEmpty(pRows->GetCount());
		for (int i = 0; i < pRows->GetCount(); i++)
			{
			SortedRows[i].iRow = pRows->GetAt(i);
			SortedRows[i].iIndex = i;
			}
		}
	else
		{
		SortedRows.InsertEmpty(pTable->GetRowCount());
		for (int i = 0; i < pTable->GetRowCount(); i++)
			{
			SortedRows[i].iRow = i;
			SortedRows[i].iIndex = i;
			}
		}

	SortedRows.Sort([&SortValues, &Order](const SRowMap& Row1, const SRowMap& Row2) {

		//	Row1 > Row2		-> 1
		//	Row1 < Row2		-> -1
		//	Row1 == Row2	-> 0

		for (int i = 0; i < SortValues.GetCount(); i++)
			{
			CDatum dValue1 = SortValues[i][Row1.iIndex];
			CDatum dValue2 = SortValues[i][Row2.iIndex];
			int iCompare = dValue1.OpCompare(dValue2) * Order[i];
			if (iCompare != 0)
				return iCompare;
			}

		return ::KeyCompare(Row1.iRow, Row2.iRow);
		});

	TArray<int> NewOrder;
	NewOrder.InsertEmpty(SortedRows.GetCount());
	for (int i = 0; i < SortedRows.GetCount(); i++)
		NewOrder[i] = SortedRows[i].iRow;

	return NewOrder;
	}

