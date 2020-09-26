//	MsgRemoveMachine.cpp
//
//	CExarchEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command@~/~");
DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule");

DECLARE_CONST_STRING(AMP1_LEAVE,						"LEAVE");

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address");
DECLARE_CONST_STRING(FIELD_MACHINES,					"machines");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_ESPER_AMP1,					"Esper.amp1");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_OK,							"OK");

DECLARE_CONST_STRING(ERR_NOT_ARCOLOGY_PRIME,			"Unable to comply because we are not Arcology Prime.");
DECLARE_CONST_STRING(ERR_CANT_REMOVE_ARCOLOGY_PRIME,	"Unable to remove Arcology Prime.");
DECLARE_CONST_STRING(ERR_CANT_WRITE_CONFIG,				"Unable to write configuration file.");

class CRemoveMachineSession : public ISessionHandler
	{
	public:
		CRemoveMachineSession (CExarchEngine &Engine, const SMachineDesc &MachineToRemove) :
				m_Engine(Engine),
				m_MachineToRemove(MachineToRemove)
			{ }

		//	ISessionHandler virtuals

		virtual void OnMark (void) override { }
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		enum class State
			{
			none,

			waitForMsg,
			};

		CExarchEngine &m_Engine;
		SMachineDesc m_MachineToRemove;

		State m_iState = State::none;
	};

void CExarchEngine::MsgRemoveMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRemoveMachine
//
//	Removes the given machine from the arcology.

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Must be Arcology Prime

	if (!IsArcologyPrime())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NOT_ARCOLOGY_PRIME, Msg);
		return;
		}

	//	Find the machine

	const CString &sInputName = Msg.dPayload.GetElement(0);
	SMachineDesc MachineDesc;
	if (!m_MecharcologyDb.FindMachineByPartialName(sInputName, &MachineDesc))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unknown machine: %s", sInputName), Msg);
		return;
		}

	//	Can't remove Arcology Prime

	if (m_MecharcologyDb.IsArcologyPrime(MachineDesc.sName))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_CANT_REMOVE_ARCOLOGY_PRIME, Msg);
		return;
		}

	//	If we don't have an address, then we just remove the machine and move on.

	if (MachineDesc.sAddress.IsEmpty())
		{
		RemoveMachine(MachineDesc);
		SendMessageReply(MSG_OK, CDatum(), Msg);
		return;
		}

	//	Start a sessions that will handle this

	StartSession(Msg, new CRemoveMachineSession(*this, MachineDesc));
	}

//	CRemoveMachineSession ------------------------------------------------------

bool CRemoveMachineSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process reply.

	{
	//	No matter what, we delete our data structures. We do this in case we 
	//	have a problem communicating with the other machine.

	m_Engine.RemoveMachine(m_MachineToRemove);

	//	Compose a reply

	switch (m_iState)
		{
		case State::waitForMsg:
			{
			//	If we get an error, log it, but continue.

			if (IsError(Msg))
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern("Error when asking %s to leave: %s", m_MachineToRemove.sName, Msg.dPayload.GetElement(0)));

			SendMessageReply(MSG_OK);
			return false;
			}

		default:
			return false;
		}
	}

bool CRemoveMachineSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start

	{
	m_iState = State::waitForMsg;

	//	We send a message to the machine to leave the arcology.

	CDatum dLeavePayload(CDatum::typeArray);
	dLeavePayload.Append(m_MachineToRemove.sAddress);
	dLeavePayload.Append(AMP1_LEAVE);

	SendMessageCommand(ADDRESS_ESPER_COMMAND, MSG_ESPER_AMP1, ADDRESS_EXARCH_COMMAND, dLeavePayload);

	//	Expect reply

	return true;
	}
