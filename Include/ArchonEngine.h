//	ArchonEngine.h
//
//	Functions and classes for Archons
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	ArchonEngine provides classes and functions for implementing an archon.
//
//	1. Requires Foundation
//	2. Include ArchonEngine.h
//	3. Link with ArchonEngine.lib
//	4. Link with ws2_32.lib

#pragma once

#include "AEON.h"
#include "ArchonClient.h"
#include "Hexe.h"

#ifdef DEBUG
//#define DEBUG_GARBAGE_COLLECTION
//#define DEBUG_MNEMOSYNTH
//#define DEBUG_MODULE_RESTART
//#define DEBUG_STARTUP
#endif

class CMessagePort;
class CMessageTransporter;
class CMnemosynthDb;

//	Some basic structures

struct SArchonMessage
	{
	CString sMsg;								//	Message name
	CString sReplyAddr;							//	Address to send errors/replies to (NULL_STR if no reply required)
	DWORD dwTicket = 0;							//	DWORD used by sender to track replies
	CDatum dPayload;							//	Payload (depending on message)

	DWORD dwEnqueueTime = 0;					//	Tick when message was added to queue.
	};

typedef TArray<SArchonMessage> CArchonMessageList;

struct SArchonEnvelope
	{
	CString sAddr;								//	Destination address of message
	SArchonMessage Msg;							//	Message
	};

typedef TArray<SArchonEnvelope> CArchonEnvelopeList;

struct SWatermark
	{
	DWORD dwEndpointID;
	DWORD dwSequence;							//	0 = no watermark
	};

extern SWatermark NULL_WATERMARK;

//	Basic Interfaces

class IArchonMessagePort
	{
	public:
		virtual ~IArchonMessagePort (void) { }

		virtual CDatum GetPortStatus (void) const { return CDatum(); }
		virtual bool IsValid (void) const { return true; }
		virtual bool IsValidForMnemosynthSequence (SequenceNumber Seq) const { return true; }
		virtual bool SendMessage (const SArchonMessage &Msg) { return false; }
	};

class IArchonExarch
	{
	public:
		virtual bool ExarchAuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) = 0;
	};

//	Engine implementations

class IArchonProcessCtx
	{
	public:
		virtual void AddEventRequest (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket) = 0;
			//	Monitors the given event and fires a message when the event is signalled.

		virtual void AddPort (const CString &sPort, IArchonMessagePort *pPort) = 0;
			//	Engines should call this to add a port that will receive messages.

		virtual void AddVirtualPort (const CString &sPort, const CString &sAddress, DWORD dwFlags) = 0;
			//	This adds a virtual port that can be called from anywhere in the arcology

		virtual bool AuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) = 0;
			//	Returns TRUE if this is the proper key for the given machine.

		virtual CMessagePort *Bind (const CString &sAddr) = 0;
			//	This method takes a physical address and returns a port. Callers do
			//	NOT need to free the returned pointer.

		virtual CString GenerateAbsoluteAddress (const CString &sAddress) = 0;
			//	Generates an absolute address.

		virtual CString GenerateAddress (const CString &sPort) = 0;
			//	Generates a port address by adding the appropriate process name.

		virtual CString GenerateMachineAddress (const CString &sMachineName, const CString &sPort) = 0;
			//	Generates a foreign machine address. If the machine does not exist, returns NULL_STR.

		virtual const CString &GetMachineName (void) const = 0;
			//	Returns the machine name

		virtual CMnemosynthDb &GetMnemosynth (void) = 0;
			//	Returns Mnemosynth

		virtual const CString &GetModuleName (void) const = 0;
			//	Returns the module name

		virtual const SFileVersionInfo& GetModuleVersion (void) const = 0;
			//	Returns the module version info

		virtual CMessageTransporter &GetTransporter (void) = 0;
			//	Returns the Transporter

		virtual void InitiateShutdown (void) = 0;
			//	Asks the process to shutdown

		virtual bool IsCentralModule (void) = 0;
			//	Returns TRUE if we are central module

		virtual bool IsOnArcologyPrime () const = 0;
			//	Returns TRUE if this module is on Arcology Prime.
			//	See: PROCESS_FLAG_ON_ARCOLOGY_PRIME

		virtual void Log (const CString &sMsg, const CString &sText) = 0;
			//	Log a message

		virtual void LogBlackBox (const CString &sText) = 0;
			//	Log a message to the black box

		virtual CDatum MnemosynthRead (const CString &sCollection, const CString &sKey, SWatermark *retWatermark = NULL) const = 0;
			//	Reads the entry of the given key in the given collection and returns
			//	its contents.

		virtual void MnemosynthReadCollection (const CString &sCollection, TArray<CString> *retKeys, SWatermark *retWatermark = NULL) const = 0;
			//	Returns the keys in the given collection.

		virtual bool MnemosynthWrite (const CString &sCollection, const CString &sKey, CDatum dValue, const SWatermark &Watermark = NULL_WATERMARK) = 0;
			//	Writes the entry to the given key and collection. If the entry already exists, then
			//	we only write if the watermark is valid (otherwise we assume that our value is stale)

		virtual void OnMnemosynthDbModified (CDatum dLocalUpdates) = 0;
			//	Notifies the Archon Process that Mnemosynth has changed.

		virtual bool ReadBlackBox (const CString &sFind, int iLines, TArray<CString> *retLines) = 0;
			//	Read the most recent lines from the log

		virtual void ReportVolumeFailure (const CString &sFilespec, const CString &sOperation = NULL_STR) = 0;
			//	Reports an error while reading or writing to filespec.

		virtual bool SendMessage (const CString &sAddress, const SArchonMessage &Msg) = 0;
			//	Utility function to send a command message

		virtual bool SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload) = 0;
			//	Utility function to send a command message

		virtual void SendMessageReply (const CString &sReplyMsg, CDatum dPayload, const SArchonMessage &OriginalMsg) = 0;
			//	This is a utility function that helps to create a message.

		virtual void TranspaceDownload (const CString &sAddress, const CString &sReplyAddr, DWORD dwTicket, CDatum dDownloadDesc, const CHexeSecurityCtx *pSecurityCtx) = 0;
			//	Translates and sends a Transpace.download message.
	};

