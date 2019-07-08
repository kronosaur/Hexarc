//	ConsoleMode.cpp
//
//	CAeonEngine class
//	Copyright (c) 2018 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(CMD_CREATE_TABLE,					"createtable")
DECLARE_CONST_STRING(CMD_HELP,							"help")
DECLARE_CONST_STRING(CMD_GET_ROWS,						"getrows")
DECLARE_CONST_STRING(CMD_IMPORT_TABLE,					"importtable")
DECLARE_CONST_STRING(CMD_LIST_TABLES,					"listtables")

DECLARE_CONST_STRING(FIELD_KEY_TYPE,					"keyType")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_X,							"x")

DECLARE_CONST_STRING(HELP_CREATE_TABLE,					"createTable {tableDesc}")
DECLARE_CONST_STRING(HELP_GET_ROWS,						"getRows {tableName} {key} {count}")
DECLARE_CONST_STRING(HELP_IMPORT_TABLE,					"importTable {tableName} {CSV filespec}")

DECLARE_CONST_STRING(PATH_AEON_FOLDER,					"AeonDB")

DECLARE_CONST_STRING(STR_HELP,							"createTable {tableDesc}\n"
														"getRows {tableName} {key} {count}\n"
														"importTable {tableName} {CSV filespec}\n"
														"listTables\n"
														"quit\n"
														)

DECLARE_CONST_STRING(STR_TABLE_CREATED,					"Table %s created.")

DECLARE_CONST_STRING(TYPE_INT32,						"int32")
DECLARE_CONST_STRING(TYPE_INT64,						"int64")

DECLARE_CONST_STRING(ERR_NO_LOCAL_STORAGE,				"Invalid local storage path: %s.")
DECLARE_CONST_STRING(ERR_CANT_OPEN,						"Unable to open tables.")
DECLARE_CONST_STRING(ERR_NO_TABLES_FOUND,				"There are no tables in AeonDB.")
DECLARE_CONST_STRING(ERR_UNKNOWN_TABLE,					"Unknown table: %s.")

CString CAeonEngine::ConsoleCommand (const CString &sCmd, const TArray<CDatum> &Args)

