//	CAEONTable.cpp
//
//	CAEONTable class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

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
		"$ArrayOfString",
		"Returns an array of all column IDs.",
		[](const CAEONTable &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);

			const IDatatype &Schema = Obj.GetSchema();
			for (int i = 0; i < Schema.GetMemberCount(); i++)
				{
				auto ColumnDesc = Schema.GetMember(i);

				dResult.Append(ColumnDesc.sID);
				}

			return dResult;
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the type of the table.",
		[](const CAEONTable& Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the schema type of the table.",
		[](const CAEONTable& Obj, const CString &sProperty)
			{
			return ((const IDatatype&)Obj.GetDatatype()).GetElementType();
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the type of the table key.",
		[](const CAEONTable& Obj, const CString &sProperty)
			{
			return ((const IDatatype&)Obj.GetDatatype()).GetKeyType();
			},
		NULL,
		},
	{
		"keys",
		"a",
		"Returns an array of keys.",
		[](const CAEONTable &Obj, const CString &sProperty)
			{
			auto pKeyIndex = Obj.GetIndex();
			if (!pKeyIndex)
				return CDatum();

			return pKeyIndex->GetKeyArray(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of rows in the table.",
		[](const CAEONTable &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetRowCount());
			},
		NULL,
		},
	{
		"nextID",
		"i",
		"The nextID to use for the table.",
		[](const CAEONTable &Obj, const CString &sProperty)
			{
			//	NOTE: We preincrement in MakeID, so we need to add 1 here.
			return CDatum(Obj.m_NextID + 1);
			},
		[](CAEONTable &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_NextID = ((DWORDLONG)dValue) - 1;
			return true;
			},
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONTable::m_pMethodsExt = NULL;

CAEONTable::CAEONTable (CDatum dSchema, TArray<CDatum>&& Cols)

//	CAEONTable constructor

	{
	CDatum dDatatype;

	const IDatatype& InputDatatype = dSchema;
	if (InputDatatype.GetClass() == IDatatype::ECategory::Table)
		dDatatype = dSchema;
	else if (InputDatatype.GetClass() == IDatatype::ECategory::Schema)
		dDatatype = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dSchema);
	else
		throw CException(errFail);

	const IDatatype& Datatype = dDatatype;
	const IDatatype& Schema = Datatype.GetElementType();
	if (Schema.GetMemberCount () != Cols.GetCount())
		throw CException(errFail);

	//	Create

	SetSchema(dDatatype, true);
	if (Cols.GetCount() > 0)
		{
		m_iRows = Cols[0].GetCount(); // Assume all columns have the same number of rows.
		m_Cols = std::move(Cols);

		//	If the table has keys, make sure the columns have unique keys.

		if (m_iKeyType != EKeyType::None)
			{
			//	We need to create an index for the table.

			const IDatatype &Schema = GetSchema();
			TArray<int> PrimaryKeys;
			if (!Schema.GetKeyMembers(PrimaryKeys))
				throw CException(errFail);

			m_pKeyIndex = std::make_shared<CAEONTableIndex>();
			m_pKeyIndex->InitColsOnly(CDatum(), PrimaryKeys);

			for (int i = 0; i < GetRowCount(); i++)
				{
				CString sKey = m_pKeyIndex->GetIndexKey(*this, i);
				m_pKeyIndex->AddRow(sKey, i);
				}

			//	If the index size is not the same as the table size, then there
			//	were some duplicate keys. We need to rebuild the columns using
			//	the index.

			if (m_pKeyIndex->GetCount() != GetRowCount())
				{
				TArray<CDatum> NewCols = CreateColumns(Schema, NULL, m_pKeyIndex->GetCount());
				for (int iCol = 0; iCol < NewCols.GetCount(); iCol++)
					{
					for (int iRow = 0; iRow < m_pKeyIndex->GetCount(); iRow++)
						{
						int iSrcRow = m_pKeyIndex->GetRow(iRow);
						NewCols[iCol].Append(m_Cols[iCol].GetElement(iSrcRow));
						}
					}

				m_iRows = NewCols[0].GetCount();
				m_Cols = std::move(NewCols);

				//	Invalidate the index because we removed rows.

				InvalidateKeys();
				}
			}
		}
	}

void CAEONTable::Append (CDatum dDatum)

//	Append
//
//	Append a row, column, or table.

	{
	if (!OnModify())
		return;

	switch (dDatum.GetBasicType())
		{
		case CDatum::typeArray:
		case CDatum::typeTensor:
			AppendColumn(dDatum);
			break;

		case CDatum::typeAEONObject:
		case CDatum::typeStruct:
		case CDatum::typeObject:
		case CDatum::typeClassInstance:
		case CDatum::typeRowRef:
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
	if (!OnModify())
		return EResult::NotMutable;

	return EResult::NotImplemented;
	}

IAEONTable::EResult CAEONTable::AppendEmptyRow (int iCount)

//	AppendEmptyRow
//
//	Adds an empty row.

	{
	if (!OnModify())
		return EResult::NotMutable;

	GrowToFit(iCount);

	for (int i = 0; i < m_Cols.GetCount(); i++)
		{
		for (int j = 0; j < iCount; j++)
			m_Cols[i].Append(CDatum());
		}

	m_iRows += iCount;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendRow (CDatum dRow, int* retiRow)

//	AppendRow
//
//	Appends a row.

	{
	if (dRow.GetBasicType() == CDatum::typeArray || dRow.GetBasicType() == CDatum::typeTensor)
		return AppendRowArray(dRow, retiRow);
	else
		return AppendRowStruct(dRow, retiRow);
	}

IAEONTable::EResult CAEONTable::AppendRowArray (CDatum dRow, int* retiRow)

//	AppendRowArray
//
//	Given an array of column values (in schema order), we add a new row.

	{
	if (!OnModify())
		return EResult::NotMutable;

	if (m_iKeyType == EKeyType::None)
		{
		const IDatatype &Schema = GetSchema();
		for (int i = 0; i < Schema.GetMemberCount(); i++)
			{
			CDatum dValue = dRow.GetElement(i);

			//	Make sure we're not trying to add ourselves.

			if (dValue.Contains(CDatum::raw_AsComplex(this)) || dValue.Contains(m_Cols[i]))
				dValue = CDatum();

			//	Add it

			m_Cols[i].Append(dValue);
			}

		if (retiRow)
			*retiRow = m_iRows;
		m_iRows++;

		return EResult::OK;
		}
	else
		{
		auto pKeyIndex = GetIndex();
		if (!pKeyIndex)
			return IAEONTable::EResult::NotImplemented;

		CDatum dKey = pKeyIndex->GetKeyFromRowArray(CDatum::raw_AsComplex(this), dRow);
		return SetRowByID(dKey, dRow, retiRow);
		}
	}

IAEONTable::EResult CAEONTable::AppendRowStruct (CDatum dRow, int* retiRow)

//	AppendRowStruct
//
//	Given a struct of column values, we add a new row.

	{
	if (!OnModify())
		return EResult::NotMutable;

	if (m_iKeyType == EKeyType::None)
		{
		const IDatatype &Schema = GetSchema();
		for (int i = 0; i < Schema.GetMemberCount(); i++)
			{
			auto ColumnDesc = Schema.GetMember(i);

			CDatum dValue = dRow.GetElement(ColumnDesc.sID);

			//	Make sure we're not trying to add ourselves.

			if (dValue.Contains(CDatum::raw_AsComplex(this)) || dValue.Contains(m_Cols[i]))
				dValue = CDatum();

			//	Add it

			m_Cols[i].Append(dValue);
			}

		if (retiRow)
			*retiRow = m_iRows;
		m_iRows++;

		//	Done

		return EResult::OK;
		}
	else
		{
		auto pKeyIndex = GetIndex();
		if (!pKeyIndex)
			return IAEONTable::EResult::NotImplemented;

		CDatum dKey = pKeyIndex->GetKeyFromRow(CDatum::raw_AsComplex(this), dRow);
		return SetRowByID(dKey, dRow, retiRow);
		}
	}

IAEONTable::EResult CAEONTable::AppendSlice (CDatum dSlice)

//	AppendSlice
//
//	Appends a slice of data using the current schema.

	{
	if (!OnModify())
		return EResult::NotMutable;

	if (m_Cols.GetCount() == 0 || dSlice.GetCount() == 0)
		return EResult::OK;

	int iRows = dSlice.GetCount() / m_Cols.GetCount();
	for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
		m_Cols[iCol].GrowToFit(iRows);

	m_cs.Lock();
	auto pKeyIndex = m_pKeyIndex;
	m_cs.Unlock();

	int i = 0;
	for (int iRow = 0; iRow < iRows; iRow++)
		{
		for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
			{
			m_Cols[iCol].Append(dSlice.GetElement(i));

			i++;
			}

		if (pKeyIndex)
			pKeyIndex->Add(CDatum::raw_AsComplex(this), m_iRows + iRow);
		}

	m_iRows += iRows;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendTable (CDatum dTable)

//	AppendTable
//
//	Appends a table.

	{
	if (dTable.IsNil())
		return EResult::OK;

	if (!OnModify())
		return EResult::NotMutable;

	//	If we're indexed, then we need a different algorithm.

	if (HasKeys())
		{
		//	If the source is a table, then we try to merge.

		if (dTable.GetBasicType() == CDatum::typeTable)
			{
			return MergeTable(dTable);
			}

		//	If dTable is a struct, then we expect each field to be a column.

		else if (dTable.GetBasicType() == CDatum::typeStruct)
			{
			if (dTable.GetElement(0).GetBasicType() == CDatum::typeArray)
				return MergeStructOfColumns(dTable);
			else
				return MergeArrayOfStructs(dTable);
			}

		//	Otherwise, we just append one row at a time.

		else
			{
			if (dTable.GetElement(0).GetBasicType() == CDatum::typeArray)
				return MergeArrayOfArrays(dTable);
			else
				return MergeArrayOfStructs(dTable);
			}
		}

	//	If the source is also a table (as opposed to an array), then we can merge.

	else if (dTable.GetBasicType() == CDatum::typeTable)
		{
		//	See if we have the same schema.

		if (dTable.GetDatatype() == GetDatatype())
			{
			const IAEONTable* pSrcTable = dTable.GetTableInterface();
			if (!pSrcTable)
				return EResult::NotATable;

			//	We can just append the columns.

			for (int iCol = 0; iCol < GetColCount(); iCol++)
				{
				CDatum dCol = pSrcTable->GetCol(iCol);

				m_Cols[iCol].GrowToFit(dCol.GetCount());
				for (int i = 0; i < dCol.GetCount(); i++)
					m_Cols[iCol].Append(dCol.GetElement(i));
				}

			m_iRows += pSrcTable->GetRowCount();
			}

		//	Otherwise, we need to append one row at a time.

		else
			{
			GrowToFit(dTable.GetCount());

			for (int i = 0; i < dTable.GetCount(); i++)
				Append(dTable.GetElement(i));
			}
		}

	//	Otherwise we can just pick simpler algorithms

	else
		{
		//	If dTable is a struct, then we expect each field to be a column.

		if (dTable.GetBasicType() == CDatum::typeStruct
				&& dTable.GetElement(0).GetBasicType() == CDatum::typeArray)
			{
			//	We assume that all columns are the same length.

			int iNewRows = dTable.GetElement(0).GetCount();
			GrowToFit(iNewRows);

			//	We can add each column at a time.

			const IDatatype &Schema = GetSchema();

			//	Append each column

			for (int i = 0; i < Schema.GetMemberCount(); i++)
				{
				auto ColumnDesc = Schema.GetMember(i);
				CDatum dCol = dTable.GetElement(ColumnDesc.sID);

				for (int j = 0; j < iNewRows; j++)
					{
					m_Cols[i].Append(dCol.GetElement(j));
					}
				}

			m_iRows += iNewRows;
			}

		//	Otherwise, this is an array or table.

		else
			{
			GrowToFit(dTable.GetCount());

			for (int i = 0; i < dTable.GetCount(); i++)
				AppendRow(dTable.GetElement(i));
			}
		}

	return EResult::OK;
	}

CString CAEONTable::AsString (void) const

//	AsString
//
//	Represent as a string

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return AsAddress();

	CStringBuffer Buffer;

	//	Output header

	const IDatatype &Schema = GetSchema();
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		if (i != 0)
			Buffer.WriteChar('\t');

		Buffer.Write(ColumnDesc.sID);
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
//
//	NOTE: We clear the read-only flag, since the clone is a copy. Callers should
//	set it again if necessary.

	{
	switch (iMode)
		{
		case CDatum::EClone::CopyOnWrite:
			//	Default handling
			return NULL;

		case CDatum::EClone::ShallowCopy:
		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CAEONTable(*this);
			pClone->m_dwTableID = 0xffffffff;	//	indicate new table
			pClone->CloneContents(iMode);
			pClone->m_bReadOnly = false;
			if (pClone->m_pKeyIndex)
				pClone->m_pKeyIndex = std::make_shared<CAEONTableIndex>(*pClone->m_pKeyIndex);

			pClone->m_GroupDef = m_GroupDef;
			pClone->m_GroupIndex = m_GroupIndex;
			return pClone;
			}

		case CDatum::EClone::Isolate:
			return NULL;

		default:
			throw CException(errFail);
		}
	}

void CAEONTable::CloneContents (CDatum::EClone iMode)

//	CloneContents
//
//	Clones all content so that it is a copy.

	{
	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i] = m_Cols[i].Clone(iMode);
	}

CDatum CAEONTable::Create (CDatum dSchema, TArray<CDatum>&& Cols)

//	Create
//
//	Creates a table from a schema and columns. We expect the caller to properly
//	create the columns and schema.

	{
	return CDatum(new CAEONTable(dSchema, std::move(Cols)));
	}

bool CAEONTable::CreateTableFromNil (CAEONTypeSystem& TypeSystem, CDatum& retdDatum)

//	CreateTableFromNil
//
//	Creates a nil table with no rows or columns.

	{
	CDatum dSchema = TypeSystem.AddAnonymousSchema(TArray<IDatatype::SMemberDesc>());
	CDatum dDatatype = TypeSystem.CreateAnonymousTable(NULL_STR, dSchema);
	CAEONTable *pNewTable = new CAEONTable(dDatatype);
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
	if (dFirstRow.IsStruct())
		{
		TArray<IDatatype::SMemberDesc> Columns;
		Columns.GrowToFit(dFirstRow.GetCount());
		for (int i = 0; i < dFirstRow.GetCount(); i++)
			{
			auto pNewColumn = Columns.Insert();
			pNewColumn->iType = IDatatype::EMemberType::InstanceVar;
			pNewColumn->sID = dFirstRow.GetKey(i);
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

	CDatum dDatatype = TypeSystem.CreateAnonymousTable(NULL_STR, dSchema);
	CAEONTable *pNewTable = new CAEONTable(dDatatype);
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

	if (Type.GetClass() == IDatatype::ECategory::Schema
			|| Type.GetClass() == IDatatype::ECategory::Table)
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
		pNewColumn->sID = dValue.GetKey(i);
		pNewColumn->dType = CalcColumnDatatype(dColumnValues);

		if (dColumnValues.GetBasicType() == CDatum::typeArray || dColumnValues.GetBasicType() == CDatum::typeTensor)
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

	CDatum dDatatype = TypeSystem.CreateAnonymousTable(NULL_STR, dSchema);
	CAEONTable *pNewTable = new CAEONTable(dDatatype);
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
	if (!OnModify())
		return EResult::NotMutable;

	//	Recreate the columns.

	SetSchema(m_dDatatype);
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::DeleteCol (int iCol)

//	DeleteCol
//
//	Deletes the given column.

	{
	if (!OnModify())
		return EResult::NotMutable;

	//	We need to create a new schema.

	CDatum dNewSchema;
	if (!DeleteColumnFromSchema(GetSchema(), iCol, dNewSchema))
		return EResult::InvalidParam;

	m_dDatatype = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dNewSchema);

	//	Delete the column

	if (iCol < 0 || iCol >= m_Cols.GetCount())
		throw CException(errFail);

	m_Cols.Delete(iCol);
	m_IsKeyCol.Delete(iCol);

	//	Always invalidate the index, in case the column numbers are affected

	InvalidateKeys();

	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::DeleteRow (int iRow)

//	DeleteRow
//
//	Deletes the given row.

	{
	if (!OnModify())
		return EResult::NotMutable;

	if (iRow < 0 || iRow >= GetRowCount())
		return EResult::OK;

	m_cs.Lock();
	auto pKeyIndex = m_pKeyIndex;
	m_cs.Unlock();

	if (pKeyIndex)
		pKeyIndex->DeleteRow(CDatum::raw_AsComplex(this), iRow);

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].DeleteElement(iRow);

	m_iRows--;

	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::DeleteRowByID (CDatum dKey)

//	DeleteRowByID
//
//	Deletes the given row by ID.

	{
	if (m_iKeyType == EKeyType::None)
		return IAEONTable::EResult::NotImplemented;

	if (!OnModify())
		return EResult::NotMutable;

	auto pKeyIndex = GetIndex();
	if (!pKeyIndex)
		return IAEONTable::EResult::NotImplemented;

	int iRow = pKeyIndex->Find(CDatum::raw_AsComplex(this), dKey);
	if (iRow == -1)
		return IAEONTable::EResult::NotFound;

	pKeyIndex->DeleteRow(CDatum::raw_AsComplex(this), iRow);

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
	const IDatatype &Schema = GetSchema();
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		if (strEqualsNoCase(sName, Schema.GetMember(i).sID))
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
	//	If we have keys, then we use the index to figure out the row index.

	if (HasKeys())
		{
		auto pKeyIndex = GetIndex();
		if (!pKeyIndex)
			return false;

		int iRow = pKeyIndex->Find(CDatum::raw_AsComplex(this), dValue);
		if (iRow == -1)
			return false;

		if (retiRow)
			*retiRow = iRow;
		}

	//	Otherwise, we identify rows by row index, so we're done.

	else
		{
		int iRow = (int)dValue;
		if (iRow < 0 || iRow >= m_iRows)
			return false;

		if (retiRow)
			*retiRow = iRow;
		}

	return true;
	}

bool CAEONTable::FindRowByID2 (CDatum dKey1, CDatum dKey2, int* retiRow) const
	{
	if (HasKeys())
		{
		auto pKeyIndex = GetIndex();
		if (!pKeyIndex)
			return false;

		int iRow = pKeyIndex->Find2(CDatum::raw_AsComplex(this), dKey1, dKey2);
		if (iRow == -1)
			return false;

		if (retiRow)
			*retiRow = iRow;

		return true;
		}
	else
		return false;
	}

bool CAEONTable::FindRowByID3 (CDatum dKey1, CDatum dKey2, CDatum dKey3, int* retiRow) const
	{
	if (HasKeys())
		{
		auto pKeyIndex = GetIndex();
		if (!pKeyIndex)
			return false;

		int iRow = pKeyIndex->Find3(CDatum::raw_AsComplex(this), dKey1, dKey2, dKey3);
		if (iRow == -1)
			return false;

		if (retiRow)
			*retiRow = iRow;

		return true;
		}
	else
		return false;
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
	else if (const CAEONExpression* pExpr = dColName.GetQueryInterface())
		{
		const CAEONExpression::SNode& Node = pExpr->GetRootNode();
		if (Node.iOp == CAEONExpression::EOp::Column)
			{
			if (!FindCol(pExpr->GetColumnID(Node.iDataID), &iIndex))
				return CDatum();

			return m_Cols[iIndex];
			}
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
	const IDatatype &Schema = GetSchema();
	if (iCol < 0 || iCol >= Schema.GetMemberCount())
		throw CException(errFail);

	return Schema.GetMember(iCol).sID;
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

std::shared_ptr<CAEONTableIndex> CAEONTable::GetIndex () const

//	GetIndex
//
//	Returns the index.

	{
	CSmartLock Lock(m_cs);

	//	If we already have an index, then return it (we assume it is updated).

	if (m_pKeyIndex)
		return m_pKeyIndex;

	//	If we don't have primary key columns, then we don't have an index.

	if (m_iKeyType == EKeyType::None)
		return NULL;

	//	Get the primary keys and create the index.

	const IDatatype &Schema = GetSchema();
	TArray<int> PrimaryKeys;
	if (!Schema.GetKeyMembers(PrimaryKeys))
		return NULL;

	m_pKeyIndex = std::make_shared<CAEONTableIndex>();
	m_pKeyIndex->Init(CDatum::raw_AsComplex(this), PrimaryKeys);

	return m_pKeyIndex;
	}

CDatum CAEONTable::GetKeyEx (int iIndex) const

//	GetKeyEx
//
//	Returns the key for a row.

	{
	auto pKeyIndex = GetIndex();
	if (!pKeyIndex)
		return CDatum(iIndex);

	return pKeyIndex->GetKeyFromRow(CDatum::raw_AsComplex(this), iIndex);
	}

CDatum CAEONTable::GetKeyFromRow (CDatum dTable, CDatum dRow) const

//	GetKeyFromRow
//
//	Returns the key for the given row. The row need not currently exist in the
//	table. If the table has no key, we return null.

	{
	auto pKeyIndex = GetIndex();
	if (!pKeyIndex)
		return CDatum();

	if (dRow.GetBasicType() == CDatum::typeArray || dRow.GetBasicType() == CDatum::typeTensor)
		return pKeyIndex->GetKeyFromRowArray(dTable, dRow);
	else
		return pKeyIndex->GetKeyFromRow(dTable, dRow);
	}

TSortMap<CDatum, int> CAEONTable::GetKeyIndex (CDatum dTable) const

//	GetKeyIndex
//
//	Returns a map of index to row position.

	{
	auto pKeyIndex = GetIndex();
	if (!pKeyIndex)
		return TSortMap<CDatum, int>();

	auto& Index = pKeyIndex->GetIndex();
	TSortMap<CDatum, int> Result;
	for (int i = 0; i < Index.GetCount(); i++)
		Result.SetAt(pKeyIndex->GetKeyFromRow(CDatum::raw_AsComplex(this), Index[i]), Index[i]);

	return Result;
	}

CDatum CAEONTable::GetRow (int iRow) const

//	GetRow
//
//	Returns a row.

	{
	if (iRow < 0 || iRow >= m_iRows)
		return CDatum();

	if (m_dwTableID == 0xffffffff)
		{
		m_dwTableID = CAEONStore::AllocTableID(CDatum::raw_AsComplex(this));

		//	If table allocation fails for some reason we fallback to an allocated
		//	object.

		if (m_dwTableID == 0xffffffff)
			return CDatum(new CAEONTableRowRef(CDatum::raw_AsComplex(this), iRow));
		}

	//	Otherwise, we create a row reference.

	return CDatum::CreateRowRef(m_dwTableID, iRow);
	}

int CAEONTable::GetRowCount () const

//	GetRowCount
//
//	Returns the number of rows.

	{
	return m_iRows;
	}

CDatum CAEONTable::GetRowID (int iRow) const

//	GetRowID
//
//	Returns the ID for the given row. (Or null if not found). For tables with
//	a primary key, we return the key. For other tables we return the row index.

	{
	if (iRow < 0 || iRow >= GetRowCount())
		return CDatum();

	if (HasKeys())
		{
		auto pKeyIndex = GetIndex();
		if (!pKeyIndex)
			return CDatum();

		return pKeyIndex->GetKeyFromRow(CDatum::raw_AsComplex(this), iRow);
		}
	else
		{
		return CDatum(iRow);
		}
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
	if (!OnModify())
		return;

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].GrowToFit(iCount);
	}

IAEONTable::EResult CAEONTable::InsertColumn (const CString& sName, CDatum dType, CDatum dValues, int iPos, int *retiCol)

//	InsertColumn
//
//	Inserts a new column.

	{
	if (!OnModify())
		return EResult::NotMutable;

	//	We need to create a new schema.

	CDatum dNewSchema;
	int iNewCol;
	if (!InsertColumnToSchema(GetSchema(), sName, dType, iPos, dNewSchema, &iNewCol))
		return EResult::InvalidParam;

	m_dDatatype = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dNewSchema);

	//	Create the column

	CDatum dNewColumn = CreateColumn(dType);
	m_Cols.Insert(dNewColumn, iNewCol);
	m_IsKeyCol.Insert(false, iNewCol);

	//	Add the values

	int iRowsLeft = GetRowCount();
	dNewColumn.GrowToFit(iRowsLeft);
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

void CAEONTable::InsertElementAt (CDatum dIndex, CDatum dRow)

//	InsertElementAt
//
//	Inserts a row at the given index.

	{
	//	Not supported for keys.

	if (GetKeyType() != EKeyType::None)
		return;

	if (!OnModify())
		return;

	bool bFromEnd;
	int iIndex = dIndex.AsArrayIndex(GetCount(), &bFromEnd);

	//	If from the end, we insert AFTER the specified position (because
	//	we want -1 to mean AFTER the last entry).

	if (bFromEnd && iIndex >= 0)
		iIndex++;

	//	Handle different cases

	if (iIndex >= 0 && iIndex < GetCount())
		{
		const IDatatype &Schema = GetSchema();
		for (int i = 0; i < Schema.GetMemberCount(); i++)
			{
			auto ColumnDesc = Schema.GetMember(i);

			CDatum dValue = dRow.GetElement(ColumnDesc.sID);

			//	Make sure we're not trying to add ourselves.

			if (dValue.Contains(CDatum::raw_AsComplex(this)) || dValue.Contains(m_Cols[i]))
				dValue = CDatum();

			//	Add it

			m_Cols[i].InsertElementAt(iIndex, dValue);
			}

		m_iRows++;
		}
	else if (iIndex >= 0)
		{
		int iNew = iIndex - GetCount();
		GrowToFit(iNew + 1);
		for (int i = 0; i < iNew; i++)
			Append(CDatum());

		Append(dRow);
		}
	}

bool CAEONTable::IsSameSchema (CDatum dDatatype) const

//	IsSameSchema
//
//	Returns TRUE if the given schema has the exact same columns.

	{
	const IDatatype& Schema = GetSchema();
	const IDatatype& Datatype = dDatatype;
	if (Datatype.GetClass() != IDatatype::ECategory::Table)
		return false;

	const IDatatype& TestSchema = Datatype.GetElementType();
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
	if (!OnModify())
		return false;

	CDatum dSchema = dStruct.GetElement(FIELD_DATATYPE);
	const IDatatype &Schema = dSchema;
	if (Schema.GetClass() == IDatatype::ECategory::Schema)
		dSchema = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dSchema);
	else if (Schema.GetClass() != IDatatype::ECategory::Table)
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
	m_dDatatype.Mark();

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].Mark();
	}

bool CAEONTable::OnModify ()

//	OnModify
//
//	The table is about to be modified, so make a copy, if necessary.

	{
	if (m_bReadOnly)
		return false;

	if (m_bCopyOnWrite)
		{
		CloneContents(CDatum::EClone::DeepCopy);
		m_bCopyOnWrite = false;
		}

	//	Increment sequence so we know it got modified.

	m_Seq++;
	return true;
	}

void CAEONTable::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a structure.

	{
	const IDatatype &Schema = GetSchema();

	pStruct->SetElement(FIELD_DATATYPE, m_dDatatype);
	pStruct->SetElement(FIELD_ROWS, m_iRows);
	pStruct->SetElement(FIELD_SEQ, m_Seq);

	CDatum dCols(CDatum::typeArray);
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		CDatum dColDef(CDatum::CDatum::typeStruct);
		auto ColumnDef = Schema.GetMember(i);
		
		dColDef.SetElement(FIELD_NAME, ColumnDef.sID);
		if (i < m_Cols.GetCount())
			dColDef.SetElement(FIELD_VALUES, m_Cols[i]);

		dCols.Append(dColDef);
		}

	pStruct->SetElement(FIELD_COLUMNS, dCols);
	}

IAEONTable::EResult CAEONTable::MergeArrayOfArrays (CDatum dArray)

//	MergeArrayOfArrays
//
//	Merges an array of arrays in which each sub array is exactly as long as the 
//	set of columns for the table.

	{
	//	Create an index for the source table.

	const IDatatype &Schema = GetSchema();
	TArray<int> PrimaryKeys;
	if (!Schema.GetKeyMembers(PrimaryKeys))
		return EResult::InvalidParam;	//	Can't happen, since we checked that we have keys.

	CAEONTableIndex SrcIndex;
	SrcIndex.InitFromArrayOfArrays(dArray, PrimaryKeys);

	//	If we're merging into an empty table and if we don't have any duplicates
	//	in the source, then we can just copy the source.

	if (GetRowCount() == 0 && SrcIndex.GetCount() == dArray.GetCount())
		{
		m_iRows = dArray.GetCount();
		m_Cols = CreateColumns(Schema, NULL, m_iRows);
		//	NOTE: m_IsKeyCol does not need to change because the schema did not change.

		for (int i = 0; i < m_iRows; i++)
			{
			CDatum dRow = dArray.GetElement(i);
			for (int j = 0; j < Schema.GetMemberCount(); j++)
				m_Cols[j].Append(dRow.GetElement(j));
			}

		SetIndex(std::move(SrcIndex));
		return EResult::OK;
		}

	//	Merge the two indexes. We get back a sort map by index where 
	//	the rowID is positive for the original table and negative for the
	//	source table.

	TSortMap<CString, int> MergeMap = GetIndex()->Merge(SrcIndex);

	//	Now we use the resulting index to merge the two tables.

	TArray<CDatum> NewCols = CreateColumns(Schema, NULL, MergeMap.GetCount());

	const IAEONTable* pDestTable = this;
	for (int i = 0; i < MergeMap.GetCount(); i++)
		{
		int iIndex = MergeMap[i];
		if (iIndex >= 0)
			{
			//	We have a row in the destination table.

			for (int j = 0; j < Schema.GetMemberCount(); j++)
				{
				NewCols[j].Append(pDestTable->GetFieldValue(iIndex, j));
				}
			}
		else
			{
			//	We have a row in the source table.

			iIndex = -iIndex - 1;
			CDatum dRow = dArray.GetElement(iIndex);
			for (int j = 0; j < Schema.GetMemberCount(); j++)
				{
				NewCols[j].Append(dRow.GetElement(j));
				}
			}
		}

	//	Replace

	m_iRows = MergeMap.GetCount();
	m_Cols = std::move(NewCols);
	//	NOTE: m_IsKeyCol does not need to change because the schema did not change.

	//	Since we've replaced the columns in index order, we can just fix
	//	the index to be sequential.

	for (int i = 0; i < MergeMap.GetCount(); i++)
		MergeMap[i] = i;

	GetIndex()->ReplaceIndex(std::move(MergeMap));

	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::MergeArrayOfStructs (CDatum dArray)

//	MergeArrayOfStructs
//
//	Merges an array of structs into an indexed table.

	{
	//	Create an index for the source table.

	const IDatatype &Schema = GetSchema();
	TArray<CString> PrimaryKeys;
	if (!Schema.GetKeyMembers(PrimaryKeys))
		return EResult::InvalidParam;	//	Can't happen, since we checked that we have keys.

	CAEONTableIndex SrcIndex;
	SrcIndex.InitFromArrayOfStructs(dArray, PrimaryKeys);

	//	If we're merging into an empty table and if we don't have any duplicates
	//	in the source, then we can just copy the source.

	if (GetRowCount() == 0 && SrcIndex.GetCount() == dArray.GetCount())
		{
		m_iRows = dArray.GetCount();
		m_Cols = CreateColumns(Schema, NULL, m_iRows);
		//	NOTE: m_IsKeyCol does not need to change because the schema did not change.

		for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
			{
			CString sID = Schema.GetMember(iCol).sID;
			for (int iRow = 0; iRow < m_iRows; iRow++)
				{
				CDatum dRow = dArray.GetElement(iRow);
				m_Cols[iCol].Append(dRow.GetElement(sID));
				}
			}

		TArray<int> PrimaryKeys;
		if (!Schema.GetKeyMembers(PrimaryKeys))
			return EResult::InvalidParam;	//	Can't happen, since we checked that we have keys.

		SrcIndex.SetColumns(PrimaryKeys);
		SetIndex(std::move(SrcIndex));
		return EResult::OK;
		}

	//	Merge the two indexes. We get back a sort map by index where 
	//	the rowID is positive for the original table and negative for the
	//	source table.

	TSortMap<CString, int> MergeMap = GetIndex()->Merge(SrcIndex);

	//	Now we use the resulting index to merge the two tables.

	TArray<CDatum> NewCols;
	NewCols.InsertEmpty(Schema.GetMemberCount());

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);
		NewCols[i] = CreateColumn(ColumnDesc.dType);
		NewCols[i].GrowToFit(MergeMap.GetCount());
		}

	const IAEONTable* pDestTable = this;
	for (int i = 0; i < MergeMap.GetCount(); i++)
		{
		int iIndex = MergeMap[i];
		if (iIndex >= 0)
			{
			//	We have a row in the destination table.

			for (int j = 0; j < Schema.GetMemberCount(); j++)
				{
				NewCols[j].Append(pDestTable->GetFieldValue(iIndex, j));
				}
			}
		else
			{
			//	We have a row in the source table.

			iIndex = -iIndex - 1;
			CDatum dRow = dArray.GetElement(iIndex);
			for (int j = 0; j < Schema.GetMemberCount(); j++)
				{
				NewCols[j].Append(dRow.GetElement(Schema.GetMember(j).sID));
				}
			}
		}

	//	Replace

	m_iRows = MergeMap.GetCount();
	m_Cols = NewCols;
	//	NOTE: m_IsKeyCol does not need to change because the schema did not change.

	//	Since we've replaced the columns in index order, we can just fix
	//	the index to be sequential.

	for (int i = 0; i < MergeMap.GetCount(); i++)
		MergeMap[i] = i;

	GetIndex()->ReplaceIndex(std::move(MergeMap));

	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::MergeStructOfColumns (CDatum dStruct)

//	MergeStructOfColumns
//
//	Merges the given table into an indexed table.

	{
	//	Create an index for the source table.

	const IDatatype &Schema = GetSchema();
	TArray<CString> PrimaryKeys;
	if (!Schema.GetKeyMembers(PrimaryKeys))
		return EResult::InvalidParam;	//	Can't happen, since we checked that we have keys.

	CAEONTableIndex SrcIndex;
	SrcIndex.InitFromStructOfArrays(dStruct, PrimaryKeys);

	//	Merge the two indexes. We get back a sort map by index where 
	//	the rowID is positive for the original table and negative for the
	//	source table.

	TSortMap<CString, int> MergeMap = GetIndex()->Merge(SrcIndex);

	//	Now we use the resulting index to merge the two tables.

	TArray<CDatum> NewCols = CreateColumns(Schema, NULL, MergeMap.GetCount());

	const IAEONTable* pDestTable = this;

	for (int iCol = 0; iCol < Schema.GetMemberCount(); iCol++)
		{
		CString sID = Schema.GetMember(iCol).sID;
		CDatum dValues = dStruct.GetElement(sID);

		for (int i = 0; i < MergeMap.GetCount(); i++)
			{
			int iIndex = MergeMap[i];
			if (iIndex >= 0)
				{
				//	We have a row in the destination table.

				NewCols[iCol].Append(pDestTable->GetFieldValue(iIndex, iCol));
				}
			else
				{
				//	We have a row in the source table.

				iIndex = -iIndex - 1;
				NewCols[iCol].Append(dValues.GetElement(iIndex));
				}
			}
		}

	//	Replace

	m_iRows = MergeMap.GetCount();
	m_Cols = NewCols;
	//	NOTE: m_IsKeyCol does not need to change because the schema did not change.

	//	Since we've replaced the columns in index order, we can just fix
	//	the index to be sequential.

	for (int i = 0; i < MergeMap.GetCount(); i++)
		MergeMap[i] = i;

	GetIndex()->ReplaceIndex(std::move(MergeMap));

	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::MergeTable (CDatum dTable)

//	MergeTable
//
//	Merges the given table into an indexed table.

	{
	//	Create an index for the source table.

	const IDatatype &Schema = GetSchema();
	TArray<CString> PrimaryKeys;
	if (!Schema.GetKeyMembers(PrimaryKeys))
		return EResult::InvalidParam;	//	Can't happen, since we checked that we have keys.

	CAEONTableIndex SrcIndex;
	SrcIndex.Init(dTable, PrimaryKeys);

	//	Merge the two indexes. We get back a sort map by index where 
	//	the rowID is positive for the original table and negative for the
	//	source table.

	TSortMap<CString, int> MergeMap = GetIndex()->Merge(SrcIndex);

	//	Now we use the resulting index to merge the two tables.

	TArray<CDatum> NewCols = CreateColumns(Schema, NULL, MergeMap.GetCount());

	const IAEONTable* pSrcTable = dTable.GetTableInterface();
	if (!pSrcTable)
		return EResult::NotATable;

	//	Resolve each column in the source table.

	TArray<int> SrcCols;
	SrcCols.InsertEmpty(Schema.GetMemberCount());
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		if (!pSrcTable->FindCol(Schema.GetMember(i).sID, &SrcCols[i]))
			SrcCols[i] = -1;
		}

	const IAEONTable* pDestTable = this;
	for (int i = 0; i < MergeMap.GetCount(); i++)
		{
		int iIndex = MergeMap[i];
		if (iIndex >= 0)
			{
			//	We have a row in the destination table.

			for (int j = 0; j < Schema.GetMemberCount(); j++)
				{
				NewCols[j].Append(pDestTable->GetFieldValue(iIndex, j));
				}
			}
		else
			{
			//	We have a row in the source table.

			iIndex = -iIndex - 1;
			for (int j = 0; j < Schema.GetMemberCount(); j++)
				{
				if (SrcCols[j] != -1)
					NewCols[j].Append(pSrcTable->GetFieldValue(iIndex, SrcCols[j]));
				else
					NewCols[j].Append(CDatum());
				}
			}
		}

	//	Replace

	m_iRows = MergeMap.GetCount();
	m_Cols = NewCols;
	//	NOTE: m_IsKeyCol does not need to change because the schema did not change.

	//	Since we've replaced the columns in index order, we can just fix
	//	the index to be sequential.

	for (int i = 0; i < MergeMap.GetCount(); i++)
		MergeMap[i] = i;

	GetIndex()->ReplaceIndex(std::move(MergeMap));

	return EResult::OK;
	}

CDatum CAEONTable::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new object and add it to the map.

	CAEONTable *pObj = new CAEONTable;
	CDatum dValue(pObj);
	Serialized.Add(dwID, dValue);

	//	Load

	pObj->m_dDatatype = CDatum::DeserializeAEON(Stream, Serialized);
	pObj->m_iRows = (int)Stream.ReadDWORD();
	pObj->m_Seq = Stream.ReadDWORDLONG();

	TArray<int> Keys;
	pObj->m_iKeyType = CalcKeyType(pObj->m_dDatatype, &Keys);

	//	Read the group definitions

	pObj->m_GroupDef = CAEONTableGroupDefinition::DeserializeAEON(Stream, Serialized);

	//	Load out each column.

	int iCount = (int)Stream.ReadDWORD();
	pObj->m_Cols.InsertEmpty(iCount);
	for (int i = 0; i < iCount; i++)
		pObj->m_Cols[i] = CDatum::DeserializeAEON(Stream, Serialized);

	pObj->m_IsKeyCol.InsertEmpty(iCount);
	for (int i = 0; i < pObj->m_IsKeyCol.GetCount(); i++)
		pObj->m_IsKeyCol[i] = false;

	for (int i = 0; i < Keys.GetCount(); i++)
		{
		int iCol = Keys[i];
		if (iCol >= 0 && iCol < pObj->m_Cols.GetCount())
			pObj->m_IsKeyCol[iCol] = true;
		}

	return dValue;
	}

CDatum CAEONTable::DeserializeAEON_v1 (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new object and add it to the map.

	CAEONTable *pObj = new CAEONTable;
	CDatum dValue(pObj);
	Serialized.Add(dwID, dValue);

	//	Load

	pObj->m_dDatatype = CDatum::DeserializeAEON(Stream, Serialized);
	pObj->m_iRows = (int)Stream.ReadDWORD();
	pObj->m_Seq = Stream.ReadDWORDLONG();

	TArray<int> Keys;
	pObj->m_iKeyType = CalcKeyType(pObj->m_dDatatype, &Keys);

	//	Load out each column.

	int iCount = (int)Stream.ReadDWORD();
	pObj->m_Cols.InsertEmpty(iCount);
	for (int i = 0; i < iCount; i++)
		pObj->m_Cols[i] = CDatum::DeserializeAEON(Stream, Serialized);

	pObj->m_IsKeyCol.InsertEmpty(iCount);
	for (int i = 0; i < pObj->m_IsKeyCol.GetCount(); i++)
		pObj->m_IsKeyCol[i] = false;

	for (int i = 0; i < Keys.GetCount(); i++)
		{
		int iCol = Keys[i];
		if (iCol >= 0 && iCol < pObj->m_Cols.GetCount())
			pObj->m_IsKeyCol[iCol] = true;
		}

	return dValue;
	}

void CAEONTable::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_TABLE_V2))
		return;

	//	Write out the basic info

	m_dDatatype.SerializeAEON(Stream, Serialized);
	Stream.Write(m_iRows);
	Stream.Write(m_Seq);

	//	Write out any group definitions

	m_GroupDef.SerializeAEON(Stream, Serialized);

	//	Write out each column.

	Stream.Write(m_Cols.GetCount());
	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].SerializeAEON(Stream, Serialized);
	}

