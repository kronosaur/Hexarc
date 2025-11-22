//	CHexeExpressionEval.cpp
//
//	CHexeExpressionEval class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

class CComputeAverage
	{
	public:
		static CDatum Compute (const CHexeColumnExpressionEval& Eval) { return Eval.Average(); }
	};

class CComputeMax
	{
	public:
		static CDatum Compute (const CHexeColumnExpressionEval& Eval) { return Eval.Max(); }
	};

class CComputeMedian
	{
	public:
		static CDatum Compute (const CHexeColumnExpressionEval& Eval) { return Eval.Median(); }
	};

class CComputeMin
	{
	public:
		static CDatum Compute (const CHexeColumnExpressionEval& Eval) { return Eval.Min(); }
	};

class CComputeSum
	{
	public:
		static CDatum Compute (const CHexeColumnExpressionEval& Eval) { return Eval.Sum(); }
	};

CDatum CHexeExpressionEval::Average () const
	{
	return Compute<CComputeAverage>();
	}

CDatum CHexeExpressionEval::CreateGroupedTableFromRows (CDatum dTable, TArray<int>&& Rows)
	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	IAEONTable::SSubset Subset;
	Subset.Rows = std::move(Rows);
		
	//	Preserve grouping

	Subset.GroupDef = pTable->GetGroups();

	TArray<TArray<int>> GroupIndexArray;
	GroupIndexArray.InsertEmpty(Subset.Rows.GetCount());
	for (int i  = 0; i < GroupIndexArray.GetCount(); i++)
		{
		GroupIndexArray[i].InsertEmpty(1);
		GroupIndexArray[i][0] = Subset.Rows[i];
		}

	Subset.GroupIndex = CAEONTableGroupIndex(std::move(GroupIndexArray));

	//	Create a new table (this has one row per group)

	CDatum dResult;
	CAEONTypeSystem TypeSystem;
	if (!IAEONTable::CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
		return CDatum();

	//	Now we re-key based on the group keys.

	CDatum dExprs = CAEONMapColumnExpression::CreateIdentity(dResult);
	const CAEONMapColumnExpression* pExprs = CAEONMapColumnExpression::Upconvert(dExprs);
	if (!pExprs)
		throw CException(errFail);

	CHexeMapColumnExpressionEval Eval(*pExprs, dResult);
	return Eval.Summarize();
	}

CDatum CHexeExpressionEval::Map () const
	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	If the table has groups, we return an array of arrays.

	const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();
	if (!GroupIndex.IsEmpty())
		{
		CDatum dResult = CDatum::CreateDictionary(CAEONTypes::Get(IDatatype::DICTIONARY));

		CHexeTableGroupEval GroupEval(pTable->GetGroups(), m_dTable);
		TArray<CDatum> Keys = GroupEval.GetKeysFromIndex(GroupIndex);

		for (int i = 0; i < GroupIndex.GetCount(); i++)
			{
			CHexeColumnExpressionEval Eval(m_Expr, m_dTable, &GroupIndex.GetGroupIndex(i));
			dResult.SetElementAt(Keys[i], Eval.Map());
			}

		return dResult;
		}

	//	Otherwise, we return a single array.

	else
		{
		CHexeColumnExpressionEval Eval(m_Expr, m_dTable);
		return Eval.Map();
		}
	}

CDatum CHexeExpressionEval::Max () const
	{
	return Compute<CComputeMax>();
	}

CDatum CHexeExpressionEval::MaxRow () const
	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	If the table has groups, then we need to process as groups, and we 
	//	return a table of 1 max row per group.

	const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();
	if (!GroupIndex.IsEmpty())
		{
		TArray<int> Result;

		for (int i = 0; i < GroupIndex.GetCount(); i++)
			{
			CHexeColumnExpressionEval Eval(m_Expr, m_dTable, &GroupIndex.GetGroupIndex(i));
			int iRow = Eval.MaxRow();
			if (iRow != -1)
				Result.Insert(iRow);
			}

		return CreateGroupedTableFromRows(m_dTable, std::move(Result));
		}

	//	Otherwise, we return a single row.

	else
		{
		CHexeColumnExpressionEval Eval(m_Expr, m_dTable);
		int iRow = Eval.MaxRow();
		if (iRow == -1)
			return CDatum();
		else
			return m_dTable.GetElement(iRow);
		}
	}

CDatum CHexeExpressionEval::Median () const
	{
	return Compute<CComputeMedian>();
	}

CDatum CHexeExpressionEval::Min () const
	{
	return Compute<CComputeMin>();
	}

CDatum CHexeExpressionEval::MinRow () const
	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	If the table has groups, then we need to process as groups, and we 
	//	return a table of 1 min row per group.

	const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();
	if (!GroupIndex.IsEmpty())
		{
		TArray<int> Result;

		for (int i = 0; i < GroupIndex.GetCount(); i++)
			{
			CHexeColumnExpressionEval Eval(m_Expr, m_dTable, &GroupIndex.GetGroupIndex(i));
			int iRow = Eval.MinRow();
			if (iRow != -1)
				Result.Insert(iRow);
			}

		return CreateGroupedTableFromRows(m_dTable, std::move(Result));
		}

	//	Otherwise, we return a single row.

	else
		{
		CHexeColumnExpressionEval Eval(m_Expr, m_dTable);
		int iRow = Eval.MinRow();
		if (iRow == -1)
			return CDatum();
		else
			return m_dTable.GetElement(iRow);
		}
	}

CDatum CHexeExpressionEval::Sum () const
	{
	return Compute<CComputeSum>();
	}

