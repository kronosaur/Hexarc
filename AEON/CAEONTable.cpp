//	CAEONTable.cpp
//
//	CAEONTable classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_COLUMNS,						"columns");
DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ROWS,						"rows");
DECLARE_CONST_STRING(FIELD_VALUES,						"values");

DECLARE_CONST_STRING(TYPENAME_TABLE,					"table");

DECLARE_CONST_STRING(ERR_INVALID_TABLE_SCHEMA,			"Invalid table schema.");
DECLARE_CONST_STRING(ERR_INVALID_TABLE_ARRAY,			"First element in array must be a structure.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_SCHEMA,		"Unable to create schema.");

TDatumPropertyHandler<CAEONTable> CAEONTable::m_Properties = {
	{
		"columns",
		"Returns an array of all column IDs.",
		[](const CAEONTable &Obj, const CString &sProperty)
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
		[](const CAEONTable &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetRowCount());
			},
		NULL,
		},
	};

void CAEONTable::Append (CDatum dDatum)

//	Append
//
//	Append a row, column, or table.

	{
	OnCopyOnWrite();

	switch (dDatum.GetBasicType())
		{
		case CDatum::typeArray:
			AppendColumn(dDatum);
			break;

		case CDatum::typeStruct:
			AppendRow(dDatum);
			break;

		case CDatum::typeTable:
			AppendTable(dDatum);
			break;

		default:
			break;
		}
	}

IAEONTable::EResult CAEONTable::AppendColumn (CDatum dColumn)

//	AppendColumn
//
//	Appends a column.

	{
	OnCopyOnWrite();

	return EResult::NotImplemented;
	}

IAEONTable::EResult CAEONTable::AppendEmptyRow (int iCount)

//	AppendEmptyRow
//
//	Adds an empty row.

	{
	OnCopyOnWrite();
	GrowToFit(iCount);

	for (int i = 0; i < m_Cols.GetCount(); i++)
		{
		for (int j = 0; j < iCount; j++)
			m_Cols[i].Append(CDatum());
		}

	m_iRows += iCount;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendRow (CDatum dRow)

//	AppendRow
//
//	Appends a row.

	{
	OnCopyOnWrite();

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

		m_Cols[i].Append(dValue);
		}

	m_iRows++;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendSlice (CDatum dSlice)

//	AppendSlice
//
//	Appends a slice of data using the current schema.

	{
	OnCopyOnWrite();

	if (m_Cols.GetCount() == 0 || dSlice.GetCount() == 0)
		return EResult::OK;

	int iRows = dSlice.GetCount() / m_Cols.GetCount();
	for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
		m_Cols[iCol].GrowToFit(iRows);

	int i = 0;
	for (int iRow = 0; iRow < iRows; iRow++)
		{
		for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
			{
			m_Cols[iCol].Append(dSlice.GetElement(i));

			i++;
			}
		}

	m_iRows += iRows;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendTable (CDatum dTable)

//	AppendTable
//
//	Appends a table.

	{
	OnCopyOnWrite();

	GrowToFit(dTable.GetCount());

	for (int i = 0; i < dTable.GetCount(); i++)
		{
		AppendRow(dTable.GetElement(i));
		}

	return EResult::OK;
	}

CString CAEONTable::AsString (void) const

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

	for (int iRow = 0; iRow < m_iRows; iRow++)
		{
		for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
			{
			if (iCol != 0)
				Buffer.WriteChar('\t');

			Buffer.Write(m_Cols[iCol].GetElement(iRow).AsString());
			}

		Buffer.WriteChar('\n');
		}

	return CString(Buffer);
	}

CDatum CAEONTable::CalcColumnDatatype (CDatum dValue)

//	CalcColumnDatatype
//
//	Computes the column datatype based on a value.

	{
	if (dValue.IsContainer())
		{
		const IDatatype &ArrayType = dValue.GetDatatype();

		switch (ArrayType.GetCoreType())
			{
			case IDatatype::ARRAY_INT_32:
				return CAEONTypeSystem::GetCoreType(IDatatype::INT_32);

			case IDatatype::ARRAY_INT_IP:
				return CAEONTypeSystem::GetCoreType(IDatatype::INT_IP);

			case IDatatype::ARRAY_FLOAT_64:
				return CAEONTypeSystem::GetCoreType(IDatatype::FLOAT_64);

			case IDatatype::ARRAY_STRING:
				return CAEONTypeSystem::GetCoreType(IDatatype::STRING);

			//	For everything else, see if we can compute an element type based
			//	on the values.

			default:
				{
				if (dValue.GetCount() == 0)
					return CAEONTypeSystem::GetCoreType(IDatatype::ANY);
				else
					{
					CDatum::Types iElementType = dValue.GetElement(0).GetBasicType();
					if (iElementType != CDatum::typeInteger32
							&& iElementType != CDatum::typeIntegerIP
							&& iElementType != CDatum::typeDouble
							&& iElementType != CDatum::typeString)
						return CAEONTypeSystem::GetCoreType(IDatatype::ANY);

					for (int i = 1; i < dValue.GetCount(); i++)
						{
						if (dValue.GetElement(i).GetBasicType() != iElementType)
							return CAEONTypeSystem::GetCoreType(IDatatype::ANY);
						}

					switch (iElementType)
						{
						case CDatum::typeInteger32:
							return CAEONTypeSystem::GetCoreType(IDatatype::INT_32);

						case CDatum::typeIntegerIP:
							return CAEONTypeSystem::GetCoreType(IDatatype::INT_IP);

						case CDatum::typeDouble:
							return CAEONTypeSystem::GetCoreType(IDatatype::FLOAT_64);

						case CDatum::typeString:
							return CAEONTypeSystem::GetCoreType(IDatatype::STRING);

						default:
							return CAEONTypeSystem::GetCoreType(IDatatype::ANY);
						}
					}
				}
			}
		}
	else
		{
		return dValue.GetDatatype();
		}
	}

size_t CAEONTable::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Returns number of bytes used.

	{
	size_t iSize = 0;

	for (int i = 0; i < m_Cols.GetCount(); i++)
		iSize += m_Cols[i].CalcMemorySize();

	return iSize;
	}

IComplexDatum *CAEONTable::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clones the table.

	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CAEONTable(*this);

		case CDatum::EClone::CopyOnWrite:
			{
			auto pClone = new CAEONTable(*this);
			pClone->m_bCopyOnWrite = true;
			return pClone;
			}

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CAEONTable(*this);
			pClone->CloneContents();
			return pClone;
			}

		default:
			throw CException(errFail);
		}
	}

