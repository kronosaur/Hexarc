//	ArcologyCommands.cpp
//
//	Runs commands on the arcology
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(CMD_ADMIN_REQUIRED,				"ADMIN-REQUIRED")
DECLARE_CONST_STRING(CMD_AUTH,							"AUTH")
DECLARE_CONST_STRING(CMD_AUTH_INVALID,					"AUTH-INVALID")
DECLARE_CONST_STRING(CMD_AUTH_REQUIRED,					"AUTH-REQUIRED")
DECLARE_CONST_STRING(CMD_AUTH_V1,						"AUTH-V1")
DECLARE_CONST_STRING(CMD_CREATE_ADMIN,					"createAdmin")
DECLARE_CONST_STRING(CMD_ERROR,							"ERROR")
DECLARE_CONST_STRING(CMD_ERROR_PREFIX,					"ERROR ")
DECLARE_CONST_STRING(CMD_OK,							"OK")
DECLARE_CONST_STRING(CMD_OUT,							"OUT")
DECLARE_CONST_STRING(CMD_WELCOME,						"WELCOME")

DECLARE_CONST_STRING(STR_PROMPT_PASSWORD,				"Password: ")
DECLARE_CONST_STRING(STR_PROMPT_PASSWORD_CONFIRM,		"Confirm password: ")
DECLARE_CONST_STRING(STR_PROMPT_USERNAME,				"Username: ")

bool ConnectToArcology (const CString &sInterface, SOptions &Options, CSocket *retSocket)
	{
	if (Options.bNoConnect)
		return true;

	//	Connect to the server

	return ConnectToArcology(sInterface, Options.sServer, Options, retSocket);
	}

bool ConnectToArcology (const CString &sInterface, const CString &sServer, const SOptions &Options, CSocket *retSocket)
	{
	CString sResult;
	CString sCommand;
	CDatum dPayload;

	//	Connect to the server

	printf("Connecting to %s...", (LPSTR)sServer);
	if (!retSocket->Connect(sServer, 7399))
		{
		printf("Unable to connect.\n");
		return false;
		}

	printf("OK\n");

	//	Access the proper interface

	printf("Accessing %s interface...", (LPSTR)sInterface);
	sResult = ExecuteArcologyCommand(*retSocket, strPattern("CONNECT %s", sInterface), &sCommand, &dPayload);
	if (strStartsWith(sResult, CMD_ERROR_PREFIX))
		{
		printf("%s\n", (LPSTR)sResult);
		return false;
		}

	//	Connection completed

	printf("OK\n");

	//	See if we need to set up the arcology

	if (!HandleConnectResponse(*retSocket, sCommand, dPayload, Options, &sResult))
		return false;

	//	Done

	printf((LPSTR)sResult);
	printf("\n");
	return true;
	}

