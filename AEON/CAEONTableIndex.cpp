//	CAEONTableIndex.cpp
//
//	CAEONTableIndex class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const CAEONTableIndex CAEONTableIndex::Null;

void CAEONTableIndex::Add (CDatum dTable, int iRow)

//	Add
//
//	Add an entry to the index.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return;

	m_Index.SetAt(GetIndexKey(*pTable, iRow), iRow);
	}

CString CAEONTableIndex::AsIndexKeyFromRow (CDatum dTable, CDatum dValue) const

//	AsIndexKeyFromRow
//
//	Returns an index key depending on the value. The value is either a row 
//	struct or an array of values or a single value.

	{
	if (m_Cols.GetCount() == 1)
		{
		return AsIndexKeyFromValue(dValue);
		}
	else if (m_Cols.GetCount() > 1)
		{
		if (dValue.IsArray())
			{
			CStringBuffer Result;
			Result.Write(AsIndexKeyFromValue(dValue.GetElement(0)));
			for (int i = 1; i < m_Cols.GetCount(); i++)
				{
				Result.WriteChar('/');
				Result.Write(AsIndexKeyFromValue(dValue.GetElement(i)));
				}

			return CString(std::move(Result));
			}
		else
			{
			CDatum dSchema = dTable.GetDatatype();
			const IDatatype& Schema = dSchema;

			CStringBuffer Result;
			for (int i = 0; i < m_Cols.GetCount(); i++)
				{
				if (m_Cols[i] >= Schema.GetMemberCount())
					return NULL_STR;

				if (i != 0)
					Result.WriteChar('/');

				Result.Write(AsIndexKeyFromValue(dValue.GetElement(Schema.GetMember(m_Cols[i]).sName)));
				}

			return CString(std::move(Result));
			}
		}
	else
		return NULL_STR;
	}

CString CAEONTableIndex::AsIndexKeyFromValue (CDatum dValue)

//	AsIndexKey
//
//	Returns as a key.

	{
	switch (dValue.GetBasicType())
		{
		case CDatum::typeString:
			return (const CString&)dValue;

		case CDatum::typeDateTime:
			return ((const CDateTime&)dValue).Format(CDateTime::formatISO8601);

		default:
			return dValue.AsString();
		}
	}

int CAEONTableIndex::Find (CDatum dTable, CDatum dValue) const

//	Find
//
//	Find an entry.

	{
	int* pRow = m_Index.GetAt(AsIndexKeyFromRow(dTable, dValue));
	if (!pRow)
		return -1;

	return *pRow;
	}

CAEONTableIndex::EFindResult CAEONTableIndex::FindOrAdd (CDatum dTable, CDatum dKey, int iNewRow, int *retiRow)

//	FindOrAdd
//
//	Looks for the given key. If it exists, we return ::Found and retiRow is
//	initialized with the row index. Otherwise, we try to add a new entry using
//	iNewRow and return ::Added.

	{
	EFindResult iResult;

	bool bAdded;
	int* pRow = m_Index.SetAt(AsIndexKeyFromRow(dTable, dKey), &bAdded);
	if (bAdded)
		{
		*pRow = iNewRow;
		iResult = EFindResult::Added;
		}
	else
		{
		iResult = EFindResult::Found;
		}

	if (retiRow)
		*retiRow = *pRow;

	return iResult;
	}

CString CAEONTableIndex::GetIndexKey (const IAEONTable& Table, int iRow) const

//	GetIndexKey
//
//	Returns the index key.

	{
	if (m_Cols.GetCount() == 1)
		return AsIndexKeyFromValue(Table.GetFieldValue(iRow, m_Cols[0]));
	else if (m_Cols.GetCount() > 1)
		{
		CStringBuffer Result;
		Result.Write(AsIndexKeyFromValue(Table.GetFieldValue(iRow, m_Cols[0])));
		for (int i = 1; i < m_Cols.GetCount(); i++)
			{
			Result.WriteChar('/');
			Result.Write(AsIndexKeyFromValue(Table.GetFieldValue(iRow, m_Cols[i])));
			}

		return CString(std::move(Result));
		}
	else
		return NULL_STR;
	}

CDatum CAEONTableIndex::GetValueFromKey (CDatum dTable, CDatum dKey, const CString& sCol) const

//	GetValueFromKey
//
//	Returns the column value from a key.

	{
	if (m_Cols.GetCount() == 1)
		{
		return dKey;
		}
	else if (m_Cols.GetCount() > 1)
		{
		if (dKey.IsArray())
			{
			const IDatatype& Schema = dTable.GetDatatype();
			int iCol = Schema.FindMember(sCol);
			if (iCol == -1)
				return CDatum();

			int iPos;
			if (!m_Cols.Find(iCol, &iPos))
				return CDatum();

			return dKey.GetElement(iPos);
			}
		else
			{
			return dKey.GetElement(sCol);
			}
		}
	else
		return CDatum();
	}

void CAEONTableIndex::Init (CDatum dTable, const TArray<int>& Cols)

//	Init
//
//	Initialize the index from the given table.

	{
	m_Cols = Cols;
	m_Index.DeleteAll();
	if (m_Cols.GetCount() == 0)
		return;

	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return;

	for (int iRow = 0; iRow < pTable->GetRowCount(); iRow++)
		{
		m_Index.SetAt(GetIndexKey(*pTable, iRow), iRow);
		}
	}

void CAEONTableIndex::Remove (CDatum dTable, int iRow)

//	Remove
//
//	Remove the given row from the index.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return;

	m_Index.DeleteAt(GetIndexKey(*pTable, iRow));
	}
