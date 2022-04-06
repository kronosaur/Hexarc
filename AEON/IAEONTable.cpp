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
