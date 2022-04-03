//	IAEONTable.cpp
//
//	IAEONTable class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_SCHEMA,		"Unable to create schema.");

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
		if (Subset.Cols[i] >= 0 && Subset.Cols[i] < Include.GetCount())
			Include[Subset.Cols[i]] = true;
		}

	//	We treat each field of the struct as a column.
	//	(We also reinitialize the subset definition, to guarantee that they 
	//	match the table schema.)

	Subset.Cols.DeleteAll();

	TArray<IDatatype::SMemberDesc> Columns;
	for (int i = 0; i < Include.GetCount(); i++)
		{
		if (Include[i])
			{
			Columns.Insert(Schema.GetMember(i));
			Subset.Cols.Insert(i);
			}
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
