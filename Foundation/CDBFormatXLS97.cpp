//	CDBFormatXLS97.cpp
//
//	CDBFormatXLS97 class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.
//
//	REFERENCES
//
//	https://www.codeproject.com/Articles/33850/Generate-Excel-files-without-using-Microsoft-Excel
//	http://download.microsoft.com/download/0/B/E/0BE8BDD7-E5E8-422A-ABFD-4342ED7AD886/Excel97-2007BinaryFileFormat(xls)Specification.pdf

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_COLUMN_ORDER,					"columnOrder");
DECLARE_CONST_STRING(FIELD_SHEET_BY,						"sheetBy");
DECLARE_CONST_STRING(FIELD_SORT_ORDER,						"sortOrder");

DECLARE_CONST_STRING(STR_BLANK,								"(Blank)");
DECLARE_CONST_STRING(STR_COMMA,								",");

void CDBFormatXLS97::AddHeader (CDBFormatXLS97Sheet &Blocks, const CString &sSheetName)

//	AddHeader
//
//	Adds the header rows to the block data.

	{
	//	If we're using a table for headers, then do it.

	if (!m_Options.HeaderRows.IsEmpty())
		{
		//	If we're splitting up by sheets, then we need to filter.

		if (m_Options.iSheetColumn != -1)
			{
			for (int i = 0; i < m_Options.HeaderRows.GetRowCount(); i++)
				{
				CString sSheet = GetDataValue(m_Options.HeaderRows.GetField(m_Options.iSheetColumn, i));
				if (sSheet.IsEmpty())
					sSheet = STR_BLANK;

				if (!strEqualsNoCase(sSheet, sSheetName))
					continue;

				InitRowData(Blocks, m_Options.HeaderRows, i, m_Options.HeaderColOrder);
				}
			}

		//	Otherwise, just emit all rows in the table. We assume that header 
		//	rows have the exact same table structure.

		else
			{
			for (int i = 0; i < m_Options.HeaderRows.GetRowCount(); i++)
				{
				InitRowData(Blocks, m_Options.HeaderRows, i, m_Options.HeaderColOrder);
				}
			}
		}

	//	Otherwise, just use column names.

	else
		{
		if (m_Header.GetCount() == 0)
			{
			m_Header.InsertEmpty(m_Options.ColOrder.GetCount());
			for (int i = 0; i < m_Options.ColOrder.GetCount(); i++)
				{
				m_Header[i] = m_Table.GetCol(m_Options.ColOrder[i]).GetName();
				}
			}

		InitRowData(Blocks, m_Header);
		}
	}

void CDBFormatXLS97::AddToSST (const CDBValue &Value)

//	AddToSST
//
//	Adds to string table, if necessary.

	{
	m_SST.AddString(Value.AsString());
	}

CBuffer CDBFormatXLS97::Encode (const CString &sString, int iLenFieldSize)

//	Encode
//
//	Write out a buffer for a string.

	{
	CBuffer Result;
	CString16 UString(sString);

	if (UString.IsASCII())
		{
		if (iLenFieldSize == 1)
			{
			BYTE byLen = (BYTE)sString.GetLength();
			Result.Write(&byLen, 1);
			}
		else
			{
			WORD wChars = sString.GetLength();
			Result.Write(&wChars, sizeof(wChars));
			}

		BYTE byFlag = 0;
		Result.Write(&byFlag, 1);

		Result.Write(sString);
		}
	else
		{
		int iLen = UString.GetLength();

		if (iLenFieldSize == 1)
			{
			BYTE byLen = (BYTE)iLen;
			Result.Write(&byLen, 1);
			}
		else
			{
			WORD wChars = (WORD)iLen;
			Result.Write(&wChars, sizeof(wChars));
			}

		BYTE byFlag = 1;
		Result.Write(&byFlag, 1);

		Result.Write((LPTSTR)UString, iLen * sizeof(TCHAR));
		}

	return Result;
	}

CString CDBFormatXLS97::GetDataValue (const CDBValue &Value)

//	GetDataValue
//
//	Returns the value, ready for outputting to XML.

	{
	switch (Value.GetType())
		{
		case CDBValue::typeDouble:
			{
			//	We can't encode NaN in a number format, so we just return blank.

			if (Value.IsNaN())
				return NULL_STR;
			else
				return CXMLElement::MakeAttribute(Value.AsString());
			}

		default:
			return CXMLElement::MakeAttribute(Value.AsString());
		}
	}

