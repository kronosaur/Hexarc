//	CDBTable.cpp
//
//	CDBTable class
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CDBTable::AddCol (const CDBColumnDef &ColDef)

//	AddCol
//
//	Adds a new column at the end. If we fail adding the column, then we return
//	FALSE (e.g., if we have a duplicate column name).

	{
	//	Make sure this is not a duplicate column

	if (FindColByName(ColDef.GetName()) != -1)
		return false;

	//	If we have no rows, then it is easy to add a new column.

	if (m_Rows.GetCount() == 0)
		{
		m_Cols.Insert(ColDef);

		//	Make sure the ordinal matches. We need to do this because we use the
		//	ordinals when writing back to OLEDB.

		m_Cols[m_Cols.GetCount() - 1].SetOrdinal(m_Cols.GetCount());

		InvalidateColIndex();
		}

	//	Otherwise, we need to move data around.

	else
		{
		throw CException(errFail);
		return false;
		}

	//	Done

	return true;
	}

bool CDBTable::AddColUnique (const CDBColumnDef &ColDef)

//	AddColUnique
//
//	Adds a new column at the end, generating a new column name if necessary.

	{
	CString sColName = ColDef.GetName();
	int iSuffix = 2;

	//	If there is already a column with this name, generate a new name

	while (FindColByName(sColName) != -1)
		{
		sColName = strPattern("%s_%d", ColDef.GetName(), iSuffix++);
		}

	//	Add the column

	if (strEquals(sColName, ColDef.GetName()))
		return AddCol(ColDef);
	else
		{
		CDBColumnDef NewCol(ColDef);
		NewCol.SetID(sColName);
		return AddCol(NewCol);
		}
	}

int CDBTable::AddRow (void)

//	AddRow
//
//	Adds a new row at the end and returns the row index of the new column.

	{
	int iRowIndex = GetRowCount();

	m_Rows.GrowToFit(GetColCount());
	m_Rows.InsertEmpty(GetColCount());

	return iRowIndex;
	}

void CDBTable::CleanUp (void)

//	CleanUp
//
//	Empty the table.

	{
	m_Cols.DeleteAll();
	m_Rows.DeleteAll();

	m_ColIndex.DeleteAll();
	}

int CDBTable::FindColByName (const CString &sName) const

//	FindColByName
//
//	Returns the index of the given column (by name). If we cannot find the 
//	column, we return -1.

	{
	InitColIndex();

	int *pCol = m_ColIndex.GetAt(strToLower(sName))	;
	if (pCol == NULL)
		return -1;

	return *pCol;
	}

bool CDBTable::FindColsByName (const TArray<CString> &Names, TArray<int> &retIndices, CString *retsError) const

//	FindColsByName
//
//	Looks up all the column names and returns an array with column indices.
//	We return TRUE if all names were found.

	{
	retIndices.DeleteAll();
	retIndices.InsertEmpty(Names.GetCount());

	bool bAllFound = true;
	for (int i = 0; i < Names.GetCount(); i++)
		{
		retIndices[i] = FindColByName(Names[i]);
		if (retIndices[i] == -1)
			{
			if (retsError && retsError->IsEmpty())
				*retsError = strPattern("Unknown column: %s", Names[i]);

			bAllFound = false;
			}
		}

	return bAllFound;
	}

const CDBValue &CDBTable::GetField (int iCol, int iRow) const

//	GetField
//
//	Returns the field value at the given col and row. If we're out of bounds, we 
//	return Null.

	{
	if (iRow < 0 || iRow >= GetRowCount() || iCol < 0 || iCol >= GetColCount())
		return CDBValue::Null;

	return m_Rows[iRow * GetColCount() + iCol];
	}

CDBValue *CDBTable::GetField (int iCol, int iRow)

//	GetField
//
//	Returns a pointer to the field at the given col and row. If we're out of 
//	bounds, we return NULL.
//
//	NOTE: There is no guarantee this pointer will be valid across class to
//	AddRow or AddCol.

	{
	if (iRow < 0 || iRow >= GetRowCount() || iCol < 0 || iCol >= GetColCount())
		return NULL;

	return &m_Rows[iRow * GetColCount() + iCol];
	}

void CDBTable::InitColIndex (void) const

//	InitColIndex
//
//	Initialize the column name index.

	{
	if (m_bColIndexValid)
		return;

	m_ColIndex.DeleteAll();
	for (int i = 0; i < m_Cols.GetCount(); i++)
		{
		m_ColIndex.Insert(m_Cols[i].GetID(), i);

		//	See if we should add display name

		CString sDisplayName = strToLower(m_Cols[i].GetDisplayName());
		if (!sDisplayName.IsEmpty() && !strEquals(sDisplayName, m_Cols[i].GetID()))
			m_ColIndex.Insert(m_Cols[i].GetDisplayName(), i);
		}

	m_bColIndexValid = true;
	}

void CDBTable::SetColumnDefs (const TArray<CDBColumnDef> &Cols)

//	SetColumnDefs
//
//	Set column definitions.

	{
	if (GetRowCount() != 0)
		{
		//	Not Yet Implemented
		ASSERT(false);
		return;
		}

	m_Cols = Cols;

	//	Fix up the ordinals. The ordinal is used in OLEDB, so we need it to 
	//	match.

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].SetOrdinal(i + 1);

	InvalidateColIndex(); 
	}

bool CDBTable::SetField (int iCol, int iRow, const CDBValue &Value)

//	SetField
//
//	Sets a field at the given row and column. If the position is out of bounds,
//	we return FALSE.

	{
	CDBValue *pField = GetField(iCol, iRow);
	if (pField == NULL)
		return false;

	*pField = Value;
	return true;
	}

TArray<int> CDBTable::Sort (const TArray<int> &Cols) const

//	Sort
//
//	Returns a list of row indices in sorted order.

	{
	TArray<int> Result;
	Result.InsertEmpty(GetRowCount());
	for (int i = 0; i < GetRowCount(); i++)
		Result[i] = i;

	//	We sort in place

	Result.Sort([this, &Cols](const int &iLeft, const int &iRight)
		{
		for (int i = 0; i < Cols.GetCount(); i++)
			{
			int iCol = Cols[i];
			int iCompare = CDBValue::Compare(GetField(iCol, iLeft), GetField(iCol, iRight));
			if (iCompare != 0)
				return iCompare;
			}

		return 0;
		});

	return Result;
	}

TArray<int> CDBTable::Sort (const TArray<int> &Rows, const TArray<int> &Cols) const

//	Sort
//
//	Sorts the given rows by the given columns and returns the row indices in 
//	sorted order.

	{
	//	We sort in place

	TArray<int> Result = Rows;
	Result.Sort([this, &Cols](const int &iLeft, const int &iRight)
		{
		for (int i = 0; i < Cols.GetCount(); i++)
			{
			int iCol = Cols[i];
			int iCompare = CDBValue::Compare(GetField(iCol, iLeft), GetField(iCol, iRight));
			if (iCompare != 0)
				return iCompare;
			}

		return 0;
		});

	return Result;
	}

void CDBTable::TakeHandoff (CDBTable &Src)

//	TakeHandoff
//
//	Take handoff

	{
	m_Cols.TakeHandoff(Src.m_Cols);
	m_Rows.TakeHandoff(Src.m_Rows);
	m_bColIndexValid = Src.m_bColIndexValid;
	m_ColIndex.TakeHandoff(Src.m_ColIndex);
	}