void CAEONTable::CloneContents ()

//	CloneContents
//
//	Clones all content so that it is a copy.

	{
	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i] = m_Cols[i].Clone(CDatum::EClone::DeepCopy);
	}

bool CAEONTable::CreateTableFromArray (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum)

//	CreateTableFromArray
//
//	Creates a table from an array, creating a schema as necessary.

	{
	if (dValue.IsNil())
		{
		retdDatum = CDatum();
		return true;
		}

	//	The first row is special because we use it to compose the schema.

	CDatum dSchema;
	CDatum dFirstRow = dValue.GetElement(0);
	int iStartRow = 0;
	if (dFirstRow.GetBasicType() == CDatum::typeStruct)
		{
		TArray<IDatatype::SMemberDesc> Columns;
		Columns.GrowToFit(dFirstRow.GetCount());
		for (int i = 0; i < dFirstRow.GetCount(); i++)
			{
			auto pNewColumn = Columns.Insert();
			pNewColumn->iType = IDatatype::EMemberType::InstanceVar;
			pNewColumn->sName = dFirstRow.GetKey(i);
			pNewColumn->dType = dFirstRow.GetElement(i).GetDatatype();
			}

		dSchema = TypeSystem.AddAnonymousSchema(Columns);
		if (dSchema.IsNil())
			{
			retdDatum = ERR_UNABLE_TO_CREATE_SCHEMA;
			return false;
			}
		}
	else
		{
		retdDatum = ERR_INVALID_TABLE_ARRAY;
		return false;
		}

	//	Create the table.

	CAEONTable *pNewTable = new CAEONTable(dSchema);
	retdDatum = CDatum(pNewTable);
	pNewTable->GrowToFit(dValue.GetCount() - iStartRow);

	//	Now load everything else.

	for (int i = iStartRow; i < dValue.GetCount(); i++)
		{
		pNewTable->AppendRow(dValue.GetElement(i));
		}

	return true;
	}

bool CAEONTable::CreateTableFromDatatype (CAEONTypeSystem &TypeSystem, CDatum dType, CDatum &retdDatum)

