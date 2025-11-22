//	CAEONTableRef.cpp
//
//	CAEONTableRef classes
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

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
		"$ArrayOfString",
		"Returns an array of all column IDs.",
		[](const CAEONTableRef &Obj, const CString &sProperty)
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
		[](const CAEONTableRef& Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the schema type of the table.",
		[](const CAEONTableRef& Obj, const CString &sProperty)
			{
			return ((const IDatatype&)Obj.GetDatatype()).GetElementType();
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the type of the table key.",
		[](const CAEONTableRef& Obj, const CString &sProperty)
			{
			return ((const IDatatype&)Obj.GetDatatype()).GetKeyType();
			},
		NULL,
		},
	{
		"keys",
		"a",
		"Returns an array of keys.",
		[](const CAEONTableRef &Obj, const CString &sProperty)
			{
			const IAEONTable* pTable = Obj.m_dTable.GetTableInterface();
			if (!pTable)
				return CDatum();

			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Obj.GetRowCount());

			if (Obj.m_bAllRows)
				{
				for (int i = 0; i < Obj.GetRowCount(); i++)
					dResult.Append(pTable->GetRowID(i));
				}
			else
				{
				Obj.InitSourceRow();

				for (int i = 0; i < Obj.GetRowCount(); i++)
					dResult.Append(pTable->GetRowID(Obj.m_SourceRow[i]));
				}

			return dResult;
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of rows in the table.",
		[](const CAEONTableRef &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetRowCount());
			},
		NULL,
		},
	{
		"nextID",
		"i",
		"The nextID to use for the table.",
		[](const CAEONTableRef &Obj, const CString &sProperty)
			{
			const IAEONTable* pTable = Obj.m_dTable.GetTableInterface();
			if (!pTable)
				return CDatum();

			//	NOTE: We preincrement in MakeID, so we need to add 1 here.
			return CDatum(pTable->GetNextID() + 1);
			},
		[](CAEONTableRef &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			IAEONTable* pTable = Obj.m_dTable.GetTableInterface();
			if (!pTable)
				return false;

			pTable->SetNextID((DWORDLONG)dValue - 1);
			return true;
			},
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONTableRef::m_pMethodsExt = NULL;

IAEONTable::EResult CAEONTableRef::AppendEmptyRow (int iCount)
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	return pTable->AppendEmptyRow(iCount);
	}

IAEONTable::EResult CAEONTableRef::AppendRow (CDatum dRow, int* retiRow)
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	return pTable->AppendRow(dRow, retiRow);
	}

IAEONTable::EResult CAEONTableRef::AppendTable (CDatum dTable)
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	auto iError = pTable->AppendTable(dTable);
	if (iError != EResult::OK)
		return iError;

	ApplyDatatype(m_dTable.GetDatatype());
	return EResult::OK;
	}

void CAEONTableRef::ApplyDatatype (CDatum dDatatype)

//	ApplyDatatype
//
//	Take the given datatype.

	{
	m_dDatatype = dDatatype;
	const IDatatype& Datatype = dDatatype;
	const IDatatype& Schema = Datatype.GetElementType();

	//	All columns

	m_Cols.DeleteAll();
	m_Cols.InsertEmpty(Schema.GetMemberCount());
	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i] = i;
	}

CString CAEONTableRef::AsString (void) const

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

IComplexDatum* CAEONTableRef::Clone (CDatum::EClone iMode) const

//	Clone
//
//	We always clone as a real table.

	{
	switch (iMode)
		{
		case CDatum::EClone::CopyOnWrite:
			return NULL;

		default:
			{
			//	Create columns.

			TArray<CDatum> Columns = IAEONTable::CreateColumns(GetSchema(), NULL, GetCount());
			for (int iCol = 0; iCol < GetColCount(); iCol++)
				{
				CDatum dCol = Columns[iCol];
				for (int iRow = 0; iRow < GetCount(); iRow++)
					dCol.Append(GetFieldValue(iRow, iCol));
				}

			//	Create the new table with these columns.

			CAEONTable* pClone = new CAEONTable(GetSchema(), std::move(Columns));
			pClone->SetGroups(m_GroupDef);
			pClone->SetGroupIndex(CAEONTableGroupIndex(m_GroupIndex));

			return pClone;
			}
		}
	}

CDatum CAEONTableRef::CombineSubset (SSubset& ioSubset) const

