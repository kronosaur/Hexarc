//	CDBFormatXLS.cpp
//
//	CDBFormatXLS class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.
//
//	REFERENCES
//
//	https://docs.microsoft.com/en-us/previous-versions/office/developer/office-xp/aa140066(v=office.10)?redirectedfrom=MSDN
//	https://social.technet.microsoft.com/wiki/contents/articles/19601.powershell-generate-real-excel-xlsx-files-without-excel.aspx

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_COLUMN_ORDER,					"columnOrder");
DECLARE_CONST_STRING(FIELD_SHEET_BY,						"sheetBy");

DECLARE_CONST_STRING(STR_BLANK,								"(Blank)");
DECLARE_CONST_STRING(STR_COMMA,								",");

DECLARE_CONST_STRING(TYPE_DATE_TIME,						"DateTime");
DECLARE_CONST_STRING(TYPE_NUMBER,							"Number");
DECLARE_CONST_STRING(TYPE_STRING,							"String");

CString CDBFormatXLS::GetDataValue (const CDBValue &Value)

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

CString CDBFormatXLS::GetDataType (CDBValue::ETypes iType)

//	GetDataType
//
//	Returns the appropriate XLS data type.

	{
	switch (iType)
		{
		case CDBValue::typeDateTime:
			return TYPE_DATE_TIME;

		case CDBValue::typeInt32:
		case CDBValue::typeInt64:
		case CDBValue::typeDouble:
			return TYPE_NUMBER;

		case CDBValue::typeString:
			return TYPE_STRING;

		default:
			return TYPE_STRING;
		}
	}

bool CDBFormatXLS::ParseOptions (const CDBTable &Table, const CDBValue &Value, SOptions &retOptions, CString *retsError)

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

bool CDBFormatXLS::Write (IByteStream &Stream, const CDBTable &Table, const SOptions &Options)

//	Write
//
//	Write a workbook to the stream.

	{
	//	Write header

	Stream.Write("<?xml version=\"1.0\"?>"
		"<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\" "
		"xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
		"xmlns:x=\"urn:schemas-microsoft-com:office:excel\" "
		"xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\" "
		"xmlns:html=\"http://www.w3.org/TR/REC-html40\">\r\n");

	//	If we split up data across sheets, then we need to category each row.

	if (Options.iSheetColumn != -1)
		{
		TSortMap<CString, TArray<int>> Sheets;
		for (int i = 0; i < Table.GetRowCount(); i++)
			{
			CString sSheet = GetDataValue(Table.GetField(Options.iSheetColumn, i));
			if (sSheet.IsEmpty())
				sSheet = STR_BLANK;

			auto *pRows = Sheets.SetAt(sSheet);
			pRows->Insert(i);
			}

		for (int i = 0; i < Sheets.GetCount(); i++)
			{
			if (!WriteSheet(Stream, Sheets.GetKey(i), Table, Sheets[i], Options))
				return false;
			}
		}

	//	Otherwise, we write a single sheet with all rows and columns

	else
		{
		if (!WriteSheet(Stream, Table.GetName(), Table, TArray<int>(), Options))
			return false;
		}

	//	Done

	Stream.Write("</Workbook>\r\n");

	return true;
	}

bool CDBFormatXLS::WriteHeaderRow (IByteStream &Stream, const CString &sSheetName, const CDBTable &Table, const SOptions &Options)

//	WriteHeaderRow
//
//	Writes the header row.

	{
	//	If we're using a table for headers, then do it.

	if (!Options.HeaderRows.IsEmpty())
		{
		//	If we're splitting up by sheets, then we need to filter.

		if (Options.iSheetColumn != -1)
			{
			for (int i = 0; i < Options.HeaderRows.GetRowCount(); i++)
				{
				CString sSheet = GetDataValue(Options.HeaderRows.GetField(Options.iSheetColumn, i));
				if (sSheet.IsEmpty())
					sSheet = STR_BLANK;

				if (!strEqualsNoCase(sSheet, sSheetName))
					continue;

				if (!WriteRow(Stream, Options.HeaderRows, i, Options.HeaderColOrder, Options))
					return false;
				}
			}

		//	Otherwise, just emit all rows in the table. We assume that header 
		//	rows have the exact same table structure.

		else
			{
			for (int i = 0; i < Options.HeaderRows.GetRowCount(); i++)
				{
				if (!WriteRow(Stream, Options.HeaderRows, i, Options.HeaderColOrder, Options))
					return false;
				}
			}
		}

	//	Otherwise, just use column names.

	else
		{
		Stream.Write("<Row>\r\n");

		for (int i = 0; i < Options.ColOrder.GetCount(); i++)
			{
			Stream.Write(strPattern("<Cell><Data ss:Type=\"%s\">%s</Data></Cell>\r\n",
					TYPE_STRING,
					CXMLElement::MakeAttribute(Table.GetCol(Options.ColOrder[i]).GetName())
					));
			}

		Stream.Write("</Row>\r\n");
		}

	return true;
	}

bool CDBFormatXLS::WriteRow (IByteStream &Stream, const CDBTable &Table, int iRow, const TArray<int> &ColOrder, const SOptions &Options)

//	WriteRow
//
//	Write the row out.

	{
	Stream.Write("<Row>\r\n");

	for (int i = 0; i < ColOrder.GetCount(); i++)
		{
		const CDBValue &Value = Table.GetField(ColOrder[i], iRow);
		Stream.Write(strPattern("<Cell><Data ss:Type=\"%s\">%s</Data></Cell>\r\n",
				GetDataType(Value.GetType()),
				GetDataValue(Value)
				));
		}

	Stream.Write("</Row>\r\n");
	return true;
	}

bool CDBFormatXLS::WriteSheet (IByteStream &Stream, const CString &sSheetName, const CDBTable &Table, const TArray<int> &Rows, const SOptions &Options)

//	WriteSheet
//
//	Writes a sheet.

	{
	Stream.Write(strPattern("<Worksheet ss:Name=\"%s\">\r\n", CXMLElement::MakeAttribute(sSheetName)));
	Stream.Write("<Table>\r\n");

	//	Write the header row

	if (!WriteHeaderRow(Stream, sSheetName, Table, Options))
		return false;

	//	Write data rows

	if (Rows.GetCount())
		{
		for (int i = 0; i < Rows.GetCount(); i++)
			{
			if (!WriteRow(Stream, Table, Rows[i], Options.ColOrder, Options))
				return false;
			}
		}
	else
		{
		for (int i = 0; i < Table.GetRowCount(); i++)
			{
			if (!WriteRow(Stream, Table, i, Options.ColOrder, Options))
				return false;
			}
		}

	Stream.Write("</Table>\r\n");
	Stream.Write("</Worksheet>\r\n");
	return true;
	}

