//	CExarchEngine.cpp
//
//	CExarchEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command")

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(PORT_EXARCH_COMMAND,				"Exarch.command")

//	Arc.install
//
//	This table is organized by module name:
//
//	/Arc.install/Arcology/Win10/Arcology.exe
//	/Arc.install/Celestial/Win10/Celestial.exe
//	/Arc.install/Celestial/Win10/DataFile1.dat
//	/Arc.install/Celestial/Win10/DataFile2.dat

DECLARE_CONST_STRING(INSTALL_TABLE_DESC,				"{"
														"name:Arc.install "
														"type:file "
														"}");

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_INSTALL_TABLE,	"Unable to create Arc.install table: %s")

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
			done,
			};

		CExarchEngine &m_Engine;
		State m_iState = State::unknown;
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

	//	Start a session to create the users table

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

			//	Now we're done

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

	SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, GenerateAddress(PORT_EXARCH_COMMAND), dPayload, MESSAGE_TIMEOUT);

	return true;
	}