//	CombineSubset
//
//	Takes the input subset, which is relative to us, and converts it so that it
//	is relative to the underlying table.

	{
	const IAEONTable* pSrcTable = m_dTable.GetTableInterface();
	if (!pSrcTable)
		throw CException(errFail);

	//	Map the columns. The input subset has column IDs relative to us, and we
	//	convert them to column IDs in the original (source) table.

	for (int i = 0; i < ioSubset.Cols.GetCount(); i++)
		ioSubset.Cols[i] = m_Cols[ioSubset.Cols[i]];

	//	If we have all rows, then ioSubset row numbers are the same as the 
	//	source table, so nothing to do.

	if (m_bAllRows)
		{ }

	//	If the subset wants all rows, then we give it our map.

	else if (ioSubset.bAllRows)
		{
		ioSubset.bAllRows = false;
		ioSubset.Rows = m_Rows;
		}

	//	Otherwise, we convert to the source rows

	else
		{
		for (int i = 0; i < ioSubset.Rows.GetCount(); i++)
			ioSubset.Rows[i] = m_Rows[ioSubset.Rows[i]];
		}

	//	If we have group indices, we need to map them to the source row.

	if (!ioSubset.GroupIndex.IsEmpty() && !m_bAllRows)
		{
		TArray<TArray<int>> DestIndex;
		DestIndex.InsertEmpty(ioSubset.GroupIndex.GetCount());

		for (int iGroup = 0; iGroup < DestIndex.GetCount(); iGroup++)
			{
			//	This index refers to our row numbers. We need to convert them to
			//	source row numbers.

			const TArray<int>& Index = ioSubset.GroupIndex.GetGroupIndex(iGroup);
			DestIndex[iGroup].InsertEmpty(Index.GetCount());
			for (int i = 0; i < Index.GetCount(); i++)
				DestIndex[iGroup][i] = m_Rows[Index[i]];
			}

		ioSubset.GroupIndex = CAEONTableGroupIndex(std::move(DestIndex));
		}

	//	Return the source table.

	return m_dTable;
	}

bool CAEONTableRef::Create (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue)

//	Create
//
//	Creates a new table reference.

	{
	const IAEONTable* pInputTable = dTable.GetTableInterface();
	if (!pInputTable)
		{
		retdValue = ERR_NOT_A_TABLE;
		return false;
		}

	//	Combine Subset to refer to the underlying table, if necessary, so that 
	//	we optimze the case of a table reference to a table reference.

	dTable = pInputTable->CombineSubset(Subset);

	//	Init the datatype.

	CDatum dDatatype;
	if (!CreateTableDatatype(TypeSystem, dTable, Subset, dDatatype))
		{
		retdValue = dDatatype;
		return false;
		}

	CAEONTableRef *pNewTable = new CAEONTableRef;
	pNewTable->m_dTable = dTable;
	pNewTable->m_dDatatype = dDatatype;
	pNewTable->m_Cols = std::move(Subset.Cols);
	pNewTable->m_bCopyOnWrite = true;

	if (Subset.bAllRows)
		pNewTable->m_bAllRows = true;
	else if (Subset.Rows.GetCount() > 0)
		pNewTable->m_Rows = std::move(Subset.Rows);

	pNewTable->m_GroupDef = Subset.GroupDef;

	//	If we have group indices, we need to map them to new table.

	if (Subset.bAllRows)
		pNewTable->m_GroupIndex = std::move(Subset.GroupIndex);

	else if (!Subset.GroupIndex.IsEmpty())
		{
		pNewTable->InitSourceRow();

		TArray<TArray<int>> NewIndex;
		NewIndex.InsertEmpty(Subset.GroupIndex.GetCount());

		for (int iGroup = 0; iGroup < NewIndex.GetCount(); iGroup++)
			{
			const TArray<int>& SrcIndex = Subset.GroupIndex.GetGroupIndex(iGroup);
			TArray<int>& DestIndex = NewIndex[iGroup];
			DestIndex.InsertEmpty(SrcIndex.GetCount());

			for (int i = 0; i < SrcIndex.GetCount(); i++)
				{
				DestIndex[i] = pNewTable->m_SourceRow[SrcIndex[i]];
				}
			}

		pNewTable->m_GroupIndex = CAEONTableGroupIndex(std::move(NewIndex));
		}

	retdValue = CDatum(pNewTable);
	return true;
	}

IAEONTable::EResult CAEONTableRef::DeleteAllRows ()
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	return pTable->DeleteAllRows();
	}

IAEONTable::EResult CAEONTableRef::DeleteCol (int iCol)

//	DeleteCol
//
//	Deletes the given column.

	{
	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return IAEONTable::EResult::InvalidParam;

	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	//	We need to create a new schema.

	CDatum dNewSchema;
	if (!DeleteColumnFromSchema(GetSchema(), iCol, dNewSchema))
		return EResult::InvalidParam;

	m_dDatatype = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dNewSchema);

	//	Remove the column

	m_Cols.Delete(iCol);

	return EResult::OK;
	}