bool CAEONTable::OpContains (CDatum dValue) const
	{
	auto pKeyIndex = GetIndex();
	if (!pKeyIndex)
		return false;

	return pKeyIndex->Find(CDatum::raw_AsComplex(this), dValue) != -1;
	}

CDatum CAEONTable::Query (const CAEONExpression& Expr) const

//	Query
//
//	Returns the set of rows that matches the given query.

	{
	//	LATER
	return CDatum();
	}

bool CAEONTable::RemoveAll ()

//	RemoveAll
//
//	Removes all rows.

	{
	if (!OnModify())
		return false;

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].RemoveAll();

	InvalidateKeys();
	m_iRows = 0;
	return true;
	}

bool CAEONTable::RemoveElementAt (CDatum dIndex)

//	RemoveElementAt
//
//	Removes element by index.
//
//	NOTE: We rely on the fact that we can always specify a row index, even if 
//	this table has keys.

	{
	int iIndex = dIndex.AsArrayIndex(GetCount());

	if (iIndex >= 0 && iIndex < GetCount())
		{
		if (!OnModify())
			return false;


		for (int i = 0; i < m_Cols.GetCount(); i++)
			m_Cols[i].RemoveElementAt(iIndex);

		m_iRows--;

		m_cs.Lock();
		auto pKeyIndex = m_pKeyIndex;
		m_cs.Unlock();

		if (pKeyIndex)
			pKeyIndex->Remove(CDatum::raw_AsComplex(this), iIndex);

		return true;
		}
	else if (dIndex.IsContainer())
		{
		if (!OnModify())
			return false;

		bool bResult = false;
		for (int i = 0; i < m_Cols.GetCount(); i++)
			if (m_Cols[i].RemoveElementAt(dIndex))
				bResult = true;

		//	Rebuild the index

		InvalidateKeys();

		//	Done

		m_iRows = (m_Cols.GetCount() > 0 ? m_Cols[0].GetCount() : 0);
		return bResult;
		}
	else
		return false;
	}

