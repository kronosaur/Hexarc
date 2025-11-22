//	CDBFormatXLS97Sheet.cpp
//
//	CDBFormatXLS97Sheet class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDBFormatXLS97Sheet::SRowAddr CDBFormatXLS97Sheet::AddRow (const CDBTable &Table, int iRow, const TArray<int> &ColOrder)

//	AddRow
//
//	Adds a row.

	{
	SRowBlock &Block = GetCurrentBlock();
	int iRowIndex = Block.Rows.GetCount();
	SRow *pNewRow = Block.Rows.Insert();
	pNewRow->iRow = m_iRowCount++;

	pNewRow->Cells.InsertEmpty(ColOrder.GetCount());
	for (int i = 0; i < ColOrder.GetCount(); i++)
		{
		pNewRow->Cells[i].iCol = i;
		pNewRow->Cells[i].pValue = &Table.GetField(ColOrder[i], iRow);
		}

	return { Block.iBlock, iRowIndex };
	}

CDBFormatXLS97Sheet::SRowAddr CDBFormatXLS97Sheet::AddRow (const TArray<CDBValue> &Row)

//	AddRow
//
//	Adds a row.

	{
	SRowBlock &Block = GetCurrentBlock();
	int iRowIndex = Block.Rows.GetCount();
	SRow *pNewRow = Block.Rows.Insert();
	pNewRow->iRow = m_iRowCount++;

	pNewRow->Cells.InsertEmpty(Row.GetCount());
	for (int i = 0; i < Row.GetCount(); i++)
		{
		pNewRow->Cells[i].iCol = i;
		pNewRow->Cells[i].pValue = &Row[i];
		}

	return { Block.iBlock, iRowIndex };
	}

const CDBValue &CDBFormatXLS97Sheet::GetCell (int iBlock, int iRow, int iCol, int *retiXF) const

//	GetCell
//
//	Returns a cell value.

	{
	if (iBlock < 0 || iBlock >= GetBlockCount())
		throw CException(errFail);

	if (iRow < 0 || iRow >= m_Blocks[iBlock].Rows.GetCount())
		throw CException(errFail);

	if (iCol < 0 || iCol >= m_Blocks[iBlock].Rows[iRow].Cells.GetCount())
		throw CException(errFail);

	if (retiXF)
		*retiXF = m_Blocks[iBlock].Rows[iRow].Cells[iCol].iXF;

	return *m_Blocks[iBlock].Rows[iRow].Cells[iCol].pValue;
	}

CDBFormatXLS97Sheet::SCell &CDBFormatXLS97Sheet::GetCell (const SRowAddr &Row, int iCol)

//	GetCell
//
//	Returns the cell.

	{
	if (Row.iBlock < 0 || Row.iBlock >= m_Blocks.GetCount())
		throw CException(errFail);

	if (Row.iRowIndex < 0 || Row.iRowIndex >= m_Blocks[Row.iBlock].Rows.GetCount())
		throw CException(errFail);

	if (iCol < 0 || iCol >= m_Blocks[Row.iBlock].Rows[Row.iRowIndex].Cells.GetCount())
		throw CException(errFail);

	return m_Blocks[Row.iBlock].Rows[Row.iRowIndex].Cells[iCol];
	}

CDBFormatXLS97Sheet::SRowBlock &CDBFormatXLS97Sheet::GetCurrentBlock ()
	{
	if (m_Blocks.GetCount() == 0)
		{
		m_Blocks.Insert();
		return m_Blocks[0];
		}
	else if (m_Blocks[m_Blocks.GetCount() - 1].Rows.GetCount() >= RECORDS_PER_BLOCK)
		{
		int iBlock = m_Blocks.GetCount();
		SRowBlock *pBlock = m_Blocks.Insert();
		pBlock->iBlock = iBlock;

		return *pBlock;
		}
	else
		{
		return m_Blocks[m_Blocks.GetCount() - 1];
		}
	}