IAEONTable::EResult CAEONTableRef::DeleteRowByID (CDatum dKey)
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	return pTable->DeleteRowByID(dKey);
	}

bool CAEONTableRef::FindCol (const CString &sName, int *retiCol) const

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

bool CAEONTableRef::FindRowByID (CDatum dValue, int *retiRow) const

//	FindRowByID
//
//	Looks for a row that matches the given ID. If found, we return true and
//	optionally return the row.

	{
	//	Look for the row in the underlying table.

	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return false;

	int iRow;
	if (!pTable->FindRowByID(dValue, &iRow))
		return false;

	if (m_bAllRows)
		{
		if (retiRow)
			*retiRow = iRow;
		}
	else
		{
		InitSourceRow();

		if (iRow < 0 || iRow >= m_SourceRow.GetCount())
			return false;

		if (m_SourceRow[iRow] == -1)
			return false;

		if (retiRow)
			*retiRow = m_SourceRow[iRow];
		}

	return true;
	}

bool CAEONTableRef::FindRowByID2 (CDatum dKey1, CDatum dKey2, int* retiRow) const
	{
	//	Look for the row in the underlying table.

	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return false;

	int iRow;
	if (!pTable->FindRowByID2(dKey1, dKey2, &iRow))
		return false;

	if (m_bAllRows)
		{
		if (retiRow)
			*retiRow = iRow;
		}
	else
		{
		InitSourceRow();

		if (iRow < 0 || iRow >= m_SourceRow.GetCount())
			return false;

		if (m_SourceRow[iRow] == -1)
			return false;

		if (retiRow)
			*retiRow = m_SourceRow[iRow];
		}

	return true;
	}

bool CAEONTableRef::FindRowByID3 (CDatum dKey1, CDatum dKey2, CDatum dKey3, int* retiRow) const
	{
	//	Look for the row in the underlying table.

	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return false;

	int iRow;
	if (!pTable->FindRowByID3(dKey1, dKey2, dKey3, &iRow))
		return false;

	if (m_bAllRows)
		{
		if (retiRow)
			*retiRow = iRow;
		}
	else
		{
		InitSourceRow();

		if (iRow < 0 || iRow >= m_SourceRow.GetCount())
			return false;

		if (m_SourceRow[iRow] == -1)
			return false;

		if (retiRow)
			*retiRow = m_SourceRow[iRow];
		}

	return true;
	}

