//	MsgAddMachine.cpp
//
//	CExarchEngine class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command@~/~")
DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule")

DECLARE_CONST_STRING(AMP1_JOIN,							"JOIN")

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address")
DECLARE_CONST_STRING(FIELD_AUTH_KEY,					"authKey")
DECLARE_CONST_STRING(FIELD_AUTH_NAME,					"authName")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_KEY,							"key")
DECLARE_CONST_STRING(FIELD_MACHINES,					"machines")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_ESPER_AMP1,					"Esper.amp1")
DECLARE_CONST_STRING(MSG_OK,							"OK")

DECLARE_CONST_STRING(ERR_CANT_WRITE_CONFIG,				"Unable to write configuration file.")

const DWORD DEFAULT_AMP1_PORT =							7397;
const int DEFAULT_KEY_SIZE =							64;

class CAddMachineSession : public ISessionHandler
	{
	public:
		CAddMachineSession (CExarchEngine *pEngine, const CString &sAddress, const CIPInteger &SecretKey) :
				m_pEngine(pEngine),
				m_sAddress(sAddress),
				m_SecretKey(SecretKey),
				m_iState(stateNone)

			{ }

		//	ISessionHandler virtuals

		virtual void OnMark (void);
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum EStates
			{
			stateNone,

			stateWaitForMsg,
			};

		CExarchEngine *m_pEngine;
		CString m_sAddress;
		CIPInteger m_SecretKey;

		EStates m_iState;
	};

void CExarchEngine::MsgAddMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAddMachine
//
//	Exarch.addMachine {address}

	{
	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get some data

	CString sFullAddress = Msg.dPayload.GetElement(0);

	//	See if we already have machine at this address. If we do, then we just
	//	resend the secret key to it.

	SMachineDesc ExistingMachine;
	if (m_MecharcologyDb.FindMachineByAddress(sFullAddress, &ExistingMachine))
		{
		StartSession(Msg, new CAddMachineSession(this, sFullAddress, ExistingMachine.Key));
		return;
		}

	//	Generate a display name

	CString sDisplayName = strPattern("Machine %s", sFullAddress);

	//	Generate a secret key which we'll use to authenticate

	CIPInteger SecretKey;
	::cryptoRandom(DEFAULT_KEY_SIZE, &SecretKey);

	//	Add the machine (with its secret key) to the arcology.

	CString sError;
	if (!m_MecharcologyDb.AddMachine(sDisplayName, sFullAddress, SecretKey, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Now save the machine and secret key to our config file, so we don't have
	//	to add it again.

	CComplexStruct *pMachineEntry = new CComplexStruct;
	pMachineEntry->SetElement(FIELD_NAME, sDisplayName);
	pMachineEntry->SetElement(FIELD_ADDRESS, sFullAddress);
	pMachineEntry->SetElement(FIELD_KEY, CDatum(SecretKey));

	CDatum dMachineList = m_dMachineConfig.GetElement(FIELD_MACHINES);
	if (dMachineList.IsNil())
		{
		CComplexArray *pMachineList = new CComplexArray;
		dMachineList = CDatum(pMachineList);
		m_dMachineConfig.SetElement(FIELD_MACHINES, dMachineList);
		}

	dMachineList.Append(CDatum(pMachineEntry));

	if (!WriteConfig())
		{
		m_MecharcologyDb.DeleteMachineByKey(SecretKey);
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_CANT_WRITE_CONFIG, Msg);
		return;
		}

	//	Start a sessions that will handle this

	StartSession(Msg, new CAddMachineSession(this, sFullAddress, SecretKey));
	}

//	CAddMachineSession ---------------------------------------------------------

void CAddMachineSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	}

bool CAddMachineSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a response

	{
	switch (m_iState)
		{
		case stateWaitForMsg:
			{
			if (IsError(Msg))
				{
				SendMessageReplyError(Msg.sMsg, Msg.dPayload.GetElement(0));
				return false;
				}

			SendMessageReply(MSG_OK);
			return false;
			}

		default:
			return false;
		}
	}

bool CAddMachineSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session

	{
	//	Payload

	CComplexStruct *pData = new CComplexStruct;
	pData->SetElement(FIELD_AUTH_NAME, m_pEngine->GetProcessCtx()->GetMachineName());
	pData->SetElement(FIELD_AUTH_KEY, m_SecretKey);

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Append(m_sAddress);
	pPayload->Append(AMP1_JOIN);
	pPayload->Append(CDatum(pData));

	//	Connect to the machine with a command.

	m_iState = stateWaitForMsg;
	SendMessageCommand(ADDRESS_ESPER_COMMAND, MSG_ESPER_AMP1, ADDRESS_EXARCH_COMMAND, CDatum(pPayload));

	//	Expect reply

	return true;
	}
