//	CExarchEngine.cpp
//
//	CExarchEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command")
DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule")

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_EXARCH_CHECK_UPGRADE,			"Exarch.checkUpgrade")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")

DECLARE_CONST_STRING(PORT_EXARCH_COMMAND,				"Exarch.command")

DECLARE_CONST_STRING(STR_REQUESTING_UPGRADE,			"Requesting %s to check for upgrades.")

DECLARE_CONST_STRING(ERR_UPGRADE_FAILED,				"Check upgrade failed: %s")

//	Arc.install
//
//	This table is organized by module name:
//
//	/Arc.install/CentralModule/Win10/Arcology.exe
//	/Arc.install/Celestial/Win10/Celestial.exe
//	/Arc.install/Celestial/Win10/DataFile1.dat
//	/Arc.install/Celestial/Win10/DataFile2.dat

DECLARE_CONST_STRING(INSTALL_TABLE_DESC,				"{"
														"name:Arc.install "
														"type:file "
														"}");

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_INSTALL_TABLE,	"Unable to create Arc.install table: %s")
DECLARE_CONST_STRING(ERR_CANT_SEND_MESSAGE,				"Unable to send %s to %s.")

static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;

class CExarchBootSession : public ISessionHandler
	{
	public:
		CExarchBootSession (CExarchEngine &Engine) : 
				m_Engine(Engine)
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		enum class State
			{
			unknown,

			creatingInstallTable,
			waitingForUpgrade,

			done,
			};

		bool RequestCheckUpgrade (const CString &sMachineName, CString *retsError = NULL);

		CExarchEngine &m_Engine;
		State m_iState = State::unknown;
		int m_iStateIndex = -1;

		TArray<CString> m_Machines;
	};

void CExarchEngine::MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAeonOnStart
//
//	Aeon.onStart

	{
	CSmartLock Lock(m_cs);

	//	If Aeon is already running, then skip this. This can happen if Aeon
	//	restarts.

	if (m_bAeonInitialized)
		return;

	//	Start a session to create the install table

	StartSession(Msg, new CExarchBootSession(*this));
	}

//	CBootSession

bool CExarchBootSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a message.

	{
	switch (m_iState)
		{
		case State::creatingInstallTable:
			{
			CString sError;

			//	If we got an error creating the table, then something is
			//	seriously wrong with the arcology. [So we log it and wait for
			//	someone to fix it.]

			if (IsError(Msg) && !strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_INSTALL_TABLE, Msg.dPayload.AsString()));
				return false;
				}

			//	Aeon is running

			m_Engine.SetAeonInitialized();

			//	Check to see if we need to ask other machines to upgrade.

			auto &MecharcologyDb = m_Engine.GetMecharcologyDb();
			for (int i = 0; i < MecharcologyDb.GetMachineCount(); i++)
				{
				SMachineDesc Machine;
				if (!MecharcologyDb.GetMachine(i, &Machine))
					continue;

				if (MecharcologyDb.CheckForUpgrade(Machine.sName))
					m_Machines.Insert(Machine.sName);
				}

			//	If no upgrade needed, we're done.

			if (m_Machines.GetCount() == 0)
				{
				m_iState = State::done;
				return false;
				}

			//	Otherwise, send a message to upgrade

			m_iState = State::waitingForUpgrade;
			m_iStateIndex = 0;
			if (!RequestCheckUpgrade(m_Machines[0], &sError))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, sError);
				return false;
				}

			//	Wait for response

			return true;
			}

		case State::waitingForUpgrade:
			{
			CString sError;

			if (IsError(Msg))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UPGRADE_FAILED, Msg.dPayload.AsString()));
				return false;
				}

			m_iStateIndex++;
			if (m_iStateIndex < m_Machines.GetCount())
				{
				if (!RequestCheckUpgrade(m_Machines[m_iStateIndex], &sError))
					{
					GetProcessCtx()->Log(MSG_LOG_ERROR, sError);
					return false;
					}

				return true;
				}

			//	Done!

			m_iState = State::done;
			return false;
			}

		default:
			return false;
		}
	}

bool CExarchBootSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start session

	{
	CString sError;
	m_iState = State::creatingInstallTable;

	//	Create the /Arc.install table
	//	NOTE: We always create the table and leave it to Aeon to send back an "already exists"
	//	message.

	CDatum dTableDesc;
	CStringBuffer Stream(INSTALL_TABLE_DESC);
	CDatum::Deserialize(CDatum::formatAEONScript, Stream, &dTableDesc);

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(dTableDesc);

	SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, ADDRESS_EXARCH_COMMAND, dPayload, MESSAGE_TIMEOUT);

	return true;
	}

bool CExarchBootSession::RequestCheckUpgrade (const CString &sMachineName, CString *retsError)

//	RequestCheckUpgrade
//
//	Ask the given machine to check for an upgrade.

	{
	GetProcessCtx()->Log(MSG_LOG_INFO, strPattern(STR_REQUESTING_UPGRADE, sMachineName));

	CString sAddress = GetProcessCtx()->GetTransporter().GenerateAddress(PORT_EXARCH_COMMAND, NULL_STR, sMachineName);
	if (!SendMessageCommand(sAddress, MSG_EXARCH_CHECK_UPGRADE, ADDRESS_EXARCH_COMMAND, 0, CDatum()))
		{
		if (retsError) *retsError = strPattern(ERR_CANT_SEND_MESSAGE, MSG_EXARCH_CHECK_UPGRADE, sAddress);
		return false;
		}

	return true;
	}
