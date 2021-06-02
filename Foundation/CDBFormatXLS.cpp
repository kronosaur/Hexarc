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

DECLARE_CONST_STRING(STR_BLANK,								"(Blank)");

DECLARE_CONST_STRING(TYPE_DATE_TIME,						"DateTime");
DECLARE_CONST_STRING(TYPE_NUMBER,							"Number");
DECLARE_CONST_STRING(TYPE_STRING,							"String");

CString CDBFormatXLS::GetDataValue (const CDBValue &Value)

//	GetDataValue
//
//	Returns the value, ready for outputting to XML.

	{
	return CXMLElement::MakeAttribute(Value.AsString());
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
			if (!WriteSheet(Stream, Sheets.GetKey(i), Table, Sheets[i]))
				return false;
			}
		}

	//	Otherwise, we write a single sheet with all rows and columns

	else
		{
		if (!WriteSheet(Stream, Table.GetName(), Table, TArray<int>()))
			return false;
		}

	//	Done

	Stream.Write("</Workbook>\r\n");

	return true;
	}

bool CDBFormatXLS::WriteRow (IByteStream &Stream, const CDBTable &Table, int iRow)

//	WriteRow
//
//	Write the row out.

	{
	Stream.Write("<Row>\r\n");

	for (int i = 0; i < Table.GetColCount(); i++)
		{
		Stream.Write(strPattern("<Cell><Data ss:Type=\"%s\">%s</Data></Cell>\r\n",
				GetDataType(Table.GetCol(i).GetType()),
				GetDataValue(Table.GetField(i, iRow))
				));
		}

	Stream.Write("</Row>\r\n");
	return true;
	}

bool CDBFormatXLS::WriteSheet (IByteStream &Stream, const CString &sSheetName, const CDBTable &Table, const TArray<int> &Rows)

//	WriteSheet
//
//	Writes a sheet.

	{
	Stream.Write(strPattern("<Worksheet ss:Name=\"%s\">\r\n", CXMLElement::MakeAttribute(sSheetName)));
	Stream.Write("<Table>\r\n");

	//	Write the header row

	Stream.Write("<Row>\r\n");
	for (int i = 0; i < Table.GetColCount(); i++)
		{
		Stream.Write(strPattern("<Cell><Data ss:Type=\"%s\">%s</Data></Cell>\r\n",
				TYPE_STRING,
				CXMLElement::MakeAttribute(Table.GetCol(i).GetName())
				));
		}
	Stream.Write("</Row>\r\n");

	//	Write data rows

	if (Rows.GetCount())
		{
		for (int i = 0; i < Rows.GetCount(); i++)
			{
			if (!WriteRow(Stream, Table, Rows[i]))
				return false;
			}
		}
	else
		{
		for (int i = 0; i < Table.GetRowCount(); i++)
			{
			if (!WriteRow(Stream, Table, i))
				return false;
			}
		}

	Stream.Write("</Table>\r\n");
	Stream.Write("</Worksheet>\r\n");
	return true;
	}

