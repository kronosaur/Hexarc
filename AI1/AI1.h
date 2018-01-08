//	AI1.h
//
//	AI1 header
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

struct SOptions
	{
	SOptions (void) :
			bHelp(false),
			bNoConnect(false),
			bTopLevel(false),
			bTiming(false),
			bNoLogo(false),
			bV1Auth(false),
			bV1AuthOld(false),
			bPEMFile(false)
		{ }

	bool bHelp;
	bool bNoConnect;
	bool bTopLevel;
	bool bTiming;
	bool bNoLogo;
	bool bV1Auth;
	bool bV1AuthOld;
	bool bPEMFile;
	CString sServer;
	CString sSingleCommand;
	CString sScriptFile;
	CString sHexeDocument;
	};

bool ConnectToArcology (const CString &sInterface, SOptions &Options, CSocket *retSocket);
bool ConnectToArcology (const CString &sInterface, const CString &sServer, const SOptions &Options, CSocket *retSocket);
CString ExecuteArcologyCommand (CSocket &theSocket, const CString &sCmd, CString *retsCommand = NULL, CDatum *retdPayload = NULL);
bool HandleConnectResponse (CSocket &theSocket, const CString &sFirstCommand, CDatum dFirstPayload, const SOptions &Options, CString *retsWelcome);
CString ParseAI1Command (const CString &sInput);
CString UploadFile (CSocket &theSocket, const CString &sOp, const CString &sFilePath, const CString &sFilespec);

int ExecuteHexeDocument (const SOptions &Options);
void RegisterAI1Library (void);

int ExecuteDebugPEMFile (const CString &sFilespec);

CString GetInputLine (const CString &sPrompt);
void PrintUTF8 (const CString sString);


