//	MsgAddModule.cpp
//
//	CExarchEngine class
//	Copyright (c) 2015 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule")

DECLARE_CONST_STRING(FIELD_DEBUG,						"debug")
DECLARE_CONST_STRING(FIELD_MODULES,						"modules")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_EXARCH_ADD_MODULE,				"Exarch.addModule")
DECLARE_CONST_STRING(MSG_OK,							"OK")

DECLARE_CONST_STRING(STR_EXE_SUFFIX,					".exe")

class CAddModuleSession : public ISessionHandler
	{
	public:
		CAddModuleSession (CExarchEngine *pEngine, const CString &sAddress, const CString &sModule) :
				m_pEngine(pEngine),
				m_sAddress(sAddress),
				m_sModule(sModule),
				m_iState(stateNone)

			{ }

		//	ISessionHandler virtuals

		virtual void OnMark (void) { }
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
		CString m_sModule;

		EStates m_iState;
	};

void CExarchEngine::MsgAddModule (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAddModule
//
//	Exarch.addModule [{machineName}] {filePath} [{debug}]

	{
	CSmartLock Lock(m_cs);
	CString sError;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get parameters

	bool bHasMachineName = (Msg.dPayload.GetCount() >= 2 && !strEndsWith(strToLower(Msg.dPayload.GetElement(0).AsStringView()), STR_EXE_SUFFIX));
	int iArg = 0;
	CString sMachineName = (bHasMachineName ? Msg.dPayload.GetElement(iArg++).AsString() : NULL_STR);
	CStringView sModuleFilePath = Msg.dPayload.GetElement(iArg++);
	CDatum dDebug = Msg.dPayload.GetElement(iArg++);

	//	If we have a machine name, try to parse it in case the user gave us
	//	a partial name.

	if (bHasMachineName)
		{
		if (!ParseMachineName(sMachineName, &sMachineName))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unknown machine: %s", sMachineName), Msg);
			return;
			}
		}

	//	If this is not for our machine, then we need to send a message to the
	//	other machine.

	if (!sMachineName.IsEmpty()
			&& !strEqualsNoCase(GetMachineName(), sMachineName))
		{
		CString sAddress = GenerateMachineAddress(sMachineName, ADDRESS_EXARCH_COMMAND);
		if (sAddress.IsEmpty())
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unable to generate address for: %s", sMachineName), Msg);
			return;
			}

		StartSession(Msg, new CAddModuleSession(this, sAddress, sModuleFilePath));
		}

	//	Otherwise, we add a local module

	else
		{
		//	Add the module

		CString sModuleName;
		if (!AddModule(sModuleFilePath, strEqualsNoCase(dDebug.AsStringView(), FIELD_DEBUG), &sModuleName, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			return;
			}

		//	Add it to our list of modules

		CComplexArray *pModuleList = new CComplexArray(m_dMachineConfig.GetElement(FIELD_MODULES));
		pModuleList->Append(CDatum(sModuleName));

		CComplexStruct *pConfig = new CComplexStruct(m_dMachineConfig);
		pConfig->SetElement(FIELD_MODULES, CDatum(pModuleList));
		m_dMachineConfig = CDatum(pConfig);

		//	Save it

		WriteConfig();

		//	Done

		SendMessageReply(MSG_OK, CDatum(), Msg);
		}
	}

//	CAddModuleSession ----------------------------------------------------------

bool CAddModuleSession::OnProcessMessage (const SArchonMessage &Msg)

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
				SendMessageReplyError(Msg.sMsg, Msg.dPayload.GetElement(0).AsStringView());
				return false;
				}

			SendMessageReply(MSG_OK);
			return false;
			}

		default:
			return false;
		}
	}

bool CAddModuleSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session

	{
	//	Payload

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Append(m_sModule);

	//	Connect to the machine with a command.

	m_iState = stateWaitForMsg;
	SendMessageCommand(m_sAddress, MSG_EXARCH_ADD_MODULE, ADDRESS_EXARCH_COMMAND, CDatum(pPayload));

	//	Expect reply

	return true;
	}
