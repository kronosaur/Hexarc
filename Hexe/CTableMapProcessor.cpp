//	CTableMapProcessor.cpp
//
//	CTableMapProcessor Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_SCHEMA,						"schema");

DECLARE_CONST_STRING(TYPENAME_TABLE_MAP_PROC,			"tableMapProcessor");
const CString &CTableMapProcessor::StaticGetTypename (void) { return TYPENAME_TABLE_MAP_PROC; }

CTableMapProcessor::CTableMapProcessor (CAEONTypeSystem &TypeSystem, CDatum dTable, CDatum dOptions, CDatum dMapFunc) :
		m_TypeSystem(TypeSystem),
		m_dTable(dTable),
		m_dOptions(dOptions),
		m_dMapFunc(dMapFunc)

//	CTableMapProcessor constructor

	{
	}

CDatum CTableMapProcessor::CreateSchemaFromStruct (CDatum dStruct)

//	CreateSchemaFromStruct
//
//	Creates a schema from the given struct.

	{
	//	We treat each field of the struct as a column.

	TArray<IDatatype::SMemberDesc> Columns;
	Columns.GrowToFit(dStruct.GetCount());
	for (int i = 0; i < dStruct.GetCount(); i++)
		{
		auto pNewColumn = Columns.Insert();
		pNewColumn->iType = IDatatype::EMemberType::InstanceVar;
		pNewColumn->sID = dStruct.GetKey(i);
		pNewColumn->dType = CAEONTypeSystem::GetCoreType(IDatatype::ANY);
		}

	//	Create a new schema.

	return m_TypeSystem.AddAnonymousSchema(Columns);
	}

void CTableMapProcessor::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dTable.Mark();
	m_dOptions.Mark();
	m_dMapFunc.Mark();

	m_dRow.Mark();
	m_dRowResult.Mark();
	m_dResult.Mark();
	}

bool CTableMapProcessor::Process (CDatum dSelf, SAEONInvokeResult& retResult)

//	Process
//
//	We return under the following conditions:
//
//	1.	If processing has completed, we return TRUE and retResult has
//		the result (mapped array).
//
//	2.	If we need to call a lambda function to continue processing, we return
//		FALSE and retResult has the proper calling parameters (ready for
//		Compute.
//
//	3.	If there is an error we return FALSE and retResult is an error string.

	{
	//	If we have a column expression then we will return an array of values.

	if (const CAEONExpression* pExpr = m_dMapFunc.GetQueryInterface())
		{
		CHexeExpressionEval Eval(*pExpr, m_dTable);
		retResult.dResult = Eval.Map();
		return true;
		}

	//	If this is a map column expression, process as such.

	else if (CAEONMapColumnExpression* pMapExpr = CAEONMapColumnExpression::Upconvert(m_dMapFunc))
		{
		return ProcessMapColExpression(*pMapExpr, retResult.dResult);
		}

	//	See if we can convert to a CAEONMapColumnExpression

	else if (CAEONMapColumnExpression* pMapExpr = CAEONMapColumnExpression::Upconvert(m_dMapFunc.AsMapColumnExpression()))
		{
		return ProcessMapColExpression(*pMapExpr, retResult.dResult);
		}

	//	If this is a structure of funcs, then we create a new table.

	else if (m_dMapFunc.GetBasicType() == CDatum::typeStruct)
		{
		//	Create a new schema based on the struct.

		CDatum dSchema = CreateSchemaFromStruct(m_dMapFunc);
		if (dSchema.IsNil())
			{
			retResult.dResult = CDatum();
			return true;
			}

		CDatum dNewTable = CDatum::CreateTable(dSchema);
		if (!m_dTable.IsContainer() || m_dTable.GetCount() == 0)
			{
			retResult.dResult = dNewTable;
			return true;
			}

		//	Initialize our state.

		m_dResult = dNewTable;
		m_dResult.GrowToFit(m_dTable.GetCount());
		m_dRowResult = CDatum(CDatum::typeStruct);
		m_iRow = 0;
		m_iCol = 0;
		m_dRow = m_dTable.GetElement(m_iRow);

		//	Keep mapping

		while (m_iRow < m_dTable.GetCount())
			{
			CDatum dColFunc = m_dMapFunc.GetElement(m_iCol);
			if (dColFunc.CanInvoke())
				return CHexe::RunFunction1Arg(dColFunc, m_dRow, dSelf, retResult);

			m_dRowResult.SetElement(m_dMapFunc.GetKey(m_iCol), dColFunc);

			NextCol();
			}

		retResult.dResult = m_dResult;
		return true;
		}

	//	Otherwise, we expect this to be a function to map the entire table row
	//	to a single value.

	else
		{
		//	See if we have a schema.

		CDatum dSchema = m_dOptions.GetElement(FIELD_SCHEMA);
		if (dSchema.IsNil())
			{
			//	If the return type of the mapping function is a schema, then use 
			//	that to construct the table.

			const IDatatype& FnType = m_dMapFunc.GetDatatype();
			CDatum dReturnType;
			if (FnType.HasMember(IDatatype::EMemberType::ReturnType, &dReturnType))
				{
				if (((const IDatatype&)dReturnType).IsNullable())
					{
					dReturnType = ((const IDatatype&)dReturnType).GetVariantType();
					}

				const IDatatype& ReturnType = dReturnType;
				if (ReturnType.GetClass() == IDatatype::ECategory::Schema)
					{
					dSchema = dReturnType;
					}
				}
			}

		//	Create the result. If we have a schema, then we create a table. 
		//	Otherwise, an array.

		if (dSchema.GetBasicType() == CDatum::typeDatatype)
			m_dResult = CDatum::CreateTable(dSchema);
		else
			m_dResult = CDatum(CDatum::typeArray);

		//	Short-circuit if we have no rows.

		if (!m_dTable.IsContainer() || m_dTable.GetCount() == 0)
			{
			retResult.dResult = m_dResult;
			return true;
			}

		//	Initialize our state.

		m_dResult.GrowToFit(m_dTable.GetCount());
		m_iRow = 0;

		//	Run the mapping function

		return CHexe::RunFunction2Args(m_dMapFunc, m_dTable.GetElement(m_iRow), CDatum(m_iRow), dSelf, retResult);
		}
	}