//	ConsoleCommand
//
//	Execute the given command

	{
	CSmartLock Lock(m_cs);
	CString sError;
	int i;

	if (strEquals(sCmd, CMD_CREATE_TABLE))
		{
		//	We expect the first arg to be the table definition.

		if (Args.GetCount() < 1)
			return HELP_CREATE_TABLE;

		CDatum dTableDesc = Args[0];

		//	Create

		CAeonTable *pTable;
		if (!CreateTable(dTableDesc, &pTable, NULL, &sError))
			return sError;

		//	Success

		return strPattern(STR_TABLE_CREATED, pTable->GetName());
		}
	else if (strEquals(sCmd, CMD_GET_ROWS))
		{
		if (Args.GetCount() < 3)
			return HELP_GET_ROWS;

		CString sTable = Args[0];

		CAeonTable *pTable;
		if (!FindTable(sTable, &pTable))
			return strPattern(ERR_UNKNOWN_TABLE, sTable);

		//	Get the row limits

		int iRowCount;
		TArray<int> Limits;
		CDatum dLimits = Args[2];
		if (dLimits.IsNil())
			iRowCount = -1;
		else if (dLimits.GetCount() <= 1)
			{
			iRowCount = (int)dLimits.GetElement(0);
			if (iRowCount <= 0)
				iRowCount = -1;
			}
		else
			{
			iRowCount = (int)dLimits.GetElement(0);
			if (iRowCount <= 0)
				iRowCount = -1;

			Limits.InsertEmpty(dLimits.GetCount() - 1);
			for (i = 1; i < dLimits.GetCount(); i++)
				Limits[i - 1] = (int)dLimits.GetElement(i);
			}

		//	Set up flags and options

		DWORD dwFlags = CAeonTable::FLAG_NO_KEY;

		//	Ask the table

		CDatum dResult;
		CString sError;
		if (!pTable->GetRows(0, Args[1], iRowCount, Limits, dwFlags, &dResult, &sError))
			return sError;

		return dResult.AsString();
		}
	else if (strEquals(sCmd, CMD_HELP))
		{
		return STR_HELP;
		}
	else if (strEquals(sCmd, CMD_IMPORT_TABLE))
		{
		//	First arg is table name; second is filespec.

		if (Args.GetCount() < 2)
			return HELP_IMPORT_TABLE;

		CString sTableName = Args[0];
		CString sFilespec = Args[1];

		if (sTableName.IsEmpty() || sFilespec.IsEmpty())
			return HELP_IMPORT_TABLE;

		//	Look for the file to import

		CFileBuffer64 Input;
		if (!Input.OpenReadOnly(sFilespec, &sError))
			return sError;

		//	Load headers

		CCSVParser Parser(Input);
		if (!Parser.ParseHeader(&sError))
			return sError;

		//	Compose a new table descriptor and create the table.

		CDatum dTableDesc(CDatum::typeStruct);
		dTableDesc.SetElement(FIELD_NAME, sTableName);
		
		CDatum dKeyX(CDatum::typeStruct);
		dKeyX.SetElement(FIELD_KEY_TYPE, TYPE_INT32);
		dTableDesc.SetElement(FIELD_X, dKeyX);

		CAeonTable *pTable;
		if (!CreateTable(dTableDesc, &pTable, NULL, &sError))
			return sError;

		//	Now loop over all rows in the input

		TArray<CString> Values;
		DWORD dwRowID = 1;
		while (Parser.HasMore())
			{
			//	Data

			Parser.ParseRow(Values);

			//	Key

			CDatum dKey(dwRowID++);
			CRowKey Path;
			if (!pTable->ParseDimensionPathForCreate(dKey, &Path, &sError))
				return sError;

			//	Create the value

			CDatum dValue(CDatum::typeStruct);
			for (i = 0; i < Min(Parser.GetHeader().GetCount(), Values.GetCount()); i++)
				{
				dValue.SetElement(Parser.GetHeader().GetAt(i), CDatum(Values[i]));
				}

			//	Add the row

			if (!pTable->Insert(Path, dValue, &sError))
				return sError;
			}

		return strPattern("Created table %s with %d rows.", sTableName, dwRowID - 1);
		}
	else if (strEquals(sCmd, CMD_LIST_TABLES))
		{
		//	Get a list of all tables

		TArray<CAeonTable *> AllTables;
		GetTables(&AllTables);

		if (AllTables.GetCount() == 0)
			return ERR_NO_TABLES_FOUND;

		//	Add to result

		CStringBuffer Output;
		for (i = 0; i < AllTables.GetCount(); i++)
			{
			Output.Write(strPattern("%s\n", AllTables[i]->GetName()));
			}

		return CString::CreateFromHandoff(Output);
		}
	else
		return NULL_STR;
	}

bool CAeonEngine::InitConsoleMode (const CString &sStoragePath, CString *retsError)

//	InitConsoleMode
//
//	Initialize console mode. This is the counterpart to MsgOnMachineStart
//	for console mode.

	{
	CSmartLock Lock(m_cs);

	//	If we've already started, then nothing to do.

	if (m_bMachineStarted)
		return true;

	//	Make a list of local volumes

	if (!m_LocalVolumes.InitLocal(sStoragePath, PATH_AEON_FOLDER, retsError))
		return false;

	//	If we have no local volumes then we cannot proceed. We wait until we
	//	have storage to send Aeon.onStart.

	if (m_LocalVolumes.GetCount() == 0)
		{
		*retsError = strPattern(ERR_NO_LOCAL_STORAGE, sStoragePath);
		return false;
		}

	//	Make sure the database is open

	if (!Open())
		{
		*retsError = ERR_CANT_OPEN;
		return false; 
		}

	//	Done

	printf("AeonDB ready. %d table%s available.\n", m_Tables.GetCount(), (m_Tables.GetCount() == 1 ? "" : "s"));
	m_bMachineStarted = true;

	return true;
	}
