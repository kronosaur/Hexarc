//	CDBFormatCSV.cpp
//
//	CDBFormatCSV class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_BAD_HEADER,					"Unable to add %s column.")
DECLARE_CONST_STRING(ERR_BAD_FIELD_INDEX,				"Bad field index.")
DECLARE_CONST_STRING(ERR_NOT_ENOUGH_COLS_IN_ROW,		"Not enough columns in row.")

bool CDBFormatCSV::Load (IByteStream &Stream, const SOptions &Options, CDBTable &Table, CString *retsError)

//	Load
//
//	Load a CSV file into a table.

	{
	int i;
	CCSVParser Parser(Stream);

	if (Options.bUseUTF8)
		Parser.SetUTF8Format();

	//	Parse the header

	if (!Parser.ParseHeader(retsError))
		return false;

	//	Create columns

	Table.CleanUp();
	const TArray<CString> &Header = Parser.GetHeader();
	for (i = 0; i < Header.GetCount(); i++)
		{
		CDBColumnDef ColDef(Header[i], CDBValue::typeString, i);
		if (!Table.AddCol(ColDef))
			{
			if (retsError) *retsError = strPattern(ERR_BAD_HEADER, Header[i]);
			return false;
			}
		}

	//	Now add all the rows.

	const int ROW_GRANULARITY = 100;
	int iRow = 0;
	while (Parser.HasMore())
		{
		//	Grow the array appropriately by 100s

		if ((iRow % ROW_GRANULARITY) == 0)
			Table.GrowToFit(ROW_GRANULARITY);

		//	Parse the row

		TArray<CString> Row;
		if (!Parser.ParseRow(Row, retsError))
			return false;

		if (!Options.bAllowShortRows && Row.GetCount() < Table.GetColCount())
			{
			if (retsError) *retsError = ERR_NOT_ENOUGH_COLS_IN_ROW;
			return false;
			}

		//	Add each field.

		Table.AddRow();
		for (i = 0; i < Table.GetColCount(); i++)
			{
			if (!Table.SetField(i, iRow, CDBValue::FromHandoff(Row[i])))
				{
				ASSERT(false);	//	Should never happen
				if (retsError) *retsError = ERR_BAD_FIELD_INDEX;
				return false;
				}
			}

		//	Next row

		iRow++;
		}

	return true;
	}