CDatum CAEONTableRef::GetCol (CAEONTypeSystem& TypeSystem, CDatum dColName) const

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
			return GetColRef(iIndex);
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

			return GetColRef(iIndex);
			}
		else
			return CDatum();
		}
	else if (dColName.IsContainer())
		{
		IAEONTable::SSubset Subset;
		Subset.bAllRows = m_bAllRows;
		Subset.Rows = m_Rows;

		if (dColName.GetCount() == 0)
			{ }
		else if (dColName.GetElement(0).IsNumberInt32())
			{
			for (int i = 0; i < dColName.GetCount(); i++)
				{
				int iCol = dColName.GetElement(i);
				if (iCol >= 0 && iCol < m_Cols.GetCount() && !Subset.Cols.Find(m_Cols[iCol]))
					Subset.Cols.Insert(m_Cols[iCol]);
				}
			}
		else
			{
			for (int i = 0; i < dColName.GetCount(); i++)
				{
				int iCol;
				if (FindCol(dColName.GetElement(i).AsString(), &iCol)
						&& !Subset.Cols.Find(m_Cols[iCol]))
					Subset.Cols.Insert(m_Cols[iCol]);
				}
			}

		//	Create the subset

		CDatum dResult;

		if (!IAEONTable::CreateRef(TypeSystem, m_dTable, std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}
	else if (FindCol(dColName.AsString(), &iIndex))
		return GetColRef(iIndex);
	else
		return CDatum();
	}

CString CAEONTableRef::GetColName (int iCol) const

//	GetColName
//
//	Returns the name of the column.

	{
	const IDatatype &Schema = GetSchema();
	if (iCol < 0 || iCol >= Schema.GetMemberCount())
		throw CException(errFail);

	return Schema.GetMember(iCol).sID;
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

CDatum CAEONTableRef::GetFieldValue (int iRow, int iCol) const

//	GetFieldValue
//
//	Returns the given field.

	{
	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return CDatum();

	const IAEONTable *pTable = m_dTable.GetTableInterface();
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

CDatum CAEONTableRef::GetKeyEx (int iIndex) const

//	GetKeyEx
//
//	Returns the key for the given row.

	{
	if (m_bAllRows)
		return m_dTable.GetKeyEx(iIndex);
	else if (iIndex >= 0 && iIndex < m_Rows.GetCount())
		return m_dTable.GetKeyEx(m_Rows[iIndex]);
	else
		return CDatum();
	}

CDatum CAEONTableRef::GetKeyFromRow (CDatum dTable, CDatum dRow) const
	{
	const IAEONTable *pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	return pTable->GetKeyFromRow(m_dTable, dRow);
	}

IAEONTable::EKeyType CAEONTableRef::GetKeyType () const

//	GetKeyType
//
//	Returns the key type of the table.

	{
	const IAEONTable *pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return EKeyType::None;

	return pTable->GetKeyType();
	}

SequenceNumber CAEONTableRef::GetNextID () const

//	GetNextID
//
//	Gets the next ID to allocate.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return 0;

	return pTable->GetNextID();
	}

CDatum CAEONTableRef::GetRow (int iRow) const

//	GetRow
//
//	Return the nth row of the table (as a struct).

	{
	if (iRow < 0 || iRow >= GetRowCount())
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

CDatum CAEONTableRef::GetRowID (int iRow) const

//	GetRowID
//
//	Returns the rowID for indexed tables.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	if (m_bAllRows)
		return pTable->GetRowID(iRow);
	else
		{
		if (iRow < 0 || iRow >= m_Rows.GetCount())
			return CDatum();

		return pTable->GetRowID(m_Rows[iRow]);
		}
	}

SequenceNumber CAEONTableRef::GetSeq () const

//	GetSeq
//
//	Get the sequence number.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return 0;

	return pTable->GetSeq();
	}

const CString &CAEONTableRef::GetTypename (void) const

//	GetTypename
//
//	Returns the typename.

	{
	return TYPENAME_TABLE_REF;
	}

bool CAEONTableRef::HasKeys () const

//	HasKeys
//
//	Returns TRUE if this is an indexed table.

	{
	return m_dTable.HasKeys();
	}

void CAEONTableRef::InitSourceRow () const

//	InitSourceRow
//
//	Initializes the source row map.

	{
	CSmartLock Lock(m_cs);

	if (m_bSourceRowValid || m_bAllRows)
		return;

	m_SourceRow.InsertEmpty(m_dTable.GetCount());
	::utlMemSet(&m_SourceRow[0], m_dTable.GetCount() * sizeof(int), (char)0xff);

	for (int i = 0; i < m_Rows.GetCount(); i++)
		m_SourceRow[m_Rows[i]] = i;

	m_bSourceRowValid = true;
	}

IAEONTable::EResult CAEONTableRef::InsertColumn (const CString& sName, CDatum dType, CDatum dValues, int iPos, int *retiCol)
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	auto iError = pTable->InsertColumn(sName, dType, dValues, iPos, retiCol);
	if (iError != EResult::OK)
		return iError;

	ApplyDatatype(m_dTable.GetDatatype());
	return EResult::OK;
	}

void CAEONTableRef::InvalidateKeys ()
	{
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return;

	pTable->InvalidateKeys();

	CSmartLock Lock(m_cs);
	m_GroupIndex.DeleteAll();
	}

bool CAEONTableRef::IsSameSchema (CDatum dDatatype) const

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

	if (TestSchema.GetMemberCount() != Schema.GetMemberCount())
		return false;

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		IDatatype::SMemberDesc Member = Schema.GetMember(i);

		CDatum dTestType;
		if (!CAEONTable::IsValidMemberType(TestSchema.HasMember(Member.sID, &dTestType)))
			return false;

		if (!Member.dType.OpIsEqual(dTestType))
			return false;
		}

	return true;
	}

SequenceNumber CAEONTableRef::MakeID ()

//	MakeID
//
//	Makes a unique ID.

	{
	Materialize();

	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return 0;

	return pTable->MakeID();
	}

void CAEONTableRef::Materialize ()

//	Materialize
//
//	If we've got the copy-on-write flag, we need to make a copy before mutating.

	{
	if (!m_bCopyOnWrite)
		return;

	//	Make a copy of just our portion.

	m_dTable = CDatum(Clone(CDatum::EClone::DeepCopy));
	ApplyDatatype(m_dTable.GetDatatype());

	//	All rows

	m_Rows.DeleteAll();
	m_SourceRow.DeleteAll();
	m_bAllRows = true;
	m_bSourceRowValid = false;

	//	Now we own the table.

	m_bCopyOnWrite = false;
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
	m_dDatatype.Mark();
	}

