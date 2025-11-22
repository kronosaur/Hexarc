// AI1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_REQUEST_FRAGMENT
#endif

DECLARE_CONST_STRING(STR_RESPONSE_ONLY,					"-")

DECLARE_CONST_STRING(STR_PROMPT,						": ")
DECLARE_CONST_STRING(STR_ARC_CONSOLE,					"Arc.console")
DECLARE_CONST_STRING(STR_HTTP_GET_PREFIX,				"httpGet ")
DECLARE_CONST_STRING(STR_LOCALHOST,						"localhost")
DECLARE_CONST_STRING(STR_QUIT,							"quit")
DECLARE_CONST_STRING(STR_UPLOAD_PREFIX,					"upload ")
DECLARE_CONST_STRING(STR_UPLOAD_CERTIFICATE_PREFIX,		"uploadCertificate ")
DECLARE_CONST_STRING(STR_UPLOAD_PACKAGE_PREFIX,			"uploadPackage ")

DECLARE_CONST_STRING(CMD_COMPLETE_UPGRADE,				"completeUpgrade")
DECLARE_CONST_STRING(CMD_REFRESH_PACKAGES,				"refreshPackages")
DECLARE_CONST_STRING(CMD_RESTART_MACHINE,				"restartMachine")
DECLARE_CONST_STRING(CMD_UPGRADE,						"upgrade")
DECLARE_CONST_STRING(CMD_UPLOAD,						"upload")
DECLARE_CONST_STRING(CMD_UPLOAD_UPGRADE,				"uploadUpgrade")

DECLARE_CONST_STRING(EXTENSION_PEM,						".pem")

DECLARE_CONST_STRING(FIELD_COMMANDS,					"commands")
DECLARE_CONST_STRING(FIELD_FILENAME,					"filename")
DECLARE_CONST_STRING(FIELD_SERVER,						"server")
DECLARE_CONST_STRING(FIELD_UPGRADE_DESC,				"upgradeDesc")

DECLARE_CONST_STRING(LIBRARY_CORE,						"core")

DECLARE_CONST_STRING(HEADER_CONNECTION,					"Connection")
DECLARE_CONST_STRING(HEADER_HOST,						"Host")
DECLARE_CONST_STRING(HEADER_USER_AGENT,					"User-Agent")

CString ExecuteHTTPGet (const CString &sInput);
CString ExecuteLispCommand (const CString &sInput);
int ExecuteScript (const SOptions &Options);
CString ExecuteUpgrade (CSocket &theSocket, const CString &sCmd);
CString ExecuteUpload (CSocket &theSocket, const CString &sCmd);
CString ExecuteUploadCertificate (CSocket &theSocket, const CString &sCmd);
CString ExecuteUploadPackage (CSocket &theSocket, const CString &sCmd);

CHexeProcess g_Process;

CString Execute (CSocket &theSocket, SOptions &Options, const CString &sInput)
	{
	CString sResult;

	DWORD dwStart;
	if (Options.bTiming)
		dwStart = sysGetTickCount();

	//	Do it

	if (sInput.IsEmpty())
		return NULL_STR;

	else if (Options.bTopLevel)
		sResult = ExecuteLispCommand(sInput);

	else if (strStartsWith(sInput, STR_HTTP_GET_PREFIX))
		sResult = ExecuteHTTPGet(sInput);

	else if (strEquals(sInput, CMD_UPGRADE))
		sResult = ExecuteUpgrade(theSocket, sInput);

	else if (strStartsWith(sInput, STR_UPLOAD_CERTIFICATE_PREFIX))
		sResult = ExecuteUploadCertificate(theSocket, sInput);

	else if (strStartsWith(sInput, STR_UPLOAD_PACKAGE_PREFIX))
		sResult = ExecuteUploadPackage(theSocket, sInput);

	else if (strStartsWith(sInput, STR_UPLOAD_PREFIX))
		sResult = ExecuteUpload(theSocket, sInput);

	else
		sResult = ExecuteArcologyCommand(theSocket, sInput);

	//	Post-processing

	if (Options.bTiming)
		{
		DWORD dwTime = sysGetTickCount() - dwStart;
		sResult = strPattern("[%d.%02d] %s", dwTime / 1000, (dwTime % 1000) / 10, sResult);
		}

	return sResult;
	}

