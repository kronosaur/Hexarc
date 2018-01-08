//	HexeRun.cpp
//
//	Runs a HexeLisp script
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "Hexe.h"

DECLARE_CONST_STRING(FIELD_CODE,						"code")

DECLARE_CONST_STRING(LIBRARY_AI1,						"ai1")

DECLARE_CONST_STRING(TYPE_PROCEDURE,					"procedure")

int ExecuteHexeDocument (const SOptions &Options)
	{
	CString sError;

	//	Open up the file

	CFile File;
	if (!File.Create(Options.sHexeDocument, CFile::FLAG_OPEN_READ_ONLY, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	//	Create a process (and add some libraries)

	CHexeProcess Process;
	if (!Process.LoadStandardLibraries(&sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	RegisterAI1Library();
	if (!Process.LoadLibrary(LIBRARY_AI1, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	//	Parse the document

	CHexeDocument HexeDoc;
	if (!HexeDoc.InitFromStream(File, Process, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	//	Load defines

	if (!Process.LoadHexeDefinitions(HexeDoc, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	//	Find the procedure

	int iType;
	if (HexeDoc.GetTypeIndexCount(iType = HexeDoc.GetTypeIndex(TYPE_PROCEDURE)) == 0)
		{
		printf("ERROR: Expected procedure.\n");
		return 1;
		}

	CString sName = HexeDoc.GetTypeIndexName(iType, 0);
	CDatum dProcedure = HexeDoc.GetTypeIndexData(iType, 0);
	CDatum dCode = dProcedure.GetElement(FIELD_CODE);

	//	Run

	TArray<CDatum> Args;
	CDatum dResult;
	CHexeProcess::ERunCodes iRun = Process.Run(dCode, Args, &dResult);

	//	Handle result

	switch (iRun)
		{
		case CHexeProcess::runOK:
			printf("%s\n", (LPSTR)dResult.AsString());
			break;

		case CHexeProcess::runError:
			printf("ERROR: %s\n", (LPSTR)dResult.AsString());
			break;

		default:
			printf("ERROR: Unable to run.\n");
			return 1;
		}

	//	Done

	return 0;
	}