void CAEONTableRef::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a structure.
//
//	NOTE: We serialize as a table and on deserialize, we turn into a real
//	table.

	{
	const IDatatype &Schema = GetSchema();

	pStruct->SetElement(FIELD_DATATYPE, m_dDatatype);
	pStruct->SetElement(FIELD_ROWS, GetRowCount());

	CDatum dCols(CDatum::typeArray);
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		CDatum dColDef(CDatum::CDatum::typeStruct);
		auto ColumnDef = Schema.GetMember(i);
		
		dColDef.SetElement(FIELD_NAME, ColumnDef.sID);
		if (i < m_Cols.GetCount())
			{
			dColDef.SetElement(FIELD_VALUES, GetColRef(i));
			}

		dCols.Append(dColDef);
		}

	pStruct->SetElement(FIELD_COLUMNS, dCols);
	}

bool CAEONTableRef::OpContains (CDatum dValue) const
	{
	return FindRowByID(dValue);
	}

void CAEONTableRef::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_TABLE_V2))
		return;

	//	Write out the basic info

	m_dDatatype.SerializeAEON(Stream, Serialized);
	Stream.Write(GetRowCount());
	Stream.Write(GetSeq());

	//	Write out any group definitions

	m_GroupDef.SerializeAEON(Stream, Serialized);

	//	Write out each column.

	Stream.Write(GetColCount());
	for (int i = 0; i < GetColCount(); i++)
		GetCol(i).SerializeAEON(Stream, Serialized);
	}

void CAEONTableRef::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResovleDatatypes
//
//	Resolve datatypes.

	{
	//	If this schema is already registered with the type system, then use 
	//	its copy. This is helpful when we've serialized/deserialized a table.

	m_dDatatype = TypeSystem.ResolveType(m_dDatatype);
	}

IAEONTable::EResult CAEONTableRef::SetColumn (int iCol, IDatatype::SMemberDesc& ColDesc, CDatum dValues)
	{
	Materialize();

	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	auto iError = pTable->SetColumn(iCol, ColDesc, dValues);
	if (iError != EResult::OK)
		return iError;

	ApplyDatatype(m_dTable.GetDatatype());
	return EResult::OK;
	}

void CAEONTableRef::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets the nth record with the given struct data.

	{
	Materialize();

	if (iIndex < 0 || iIndex >= GetRowCount())
		return;

	const IDatatype &Schema = GetSchema();
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dDatum.GetElement(ColumnDesc.sID);
		SetFieldValue(iIndex, i, dValue);
		}
	}

void CAEONTableRef::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets a row or a column

	{
	Materialize();

	m_dTable.SetElementAt(dIndex, dDatum);
	ApplyDatatype(m_dTable.GetDatatype());
	}

void CAEONTableRef::SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue)
	{
	Materialize();
	m_dTable.SetElementAt2DA(dIndex1, dIndex2, dValue);
	}

void CAEONTableRef::SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue)
	{
	Materialize();
	m_dTable.SetElementAt3DA(dIndex1, dIndex2, dIndex3, dValue);
	}

bool CAEONTableRef::SetFieldValue (int iRow, int iCol, CDatum dValue)

//	SetFieldValue
//
//	Sets the field value.

	{
	Materialize();

	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return false;

	IAEONTable *pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return false;

	ASSERT(m_bAllRows);
	return pTable->SetFieldValue(iRow, m_Cols[iCol], dValue);
	}

IAEONTable::EResult CAEONTableRef::SetNextID (SequenceNumber NextID)

//	SetNextID
//
//	Sets the next ID.

	{
	Materialize();

	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return EResult::NotATable;

	return pTable->SetNextID(NextID);
	}

void CAEONTableRef::SetReadOnly (bool bValue)
	{
	IAEONTable *pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return;

	pTable->SetReadOnly(bValue);
	}

IAEONTable::EResult CAEONTableRef::SetRow (int iRow, CDatum dRow)

//	SetRow
//
//	Sets the given row.

	{
	Materialize();
	IAEONTable *pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return EResult::NotATable;

	return pTable->SetRow(iRow, dRow);
	}

IAEONTable::EResult CAEONTableRef::SetRowByID (CDatum dKey, CDatum dRow, int *retiRow)
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return IAEONTable::EResult::NotATable;

	return pTable->SetRowByID(dKey, dRow, retiRow);
	}

void CAEONTableRef::SetSeq (SequenceNumber Seq)
	{
	Materialize();
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return;

	pTable->SetSeq(Seq);
	}