//	CreateTableFromDatatype
//
//	Creates an empty table from a datatype, creating a schema if necessary.

	{
	const IDatatype &Type = dType;

	//	If this is a schema datatype, then we're done.

	if (Type.GetClass() == IDatatype::ECategory::Schema)
		{
		retdDatum = CDatum::CreateTable(dType);
		return true;
		}

	//	Otherwise, nothing.

	else
		{
		retdDatum = ERR_INVALID_TABLE_SCHEMA;
		return false;
		}
	}

bool CAEONTable::CreateTableFromStruct (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum)

//	CreateTableFromStruct
//
//	Creates a table from a structure, generating a schema, if necessary.

	{
	int iRows = 0;
	
	if (dValue.IsNil())
		{
		retdDatum = CDatum();
		return true;
		}

	//	We treat each field of the struct as a column.

	TArray<IDatatype::SMemberDesc> Columns;
	Columns.GrowToFit(dValue.GetCount());
	for (int i = 0; i < dValue.GetCount(); i++)
		{
		CDatum dColumnValues = dValue.GetElement(i);

		auto pNewColumn = Columns.Insert();
		pNewColumn->iType = IDatatype::EMemberType::InstanceVar;
		pNewColumn->sName = dValue.GetKey(i);
		pNewColumn->dType = CalcColumnDatatype(dColumnValues);

		if (dColumnValues.GetBasicType() == CDatum::typeArray)
			iRows = Max(iRows, dColumnValues.GetCount());
		else
			iRows = Max(iRows, 1);
		}

	//	Create a new schema.

	CDatum dSchema = TypeSystem.AddAnonymousSchema(Columns);
	if (dSchema.IsNil())
		{
		retdDatum = ERR_UNABLE_TO_CREATE_SCHEMA;
		return false;
		}

	//	Create the table.

	CAEONTable *pNewTable = new CAEONTable(dSchema);
	pNewTable->GrowToFit(iRows);
	pNewTable->m_iRows = iRows;

	for (int i = 0; i < dValue.GetCount(); i++)
		{
		CDatum dColumnValues = dValue.GetElement(i);
		int iValueCount = dColumnValues.GetCount();
		if (iValueCount == 0)
			{
			for (int iRow = 0; iRow < iRows; iRow++)
				{
				pNewTable->m_Cols[i].Append(CDatum());
				}
			}
		else
			{
			for (int iRow = 0; iRow < iRows; iRow++)
				{
				pNewTable->m_Cols[i].Append(dColumnValues.GetElement(iRow % iValueCount));
				}
			}
		}

	retdDatum = CDatum(pNewTable);
	return true;
	}

IAEONTable::EResult CAEONTable::DeleteAllRows ()

//	DeleteAllRows
//
//	Deletes all rows (but leaves the schema intact).

	{
	OnCopyOnWrite();

	//	Recreate the columns.

	SetSchema(m_dSchema);
	return EResult::OK;
	}

bool CAEONTable::FindCol (const CString &sName, int *retiCol) const

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

int CAEONTable::GetColCount () const

//	GetColCount
//
//	Returns the number of column.

	{
	return m_Cols.GetCount();
	}

CString CAEONTable::GetColName (int iCol) const

//	GetColName
//
//	Returns the name of the column.

	{
	const IDatatype &Schema = m_dSchema;
	if (iCol < 0 || iCol >= Schema.GetMemberCount())
		throw CException(errFail);

	return Schema.GetMember(iCol).sName;
	}

CDatum CAEONTable::GetDataSlice (int iFirstRow, int iRowCount) const

//	GetDataSlice
//
//	Returns a 2D array of values.

	{
	if (iFirstRow < 0 || iFirstRow >= m_iRows || GetColCount() == 0)
		return CDatum();

	int iLastRow = Min(iFirstRow + iRowCount, m_iRows);

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetColCount() * (iLastRow - iFirstRow));

	for (int iRow = iFirstRow; iRow < iLastRow; iRow++)
		{
		for (int iCol = 0; iCol < GetColCount(); iCol++)
			{
			dResult.Append(m_Cols[iCol].GetElement(iRow));
			}
		}

	return dResult;
	}

