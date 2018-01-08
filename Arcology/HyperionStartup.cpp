//	HyperionStartup.cpp
//
//	HypersionStartup
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command")
DECLARE_CONST_STRING(ADDR_AEON,							"Aeon.command")

DECLARE_CONST_STRING(MSG_AEON_FILE_DIRECTORY,			"Aeon.fileDirectory")

DECLARE_CONST_STRING(STR_ARC_SERVICES,					"/Arc.services/")

class CLoadServicesSession : public ISessionHandler
	{
	public:
		CLoadServicesSession (CHyperionEngine *pEngine) : 
				m_pEngine(pEngine),
				m_iPos(-1) 
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		CHyperionEngine *m_pEngine;

		TArray<CString> m_ServiceFiles;
		int m_iPos;
	};

void CHyperionEngine::MsgAeonOnStart (const SArchonMessage &Msg)

//	MsgAeonOnStart
//
//	Aeon.onStart

	{
	//	Start a session to load all services

	StartSession(Msg, new CLoadServicesSession(this));
	}

//	CLoadServicesSession -------------------------------------------------------

bool CLoadServicesSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a reply

	{
	int i;

	//	If m_iPos is -1 then this is the reply that includes all service files

	if (m_iPos)
		{
		for (i = 0; i < Msg.dPayload.GetCount(); i++)
			m_ServiceFiles.Insert(Msg.dPayload.GetElement(i));

		m_iPos = 0;
		}

	//	Otherwise, this is a reply for the contents of a specific file

	else
		{
		m_iPos++;
		}

	//	Request the next file

	if (m_iPos < m_ServiceFiles.GetCount())
		{
		CComplexArray *pArray = new CComplexArray;
		pArray->Insert(m_ServiceFiles[m_iPos]);

		ISessionHandler::SendMessageCommand(ADDR_AEON,
				MSG_AEON_FILE_DOWNLOAD,
				GenerateAddress(PORT_HYPERION_COMMAND),
				CDatum(pArray));

		return true;
		}
	else
		return false;
	}

bool CLoadServicesSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session

	{
	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(STR_ARC_SERVICES);

	//	Ask Aeon for the list of all files under /Arc.services/

	ISessionHandler::SendMessageCommand(ADDR_AEON,
			MSG_AEON_FILE_DIRECTORY,
			GenerateAddress(PORT_HYPERION_COMMAND),
			CDatum(pArray));

	return true;
	}
