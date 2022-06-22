//	CAEONTableRef.cpp
//
//	CAEONTableRef classes
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_COLUMNS,						"columns");
DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ROWS,						"rows");
DECLARE_CONST_STRING(FIELD_VALUES,						"values");

DECLARE_CONST_STRING(TYPENAME_TABLE_REF,				"tableRef");

DECLARE_CONST_STRING(ERR_NOT_A_TABLE,					"Not a table.");

TDatumPropertyHandler<CAEONTableRef> CAEONTableRef::m_Properties = {
	{
		"columns",
		"Returns an array of all column IDs.",
		[](const CAEONTableRef &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);

			const IDatatype &Schema = Obj.m_dSchema;
			for (int i = 0; i < Schema.GetMemberCount(); i++)
				{
				auto ColumnDesc = Schema.GetMember(i);

				dResult.Append(ColumnDesc.sName);
				}

			return dResult;
			},
		NULL,
		},
	{
		"length",
		"Returns the number of rows in the table.",
		[](const CAEONTableRef &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetRowCount());
			},
		NULL,
		},
	};

CString CAEONTableRef::AsString (void) const

//	AsString
//
//	Represent as a string

	{
	CStringBuffer Buffer;

	//	Output header

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		if (i != 0)
			Buffer.WriteChar('\t');

		Buffer.Write(ColumnDesc.sName);
		}

	Buffer.WriteChar('\n');

	//	Output each value

	for (int iRow = 0; iRow < GetCount(); iRow++)
		{
		for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
			{
			if (iCol != 0)
				Buffer.WriteChar('\t');

			Buffer.Write(GetFieldValue(iRow, iCol).AsString());
			}

		Buffer.WriteChar('\n');
		}

	return CString(Buffer);
	}

size_t CAEONTableRef::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Returns number of bytes used.

	{
	//	LATER: Compute as if we serialized the table.
	return 0;
	}

bool CAEONTableRef::Create (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue)

//	Create
//
//	Creates a new table reference.

	{
	if (dTable.GetBasicType() != CDatum::typeTable)
		{
		retdValue = ERR_NOT_A_TABLE;
		return false;
		}

	CDatum dSchema;
	if (!CreateSchema(TypeSystem, dTable, Subset, dSchema))
		{
		retdValue = dSchema;
		return false;
		}

	CAEONTableRef *pNewTable = new CAEONTableRef;
	pNewTable->m_dTable = dTable;
	pNewTable->m_dSchema = dSchema;
	pNewTable->m_Cols = std::move(Subset.Cols);

	if (Subset.bAllRows)
		pNewTable->m_bAllRows = true;
	else
		pNewTable->m_Rows = std::move(Subset.Rows);

	retdValue = CDatum(pNewTable);
	return true;
	}

bool CAEONTableRef::FindCol (const CString &sName, int *retiCol) const

//	FindCol
//
//	Find column.

	{
	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		if (strEqualsNoCase(sName, Schema.GetMember(i).sName))
			{
			if (retiCol)
				*retiCol = i;

			return true;
			}
		}

	return false;
	}

CString CAEONTableRef::GetColName (int iCol) const

//	GetColName
//
//	Returns the name of the column.

	{
	const IDatatype &Schema = m_dSchema;
	if (iCol < 0 || iCol >= Schema.GetMemberCount())
		throw CException(errFail);

	return Schema.GetMember(iCol).sName;
	}

CDatum CAEONTableRef::GetColRef (int iCol) const

//	GetColRef
//
//	Create a column reference.

	{
	//	LATER: Eventually we return a column reference.
	//	FOr now we just create an array.

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetRowCount());

	for (int i = 0; i < GetRowCount(); i++)
		dResult.Append(GetFieldValue(i, iCol));

	return dResult;
	}

CDatum CAEONTableRef::GetDataSlice (int iFirstRow, int iRowCount) const