bool CTableMapProcessor::ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult)

//	ProcessContinues
//
//	Handle the next element.

	{
	if (m_dMapFunc.GetBasicType() == CDatum::typeStruct)
		{
		//	Add the mapped result.

		m_dRowResult.SetElement(m_dMapFunc.GetKey(m_iCol), dResult);
		NextCol();

		//	Keep looping

		while (m_iRow < m_dTable.GetCount())
			{
			CDatum dColFunc = m_dMapFunc.GetElement(m_iCol);
			if (dColFunc.CanInvoke())
				return CHexe::RunFunction1Arg(dColFunc, m_dRow, dSelf, retResult);

			m_dRowResult.SetElement(m_dMapFunc.GetKey(m_iCol), dColFunc);

			NextCol();
			}

		//	Done!

		retResult.dResult = m_dResult;
		return true;
		}
	else
		{
		//	Add the mapped result.

		if (!dResult.IsIdenticalToNil())
			m_dResult.Append(dResult);

		//	Are we done?

		m_iRow++;
		if (m_iRow >= m_dTable.GetCount())
			{
			retResult.dResult = m_dResult;
			return true;
			}

		//	Continue mapping

		return CHexe::RunFunction2Args(m_dMapFunc, m_dTable.GetElement(m_iRow), CDatum(m_iRow), dSelf, retResult);
		}
	}

bool CTableMapProcessor::ProcessMapColExpression (const CAEONMapColumnExpression& MapColExpr, CDatum& retResult)
	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	//	If we have a schema in options, use that.

	CDatum dSchema;
	TArray<int> SchemaToColMap;

	if (m_dOptions.FindElement(FIELD_SCHEMA, &dSchema))
		{
		//	Get the schema in the column expression and map from one to the other.

		const IDatatype& DestSchema = dSchema;
		const IDatatype& MapSchema = MapColExpr.GetSchema();

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
		dSchema = MapColExpr.GetSchema();

		//	Mapping is 1-to-1.

		SchemaToColMap.InsertEmpty(MapColExpr.GetColCount());
		for (int i = 0; i < MapColExpr.GetColCount(); i++)
			SchemaToColMap[i] = i;
		}

	const IDatatype& Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);	//	Wouldn't be map col expression.

	const CAEONTableGroupDefinition& GroupDef = pTable->GetGroups();

	//	If grouped, do things differently.

	TArray<CDatum> Columns = IAEONTable::CreateColumns(Schema, NULL, m_dTable.GetCount());

	if (!GroupDef.IsEmpty())
		{
		const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();

		for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
			{
			int iColExpr = SchemaToColMap[iCol];
			CDatum dColExpr = (iColExpr != -1 ? MapColExpr.GetColExpression(iColExpr) : CDatum());

			//	If this is an expression, then we compute it.

			if (dColExpr.GetBasicType() == CDatum::typeExpression)
				{
				const CAEONExpression& ColExpr = dColExpr.AsExpression();

				for (int iGroup = 0; iGroup < GroupIndex.GetCount(); iGroup++)
					{
					CHexeColumnExpressionEval Eval(ColExpr, m_dTable, &GroupIndex.GetGroupIndex(iGroup));
					Eval.AppendValues(Columns[iCol]);
					}
				}

			//	Otherwise we treat it as a constant (in which case, groups don't matter).

			else
				{
				for (int iRow = 0; iRow < m_dTable.GetCount(); iRow++)
					Columns[iCol].Append(dColExpr);
				}
			}
		}

	//	Otherwise, flat

	else
		{
		//	Loop over the members in order and compute each column

		for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
			{
			int iColExpr = SchemaToColMap[iCol];
			CDatum dColExpr = (iColExpr != -1 ? MapColExpr.GetColExpression(iColExpr) : CDatum());

			//	If this is an expression, then we compute it.

			if (dColExpr.GetBasicType() == CDatum::typeExpression)
				{
				const CAEONExpression& ColExpr = dColExpr.AsExpression();
				CHexeColumnExpressionEval Eval(ColExpr, m_dTable);
				Eval.AppendValues(Columns[iCol]);
				}

			//	Otherwise we treat it as a constant.

			else
				{
				for (int iRow = 0; iRow < m_dTable.GetCount(); iRow++)
					Columns[iCol].Append(dColExpr);
				}
			}
		}

	//	Now create a table.

	retResult = CDatum::CreateTable(dSchema, std::move(Columns));
	return true;
	}

void CTableMapProcessor::NextCol ()
	{
	m_iCol++;
	if (m_iCol >= m_dMapFunc.GetCount())
		{
		m_iCol = 0;
		m_dResult.Append(m_dRowResult);
		m_dRowResult = CDatum(CDatum::typeStruct);
		m_iRow++;
		if (m_iRow < m_dTable.GetCount())
			m_dRow = m_dTable.GetElement(m_iRow);
		}
	}