CDatum CAEONTable::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElementAt
//
//	Handles array subscript

	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		return GetElement(iIndex);
	else if (dIndex.IsContainer())
		{
		IAEONTable::SSubset Subset;

		if (dIndex.GetCount() == 0)
			{ }
		else if (dIndex.GetElement(0).IsNumberInt32())
			{
			//	NOTE: We allow rows to be out of order or even duplicated.
			//	But the row must exist.

			for (int i = 0; i < dIndex.GetCount(); i++)
				{
				if (dIndex.GetElement(i).IsNumberInt32(&iIndex) && iIndex >= 0 && iIndex < GetRowCount())
					Subset.Rows.Insert(iIndex);
				}
			}
		else
			{
			Subset.bAllRows = true;

			for (int i = 0; i < dIndex.GetCount(); i++)
				{
				int iCol;
				if (FindCol(dIndex.GetElement(i).AsString(), &iCol))
					Subset.Cols.Insert(iCol);
				}
			}

		//	Create the subset

		CDatum dResult;

		if (!IAEONTable::CreateRef(TypeSystem, CDatum::raw_AsComplex(this), std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}
	else if (FindCol(dIndex.AsString(), &iIndex))
		return m_Cols[iIndex];
	else
		return CDatum();
	}

CDatum CAEONTable::GetFieldValue (int iRow, int iCol) const

//	GetFieldValue
//
//	Returns the given field.

	{
	if (iRow < 0 || iRow >= m_iRows)
		return CDatum();

	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return CDatum();

	return m_Cols[iCol].GetElement(iRow);
	}

int CAEONTable::GetRowCount () const

//	GetRowCount
//
//	Returns the number of rows.

	{
	return m_iRows;
	}

CDatum CAEONTable::GetElement (int iIndex) const

//	GetElement
//
//	Return the nth row of the table (as a struct).

	{
	if (iIndex < 0 || iIndex >= m_iRows)
		return CDatum();

	CDatum dRow(CDatum::typeStruct);

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		dRow.SetElement(ColumnDesc.sName, m_Cols[i].GetElement(iIndex));
		}

	return dRow;
	}

const CString &CAEONTable::GetTypename (void) const

//	GetTypename
//
//	Returns the typename.

	{
	return TYPENAME_TABLE;
	}

void CAEONTable::GrowToFit (int iCount)

//	GrowToFit
//
//	Allocate to fit an additional n rows.

	{
	OnCopyOnWrite();

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].GrowToFit(iCount);
	}

bool CAEONTable::IsSameSchema (CDatum dSchema) const

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

size_t CAEONTable::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Estimate of how much space is required to serialize.

	{
	return CalcMemorySize();
	}

void CAEONTable::OnCopyOnWrite ()

//	OnCopyOnWrite
//
//	The table is about to be modified, so make a copy, if necessary.

	{
	if (m_bCopyOnWrite)
		{
		CloneContents();
		m_bCopyOnWrite = false;
		}
	}

bool CAEONTable::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Load from the given struct.

	{
	OnCopyOnWrite();

	CDatum dSchema = dStruct.GetElement(FIELD_DATATYPE);
	const IDatatype &Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		return false;

	SetSchema(dSchema);

	CDatum dCols = dStruct.GetElement(FIELD_COLUMNS);

	m_iRows = -1;
	for (int i = 0; i < m_Cols.GetCount(); i++)
		{
		CDatum dColDef = dCols.GetElement(i);
		if (dColDef.IsNil())
			continue;

		CDatum dValues = dColDef.GetElement(FIELD_VALUES);

		if (m_iRows == -1)
			m_iRows = dValues.GetCount();
		else
			m_iRows = Min(m_iRows, dValues.GetCount());

		m_Cols[i].GrowToFit(dValues.GetCount());
		for (int j = 0; j < dValues.GetCount(); j++)
			m_Cols[i].Append(dValues.GetElement(j));
		}

	if (m_iRows == -1)
		m_iRows = 0;

	return true;
	}

void CAEONTable::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dSchema.Mark();

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].Mark();
	}

void CAEONTable::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a structure.

	{
	const IDatatype &Schema = m_dSchema;

	pStruct->SetElement(FIELD_DATATYPE, m_dSchema);
	pStruct->SetElement(FIELD_ROWS, m_iRows);

	CDatum dCols(CDatum::typeArray);
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		CDatum dColDef(CDatum::CDatum::typeStruct);
		auto ColumnDef = Schema.GetMember(i);
		
		dColDef.SetElement(FIELD_NAME, ColumnDef.sName);
		if (i < m_Cols.GetCount())
			dColDef.SetElement(FIELD_VALUES, m_Cols[i]);

		dCols.Append(dColDef);
		}

	pStruct->SetElement(FIELD_COLUMNS, dCols);
	}

