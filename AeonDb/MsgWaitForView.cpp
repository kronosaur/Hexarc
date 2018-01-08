//	MsgWaitForView.cpp
//
//	CAeonEngine class
//	Copyright (c) 2012 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command@~/~")

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_OK,							"OK")

class CWaitForViewSession : public ISessionHandler
	{
	public:
		CWaitForViewSession (CAeonEngine &Engine, const CString &sTable, DWORD dwViewID) : m_Engine(Engine), m_sTable(sTable), m_dwViewID(dwViewID) { }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);
		virtual bool OnTimeout (const SArchonMessage &Msg);

	private:
		CAeonEngine &m_Engine;

		CString m_sTable;
		DWORD m_dwViewID;
	};

void CAeonEngine::MsgWaitForView (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgWaitForView
//
//	Aeon.waitForVolume {volume}

	{
	//	Make sure we can do this

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Parameters

	CAeonTable *pTable;
	DWORD dwViewID;
	if (!ParseTableAndView(Msg, pSecurityCtx, Msg.dPayload.GetElement(0), &pTable, &dwViewID))
		return;

	//	See if the view is up to date

	bool bUpToDate;
	CString sError;
	if (!pTable->GetViewStatus(dwViewID, &bUpToDate, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	if (bUpToDate)
		{
		SendMessageReply(MSG_OK, CDatum(), Msg);
		return;
		}

	//	If not, we start a session and wait for it to become up to date.

	StartSession(Msg, new CWaitForViewSession(*this, pTable->GetName(), dwViewID));
	}

//	CWaitForViewSession --------------------------------------------------------

bool CWaitForViewSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession

	{
	//	Wait 100 ms

	ResetTimeout(ADDRESS_AEON_COMMAND, 100);
	return true;
	}

bool CWaitForViewSession::OnTimeout (const SArchonMessage &Msg)

//	OnTimeout

	{
	//	Check to see if we have the view yet

	bool bUpToDate;
	CString sError;
	if (!m_Engine.GetViewStatus(m_sTable, m_dwViewID, &bUpToDate, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
		return false;
		}

	if (bUpToDate)
		{
		SendMessageReply(MSG_OK, CDatum());
		return false;
		}

	//	Otherwise, keep waiting

	ResetTimeout(ADDRESS_AEON_COMMAND, 100);
	return true;
	}
