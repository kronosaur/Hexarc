//	MsgRunTask.cpp
//
//	Hyperion.runTask
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(LIBRARY_CORE,						"core")
DECLARE_CONST_STRING(LIBRARY_HYPERION,					"hyperion")

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command")

DECLARE_CONST_STRING(ERR_RUN_ERROR,						"Error executing task %s: %s")
DECLARE_CONST_STRING(ERR_COMPILER,						"HexeLisp compiler error: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_CODE,			"Unable to parse input.")
DECLARE_CONST_STRING(ERR_CANT_RUN,						"Unable to run task: %s.")

const DWORD MESSAGE_TIMEOUT =							30 * 1000;

class CRunTaskSession : public ISessionHandler
	{
	public:
		CRunTaskSession (CHyperionEngine *pEngine, CHyperionScheduler &Scheduler, const CString &sTask) : 
				m_pEngine(pEngine),
				m_Scheduler(Scheduler),
				m_sTask(sTask),
				m_iState(stateNone)
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual void OnMark (void);
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum States
			{
			stateNone,
			stateWaitingForHexarcReply,
			};

		bool HandleResult (CHexeProcess::ERun iRun, CDatum dResult);

		CHyperionEngine *m_pEngine;
		CHyperionScheduler &m_Scheduler;
		CString m_sTask;
		States m_iState;
		CHexeProcess m_Process;
	};

void CHyperionEngine::MsgRunTask (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRunTask
//
//	Hyperion.runTask {taskName}

	{
	StartSession(Msg, new CRunTaskSession(this, m_Scheduler, Msg.dPayload.AsStringView()));
	}

//	CRunTaskSession ------------------------------------------------------------

bool CRunTaskSession::HandleResult (CHexeProcess::ERun iRun, CDatum dResult)

//	HandleResult
//
//	Handles the result from a run

	{
	switch (iRun)
		{
		case CHexeProcess::ERun::OK:
		case CHexeProcess::ERun::Error:
		case CHexeProcess::ERun::ForcedTerminate:
			{
			//	If we got an error, then log it.

			if (iRun == CHexeProcess::ERun::Error || iRun == CHexeProcess::ERun::ForcedTerminate)
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_RUN_ERROR, m_sTask, dResult.AsString()));

			//	Either way we're done running

			m_Scheduler.SetRunComplete(m_sTask);

			//	FALSE means we're done with the session

			return false;
			}

		case CHexeProcess::ERun::AsyncRequest:
			{
			//	Check to see if we've been signalled to quit. If so, we stop.

			CString sMessage;
			if (m_Scheduler.IsSignalStop(m_sTask, &sMessage))
				{
				GetProcessCtx()->Log(MSG_LOG_INFO, sMessage);
				m_Scheduler.SetRunComplete(m_sTask);
				return false;
				}

			//	Async request

			SendMessageCommand(dResult.GetElement(0).AsStringView(), 
					dResult.GetElement(1).AsStringView(), 
					GenerateAddress(PORT_HYPERION_COMMAND),
					dResult.GetElement(2),
					MESSAGE_TIMEOUT);

			m_iState = stateWaitingForHexarcReply;

			return true;
			}

		default:
			//	LATER: This should never happen
			ASSERT(false);
			return false;
		}
	}

void CRunTaskSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	m_Process.Mark();
	}

bool CRunTaskSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a message

	{
	//	If we a reply, then process it

	if (m_iState == stateWaitingForHexarcReply)
		{
		//	continue with execution

		CDatum dResult;
		CHexeProcess::ERun iRun = m_Process.RunContinues(CSimpleEngine::MessageToHexeResult(Msg), &dResult);

		//	Handle it

		return HandleResult(iRun, dResult);
		}

	//	We should never get here.

	ASSERT(false);
	return false;
	}

bool CRunTaskSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session

	{
	//	Start running. If we fail, log an error

	CDatum dFunction;
	CHyperionScheduler::EResults iResult;
	if ((iResult = m_Scheduler.SetRunning(m_sTask, &dFunction)) != CHyperionScheduler::resultOK)
		{
		if (iResult == CHyperionScheduler::resultError)
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_CANT_RUN, m_sTask));

		//	If we get back resultRunning, then we exit without error.

		return false;
		}

	//	Set some context

	m_Process.SetLibraryCtx(LIBRARY_HYPERION, m_pEngine);

	//	Run the function

	CDatum dResult;
	CHexeProcess::ERun iRun = m_Process.Run(dFunction, TArray<CDatum>(), &dResult);

	//	Deal with the result

	return HandleResult(iRun, dResult);
	}
