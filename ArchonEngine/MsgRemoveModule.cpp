//	MsgRemoveModule.cpp
//
//	CExarchEngine class
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule");

DECLARE_CONST_STRING(FIELD_MODULES,						"modules");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_EXARCH_ADD_MODULE,				"Exarch.addModule");
DECLARE_CONST_STRING(MSG_OK,							"OK");

void CExarchEngine::MsgRemoveModule (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRemoveModule
//
//	Exarch.removeModule {module}

	{
	int i;
	CString sError;

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get parameters

	CString sMachineName;
	CString sModule;
	if (!ParseModuleName(Msg.dPayload.GetElement(0).AsStringView(), &sMachineName, &sModule))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unknown module: %s.", Msg.dPayload.GetElement(0).AsString()), Msg);
		return;
		}

	//	If this is not for our machine, then we need to send a message to the other machine.

	if (!sMachineName.IsEmpty())
		{
		CString sAddress = GenerateMachineAddress(sMachineName, ADDRESS_EXARCH_COMMAND);
		if (sAddress.IsEmpty())
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unable to generate address for: %s", sMachineName), Msg);
			return;
			}

		StartSession(Msg, new CMessageDispatchSession(sAddress, Msg.sMsg, Msg.sReplyAddr, Msg.dPayload));
		}

	//	Otherwise, we remove a local module.

	else
		{
		CSmartLock Lock(m_cs);

		//	Remove the module

		if (!RemoveModule(sModule, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			return;
			}

		//	Remove it from our list of modules

		CComplexArray *pModuleList = new CComplexArray(m_dMachineConfig.GetElement(FIELD_MODULES));
		for (i = 0; i < pModuleList->GetCount(); i++)
			{
			if (strEquals(sModule, pModuleList->GetElement(i).AsString()))
				{
				pModuleList->Delete(i);
				break;
				}
			}

		CComplexStruct *pConfig = new CComplexStruct(m_dMachineConfig);
		pConfig->SetElement(FIELD_MODULES, CDatum(pModuleList));
		m_dMachineConfig = CDatum(pConfig);

		//	Save it

		WriteConfig();

		//	Done

		SendMessageReply(MSG_OK, CDatum(), Msg);
		}
	}