//	GetDataSlice
//
//	Returns a 2D array of values.

	{
	int iRows = GetCount();

	if (iFirstRow < 0 || iFirstRow >= iRows || GetColCount() == 0)
		return CDatum();

	int iLastRow = Min(iFirstRow + iRowCount, iRows);

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetColCount() * (iLastRow - iFirstRow));

	for (int iRow = iFirstRow; iRow < iLastRow; iRow++)
		{
		for (int iCol = 0; iCol < GetColCount(); iCol++)
			{
			dResult.Append(GetFieldValue(iRow, iCol));
			}
		}

	return dResult;
	}

CDatum CAEONTableRef::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElementAt
//
//	Handles array subscript

	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		return GetElement(iIndex);
	else if (FindCol(dIndex.AsString(), &iIndex))
		return GetColRef(iIndex);
	else
		return CDatum();
	}

CDatum CAEONTableRef::GetFieldValue (int iRow, int iCol) const

//	GetFieldValue
//
//	Returns the given field.

	{
	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return CDatum();

	IAEONTable *pTable = const_cast<CDatum&>(m_dTable).GetTableInterface();
	if (!pTable)
		return CDatum();

	if (m_bAllRows)
		{
		return pTable->GetFieldValue(iRow, m_Cols[iCol]);
		}
	else
		{
		if (iRow < 0 || iRow >= m_Rows.GetCount())
			return CDatum();

		return pTable->GetFieldValue(m_Rows[iRow], m_Cols[iCol]);
		}
	}

int CAEONTableRef::GetRowCount () const

//	GetRowCount
//
//	Returns the number of rows.

	{
	if (m_bAllRows)
		return m_dTable.GetCount();
	else
		return m_Rows.GetCount();
	}

CDatum CAEONTableRef::GetElement (int iIndex) const

//	GetElement
//
//	Return the nth row of the table (as a struct).

	{
	if (iIndex < 0 || iIndex >= GetRowCount())
		return CDatum();

	CDatum dRow(CDatum::typeStruct);

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		dRow.SetElement(ColumnDesc.sName, GetFieldValue(iIndex, i));
		}

	return dRow;
	}

const CString &CAEONTableRef::GetTypename (void) const

//	GetTypename
//
//	Returns the typename.

	{
	return TYPENAME_TABLE_REF;
	}

bool CAEONTableRef::IsSameSchema (CDatum dSchema) const

//	IsSameSchema
//
//	Returns TRUE if the given schema has the exact same columns.

	{
	const IDatatype &Schema = m_dSchema;
	const IDatatype &TestSchema = dSchema;
	if (TestSchema.GetClass() != IDatatype::ECategory::Schema)
		return false;

	if (TestSchema.GetMemberCount() != Schema.GetMemberCount())
		return false;

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		IDatatype::SMemberDesc Member = Schema.GetMember(i);

		CDatum dTestType;
		if (TestSchema.HasMember(Member.sName, &dTestType) != IDatatype::EMemberType::InstanceVar)
			return false;

		if (!Member.dType.IsEqual(dTestType))
			return false;
		}

	return true;
	}

size_t CAEONTableRef::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Estimate of how much space is required to serialize.

	{
	return CalcMemorySize();
	}

bool CAEONTableRef::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Load from the given struct.

	{
	//	Should never happen because we serialize as a table.

	throw CException(errFail);
	}

void CAEONTableRef::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dTable.Mark();
	m_dSchema.Mark();
	}

void CAEONTableRef::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a structure.
//
//	NOTE: We serialize as a table and on deserialize, we turn into a real
//	table.

	{
	const IDatatype &Schema = m_dSchema;

	pStruct->SetElement(FIELD_DATATYPE, m_dSchema);
	pStruct->SetElement(FIELD_ROWS, GetRowCount());

	CDatum dCols(CDatum::typeArray);
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		CDatum dColDef(CDatum::CDatum::typeStruct);
		auto ColumnDef = Schema.GetMember(i);
		
		dColDef.SetElement(FIELD_NAME, ColumnDef.sName);
		if (i < m_Cols.GetCount())
			{
			dColDef.SetElement(FIELD_VALUES, GetColRef(i));
			}

		dCols.Append(dColDef);
		}

	pStruct->SetElement(FIELD_COLUMNS, dCols);
	}

