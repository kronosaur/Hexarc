//	IAEONTable.cpp
//
//	IAEONTable class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DELETED,						"deleted");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_FORMAT,						"format");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_KEY,							"key");
DECLARE_CONST_STRING(FIELD_KEYS,						"keys");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_MODIFIED,					"modified");
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");
DECLARE_CONST_STRING(FIELD_UI,							"ui");

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_SCHEMA,		"Unable to create schema.");
DECLARE_CONST_STRING(ERR_INVALID_SCHEMA_DESC,			"Invalid schema desc.");
DECLARE_CONST_STRING(ERR_CANT_COMBINE_KEYS_DONT_MATCH,	"Unable to combine schemas because they have different keys.");
DECLARE_CONST_STRING(ERR_NOT_A_TABLE,					"Not a table.");
DECLARE_CONST_STRING(ERR_SCHEMAS_MUST_MATCH,			"Unable to compare tables because schemas don't match.");

IAEONTable::EResult IAEONTable::AppendRowIfNew (CDatum dTable, CDatum dRow, int* retiRow)

//	AppendRowIfNew
//
//	Appends a row to the table. If the table has a key and the row key already
//	exists, then we return AlreadyExists

	{
	if (dTable.HasKeys())
		{
		CDatum dKey = GetKeyFromRow(dTable, dRow);
		if (FindRowByID(dKey))
			return EResult::AlreadyExists;

		return SetRowByID(dKey, dRow, retiRow);
		}
	else
		return AppendRow(dRow, retiRow);
	}

IAEONTable::EResult IAEONTable::AppendTableColumns (CDatum dTable, CDatum dSrcTable)

//	AppendTableColumns
//
//	Takes the columns from dSrcTable and appends them to dTable, subject to the
//	following conditions:
//
//	1.	If there are conflicting column IDs, we add a suffix to the new column
//	2.	If there aren't enough rows in dSrcTable, we add empty rows to dTable
//	3.	If there are too many rows in dSrcTable, we truncate.

	{
	const IAEONTable* pSrcTable = dSrcTable.GetTableInterface();
	if (!pSrcTable)
		return EResult::NotATable;

	//	Make sure we have a table

	IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return EResult::NotATable;

	//	Insert all columns

	for (int iCol = 0; iCol < pSrcTable->GetColCount(); iCol++)
		{
		const IDatatype& DestSchema = pTable->GetSchema();
		CString sColName = CalcUniqueColName(DestSchema, pSrcTable->GetColName(iCol));
		EResult iResult = pTable->InsertColumn(sColName, pSrcTable->GetColDesc(dSrcTable, iCol).dType, pSrcTable->GetCol(iCol));
		if (iResult != EResult::OK)
			return iResult;
		}

	return EResult::OK;
	}

IAEONTable::EKeyType IAEONTable::CalcKeyType (CDatum dDatatype, TArray<int>* retpKeyCols)

//	CalcKeyType
//
//	Computes the key type based on the table datatype.

	{
	//	Make sure this is an actual table.

	const IDatatype& Datatype = dDatatype;
	if (Datatype.GetClass() != IDatatype::ECategory::Table)
		throw CException(errFail);

	const IDatatype &Schema = Datatype.GetElementType();

	int iKeyCount = 0;
	DWORD dwKeyType = 0;

	//	Set the columns (OK if no members)

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);
		if (ColumnDesc.iType == IDatatype::EMemberType::InstanceKeyVar)
			{
			iKeyCount++;
			dwKeyType = ((const IDatatype&)ColumnDesc.dType).GetCoreType();

			if (retpKeyCols)
				retpKeyCols->Insert(i);
			}
		}

	//	Initialize the index type.

	if (iKeyCount == 0)
		return EKeyType::None;
	else if (iKeyCount == 1)
		{
		if (dwKeyType == IDatatype::INT_32)
			return EKeyType::SingleInt32;
		else if (dwKeyType == IDatatype::FLOAT || dwKeyType == IDatatype::FLOAT_64 || dwKeyType == IDatatype::REAL)
			return EKeyType::SingleFloat;
		else if (dwKeyType == IDatatype::NUMBER)
			return EKeyType::SingleNumber;
		else
			return EKeyType::Single;
		}
	else
		return EKeyType::Multiple;
	}

CString IAEONTable::CalcUniqueColName (const IDatatype& Schema, CStringView sName)

//	CalcUniqueColName
//
//	Generates a unique column name based on the given name.

	{
	CString sNewName = CString(sName);
	int iSuffix = 2;
	while (Schema.FindMember(sNewName) != -1)
		{
		sNewName = strPattern("%s%d", sName, iSuffix++);
		}

	return sNewName;
	}

bool IAEONTable::CombineSchema (CDatum dDatatype1, CDatum dDatatype2, CDatum& retdDatatype)

//	CombineSchema
//
//	Creates a new schema that is a superset of the two given schemas. If the two
//	schemas are not compatible, we return FALSE.

	{
	const IDatatype& Datatype1 = dDatatype1;
	const IDatatype& Datatype2 = dDatatype2;
	if (Datatype1.GetClass() != IDatatype::ECategory::Table || Datatype2.GetClass() != IDatatype::ECategory::Table)
		{
		retdDatatype = ERR_NOT_A_TABLE;
		return false;
		}

	CDatum dSchema1 = Datatype1.GetElementType();
	const IDatatype& Schema1 = dSchema1;

	CDatum dSchema2 = Datatype2.GetElementType();
	const IDatatype& Schema2 = dSchema2;

	//	If one schema has no members, then we just return the other one.

	if (Schema2.GetMemberCount() == 0)
		{
		retdDatatype = dDatatype1;
		return true;
		}
	else if (Schema1.GetMemberCount() == 0)
		{
		retdDatatype = dDatatype2;
		return true;
		}

	//	If the two schemas equal each other, then just return one.

	else if (Schema1 == Schema2)
		{
		retdDatatype = dDatatype1;
		return true;
		}

	//	Accumulate columns by name

	TArray<IDatatype::SMemberDesc> Columns;
	TSortMap<CString, int> ByName;

	for (int i = 0; i < Schema1.GetMemberCount(); i++)
		{
		auto Member = Schema1.GetMember(i);

		int iIndex = Columns.GetCount();
		Columns.Insert(Member);
		ByName.SetAt(strToLower(Member.sID), iIndex);
		}

	//	Now add the second schema.

	for (int i = 0; i < Schema2.GetMemberCount(); i++)
		{
		auto Member2 = Schema2.GetMember(i);

		//	If this column is already in schema1, then we continue.

		int *pCol = ByName.GetAt(strToLower(Member2.sID));
		if (pCol)
			{
			//	If schema2 does not consider this a key column, then we need to
			//	convert it back to a normal column.

			if (Columns[*pCol].iType == IDatatype::EMemberType::InstanceKeyVar
					&& Member2.iType != IDatatype::EMemberType::InstanceKeyVar)
				Columns[*pCol].iType = IDatatype::EMemberType::InstanceVar;

			//	NOTE: If the datatypes don't match, we take schema1's datatype.
			}

		//	Otherwise, we need to add this column to the schema.

		else
			{
			//	Make sure it is not a key

			Member2.iType = IDatatype::EMemberType::InstanceVar;
			Columns.Insert(Member2);
			}
		}

	//	Done

	CAEONTypeSystem TypeSystem;
	CDatum dSchema = TypeSystem.AddAnonymousSchema(Columns);
	retdDatatype = TypeSystem.CreateAnonymousTable(NULL_STR, dSchema);
	if (dSchema.IsNil() || retdDatatype.IsNil())
		{
		retdDatatype = ERR_UNABLE_TO_CREATE_SCHEMA;
		return false;
		}

	return true;
	}