CString ExecuteArcologyCommand (CSocket &theSocket, const CString &sCmd, CString *retsCommand, CDatum *retdPayload)
	{
	int i;

	if (!theSocket.IsValid())
		return CString("No server connection.");

	//	Send the data

	CString sSendBuffer = ParseAI1Command(sCmd);
	//CString sSendBuffer = strPattern("AI/1.00 %s|", sCmd);
	int iBytesSent = theSocket.Write(sSendBuffer);
	if (iBytesSent == 0)
		return CString("Unable to communicate with server.");

	//	Wait for a reply

	CAI1Stream Stream;
	while (true)
		{
		//	Keep parsing until we're done

		Stream.ReadFromSocket(theSocket);

		//	Get the next command

		CString sCommand;
		CDatum dPayload;
		Stream.GetNext(&sCommand, &dPayload);

		//	Compose the payload

		CStringBuffer Result;
		if (dPayload.GetBasicType() == CDatum::typeStruct)
			{
			for (i = 0; i < dPayload.GetCount(); i++)
				{
				CDatum dItem = dPayload.GetElement(i);
				if (i != 0)
					Result.Write("\n", 1);
				Result.Write(dPayload.GetKey(i));
				Result.Write("\t", 1);
				Result.Write(dItem.AsString());
				}
			}
		else if (dPayload.GetBasicType() == CDatum::typeArray && dPayload.GetCount() == 1 && dPayload.GetElement(0).GetBasicType() == CDatum::typeStruct)
			{
			CDatum dStruct = dPayload.GetElement(0);
			for (i = 0; i < dStruct.GetCount(); i++)
				{
				CDatum dItem = dStruct.GetElement(i);
				if (i != 0)
					Result.Write("\n", 1);
				Result.Write(dStruct.GetKey(i));
				Result.Write(":\t", 2);
				Result.Write(dItem.AsString());
				}
			}
		else
			{
			for (i = 0; i < dPayload.GetCount(); i++)
				{
				CDatum dItem = dPayload.GetElement(i);
				if (i != 0)
					Result.Write("\n", 1);
				Result.Write(dItem.AsString());
				}
			}

		CString sPayload;
		sPayload.TakeHandoff(Result);

		//	Return command and payload separately, if necessary

		if (retsCommand)
			*retsCommand = sCommand;

		if (retdPayload)
			*retdPayload = dPayload;

		//	If the command is OUT then we need to parse more. Otherwise
		//	we're done receiving the reply.

		if (strEquals(sCommand, CMD_OUT))
			{
			PrintUTF8(sPayload);
			printf("\n");
			}

		//	If the result is OK then we just return the payload

		else if (strEquals(sCommand, CMD_OK))
			{
			if (dPayload.IsNil())
				return CMD_OK;
			else
				return sPayload;
			}

		//	Otherwise output both the command and the payload

		else
			return strPattern("%s %s", sCommand, sPayload);
		}

	//	Never gets here
	return NULL_STR;
	}

bool HandleConnectResponse (CSocket &theSocket, const CString &sFirstCommand, CDatum dFirstPayload, const SOptions &Options, CString *retsWelcome)
	{
	CString sCommand = sFirstCommand;
	CDatum dPayload = dFirstPayload;

	//	Keep looping until we handle the entire connect protocol

	while (true)
		{
		//	WELCOME means that the connection has succeeded

		if (strEquals(sCommand, CMD_WELCOME))
			{
			*retsWelcome = dPayload.GetElement(0);
			return true;
			}

		//	ADMIN-REQUIRED means that we need to create an admin account before we
		//	can proceed.

		else if (strEquals(sCommand, CMD_ADMIN_REQUIRED))
			{
			printf("Arcology requires an admin account.\n");

			//	Ask for an admin username

			CString sUsername = GetInputLine(STR_PROMPT_USERNAME);
			if (sUsername.IsEmpty())
				return false;

			//	Ask for a password

			CString sPassword;
			CString sPasswordConfirm;
			do
				{
				sPassword = GetInputLine(STR_PROMPT_PASSWORD);
				sPasswordConfirm = GetInputLine(STR_PROMPT_PASSWORD_CONFIRM);
				}
			while (!strEquals(sPassword, sPasswordConfirm));

			//	Generate an authDesc structure

			CDatum dAuthDesc = CAI1Protocol::CreateAuthDescSHAPassword(sUsername, sPassword);
			
			//	Generate the proper command

			CStringBuffer Command;
			Command.Write(CMD_CREATE_ADMIN);
			Command.Write(" ", 1);
			CDatum(sUsername).Serialize(CDatum::formatAEONScript, Command);
			Command.Write(" ", 1);
			dAuthDesc.Serialize(CDatum::formatAEONScript, Command);

			//	Execute it and get a reply

			ExecuteArcologyCommand(theSocket, Command, &sCommand, &dPayload);

			//	Loop and process the reply
			}

		//	AUTH-REQUIRED means that we need to log in before we can proceeed

		else if (strEquals(sCommand, CMD_AUTH_REQUIRED)
				|| strEquals(sCommand, CMD_AUTH_INVALID))
			{
			if (strEquals(sCommand, CMD_AUTH_INVALID))
				printf("Invalid username or password\n");

			//	Get the challenge

			CDatum dChallenge = dPayload.GetElement(0);

			//	Ask for username and password

			CString sUsername = GetInputLine(STR_PROMPT_USERNAME);
			if (sUsername.IsEmpty())
				return false;

			CString sPassword = GetInputLine(STR_PROMPT_PASSWORD);

			//	Generate the response to the challenge

			CDatum dResponse;
			if (Options.bV1Auth)
				dResponse = CAI1Protocol::CreateSHAPasswordChallengeResponse_V1(sUsername, sPassword, dChallenge);
			else
				CCryptosaurInterface::CreateChallengeCredentials(sUsername, sPassword, dChallenge, &dResponse);

			//	Generate the proper command

			CStringBuffer Command;
			if (Options.bV1Auth && !Options.bV1AuthOld)
				Command.Write(CMD_AUTH_V1);
			else
				Command.Write(CMD_AUTH);
			Command.Write(" ", 1);
			CDatum(sUsername).Serialize(CDatum::formatAEONScript, Command);
			Command.Write(" ", 1);
			dResponse.Serialize(CDatum::formatAEONScript, Command);

			//	Execute it and get a reply

			ExecuteArcologyCommand(theSocket, Command, &sCommand, &dPayload);

			//	Loop and process the reply
			}

		//	Error

		else if (strEquals(sCommand, CMD_ERROR))
			{
			printf("ERROR: %s\n", (LPSTR)(const CString &)dPayload.GetElement(0));
			return false;
			}

		//	Otherwise, we don't know

		else
			{
			printf("Unknown response: %s %s\n", (LPSTR)sCommand, (LPSTR)dPayload.AsString());
			return false;
			}
		}
	}