void CAEONTable::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResovleDatatypes
//
//	Resolve datatypes.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	//	If this schema is already registered with the type system, then use 
	//	its copy. This is helpful when we've serialized/deserialized a table.

	m_dDatatype = TypeSystem.ResolveType(m_dDatatype);

	//	Now resolve all columns.

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].ResolveDatatypes(TypeSystem);
	}

IAEONTable::EResult CAEONTable::SetColumn (int iCol, IDatatype::SMemberDesc& ColDesc, CDatum dValues)

//	SetColumn
//
//	Replaces the given column.

	{
	if (iCol < 0 || iCol >= m_Cols.GetCount())
		throw CException(errFail);

	if (!OnModify())
		return EResult::NotMutable;

	//	If ColDesc is empty, then we're just replacing the values.

	if (ColDesc.iType == IDatatype::EMemberType::None)
		{
		for (int i = 0; i < Min(dValues.GetCount(), GetRowCount()); i++)
			m_Cols[iCol].SetElement(i, dValues.GetElement(i));
		}

	//	Otherwise we need to change our schema.

	else
		{
		//	We need to create a new schema.

		CDatum dNewSchema;
		if (!SetColumnSchema(GetSchema(), iCol, ColDesc, dNewSchema))
			return EResult::InvalidParam;

		CDatum dNewType = ((const IDatatype&)dNewSchema).GetMember(iCol).dType;
		m_dDatatype = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dNewSchema);

		//	Create a new column.

		CDatum dNewColumn = CreateColumn(dNewType);
		dNewColumn.GrowToFit(GetRowCount());
		CDatum dOldColumn = m_Cols[iCol];

		//	If no values, then take from the original column.

		if (dValues.IsNil())
			dValues = m_Cols[iCol];

		for (int i = 0; i < GetRowCount(); i++)
			dNewColumn.Append(dValues.GetElement(i));

		//	Update

		m_Cols[iCol] = dNewColumn;
		m_IsKeyCol[iCol] = ColDesc.iType == IDatatype::EMemberType::InstanceKeyVar;
		}

	return EResult::OK;
	}