CDatum IAEONTable::CreateColumn (CDatum dType)

//	CreateColumn
//
//	Creates an empty column.

	{
	return CDatum::CreateArrayAsTypeOfElement(dType);
	}

TArray<CDatum> IAEONTable::CreateColumns (const IDatatype& Schema, TArray<bool>* retpIsKeyCol, int iGrowToFit)

//	CreateColumns
//
//	Creates an array of columns of the proper types.

	{
	TArray<CDatum> Cols;
	Cols.InsertEmpty(Schema.GetMemberCount());
	if (retpIsKeyCol)
		retpIsKeyCol->InsertEmpty(Schema.GetMemberCount());

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		if (!IsValidMemberType(ColumnDesc.iType)
				|| ColumnDesc.dType.GetBasicType() != CDatum::typeDatatype
				|| ColumnDesc.sID.IsEmpty())
			throw CException(errFail);

		//	Create columns based on the datatype

		Cols[i] = CreateColumn(ColumnDesc.dType);
		Cols[i].GrowToFit(iGrowToFit);

		//	If this is a key column, then we need to set the key type.

		if (retpIsKeyCol)
			(*retpIsKeyCol)[i] = (ColumnDesc.iType == IDatatype::EMemberType::InstanceKeyVar);
		}

	return Cols;
	}

CDatum IAEONTable::CreateFormattedTable (CDatum dTable) const

//	CreateFormattedTable
//
//	Creates a new table with all string columns in which values have been 
//	formatted based on the original table's schema format.

	{
	//	Start by create a new table schema with the same columns, but all string
	//	types.

	const IDatatype& Schema = GetSchema();
	TArray<CString> Format;
	TArray<IDatatype::SMemberDesc> NewMembers;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto MemberDesc = Schema.GetMember(i);
		if (MemberDesc.iType != IDatatype::EMemberType::InstanceKeyVar && MemberDesc.iType != IDatatype::EMemberType::InstanceVar)
			throw CException(errFail);

		//	No keys
		MemberDesc.iType = IDatatype::EMemberType::InstanceVar;
		MemberDesc.dType = CAEONTypeSystem::GetCoreType(IDatatype::STRING);
		Format.Insert(MemberDesc.sFormat);
		MemberDesc.sFormat = NULL_STR;
		NewMembers.Insert(MemberDesc);
		}

	CAEONTypeSystem TypeSystem;
	CDatum dNewSchema = TypeSystem.AddAnonymousSchema(NewMembers);
	const IDatatype& NewSchema = dNewSchema;

	//	Now create the column values.

	TArray<CDatum> NewColumns = IAEONTable::CreateColumns(NewSchema, NULL, dTable.GetCount());
	if (NewColumns.GetCount() != NewMembers.GetCount())
		throw CException(errFail);

	for (int iCol = 0; iCol < NewColumns.GetCount(); iCol++)
		{
		CDatum dOldCol = GetCol(iCol);
		if (Format[iCol].IsEmpty())
			{
			for (int i = 0; i < dOldCol.GetCount(); i++)
				{
				NewColumns[iCol].Append(dOldCol.GetElement(i));
				}
			}
		else
			{
			for (int i = 0; i < dOldCol.GetCount(); i++)
				{
				NewColumns[iCol].Append(dOldCol.GetElement(i).Format(Format[iCol]));
				}
			}
		}

	return CDatum::CreateTable(dNewSchema, std::move(NewColumns));
	}

bool IAEONTable::CreateRef (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue)

//	CreateRef
//
//	Creates a table reference.

	{
	return CAEONTableRef::Create(TypeSystem, dTable, std::move(Subset), retdValue);
	}

bool IAEONTable::CreateTableDatatype (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset& Subset, CDatum& retdDatatype)

//	CreateTableDatatype
//
//	Creates a subset schema.

	{
	//	Otherwise we need to construct a new schema.

	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		{
		retdDatatype = ERR_NOT_A_TABLE;
		return false;
		}

	const IDatatype& Schema = pTable->GetSchema();

	//	If no columns listed, then we take all the columns in the source table,
	//	which means the schema is the same.

	if (Subset.Cols.GetCount() == 0)
		{
		retdDatatype = dTable.GetDatatype();
		Subset.Cols.InsertEmpty(Schema.GetMemberCount());
		for (int i = 0; i < Subset.Cols.GetCount(); i++)
			Subset.Cols[i] = i;

		return true;
		}

	//	Make a list of columns to take.

	TArray<bool> Include;
	Include.InsertEmpty(Schema.GetMemberCount());
	for (int i = 0; i < Include.GetCount(); i++)
		Include[i] = false;

	for (int i = 0; i < Subset.Cols.GetCount(); i++)
		{
		if (Subset.Cols[i] >= 0 
				&& Subset.Cols[i] < Include.GetCount()
				&& !Include[Subset.Cols[i]])
			Include[Subset.Cols[i]] = true;

		//	Otherwise, we delete it from the list because it is either an 
		//	invalid column or a duplicate column.

		else
			{
			Subset.Cols.Delete(i);
			i--;
			}
		}

	//	We treat each field of the struct as a column.
	//	Note that we add the columns in the specified order, which may be
	//	different from the schema order.

	TArray<IDatatype::SMemberDesc> Columns;
	for (int i = 0; i < Subset.Cols.GetCount(); i++)
		{
		Columns.Insert(Schema.GetMember(Subset.Cols[i]));
		}

	//	Create a new schema.

	CDatum dSchema = TypeSystem.AddAnonymousSchema(Columns);
	retdDatatype = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dSchema);
	if (dSchema.IsNil() || retdDatatype.IsNil())
		{
		retdDatatype = ERR_UNABLE_TO_CREATE_SCHEMA;
		return false;
		}

	return true;
	}

