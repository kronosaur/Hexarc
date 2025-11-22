//	MsgWaitForVolume.cpp
//
//	CAeonEngine class
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command@~/~")

DECLARE_CONST_STRING(MSG_OK,							"OK")

class CWaitForVolumeSession : public ISessionHandler
	{
	public:
		CWaitForVolumeSession (const CString &sVolume, CMachineStorage &Storage) : m_sVolume(sVolume), m_Storage(Storage) { }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);
		virtual bool OnTimeout (const SArchonMessage &Msg);

	private:
		CString m_sVolume;
		CMachineStorage &m_Storage;
	};

void CAeonEngine::MsgWaitForVolume (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgWaitForVolume
//
//	Aeon.waitForVolume {volume}

	{
	//	Make sure we can do this

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Parameters

	CStringView sVolume = Msg.dPayload.GetElement(0);

	//	If the volume is online then we return OK

	if (m_LocalVolumes.FindVolume(sVolume))
		{
		SendMessageReply(MSG_OK, CDatum(), Msg);
		return;
		}

	//	Otherwise we start a session to wait for the volume.

	StartSession(Msg, new CWaitForVolumeSession(sVolume, m_LocalVolumes));
	}

//	CWaitForVolumeSession ------------------------------------------------------

bool CWaitForVolumeSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession

	{
	//	Wait 100 ms

	ResetTimeout(ADDRESS_AEON_COMMAND, 100);
	return true;
	}

bool CWaitForVolumeSession::OnTimeout (const SArchonMessage &Msg)

//	OnTimeout

	{
	//	Check to see if we have the volume yet

	if (m_Storage.FindVolume(m_sVolume))
		{
		SendMessageReply(MSG_OK);
		return false;
		}

	//	Otherwise, keep waiting

	ResetTimeout(ADDRESS_AEON_COMMAND, 100);
	return true;
	}
