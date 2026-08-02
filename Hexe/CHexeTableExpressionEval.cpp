//	CHexeTableExpressionEval.cpp
//
//	CHexeTableExpressionEval class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CHILD_ORDER,					"childOrder");
DECLARE_CONST_STRING(FIELD_LEVEL,						"level");
DECLARE_CONST_STRING(FIELD_LEVEL_COL_ID,				"levelColID");
DECLARE_CONST_STRING(FIELD_PARENT_COL_ID,				"parentColID");
DECLARE_CONST_STRING(FIELD_ROOT_ORDER,					"rootOrder");
DECLARE_CONST_STRING(FIELD_ROOT_VALUE,					"rootValue");

CDatum CHexeTableExpressionEval::Hierarchize (CDatum dTable, CDatum dOptions)

//	Hierarchize
//
//	Generate a table ordered by hierarchy with a Level column added (0 = root, 
//	1 = first level, etc.)
//
//	dOptions:
//
//		parentColID: The column expression that defines the parent relationship
//		rootValue: The value that defines a root node (default: nil)
//		rootOrder: An array of column expressions that define the order of root nodes
//		childOrder: An array of column expressions that define the order of child nodes
//		levelColID: Name of level column

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	Table must have an index.

	if (pTable->GetKeyType() == IAEONTable::EKeyType::None)
		return CDatum::CreateError(strPattern(".hierarchical: Table must have an index"));

	//	Options

	SHierarchicalOptions Options;
	CString sError;
	if (!ParseHierarchicalOptions(*pTable, dOptions, Options, &sError))
		return CDatum::CreateError(sError);

	//	Generate a list of all root rows.

	TArray<int> Roots;
	TSortMap<CDatum, TArray<int>> ChildIndex;
	for (int iRow = 0; iRow < pTable->GetRowCount(); iRow++)
		{
		CDatum dParentID = pTable->GetFieldValue(iRow, Options.iParentCol);

		//	If this is a root, add it to the root list.

		if (dParentID.OpIsEqual(Options.dRootValue))
			Roots.Insert(iRow);

		//	Otherwise, this is a child, so add it to a list of children of the
		//	same parent.

		else
			{
			auto* pChildList = ChildIndex.SetAt(dParentID);
			pChildList->Insert(iRow);
			}
		}

	//	Sort the root rows

	if (!Options.dRootOrder.IsNil())
		Roots = SortRows(dTable, Options.dRootOrder, &Roots);

	//	Sort the child lists

	if (!Options.dChildOrder.IsNil())
		{
		for (int i = 0; i < ChildIndex.GetCount(); i++)
			ChildIndex[i] = SortRows(dTable, Options.dChildOrder, &ChildIndex[i]);
		}

	//	Generate a new table in hierarchical order.

	TArray<int> HierarchyOrder;
	TSortMap<CDatum, SNode> Visited;
	CDatum dLevelCol = IAEONTable::CreateColumn(CAEONTypes::Get(IDatatype::INT_32));

	//	Look over all roots and add their children recursively.

	for (int i = 0; i < Roots.GetCount(); i++)
		{
		int iRootRow = Roots[i];
		CDatum dID = pTable->GetRowID(iRootRow);
		SNode* pNode = Visited.SetAt(dID);
		if (pNode->state != ENode::white)
			throw CException(errFail);

		//	Add the root

		HierarchyOrder.Insert(iRootRow);
		dLevelCol.Append(CDatum(0));
		pNode->state = ENode::gray;

		//	Add the children

		const TArray<int>* pChildren = ChildIndex.GetAt(dID);
		if (pChildren && pChildren->GetCount() > 0)
			{
			AppendChildRows(*pTable, iRootRow, ChildIndex, Visited, 1, *pChildren, HierarchyOrder, dLevelCol);
			}

		//	Mark the root as complete. NOTE: We need to get pNode again because
		//	it might have moved.

		pNode = Visited.SetAt(dID);
		pNode->state = ENode::black;
		}

	//	We will now generate a new table with the specified order and the extra column.

	IAEONTable::SSubset Subset;
	Subset.Rows = std::move(HierarchyOrder);

	CAEONTypeSystem TypeSystem;
	CDatum dResult;

	if (!IAEONTable::CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
		return CDatum();

	//	We need to clone the sorted table because we're going to add a column.

	dResult = dResult.Clone(CDatum::EClone::ShallowCopy);
	IAEONTable* pResultTable = dResult.GetTableInterface();
	if (!pResultTable)
		throw CException(errFail);

	//	Add the level column.

	IAEONTable::EResult iResult = pResultTable->InsertColumn(Options.sLevelColName, CAEONTypes::Get(IDatatype::INT_32), dLevelCol);
	if (iResult != IAEONTable::EResult::OK)
		return CDatum();

	//	Done

	return dResult;
	}