bool IAEONTable::CreateSchemaFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdDatatype)

//	CreateSchemaFromDesc
//
//	Creates a schema from a descriptor. A schema descriptor is an array of 
//	structs; each struct has the following fields:
//
//	datatype (required)
//	name (required)
//	label (optional, defaults to name)
//	description (optional)
//	format (optional)
//
//	On error, we return FALSE and retdSchema is the error.

	{
	//	If the descriptor is already a type, make sure it is a schema type.

	if (dSchemaDesc.GetBasicType() == CDatum::typeDatatype)
		{
		const IDatatype& Type = dSchemaDesc;
		if (Type.GetClass() == IDatatype::ECategory::Schema)
			{
			retdDatatype = dSchemaDesc;
			return true;
			}
		else
			{
			retdDatatype = ERR_INVALID_SCHEMA_DESC;
			return false;
			}
		}

	//	If the descriptor is a structure, then each key is one column in a 
	//	schema definition.

	else if (dSchemaDesc.GetBasicType() == CDatum::typeStruct)
		{

		CDatum dIDs = dSchemaDesc.GetElement(FIELD_ID);
		if (dIDs.GetCount() == 0)
			{
			retdDatatype = ERR_INVALID_SCHEMA_DESC;
			return false;
			}

		CDatum dKeys = dSchemaDesc.GetElement(FIELD_KEY);
		CDatum dTypes = dSchemaDesc.GetElement(FIELD_TYPE);
		CDatum dLabels = dSchemaDesc.GetElement(FIELD_LABEL);
		CDatum dFormats = dSchemaDesc.GetElement(FIELD_FORMAT);
		CDatum dDisplays = dSchemaDesc.GetElement(FIELD_UI);

		TArray<IDatatype::SMemberDesc> Columns;
		Columns.InsertEmpty(dIDs.GetCount());
		TSortMap<CString, int> DuplicateCheck;

		for (int i = 0; i < Columns.GetCount(); i++)
			{
			Columns[i].iType = (dKeys.GetElement(i).AsBool() ? IDatatype::EMemberType::InstanceKeyVar : IDatatype::EMemberType::InstanceVar);
			Columns[i].sID = dIDs.GetElement(i).AsString();
			CString sID = strToLower(Columns[i].sID);
			if (sID.IsEmpty() || DuplicateCheck.GetAt(sID))
				{
				retdDatatype = ERR_INVALID_SCHEMA_DESC;
				return false;
				}

			Columns[i].dType = dTypes.GetElement(i);
			if (Columns[i].dType.GetBasicType() != CDatum::typeDatatype)
				{
				retdDatatype = ERR_INVALID_SCHEMA_DESC;
				return false;
				}

			Columns[i].sLabel = dLabels.GetElement(i).AsString();
			Columns[i].sFormat = dFormats.GetElement(i).AsString();
			Columns[i].iDisplay = IDatatype::ParseDisplay(dDisplays.GetElement(i));

			DuplicateCheck.SetAt(sID, i);
			}

		//	Create a new schema
	
		retdDatatype = TypeSystem.AddAnonymousSchema(Columns);
		if (retdDatatype.IsNil())
			{
			retdDatatype = ERR_UNABLE_TO_CREATE_SCHEMA;
			return false;
			}
		}

	//	If this is a container, then treat as an array

	else if (dSchemaDesc.IsContainer() && dSchemaDesc.GetCount() > 0)
		{
		TArray<IDatatype::SMemberDesc> Columns;
		TSortMap<CString, int> DuplicateCheck;
		for (int i = 0; i < dSchemaDesc.GetCount(); i++)
			{
			CDatum dColDesc = dSchemaDesc.GetElement(i);

			IDatatype::SMemberDesc ColDesc;
			ColDesc.iType = (dColDesc.GetElement(FIELD_KEY).AsBool() ? IDatatype::EMemberType::InstanceKeyVar : IDatatype::EMemberType::InstanceVar);
			ColDesc.sID = dColDesc.GetElement(FIELD_ID).AsString();
			if (ColDesc.sID.IsEmpty())
				ColDesc.sID = dColDesc.GetElement(FIELD_NAME).AsString();
			CString sID = strToLower(ColDesc.sID);

			if (ColDesc.sID.IsEmpty() || DuplicateCheck.GetAt(sID))
				{
				retdDatatype = ERR_INVALID_SCHEMA_DESC;
				return false;
				}

			ColDesc.dType = dColDesc.GetElement(FIELD_TYPE);
			if (ColDesc.dType.IsNil())
				ColDesc.dType = dColDesc.GetElement(FIELD_DATATYPE);
			if (ColDesc.dType.GetBasicType() != CDatum::typeDatatype)
				{
				retdDatatype = ERR_INVALID_SCHEMA_DESC;
				return false;
				}

			ColDesc.sLabel = dColDesc.GetElement(FIELD_LABEL).AsString();
			ColDesc.sFormat = dColDesc.GetElement(FIELD_FORMAT).AsString();
			ColDesc.iDisplay = IDatatype::ParseDisplay(dColDesc.GetElement(FIELD_UI));

			DuplicateCheck.SetAt(sID, i);
			Columns.Insert(ColDesc);
			}

		//	Create a new schema
	
		retdDatatype = TypeSystem.AddAnonymousSchema(Columns);
		if (retdDatatype.IsNil())
			{
			retdDatatype = ERR_UNABLE_TO_CREATE_SCHEMA;
			return false;
			}
		}

	//	Otherwise, error

	else
		{
		retdDatatype = ERR_INVALID_SCHEMA_DESC;
		return false;
		}

	return true;
	}

bool IAEONTable::CreateTableDatatypeFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdDatatype)

//	CreateSchemaFromDesc
//
//	Creates a schema from a descriptor. A schema descriptor is an array of 
//	structs; each struct has the following fields:
//
//	datatype (required)
//	name (required)
//	label (optional, defaults to name)
//	description (optional)
//	format (optional)
//
//	On error, we return FALSE and retdSchema is the error.

	{
	CDatum dSchema;
	if (!CreateSchemaFromDesc(TypeSystem, dSchemaDesc, dSchema))
		{
		retdDatatype = dSchema;
		return false;
		}

	//	Now create the table type

	retdDatatype = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dSchema);
	if (dSchema.IsNil() || retdDatatype.IsNil())
		{
		retdDatatype = ERR_UNABLE_TO_CREATE_SCHEMA;
		return false;
		}

	return true;
	}

CDatum IAEONTable::CreateSorted (CDatum dTable, const TArray<SSort>& Sort)