void CAEONTable::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets the nth record with the given struct data.

	{
	if (iIndex < 0 || iIndex >= m_iRows)
		return;

	SetRow(iIndex, dDatum);
	}

void CAEONTable::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets a row or a column

	{
	if (!OnModify())
		return;

	SetElementAt_Impl(CDatum::raw_AsComplex(this), dIndex, dDatum);
	}

void CAEONTable::SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue)
	{
	CDatum dKey(CDatum::typeArray);
	dKey.GrowToFit(2);
	dKey.Append(dIndex1);
	dKey.Append(dIndex2);

	return SetElementAt(dKey, dValue);
	}

void CAEONTable::SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue)
	{
	CDatum dKey(CDatum::typeArray);
	dKey.GrowToFit(3);
	dKey.Append(dIndex1);
	dKey.Append(dIndex2);
	dKey.Append(dIndex3);

	return SetElementAt(dKey, dValue);
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

	if (!OnModify())
		return false;

	m_Cols[iCol].SetElement(iRow, dValue);

	//	If we've modified a key column, then we need to invalidate the keys.

	if (m_IsKeyCol[iCol])
		InvalidateKeys();

	return true;
	}

IAEONTable::EResult CAEONTable::SetRow (int iRow, CDatum dRow)

//	SetRow
//
//	Sets the given row.

	{
	if (iRow < 0 || iRow >= m_iRows)
		return EResult::InvalidParam;

	if (!OnModify())
		return EResult::NotMutable;

	m_cs.Lock();
	auto pKeyIndex = m_pKeyIndex;
	m_cs.Unlock();

	if (pKeyIndex)
		pKeyIndex->Remove(CDatum::raw_AsComplex(this), iRow);

	const IDatatype &Schema = GetSchema();
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dRow.GetElement(ColumnDesc.sID);

		//	Make sure we're not trying to add ourselves.

		if (dValue.Contains(CDatum::raw_AsComplex(this)) || dValue.Contains(m_Cols[i]))
			dValue = CDatum();

		//	Add it

		m_Cols[i].SetElement(iRow, dValue);
		}

	if (pKeyIndex)
		pKeyIndex->Add(CDatum::raw_AsComplex(this), iRow);

	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::SetRowByID (CDatum dKey, CDatum dRow, int *retiRow)