enum EProcessingState
	{
	processingUnknown =					0,
	processingWaiting =					1,
	processingMessages =				2,
	processingStopped =					3,
	processingExit =					4,
	processingRefresh =					5,
	processingEvent =					6,
	};

class IArchonEngine
	{
	public:
		virtual ~IArchonEngine (void) { }

		virtual void AccumulateModuleData (CDatum dData) const
			{
			//	The engine should set any fields on dData that should be 
			//	published in the Mnemosynth entry for this module.
			}

		virtual void AccumulateCrashData (TArray<CString>& retLines) const { }

		virtual void Boot (IArchonProcessCtx *pProcess, DWORD dwID)
			{
			//	The engine should initialize its state. After Boot returns,
			//	the engine should expect calls on any IArchonMessagePorts that
			//	it has added.
			//
			//	Engines should not bind or send messages to other engines at
			//	this point.
			}

		virtual CString ConsoleCommand (const CString &sCmd, const TArray<CDatum> &Args)
			{
			//	This is used to implement a console/testing interface for the
			//	engine. It is only used in console mode.

			return NULL_STR;
			}

		virtual const CString &GetName (void) = 0;
			//	This returns the engine class's short name (generally the same
			//	as the prefix used for its messages).

		virtual bool IsIdle (void)
			{
			//	The engine should return TRUE if all of its threads are currently
			//	idle. This is used heuritically to determine whether or not a
			//	SignalStop will succeed quickly. But there is no guarantee...

			return true;
			}

		virtual void Mark (void)
			{
			//	Mark is guaranteed to be called only while all engines are
			//	stopped and only from a single thread. Engines should mark all
			//	their AEON objects.
			}

		virtual void SignalShutdown (void)
			{
			//	The process wants the engine to shutdown. This is called in addition
			//	to setting the QuitEvent. Engines are free to ignore this call if they
			//	respond to the QuitEvent.
			}

		virtual void SignalPause (void)
			{
			//	The process wants the engine to pause its threads. This is called in addition
			//	to setting the PauseEvent. Engines are free to ignore this call if they
			//	respond to the PauseEvent.
			}

		virtual void SignalResume (void)
			{
			//	The process wants the engine to continue after a pause.
			}

		virtual void StartRunning (CManualEvent &RunEvent, CManualEvent &PauseEvent, CManualEvent &QuitEvent)
			{
			//	This signals to the engine that all engines in the process
			//	have booted. At this point the engine can start processing
			//	messages and can bind and/or send messages to any engine.
			//
			//	At this point we have not yet joined an arcology, so we should not
			//	service any user messages (or do any processing that requires an
			//	arcology).
			//
			//	When the PauseEvent is signalled the engine should stop all processing and
			//	wait for the RunEvent to be signalled (or the QuitEvent).
			//
			//	When the QuitEvent is signalled the engine should stop all threads.
			}

		virtual void WaitForShutdown (void)
			{
			//	This call blocks until the engine has stopped running.
			}

		virtual void WaitForPause (void)
			{
			//	This call blocks until the engine has paused running. The engine
			//	should start running when the RunEvent is set. (Or if the QuitEvent
			//	is set).
			}
	};