CString ExecuteHTTPGet (const CString &sInput)
	{
	//	Parse the input

	char *pPos = sInput.GetParsePointer() + STR_HTTP_GET_PREFIX.GetLength();

	//	Parse the URL

	CString sHost;
	CString sPath;
	if (!urlParse(pPos, NULL, &sHost, &sPath))
		return CString("Invalid URL.");

	//	If no host, then local host

	if (sHost.IsEmpty())
		sHost = CString("localhost");

	//	Connect

	CSocket theSocket;
	if (!theSocket.Connect(sHost, 80))
		return strPattern("Unable to connect to: %s.", sHost);

	//	Compose a request

	CHTTPMessage Request;
	Request.InitRequest(CString("GET"), sPath);
	Request.AddHeader(HEADER_HOST, sHost);
	Request.AddHeader(HEADER_CONNECTION, CString("keep-alive"));
#ifdef DEBUG_REQUEST_FRAGMENT_X
	Request.AddHeader(HEADER_USER_AGENT, CString("AI1/1.0 (This is a test of the header parsing system in Hexarc. There is probably a bug in which splitting the header across packets will cause failure of the HTTP parsing engine.)"));
#else
	Request.AddHeader(HEADER_USER_AGENT, CString("AI1/1.0"));
#endif

	//	Send the request

	CBuffer Buffer(4096);
	Request.WriteToBuffer(Buffer);

#ifdef DEBUG_REQUEST_FRAGMENT
	int iTotalLen = Buffer.GetLength();
	int iSplit = 105;

	if (iSplit < iTotalLen)
		{
		printf("Split at %d bytes\n", iSplit);
		CString sPart(Buffer.GetPointer(), iSplit);
		printf("%s\n", (LPSTR)sPart);

		theSocket.Write(Buffer.GetPointer(), iSplit);
		::Sleep(10);
		theSocket.Write(Buffer.GetPointer() + iSplit, iTotalLen - iSplit);
		}
	else
		theSocket.Write(Buffer.GetPointer(), Buffer.GetLength());
#else
	theSocket.Write(Buffer.GetPointer(), Buffer.GetLength());
#endif

	//	Now read the response. We build up a buffer to hold it.

	CHTTPMessage Response;
	CBuffer ResponseBuff;

	//	Keep reading until we've got enough (or until the connection drops)

	while (!Response.IsMessageComplete())
		{
		CBuffer TempBuffer(8192);

		//	Read

		int iBytesRead = theSocket.Read(TempBuffer.GetPointer(), 8192);
		TempBuffer.SetLength(iBytesRead);

		//	If we're no making progress, then we're done

		if (iBytesRead == 0)
			return strPattern("Unable to read entire message.");

		//	Add to entire buffer

		ResponseBuff.Write(TempBuffer.GetPointer(), iBytesRead);

		//	Parse to see if we're done

		if (!Response.InitFromPartialBuffer(TempBuffer))
			return strPattern("Unable to parse HTTP message.");
		}

	//	Done

	theSocket.Disconnect();

	return CString(ResponseBuff.GetPointer(), ResponseBuff.GetLength());
	}

CString ExecuteLispCommand (const CString &sInput)
	{
	CString sOutput;

	CDatum dExpression;
	if (!CHexeDocument::ParseLispExpression(sInput, &dExpression, &sOutput))
		return sOutput;

	CDatum dResult;
	CHexeProcess::ERun iRun = g_Process.Run(dExpression, &dResult);
	switch (iRun)
		{
		case CHexeProcess::ERun::OK:
		case CHexeProcess::ERun::Error:
		case CHexeProcess::ERun::ForcedTerminate:
			return dResult.AsString();

		default:
			return CString("Unable to complete run.");
		}
	}