void CHexeTableExpressionEval::AppendChildRows (const IAEONTable& Table, int iParentRow, const TSortMap<CDatum, TArray<int>>& ChildIndex, TSortMap<CDatum, SNode>& Visited, int iLevel, const TArray<int>& ChildRows, TArray<int>& retResult, CDatum dLevelColumn)
	{
	for (int i = 0; i < ChildRows.GetCount(); i++)
		{
		int iChildRow = ChildRows[i];
		CDatum dID = Table.GetRowID(iChildRow);
		SNode* pNode = Visited.SetAt(dID);
		if (pNode->state != ENode::white)
			continue;

		pNode->state = ENode::gray;

		//	Add the child

		retResult.Insert(iChildRow);
		dLevelColumn.Append(CDatum(iLevel));

		//	Add the grandchildren

		const TArray<int>* pGrandChildren = ChildIndex.GetAt(dID);
		if (pGrandChildren && pGrandChildren->GetCount() > 0)
			{
			AppendChildRows(Table, iChildRow, ChildIndex, Visited, iLevel + 1, *pGrandChildren, retResult, dLevelColumn);
			}

		//	Mark the child as complete. NOTE: We need to get pNode again because
		//	it might have moved.

		pNode = Visited.SetAt(dID);
		pNode->state = ENode::black;
		}
	}

bool CHexeTableExpressionEval::ParseHierarchicalColumnName (const IAEONTable& Table, CDatum dValue, int* retiColIndex, CString* retsError)
	{
	if (dValue.GetBasicType() == CDatum::typeString)
		{
		CStringView sColID = dValue.AsStringView();
		if (!Table.FindCol(sColID, retiColIndex))
			{
			if (retsError) *retsError = strPattern(".hierarchical: Unknown table column: %s", sColID);
			return false;
			}
		}
	else if (dValue.GetBasicType() == CDatum::Types::typeExpression)
		{
		const CAEONExpression& Expr = dValue.AsExpression();
		if (Expr.GetRootNode().iOp != CAEONExpression::EOp::Column)
			{
			if (retsError) *retsError = strPattern(".hierarchical: Must be a column reference: %s", Expr.AsString());
			return false;
			}

		CStringView sColID = Expr.GetColumnID(Expr.GetRootNode().iDataID);
		if (!Table.FindCol(sColID, retiColIndex))
			{
			if (retsError) *retsError = strPattern(".hierarchical: Unknown table column: %s", sColID);
			return false;
			}
		}
	else
		{
		if (retsError) *retsError = strPattern(".hierarchical: Invalid column identifier: %s", dValue.AsString());
		return false;
		}

	return true;
	}

bool CHexeTableExpressionEval::ParseHierarchicalOptions (const IAEONTable& Table, CDatum dOptions, SHierarchicalOptions& retOptions, CString* retsError)
	{
	CDatum dResultSchema = Table.GetSchema();

	if (dOptions.GetBasicType() == CDatum::typeString || dOptions.GetBasicType() == CDatum::typeExpression)
		{
		if (!ParseHierarchicalColumnName(Table, dOptions, &retOptions.iParentCol, retsError))
			return false;

		retOptions.dChildOrder = CDatum();
		retOptions.dRootOrder = CDatum();
		retOptions.dRootValue = CDatum();
		retOptions.sLevelColName = IAEONTable::CalcUniqueColName(dResultSchema, FIELD_LEVEL);
		}
	else if (CAEONMapColumnExpression* pSrc = CAEONMapColumnExpression::Upconvert(dOptions))
		{
		if (!ParseHierarchicalColumnName(Table, pSrc->GetElementAsValue(FIELD_PARENT_COL_ID), &retOptions.iParentCol, retsError))
			return false;

		retOptions.dRootValue = pSrc->GetElementAsValue(FIELD_ROOT_VALUE);
		retOptions.dRootOrder = pSrc->GetElementAsValue(FIELD_ROOT_ORDER);
		retOptions.dChildOrder = pSrc->GetElementAsValue(FIELD_CHILD_ORDER);

		retOptions.sLevelColName = pSrc->GetElementAsValue(FIELD_LEVEL_COL_ID).AsString();
		if (retOptions.sLevelColName.IsEmpty())
			retOptions.sLevelColName = FIELD_LEVEL;

		retOptions.sLevelColName = IAEONTable::CalcUniqueColName(dResultSchema, retOptions.sLevelColName);
		}
	else
		{
		if (!ParseHierarchicalColumnName(Table, dOptions.GetElement(FIELD_PARENT_COL_ID), &retOptions.iParentCol, retsError))
			return false;

		retOptions.dRootValue = dOptions.GetElement(FIELD_ROOT_VALUE);
		retOptions.dRootOrder = dOptions.GetElement(FIELD_ROOT_ORDER);
		retOptions.dChildOrder = dOptions.GetElement(FIELD_CHILD_ORDER);

		retOptions.sLevelColName = dOptions.GetElement(FIELD_LEVEL_COL_ID).AsString();
		if (retOptions.sLevelColName.IsEmpty())
			retOptions.sLevelColName = FIELD_LEVEL;

		retOptions.sLevelColName = IAEONTable::CalcUniqueColName(dResultSchema, retOptions.sLevelColName);
		}

	return true;
	}

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

	if (dSortColumns.GetBasicType() == CDatum::Types::typeExpression)
		{
		CDatum dArray(CDatum::typeArray);
		dArray.Append(dSortColumns);
		dSortColumns = dArray;
		}

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