void CAEONTableRef::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResovleDatatypes
//
//	Resolve datatypes.

	{
	//	If this schema is already registered with the type system, then use 
	//	its copy. This is helpful when we've serialized/deserialized a table.

	m_dSchema = TypeSystem.ResolveType(m_dSchema);
	}

void CAEONTableRef::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets the nth record with the given struct data.

	{
	if (iIndex < 0 || iIndex >= GetRowCount())
		return;

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dDatum.GetElement(ColumnDesc.sName);
		SetFieldValue(iIndex, i, dValue);
		}
	}

void CAEONTableRef::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets a row or a column

	{
	int iIndex;

	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32(&iIndex))
		SetElement(iIndex, dDatum);
	else if (FindCol(dIndex.AsString(), &iIndex))
		{
		if (dDatum.GetCount() <= GetRowCount())
			{
			for (int i = 0; i < dDatum.GetCount(); i++)
				SetFieldValue(i, iIndex, dDatum.GetElement(i));

			for (int i = dDatum.GetCount(); i < GetRowCount(); i++)
				SetFieldValue(i, iIndex, CDatum());
			}
		else
			{
			int iCurRowCount = GetRowCount();
			int iExtraRows = dDatum.GetCount() - iCurRowCount;

			for (int i = 0; i < iCurRowCount; i++)
				SetFieldValue(i, iIndex, dDatum.GetElement(i));

			CString sColName = GetColName(iIndex);

			//	Add extra rows to the underlying table.

			int iFirstNewRow = m_dTable.GetCount();
			for (int i = 0; i < iExtraRows; i++)
				{
				CDatum dRow(CDatum::typeStruct);
				dRow.SetElement(sColName, dDatum.GetElement(iCurRowCount + i));
				}

			//	Now add reference rows.

			if (!m_bAllRows)
				{
				m_Rows.GrowToFit(iExtraRows);
				for (int i = 0; i < iExtraRows; i++)
					m_Rows.Insert(iFirstNewRow + i);
				}
			}
		}
	}

bool CAEONTableRef::SetFieldValue (int iRow, int iCol, CDatum dValue)

//	SetFieldValue
//
//	Sets the field value.

	{
	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return false;

	IAEONTable *pTable = const_cast<CDatum&>(m_dTable).GetTableInterface();
	if (!pTable)
		return false;

	if (m_bAllRows)
		{
		return pTable->SetFieldValue(iRow, m_Cols[iCol], dValue);
		}
	else
		{
		if (iRow < 0 || iRow >= m_Rows.GetCount())
			return false;

		return pTable->SetFieldValue(m_Rows[iRow], m_Cols[iCol], dValue);
		}
	}

IAEONTable::EResult CAEONTableRef::SetRow (int iRow, CDatum dRow)

//	SetRow
//
//	Sets the given row.

	{
	IAEONTable *pTable = const_cast<CDatum&>(m_dTable).GetTableInterface();
	if (!pTable)
		return EResult::NotATable;

	int iRowActual;
	if (m_bAllRows)
		iRowActual = iRow;
	else
		{
		if (iRow < 0 || iRow >= m_Rows.GetCount())
			return EResult::InvalidParam;

		iRowActual = m_Rows[iRow];
		}

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dRow.GetElement(ColumnDesc.sName);

		//	Make sure we're not trying to add ourselves.

		TArray<IComplexDatum *> Checked1;
		TArray<IComplexDatum *> Checked2;
		if (dValue.Contains(CDatum::raw_AsComplex(this), Checked1) || dValue.Contains(m_Cols[i], Checked2))
			dValue = CDatum();

		//	Add it

		pTable->SetFieldValue(iRowActual, m_Cols[i], dValue);
		}

	return EResult::OK;
	}