//	SetRowByID
//
//	Sets the row by key.

	{
	if (m_iKeyType == EKeyType::None)
		return IAEONTable::EResult::NotImplemented;

	if (!OnModify())
		return EResult::NotMutable;

	auto pKeyIndex = GetIndex();
	if (!pKeyIndex)
		return IAEONTable::EResult::NotImplemented;

	int iRow;
	auto iResult = pKeyIndex->FindOrAdd(CDatum::raw_AsComplex(this), dKey, GetRowCount(), &iRow);

	if (iResult == CAEONTableIndex::EFindResult::Added)
		AppendEmptyRow();

	//	Loop over all columns and set them appropriately.

	const IDatatype &Schema = GetSchema();
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

			dValue = pKeyIndex->GetValueFromKey(CDatum::raw_AsComplex(this), dKey, ColumnDesc.sID);
			}
		else
			{
			//	Look for the value in the row. We do this manually because we
			//	need to distinguish between a missing value and a null value.

			bool bFound = false;
			if (dRow.GetBasicType() == CDatum::typeArray || dRow.GetBasicType() == CDatum::typeTensor)
				{
				if (i < dRow.GetCount())
					{
					dValue = dRow.GetElement(i);
					bFound = true;
					}
				}
			else
				{
				for (int iEntry = 0; iEntry < dRow.GetCount(); iEntry++)
					{
					if (strEqualsNoCase(dRow.GetKey(iEntry), ColumnDesc.sID))
						{
						dValue = dRow.GetElement(iEntry);
						bFound = true;
						break;
						}
					}
				}

			if (!bFound)
				continue;
			}

		//	Make sure we're not trying to add ourselves.

		if (dValue.Contains(CDatum::raw_AsComplex(this)) || dValue.Contains(m_Cols[i]))
			dValue = CDatum();

		//	Add it

		m_Cols[i].SetElement(iRow, dValue);
		}

	if (retiRow)
		*retiRow = iRow;

	return IAEONTable::EResult::OK;
	}

