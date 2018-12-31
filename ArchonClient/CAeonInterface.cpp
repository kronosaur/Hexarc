//	CAeonInterface.cpp
//
//	CAeonInterface class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_AEON,							"Aeon.command");
DECLARE_CONST_STRING(ADDRESS_HYPERION_COMMAND,			"Hyperion.command");

DECLARE_CONST_STRING(FIELD_IF_MODIFIED_AFTER,			"ifModifiedAfter");
DECLARE_CONST_STRING(FIELD_PARTIAL_MAX_SIZE,			"partialMaxSize");
DECLARE_CONST_STRING(FIELD_PARTIAL_POS,					"partialPos");

DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD,			"Aeon.fileDownload");
DECLARE_CONST_STRING(MSG_HYPERION_SERVICE_MSG,			"Hyperion.serviceMsg");
DECLARE_CONST_STRING(MSG_TRANSPACE_DOWNLOAD,			"Transpace.download");

DECLARE_CONST_STRING(SEPARATOR_SLASH,					"/");

DECLARE_CONST_STRING(STR_ERROR_ABSOLUTE_PATH_REQUIRED,	"Absolute filePath expected.");
DECLARE_CONST_STRING(STR_ERROR_NO_TABLE_IN_PATH,		"Table name expected.");

CString CAeonInterface::CreateTableFilePath (const CString &sTable, const CString &sFilePath)

//	CreateTableFilePath
//
//	Composes a full filepath.

	{
	return strPattern("/%s/%s", sTable, sFilePath);
	}

CString CAeonInterface::EncodeFilePathComponent (const CString &sValue)

//	EncodeFilePathComponent
//
//	Encodes a file path component by replacing any invalid characters with ~hh
//	(where hh is the hexcode).

	{
	char *pPos = sValue.GetParsePointer();
	char *pEndPos = pPos + sValue.GetLength();
	CStringBuffer Output;

	while (pPos < pEndPos)
		{
		if (!strIsASCIIAlpha(pPos)					//	alpha
				&& !strIsDigit(pPos)				//	numbers
				&& *pPos != '-'
				&& *pPos != '_'
				&& *pPos != '.'
				&& *pPos != '~'
				&& !strIsASCIIHigh(pPos))			//	unicode characters
			{
			CString sChar = strPattern("~%x", (DWORD)(BYTE)*pPos);
			Output.Write(sChar);
			}
		else
			Output.Write(pPos, 1);

		pPos++;
		}

	//	Done

	return CString::CreateFromHandoff(Output);
	}

CString CAeonInterface::FilespecToFilePath (const CString &sFilespec)

//	FilespecToFilePath
//
//	Converts a filespec to an Aeon filePath. We convert \ to / and escape all
//	characters that are not valid Aeon path characters.

	{
	char *pPos = sFilespec.GetParsePointer();
	char *pEndPos = pPos + sFilespec.GetLength();
	CStringBuffer Output;

	while (pPos < pEndPos)
		{
		if (*pPos == '\\')
			Output.Write("/", 1);
		else if (!strIsASCIIAlpha(pPos)				//	alpha
				&& !strIsDigit(pPos)				//	numbers
				&& *pPos != '-'
				&& *pPos != '_'
				&& *pPos != '.'
				&& *pPos != '~'
				&& !strIsASCIIHigh(pPos))			//	unicode characters
			{
			CString sChar = strPattern("_%x_", (DWORD)(BYTE)*pPos);
			Output.Write(sChar);
			}
		else
			Output.Write(pPos, 1);

		pPos++;
		}

	//	Done

	return CString::CreateFromHandoff(Output);
	}

bool CAeonInterface::ParseFilePath (const CString &sFilePath, const CString &sRoot, int iOffset, const CDateTime &IfModifiedAfter, CString *retsAddr, CString *retsMsg, CDatum *retdPayload)