CString UploadFile (CSocket &theSocket, const CString &sOp, const CString &sFilePath, const CString &sFilespec)
	{
	//	Load the file

	CFile theFile;
	CString sError;
	if (!theFile.Create(sFilespec, CFile::FLAG_OPEN_READ_ONLY, &sError))
		return sError;

	DWORDLONG dwTotalSize = theFile.GetStreamLength();
	DWORDLONG dwLeft = dwTotalSize;
	DWORDLONG dwPos = 0;

	CString sFinalResult;
	while (dwLeft > 0)
		{
		DWORDLONG dwChunk = Min(dwLeft, MEGABYTE_DISK / 8);

		//	Read the data from the file

		CDatum dData;
		if (!CDatum::CreateBinary(theFile, (int)dwChunk, &dData))
			return strPattern("Unable to load file: %s\n", sFilespec);

		//	Compose a command buffer

		CBuffer Command((int)dwChunk + 1000);
		CString sCommand;
		if (dwChunk == dwTotalSize)
			sCommand = strPattern("%s \"%s\" { } ", sOp, sFilePath);
		else
			sCommand = strPattern("%s \"%s\" { uploadSize:%s partialPos:%s } ", sOp, sFilePath, CIPInteger(dwTotalSize).AsString(), CIPInteger(dwPos).AsString());

		Command.Write(sCommand);

		//	Serialize data

		dData.Serialize(CDatum::formatAEONScript, Command);

		//	Send the command

		CString sResultCmd;
		CString sResult = ExecuteArcologyCommand(theSocket, CString(Command.GetPointer(), Command.GetLength()), &sResultCmd);
		if (strEquals(sResultCmd, CMD_ERROR))
			return sResult;
		else if (dwChunk == dwTotalSize)
			sFinalResult = sResult;
		else
			{
			printf("Uploading %s...%d%%\n", (LPSTR)sFilePath, (int)(100 * (dwPos + dwChunk) / dwTotalSize));
			sFinalResult = CMD_OK;
			}

		//	Next

		dwLeft -= dwChunk;
		dwPos += dwChunk;
		}

	//	Done

	return sFinalResult;
	}
