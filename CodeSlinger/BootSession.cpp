//	BootSession.cpp
//
//	CCodeSlingerEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(ADDR_AEON_COMMAND,					"Aeon.command")
DECLARE_CONST_STRING(PORT_CODE_COMMAND,					"Code.command")

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_ERROR_ALREADY_EXISTS,			"Error.alreadyExists")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(CODE_TABLE_DESC,					"{"
														"name:Arc.code "
														"x:{keyType:utf8} "
														"}");

DECLARE_CONST_STRING(PROGRAMS_TABLE_NAME,				"Arc.programs");
DECLARE_CONST_STRING(PROGRAMS_TABLE_DESC,				"Arc.table Arc.programs "
														"{"
														"name:Arc.programs "
														"x:{keyType:utf8} "

														"secondaryViews: ("
															"{ name:byName "
																"x: {"
																"	key:name "
																"	keyType:utf8 "
																"	keySort:ascending "
																"	} "
																"columns: (primaryKey) "
																"excludeNilKeys: true "
																"}"
															")"
														"}");

DECLARE_CONST_STRING(SOURCE_TABLE_DESC,					"{"
														"name:Arc.source "
														"type:file "
														"}");

DECLARE_CONST_STRING(TYPE_ARC_TABLE,					"Arc.table")

DECLARE_CONST_STRING(ERR_EXPECTED_ARC_TABLE,			"Unable to interpret table definition.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_CODE_TABLE,	"Unable to create Arc.code table: %s")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_PROGRAMS_TABLE,	"Unable to create Arc.programs table: %s")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_SOURCE_TABLE,	"Unable to create Arc.source table: %s")

static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;

class CBootSession : public ISessionHandler
	{
	public:
		CBootSession (CCodeSlingerEngine &Engine) : 
				m_Engine(Engine)
			{ }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum class States
			{
			unknown,
			creatingProgramsTable,
			creatingSourceTable,
			creatingCodeTable,
			done,
			};

		CCodeSlingerEngine &m_Engine;
		States m_iState = States::unknown;
	};

void CCodeSlingerEngine::MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

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

	StartSession(Msg, new CBootSession(*this));
	}

//	CBootSession ---------------------------------------------------------------

bool CBootSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a response

	{
	switch (m_iState)
		{
		case States::creatingProgramsTable:
			{
			//	If we got an error creating the table, then something is
			//	seriously wrong with the arcology. [So we log it and wait for
			//	someone to fix it.]

			if (IsError(Msg) && !strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_PROGRAMS_TABLE, Msg.dPayload.AsString()));
				return false;
				}

			//	Now create the Arc.source table

			m_iState = States::creatingSourceTable;

			CDatum dTableDesc;
			CStringBuffer Stream(SOURCE_TABLE_DESC);
			CDatum::Deserialize(CDatum::formatAEONScript, Stream, &dTableDesc);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(dTableDesc);

			SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, GenerateAddress(PORT_CODE_COMMAND), dPayload, MESSAGE_TIMEOUT);

			return true;
			}

		case States::creatingSourceTable:
			{
			//	If we got an error creating the table, then something is
			//	seriously wrong with the arcology. [So we log it and wait for
			//	someone to fix it.]

			if (IsError(Msg) && !strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_SOURCE_TABLE, Msg.dPayload.AsString()));
				return false;
				}

			//	Now create the Arc.code table

			m_iState = States::creatingCodeTable;

			CDatum dTableDesc;
			CStringBuffer Stream(CODE_TABLE_DESC);
			CDatum::Deserialize(CDatum::formatAEONScript, Stream, &dTableDesc);

			CDatum dPayload(CDatum::typeArray);
			dPayload.Append(dTableDesc);

			SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, GenerateAddress(PORT_CODE_COMMAND), dPayload, MESSAGE_TIMEOUT);

			return true;
			}

		case States::creatingCodeTable:
			{
			//	If we got an error creating the table, then something is
			//	seriously wrong with the arcology. [So we log it and wait for
			//	someone to fix it.]

			if (IsError(Msg) && !strEquals(Msg.sMsg, MSG_ERROR_ALREADY_EXISTS))
				{
				GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_CODE_TABLE, Msg.dPayload.AsString()));
				return false;
				}

			//	Aeon is running

			m_Engine.SetAeonInitialized();

			//	We're done initializing

			m_iState = States::done;
			return false;
			}

		default:
			return false;
		}
	}

bool CBootSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start session

	{
	CString sError;
	m_iState = States::creatingProgramsTable;

	//	Create the /Arc.programs table
	//	NOTE: We always create the table and leave it to Aeon to send back an "already exists"
	//	message.

	CHexeProcess Process;
	if (!Process.LoadStandardLibraries(&sError))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, sError);
		return false;
		}

	//	Parse the table definition into a document

	CHexeDocument TableDoc;
	CStringBuffer Stream(PROGRAMS_TABLE_DESC);
	if (!TableDoc.InitFromStream(Stream, Process, &sError))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, sError);
		return false;
		}

	//	Get the table description (we expect only 1)

	int iTableIndex = TableDoc.GetTypeIndex(TYPE_ARC_TABLE);
	if (TableDoc.GetTypeIndexCount(iTableIndex) != 1)
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, ERR_EXPECTED_ARC_TABLE);
		return false;
		}

	const CString &sTableName = TableDoc.GetTypeIndexName(iTableIndex, 0);
	CDatum dTableDesc = TableDoc.GetTypeIndexData(iTableIndex, 0);

	//	Create the command

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(dTableDesc);

	SendMessageCommand(ADDR_AEON_COMMAND, MSG_AEON_CREATE_TABLE, GenerateAddress(PORT_CODE_COMMAND), dPayload, MESSAGE_TIMEOUT);

	return true;
	}