//	CreateSorted
//
//	Creates a sorted table reference.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	if (!pTable->ValidateSort(Sort))
		return CDatum();

	IAEONTable::SSubset Subset;
	Subset.Rows.InsertEmpty(pTable->GetRowCount());
	for (int i = 0; i < pTable->GetRowCount(); i++)
		Subset.Rows[i] = i;

	if (Sort.GetCount() > 0)
		{
		Subset.Rows.Sort([pTable, &Sort](const int &iRow1, const int &iRow2) {

			//	Row1 > Row2		-> 1
			//	Row1 < Row2		-> -1
			//	Row1 == Row2	-> 0

			for (int i = 0; i < Sort.GetCount(); i++)
				{
				CDatum dValue1 = pTable->GetFieldValue(iRow1, Sort[i].iCol);
				CDatum dValue2 = pTable->GetFieldValue(iRow2, Sort[i].iCol);
				int iCompare = dValue1.OpCompare(dValue2);
				if (iCompare != 0)
					{
					if (Sort[i].iOrder == IAEONTable::ESort::Descending)
						return -1 * iCompare;
					else
						return iCompare;
					}
				}

			return ::KeyCompare(iRow1, iRow2);
			});
		}

	CAEONTypeSystem TypeSystem;
	CDatum dResult;

	if (!CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
		return CDatum();

	return dResult;
	}

bool IAEONTable::DeleteColumnFromSchema (CDatum dSchema, int iCol, CDatum& retdSchema)

//	DeleteColumnFromSchema
//
//	Deletes the given column.

	{
	const IDatatype& OldSchema = dSchema;
	if (iCol < 0 || iCol >= OldSchema.GetMemberCount() || OldSchema.GetMemberCount() == 1)
		return false;

	TArray<IDatatype::SMemberDesc> Members;
	Members.InsertEmpty(OldSchema.GetMemberCount() - 1);

	int iDest = 0;
	for (int i = 0; i < OldSchema.GetMemberCount(); i++)
		{
		if (i != iCol)
			Members[iDest++] = OldSchema.GetMember(i);
		}

	CAEONTypeSystem TypeSystem;
	retdSchema = TypeSystem.AddAnonymousSchema(Members);

	return true;
	}

bool IAEONTable::DerefByPosition (EKeyType iKeyType, CDatum dIndex)

//	DerefByPosition
//
//	Returns TRUE if we should dereference a table by original row position
//	or FALSE if we should use dIndex as a key.

	{
	//	If we have an Int32 key, then we need to lookup by key.

	if (iKeyType == EKeyType::SingleInt32)
		return false;
	
	//	If we have a float key, then see if we have a float index.

	else if (iKeyType == EKeyType::SingleFloat)
		{
		if (dIndex.GetBasicType() == CDatum::typeDouble)
			return false;
		else
			return dIndex.IsNumberInt32();
		}

	//	If we hae a number key, then we can never dereference by ordinal.

	else if (iKeyType == EKeyType::SingleNumber)
		return false;

	//	Otherwise, if dIndex can be coerced to an Int32, then we should use
	//	it as a position.

	else
		return dIndex.IsNumberInt32();
	}

bool IAEONTable::Diff (CDatum dTable, CDatum dOriginalTable, const SDiffOptions& Options, CArrayDiff::Results& retResults, CString* retsError) const

//	Diff
//
//	Returns a table of differences in this table relative to the original table.
//
//	We return a table of DiffResultSchema records.

	{
	const IAEONTable* pOriginal = dOriginalTable.GetTableInterface();
	if (!pOriginal)
		{
		if (retsError) *retsError = ERR_NOT_A_TABLE;
		return false;
		}

	//	The schema needs to match, or else we can't compare.

	const IDatatype& Schema = GetSchema();
	const IDatatype& OriginalSchema = pOriginal->GetSchema();
	if (Schema != OriginalSchema)
		{
		if (retsError) *retsError = ERR_SCHEMAS_MUST_MATCH;
		return false;
		}

	//	Get the indices of the columns to check. This is faster than
	//	getting each row.

	TArray<int> OriginalCols;
	TArray<int> NewCols;
	for (int i = 0; i < pOriginal->GetColCount(); i++)
		{
		CString sCol = pOriginal->GetColName(i);
		if (!FindColName(sCol, Options.ExcludeCols))
			{
			OriginalCols.Insert(i);
			int iNewCol;
			if (!FindCol(sCol, &iNewCol))
				{
				if (retsError) *retsError = ERR_SCHEMAS_MUST_MATCH;
				return false;
				}

			NewCols.Insert(iNewCol);
			}
		}

	//	If we have keys, then we compare by keys.

	if (GetKeyType() != EKeyType::None)
		{
		//	Compare

		TSortMap<CDatum, int> Keys = GetKeyIndex(dTable);
		TSortMap<CDatum, int> OriginalKeys = pOriginal->GetKeyIndex(dOriginalTable);

		int iCursor = 0;
		int iOriginalCursor = 0;
		while (iCursor < Keys.GetCount() && iOriginalCursor < OriginalKeys.GetCount())
			{
			//	If the keys match, then see if the row has changed.

			int iCompare = Keys.GetKey(iCursor).OpCompare(OriginalKeys.GetKey(iOriginalCursor));
			if (iCompare == 0)
				{
				//	Compare each field

				for (int i = 0; i < OriginalCols.GetCount(); i++)
					{
					CDatum dValue = GetFieldValue(Keys[iCursor], NewCols[i]);
					CDatum dOriginalValue = pOriginal->GetFieldValue(OriginalKeys[iOriginalCursor], OriginalCols[i]);

					if (dValue != dOriginalValue)
						{
						//	If we need to clean, then we need to compare the cleaned
						//	values.

						if (Options.bCleaned && (dValue.GetBasicType() == CDatum::typeString || dOriginalValue.GetBasicType() == CDatum::typeString))
							{
							CString sCleanedValue = strClean(dValue.AsString());
							CString sCleanedOriginalValue = strClean(dOriginalValue.AsString());

							if (!strEqualsNoCase(sCleanedValue, sCleanedOriginalValue))
								{
								retResults.Changes.Insert({ CArrayDiff::ChangeType::Modified, OriginalKeys[iOriginalCursor], Keys[iCursor], 1 });
								break;
								}
							}
						else
							{
							retResults.Changes.Insert({ CArrayDiff::ChangeType::Modified, OriginalKeys[iOriginalCursor], Keys[iCursor], 1 });
							break;
							}
						}
					}

				//	Next

				iCursor++;
				iOriginalCursor++;
				}

			//	If the key is greater than the original key at this cursor 
			//	position then it means we've deleted something

			else if (iCompare == 1)
				{
				retResults.Changes.Insert({ CArrayDiff::ChangeType::Deleted, OriginalKeys[iOriginalCursor], Keys[iCursor], 1 });

				iOriginalCursor++;
				}

			//	If the key is less than the original key at this cursor position
			//	then it means we've inserted something

			else
				{
				retResults.Changes.Insert({ CArrayDiff::ChangeType::Inserted, OriginalKeys[iOriginalCursor], Keys[iCursor], 1 });

				iCursor++;
				}
			}

		//	Anything left in the new table is an insert

		if (iCursor < Keys.GetCount())
			{
			if (Options.bCollapsed)
				retResults.Changes.Insert({ CArrayDiff::ChangeType::Inserted, pOriginal->GetRowCount(), Keys[iCursor], Keys.GetCount() - iCursor });
			else
				{
				for (int i = 0; i < Keys.GetCount() - iCursor; i++)
					retResults.Changes.Insert({ CArrayDiff::ChangeType::Inserted, pOriginal->GetRowCount(), Keys[iCursor + i], 1 });
				}
			}

		//	Anything left in the old table is a delete

		if (iOriginalCursor < OriginalKeys.GetCount())
			{
			if (Options.bCollapsed)
				retResults.Changes.Insert({ CArrayDiff::ChangeType::Deleted, OriginalKeys[iOriginalCursor], pOriginal->GetRowCount(), OriginalKeys.GetCount() - iOriginalCursor });
			else
				{
				for (int i = 0; i < OriginalKeys.GetCount() - iOriginalCursor; i++)
					retResults.Changes.Insert({ CArrayDiff::ChangeType::Deleted, OriginalKeys[iOriginalCursor + i], pOriginal->GetRowCount(), 1 });
				}
			}

		return true;
		}
	else
		{
		CDatumTableWrapper OriginalTable(dOriginalTable);
		CDatumTableWrapper NewTable(dTable);

		TArrayDiff<CDatumTableWrapper, CDatum> Diff(OriginalTable, NewTable);
		retResults = Diff.Diff();

		//	If not collapsed, then we need to expand the results.

		if (!Options.bCollapsed)
			{
			TArray<CArrayDiff::Change> ExpandedChanges;
			for (int i = 0; i < retResults.Changes.GetCount(); i++)
				{
				const CArrayDiff::Change& Change = retResults.Changes[i];
				if (Change.iCount > 1)
					{
					for (int j = 0; j < Change.iCount; j++)
						{
						ExpandedChanges.Insert({ Change.Type, Change.iOldIndex + j, Change.iNewIndex + j, 1 });
						}
					}
				else
					{
					ExpandedChanges.Insert(Change);
					}
				}

			retResults.Changes = std::move(ExpandedChanges);
			}

		return true;
		}
	}

