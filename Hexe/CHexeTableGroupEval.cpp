//	CHexeTableGroupEval.cpp
//
//	CHexeTableGroupEval class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_SEPARATOR,						"|");

CAEONTableGroupIndex CHexeTableGroupEval::CreateIndex () const
	{
	if (m_GroupDef.IsEmpty())
		throw CException(errFail);

	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	if (m_GroupDef.GetCount() == 1)
		{
		CHexeColumnExpressionEval Eval(m_GroupDef.GetExpression(0), m_dTable);
		return Eval.MakeGroupIndex();
		}
	else
		{
		//	For multiple we convert to a string key.

		TSortMap<CString, TArray<int>> Map;

		TArray<CHexeColumnExpressionEval> Eval;
		for (int i = 0; i < m_GroupDef.GetCount(); i++)
			{
			Eval.Insert(CHexeColumnExpressionEval(m_GroupDef.GetExpression(i), m_dTable));
			}

		for (int iRow = 0; iRow < pTable->GetRowCount(); iRow++)
			{
			CString sKey = MakeGroupKey(Eval, iRow);

			TArray<int>* pGroup = Map.SetAt(sKey);
			pGroup->Insert(iRow);
			}

		TArray<TArray<int>> Index;
		for (int i = 0; i < Map.GetCount(); i++)
			Index.Insert(std::move(Map[i]));

		return CAEONTableGroupIndex(std::move(Index));
		}
	}

TArray<CDatum> CHexeTableGroupEval::GetKeysFromIndex (const CAEONTableGroupIndex& Index) const
	{
	if (m_GroupDef.IsEmpty())
		throw CException(errFail);

	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	TArray<CDatum> Keys;
	Keys.InsertEmpty(Index.GetCount());

	if (m_GroupDef.GetCount() == 1)
		{
		CHexeColumnExpressionEval Eval(m_GroupDef.GetExpression(0), m_dTable);
		for (int iGroup = 0; iGroup < Index.GetCount(); iGroup++)
			{
			int iRow = Index.GetGroupIndex(iGroup).GetAt(0);
			Keys[iGroup] = Eval.EvalRow(iRow);
			}
		}
	else
		{
		TArray<CHexeColumnExpressionEval> Eval;
		for (int i = 0; i < m_GroupDef.GetCount(); i++)
			{
			Eval.Insert(CHexeColumnExpressionEval(m_GroupDef.GetExpression(i), m_dTable));
			}

		for (int iGroup = 0; iGroup < Index.GetCount(); iGroup++)
			{
			int iRow = Index.GetGroupIndex(iGroup).GetAt(0);
			CString sKey = MakeGroupKey(Eval, iRow);
			Keys[iGroup] = CDatum(sKey);
			}
		}

	return Keys;
	}

CString CHexeTableGroupEval::MakeGroupKey (const TArray<CHexeColumnExpressionEval>& Eval, int iRow) const
	{
	CString sKey = Eval[0].EvalRow(iRow).AsString();
	for (int i = 1; i < Eval.GetCount(); i++)
		{
		sKey += STR_SEPARATOR;
		sKey += Eval[i].EvalRow(iRow).AsString();
		}

	return sKey;
	}