CString CDBFormatXLS97::GetSortKey (const CDBValue &Value)

//	GetSortKey
//
//	Returns a sort key.
//	LATER: We should be smarter and use the Compare function instead of 
//	converting to a string.

	{
	switch (Value.GetType())
		{
		case CDBValue::typeDateTime:
			{
			const CDateTime DateTime = Value;
			return strPattern("%010d.%010d", DateTime.DaysSince1AD(), DateTime.MillisecondsSinceMidnight());
			}

		default:
			return Value.AsString();
		}
	}

void CDBFormatXLS97::InitData ()

//	InitData
//
//	Initialize m_Data

	{
	//	If we split up data across sheets, then we need to category each row.

	if (m_Options.iSheetColumn != -1)
		{
		TSortMap<CString, TArray<int>> Sheets;
		for (int i = 0; i < m_Table.GetRowCount(); i++)
			{
			CString sSheet = GetDataValue(m_Table.GetField(m_Options.iSheetColumn, i));
			if (sSheet.IsEmpty())
				sSheet = STR_BLANK;

			bool bNew;
			auto *pRows = Sheets.SetAt(sSheet, &bNew);
			pRows->Insert(i);
			}

		TSortMap<CString, int> SheetSortOrder;
		for (int i = 0; i < Sheets.GetCount(); i++)
			{
			int iKeyRow = Sheets[i][0];
			SheetSortOrder.Insert(MakeSortKey(m_Options.SheetSortOrder, iKeyRow, GetDataValue(m_Table.GetField(m_Options.iSheetColumn, iKeyRow))), i);
			}

		//	Write all the sheets

		for (int i = 0; i < SheetSortOrder.GetCount(); i++)
			{
			int iSheet = SheetSortOrder[i];
			CDBFormatXLS97Sheet *pNewSheet = m_Data.Insert();

			if (m_Options.SortOrder.GetCount() > 0)
				{
				TArray<int> Sorted = SortRows(Sheets[iSheet], m_Options.SortOrder);

				InitSheetData(*pNewSheet, Sheets.GetKey(iSheet), Sorted);
				}
			else
				{
				InitSheetData(*pNewSheet, Sheets.GetKey(iSheet), Sheets[iSheet]);
				}
			}
		}

	//	Otherwise, we write a single sheet with all rows and columns

	else
		{
		CDBFormatXLS97Sheet *pNewSheet = m_Data.Insert();

		//	Write the sheet

		if (m_Options.SortOrder.GetCount() > 0)
			{
			TArray<int> Sorted = SortRows(TArray<int>(), m_Options.SortOrder);

			InitSheetData(*pNewSheet, m_Table.GetName(), Sorted);
			}
		else
			{
			InitSheetData(*pNewSheet, m_Table.GetName(), TArray<int>());
			}
		}
	}

void CDBFormatXLS97::InitRowData (CDBFormatXLS97Sheet &SheetData, const CDBTable &Table, int iRow, const TArray<int> ColOrder)

//	InitRowData
//
//	Initializes a row.

	{
	SheetData.AddRow(Table, iRow, ColOrder);

	//	For each string value, add to the string table.

	for (int i = 0; i < ColOrder.GetCount(); i++)
		{
		const CDBValue &Value = Table.GetField(ColOrder[i], iRow);
		AddToSST(Value);
		}
	}

void CDBFormatXLS97::InitRowData (CDBFormatXLS97Sheet &SheetData, const TArray<CDBValue> &Row)

//	InitRowData
//
//	Initializes a row.

	{
	SheetData.AddRow(Row);

	//	Add to string table.

	for (int i = 0; i < Row.GetCount(); i++)
		{
		AddToSST(Row[i]);
		}
	}

void CDBFormatXLS97::InitSheetData (CDBFormatXLS97Sheet &SheetData, const CString &sSheetName, const TArray<int> &Rows)

//	InitSheetData
//
//	Initializes a sheet.

	{
	SheetData.SetName(sSheetName);
	AddHeader(SheetData, sSheetName);

	if (Rows.GetCount() == 0)
		{
		for (int i = 0; i < m_Table.GetRowCount(); i++)
			{
			InitRowData(SheetData, m_Table, i, m_Options.ColOrder);
			}
		}

	//	Otherwise, write the specified rows.

	else
		{
		for (int i = 0; i < Rows.GetCount(); i++)
			{
			InitRowData(SheetData, m_Table, Rows[i], m_Options.ColOrder);
			}
		}
	}