void CAEONTable::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResovleDatatypes
//
//	Resolve datatypes.

	{
	//	If this schema is already registered with the type system, then use 
	//	its copy. This is helpful when we've serialized/deserialized a table.

	m_dSchema = TypeSystem.ResolveType(m_dSchema);

	//	Now resolve all columns.

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].ResolveDatatypes(TypeSystem);
	}

void CAEONTable::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets the nth record with the given struct data.

	{
	if (iIndex < 0 || iIndex >= m_iRows)
		return;

	OnCopyOnWrite();

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dDatum.GetElement(ColumnDesc.sName);
		m_Cols[i].SetElement(iIndex, dValue);
		}
	}

void CAEONTable::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets a row or a column

	{
	int iIndex;

	OnCopyOnWrite();

	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32(&iIndex))
		SetElement(iIndex, dDatum);
	else if (FindCol(dIndex.AsString(), &iIndex))
		{
		if (dDatum.GetCount() <= m_iRows)
			{
			for (int i = 0; i < dDatum.GetCount(); i++)
				m_Cols[iIndex].SetElement(i, dDatum.GetElement(i));

			for (int i = dDatum.GetCount(); i < m_iRows; i++)
				m_Cols[iIndex].SetElement(i, CDatum());
			}
		else
			{
			int iExtraRows = dDatum.GetCount() - m_iRows;

			for (int i = 0; i < m_iRows; i++)
				m_Cols[iIndex].SetElement(i, dDatum.GetElement(i));

			m_Cols[iIndex].GrowToFit(iExtraRows);
			for (int i = m_iRows; i < dDatum.GetCount(); i++)
				m_Cols[iIndex].Append(dDatum.GetElement(i));

			for (int i = 0; i < m_Cols.GetCount(); i++)
				{
				if (i == iIndex)
					continue;

				m_Cols[i].GrowToFit(iExtraRows);
				for (int j = 0; j < iExtraRows; j++)
					m_Cols[i].Append(CDatum());
				}

			m_iRows += iExtraRows;
			}
		}
	}

bool CAEONTable::SetFieldValue (int iRow, int iCol, CDatum dValue)

//	SetFieldValue
//
//	Sets the value.

	{
	if (iRow < 0 || iRow >= m_iRows)
		return false;

	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return false;

	OnCopyOnWrite();

	m_Cols[iCol].SetElement(iRow, dValue);
	return true;
	}

void CAEONTable::SetSchema (CDatum dSchema)

//	SetSchema
//
//	Sets the schema. This will delete all existing data in the table.

	{
	OnCopyOnWrite();

	//	Make sure this is an actual schema.

	const IDatatype &Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);

	//	Reset

	m_iRows = 0;
	m_dSchema = dSchema;
	m_Cols.DeleteAll();
	m_Cols.InsertEmpty(Schema.GetMemberCount());

	//	Set the columns (OK if no members)

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		if (ColumnDesc.iType != IDatatype::EMemberType::InstanceVar
				|| ColumnDesc.dType.GetBasicType() != CDatum::typeDatatype
				|| ColumnDesc.sName.IsEmpty())
			throw CException(errFail);

		//	Create columns based on the datatype

		const IDatatype &ColSchema = ColumnDesc.dType;
		switch (ColSchema.GetCoreType())
			{
			case IDatatype::INT_32:
				m_Cols[i] = CDatum::VectorOf(CDatum::typeInteger32);
				break;

			case IDatatype::INT_IP:
				m_Cols[i] = CDatum::VectorOf(CDatum::typeIntegerIP);
				break;

			case IDatatype::FLOAT_64:
				m_Cols[i] = CDatum::VectorOf(CDatum::typeDouble);
				break;

			case IDatatype::STRING:
				m_Cols[i] = CDatum::VectorOf(CDatum::typeString);
				break;

			default:
				//	For anything else, we create a generic array.

				m_Cols[i] = CDatum::VectorOf(CDatum::typeUnknown);
				break;
			}
		}
	}
