//	CHexeMapColumnExpressionEval.cpp
//
//	CHexeMapColumnExpressionEval class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CHexeMapColumnExpressionEval::CHexeMapColumnExpressionEval (const CAEONMapColumnExpression& ColExpr, CDatum dTable) :
			m_ColExpr(ColExpr),
			m_dTable(dTable)

//	CHexeMapColumnExpressionEval constructor

	{
	}

TArray<int> CHexeMapColumnExpressionEval::CreateColumnIndicesFromSchema (CDatum& dSchema) const

//	CreateColumnIndicesFromSchema
//
//	Maps from the column expression schema to the indices of the columns in 
//	output table schema.

	{
	TArray<int> SchemaToColMap;

	if (dSchema.GetBasicType() == CDatum::typeDatatype)
		{
		//	Get the schema in the column expression and map from one to the other.

		const IDatatype& DestSchema = dSchema;
		const IDatatype& MapSchema = m_ColExpr.GetSchema();

		SchemaToColMap.InsertEmpty(DestSchema.GetMemberCount());
		for (int i = 0; i < DestSchema.GetMemberCount(); i++)
			{
			auto Member = DestSchema.GetMember(i);
			SchemaToColMap[i] = MapSchema.FindMember(Member.sID);	//	-1 if not found
			}
		}

	//	Otherwise, get it from the column expression.

	else
		{
		dSchema = m_ColExpr.GetSchema();

		//	Mapping is 1-to-1.

		SchemaToColMap.InsertEmpty(m_ColExpr.GetColCount());
		for (int i = 0; i < m_ColExpr.GetColCount(); i++)
			SchemaToColMap[i] = i;
		}

	return SchemaToColMap;
	}

CDatum CHexeMapColumnExpressionEval::Summarize (CDatum dSchema) const

//	Summarize
//
//	Returns a table with the results of the column expression evaluation.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	If we have a schema, map it to the column indices. Otherwise, we will
	//	get back the schema implied by the column expressions.

	TArray<int> SchemaToColMap = CreateColumnIndicesFromSchema(dSchema);

	//	Get the schema type

	const IDatatype& Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);	//	Wouldn't be map col expression.

	const CAEONTableGroupDefinition& GroupDef = pTable->GetGroups();
	const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();

	int iGroupCount = (GroupDef.IsEmpty() ? 1 : GroupIndex.GetCount());
	TArray<CDatum> Columns = IAEONTable::CreateColumns(Schema, NULL, iGroupCount);

	//	If grouped, we do things differently.

	if (!GroupDef.IsEmpty())
		{
		for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
			{
			int iColExpr = SchemaToColMap[iCol];
			CDatum dColExpr = (iColExpr != -1 ? m_ColExpr.GetColExpression(iColExpr) : CDatum());

			//	If this is an expression, then we compute it.

			if (dColExpr.GetBasicType() == CDatum::typeExpression)
				{
				const CAEONExpression& ColExpr = dColExpr.AsExpression();

				for (int i = 0; i < GroupIndex.GetCount(); i++)
					{
					CHexeColumnExpressionEval Eval(ColExpr, m_dTable, &GroupIndex.GetGroupIndex(i));
					CDatum dValue = Eval.Eval();
					Columns[iCol].Append(dValue);
					}
				}

			//	Otherwise we treat it as a constant.

			else
				{
				for (int i = 0; i < GroupIndex.GetCount(); i++)
					{
					Columns[iCol].Append(dColExpr);
					}
				}
			}
		}
	else
		{
		//	Loop over the members in order and compute each column

		for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
			{
			int iColExpr = SchemaToColMap[iCol];
			CDatum dColExpr = (iColExpr != -1 ? m_ColExpr.GetColExpression(iColExpr) : CDatum());

			//	If this is an expression, then we compute it.

			if (dColExpr.GetBasicType() == CDatum::typeExpression)
				{
				const CAEONExpression& ColExpr = dColExpr.AsExpression();
				CHexeColumnExpressionEval Eval(ColExpr, m_dTable);
				CDatum dValue = Eval.Eval();
				Columns[iCol].Append(dValue);
				}

			//	Otherwise we treat it as a constant.

			else
				{
				Columns[iCol].Append(dColExpr);
				}
			}
		}

	//	Now create a table.

	return CDatum::CreateTable(dSchema, std::move(Columns));
	}
