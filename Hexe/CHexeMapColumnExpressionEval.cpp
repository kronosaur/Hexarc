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

CDatum CHexeMapColumnExpressionEval::LinearRegression (CDatum dXExpr, CDatum dYExpr, CDatum dOptions)
	{
	constexpr int COL_N = -1;
	constexpr int COL_SLOPE = -2;
	constexpr int COL_INTERCEPT = -3;
	constexpr int COL_R_SQUARED = -4;
	constexpr int COL_RMSE = -5;
	constexpr int COL_DF = -6;

	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	const CAEONExpression& XExpr = dXExpr.AsExpression();
	const CAEONExpression& YExpr = dYExpr.AsExpression();

	//	Create the schema for the result table.

	CDatum dSchema = m_ColExpr.GetSchema();

	//	If we have a schema, map it to the column indices. Otherwise, we will
	//	get back the schema implied by the column expressions.

	TArray<int> SchemaToColMap = CreateColumnIndicesFromSchema(dSchema);

	//	For columns after the group keys, we will use sentinel values to
	//	indicate which linear regression column we're applying.

	const CAEONTableGroupDefinition& GroupDef = pTable->GetGroups();
	const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();
	for (int i = 0; i < 6; i++)
		SchemaToColMap[GroupDef.GetCount() + i] = COL_N - i;

	//	Get the schema type

	const IDatatype& Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);	//	Wouldn't be map col expression.

	int iGroupCount = (GroupDef.IsEmpty() ? 1 : GroupIndex.GetCount());
	TArray<CDatum> Columns = IAEONTable::CreateColumns(Schema, NULL, iGroupCount);

	//	Compute the linear regression

	TArray<SLinearFitResult> Results;
	Results.InsertEmpty(iGroupCount);
	if (GroupDef.IsEmpty())
		{
		CHexeColumnExpressionEval XEval(XExpr, m_dTable);
		TArray<double> xValues = XEval.EvalArrayOfDouble();

		CHexeColumnExpressionEval YEval(YExpr, m_dTable);
		TArray<double> yValues = YEval.EvalArrayOfDouble();

		Results[0] = CStatistics::LinearRegression(xValues, yValues);
		}
	else
		{
		for (int i = 0; i < GroupIndex.GetCount(); i++)
			{
			CHexeColumnExpressionEval XEval(XExpr, m_dTable, &GroupIndex.GetGroupIndex(i));
			TArray<double> xValues = XEval.EvalArrayOfDouble();

			CHexeColumnExpressionEval YEval(YExpr, m_dTable, &GroupIndex.GetGroupIndex(i));
			TArray<double> yValues = YEval.EvalArrayOfDouble();

			Results[i] = CStatistics::LinearRegression(xValues, yValues);
			}
		}

	//	If grouped, we do things differently.

	if (!GroupDef.IsEmpty())
		{
		for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
			{
			int iColExpr = SchemaToColMap[iCol];

			if (iColExpr == COL_N)
				{
				for (int i = 0; i < iGroupCount; i++)
					Columns[iCol].Append(Results[i].n);
				}
			else if (iColExpr == COL_SLOPE)
				{
				for (int i = 0; i < iGroupCount; i++)
					Columns[iCol].Append(Results[i].rSlope);
				}
			else if (iColExpr == COL_INTERCEPT)
				{
				for (int i = 0; i < iGroupCount; i++)
					Columns[iCol].Append(Results[i].rIntercept);
				}
			else if (iColExpr == COL_R_SQUARED)
				{
				for (int i = 0; i < iGroupCount; i++)
					Columns[iCol].Append(Results[i].rRSquared);
				}
			else if (iColExpr == COL_RMSE)
				{
				for (int i = 0; i < iGroupCount; i++)
					Columns[iCol].Append(Results[i].rRMSE);
				}
			else if (iColExpr == COL_DF)
				{
				for (int i = 0; i < iGroupCount; i++)
					Columns[iCol].Append(Results[i].df);
				}
			else
				{
				CDatum dColExpr = (iColExpr >= 0 ? m_ColExpr.GetColExpression(iColExpr) : CDatum());

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
		}
	else
		{
		//	Loop over the members in order and compute each column

		for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
			{
			int iColExpr = SchemaToColMap[iCol];

			if (iColExpr == COL_N)
				Columns[iCol].Append((int)Results[0].n);
			else if (iColExpr == COL_SLOPE)
				Columns[iCol].Append(Results[0].rSlope);
			else if (iColExpr == COL_INTERCEPT)
				Columns[iCol].Append(Results[0].rIntercept);
			else if (iColExpr == COL_R_SQUARED)
				Columns[iCol].Append(Results[0].rRSquared);
			else if (iColExpr == COL_RMSE)
				Columns[iCol].Append(Results[0].rRMSE);
			else if (iColExpr == COL_DF)
				Columns[iCol].Append((int)Results[0].df);
			else
				{
				CDatum dColExpr = (iColExpr >= 0 ? m_ColExpr.GetColExpression(iColExpr) : CDatum());

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
		}

	//	Now create a table.

	return CDatum::CreateTable(dSchema, std::move(Columns));
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