CDatum IAEONTable::FilterModified (CDatum dTable, const CDateTime& ModifiedOn)

//	FilterModified
//
//	Returns a table with rows where modifiedOn is greater than the given 
//	value.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	int iModifiedOnCol;
	if (!pTable->FindCol(FIELD_MODIFIED_ON, &iModifiedOnCol))
		return CDatum();

	IAEONTable::SSubset Subset;
	for (int i = 0; i < pTable->GetRowCount(); i++)
		{
		CDatum dModifiedOn = pTable->GetFieldValue(i, iModifiedOnCol);
		if (!dModifiedOn.IsNil() && (const CDateTime&)dModifiedOn > ModifiedOn)
			Subset.Rows.Insert(i);
		}

	CAEONTypeSystem TypeSystem;
	CDatum dResult;

	if (!CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
		return CDatum();

	return dResult;
	}

CDatum IAEONTable::FilterModifiedByTable (CDatum dTable, CDatum dOriginalTable, const SFilterModifiedOptions& Options)

//	FilterModifiedByTable
//
//	For each row in the original table, if the modifiedOn field is less than the
//	corresponding row in the new table, then we include the row in the result.
//	If there is a row in the new table that is not present in the original table,
//	then we include it in the result (unless Options.bExcludeNull is TRUE).
//
//	We return a struct with two fields:
//
//	modified: A table with rows that are modified.
//	deleted: A list of deleted rows.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable || pTable->GetKeyType() == IAEONTable::EKeyType::None)
		return CDatum();

	int iNewModifiedOnCol;
	if (!pTable->FindCol(FIELD_MODIFIED_ON, &iNewModifiedOnCol))
		return CDatum();

	const IAEONTable* pOriginalTable = dOriginalTable.GetTableInterface();
	if (!pOriginalTable || pOriginalTable->GetKeyType() == IAEONTable::EKeyType::None)
		return CDatum();

	int iOriginalModifiedOnCol;
	if (!pOriginalTable->FindCol(FIELD_MODIFIED_ON, &iOriginalModifiedOnCol))
		return CDatum();

	//	We keep track of rows in the OriginalTable that we know are in the new
	//	table.

	TArray<bool> ExistingRows;
	ExistingRows.InsertEmpty(pOriginalTable->GetRowCount());
	for (int i = 0; i < pOriginalTable->GetRowCount(); i++)
		ExistingRows[i] = false;

	IAEONTable::SSubset Subset;
	for (int i = 0; i < pTable->GetRowCount(); i++)
		{
		CDatum dModifiedOn = pTable->GetFieldValue(i, iNewModifiedOnCol);
		if (dModifiedOn.IsNil())
			continue;
		
		int iOriginalRow;
		if (pOriginalTable->FindRowByID(pTable->GetRowID(i), &iOriginalRow))
			{
			CDatum dOriginalModifiedOn = pOriginalTable->GetFieldValue(iOriginalRow, iOriginalModifiedOnCol);
			if (dOriginalModifiedOn.IsNil() || (const CDateTime&)dModifiedOn > (const CDateTime&)dOriginalModifiedOn)
				Subset.Rows.Insert(i);

			ExistingRows[iOriginalRow] = true;
			}
		else if (!Options.bExcludeNull)
			Subset.Rows.Insert(i);
		}

	CAEONTypeSystem TypeSystem;
	CDatum dResult(CDatum::typeStruct);

	CDatum dModified;
	if (!CreateRef(TypeSystem, dTable, std::move(Subset), dModified))
		return CDatum();

	dResult.SetElement(FIELD_MODIFIED, dModified);

	CDatum dDeleted(CDatum::typeArray);
	for (int i = 0; i < ExistingRows.GetCount(); i++)
		if (!ExistingRows[i])
			dDeleted.Append(pOriginalTable->GetRowID(i));

	dResult.SetElement(FIELD_DELETED, dDeleted);

	return dResult;
	}

