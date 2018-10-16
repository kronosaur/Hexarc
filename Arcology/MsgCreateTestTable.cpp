//	MsgCreateTestTable.cpp
//
//	Aeon table tests
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int DEFAULT_DATA_SIZE =							1200;
const int DEFAULT_ROW_COUNT =							10;
const DWORD DEFAULT_TIMEOUT =							30 * 1000;

DECLARE_CONST_STRING(DEFAULT_TABLE_NAME,				"drhouse_t1")

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")
DECLARE_CONST_STRING(PORT_DRHOUSE_COMMAND,				"DrHouse.command")

DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_OK,							"OK")

DECLARE_CONST_STRING(TABLE_DEF,							"{ name:%s x:{ keyType:utf8 }}")

static char RANDOM_CHAR_TABLE[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static int RANDOM_CHAR_TABLE_SIZE = sizeof(RANDOM_CHAR_TABLE) - 1;	//	exclude null-terminator

class CCreateTestTableSession : public ISessionHandler
	{
	public:
		CCreateTestTableSession (CDatum dTable, int iRows, int iDataSize);

	protected:
		//	ISessionHandler virtuals
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		enum States
			{
			stateCreateTable,
			stateCreatingRows,
			};

		bool CreateTableRow (void);
		CString RandomString (int iLength);

		States m_iState;
		CDatum m_dTable;
		CString m_sTableSerialized;
		int m_iRowsLeft;
		int m_iDataSize;
	};

void CDrHouseEngine::MsgCreateTestTable (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateTestTable
//
//	DrHouse.createTestTable [{noOfRows} [{dataSize} [{tableName}]]]

	{
	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	int iRows = (int)Msg.dPayload.GetElement(0);
	if (iRows <= 0)
		iRows = DEFAULT_ROW_COUNT;

	int iDataSize = (int)Msg.dPayload.GetElement(1);
	if (iDataSize <= 0)
		iDataSize = DEFAULT_DATA_SIZE;

	CDatum dTable = Msg.dPayload.GetElement(2);

	StartSession(Msg, new CCreateTestTableSession(dTable, iRows, iDataSize));
	}

//	CCreateTestTableSession ----------------------------------------------------

CCreateTestTableSession::CCreateTestTableSession (CDatum dTable, int iRows, int iDataSize) : 
		m_dTable(dTable), 
		m_iRowsLeft(iRows), 
		m_iDataSize(iDataSize)

//	CCreateTestTableSession constructor

	{
	if (dTable.IsNil())
		{
		m_sTableSerialized = DEFAULT_TABLE_NAME;
		m_dTable = CDatum(DEFAULT_TABLE_NAME);
		}
	else
		{
		CStringBuffer TableName;
		m_dTable.Serialize(CDatum::formatAEONScript, TableName);
		m_sTableSerialized.TakeHandoff(TableName);
		}
	}

bool CCreateTestTableSession::CreateTableRow (void)

//	CreateTableRow
//
//	Adds a table row 

	{
	if (m_iRowsLeft <= 0)
		return false;

	m_iState = stateCreatingRows;
	m_iRowsLeft--;

	//	Generate a random key and random data

	CString sKey = strPattern("%s-%d", RandomString(mathRandom(1, 32)), m_iRowsLeft);
	CString sData = RandomString(mathRandom((int)(m_iDataSize * 0.8), (int)(m_iDataSize * 1.2)));

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(m_dTable);
	pPayload->Insert(sKey);
	pPayload->Insert(sData);

	//	Send message

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_INSERT,
			GenerateAddress(PORT_DRHOUSE_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

#ifdef DEBUG
	//printf("Insert: %s\n", (LPSTR)sKey);
#endif

	return true;
	}

bool CCreateTestTableSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a reply

	{
	//	If we got an error, then return it to the client.

	if (IsError(Msg))
		{
		SendMessageReplyError(Msg.sMsg, Msg.dPayload);
		return false;
		}

	switch (m_iState)
		{
		case stateCreateTable:
			CreateTableRow();
			return true;

		case stateCreatingRows:
			{
			//	If we're done creating rows, send a reply

			if (!CreateTableRow())
				{
				SendMessageReply(MSG_OK);
				return false;
				}

			//	Otherwise, wait for response from insert rows

			return true;
			}
		}

	return false;
	}

bool CCreateTestTableSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session

	{
	srand(sysGetTickCount());

	//	We start by creating the table.

	m_iState = stateCreateTable;

	//	Payload

	CDatum dTableDesc;
	if (!CDatum::Deserialize(CDatum::formatAEONScript, CStringBuffer(strPattern(TABLE_DEF, m_sTableSerialized)), &dTableDesc))
		{
		ASSERT(false);
		return false;
		}

	CComplexArray *pPayload = new CComplexArray;
	pPayload->Insert(dTableDesc);

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			MSG_AEON_CREATE_TABLE,
			GenerateAddress(PORT_DRHOUSE_COMMAND),
			CDatum(pPayload),
			DEFAULT_TIMEOUT);

	return true;
	}

CString CCreateTestTableSession::RandomString (int iLength)

//	RandomString
//
//	Generates a random string.

	{
	CString sString(iLength);

	char *pPos = sString.GetParsePointer();
	while (*pPos != '\0')
		*pPos++ = RANDOM_CHAR_TABLE[mathRandom(0, RANDOM_CHAR_TABLE_SIZE-1)];

	return sString;
	}