//	ParseFilePath
//
//	Parses a filePath of the form:
//
//	@Aeon.command/Arc.services/TransPackage.ars
//	/Arc.services/TransPackage.ars
//	./TransPackage.ars
//
//	Returns FALSE if error.

	{
	char *pPos = sFilePath.GetParsePointer();
	char *pPosEnd = pPos + sFilePath.GetLength();

	//	Create a fileDownloadDesc to specify that we not read more than 100K
	//	at a time (so that we don't overload our IPC buffer).

	CComplexStruct *pFDDesc = new CComplexStruct;
	pFDDesc->SetElement(FIELD_PARTIAL_MAX_SIZE, 100000);
	pFDDesc->SetElement(FIELD_PARTIAL_POS, iOffset);
	if (IfModifiedAfter.IsValid())
		pFDDesc->SetElement(FIELD_IF_MODIFIED_AFTER, IfModifiedAfter);

	CDatum dFileDownloadDesc(pFDDesc);

	//	If the path starts with @ then this is an absolute path

	CString sParsedPath;
	if (*pPos == '@')
		{
		//	LATER

		return false;
		}

	//	Is this a service namespace?

	else if (*pPos == '#')
		{
		CString sNamespace;
		if (!CTranspaceInterface::ParseAddress(sFilePath, &sNamespace))
			return false;

		//	Compose a proper download command payload

		CComplexArray *pPayload = new CComplexArray;
		pPayload->Append(sFilePath);
		pPayload->Append(sFilePath);
		pPayload->Append(dFileDownloadDesc);
		CDatum dPayload(pPayload);

		//	Parse the service endpoint

		CString sService(sNamespace.GetParsePointer() + 1);

		//	Encode into the proper payload

		CComplexArray *pPayload2 = new CComplexArray;
		pPayload2->Append(sService);
		pPayload2->Append(MSG_TRANSPACE_DOWNLOAD);
		pPayload2->Append(dPayload);

		//	Done

		*retsAddr = ADDRESS_HYPERION_COMMAND;
		*retsMsg = MSG_HYPERION_SERVICE_MSG;
		*retdPayload = CDatum(pPayload2);
		return true;
		}

	//	If it starts with a slash then it is an Aeon path

	else if (*pPos == '/')
		{
		sParsedPath = sFilePath;
		*retsAddr = ADDR_AEON;
		*retsMsg = MSG_AEON_FILE_DOWNLOAD;

		//	Generate a message for Aeon to load the file

		CComplexArray *pPayload = new CComplexArray;
		pPayload->Insert(sParsedPath);
		pPayload->Insert(dFileDownloadDesc);

		//	Done

		*retdPayload = CDatum(pPayload);
		}

	//	If it starts with a ./ then this is a relative path

	else if (*pPos == '.')
		{
		pPos++;
		if (pPos == pPosEnd || *pPos != '/')
			return false;

		//	Root must be valid

		if (sRoot.IsEmpty())
			return false;

		//	If the root already ends in '/' then skip.

		if (*(sRoot.GetParsePointer() + sRoot.GetLength() - 1) == '/')
			pPos++;

		sParsedPath = sRoot + CString(pPos);
		*retsAddr = ADDR_AEON;
		*retsMsg = MSG_AEON_FILE_DOWNLOAD;

		//	Generate a message for Aeon to load the file

		CComplexArray *pPayload = new CComplexArray;
		pPayload->Insert(sParsedPath);
		pPayload->Insert(dFileDownloadDesc);

		//	Done

		*retdPayload = CDatum(pPayload);
		}

	//	Otherwise this is a relative path.

	else
		{
		//	Root must be valid

		if (sRoot.IsEmpty())
			return false;

		//	Add '/' separator

		if (*(sRoot.GetParsePointer() + sRoot.GetLength() - 1) == '/')
			sParsedPath = sRoot + sFilePath;
		else
			sParsedPath = sRoot + SEPARATOR_SLASH, sFilePath;

		*retsAddr = ADDR_AEON;
		*retsMsg = MSG_AEON_FILE_DOWNLOAD;

		//	Generate a message for Aeon to load the file

		CComplexArray *pPayload = new CComplexArray;
		pPayload->Insert(sParsedPath);
		pPayload->Insert(dFileDownloadDesc);

		//	Done

		*retdPayload = CDatum(pPayload);
		}


	return true;
	}

bool CAeonInterface::ParseTableFilePath (const CString &sPath, CString *retsTable, CString *retsFilePath, CString *retsError)

//	ParseTableFilePath
//
//	Parses a filePath

	{
	char *pPos = sPath.GetParsePointer();
	char *pEndPos = pPos + sPath.GetLength();

	//	First character must be a slash because we need an absolute path

	if (*pPos != '/')
		{
		if (retsError)
			*retsError = STR_ERROR_ABSOLUTE_PATH_REQUIRED;
		return false;
		}

	pPos++;

	//	The first segment is the table name

	char *pStart = pPos;
	while (pPos < pEndPos && *pPos != '/')
		pPos++;

	CString sTable(pStart, pPos - pStart);
	if (sTable.IsEmpty())
		{
		if (retsError)
			*retsError = STR_ERROR_NO_TABLE_IN_PATH;
		return false;
		}

	//	We have the table name

	if (retsTable)
		*retsTable = sTable;

	//	If we've reached the end, then we have an empty path

	if (pPos >= pEndPos)
		{
		if (retsFilePath)
			*retsFilePath = CString("/", 1);
		return true;
		}

	//	filePath starts with a slash and goes to the end.

	if (retsFilePath)
		*retsFilePath = CString(pPos, (int)(pEndPos - pPos));

	//	Done

	return true;
	}
