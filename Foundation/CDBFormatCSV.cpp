//	CDBFormatCSV.cpp
//
//	CDBFormatCSV class
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_BAD_HEADER,					"Unable to add %s column.");
DECLARE_CONST_STRING(ERR_BAD_FIELD_INDEX,				"Bad field index.");
DECLARE_CONST_STRING(ERR_NOT_ENOUGH_COLS_IN_ROW,		"Not enough columns in row.");
DECLARE_CONST_STRING(ERR_INVALID_UTF8,					"CSV file is not valid UTF-8.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_CHARSET,			"Unsupported CSV character set.");

constexpr int CDBFormatCSV::EstimateRowCount (DWORDLONG dwStreamSize)

//	EstimateRowCount
//
//	Estimate the number of rows based on stream size.

	{
	return (int)(dwStreamSize / 200);
	}

bool CDBFormatCSV::Load (IByteStream64 &Stream, const SOptions &Options, CDBTable &Table, CString *retsError)

//	Load
//
//	Load a CSV file into a table.

	{
	constexpr int PROGRESS_GRANULARITY = 1000;
	const DWORDLONG dwStreamSize = Stream.GetStreamLength();
	ECharSetType iCharSet = Options.iCharSet;
	if (iCharSet == ECharSetType::Unknown)
		iCharSet = CCSVParser::DetectCharSet(Stream);
	else if (iCharSet == ECharSetType::UTF8
			&& CCSVParser::DetectCharSet(Stream) != ECharSetType::UTF8)
		{
		if (retsError) *retsError = ERR_INVALID_UTF8;
		return false;
		}
	else if (iCharSet == ECharSetType::UTF16BE
			|| iCharSet == ECharSetType::UTF16LE
			|| iCharSet == ECharSetType::UTF32BE
			|| iCharSet == ECharSetType::UTF32LE)
		{
		if (retsError) *retsError = ERR_UNSUPPORTED_CHARSET;
		return false;
		}

	CCSVParser Parser(Stream);
	Parser.SetCharSet(iCharSet);
	Parser.SetDelimiter(Options.chDelimiter);

	//	Parse the header

	if (!Parser.ParseHeader(retsError))
		return false;

	//	Create columns

	Table.CleanUp();
	const TArray<CString> &Header = Parser.GetHeader();
	for (int i = 0; i < Header.GetCount(); i++)
		{
		CDBColumnDef ColDef(Header[i], CDBValue::typeString, i);
		if (!Table.AddCol(ColDef))
			{
			if (retsError) *retsError = strPattern(ERR_BAD_HEADER, Header[i]);
			return false;
			}
		}

	//	Now add all the rows.

	const int ROW_GRANULARITY = Max(100, EstimateRowCount(dwStreamSize) / 10);
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
		for (int i = 0; i < Min(Row.GetCount(), Table.GetColCount()); i++)
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

		//	Progress
		
		if (Options.fnOnProgress && (iRow % PROGRESS_GRANULARITY) == 0)
			{
			DWORDLONG dwPos = Stream.GetPos();
			int iPercent = (int)(dwPos * 100 / dwStreamSize);

			Options.fnOnProgress(iPercent, NULL_STR);
			}
		}

	return true;
	}