CString CDBFormatXLS97::MakeSortKey (const TArray<int> &SortOrder, int iRow, const CString &sDefault)

//	MakeSortKey
//
//	Generates a sort key for the given set of columns.

	{
	CString sKey;

	for (int i = 0; i < SortOrder.GetCount(); i++)
		{
		if (i != 0)
			sKey += CString("|");

		sKey += GetSortKey(m_Table.GetField(SortOrder[i], iRow));
		}

	if (sKey.IsEmpty())
		return sDefault;
	else
		return sKey;
	}

bool CDBFormatXLS97::ParseOptions (const CDBTable &Table, const CDBValue &Value, SOptions &retOptions, CString *retsError)

//	ParseOptions
//
//	Parses options.

	{
	//	For backwards compatibility, we support a string.

	if (Value.GetType() == CDBValue::typeString)
		{
		CDBValue NewOptions(CDBValue::typeStruct);
		NewOptions.SetElement(FIELD_SHEET_BY, Value);
		return ParseOptions(Table, NewOptions, retOptions, retsError);
		}

	//	Otherwise, we expect a structure.

	retOptions.ColOrder.DeleteAll();
	const CDBValue &Columns = Value.GetElement(FIELD_COLUMN_ORDER);
	if (!Columns.IsNil())
		{
		TArray<CString> ColumnOrder;
		strSplit(Columns, STR_COMMA, &ColumnOrder, -1, SSP_FLAG_NO_EMPTY_ITEMS);

		retOptions.ColOrder.InsertEmpty(ColumnOrder.GetCount());
		for (int i = 0; i < ColumnOrder.GetCount(); i++)
			{
			CString sCol = strClean(ColumnOrder[i]);
			int iIndex = Table.FindColByName(sCol);
			if (iIndex == -1)
				{
				if (retsError) *retsError = strPattern("Unknown column: %s", sCol);
				return false;
				}

			retOptions.ColOrder[i] = iIndex;
			}
		}
	else
		{
		retOptions.ColOrder.InsertEmpty(Table.GetColCount());
		for (int i = 0; i < retOptions.ColOrder.GetCount(); i++)
			retOptions.ColOrder[i] = i;
		}

	//	Sort order

	retOptions.SortOrder.DeleteAll();
	const CDBValue &Sort = Value.GetElement(FIELD_SORT_ORDER);
	if (!Sort.IsNil())
		{
		TArray<CString> SortOrder;
		strSplit(Sort, STR_COMMA, &SortOrder, -1, SSP_FLAG_NO_EMPTY_ITEMS);

		retOptions.SortOrder.InsertEmpty(SortOrder.GetCount());
		for (int i = 0; i < SortOrder.GetCount(); i++)
			{
			CString sCol = strClean(SortOrder[i]);
			int iIndex = Table.FindColByName(sCol);
			if (iIndex == -1)
				{
				if (retsError) *retsError = strPattern("Unknown column: %s", sCol);
				return false;
				}

			retOptions.SortOrder[i] = iIndex;
			}
		}

	//	Sheets

	CString sSheetBy = Value.GetElement(FIELD_SHEET_BY);
	if (!sSheetBy.IsEmpty())
		{
		retOptions.iSheetColumn = Table.FindColByName(sSheetBy);
		if (retOptions.iSheetColumn == -1)
			{
			if (retsError) *retsError = strPattern("Unknown column: %s", sSheetBy);
			return false;
			}
		}

	return true;
	}

void CDBFormatXLS97::RequestFixup (EFixup iType, int iOffset, DWORD dwData)

//	RequestFixup
//
//	Fixup.

	{
	auto *pEntry = m_Fixups.Insert();
	pEntry->iType = iType;
	pEntry->iOffset = iOffset;
	pEntry->dwData = dwData;
	}

void CDBFormatXLS97::ResolveFixup (EFixup iType, DWORD dwData, int iResolvedOffset)

