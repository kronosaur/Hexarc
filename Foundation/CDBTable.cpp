//	CDBTable.cpp
//
//	CDBTable class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

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
		InitColIndex();
		}

	//	Otherwise, we need to move data around.

	else
		{
		ASSERT(false);
		return false;
		}

	//	Done

	return true;
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
	int *pCol = m_ColIndex.GetAt(strToLower(sName))	;
	if (pCol == NULL)
		return -1;

	return *pCol;
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

void CDBTable::InitColIndex (void)

//	InitColIndex
//
//	Initialize the column name index.

	{
	int i;

	m_ColIndex.DeleteAll();
	for (i = 0; i < m_Cols.GetCount(); i++)
		{
		m_ColIndex.Insert(strToLower(m_Cols[i].GetName()), i);
		}
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