IAEONTable::EResult CAEONTable::SetRowFromColumnStruct (CDatum dCols, int iIndex, int *retiRow)

//	SetRowFromColumnStruct
//
//	dCols is a struct of column values. We set the row based on the column
//	values at the given index.
	
	{
	if (m_iKeyType == EKeyType::None)
		return IAEONTable::EResult::NotImplemented;

	if (!OnModify())
		return EResult::NotMutable;

	auto pKeyIndex = GetIndex();
	if (!pKeyIndex)
		return IAEONTable::EResult::NotImplemented;

	CDatum dKey = pKeyIndex->GetKeyFromColumnStruct(CDatum::raw_AsComplex(this), dCols, iIndex);

	int iRow;
	auto iResult = pKeyIndex->FindOrAdd(CDatum::raw_AsComplex(this), dKey, GetRowCount(), &iRow);

	if (iResult == CAEONTableIndex::EFindResult::Added)
		AppendEmptyRow();

	//	Loop over all columns and set them appropriately.

	const IDatatype &Schema = GetSchema();
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

			dValue = pKeyIndex->GetValueFromKey(CDatum::raw_AsComplex(this), dKey, ColumnDesc.sID);
			}
		else
			{
			CDatum dCol = dCols.GetElement(ColumnDesc.sID);
			dValue = dCol.GetElement(iIndex);
			}

		//	Make sure we're not trying to add ourselves.

		if (dValue.Contains(CDatum::raw_AsComplex(this)) || dValue.Contains(m_Cols[i]))
			dValue = CDatum();

		//	Add it

		m_Cols[i].SetElement(iRow, dValue);
		}

	if (retiRow)
		*retiRow = iRow;

	return IAEONTable::EResult::OK;
	}

