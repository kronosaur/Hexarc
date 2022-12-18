//	IAEONTable.cpp
//
//	IAEONTable class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_NAME,						"name");

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_SCHEMA,		"Unable to create schema.");
DECLARE_CONST_STRING(ERR_INVALID_SCHEMA_DESC,			"Invalid schema desc.");
DECLARE_CONST_STRING(ERR_CANT_COMBINE_KEYS_DONT_MATCH,	"Unable to combine schemas because they have different keys.");

bool IAEONTable::CombineSchema (CDatum dSchema1, CDatum dSchema2, CDatum& retdSchema)

//	CombineSchema
//
//	Creates a new schema that is a superset of the two given schemas. If the two
//	schemas are not compatible, we return FALSE.

	{
	const IDatatype &Schema1 = dSchema1;
	const IDatatype &Schema2 = dSchema2;

	//	If one schema has no members, then we just return the other one.

	if (Schema2.GetMemberCount() == 0)
		{
		retdSchema = dSchema1;
		return true;
		}
	else if (Schema1.GetMemberCount() == 0)
		{
		retdSchema = dSchema2;
		return true;
		}

	//	If the two schemas equal each other, then just return one.

	else if (Schema1 == Schema2)
		{
		retdSchema = dSchema1;
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
		ByName.SetAt(strToLower(Member.sName), iIndex);
		}

	//	Now add the second schema.

	for (int i = 0; i < Schema2.GetMemberCount(); i++)
		{
		auto Member2 = Schema2.GetMember(i);

		//	If this column is already in schema1, then we continue.

		int *pCol = ByName.GetAt(strToLower(Member2.sName));
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
	retdSchema = TypeSystem.AddAnonymousSchema(Columns);
	if (retdSchema.IsNil())
		{
		retdSchema = ERR_UNABLE_TO_CREATE_SCHEMA;
		return false;
		}

	return true;
	}

CDatum IAEONTable::CreateColumn (CDatum dType)

//	CreateColumn
//
//	Creates an empty column.

	{
	const IDatatype &ColSchema = dType;
	switch (ColSchema.GetCoreType())
		{
		case IDatatype::INT_32:
			return CDatum::VectorOf(CDatum::typeInteger32);

		case IDatatype::INT_IP:
			return CDatum::VectorOf(CDatum::typeIntegerIP);

		case IDatatype::FLOAT_64:
			return CDatum::VectorOf(CDatum::typeDouble);

		case IDatatype::STRING:
			return CDatum::VectorOf(CDatum::typeString);

		default:
			//	For anything else, we create a generic array.

			return CDatum::VectorOf(CDatum::typeUnknown);
		}
	}

bool IAEONTable::CreateRef (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue)

//	CreateRef
//
//	Creates a table reference.

	{
	return CAEONTableRef::Create(TypeSystem, dTable, std::move(Subset), retdValue);
	}

bool IAEONTable::CreateSchema (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset& Subset, CDatum& retdSchema)

//	CreateSchema
//
//	Creates a subset schema.

	{
	//	Otherwise we need to construct a new schema.

	const IDatatype &Schema = dTable.GetDatatype();

	//	If no columns listed, then we take all the columns in the source table,
	//	which means the schema is the same.

	if (Subset.Cols.GetCount() == 0)
		{
		retdSchema = dTable.GetDatatype();
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

	retdSchema = TypeSystem.AddAnonymousSchema(Columns);
	if (retdSchema.IsNil())
		{
		retdSchema = ERR_UNABLE_TO_CREATE_SCHEMA;
		return false;
		}

	return true;
	}

bool IAEONTable::CreateSchemaFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdSchema)

//	CreateSchemaFromDesc
//
//	Creates a schema from a descriptor. A schema descriptor is an array of 
//	structs; each struct has the following fields:
//
//	datatype (required)
//	name (required)
//	label (optional, defaults to name)
//	description (optional)
//
//	On error, we return FALSE and retdSchema is the error.

	{
	if (dSchemaDesc.GetBasicType() == CDatum::typeDatatype)
		{
		const IDatatype& Type = dSchemaDesc;
		if (Type.GetClass() == IDatatype::ECategory::Schema)
			{
			retdSchema = dSchemaDesc;
			return true;
			}
		else
			{
			retdSchema = ERR_INVALID_SCHEMA_DESC;
			return false;
			}
		}

	if (!dSchemaDesc.IsContainer() || dSchemaDesc.GetCount() == 0)
		{
		retdSchema = ERR_INVALID_SCHEMA_DESC;
		return false;
		}

	TArray<IDatatype::SMemberDesc> Columns;
	TSortMap<CString, int> DuplicateCheck;
	for (int i = 0; i < dSchemaDesc.GetCount(); i++)
		{
		CDatum dColDesc = dSchemaDesc.GetElement(i);

		IDatatype::SMemberDesc ColDesc;
		ColDesc.iType = IDatatype::EMemberType::InstanceVar;
		ColDesc.sName = dColDesc.GetElement(FIELD_NAME).AsString();
		CString sID = strToLower(ColDesc.sName);

		if (ColDesc.sName.IsEmpty() || DuplicateCheck.GetAt(sID))
			{
			retdSchema = ERR_INVALID_SCHEMA_DESC;
			return false;
			}

		ColDesc.dType = dColDesc.GetElement(FIELD_DATATYPE);
		if (ColDesc.dType.GetBasicType() != CDatum::typeDatatype)
			{
			retdSchema = ERR_INVALID_SCHEMA_DESC;
			return false;
			}

		DuplicateCheck.SetAt(sID, i);
		Columns.Insert(ColDesc);
		}

	//	Create a new schema
	
	retdSchema = TypeSystem.AddAnonymousSchema(Columns);
	if (retdSchema.IsNil())
		{
		retdSchema = ERR_UNABLE_TO_CREATE_SCHEMA;
		return false;
		}

	return true;
	}

bool IAEONTable::FindRow (int iCol, CDatum dValue, int *retiRow) const

//	FindRow
//
//	Looks for the first row in which the given column has the given value. If
//	found, we return TRUE and the row index. Otherwise, FALSE.

	{
	for (int i = 0; i < GetRowCount(); i++)
		{
		if (dValue.IsEqual(GetFieldValue(i, iCol)))
			{
			if (retiRow)
				*retiRow = i;

			return true;
			}
		}

	return false;
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

		dRow.SetElement(ColumnDesc.sName, GetFieldValue(iRow, i));
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
