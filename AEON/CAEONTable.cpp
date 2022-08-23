//	CAEONTable.cpp
//
//	CAEONTable classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_COLUMNS,						"columns");
DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ROWS,						"rows");
DECLARE_CONST_STRING(FIELD_SEQ,							"seq");
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
	OnModify();

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
	OnModify();

	return EResult::NotImplemented;
	}

IAEONTable::EResult CAEONTable::AppendEmptyRow (int iCount)

//	AppendEmptyRow
//
//	Adds an empty row.

	{
	OnModify();
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
	if (m_iKeyType == CAEONTableIndex::EType::None)
		{
		OnModify();

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

		//	If we have an index, then update it.

		if (m_pKeyIndex)
			m_pKeyIndex->Add(CDatum::raw_AsComplex(this), m_iRows - 1);

		//	Done

		return EResult::OK;
		}
	else
		{
		OnModify();

		GetIndex();
		if (!m_pKeyIndex)
			return IAEONTable::EResult::NotImplemented;

		CDatum dKey = m_pKeyIndex->GetKeyFromRow(CDatum::raw_AsComplex(this), dRow);
		return SetRowByID(dKey, dRow);
		}
	}

IAEONTable::EResult CAEONTable::AppendSlice (CDatum dSlice)

//	AppendSlice
//
//	Appends a slice of data using the current schema.

	{
	OnModify();

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

		if (m_pKeyIndex)
			m_pKeyIndex->Add(CDatum::raw_AsComplex(this), m_iRows + iRow);
		}

	m_iRows += iRows;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendTable (CDatum dTable)