//	This process-singleton class implements everything for a process to
//	be a part of an arcology.

struct SEngineDesc
	{
	IArchonEngine *pEngine;						//	Will be owned by CArchonProcess
	};

struct SProcessDesc
	{
	SProcessDesc (void) :
			dwFlags(0),
			pExarch(NULL)
		{ }

	CString sMachineName;						//	Name of machine
	TArray<SEngineDesc> Engines;				//	Set of engines
	DWORD dwFlags;								//	Flags

	IArchonExarch *pExarch;
	};

const DWORD PROCESS_FLAG_CENTRAL_MODULE =	0x00000001;
const DWORD PROCESS_FLAG_DEBUG =			0x00000002;
const DWORD PROCESS_FLAG_ARCOLOGY_PRIME =	0x00000004;
const DWORD PROCESS_FLAG_CONSOLE_OUTPUT =	0x00000008;
const DWORD PROCESS_FLAG_CONSOLE_MODE =		0x00000010;
const DWORD PROCESS_FLAG_ON_ARCOLOGY_PRIME =0x00000020;

//	Archon Implementations

#include "ArchonUtilities.h"
#include "ArchonEngineImpl.h"
#include "ArchonFilterImpl.h"
#include "ArchonSessionImpl.h"

#include "Mnemosynth.h"

//	CArchonProcess

