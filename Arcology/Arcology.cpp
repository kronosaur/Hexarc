// Arcology.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(CMDLINE_ARCOLOGY,					"arcology")
DECLARE_CONST_STRING(CMDLINE_CONFIG,					"config")
DECLARE_CONST_STRING(CMDLINE_HEXARC_PORT,				"hexarcPort")

class CArcologyService : public CWin32Service
	{
	public:
		virtual void GetInfo (SWin32ServiceInfo *retInfo);
		virtual void OnRun (void);
		virtual void OnStart (const TArray<CString> &Params);
		virtual void OnStop (void);
		virtual void OnWaitForStop (void);

	private:
		struct SOptions
			{
			SOptions (void) :
					dwAMP1Port(0)
				{ }

			CString sArcologyPrime;			//	If set, then we should connect to this arcology
			CString sConfigFilename;		//	Name of config file (default to "Config.ars")
			DWORD dwAMP1Port;				//	Defaults 7367
			};

		void ParseOptions (const TArray<CString> &Params, SOptions *retOptions) const;

		CArchonProcess m_Process;
		HANDLE m_hStop;
	};

void CArcologyService::GetInfo (SWin32ServiceInfo *retInfo)
	{
	retInfo->sServiceID = "HexarcService";
	retInfo->sServiceName = "Hexarc";
	retInfo->sServiceDesc = "Implements a Hexarc arcology.";
	}

void CArcologyService::OnRun (void)
	{
	m_Process.Run();
	::SetEvent(m_hStop);
	}

void CArcologyService::OnStart (const TArray<CString> &Params)
	{
#ifdef DEBUG
	//	Set the codepage of the console to UTF8

	::SetConsoleOutputCP(65001);
#endif

	//	Need to register Luminous objects so that we can serve them.

	CAEONLuminous::Boot();

	//	Get options from the command line

	SOptions Options;
	ParseOptions(Params, &Options);

	//	If we're not connecting to an arcology, then we ARE the primary machine
	//	in the arcology.

	bool bIsArcologyPrime = (Options.sArcologyPrime.IsEmpty());

	//	Create the process

	SProcessDesc Config;
	Config.dwFlags = PROCESS_FLAG_CENTRAL_MODULE;
	Config.dwFlags |= (bIsArcologyPrime ? PROCESS_FLAG_ARCOLOGY_PRIME : 0);
	Config.dwFlags |= (InServiceDebugMode() ? PROCESS_FLAG_CONSOLE_OUTPUT : 0);

	//	Generate a machine name

	Config.sMachineName = strPattern("%s-%x", sysGetDNSName(), sysGetTickCount());
	
	//	Add engines that are available on all machines.
	//
	//	Esper

	SEngineDesc *pDesc;
	pDesc = Config.Engines.Insert();
	pDesc->pEngine = new CEsperEngine;

	//	Exarch

	CExarchEngine::SOptions ExarchOptions;
	ExarchOptions.sArcologyPrime = Options.sArcologyPrime;
	ExarchOptions.sConfigFilename = Options.sConfigFilename;
	ExarchOptions.dwAMP1Port = Options.dwAMP1Port;

	pDesc = Config.Engines.Insert();
	CExarchEngine *pExarch = new CExarchEngine(ExarchOptions);
	pDesc->pEngine = pExarch;
	Config.pExarch = pExarch;

	//	DrHouse

	pDesc = Config.Engines.Insert();
	pDesc->pEngine = new CDrHouseEngine;

	//	Now add engines that are only available on Arcology Prime

	if (bIsArcologyPrime)
		{
		//	Cryptosaur

		pDesc = Config.Engines.Insert();
		pDesc->pEngine = new CCryptosaurEngine;

		//	Hexe

		pDesc = Config.Engines.Insert();
		pDesc->pEngine = new CHexeEngine;

		//	Hyperion

		pDesc = Config.Engines.Insert();
		pDesc->pEngine = new CHyperionEngine;
		}

	//	Boot

	m_Process.Boot(Config);

	m_hStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	}

void CArcologyService::OnStop (void)
	{
	m_Process.SignalShutdown();
	}

void CArcologyService::OnWaitForStop (void)
	{
	::WaitForSingleObject(m_hStop, INFINITE);
	::CloseHandle(m_hStop);
	m_hStop = NULL;
	}

void CArcologyService::ParseOptions (const TArray<CString> &Params, SOptions *retOptions) const

//	ParseOptions
//
//	Parses options

	{
	int i;

	//	Initialize to defaults

	retOptions->sArcologyPrime = NULL_STR;
	retOptions->sConfigFilename = NULL_STR;

	//	Append all the parameters together so that we can parse ourselves

	CString sLine = (Params.GetCount() > 0 ? Params[0] : NULL_STR);
	for (i = 1; i < Params.GetCount(); i++)
		sLine += strPattern(" %s", Params[i]);

	//	Now parse the line

	char *pPos = sLine.GetParsePointer();
	char *pStart;
	while (*pPos != '\0')
		{
		//	Skip until we have a parameter

		while (*pPos != '/' && *pPos != '-' && *pPos != '\0')
			pPos++;

		if (*pPos == '\0')
			break;

		pPos++;

		//	Get the command

		pStart = pPos;
		while (!strIsWhitespace(pPos) && *pPos != ':' && *pPos != '\0')
			pPos++;

		CString sCmd(pStart, pPos - pStart);

		//	Get the parameter

		CString sParam;
		if (*pPos == ':')
			{
			pPos++;

			//	Skip whitespace

			while (strIsWhitespace(pPos))
				pPos++;

			//	See if we have quotes

			if (*pPos == '\"')
				{
				pPos++;

				pStart = pPos;
				while (*pPos != '\"' && *pPos != '\0')
					pPos++;

				sParam = CString(pStart, pPos - pStart);

				if (*pPos == '\"')
					pPos++;
				}

			//	Otherwise, take until whitepace

			else
				{
				pStart = pPos;
				while (!strIsWhitespace(pPos) && *pPos != '\0')
					pPos++;

				sParam = CString(pStart, pPos - pStart);
				}
			}

		//	Now deal with the options

		if (strEquals(sCmd, CMDLINE_ARCOLOGY))
			retOptions->sArcologyPrime = sParam;

		else if (strEquals(sCmd, CMDLINE_CONFIG))
			retOptions->sConfigFilename = sParam;

		else if (strEquals(sCmd, CMDLINE_HEXARC_PORT))
			retOptions->dwAMP1Port = strToInt(sParam, 0);
		else
			{
			//	LATER: Report error
			}
		}
	}

CWin32Service *CreateWin32Service (void)
	{
	return new CArcologyService;
	}
