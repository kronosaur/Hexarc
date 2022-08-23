//	CArchonProcess.cpp
//
//	CArchonProcess class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const DWORD MAX_GARBAGE_COLLECT_WAIT =					15 * 60 * 1000;		//	Garbage must be collected at least every 15 minutes
const DWORDLONG MAX_GARBAGE_COLLECT_MEMORY =			1024 * 1024 * 1024;	//	If we reach 1 GB of private working set, then collect garbage

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command");
DECLARE_CONST_STRING(PORT_ARC_LOG,						"Arc.log");
DECLARE_CONST_STRING(ADDR_NULL,							"Arc.null");
DECLARE_CONST_STRING(ADDRESS_EXARCH_LOG,				"Exarch.log@~/CentralModule");
DECLARE_CONST_STRING(ADDR_EXARCH_COMMAND,				"Exarch.command@~/CentralModule");
DECLARE_CONST_STRING(ADDRESS_HYPERION_COMMAND,			"Hyperion.command");

DECLARE_CONST_STRING(FIELD_ON_ARCOLOGY_PRIME,			"onArcologyPrime");

DECLARE_CONST_STRING(STR_CENTRAL_MODULE,				"CentralModule");
DECLARE_CONST_STRING(STR_ARCOLOGY_PRIME_SEMAPHORE,		"ArcologyPrimeRunning");
DECLARE_CONST_STRING(STR_CENTRAL_MODULE_SEMAPHORE,		"CentralModuleRunning");

DECLARE_CONST_STRING(MSG_ARC_HOUSEKEEPING,				"Arc.housekeeping");
DECLARE_CONST_STRING(MSG_ERROR_PREFIX,					"Error.");
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_EXARCH_ON_MODULE_START,		"Exarch.onModuleStart");
DECLARE_CONST_STRING(MSG_EXARCH_REPORT_DISK_ERROR,		"Exarch.reportDiskError");
DECLARE_CONST_STRING(MSG_HYPERION_SERVICE_MSG,			"Hyperion.serviceMsg");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");
DECLARE_CONST_STRING(MSG_TRANSPACE_DOWNLOAD,			"Transpace.download");

DECLARE_CONST_STRING(STR_NOTIFY_SUFFIX,					".notify");
DECLARE_CONST_STRING(STR_DEFAULT_ARCOLOGY,				"TransArc");

DECLARE_CONST_STRING(STR_ARCHON_BOOT,					"-");
DECLARE_CONST_STRING(STR_ARCOLOGY_PRIME,				"ArcologyPrime");
DECLARE_CONST_STRING(STR_BOOT_COMPLETE,					"Module started.");
DECLARE_CONST_STRING(STR_CENTRAL_MODULE_STARTED,		"CentralModule started.");
DECLARE_CONST_STRING(STR_GARBAGE_COLLECTION,			"Garbage collection: %d.%02d seconds (Sweep: %d.%02d seconds).");
DECLARE_CONST_STRING(STR_ENGINE_PAUSE_TIME,				"Garbage collection: %s took %d.%02d seconds to pause.");
DECLARE_CONST_STRING(STR_MARKING_MNEMOSYNTH,			"Marking MnemosynthDb.");
DECLARE_CONST_STRING(STR_MARKING_EVENT_THREAD,			"Marking EventThread.");
DECLARE_CONST_STRING(STR_MARKING_IMPORT_THREAD,			"Marking ImportThread.");
DECLARE_CONST_STRING(STR_MARK_AND_SWEEP,				"In CDatum::MarkAndSweep.");