int ExecuteScript (const SOptions &Options)
	{
	int i, j;

	//	Load the script file

	CDatum dScript;
	CString sError;
	if (!CDatum::CreateFromFile(Options.sScriptFile, CDatum::EFormat::AEONScript, &dScript, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	//	Get the server to connect to

	CStringView sServer = dScript.GetElement(FIELD_SERVER);
	if (sServer.IsEmpty())
		sServer = Options.sServer;

	//	Connect

	CSocket theSocket;
	if (!ConnectToArcology(STR_ARC_CONSOLE, sServer, Options, &theSocket))
		return 1;

	//	Run the script

	CDatum dCommands = dScript.GetElement(FIELD_COMMANDS);
	for (i = 0; i < dCommands.GetCount(); i++)
		{
		CDatum dCommand = dCommands.GetElement(i);

		//	Generate a command-line from the command

		CStringBuffer Buffer;
		for (j = 0; j < dCommand.GetCount(); j++)
			{
			if (j != 0)
				Buffer.Write(" ", 1);

			dCommand.Serialize(CDatum::EFormat::AEONScript, Buffer);
			}

		//	Run

		printf("%s\n", (LPSTR)(const CString &)Buffer);
		CString sResult = ExecuteArcologyCommand(theSocket, Buffer);
		PrintUTF8(sResult);
		printf("\n");
		}

	//	Done

	return 0;
	}

CString ExecuteUpgrade (CSocket &theSocket, const CString &sCmd)
	{
	int i;

	CString sRoot = fileGetPath(fileGetExecutableFilespec());

	//	Make a list of all executable files to upgrade

	TArray<CString> FileList;
	if (!fileGetFileList(sRoot, NULL_STR, CString("*.exe"), FFL_FLAG_RELATIVE_FILESPEC, &FileList))
		return CString("ERROR: Unable to obtain a list of executable files to upgrade.");

	//	Prepare a request upgrade command

	CStringBuffer Output;
	Output.Write("requestUpgrade (", 16);

	for (i = 0; i < FileList.GetCount(); i++)
		{
		CString sFilespec = fileAppend(sRoot, FileList[i]);

		//	Version

		SFileVersionInfo Info;
		if (!fileGetVersionInfo(sFilespec, &Info))
			{
			printf("ERROR: Unable to get file version: %s\n", (LPSTR)sFilespec);
			continue;
			}

		CIPInteger Version(Info.dwProductVersion);
		CString sVersion = Version.AsString();

		//	Checksum

		DWORD dwChecksum = fileChecksumAdler32(sFilespec);
		if (dwChecksum == 0)
			{
			printf("ERROR: Unable to get file checksum: %s\n", (LPSTR)sFilespec);
			continue;
			}

		CString sOutput = strPattern("{filename:\"%s\" version:%s checksum:%d} ", FileList[i], sVersion, dwChecksum);
		Output.Write(sOutput);
		}

	Output.Write(")", 1);

	//	Send the command

	CString sSend = CString::CreateFromHandoff(Output);
	CString sResult;
	CDatum dResult;
	ExecuteArcologyCommand(theSocket, sSend, &sResult, &dResult);
	if (strEquals(sResult, CString("ERROR")))
		return dResult.AsString();

	//	Show all the files to upgrade

	CDatum dUpgradeDesc = dResult.GetElement(0).GetElement(FIELD_UPGRADE_DESC);
	for (i = 0; i < dUpgradeDesc.GetCount(); i++)
		{
		CDatum dFileDesc = dUpgradeDesc.GetElement(i);

		printf("Upgrading %s\n", (LPSTR)dFileDesc.GetElement(FIELD_FILENAME).AsString());
		}

	//	Confirm

	CString sConfirm = GetInputLine(CString("\nAre you sure you want to upgrade the arcology? [y/n] : "));
	if (*sConfirm.GetParsePointer() != 'y' && *sConfirm.GetParsePointer() != 'Y')
		return NULL_STR;

	//	Upload the new files.

	for (i = 0; i < dUpgradeDesc.GetCount(); i++)
		{
		CDatum dFileDesc = dUpgradeDesc.GetElement(i);
		CStringView sFilename = dFileDesc.GetElement(FIELD_FILENAME);
		CString sFilespec = fileAppend(sRoot, sFilename);

		CString sResult = UploadFile(theSocket, CMD_UPLOAD_UPGRADE, sFilename, sFilespec);
		printf("%s\n", (LPSTR)sResult);
		}

	//	Complete the upgrade

	return ExecuteArcologyCommand(theSocket, CMD_COMPLETE_UPGRADE);
	}

CString ExecuteUpload (CSocket &theSocket, const CString &sCmd)
	{
	char *pPos = sCmd.GetParsePointer() + STR_UPLOAD_PREFIX.GetLength();
	
	//	Get the filePath

	if (*pPos == '"')
		pPos++;

	char *pStart = pPos;
	while (*pPos != ' ' && *pPos != '"' && *pPos != '\0')
		pPos++;

	CString sFilePath(pStart, pPos - pStart);
	if (*pPos != '\0')
		pPos++;

	if (*pPos == '"')
		pPos++;

	//	Skip whitespace

	while (*pPos == ' ')
		pPos++;
	
	//	Get the filespec

	pStart = pPos;
	while (*pPos != '\0')
		pPos++;

	CString sFilespec(pStart, pPos - pStart);

	//	Load the file

	return UploadFile(theSocket, CMD_UPLOAD, sFilePath, sFilespec);
	}

CString ExecuteUploadCertificate (CSocket &theSocket, const CString &sCmd)
	{
	char *pPos = sCmd.GetParsePointer() + STR_UPLOAD_CERTIFICATE_PREFIX.GetLength();
	
	//	Skip whitespace

	while (*pPos == ' ')
		pPos++;
	
	//	Get the filespec

	char *pStart = pPos;
	while (*pPos != '\0')
		pPos++;

	CString sFilespec(pStart, pPos - pStart);

	//	Upload

	return UploadFile(theSocket, CMD_UPLOAD, CString("/Arc.certificates"), sFilespec);
	}

CString ExecuteUploadPackage (CSocket &theSocket, const CString &sCmd)
	{
	int i;
	char *pPos = sCmd.GetParsePointer() + STR_UPLOAD_PACKAGE_PREFIX.GetLength();
	
	//	Get the package name

	if (*pPos == '"')
		pPos++;

	char *pStart = pPos;
	while (*pPos != ' ' && *pPos != '"' && *pPos != '\0')
		pPos++;

	CString sPackageName(pStart, pPos - pStart);
	if (*pPos != '\0')
		pPos++;

	if (*pPos == '"')
		pPos++;

	//	Skip whitespace

	while (*pPos == ' ')
		pPos++;
	
	//	Get the filespec

	pStart = pPos;
	while (*pPos != '\0')
		pPos++;

	CString sFilespec(pStart, pPos - pStart);

	//	Get the directory

	CString sPackageFolder = fileGetPath(sFilespec);
	CString sPackageDescFilespec = fileGetFilename(sFilespec);

	//	Upload the package descriptor

	CString sPackageDesc = strPattern("/Arc.services/%s.ars", sPackageName);
	printf("Uploading %s to /Arc.services/%s.ars...", (LPSTR)sFilespec, (LPSTR)sPackageName);
	CString sResult = UploadFile(theSocket, CMD_UPLOAD, sPackageDesc, sFilespec);
	printf("%s\n", (LPSTR)sResult);

	//	Now loop over all files in the directory

	TArray<CString> Files;
	if (!fileGetFileList(fileAppend(sPackageFolder, CString("*.*")), 
			FFL_FLAG_RELATIVE_FILESPEC | FFL_FLAG_RECURSIVE,
			&Files))
		{
		printf("ERROR: Unable to list directory: %s\\*.*\n", (LPSTR)sPackageFolder);
		return NULL_STR;
		}

	for (i = 0; i < Files.GetCount(); i++)
		{
		if (!strEquals(Files[i], sPackageDescFilespec))
			{
			CString sFilePath = strPattern("/Arc.services/%s/%s", sPackageName, CAeonInterface::FilespecToFilePath(Files[i]));
			printf("Uploading %s to %s...", (LPSTR)fileAppend(sPackageFolder, Files[i]), (LPSTR)sFilePath);
			CString sResult = UploadFile(theSocket, CMD_UPLOAD, sFilePath, fileAppend(sPackageFolder, Files[i]));
			printf("%s\n", (LPSTR)sResult);
			}
		}

	//	Refresh

	printf("Refreshing Hyperion...");
	return ExecuteArcologyCommand(theSocket, CMD_REFRESH_PACKAGES);
	}

CString GetInputLine (const CString &sPrompt)
	{
	char szBuffer[1024];
	printf((LPSTR)sPrompt);
	gets_s(szBuffer, sizeof(szBuffer)-1);
	return CString(szBuffer);
	}

void ParseCommandLine (int argc, char *argv[], SOptions *retOptions)
	{
	int i;

	//	Initialize

	*retOptions = SOptions();

	//	Parse

	for (i = 1; i < argc; i++)
		{
		char *pPos = argv[i];
		if (*pPos == '/')
			{
			pPos++;

			switch (*pPos)
				{
				case '?':
					retOptions->bHelp = true;
					break;

				case '1':
					retOptions->bV1Auth = true;
					break;

				case '!':
					retOptions->bV1Auth = true;
					retOptions->bV1AuthOld = true;
					break;

				case 'h':
				case 'H':
					pPos++;
					if (*pPos != ':')
						break;
					pPos++;

					retOptions->sHexeDocument = CString(pPos);
					retOptions->bNoConnect = true;
					break;

				case 'l':
				case 'L':
					retOptions->bTopLevel = true;
					retOptions->bNoConnect = true;
					break;

				case 'n':
				case 'N':
					retOptions->bNoLogo = true;
					break;

				case 'r':
				case 'R':
					{
					pPos++;
					if (*pPos != ':')
						break;
					pPos++;

					retOptions->sScriptFile = CString(pPos);
					retOptions->bNoConnect = true;
					break;
					}

				case 's':
				case 'S':
					{
					pPos++;
					if (*pPos != ':')
						break;
					pPos++;

					retOptions->sServer = CString(pPos);
					break;
					}

				case 't':
				case 'T':
					retOptions->bTiming = true;
					break;

				case 'z':
				case 'Z':
					retOptions->bNoConnect = true;
					break;
				}
			}
		else
			{
			if (retOptions->sSingleCommand.IsEmpty())
				retOptions->sSingleCommand = CString(pPos);
			else
				retOptions->sSingleCommand = strPattern("%s %s", retOptions->sSingleCommand, CString(pPos));
			}
		}

	//	If the only argument is a PEM file, then remember that

	if (argc == 2 
			&& strEqualsNoCase(fileGetExtension(retOptions->sSingleCommand), EXTENSION_PEM))
		{
		retOptions->bPEMFile = true;
		retOptions->bNoConnect = true;
		}

	//	Set some defaults

	if (retOptions->sServer.IsEmpty())
		retOptions->sServer = STR_LOCALHOST;
	}

int AI1 (SOptions &Options)
	{
	CSocket theSocket;

	//	Help

	if (!Options.bNoLogo)
		{
		printf("AI1 1.0\n");
		printf("Copyright (c) 2011-2023 by GridWhale Corporation. All Rights Reserved.\n");
		printf("\n");
		}

	if (Options.bHelp)
		{
		printf("ai1 [options] [\"command\"]\n");
		printf("\n");
		printf("  /?              Help.\n");
		printf("  /h:{filespec}   Run HexeDocument.\n");
		printf("  /l              Lisp engine top-level.\n");
		printf("  /n              No logo.\n");
		printf("  /r:{filespec}   Run script file.\n");
		printf("  /s:{hostname}   Connect to given server.\n");
		printf("  /t              Time each command.\n");
		printf("  /z              Do not connect to server.\n");
		printf("  /1              V1 authentication.\n");
		printf("  /!              V1 auth to old server.\n");

		if (Options.bNoConnect)
			return 0;

		printf("\n");
		Options.sSingleCommand = CString("help");
		}

	//	Connect (if necessary)

	if (!ConnectToArcology(STR_ARC_CONSOLE, Options, &theSocket))
		return 1;

	//	If we have a HexeDocument, run it.

	if (!Options.sHexeDocument.IsEmpty())
		return ExecuteHexeDocument(Options);

	//	If we have a script file, defer to it.

	else if (!Options.sScriptFile.IsEmpty())
		return ExecuteScript(Options);

	//	PEM file

	else if (Options.bPEMFile)
		return ExecuteDebugPEMFile(Options.sSingleCommand);

	//	If we have a single command, send it

	else if (!Options.sSingleCommand.IsEmpty())
		{
		printf("%s\n", (LPSTR)Options.sSingleCommand);

		CString sOutput = Execute(theSocket, Options, Options.sSingleCommand);
		PrintUTF8(sOutput);
		printf("\n");

		return 0;
		}

	//	Otherwise, do an interactive loop

	else
		{
		while (true)
			{
			CString sInput = GetInputLine(STR_PROMPT);

			if (strEquals(sInput, STR_QUIT))
				break;

			else if (!sInput.IsEmpty())
				{
				CString sOutput = Execute(theSocket, Options, sInput);
				PrintUTF8(sOutput);
				printf("\n");
				}
			}

		return 0;
		}
	}

int main (int argc, char* argv[])
	{
	CString sError;
	if (!CFoundation::Boot(0, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	CHexe::Boot();
	g_Process.LoadLibrary(LIBRARY_CORE);

	//	Commands

	SOptions Options;
	ParseCommandLine(argc, argv, &Options);

	//	Do it

	return AI1(Options);
	}

void PrintUTF8 (const CString sString)
	{
	CString16 sUnicode(strEscapePrintf(sString));
	wprintf((LPTSTR)sUnicode);
	}