bool IAEONTable::FindColExt (CDatum dColID, int *retiCol) const

//	FindColExt
//
//	Looks for a column either by ID or by expression.

	{
	if (dColID.GetBasicType() == CDatum::typeString)
		return FindCol(dColID.AsStringView(), retiCol);
	else if (const CAEONExpression* pExpr = dColID.GetQueryInterface())
		{
		auto& Node = pExpr->GetRootNode();
		if (Node.iOp == CAEONExpression::EOp::Column)
			return FindCol(pExpr->GetColumnID(Node.iDataID), retiCol);
		else
			return false;
		}
	else
		return false;
	}

bool IAEONTable::FindColName (CStringView sCol, const TArray<CString>& Cols)

//	FindColName
//
//	Returns TRUE if the column name is in the list.

	{
	for (int i = 0; i < Cols.GetCount(); i++)
		if (strEqualsNoCase(sCol, Cols[i]))
			return true;

	return false;
	}

bool IAEONTable::FindRow (int iCol, CDatum dValue, int *retiRow) const

//	FindRow
//
//	Looks for the first row in which the given column has the given value. If
//	found, we return TRUE and the row index. Otherwise, FALSE.

	{
	for (int i = 0; i < GetRowCount(); i++)
		{
		if (dValue.IsEqualCompatible(GetFieldValue(i, iCol)))
			{
			if (retiRow)
				*retiRow = i;

			return true;
			}
		}

	return false;
	}

int IAEONTable::FindRowByMaxColValue (int iCol) const

//	FindRowByMaxColValue
//
//	Returns the row index with the maximum value in the given column. If the
//	table if empty or if there are nan values, we return -1.

	{
	if (iCol < 0 || iCol >= GetColCount())
		return -1;

	CDatum dCol = GetCol(iCol);
	if (dCol.IsNil())
		return -1;

	return dCol.FindMaxElement();
	}

int IAEONTable::FindRowByMinColValue (int iCol) const

//	FindRowByMinColValue
//
//	Returns the row index with the minimum value in the given column. If the
//	table if empty or if there are nan values, we return -1.

	{
	if (iCol < 0 || iCol >= GetColCount())
		return -1;

	CDatum dCol = GetCol(iCol);
	if (dCol.IsNil())
		return -1;

	return dCol.FindMinElement();
	}

IDatatype::SMemberDesc IAEONTable::GetColDesc (CDatum dTable, int iCol) const

//	GetColDesc
//
//	Returns the column descriptor by index.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return IDatatype::SMemberDesc();

	const IDatatype& Schema = pTable->GetSchema();
	if (iCol < 0 || iCol >= Schema.GetMemberCount())
		return IDatatype::SMemberDesc();

	return Schema.GetMember(iCol);
	}

TArray<int> IAEONTable::GetColsInSortedOrder () const

//	GetColsInSortedOrder
//
//	Returns columns in alphabetical order (so that we can compare columns 
//	without regard to order).

	{
	TSortMap<CString, int> ColMap;

	const IDatatype &Schema = GetSchema();
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		ColMap.SetAt(strToLower(Schema.GetMember(i).sID), i);

	TArray<int> Result;
	Result.InsertEmpty(ColMap.GetCount());
	for (int i = 0; i < ColMap.GetCount(); i++)
		Result[i] = ColMap[i];

	return Result;
	}

CDatum IAEONTable::GetElementAt_Impl (CDatum dTable, int iIndex) const
	{
	if (GetKeyType() != EKeyType::None)
		{
		CAEONTypeSystem TypeSystem;
		return GetElementAtKey(TypeSystem, dTable, iIndex);
		}

	if (iIndex >= 0 && iIndex < GetRowCount())
		return GetRow(iIndex);
	else if (iIndex < 0)
		{
		iIndex = GetRowCount() + iIndex;
		if (iIndex >= 0 && iIndex < GetRowCount())
			return GetRow(iIndex);
		else
			return CDatum();
		}
	else
		return CDatum();
	}

CDatum IAEONTable::GetElementAt_Impl (CAEONTypeSystem &TypeSystem, CDatum dTable, CDatum dIndex) const

//	GetElementAt
//
//	Handles array subscript

	{
	if (GetKeyType() != EKeyType::None)
		return GetElementAtKey(TypeSystem, dTable, dIndex);
	else
		return GetElementAtIndex(TypeSystem, dTable, dIndex);
	}

CDatum IAEONTable::GetElementAt2DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2) const
	{
	if (GetKeyType() == EKeyType::None)
		return CDatum();
	else
		{
		int iRow;
		if (!FindRowByID2(dIndex1, dIndex2, &iRow))
			return CDatum();

		return GetRow(iRow);
		}
	}

CDatum IAEONTable::GetElementAt3DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const
	{
	if (GetKeyType() == EKeyType::None)
		return CDatum();
	else
		{
		int iRow;
		if (!FindRowByID3(dIndex1, dIndex2, dIndex3, &iRow))
			return CDatum();

		return GetRow(iRow);
		}
	}

CDatum IAEONTable::GetElementAt2DI_Impl (CDatum dTable, int iIndex1, int iIndex2) const
	{
	return GetElementAt2DA_Impl(dTable, CDatum(iIndex1), CDatum(iIndex2));
	}

CDatum IAEONTable::GetElementAt3DI_Impl (CDatum dTable, int iIndex1, int iIndex2, int iIndex3) const
	{
	return GetElementAt3DA_Impl(dTable, CDatum(iIndex1), CDatum(iIndex2), CDatum(iIndex3));
	}

CDatum IAEONTable::GetElementAtIndex (CAEONTypeSystem& TypeSystem, CDatum dTable, CDatum dIndex) const