//	AppendTable
//
//	Appends a table.

	{
	OnModify();

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

bool CAEONTable::CreateTableFromNil (CAEONTypeSystem& TypeSystem, CDatum& retdDatum)

//	CreateTableFromNil
//
//	Creates a nil table with no rows or columns.

	{
	CDatum dSchema = TypeSystem.AddAnonymousSchema(TArray<IDatatype::SMemberDesc>());
	CAEONTable *pNewTable = new CAEONTable(dSchema);
	retdDatum = CDatum(pNewTable);
	return true;
	}

bool CAEONTable::CreateTableFromArray (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum)

//	CreateTableFromArray
//
//	Creates a table from an array, creating a schema as necessary.

	{
	if (dValue.IsNil())
		return CreateTableFromNil(TypeSystem, retdDatum);

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
		return CreateTableFromNil(TypeSystem, retdDatum);

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
	OnModify();

	//	Recreate the columns.

	SetSchema(m_dSchema);
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::DeleteRow (int iRow)

//	DeleteRow
//
//	Deletes the given row.

	{
	if (iRow < 0 || iRow >= GetRowCount())
		return EResult::OK;

	OnModify();

	if (m_pKeyIndex)
		m_pKeyIndex->Remove(CDatum::raw_AsComplex(this), iRow);

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].DeleteElement(iRow);

	m_iRows--;

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

bool CAEONTable::FindRowByID (CDatum dValue, int *retiRow) const

//	FindRowByID
//
//	Looks for a row that matches the given ID. If found, we return true and
//	optionally return the row.

	{
	auto& Index = GetIndex();

	int iRow = Index.Find(CDatum::raw_AsComplex(this), dValue);
	if (iRow == -1)
		return false;

	if (retiRow)
		*retiRow = iRow;

	return true;
	}

CDatum CAEONTable::GetCol (CAEONTypeSystem& TypeSystem, CDatum dColName) const

//	GetCol
//
//	Returns a column. dColName can be either a column name or an array of
//	column names.

	{
	int iIndex;

	if (dColName.IsNil())
		return CDatum();
	else if (dColName.IsNumberInt32(&iIndex))
		{
		if (iIndex >= 0 && iIndex < m_Cols.GetCount())
			return m_Cols[iIndex];
		else
			return CDatum();
		}
	else if (dColName.IsContainer())
		{
		IAEONTable::SSubset Subset;
		Subset.bAllRows = true;

		if (dColName.GetCount() == 0)
			{ }
		else if (dColName.GetElement(0).IsNumberInt32())
			{
			for (int i = 0; i < dColName.GetCount(); i++)
				{
				int iCol = dColName.GetElement(i);
				if (iCol >= 0 && iCol < m_Cols.GetCount() && !Subset.Cols.Find(iCol))
					Subset.Cols.Insert(iCol);
				}
			}
		else
			{
			for (int i = 0; i < dColName.GetCount(); i++)
				{
				int iCol;
				if (FindCol(dColName.GetElement(i).AsString(), &iCol)
						&& !Subset.Cols.Find(iCol))
					Subset.Cols.Insert(iCol);
				}
			}

		//	Create the subset

		CDatum dResult;

		if (!IAEONTable::CreateRef(TypeSystem, CDatum::raw_AsComplex(this), std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}
	else if (FindCol(dColName.AsString(), &iIndex))
		return m_Cols[iIndex];
	else
		return CDatum();
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

CDatum CAEONTable::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElementAt
//
//	Handles array subscript

	{
	int iIndex;

	if (m_iKeyType != CAEONTableIndex::EType::None)
		return GetElementAt_Indexed(dIndex);
	else if (dIndex.IsNil())
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

CDatum CAEONTable::GetElementAt_Indexed (CDatum dKey) const

//	GetElementAt_Indexed
//
//	Indexed tables (tables with one or more key columns) use a different method
//	for subscript indexing.

	{
	int iIndex;

	if (m_iKeyType == CAEONTableIndex::EType::None)
		{
		return CDatum();
		}

	else if (m_iKeyType == CAEONTableIndex::EType::SingleInt32
			&& dKey.IsNumberInt32(&iIndex))
		{
		return GetElement(iIndex);
		}
	else
		{
		if (dKey.IsNil())
			return CDatum();
		else if (dKey.IsNumberInt32(&iIndex))
			return GetElement(iIndex);

		int iRow;
		if (!FindRowByID(dKey, &iRow))
			return CDatum();

		return GetElement(iRow);
		}
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

const CAEONTableIndex& CAEONTable::GetIndex () const

//	GetIndex
//
//	Returns the index.

	{
	//	If we already have an index, then return it (we assume it is updated).

	if (m_pKeyIndex)
		return *m_pKeyIndex;

	//	If we don't have primary key columns, then we don't have an index.

	if (m_iKeyType == CAEONTableIndex::EType::None)
		return CAEONTableIndex::Null;

	//	Get the primary keys and create the index.

	const IDatatype &Schema = m_dSchema;
	TArray<int> PrimaryKeys;
	if (!Schema.GetKeyMembers(PrimaryKeys))
		return CAEONTableIndex::Null;

	m_pKeyIndex.Set(new CAEONTableIndex);
	m_pKeyIndex->Init(CDatum::raw_AsComplex(this), PrimaryKeys);

	return *m_pKeyIndex;
	}

int CAEONTable::GetRowCount () const

//	GetRowCount
//
//	Returns the number of rows.

	{
	return m_iRows;
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
	OnModify();

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].GrowToFit(iCount);
	}

IAEONTable::EResult CAEONTable::InsertColumn (const CString& sName, CDatum dType, CDatum dValues, int iPos, int *retiCol)

//	InsertColumn
//
//	Inserts a new column.

	{
	OnModify();

	//	We need to create a new schema.

	CDatum dNewSchema;
	int iNewCol;
	if (!InsertColumnToSchema(m_dSchema, sName, dType, iPos, dNewSchema, &iNewCol))
		return EResult::InvalidParam;

	m_dSchema = dNewSchema;

	//	Create the column

	CDatum dNewColumn = CreateColumn(dType);
	m_Cols.Insert(dNewColumn, iNewCol);

	//	Add the values

	int iRowsLeft = GetRowCount();
	if (dValues.IsArray())
		{
		for (int i = 0; i < dValues.GetCount() && iRowsLeft > 0; i++)
			{
			dNewColumn.Append(dValues.GetElement(i));
			iRowsLeft--;
			}
		}

	while (iRowsLeft > 0)
		{
		dNewColumn.Append(CDatum());
		iRowsLeft--;
		}

	//	Done

	if (retiCol)
		*retiCol = iNewCol;

	return EResult::OK;
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

	return Schema == TestSchema;
	}

size_t CAEONTable::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Estimate of how much space is required to serialize.

	{
	return CalcMemorySize();
	}

bool CAEONTable::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Load from the given struct.

	{
	OnModify();

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

	m_Seq = dStruct.GetElement(FIELD_SEQ);

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

void CAEONTable::OnModify ()

//	OnModify
//
//	The table is about to be modified, so make a copy, if necessary.

	{
	if (m_bCopyOnWrite)
		{
		CloneContents();
		m_bCopyOnWrite = false;
		}

	//	Increment sequence so we know it got modified.

	m_Seq++;
	}

void CAEONTable::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a structure.

	{
	const IDatatype &Schema = m_dSchema;

	pStruct->SetElement(FIELD_DATATYPE, m_dSchema);
	pStruct->SetElement(FIELD_ROWS, m_iRows);
	pStruct->SetElement(FIELD_SEQ, m_Seq);

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

CDatum CAEONTable::Query (const CAEONQuery& Expr) const

//	Query
//
//	Returns the set of rows that matches the given query.

	{
	//	LATER
	return CDatum();
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

	OnModify();

	if (m_pKeyIndex)
		m_pKeyIndex->Remove(CDatum::raw_AsComplex(this), iIndex);

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dDatum.GetElement(ColumnDesc.sName);
		m_Cols[i].SetElement(iIndex, dValue);
		}

	if (m_pKeyIndex)
		m_pKeyIndex->Add(CDatum::raw_AsComplex(this), iIndex);
	}

void CAEONTable::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets a row or a column

	{
	int iIndex;

	OnModify();

	if (m_iKeyType != CAEONTableIndex::EType::None)
		SetElementAt_Indexed(dIndex, dDatum);
	else if (dIndex.IsNil())
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

		if (m_pKeyIndex)
			m_pKeyIndex = NULL;
		}
	}

void CAEONTable::SetElementAt_Indexed (CDatum dKey, CDatum dRow)

//	SetElementAt_Indexed
//
//	Set an element at the given indexed position.

	{
	int iIndex;

	if (m_iKeyType == CAEONTableIndex::EType::None)
		{ } 

	else if (m_iKeyType == CAEONTableIndex::EType::SingleInt32
			&& dKey.IsNumberInt32(&iIndex))
		{
		SetElement(iIndex, dRow);
		}
	else
		{
		SetRowByID(dKey, dRow);
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

	OnModify();

	m_Cols[iCol].SetElement(iRow, dValue);

	//	Probably most efficient to just delete the index, because we don't know
	//	how many fields are getting set.

	if (m_pKeyIndex)
		m_pKeyIndex = NULL;

	return true;
	}

IAEONTable::EResult CAEONTable::SetRow (int iRow, CDatum dRow)

//	SetRow
//
//	Sets the given row.

	{
	OnModify();

	if (m_pKeyIndex)
		m_pKeyIndex->Remove(CDatum::raw_AsComplex(this), iRow);

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

		m_Cols[i].SetElement(iRow, dValue);
		}

	if (m_pKeyIndex)
		m_pKeyIndex->Add(CDatum::raw_AsComplex(this), iRow);

	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::SetRowByID (CDatum dKey, CDatum dRow, int *retiRow)

//	SetRowByID
//
//	Sets the row by key.

	{
	if (m_iKeyType == CAEONTableIndex::EType::None)
		return IAEONTable::EResult::NotImplemented;

	OnModify();

	GetIndex();
	if (!m_pKeyIndex)
		return IAEONTable::EResult::NotImplemented;

	int iRow;
	auto iResult = m_pKeyIndex->FindOrAdd(CDatum::raw_AsComplex(this), dKey, GetRowCount(), &iRow);

	if (iResult == CAEONTableIndex::EFindResult::Added)
		AppendEmptyRow();

	//	Loop over all columns and set them appropriately.

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue;

		//	If this is a key column, then the value comes from the dKey param.

		if (ColumnDesc.iType == IDatatype::EMemberType::InstanceKeyVar)
			{
			//	If we're not adding a new row, then we can skip this because the
			//	key is already set.

			if (iResult != CAEONTableIndex::EFindResult::Added)
				continue;

			//	Get the value from the key.

			dValue = m_pKeyIndex->GetValueFromKey(CDatum::raw_AsComplex(this), dKey, ColumnDesc.sName);
			}
		else
			{
			//	Look for the value in the row. We do this manually because we
			//	need to distinguish between a missing value and a null value.

			bool bFound = false;
			for (int iEntry = 0; iEntry < dRow.GetCount(); iEntry++)
				{
				if (strEqualsNoCase(dRow.GetKey(iEntry), ColumnDesc.sName))
					{
					dValue = dRow.GetElement(iEntry);
					bFound = true;
					break;
					}
				}

			if (!bFound)
				continue;
			}

		//	Make sure we're not trying to add ourselves.

		TArray<IComplexDatum *> Checked1;
		TArray<IComplexDatum *> Checked2;
		if (dValue.Contains(CDatum::raw_AsComplex(this), Checked1) || dValue.Contains(m_Cols[i], Checked2))
			dValue = CDatum();

		//	Add it

		m_Cols[i].SetElement(iRow, dValue);
		}

	if (retiRow)
		*retiRow = iRow;

	return IAEONTable::EResult::OK;
	}

void CAEONTable::SetSchema (CDatum dSchema)

//	SetSchema
//
//	Sets the schema. This will delete all existing data in the table.

	{
	OnModify();

	//	Make sure this is an actual schema.

	const IDatatype &Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);

	//	Reset

	m_iRows = 0;
	m_dSchema = dSchema;
	m_Cols.DeleteAll();
	m_Cols.InsertEmpty(Schema.GetMemberCount());
	m_pKeyIndex = NULL;

	int iKeyCount = 0;
	bool bInt32Key = false;

	//	Set the columns (OK if no members)

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		if (!IsValidMemberType(ColumnDesc.iType)
				|| ColumnDesc.dType.GetBasicType() != CDatum::typeDatatype
				|| ColumnDesc.sName.IsEmpty())
			throw CException(errFail);

		if (ColumnDesc.iType == IDatatype::EMemberType::InstanceKeyVar)
			{
			iKeyCount++;
			if (((const IDatatype&)ColumnDesc.dType).GetCoreType() == IDatatype::INT_32)
				bInt32Key = true;
			}

		//	Create columns based on the datatype

		m_Cols[i] = CreateColumn(ColumnDesc.dType);
		}

	//	Initialize the index type.

	if (iKeyCount == 0)
		m_iKeyType = CAEONTableIndex::EType::None;
	else if (iKeyCount == 1)
		{
		if (bInt32Key)
			m_iKeyType = CAEONTableIndex::EType::SingleInt32;
		else
			m_iKeyType = CAEONTableIndex::EType::Single;
		}
	else
		m_iKeyType = CAEONTableIndex::EType::Multiple;
	}