//	ResolveFixup
//
//	Resolves a fixup.

	{
	for (int i = 0; i < m_Fixups.GetCount(); i++)
		{
		if (m_Fixups[i].iType == iType && m_Fixups[i].dwData == dwData)
			{
			//	Save the current position

			int iSavedPos = m_Stream.GetPos();

			//	Seek to where we need to do a fix up.

			m_Stream.Seek(m_Fixups[i].iOffset);

			//	Write a DWORD for the resolved offset

			m_Stream.Write(&iResolvedOffset, sizeof(DWORD));

			//	Return to original position

			m_Stream.Seek(iSavedPos);

			//	Remove this fix up.

			m_Fixups.Delete(i);
			i--;
			}
		}
	}

TArray<int> CDBFormatXLS97::SortRows (const TArray<int> &Rows, const TArray<int> &SortOrder)

//	SortRows
//
//	Sorts the rows.

	{
	TSortMap<CString, int> Sorted;
	if (Rows.GetCount() == 0)
		{
		Sorted.GrowToFit(m_Table.GetRowCount());
		for (int i = 0; i < m_Table.GetRowCount(); i++)
			{
			Sorted.Insert(MakeSortKey(SortOrder, i), i);
			}
		}
	else
		{
		Sorted.GrowToFit(Rows.GetCount());
		for (int i = 0; i < Rows.GetCount(); i++)
			{
			Sorted.Insert(MakeSortKey(SortOrder, Rows[i]), Rows[i]);
			}
		}

	TArray<int> Result;
	Result.InsertEmpty(Sorted.GetCount());
	for (int i = 0; i < Sorted.GetCount(); i++)
		Result[i] = Sorted[i];

	return Result;
	}

bool CDBFormatXLS97::Write ()

//	Write
//
//	Write a workbook to the stream.

	{
	//	First we need to collect all data in a format suitable for exporting to 
	//	Excel. This allows us to create shared string tables, etc.

	InitData();

	//	Write workbook globals.

	WriteBOF(SUBSTREAM_TYPE_GLOBALS);

	WriteWorkbookDefinitions();

	//	Writes BOUNDSHEET records for every sheet.

	for (int i = 0; i < m_Data.GetCount(); i++)
		{
		WriteBOUNDSHEET(m_Data[i].GetName(), i);
		}

	//	Write shared strings

	WriteSST();

	//	Done with workbook globals.

	WriteEOF();

	//	Now write out each sheet

	for (int i = 0; i < m_Data.GetCount(); i++)
		{
		WriteSheet(m_Data[i], i);
		}

	//	HACK: If the stream is less than 4096 bytes, then increase size.

	if (m_Stream.GetStreamLength() < 4096)
		{
#if 1
		m_Stream.Write(NULL, 4096 - m_Stream.GetStreamLength());
#else
		WriteFiller(4096 - m_Stream.GetStreamLength());
#endif
		}

	//	Done

	return true;
	}

bool CDBFormatXLS97::WriteCell (const CDBValue &Value, int iRow, int iCol)

//	WriteCell
//
//	Writes a single cell.

	{
	switch (Value.GetType())
		{
		case CDBValue::typeInt32:
		case CDBValue::typeInt64:
		case CDBValue::typeDouble:
			WriteLABELSST(m_SST.GetStringIndex(Value.AsString()), 0, iRow, iCol);
			break;

		default:
			WriteLABELSST(m_SST.GetStringIndex(Value.AsString()), 0, iRow, iCol);
			break;
		}

	return true;
	}

bool CDBFormatXLS97::WriteSheet (const CDBFormatXLS97Sheet &Sheet, int iSheetIndex)