//	GetElementAtIndex
//
//	Returns the element at the given index (regardless of whether this table
//	has keys or not).

	{
	int iIndex = dIndex.AsArrayIndex(GetRowCount());
	if (iIndex >= 0 && iIndex < GetRowCount())
		return GetRow(iIndex);
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
				int iIndex = dIndex.GetElement(i).AsArrayIndex(GetRowCount());
				if (iIndex >= 0 && iIndex < GetRowCount())
					Subset.Rows.Insert(iIndex);
				}
			}
		else if (dIndex.IsStruct())
			{
			Subset.bAllRows = true;

			for (int i = 0; i < dIndex.GetCount(); i++)
				{
				int iCol;
				if (FindCol(dIndex.GetKey(i), &iCol))
					Subset.Cols.Insert(iCol);
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

		if (!IAEONTable::CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}
	else if (FindCol(dIndex.AsString(), &iIndex))
		return GetCol(iIndex);
	else
		return CDatum();
	}

CDatum IAEONTable::GetElementAtKey (CAEONTypeSystem& TypeSystem, CDatum dTable, CDatum dKey) const

//	GetElementAtKey
//
//	Indexed tables (tables with one or more key columns) use a different method
//	for subscript indexing.

	{
	EKeyType iKeyType = GetKeyType();
	if (iKeyType == EKeyType::None)
		return CDatum();

	if (dKey.IsStruct())
		{
		IAEONTable::SSubset Subset;
		Subset.bAllRows = true;

		for (int i = 0; i < dKey.GetCount(); i++)
			{
			int iCol;
			if (FindCol(dKey.GetKey(i), &iCol))
				Subset.Cols.Insert(iCol);
			}

		//	Create the subset

		CDatum dResult;
		if (!IAEONTable::CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}
	else if (dKey.IsContainer() 
			&& (iKeyType != EKeyType::Multiple || (dKey.GetCount() > 0 && dKey.GetElement(0).IsContainer())))
		{
		IAEONTable::SSubset Subset;

		for (int i = 0; i < dKey.GetCount(); i++)
			{
			CDatum dIndex = dKey.GetElement(i);
			if (DerefByPosition(iKeyType, dIndex))
				{
				int iIndex = dIndex.AsArrayIndex(GetRowCount());
				if (iIndex >= 0 && iIndex < GetRowCount())
					Subset.Rows.Insert(iIndex);
				}
			else
				{
				int iRow;
				if (FindRowByID(dIndex, &iRow))
					Subset.Rows.Insert(iRow);
				}
			}

		//	Create the subset

		CDatum dResult;
		if (!IAEONTable::CreateRef(TypeSystem, dTable, std::move(Subset), dResult))
			return CDatum();

		return dResult;
		}
	else
		{
		if (DerefByPosition(iKeyType, dKey))
			{
			int iIndex = dKey.AsArrayIndex(GetRowCount());
			if (iIndex >= 0 && iIndex < GetRowCount())
				return GetRow(iIndex);
			else
				return CDatum();
			}
		else
			{
			if (dKey.IsNil())
				return CDatum();

			int iRow;
			if (!FindRowByID(dKey, &iRow))
				return CDatum();

			return GetRow(iRow);
			}
		}
	}

TSortMap<CDatum, int> IAEONTable::GetKeyIndex (CDatum dTable) const
	{
	TSortMap<CDatum, int> Result;
	for (int i = 0; i < GetRowCount(); i++)
		Result.SetAt(dTable.GetKeyEx(i), i);
	return Result;
	}

CDatum IAEONTable::GetRow (int iRow) const

//	GetRow
//
//	Returns the row.

	{
	if (iRow < 0 || iRow >= GetRowCount())
		return CDatum();

	CDatum dRow(CDatum::typeStruct);

	const IDatatype &Schema = GetSchema();
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		dRow.SetElement(ColumnDesc.sID, GetFieldValue(iRow, i));
		}

	return dRow;
	}

bool IAEONTable::InsertColumnToSchema (CDatum dSchema, const CString& sName, CDatum dType, int iPos, CDatum& retdSchema, int* retiCol)

//	InsertColumnToSchema
//
//	Creates a new schema with a new column.

	{
	const IDatatype& OldSchema = dSchema;

	if (OldSchema.FindMember(sName) != -1)
		return false;

	if (dType.GetBasicType() != CDatum::typeDatatype)
		return false;

	int iNewCol;
	if (iPos >= 0)
		iNewCol = Min(iPos, OldSchema.GetMemberCount());
	else
		iNewCol = Max(0, OldSchema.GetMemberCount() + iPos + 1);

	TArray<IDatatype::SMemberDesc> Members;
	Members.InsertEmpty(OldSchema.GetMemberCount() + 1);

	int iDest = 0;
	for (int i = 0; i < OldSchema.GetMemberCount(); i++)
		{
		if (iDest == iNewCol)
			{
			Members[iDest++] = { IDatatype::EMemberType::InstanceVar, sName, dType };
			}

		Members[iDest++] = OldSchema.GetMember(i);
		}

	if (iDest < Members.GetCount())
		Members[iDest++] = { IDatatype::EMemberType::InstanceVar, sName, dType };

	CAEONTypeSystem TypeSystem;
	retdSchema = TypeSystem.AddAnonymousSchema(Members);

	if (retiCol)
		*retiCol = iNewCol;

	return true;
	}

bool IAEONTable::ParseColumnDesc (CDatum dDesc, IDatatype::SMemberDesc& retDesc, bool bAllowPartial)

//	ParseColumnDesc
//
//	Parses a column descriptor. We expect a struct with the following fields:
//
//	key: true|false
//	id: column name
//	type: column type
//	label: column label

	{
	retDesc.iType = IDatatype::EMemberType::None;
	if (dDesc.IsNil())
		return true;

	else if (dDesc.GetBasicType() == CDatum::typeStruct)
		{
		bool bKey = !dDesc.GetElement(FIELD_KEY).IsNil();
		retDesc.iType = bKey ? IDatatype::EMemberType::InstanceKeyVar : IDatatype::EMemberType::InstanceVar;
		retDesc.sID = dDesc.GetElement(FIELD_ID).AsString();
		if (retDesc.sID.IsEmpty() && !bAllowPartial)
			return false;

		retDesc.dType = dDesc.GetElement(FIELD_TYPE);
		if (retDesc.dType.IsNil())
			retDesc.dType = CAEONTypes::Get(IDatatype::ANY);
		else if (retDesc.dType.GetBasicType() != CDatum::typeDatatype)
			return false;

		retDesc.sLabel = dDesc.GetElement(FIELD_LABEL).AsString();
		retDesc.sFormat = dDesc.GetElement(FIELD_FORMAT).AsString();
		retDesc.iDisplay = IDatatype::ParseDisplay(dDesc.GetElement(FIELD_UI));

		return true;
		}
	else
		return false;
	}

CDatum IAEONTable::MakeSortDesc (CDatum dTable, const TArray<SSort>& SortOrder) const

//	MakeSortDesc
//
//	Generates a sort descriptor datum from an array.

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(SortOrder.GetCount());

	for (int i = 0; i < SortOrder.GetCount(); i++)
		{
		auto ColDesc = GetColDesc(dTable, SortOrder[i].iCol);
		if (SortOrder[i].iOrder == ESort::Descending)
			dResult.Append(strPattern(">%s", ColDesc.sID));
		else
			dResult.Append(ColDesc.sID);
		}

	return dResult;
	}

bool IAEONTable::ParseSort (CDatum dValue, TArray<SSort>& retSort) const