class CArchonProcess : public IArchonProcessCtx
	{
	public:
		CArchonProcess (void);
		~CArchonProcess (void);

		bool Boot (const SProcessDesc &Config);
		CString ConsoleCommand (const CString &sCmd, const TArray<CDatum> &Args);
		void Run (void);
		void SendGlobalMessage (const CString &sMsg, CDatum dPayload = CDatum());
		void SignalShutdown (void);
		void UnhandledException (CStringView sError);

		//	IArchonProcessCtx
		virtual void AddEventRequest (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket) override;
		virtual void AddPort (const CString &sPort, IArchonMessagePort *pPort) override { m_Transporter.AddLocalPort(sPort, pPort); }
		virtual void AddVirtualPort (const CString &sPort, const CString &sAddress, DWORD dwFlags) override { m_Transporter.AddVirtualPort(sPort, sAddress, dwFlags); }
		virtual bool AuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) override { return (m_pExarch ? m_pExarch->ExarchAuthenticateMachine(sMachineName, Key) : false); }
		virtual CMessagePort *Bind (const CString &sAddr) override { return m_Transporter.Bind(sAddr); }
		virtual CString GenerateAbsoluteAddress (const CString &sAddress) override { return m_Transporter.GenerateAbsoluteAddress(sAddress); }
		virtual CString GenerateAddress (const CString &sPort) override { return m_Transporter.GenerateAddress(sPort, m_sName, m_sMachineName); }
		virtual CString GenerateMachineAddress (const CString &sMachineName, const CString &sAddress) override { return m_Transporter.GenerateMachineAddress(sMachineName, sAddress); }
		virtual const CString &GetMachineName (void) const override { return m_sMachineName; }
		virtual CMnemosynthDb &GetMnemosynth (void) override { return m_MnemosynthDb; }
		virtual const CString &GetModuleName (void) const override { return m_sName; }
		virtual const SFileVersionInfo& GetModuleVersion (void) const override { return m_Version; }
		virtual CMessageTransporter &GetTransporter (void) override { return m_Transporter; }
		virtual void InitiateShutdown (void) override { SignalShutdown(); }
		virtual bool IsCentralModule (void) override { return m_bCentralModule; }
		virtual bool IsOnArcologyPrime () const override { return m_bOnArcologyPrime; }
		virtual void Log (const CString &sMsg, const CString &sText) override;
		virtual void LogBlackBox (const CString &sText) override;
		virtual CDatum MnemosynthRead (const CString &sCollection, const CString &sKey, SWatermark *retWatermark = NULL) const override { return m_MnemosynthDb.Read(sCollection, sKey); }
		virtual void MnemosynthReadCollection (const CString &sCollection, TArray<CString> *retKeys, SWatermark *retWatermark = NULL) const override { return m_MnemosynthDb.ReadCollection(sCollection, retKeys); }
		virtual bool MnemosynthWrite (const CString &sCollection, const CString &sKey, CDatum dValue, const SWatermark &Watermark = NULL_WATERMARK) override { m_MnemosynthDb.Write(sCollection, sKey, dValue); return true; }
		virtual void OnMnemosynthDbModified (CDatum dLocalUpdates) override;
		virtual bool ReadBlackBox (const CString &sFind, int iLines, TArray<CString> *retLines) override;
		virtual void ReportVolumeFailure (const CString &sFilespec, const CString &sOperation = NULL_STR) override;
		virtual bool SendMessage (const CString &sAddress, const SArchonMessage &Msg) override;
		virtual bool SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload) override;
		virtual void SendMessageReply (const CString &sReplyMsg, CDatum dPayload, const SArchonMessage &OriginalMsg) override;
		virtual void TranspaceDownload (const CString &sAddress, const CString &sReplyAddr, DWORD dwTicket, CDatum dDownloadDesc, const CHexeSecurityCtx *pSecurityCtx) override;

	private:

		static constexpr DWORD GC_IDLE_SPIN_TIME = 3000;
		static constexpr DWORD GC_IDLE_WAIT = 100;

		enum States
			{
			stateNone,
			stateWaitingForCentralModule,		//	We're waiting for CentralModule to boot
			stateRunning,						//	Process is running normally

			stateBootError,						//	Boot failed and we cannot run
			};

		struct SEngine
			{
			IArchonEngine *pEngine;
			CMessagePort *pCommand;				//	Default command port for the engine
			};

		void CollectGarbage (void);
		void CriticalError (const CString &sError);
		bool IsIdle () const;
		bool OnModuleStart (CDatum dModuleData);
		void Shutdown (void);
		bool TransformAddress (const CString &sAddress, const CString &sMsg, CDatum dPayload, CString *retsDestMsgAddress, CString *retsDestMsg, CDatum *retdPayload, CString *retsError);

		States m_iState = stateNone;			//	Current process state
		CString m_sArcology;					//	Name of our arcology (or NULL_STR if we don't know)
		CString m_sMachineName;					//	Machine name
		CString m_sName;						//	Name of this process (unique within machine)
		SFileVersionInfo m_Version;				//	Process version info

		CCriticalSection m_cs;					//	Controls access to our member variables
		CManualEvent m_QuitSignalEvent;			//	When set, our parent wants us to quit

		bool m_bArcologyPrime = false;			//	TRUE if we are central module on Arcology Prime
		bool m_bOnArcologyPrime = false;		//	TRUE if we are modile on Arcology Prime
		bool m_bCentralModule = false;			//	TRUE if we are the machine's central module
		bool m_bConsoleMode = false;			//	TRUE if we are running in console mode (isolated)
		bool m_bDebugger = false;				//	TRUE if we should launch debugger on load
		CSemaphore m_CentralModuleSem;			//	Locked if we are the central module
		DWORD m_dwLastGarbageCollect = 0;		//	Tick when we last collected garbage
		DWORD m_dwCriticalErrors = 0;			//	Number of crashes inside Run loop

		IArchonExarch *m_pExarch = NULL;		//	Wormhole to CExarchEngine
		CBlackBox m_BlackBox;					//	Diagnostic log
		CMnemosynthDb m_MnemosynthDb;			//	Mnemosynth database
		CMessageTransporter m_Transporter;		//	Helps to transport messages
		CEventProcessingThread m_EventThread;	//	Monitors events and fires messages when signalled
		CInterprocessMessageThread m_ImportThread;	//	Thread to import message from other processes

		TArray<SEngine> m_Engines;				//	Set of engines
		CManualEvent m_RunEvent;				//	A stopped engine should start running again when signalled
		CManualEvent m_PauseEvent;				//	When signalled, all engines should stop
		CManualEvent m_QuitEvent;				//	When signalled, all engines should quit
	};