DECLARE_CONST_STRING(ERR_CANT_BIND,						"Unable to bind to address: %s.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_BOOT_WINSOCK,		"Unable to boot WinSock.");
DECLARE_CONST_STRING(ERR_NOT_CENTRAL_MODULE,			"Unable to obtain CentralModule semphore.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_GET_VERSION,			"Unable to obtain module version information.");
DECLARE_CONST_STRING(ERR_CANT_GET_MEMORY_INFO,			"Unable to obtain process memory info.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_OPEN_MNEMOSYNTH,		"Unable to open Mnemosynth Db.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND_TO_EXARCH,		"Unable to send message to Exarch.");
DECLARE_CONST_STRING(ERR_CRASH_WHILE_BOOTING,			"Unhandled exception in CArchonProcess::Boot.");
DECLARE_CONST_STRING(ERR_DISK,							"Hard drive error: %s.");
DECLARE_CONST_STRING(ERR_BAD_TRANSPACE_ADDRESS,			"Invalid Transpace address: %s.");
DECLARE_CONST_STRING(ERR_CANT_SEND_TO,					"Cannot send message to address: %s.");
DECLARE_CONST_STRING(ERR_MEMORY_WARNING,				"WARNING: Process exceeding safe memory limits.");
DECLARE_CONST_STRING(ERR_NO_MACHINE_NAME,				"No machine name provided at processes launch.");
DECLARE_CONST_STRING(ERR_NOT_IN_CONSOLE_MODE,			"Not in console mode.");
DECLARE_CONST_STRING(ERR_CRASH_IN_CONSOLE_COMMAND,		"Crash processing console command.");
DECLARE_CONST_STRING(ERR_CRASH_IN_SEND_GLOBAL_MESSAGE,	"CRASH: In SendGlobalMessage.");
DECLARE_CONST_STRING(ERR_CRASH_IN_COLLECT_GARBAGE,		"CRASH: In CollectGarbage.");

CArchonProcess::CArchonProcess (void) :
		m_EventThread(m_RunEvent, m_PauseEvent, m_QuitEvent)

//	CArchonProcess constructor

	{
	//	We take responsibility for the Hexe system in this process

	CHexe::Boot();
	}

CArchonProcess::~CArchonProcess (void)

//	CArchonProcess destructor

	{
	int i;

	//	Free our engines

	for (i = 0; i < m_Engines.GetCount(); i++)
		delete m_Engines[i].pEngine;
	}

bool CArchonProcess::Boot (const SProcessDesc &Config)

//	Boot
//
//	Boots the process. We take ownership of all engines.

	{
	try
		{
		int i;

		//	Get our current process location

		CString sExecutableFilespec = fileGetExecutableFilespec();
		CString sExecutablePath = fileGetPath(sExecutableFilespec);

		//	LATER: Hard-coded for now

		m_sArcology = STR_DEFAULT_ARCOLOGY;

		//	Set some options

		m_bArcologyPrime = ((Config.dwFlags & PROCESS_FLAG_ARCOLOGY_PRIME) != 0);
		m_bOnArcologyPrime = ((Config.dwFlags & PROCESS_FLAG_ON_ARCOLOGY_PRIME) != 0);
		m_bCentralModule = ((Config.dwFlags & PROCESS_FLAG_CENTRAL_MODULE) != 0);
		m_bConsoleMode = ((Config.dwFlags & PROCESS_FLAG_CONSOLE_MODE) != 0);
		m_bDebugger = ((Config.dwFlags & PROCESS_FLAG_DEBUG) != 0);

		m_sMachineName = Config.sMachineName;

		//	Set the name

		if (m_bCentralModule)
			m_sName = STR_CENTRAL_MODULE;
		else
			{
			//	Get a module name by taking just the filename (without extension)

			fileGetExtension(fileGetFilename(sExecutableFilespec), &m_sName);
			}

		m_iState = stateNone;

		//	Initialize the black box so that we can record boot errors.
		//	(But only if we're the central module on arcology prime).

		if (m_bCentralModule
				&& m_bArcologyPrime)
			{
			m_BlackBox.Boot(sExecutablePath);
			if (Config.dwFlags & PROCESS_FLAG_CONSOLE_OUTPUT)
				m_BlackBox.SetConsoleOutput(true);

			LogBlackBox(STR_ARCHON_BOOT);
			}
		else if (m_bConsoleMode)
			{
			m_BlackBox.SetConsoleOutput(true);
			}

		//	Validate options

		if (m_sMachineName.IsEmpty() && !m_bConsoleMode)
			{
			LogBlackBox(ERR_NO_MACHINE_NAME);
			m_iState = stateBootError;
			return false;
			}

		//	Get version information

		if (!fileGetVersionInfo(NULL_STR, &m_Version))
			{
			LogBlackBox(ERR_UNABLE_TO_GET_VERSION);
			m_iState = stateBootError;
			return false;
			}

		//	Log version information

		if (m_bCentralModule || m_bConsoleMode)
			{
			LogBlackBox(strPattern("%s %s", m_Version.sProductName, m_Version.sProductVersion));
			LogBlackBox(m_Version.sCopyright);
			}

		//	Boot Foundation, including Winsock

		CString sError;
		if (!CFoundation::Boot(0, &sError))
			{
			LogBlackBox(sError);
			m_iState = stateBootError;
			return false;
			}

		//	Init some stuff

		m_dwLastGarbageCollect = sysGetTickCount();
		m_QuitEvent.Create();
		m_RunEvent.Create();
		m_PauseEvent.Create();
		m_QuitSignalEvent.Create();

		//	If we're the Central Module, then claim a mutex so that no one
		//	else tries it.

		if (m_bCentralModule)
			{
			if (m_bArcologyPrime)
				m_CentralModuleSem.Create(STR_ARCOLOGY_PRIME_SEMAPHORE, 1);
			else
				m_CentralModuleSem.Create(STR_CENTRAL_MODULE_SEMAPHORE, 1);

			if (!m_CentralModuleSem.TryIncrement())
				{
				LogBlackBox(ERR_NOT_CENTRAL_MODULE);
				m_iState = stateBootError;
				return false;
				}
			}

		//	Boot some stuff

		m_MnemosynthDb.Boot(this);
		m_Transporter.Boot(this);

		//	Boot some threads

		m_ImportThread.Boot(this, m_sName, m_RunEvent, m_PauseEvent, m_QuitEvent);

		//	Remember the Exarch engine, if we have it.

		m_pExarch = Config.pExarch;

		//	Add the list of engines

		DWORD dwEngineID = 0;

		//	Add the mnemosynth engine (if not in console mode)

		if (!m_bConsoleMode)
			{
			SEngine *pEngine = m_Engines.Insert();
			pEngine->pEngine = new CMnemosynthEngine(&m_MnemosynthDb);
			pEngine->pEngine->Boot(this, dwEngineID++);
			}

		//	Add user provided engines

		for (i = 0; i < Config.Engines.GetCount(); i++)
			{
			SEngine *pEngine = m_Engines.Insert();
			pEngine->pEngine = Config.Engines[i].pEngine;

			//	Allow the engine to add its ports

			pEngine->pEngine->Boot(this, dwEngineID++);
			}

		//	After this point we cannot create or destroy any engines.
		//
		//	Get all engine's command port (in case we need it).
		//	It is OK if an engine does not have a command port; that just means
		//	that it doesn't want any messages from us.

		for (i = 0; i < m_Engines.GetCount(); i++)
			m_Engines[i].pCommand = Bind(GenerateAddress(strPattern("%s.command", m_Engines[i].pEngine->GetName())));

		//	Publish this module's virtual ports to Mnemosynth

		m_Transporter.PublishToMnemosynth();

		//	Done

		return true;
		}
	catch (...)
		{
		LogBlackBox(ERR_CRASH_WHILE_BOOTING);
		m_iState = stateBootError;
		return false;
		}
	}

void CArchonProcess::CollectGarbage (void)

//	CollectGarbage
//
//	Collects garbage

	{
	CString sTask;

	try 
		{
		int i;

		//	Compute how long it's been since we last collected garbage

		DWORD dwStart = sysGetTickCount();
		DWORD dwTimeSinceLastCollection = dwStart - m_dwLastGarbageCollect;

		//	If it has been less than a certain time, then only collect garbage if
		//	all engines are idle.

		if (dwTimeSinceLastCollection < MAX_GARBAGE_COLLECT_WAIT)
			{
			//	Check to see if engines are idle

			bool bEnginesIdle = true;
			for (i = 0; i < m_Engines.GetCount(); i++)
				if (!m_Engines[i].pEngine->IsIdle())
					{
					bEnginesIdle = false;
					break;
					}

			//	If the engines are busy, then check to see if we have enough
			//	memory to wait some more.

			if (!bEnginesIdle)
				{
				CProcess CurrentProc;
				CurrentProc.CreateCurrentProcess();
				CProcess::SMemoryInfo MemoryInfo;
				if (!CurrentProc.GetMemoryInfo(&MemoryInfo))
					{
					LogBlackBox(ERR_CANT_GET_MEMORY_INFO);
					return;
					}

				//	If we have enough memory, then wait to collect garbage

				if (MemoryInfo.dwCurrentAlloc < MAX_GARBAGE_COLLECT_MEMORY)
					return;

				//	Warning that we're running out of memory

				LogBlackBox(ERR_MEMORY_WARNING);
				}
			}

		//	Collect

	#ifdef DEBUG_GARBAGE_COLLECTION
		printf("[%s] Collecting garbage.\n", (LPSTR)m_sName);
	#endif

		m_dwLastGarbageCollect = dwStart;

		m_RunEvent.Reset();
		m_PauseEvent.Set();

		//	Keep track of how long each engine takes to pause (for diagnostic
		//	purposes).

		TArray<DWORD> PauseTime;
		PauseTime.InsertEmpty(m_Engines.GetCount());

		//	Some engines still need an explicit call (because they don't have
		//	their own main thread).

		for (i = 0; i < m_Engines.GetCount(); i++)
			m_Engines[i].pEngine->SignalPause();

		//	Wait for the engines to stop

		for (i = 0; i < m_Engines.GetCount(); i++)
			{
			DWORD dwStart = sysGetTickCount();

			m_Engines[i].pEngine->WaitForPause();

			PauseTime[i] = sysGetTicksElapsed(dwStart);
			}

		//	Wait for our threads to stop

		m_EventThread.WaitForPause();
		m_ImportThread.WaitForPause();

		//	Now we ask all engines to mark their data in use

		for (i = 0; i < m_Engines.GetCount(); i++)
			{
			sTask = strPattern("Marking data: %s engine.", m_Engines[i].pEngine->GetName());
			m_Engines[i].pEngine->Mark();
			}

		//	Now we mark our own structures

		sTask = STR_MARKING_MNEMOSYNTH;
		m_MnemosynthDb.Mark();

		sTask = STR_MARKING_EVENT_THREAD;
		m_EventThread.Mark();

		sTask = STR_MARKING_IMPORT_THREAD;
		m_ImportThread.Mark();

		//	Now we sweep all unused

		sTask = STR_MARK_AND_SWEEP;

		DWORD dwSweepStart = sysGetTickCount();
		CDatum::MarkAndSweep();
		DWORD dwSweepTime = sysGetTickCount() - dwSweepStart;

		sTask = NULL_STR;

		//	Now we start all engines up again

		m_PauseEvent.Reset();
		m_RunEvent.Set();

		//	If garbage collection took too long, then we need to log it.

		DWORD dwTime = sysGetTickCount() - dwStart;
		if (dwTime >= 500)
			{
			//	Log each engine that took too long

			for (i = 0; i < m_Engines.GetCount(); i++)
				if (PauseTime[i] >= 500)
					Log(MSG_LOG_INFO, strPattern(STR_ENGINE_PAUSE_TIME, m_Engines[i].pEngine->GetName(), PauseTime[i] / 1000, (PauseTime[i] % 1000) / 10));

			//	Log overall time

			Log(MSG_LOG_INFO, strPattern(STR_GARBAGE_COLLECTION, 
					dwTime / 1000, (dwTime % 1000) / 10,
					dwSweepTime / 1000, (dwSweepTime % 1000) / 10));
			}
		}
	catch (...)
		{
		if (sTask.IsEmpty())
			CriticalError(ERR_CRASH_IN_COLLECT_GARBAGE);
		else
			CriticalError(strPattern("CRASH: %s", sTask));
		}
	}

CString CArchonProcess::ConsoleCommand (const CString &sCmd, const TArray<CDatum> &Args)

//	ConsoleCommand
//
//	Process a console command.

	{
	int i;

	if (!m_bConsoleMode)
		return ERR_NOT_IN_CONSOLE_MODE;

	//	Call the engines

	for (i = 0; i < m_Engines.GetCount(); i++)
		{
		CString sOutput;
		try
			{
			sOutput = m_Engines[i].pEngine->ConsoleCommand(sCmd, Args);
			if (!sOutput.IsEmpty())
				return sOutput;
			}
		catch (...)
			{
			return ERR_CRASH_IN_CONSOLE_COMMAND;
			}
		}

	//	If we get this far, then no engine handled it.

	return NULL_STR;
	}

void CArchonProcess::CriticalError (const CString &sError)

//	CriticalError
//
//	Report a critical error; a critical error is an error in the Run loop that
//	we cannot recover from.

	{
	static constexpr DWORD MAX_CRITICAL_ERRORS = 4;

	//	If we've exceeded the max count of critical errors, then we just throw
	//	and terminate the process.

	if (++m_dwCriticalErrors >= MAX_CRITICAL_ERRORS)
		throw CException(errFail);

	//	Otherwise, we log it, but continue.

	else if ((m_bCentralModule && m_bArcologyPrime) || m_bConsoleMode)
		LogBlackBox(sError);
	else
		Log(MSG_LOG_ERROR, sError);
	}

void CArchonProcess::OnMnemosynthDbModified (CDatum dLocalUpdates)

//	OnMnemosynthDbModified
//
//	Mnemosynth has been modified

	{
	}

bool CArchonProcess::OnModuleStart (CDatum dModuleData)

//	OnModuleStart
//
//	Sends a message to central process telling it our status

	{
	//	If we're not the CentralModule then we need to send a message to
	//	this machine's Exarch telling it that we have started.

	if (!m_bCentralModule && !m_bConsoleMode)
		{
		//	Figure out the Mnemosynth sequence number for this endpoint at this
		//	point in time. We send it over to Exarch so that it waits until it
		//	synchronizes to that point before assuming that loading is complete.

		CString sEndpoint = CMnemosynthDb::GenerateEndpointName(GetMachineName(), GetModuleName());
		MnemosynthSequence dwSeq = m_MnemosynthDb.GetSequence(sEndpoint);
		ASSERT(dwSeq != NULL_MNEMO_SEQ);
		if (dwSeq == NULL_MNEMO_SEQ)
			{
			Log(MSG_LOG_ERROR, ERR_UNABLE_TO_OPEN_MNEMOSYNTH);
			return false;
			}

		//	Compose the message

		CComplexArray *pArray = new CComplexArray;
		pArray->Insert(m_sName);
		pArray->Insert(dwSeq);
		pArray->Insert(dModuleData);

		if (!SendMessageCommand(ADDR_EXARCH_COMMAND, MSG_EXARCH_ON_MODULE_START, NULL_STR, 0, CDatum(pArray)))
			{
			Log(MSG_LOG_ERROR, ERR_UNABLE_TO_SEND_TO_EXARCH);
			return false;
			}
		}

	//	Done

	return true;
	}

void CArchonProcess::Run (void)

//	Run
//
//	This thread will start all engines and continue running
//	until all engines stop.

	{
	int i;

	if (m_bDebugger)
		DebugBreak();

	//	If we could not boot properly, then we stop

	if (m_iState == stateBootError)
		return;

	//	Set event indicating that thread should run

	m_RunEvent.Set();

	//	Start some threads

	m_ImportThread.Start();

	//	Compose some data for the Mnemosynth entry.

	CDatum dModuleData(CDatum::typeStruct);
	if (IsOnArcologyPrime())
		dModuleData.SetElement(FIELD_ON_ARCOLOGY_PRIME, true);

	//	Start all engines. At this point we have multiple threads
	//	potentially calling on the IArchonProcessCtx interface
	//
	//	This allows the engines to start at least one thread.
	//	LATER: Catch errors.

	for (i = 0; i < m_Engines.GetCount(); i++)
		{
		m_Engines[i].pEngine->StartRunning(m_RunEvent, m_PauseEvent, m_QuitEvent);
		m_Engines[i].pEngine->AccumulateModuleData(dModuleData);
		}

	//	Do stuff now that the module has started

	OnModuleStart(dModuleData);

	//	At this point it is safe to call the logging system

	if (m_bCentralModule)
		Log(MSG_LOG_INFO, STR_CENTRAL_MODULE_STARTED);
	else if (!m_bConsoleMode)
		Log(MSG_LOG_INFO, STR_BOOT_COMPLETE);

	//	If we've in console mode, then run the input loop.

	CConsoleThread Console(this);
	if (m_bConsoleMode)
		Console.Start();

	//	Loop until we quit. We stop every so often to do clean up
	//	tasks such as garbage collection.

	CWaitArray Wait;
	int QUIT_SIGNAL = Wait.Insert(m_QuitSignalEvent);

	while (true)
		{
		int iEvent = Wait.WaitForAny(15000);
		if (iEvent == QUIT_SIGNAL)
			break;
		else if (iEvent == OS_WAIT_TIMEOUT)
			{
			if (m_bConsoleMode)
				Console.OnStartHousekeeping();

			CollectGarbage();
			SendGlobalMessage(MSG_ARC_HOUSEKEEPING);

			if (m_bConsoleMode)
				Console.OnEndHousekeeping();
			}
		}

	//	Some engines still need an explicit call (because they don't have
	//	their own main thread).

	for (i = 0; i < m_Engines.GetCount(); i++)
		m_Engines[i].pEngine->SignalShutdown();

	//	Signal the engines to quit

	m_QuitEvent.Set();

	//	We wait until all engines have been shut down

	for (i = 0; i < m_Engines.GetCount(); i++)
		m_Engines[i].pEngine->WaitForShutdown();

	//	Done

	Shutdown();
	}

void CArchonProcess::Shutdown (void)

//	Shutdown
//
//	Called just before shutdown of the process

	{
	int i;

	//	Delete all engines

	for (i = 0; i < m_Engines.GetCount(); i++)
		delete m_Engines[i].pEngine;

	m_Engines.DeleteAll();

	//	If debugging memory leaks then we need to free all datums; otherwise
	//	they look like leaks.

#ifdef DEBUG_LEAKS
	CDatum::MarkAndSweep();
#endif
	}

void CArchonProcess::SendGlobalMessage (const CString &sMsg, CDatum dPayload)

//	SendGlobalMessage
//
//	Sends a message to all engines.

	{
	try
		{
		for (int i = 0; i < m_Engines.GetCount(); i++)
			{
			SArchonMessage Msg;
			Msg.sMsg = sMsg;
			Msg.sReplyAddr = ADDR_NULL;
			Msg.dwTicket = 0;
			Msg.dPayload = dPayload;

			if (m_Engines[i].pCommand)
				m_Engines[i].pCommand->SendMessage(Msg);
			}
		}
	catch (...)
		{
		Log(MSG_LOG_ERROR, ERR_CRASH_IN_SEND_GLOBAL_MESSAGE);
		}
	}

void CArchonProcess::SignalShutdown (void)

//	SignalShutdown
//
//	Initiates shutdown of the process

	{
	m_QuitSignalEvent.Set();
	}

//	IArchonProcessCtx Methods

void CArchonProcess::AddEventRequest (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket)

//	AddEventRequest
//
//	Monitors event and sends a message when signalled.

	{
	//	Add the event to the event thread

	m_EventThread.AddEvent(sName, Event, pPort, sMsg, dwTicket);

	//	If we haven't started the event thread, do it now

	m_cs.Lock();
	if (!m_EventThread.IsStarted())
		m_EventThread.Start();
	m_cs.Unlock();
	}

void CArchonProcess::Log (const CString &sMsg, const CString &sText)

//	Log
//
//	Log a status or error from the engine

	{
	CString sAddress;

	//	In console mode we just output

	if (m_bConsoleMode)
		{
		LogBlackBox(strPattern("%s: %s", sMsg, sText));
		return;
		}

	//	We don't want to recurse, so we keep track here.

	static __declspec(thread) bool bInLog = false;
	if (bInLog)
		{
#ifdef DEBUG
		printf("ERROR: Unable to log: %s\n", (LPSTR)sText);
#endif
		return;
		}

	bInLog = true;

	//	Protect ourselves in case we crash

	try
		{
		//	If we are Arcology Prime, then we send to our machine's Exarch

		if (m_bArcologyPrime)
			sAddress = ADDRESS_EXARCH_LOG;

		//	Otherwise we need to send to Arcology Prime

		else
			{
			//	NOTE: We can't easily cache this because Arcology Prime might change
			//	(if it restarts, for example).

			sAddress = m_Transporter.GenerateMachineAddress(STR_ARCOLOGY_PRIME, ADDRESS_EXARCH_LOG);
			if (sAddress.IsEmpty())
				{
				bInLog = false;
				return;
				}
			}

		//	Compose the message

		SArchonMessage Msg;
		Msg.sMsg = sMsg;
		Msg.sReplyAddr = ADDR_NULL;
		Msg.dwTicket = 0;

		//	Log the message and the machine/module that it came from

		CComplexArray *pArray = new CComplexArray;
		pArray->Insert(sText);
		pArray->Insert(GenerateAddress(PORT_ARC_LOG));
		Msg.dPayload = CDatum(pArray);

		//	NOTE: We need to use a real address (instead of a virtual port) because the log
		//	command is used before virtual ports replicate to all modules.

		SendMessage(sAddress, Msg);
		}
	catch (...)
		{
#ifdef DEBUG
		printf("ERROR: Crash in log.\n");
#endif
		}

	bInLog = false;
	}

void CArchonProcess::LogBlackBox (const CString &sText)

//	LogBlackBox
//
//	Logs a message to the persistent black box.

	{
	m_BlackBox.Log(sText);
	}

bool CArchonProcess::ReadBlackBox (const CString &sFind, int iLines, TArray<CString> *retLines)

//	ReadBlackBox
//
//	Returns the most recent lines from the log.

	{
	CString sExecutablePath = fileGetPath(fileGetExecutableFilespec());

	return m_BlackBox.ReadRecent(sExecutablePath, sFind, iLines, retLines);
	}

void CArchonProcess::ReportVolumeFailure (const CString &sFilespec, const CString &sOperation)

//	ReportVolumeFailure
//
//	And engine calls this method when it fails reading/writing to the given
//	filespec.

	{
	//	We log from here so that we transmit the module that ran into the problem.

	Log(MSG_LOG_ERROR, strPattern(ERR_DISK, sFilespec));

	//	Send a message to Exarch to check the volume.

	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(sFilespec);
	if (!sOperation.IsEmpty())
		pArray->Insert(sOperation);

	SendMessageCommand(ADDR_EXARCH_COMMAND, MSG_EXARCH_REPORT_DISK_ERROR, NULL_STR, 0, CDatum(pArray));
	}

bool CArchonProcess::SendMessage (const CString &sAddress, const SArchonMessage &Msg)

//	SendMessage
//
//	Sends a message to the given address. If we fail, we reply as appropriate.

	{
	bool bSuccess;

	CMessagePort *pPort = Bind(sAddress);
	if (pPort)
		{
		bSuccess = pPort->SendMessage(Msg);
		}
	else
		{
		Log(MSG_LOG_ERROR, strPattern(ERR_CANT_BIND, sAddress));
		bSuccess = false;
		}

	//	If we failed to send the message, then we might want to reply to the 
	//	client.

	if (!bSuccess)
		{
		//	If this is a notification port then we ignore the error (it just
		//	means that no one is listening).

		if (!strEndsWith(sAddress, STR_NOTIFY_SUFFIX) 
				&& !strStartsWith(Msg.sMsg, MSG_ERROR_PREFIX))
			{
			//	Reply to client, if necessary

			if (!CMessagePort::IsNullAddr(Msg.sReplyAddr))
				SendMessageCommand(Msg.sReplyAddr, MSG_ERROR_UNABLE_TO_COMPLY, NULL_STR, Msg.dwTicket, CDatum(strPattern(ERR_CANT_SEND_TO, sAddress)));
			else
				Log(MSG_LOG_ERROR, strPattern("Failed sending message to: %s.", sAddress));
			}
		}

	//	Done

	return bSuccess;
	}

bool CArchonProcess::SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload)

//	SendMessageCommand
//
//	Sends a command message to the given address

	{
	SArchonMessage Msg;
	Msg.sMsg = sMsg;
	Msg.sReplyAddr = (!sReplyAddr.IsEmpty() ? sReplyAddr : ADDR_NULL);
	Msg.dwTicket = dwTicket;
	Msg.dPayload = dPayload;

	return SendMessage(sAddress, Msg);
	}

void CArchonProcess::SendMessageReply (const CString &sReplyMsg, CDatum dPayload, const SArchonMessage &OriginalMsg)

//	SendMessageReply
//
//	Sends a reply message

	{
	if (CMessagePort::IsNullAddr(OriginalMsg.sReplyAddr))
		return;

	CMessagePort *pPort = Bind(OriginalMsg.sReplyAddr);
	if (pPort == NULL)
		{
		//	It is pretty rare for this to happen, so we only check if bind fails
		//	(otherwise we would be checking on every reply).

		if (strEquals(OriginalMsg.sReplyAddr, ADDR_NULL))
			return;

		Log(MSG_LOG_ERROR, strPattern(ERR_CANT_BIND, OriginalMsg.sReplyAddr));
		return;
		}

	SArchonMessage Msg;
	Msg.sMsg = sReplyMsg;
	Msg.dwTicket = OriginalMsg.dwTicket;
	Msg.dPayload = dPayload;

	if (!pPort->SendMessage(Msg))
		{
		//	LATER: If we fail sending the message it is probably because the message
		//	size exceeded some limit. In that case, just send an error back to
		//	the client so at least it knows that something went wrong.


		}
	}

bool CArchonProcess::TransformAddress (const CString &sAddress, 
									   const CString &sMsg, 
									   CDatum dPayload, 
									   CString *retsDestMsgAddress, 
									   CString *retsDestMsg, 
									   CDatum *retdPayload, 
									   CString *retsError)

//	TransformAddress
//
//	Transforms a Transpace Address.

	{
	CString sNamespace;
	if (!CTranspaceInterface::ParseAddress(sAddress, &sNamespace))
		{
		*retsError = strPattern(ERR_BAD_TRANSPACE_ADDRESS, sAddress);
		return false;
		}

	//	Is this a message namespace?

	char *pPos = sNamespace.GetParsePointer();
	if (*pPos == '@')
		{
		//	Send the message directly to the port specified by the namespace

		*retsDestMsgAddress = CString(pPos + 1);
		*retsDestMsg = sMsg;
		*retdPayload = dPayload;
		}

	//	Is this a service namespace?

	else if (*pPos == '#')
		{
		//	Parse the service endpoint

		CString sService(pPos + 1);

		//	Encode into the proper payload

		CComplexArray *pPayload = new CComplexArray;
		pPayload->Append(sService);
		pPayload->Append(sMsg);
		pPayload->Append(dPayload);

		//	Done

		*retsDestMsgAddress = ADDRESS_HYPERION_COMMAND;
		*retsDestMsg = MSG_HYPERION_SERVICE_MSG;
		*retdPayload = CDatum(pPayload);
		}

	//	If this is a slash, then convert to Aeon

	else if (*pPos == '/')
		{
		*retsDestMsgAddress = ADDRESS_AEON_COMMAND;
		*retsDestMsg = sMsg;
		*retdPayload = dPayload;
		}

	//	Otherwise, can't parse

	else
		{
		*retsError = strPattern(ERR_BAD_TRANSPACE_ADDRESS, sAddress);
		return false;
		}

	return true;
	}

void CArchonProcess::TranspaceDownload (const CString &sAddress, const CString &sReplyAddr, DWORD dwTicket, CDatum dDownloadDesc, const CHexeSecurityCtx *pSecurityCtx)

//	TranspaceDownload
//
//	Downloads a file by Transpace address.

	{
	//	Compose a proper download command payload

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Append(sAddress);
	pPayload->Append(sAddress);
	pPayload->Append(dDownloadDesc);
	CDatum dPayload(pPayload);

	//	Transform the address into a message and a new address

	CString sDestMsgAddress;
	CString sDestMsg;
	CDatum dDestPayload;
	CString sError;
	if (!TransformAddress(sAddress, MSG_TRANSPACE_DOWNLOAD, dPayload, &sDestMsgAddress, &sDestMsg, &dDestPayload, &sError))
		{
		SendMessageCommand(sReplyAddr, MSG_ERROR_UNABLE_TO_COMPLY, NULL_STR, dwTicket, CDatum(sError));
		return;
		}

	//	Send the message

	if (pSecurityCtx)
		{
		CString sWrappedMsg;
		CDatum dWrappedPayload;

		CHexeProcess::ComposeHexarcMessage(*pSecurityCtx, sDestMsg, dDestPayload, &sWrappedMsg, &dWrappedPayload);
		SendMessageCommand(sDestMsgAddress, sWrappedMsg, sReplyAddr, dwTicket, dWrappedPayload);
		}
	else
		SendMessageCommand(sDestMsgAddress, sDestMsg, sReplyAddr, dwTicket, dDestPayload);
	}