void CAEONTable::SetIndex (const CAEONTableIndex& Index)

//	SetIndex
//
//	Set the index.

	{
	CSmartLock Lock(m_cs);
	m_pKeyIndex = std::make_shared<CAEONTableIndex>(Index);
	}

void CAEONTable::SetIndex (CAEONTableIndex&& Index)

//	SetIndex
//
//	Set the index.

	{
	CSmartLock Lock(m_cs);
	m_pKeyIndex = std::make_shared<CAEONTableIndex>(std::move(Index));
	}

void CAEONTable::SetSchema (CDatum dDatatype, bool bNoColCreate)

//	SetSchema
//
//	Sets the schema. This will delete all existing data in the table.

	{
	if (!OnModify())
		return;

	//	Make sure this is an actual table.

	const IDatatype& Datatype = dDatatype;
	if (Datatype.GetClass() != IDatatype::ECategory::Table)
		throw CException(errFail);

	const IDatatype &Schema = Datatype.GetElementType();

	//	Reset

	m_iRows = 0;
	m_dDatatype = dDatatype;
	if (!bNoColCreate)
		m_Cols = CreateColumns(Schema, &m_IsKeyCol);
	else
		{
		m_IsKeyCol.InsertEmpty(Schema.GetMemberCount());
		for (int i = 0; i < m_IsKeyCol.GetCount(); i++)
			m_IsKeyCol[i] = Schema.GetMember(i).iType == IDatatype::EMemberType::InstanceKeyVar;
		}

	InvalidateKeys();
	m_iKeyType = CalcKeyType(dDatatype);
	}