//	ParseSort
//
//	Parses a sort descriptor. We expect an array of sort elements. Each element
//	is a column name with the following format:
//
//	"colName" -> Ascending sort
//	"<colName" -> Ascending sort
//	">colName" -> Descending sort

	{
	retSort.DeleteAll();

	if (dValue.IsNil())
		{ }
	else if (dValue.IsArray())
		{
		if (dValue.GetCount() == 1 && dValue.GetElement(0).IsArray())
			{
			return ParseSort(dValue.GetElement(0), retSort);
			}

		retSort.InsertEmpty(dValue.GetCount());
		for (int i = 0; i < dValue.GetCount(); i++)
			{
			if (!ParseSort(dValue.GetElement(i), retSort[i]))
				return false;
			}
		}
	else
		{
		retSort.InsertEmpty();
		if (!ParseSort(dValue, retSort[0]))
			return false;
		}

	return true;
	}

bool IAEONTable::ParseSort (CDatum dValue, SSort& retSort) const

//	ParseSort
//
//	Parses a single sort column definition.

	{
	retSort.iOrder = ESort::Ascending;

	CStringView sValue = dValue;
	CString sColName;
	const char *pPos = sValue.GetParsePointer();
	if (*pPos == '<' || *pPos == '>')
		{
		if (*pPos == '>')
			retSort.iOrder = ESort::Descending;

		pPos++;
		while (*pPos != '\0' && strIsWhitespace(pPos))
			pPos++;

		sColName = CString(pPos);
		}
	else
		sColName = sValue;

	if (!FindCol(sColName, &retSort.iCol))
		return false;

	return true;
	}

bool IAEONTable::SetColumnSchema (CDatum dSchema, int iCol, IDatatype::SMemberDesc& ColDesc, CDatum& retdSchema)

//	SetColumnSchema
//
//	Returns a new schema in which the given column has been changed.

	{
	const IDatatype& OldSchema = dSchema;
	if (iCol < 0 || iCol >= OldSchema.GetMemberCount())
		return false;

	//	If we're changing the column ID, then make sure that it is unique.

	CString sOldColID;
	int iOldCol = (ColDesc.sID.IsEmpty() ? iCol : OldSchema.FindMember(ColDesc.sID));
	if (iOldCol != -1 && iOldCol != iCol)
		{
		//	We change the name of the conflicting column to a temporary name.
		//	this makes it easier for callers to change all columns in a loop.

		sOldColID = strPattern("%s_%x", OldSchema.GetMember(iOldCol).sID, mathRandom());
		}

	if (ColDesc.dType.GetBasicType() != CDatum::typeDatatype)
		return false;

	TArray<IDatatype::SMemberDesc> Members;
	Members.InsertEmpty(OldSchema.GetMemberCount());

	for (int i = 0; i < OldSchema.GetMemberCount(); i++)
		{
		Members[i] = OldSchema.GetMember(i);

		if (i == iCol)
			{
			if (!ColDesc.sID.IsEmpty())
				{
				Members[i].iType = ColDesc.iType;
				Members[i].sID = ColDesc.sID;
				Members[i].sLabel = ColDesc.sLabel;
				Members[i].sFormat = ColDesc.sFormat;
				}
			else if (!ColDesc.sLabel.IsEmpty())
				Members[i].sLabel = ColDesc.sLabel;

			Members[i].dType = ColDesc.dType;
			}
		else if (i == iOldCol)
			{
			Members[i].sID = sOldColID;
			}
		}

	CAEONTypeSystem TypeSystem;
	retdSchema = TypeSystem.AddAnonymousSchema(Members);

	return true;
	}

void IAEONTable::SetElementAt_Impl (CDatum dTable, CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets a row or a column

	{
	if (GetKeyType() != EKeyType::None)
		SetElementAtKey(dTable, dIndex, dDatum);
	else
		SetElementAtIndex(dTable, dIndex, dDatum);
	}

void IAEONTable::SetElementAt2DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2, CDatum dValue)
	{
	if (GetKeyType() != EKeyType::None)
		{
//		SetRowByID2(dIndex1, dIndex2, dValue);
		}
	}

void IAEONTable::SetElementAt3DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue)
	{
	if (GetKeyType() != EKeyType::None)
		{
//		SetRowByID3(dIndex1, dIndex2, dIndex3, dValue);
		}
	}

void IAEONTable::SetElementAtIndex (CDatum dTable, CDatum dIndex, CDatum dValue, int* retiIndex)

//	SetElementAtIndex
//
//	Sets the element at the given row index (even for keyed tables).

	{
	if (retiIndex)
		*retiIndex = -1;

	int iIndex = dIndex.AsArrayIndex(GetRowCount());
	if (iIndex >= 0 && iIndex < GetRowCount())
		{
		if (retiIndex)
			*retiIndex = iIndex;
		SetRow(iIndex, dValue);
		}
	else if (iIndex >= 0)
		{
		int iExtraRows = (iIndex - GetRowCount()) + 1;
		AppendEmptyRow(iExtraRows);
		SetRow(GetRowCount() - 1, dValue);
		}

	//	Otherwise, dIndex might be a column name.

	else if (FindCol(dIndex.AsString(), &iIndex))
		{
		if (dValue.GetCount() <= GetRowCount())
			{
			CDatum dCol = GetCol(iIndex);

			for (int i = 0; i < dValue.GetCount(); i++)
				dCol.SetElement(i, dValue.GetElement(i));

			for (int i = dValue.GetCount(); i < GetRowCount(); i++)
				dCol.SetElement(i, CDatum());
			}
		else
			{
			int iExtraRows = dValue.GetCount() - GetRowCount();
			AppendEmptyRow(iExtraRows);

			CDatum dCol = GetCol(iIndex);
			for (int i = 0; i < GetRowCount(); i++)
				dCol.SetElement(i, dValue.GetElement(i));
			}

		InvalidateKeys();
		}
	}

void IAEONTable::SetElementAtKey (CDatum dTable, CDatum dKey, CDatum dRow)

//	SetElementAtKey
//
//	Sets the element at the given index.

	{
	EKeyType iKeyType = GetKeyType();

	if (iKeyType == EKeyType::None)
		{ } 

	else if (DerefByPosition(iKeyType, dKey))
		{
		int iIndex = dKey.AsArrayIndex(GetRowCount());
		SetRow(iIndex, dRow);
		}
	else
		{
		SetRowByID(dKey, dRow);
		}
	}

bool IAEONTable::ValidateSort (const TArray<SSort>& Sort) const

//	ValidateSort
//
//	Returns TRUE if all the columns in the sort description are valid.

	{
	for (int i = 0; i < Sort.GetCount(); i++)
		{
		if (Sort[i].iCol < 0 || Sort[i].iCol >= GetColCount())
			return false;
		}

	return true;
	}