//	WriteSheet
//
//	Writes a sheet.

	{
	//	Before writing we resolve any required fixups.

	ResolveFixup(EFixup::Worksheet, iSheetIndex, m_Stream.GetPos());

	//	Start

	WriteBOF();

	//	Write the INDEX record. This will generate some fixup entries because we
	//	don't yet know the offsets for the rows.

	WriteINDEX(Sheet.GetRowCount(), Sheet.GetBlockCount());

	//	Output the dimensions record

	WriteDIMENSIONS(Sheet.GetRowCount(), m_Options.ColOrder.GetCount());

	//	Now write each block.

	int iRow = 0;
	for (int i = 0; i < Sheet.GetBlockCount(); i++)
		{
		int iFirstRowInBlock = iRow;

		//	Remember the position of the first ROW record (because we'll need it
		//	for the DBCELL record).

		int iFirstROWOffset = m_Stream.GetPos();

		//	Write all the ROW records.

		int iRowsInBlock = Sheet.GetRowCount(i);
		for (int j = 0; j < iRowsInBlock; j++)
			{
			WriteROW(iFirstRowInBlock + j, m_Options.ColOrder.GetCount());
			}

		//	Write all the cell values for each row.

		TArray<int> FirstCellOffset;
		FirstCellOffset.InsertEmpty(iRowsInBlock);

		int iOffsetStart = iFirstROWOffset + sizeof(R_ROW);

		for (int j = 0; j < iRowsInBlock; j++)
			{
			//	Remember the offset of the first cell in the row.

			FirstCellOffset[j] = m_Stream.GetPos() - iOffsetStart;
			iOffsetStart = m_Stream.GetPos();

			//	Write all the cells for the row.

			for (int k = 0; k < m_Options.ColOrder.GetCount(); k++)
				{
				const CDBValue &Value = Sheet.GetCell(i, j, k);
				WriteCell(Value, iFirstRowInBlock + j, k);
				}
			}

		//	Now we can fix up the position of this block's DBCELL.

		ResolveFixup(EFixup::DBCell, i, m_Stream.GetPos());

		//	Now we can write the DBCELL.

		int iOffsetFromDBCELLToFirstROW = m_Stream.GetPos() - iFirstROWOffset;
		WriteDBCELL(iOffsetFromDBCELLToFirstROW, FirstCellOffset);

		//	Next

		iRow += iRowsInBlock;
		}

	//	Done

	WriteEOF();

	return true;
	}

void CDBFormatXLS97::WriteSST ()

//	WriteSST
//
//	Writes the shared string table.

	{
	//	State

	bool bNeedRecordHeader = true;
	int iTotalRecordSize = 0;
	int iFixupLengthOffset = 0;

	//	Loop over all strings.

	for (int i = 0; i < m_SST.GetCount(); i++)
		{
		//	Encode the string.

		CBuffer String = Encode(m_SST.GetString(i));

		//	If the length of this string exceeds the maximum record size, then
		//	we need to create a new record.

		if (!bNeedRecordHeader && (iTotalRecordSize + sizeof(R_SST) + String.GetLength() > MAX_RECORD_SIZE))
			{
			//	Fixup the length

			int iOldPos = m_Stream.GetPos();
			m_Stream.Seek(iFixupLengthOffset);
			WORD wLen = (WORD)iTotalRecordSize;
			m_Stream.Write(&wLen, sizeof(wLen));
			m_Stream.Seek(iOldPos);

			//	Need a new header

			bNeedRecordHeader = true;
			}

		//	Write the header, if necessary.

		if (bNeedRecordHeader)
			{
			//	First header

			if (i == 0)
				{
				//	Remember to fixup the length

				iFixupLengthOffset = m_Stream.GetPos() + offsetof(R_SST, wLen);

				//	Write the record

				R_SST Record;
				Record.dwTotalStrings = m_SST.GetInstanceCount();
				Record.dwUniqueStrings = m_SST.GetCount();

				m_Stream.Write(&Record, sizeof(Record));
				iTotalRecordSize = sizeof(R_SST) - sizeof(DWORD);
				}

			//	Continue header

			else
				{
				iFixupLengthOffset = m_Stream.GetPos() + offsetof(R_CONTINUE, wLen);

				R_CONTINUE Record;
				m_Stream.Write(&Record, sizeof(Record));
				iTotalRecordSize = 0;
				}

			bNeedRecordHeader = false;
			}

		//	Remember the position of this string.

		m_SST.SetOffset(i, m_Stream.GetPos());

		//	Write the string

		m_Stream.Write(String);

		//	Keep track of total length.

		iTotalRecordSize += String.GetLength();
		}

	//	Fixup length

	int iOldPos = m_Stream.GetPos();
	m_Stream.Seek(iFixupLengthOffset);
	WORD wLen = (WORD)iTotalRecordSize;
	m_Stream.Write(&wLen, sizeof(wLen));
	m_Stream.Seek(iOldPos);
	}

void CDBFormatXLS97::WriteWorkbookDefinitions ()

//	WriteWorkbookDefinitions
//
//	Writes standard workbook definitions.

	{
	WriteINTERFACEHDR();
	WriteWINDOW1();
	}

void CDBFormatXLS97::WriteBOUNDSHEET (const CString &sSheetName, int iSheetIndex)
	{
	//	Remember that we need to fix up the offset.

	RequestFixup(EFixup::Worksheet, m_Stream.GetPos() + offsetof(R_BOUNDSHEET, dwOffset), iSheetIndex);

	//	Write the record

	R_BOUNDSHEET Record;
	CBuffer Name = Encode(sSheetName, 1);

	Record.wLen += Name.GetLength();

	m_Stream.Write(&Record, sizeof(Record));
	m_Stream.Write(Name);
	}

void CDBFormatXLS97::WriteBOF (WORD wSubstream)
	{
	R_BOF Record;
	Record.wSubstream = wSubstream;

	m_Stream.Write(&Record, sizeof(Record));
	}

void CDBFormatXLS97::WriteDBCELL (int iRowOffset, const TArray<int> &CellOffsets)
	{
	R_DBCELL Record;
	Record.wLen += (WORD)(CellOffsets.GetCount() * sizeof(WORD));
	Record.dwRowOffset = iRowOffset;

	m_Stream.Write(&Record, sizeof(Record));
	for (int i = 0; i < CellOffsets.GetCount(); i++)
		{
		WORD wValue = (WORD)CellOffsets[i];
		m_Stream.Write(&wValue, sizeof(WORD));
		}
	}

void CDBFormatXLS97::WriteDIMENSIONS (int iRowCount, int iColCount)
	{
	R_DIMENSIONS Record;
	Record.dwFirstRow = 0;
	Record.dwLastRowPlus1 = iRowCount;
	Record.wFirstCol = 0;
	Record.wLastColPlus1 = (WORD)iColCount;

	m_Stream.Write(&Record, sizeof(Record));
	}

void CDBFormatXLS97::WriteEOF ()
	{
	R_EOF Record;
	m_Stream.Write(&Record, sizeof(Record));
	}

void CDBFormatXLS97::WriteINDEX (int iRowCount, int iBlockCount)
	{
	R_INDEX Record;
	Record.wLen += (WORD)(iBlockCount * sizeof(DWORD));
	Record.dwFirstRow = 0;
	Record.dwLastRowPlus1 = iRowCount;

	m_Stream.Write(&Record, sizeof(Record));

	for (int i = 0; i < iBlockCount; i++)
		{
		RequestFixup(EFixup::DBCell, m_Stream.GetPos() + i * sizeof(DWORD), i);
		}

	m_Stream.Write(NULL, iBlockCount * sizeof(DWORD));
	}

void CDBFormatXLS97::WriteINTERFACEHDR ()
	{
	R_INTERFACEHDR HeaderRecord;
	m_Stream.Write(&HeaderRecord, sizeof(HeaderRecord));

	R_MMS MMSRecord;
	m_Stream.Write(&MMSRecord, sizeof(MMSRecord));

	R_INTERFACEEND EndRecord;
	m_Stream.Write(&EndRecord, sizeof(EndRecord));
	}

void CDBFormatXLS97::WriteLABEL (const CString &sValue, int iRow, int iCol)
	{
	R_LABEL Record;
	CBuffer Value = Encode(sValue);

	Record.wLen += Value.GetLength();
	Record.wRow = (WORD)iRow;
	Record.wCol = (WORD)iCol;

	m_Stream.Write(&Record, sizeof(Record));
	m_Stream.Write(Value);
	}

void CDBFormatXLS97::WriteLABELSST (int iIndex, int iFormat, int iRow, int iCol)
	{
	R_LABELSST Record;
	Record.wRow = (WORD)iRow;
	Record.wCol = (WORD)iCol;
	Record.wXF = (WORD)iFormat;
	Record.dwIndex = iIndex;

	m_Stream.Write(&Record, sizeof(Record));
	}

void CDBFormatXLS97::WriteROW (int iRowNumber, int iColCount)
	{
	R_ROW Record;
	Record.wRow = iRowNumber;
	Record.wFirstCol = 0;
	Record.wLastColPlus1 = iColCount;

	m_Stream.Write(&Record, sizeof(Record));
	}

void CDBFormatXLS97::WriteWINDOW1 ()
	{
	R_WINDOW1 Record;
	m_Stream.Write(&Record, sizeof(Record));
	}

void CDBFormatXLS97::WriteFiller (int iBytes)
	{
	R_FRT Record;
	Record.wLen += iBytes;

	m_Stream.Write(&Record, sizeof(Record));
	m_Stream.Write(NULL, iBytes);
	}
