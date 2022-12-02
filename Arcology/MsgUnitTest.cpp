//	MsgUnitTest.cpp
//
//	Unit tests
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_RESPONSE_ONLY,					"-")
DECLARE_CONST_STRING(STR_NO_TEST_SPECIFIED,				"No unit tests specified")
DECLARE_CONST_STRING(STR_TIMEOUT_EXPECTED,				"T")
DECLARE_CONST_STRING(STR_ERROR_EXPECTED,				"X")
DECLARE_CONST_STRING(STR_FAIL_PATTERN,					"FAIL: %s")
DECLARE_CONST_STRING(STR_PASS,							"pass")
DECLARE_CONST_STRING(STR_DEBUG,							"debug")

DECLARE_CONST_STRING(MSG_ERROR_NO_RIGHT,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_TIMEOUT,					"Error.timeout")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_DRHOUSE_COMMAND,				"DrHouse.command")

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin")

DECLARE_CONST_STRING(ERR_NEED_ADMIN,					"Arc.admin rights required.")
DECLARE_CONST_STRING(ERR_INVALID_PAYLOAD,				"Unable to parse payload: %s.")

const DWORD FLAG_ABORT_ON_FAIL =						0x00000001;

const DWORD MESSAGE_TIMEOUT =							30 * 1000;

class CUnitTestSession : public ISessionHandler
	{
	public:
		CUnitTestSession (void) : m_iTimeout(MESSAGE_TIMEOUT) { }

	protected:
		//	ISessionHandler virtuals
		virtual bool OnProcessMessage (const SArchonMessage &Msg);
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket);

	private:
		struct STestMessageEntry
			{
			const CString &sUnitTest;
			const CString &sAddr;
			const CString &sMsg;
			
			int iPayloadLen;
			const char *pPayload;

			int iReplyLen;
			const char *pReply;

			DWORD dwFlags;
			};

		void SendMessageCommand (int iPos, DWORD dwTicket);

		TArray<int> m_Messages;					//	List of messages to test
		int m_iPos;								//	Current message being tested
		int m_iPass;							//	Number of messages that passed
		int m_iTimeout;							//	Timeout to wait for reply

		static STestMessageEntry m_TestMessageList[];
		static int m_iTestMessageListCount;
	};

//	Test Messages --------------------------------------------------------------

DECLARE_CONST_STRING(UT_AEON,							"Aeon")
DECLARE_CONST_STRING(UT_AEON_BACKUP,					"AeonBackup")
DECLARE_CONST_STRING(UT_AEON_MUTATE,					"AeonMutate")
DECLARE_CONST_STRING(UT_AEON_VIEW_UPDATE,				"AeonViewUpdate")
DECLARE_CONST_STRING(UT_HEXE,							"Hexe")
DECLARE_CONST_STRING(UT_HEXE_TEXT,						"HexeText")

DECLARE_CONST_STRING(ADDR_AEON,							"Aeon.command")
DECLARE_CONST_STRING(ADDR_EXARCH,						"Exarch.command@~/CentralModule")
DECLARE_CONST_STRING(ADDR_HEXE,							"Hexe.command")

DECLARE_CONST_STRING(MSG_AEON_BAD_COMMAND1,				"Aeon.badCommand")
DECLARE_CONST_STRING(MSG_AEON_CREATE_TABLE,				"Aeon.createTable")
DECLARE_CONST_STRING(MSG_AEON_DELETE_TABLE,				"Aeon.deleteTable")
DECLARE_CONST_STRING(MSG_AEON_FILE_DIRECTORY,			"Aeon.fileDirectory")
DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD,			"Aeon.fileDownload")
DECLARE_CONST_STRING(MSG_AEON_FILE_GET_DESC,			"Aeon.fileGetDesc")
DECLARE_CONST_STRING(MSG_AEON_FILE_UPLOAD,				"Aeon.fileUpload")
DECLARE_CONST_STRING(MSG_AEON_FLUSH_DB,					"Aeon.flushDb")
DECLARE_CONST_STRING(MSG_AEON_GET_KEY_RANGE,			"Aeon.getKeyRange")
DECLARE_CONST_STRING(MSG_AEON_GET_ROWS,					"Aeon.getRows")
DECLARE_CONST_STRING(MSG_AEON_GET_TABLES,				"Aeon.getTables")
DECLARE_CONST_STRING(MSG_AEON_GET_VALUE,				"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_AEON_MUTATE,					"Aeon.mutate")
DECLARE_CONST_STRING(MSG_AEON_RECOVER_TABLE_TEST,		"Aeon.recoverTableTest")
DECLARE_CONST_STRING(MSG_AEON_WAIT_FOR_VIEW,			"Aeon.waitForView")
DECLARE_CONST_STRING(MSG_AEON_WAIT_FOR_VOLUME,			"Aeon.waitForVolume")
DECLARE_CONST_STRING(MSG_EXARCH_CREATE_TEST_VOLUME,		"Exarch.createTestVolume")
DECLARE_CONST_STRING(MSG_EXARCH_DELETE_TEST_DRIVE,		"Exarch.deleteTestDrive")
DECLARE_CONST_STRING(MSG_EXARCH_REMOVE_VOLUME,			"Exarch.removeVolume")
DECLARE_CONST_STRING(MSG_HEXE_RUN,						"Hexe.run")
DECLARE_CONST_STRING(MSG_OK,							"OK")

CUnitTestSession::STestMessageEntry CUnitTestSession::m_TestMessageList[] =	{

	//	Aeon -----------------------------------------------------------------------------------------------------------
	//
	//	Make sure Aeon.getTables responds with something (the result will vary, so we
	//	don't bother checking that the result is correct).

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_TABLES,		DEF_STRING(""),		DEF_STRING("-"),	0 },

	//	Test some bad commands to make sure we get errors

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_BAD_COMMAND1,		DEF_STRING(""),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 type:file })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/\\u0009\\u000a\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("!@#$%&^(*&(*"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Make sure we do not allow table names with illegal characters

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:\"no spaces allowed\" x:{keyType:utf8} })"),	DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:\"\\t\\u0009\\f\" x:{keyType:utf8} })"),	DEF_STRING("X"),	0 },

	//	Make sure we accept table names with Unicode characters
	//	Note that there are two way to encode a Unicode character:
	//	(1) directly as utf8 characters
	//	(2) using the \uXXXX syntax (which gets converted to utf8 by the AEON deserializer

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:\"OK\x0C2\x0A9-._~\" x:{keyType:utf8} })"),	DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(\"OK\\u00A9-._~\" row1 row1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(\"OK\x0C2\x0A9-._~\" row1)"),	DEF_STRING("row1_value"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FLUSH_DB,			DEF_STRING(""),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(\"OK\x0C2\x0A9-._~\" row1)"),	DEF_STRING("row1_value"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(\"\") dont_support_null_keys)"),	DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(\"OK\x0C2\x0A9-._~\")"),		DEF_STRING(""),	0 },

	//	Make sure we accept table names that are just numbers (note that quotes are necessary)

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:\"17\" x:{keyType:utf8} })"),	DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(\"17\")"),		DEF_STRING(""),	0 },

	//	Basic table testing

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test iterators

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row3 row1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 row1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 row1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FLUSH_DB,			DEF_STRING(""),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 row1_modified)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_KEY_RANGE,		DEF_STRING("(drhouse_t1 10)"),		DEF_STRING("(row1 row2 row3)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test mutators

	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { firstName:John counter:1 } {} )"),	DEF_STRING("{counter:1 firstName:John}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { lastName:Carter } {} )"),	DEF_STRING("{counter:1 firstName:John lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { } { firstName:delete } )"),	DEF_STRING("{counter:1 lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { temp:foo } { temp:ignore } )"),	DEF_STRING("{counter:1 lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { counter:5 } { counter:increment } )"),	DEF_STRING("{counter:6 lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { counter:10 } { counter:max } )"),	DEF_STRING("{counter:10 lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { counter:1 } { counter:min } )"),	DEF_STRING("{counter:1 lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { counter:10 } { counter:min } )"),	DEF_STRING("{counter:1 lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { counter:0 } { counter:max } )"),	DEF_STRING("{counter:1 lastName:Carter}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { } { version:updateVersion } )"),	DEF_STRING("{counter:1 lastName:Carter version:1}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { version:0 } { version:updateVersion } )"),	DEF_STRING("X"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { version:1 } { version:updateVersion } )"),	DEF_STRING("{counter:1 lastName:Carter version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { firstName:John } { firstName:writeNew } )"),	DEF_STRING("{counter:1 firstName:John lastName:Carter version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { firstName:Mary } { firstName:writeNew } )"),	DEF_STRING("{counter:1 firstName:John lastName:Carter version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { firstName:Mary } { firstName:write } )"),	DEF_STRING("{counter:1 firstName:Mary lastName:Carter version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { skills:(fighting) } { skills:addToSet } )"),	DEF_STRING("{counter:1 firstName:Mary lastName:Carter skills:(fighting) version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { skills:(violin) } { skills:addToSet } )"),	DEF_STRING("{counter:1 firstName:Mary lastName:Carter skills:(fighting violin) version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { skills:(fighting) } { skills:addToSet } )"),	DEF_STRING("{counter:1 firstName:Mary lastName:Carter skills:(fighting violin) version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { skills:(fighting) } { skills:removeFromSet } )"),	DEF_STRING("{counter:1 firstName:Mary lastName:Carter skills:(violin) version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row1 { skills:(speakToAnimals) } { skills:removeFromSet } )"),	DEF_STRING("{counter:1 firstName:Mary lastName:Carter skills:(violin) version:2}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 nil { firstName:John lastName:Arkin } { id:rowID } )"),	DEF_STRING("{firstName:John id:\"1\" lastName:Arkin primaryKey:\"1\"}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 nil { firstName:John lastName:Boorman } { id:rowID } )"),	DEF_STRING("{firstName:John id:\"2\" lastName:Boorman primaryKey:\"2\"}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 nil { firstName:John lastName:Arkin } { primaryKey:code8 } )"),	DEF_STRING(""),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 nil { firstName:Mary lastName:Arkin } { primaryKey:code8 } )"),	DEF_STRING(""),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil 2)"),	DEF_STRING(	""),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 1 { charges:5 } { } )"),			DEF_STRING("{charges:5}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 1 { } { charges:consume } )"),		DEF_STRING("{charges:4}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 1 { charges:5 } { charges:consume } )"),		DEF_STRING("X"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 1 { charges:4 } { charges:consume } )"),		DEF_STRING("{charges:0}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 1 { charges:0 } { charges:consume } )"),		DEF_STRING("{charges:0}"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 1 { charges:1 } { charges:consume } )"),		DEF_STRING("X"),	0 },
	{	UT_AEON_MUTATE,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	2D tables

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} y:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (x1 y1) x1_y1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (x1 y2) x1_y2_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (x1 y3) x1_y3_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (x2 y1) x2_y1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (x2 y2) x2_y2_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(drhouse_t1 (x1 y3))"),	DEF_STRING("x1_y3_value"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(drhouse_t1 (x2 y1))"),	DEF_STRING("x2_y1_value"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FLUSH_DB,			DEF_STRING(""),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(drhouse_t1 (x1 y1))"),	DEF_STRING("x1_y1_value"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(drhouse_t1 (x2 y2))"),	DEF_STRING("x2_y2_value"),	0 },

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (x1 y2) x1_y2_updated_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(drhouse_t1 (x1 y1))"),	DEF_STRING("x1_y1_value"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(drhouse_t1 (x1 y2))"),	DEF_STRING("x1_y2_updated_value"),	0 },

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Partial row iterators

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} y:{keyType:utf8} z:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (johnson albert a100) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (johnson albert a200) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (johnson albert a300) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (johnson mary a55) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (johnson mary a65) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (johnson thor a55) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (merkle anthony a113) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (merkle baylor a119) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (merkle clara a271) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (merkle daria a111) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (merkle daria a112) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (merkle daria a113) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (travis aaron a556) foo)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil 4)"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson albert a200) foo"
																												" (johnson albert a300) foo"
																												" (johnson mary a55) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 1))"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson albert a200) foo"
																												" (johnson albert a300) foo"
																												" (johnson mary a55) foo"
																												" (johnson mary a65) foo"
																												" (johnson thor a55) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 2))"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson albert a200) foo"
																												" (johnson albert a300) foo"
																												" (johnson mary a55) foo"
																												" (johnson mary a65) foo"
																												" (johnson thor a55) foo"
																												" (merkle anthony a113) foo"
																												" (merkle baylor a119) foo"
																												" (merkle clara a271) foo"
																												" (merkle daria a111) foo"
																												" (merkle daria a112) foo"
																												" (merkle daria a113) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 0))"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson albert a200) foo"
																												" (johnson albert a300) foo"
																												" (johnson mary a55) foo"
																												" (johnson mary a65) foo"
																												" (johnson thor a55) foo"
																												" (merkle anthony a113) foo"
																												" (merkle baylor a119) foo"
																												" (merkle clara a271) foo"
																												" (merkle daria a111) foo"
																												" (merkle daria a112) foo"
																												" (merkle daria a113) foo"
																												" (travis aaron a556) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 0 1))"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson albert a200) foo"
																												" (johnson albert a300) foo"
																												" (merkle anthony a113) foo"
																												" (travis aaron a556) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 0 2))"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson albert a200) foo"
																												" (johnson albert a300) foo"
																												" (johnson mary a55) foo"
																												" (johnson mary a65) foo"
																												" (merkle anthony a113) foo"
																												" (merkle baylor a119) foo"
																												" (travis aaron a556) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 0 0 1))"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson mary a55) foo"
																												" (johnson thor a55) foo"
																												" (merkle anthony a113) foo"
																												" (merkle baylor a119) foo"
																												" (merkle clara a271) foo"
																												" (merkle daria a111) foo"
																												" (travis aaron a556) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 0 0 2))"),	DEF_STRING(	"((johnson albert a100) foo"
																												" (johnson albert a200) foo"
																												" (johnson mary a55) foo"
																												" (johnson mary a65) foo"
																												" (johnson thor a55) foo"
																												" (merkle anthony a113) foo"
																												" (merkle baylor a119) foo"
																												" (merkle clara a271) foo"
																												" (merkle daria a111) foo"
																												" (merkle daria a112) foo"
																												" (travis aaron a556) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (johnson albert a100) nil)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil (0 0 0 1))"),	DEF_STRING(	"((johnson albert a200) foo"
																												" (johnson mary a55) foo"
																												" (johnson thor a55) foo"
																												" (merkle anthony a113) foo"
																												" (merkle baylor a119) foo"
																												" (merkle clara a271) foo"
																												" (merkle daria a111) foo"
																												" (travis aaron a556) foo)"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Bug: This was a bug in partial iterators that I reproduced. It's a test
	//	case now since previous ones did not catch it.

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x: { keyType:utf8 } secondaryViews: ({ name:byStatusAndAssignment x:{ key:status keyType:utf8 } y:{ key:assignment keyType:utf8 } columns: (primaryKey)	} ) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0000000\" { assignment:\"george moromisato\" status:assigned unid:\"a0000000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0010000\" { assignment:\"george moromisato\" status:assigned unid:\"a0010000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0020000\" { assignment:\"saint leibowitz\" status:assigned unid:\"a0020000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0030000\" { assignment:\"wolfy\" status:assigned unid:\"a0030000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0040000\" { assignment:\"rpc\" status:assigned unid:\"a0040000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0050000\" { assignment:\"digdug\" status:assigned unid:\"a0050000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0060000\" { assignment:\"star weaver\" status:assigned unid:\"a0060000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0070000\" { assignment:\"atarlost\" status:assigned unid:\"a0070000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0080000\" { assignment:\"alterecco\" status:assigned unid:\"a0080000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0090000\" { assignment:\"ttech\" status:assigned unid:\"a0090000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a00a0000\" { status:reservedForRegistered unid:\"a00a0000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a00b0000\" { status:reservedForRegistered unid:\"a00b0000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a00c0000\" { status:reservedForRegistered unid:\"a00c0000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a00d0000\" { status:reservedForRegistered unid:\"a00d0000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a00e0000\" { status:reservedForRegistered unid:\"a00e0000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a00f0000\" { status:reservedForRegistered unid:\"a00f0000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0100000\" { status:reservedForRegistered unid:\"a0100000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0110000\" { status:reservedForRegistered unid:\"a0110000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0120000\" { status:reservedForRegistered unid:\"a0120000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0130000\" { status:reservedForRegistered unid:\"a0130000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0140000\" { status:reservedForRegistered unid:\"a0140000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 \"a0150000\" { status:reservedForRegistered unid:\"a0150000\"})"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 byStatusAndAssignment) (assigned rpc) (100 1 1))"),	DEF_STRING("((assigned rpc 5) {primaryKey:a0040000})"), 0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	File tables

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 type:file })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1//\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/foo/\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/\\u0009\\u000a\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/!&@\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/[foo]/test\" { version:0 fileDesc:{} } \"This is a bad path\")"),		DEF_STRING("X"),	0 },

	//	Testing file upload/download

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testFile\" { version:0 fileDesc:{} } \"This is the content of the file\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_GET_DESC,		DEF_STRING("(\"/drhouse_t1/testFile\")"),		DEF_STRING("-"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_DOWNLOAD,		DEF_STRING("(\"/drhouse_t1/testFile\")"),		DEF_STRING("-"),	0 },

	//	Test directory

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testFile1\" { } \"testFile1 contents\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testFile2\" { } \"testFile2 contents\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testFile3\" { } \"testFile3 contents\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/anotherFile1\" { } \"anotherFile1 contents\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/anotherFile2\" { } \"anotherFile2 contents\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/zuluFile1\" { } \"zuluFile1 contents\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_DIRECTORY,	DEF_STRING("(\"/drhouse_t1/testFile\" filePath)"),		DEF_STRING("(\"/drhouse_t1/testFile\" \"/drhouse_t1/testFile1\" \"/drhouse_t1/testFile2\" \"/drhouse_t1/testFile3\")"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_DIRECTORY,	DEF_STRING("(\"/drhouse_t1/testFile\" (filePath version))"),		DEF_STRING("({filePath:\"/drhouse_t1/testFile\" version:1} {filePath:\"/drhouse_t1/testFile1\" version:1} {filePath:\"/drhouse_t1/testFile2\" version:1} {filePath:\"/drhouse_t1/testFile3\" version:1})"),	0 },

	//	Test directory hierarchy

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testDir/File1\" { version:0 fileDesc:{} } \"File 1\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testDir/File2\" { version:0 fileDesc:{} } \"File 2\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testDir/File2/File2A\" { version:0 fileDesc:{} } \"File 2a\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_UPLOAD,		DEF_STRING("(\"/drhouse_t1/testDir/File3\" { version:0 fileDesc:{} } \"File 3\")"),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_DIRECTORY,	DEF_STRING("(\"/drhouse_t1/testDir/\" filePath)"),		DEF_STRING("(\"/drhouse_t1/testDir/File1\" \"/drhouse_t1/testDir/File2\" \"/drhouse_t1/testDir/File3\")"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FILE_DIRECTORY,	DEF_STRING("(\"/drhouse_t1/testDir/\" filePath recursive)"),		DEF_STRING("(\"/drhouse_t1/testDir/File1\" \"/drhouse_t1/testDir/File2\" \"/drhouse_t1/testDir/File3\" \"/drhouse_t1/testDir/File2/File2A\")"),	0 },

	//	Test partial uploads

	//	Test partial downloads

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test secondary views

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} secondaryViews:({ name:view1 x:{ key:lastName keyType:utf8 }}) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 { firstName:Albert lastName:Zerg score:100 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 { firstName:Betty lastName:Yanis score:150 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row3 { firstName:Clark lastName:Xerxes score:50 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row4 { firstName:Dana lastName:Zerg score:250 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 nil 10)"),	DEF_STRING("(row1 {firstName:Albert lastName:Zerg score:100} row2 {firstName:Betty lastName:Yanis score:150} row3 {firstName:Clark lastName:Xerxes score:50} row4 {firstName:Dana lastName:Zerg score:250})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING("((Xerxes 3) {primaryKey:row3} (Yanis 2) {primaryKey:row2} (Zerg 1) {primaryKey:row1} (Zerg 4) {primaryKey:row4})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row2 { firstName:Fiona lastName:Huari score:250 } { score:updateGreater })"),	DEF_STRING("{firstName:Fiona lastName:Huari score:250}"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row4 { firstName:Logan lastName:Unger score:450 } { score:updateGreater })"),	DEF_STRING("{firstName:Logan lastName:Unger score:450}"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING("((Huari 2) {primaryKey:row2} (Unger 4) {primaryKey:row4} (Xerxes 3) {primaryKey:row3} (Zerg 1) {primaryKey:row1})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row3 nil)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING("((Huari 2) {primaryKey:row2} (Unger 4) {primaryKey:row4} (Zerg 1) {primaryKey:row1})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} y:{keyType:utf8} secondaryViews:({ name:view1 x:{ key:adventureID keyType:utf8 } y:{ key:score keyType:int32 keySort:descending } }) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 (20000 \"Allen Zerg\") { adventureID:20000 score:250 username:\"Allen Zerg\" } { score:updateGreater })"),	DEF_STRING("{adventureID:20000 score:250 username:\"Allen Zerg\"}"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 (20000 \"Allen Zerg\") { adventureID:20000 score:150 username:\"Allen Zerg\" } { score:updateGreater })"),	DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 (20000 \"Allen Zerg\") { adventureID:20000 score:350 username:\"Allen Zerg\" } { score:updateGreater })"),	DEF_STRING("{adventureID:20000 score:350 username:\"Allen Zerg\"}"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING("((\"20000\" 350 1) {primaryKey:(\"20000\" \"Allen Zerg\")})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test row recovery

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} y:{keyType:utf8} secondaryViews:({ name:view1 x:{ key:adventureID keyType:utf8 } y:{ key:score keyType:int32 keySort:descending } }) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 (20000 \"Allen Zerg\") { adventureID:20000 score:250 username:\"Allen Zerg\" } { score:updateGreater })"),	DEF_STRING("{adventureID:20000 score:250 username:\"Allen Zerg\"}"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 (20000 \"Allen Zerg\") { adventureID:20000 score:150 username:\"Allen Zerg\" } { score:updateGreater })"),	DEF_STRING("X"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FLUSH_DB,			DEF_STRING(""),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 (20000 \"Allen Zerg\") { adventureID:20000 score:350 username:\"Allen Zerg\" } { score:updateGreater })"),	DEF_STRING("{adventureID:20000 score:350 username:\"Allen Zerg\"}"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_RECOVER_TABLE_TEST,	DEF_STRING("(drhouse_t1 (20000 \"Allen Zerg\") { adventureID:20000 score:350 username:\"Allen Zerg\" } { score:updateGreater })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING("((\"20000\" 350 1) {primaryKey:(\"20000\" \"Allen Zerg\")})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test computed secondary views

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} secondaryViews:({ name:view1 x:{ key:(lambda (data) (cat \"KEY_\" (lowercase (@ data 'lastName)))) keyType:utf8 } }) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 { firstName:Albert lastName:Zerg score:100 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 { firstName:Betty lastName:Yanis score:150 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row3 { firstName:Clark lastName:Xerxes score:50 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING("((KEY_xerxes 3) {primaryKey:row3} (KEY_yanis 2) {primaryKey:row2} (KEY_zerg 1) {primaryKey:row1})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_MUTATE,			DEF_STRING("(drhouse_t1 row2 { firstName:Fiona lastName:Huari score:250 } { })"),	DEF_STRING("{firstName:Fiona lastName:Huari score:250}"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING("((KEY_huari 2) {primaryKey:row2} (KEY_xerxes 3) {primaryKey:row3} (KEY_zerg 1) {primaryKey:row1})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test a bug in 2D datetime tables

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} y:{keyType:dateTime keySort:descending} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (row1 #2013-09-20T0:0:0.0) \"version1-data\")"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (row1 #2013-09-20T0:0:0.0) nil)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (row1 #2013-09-09T0:0:0.0) \"version1-data\")"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (row1 #2013-09-09T0:0:0.0) nil)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (row1 #2013-01-01T0:0:0.0) \"version1-data\")"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (row1 #2013-01-01T0:0:0.0) nil)"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 (row1 #2013-09-10T0:0:0.0) \"version2-data\")"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("(drhouse_t1 row1 (10 1 10))"),	DEF_STRING("((row1 #2013-09-10T00:00:00.0000) \"version2-data\")"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} secondaryViews:({ name:view1 x:{ key:type keyType:utf8 } y:{ key:date keyType:dateTime keySort:descending } excludeNilKeys:true }) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 { type:type1 date:#2013-09-05T0:0:0.0 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row3 { type:type1 date:#2013-10-01T0:0:0.0 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 { type:type2 date:#2013-09-20T0:0:0.0 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_FLUSH_DB,			DEF_STRING(""),		DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 { type:type2 date:#2013-09-10T0:0:0.0 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row4 { type:type2 date:#2013-08-30T0:0:0.0 })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) type1 (10 1 10))"),	DEF_STRING("((type1 #2013-10-01T00:00:00.0000 2) {primaryKey:row3} (type1 #2013-09-05T00:00:00.0000 1) {primaryKey:row2})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	List-key secondary views

	{	UT_AEON,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} secondaryViews:({ name:view1 x:{ key:tags keyType:list_utf8 } excludeNilKeys:true }) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 { tags:(a b c) })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 { tags:(b c d) })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row3 { tags:(a d) })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row4 { tags:b })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) a (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row1} {primaryKey:row3})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) b (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row1} {primaryKey:row2} {primaryKey:row4})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) c (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row1} {primaryKey:row2})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) d (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row2} {primaryKey:row3})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 { tags:(a d) })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) a (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row1} {primaryKey:row3})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) b (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row2} {primaryKey:row4})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) c (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row2})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) d (10 1 10) noKey)"),	DEF_STRING("({primaryKey:row1} {primaryKey:row2} {primaryKey:row3})"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 { tags:(b d) })"),	DEF_STRING(""),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) c (10 1 10) noKey)"),	DEF_STRING("()"),	0 },
	{	UT_AEON,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test adding a secondary view after first creation

	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 { firstName:Albert lastName:Zerg score:100 })"),	DEF_STRING(""),	0 },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row2 { firstName:Betty lastName:Yanis score:150 })"),	DEF_STRING(""),	0 },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row3 { firstName:Clark lastName:Xerxes score:50 })"),	DEF_STRING(""),	0 },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row4 { firstName:Dana lastName:Zerg score:250 })"),	DEF_STRING(""),	0 },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 x:{keyType:utf8} secondaryViews:({ name:view1 x:{ key:lastName keyType:utf8 }}) })"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_WAIT_FOR_VIEW,		DEF_STRING("((drhouse_t1 view1))"),		DEF_STRING(""),	0 },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_GET_ROWS,			DEF_STRING("((drhouse_t1 view1) nil 10)"),	DEF_STRING(""),	0 },
	{	UT_AEON_VIEW_UPDATE,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },

	//	Test drive failure
	//
	//	Aeon.createTable should fail if the primary drive failed.

	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_CREATE_TEST_VOLUME,	DEF_STRING("(drhouse_vol01)"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_WAIT_FOR_VOLUME,		DEF_STRING("(drhouse_vol01)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_DELETE_TEST_DRIVE,	DEF_STRING("(drhouse_vol01)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 primaryVolume:drhouse_vol01 x:{keyType:utf8}})"),		DEF_STRING("X"),	0 },
	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_REMOVE_VOLUME,	DEF_STRING("(drhouse_vol01)"),		DEF_STRING(""),	0 },

	//	Aeon.flushDb should use a backup.

	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_CREATE_TEST_VOLUME,	DEF_STRING("(drhouse_vol02)"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_WAIT_FOR_VOLUME,		DEF_STRING("(drhouse_vol02)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 primaryVolume:drhouse_vol02 x:{keyType:utf8}})"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 row1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_DELETE_TEST_DRIVE,	DEF_STRING("(drhouse_vol02)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_FLUSH_DB,			DEF_STRING(""),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_REMOVE_VOLUME,	DEF_STRING("(drhouse_vol02)"),		DEF_STRING(""),	0 },

	//	Aeon.getValue should use a backup.

	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_CREATE_TEST_VOLUME,	DEF_STRING("(drhouse_vol03)"),		DEF_STRING(""),	FLAG_ABORT_ON_FAIL },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_WAIT_FOR_VOLUME,		DEF_STRING("(drhouse_vol03)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_CREATE_TABLE,		DEF_STRING("({ name:drhouse_t1 primaryVolume:drhouse_vol03 x:{keyType:utf8}})"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_INSERT,			DEF_STRING("(drhouse_t1 row1 row1_value)"),	DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_FLUSH_DB,			DEF_STRING(""),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_DELETE_TEST_DRIVE,	DEF_STRING("(drhouse_vol03)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_GET_VALUE,			DEF_STRING("(drhouse_t1 row1)"),		DEF_STRING("row1_value"),	0 },
	{	UT_AEON_BACKUP,	ADDR_AEON,		MSG_AEON_DELETE_TABLE,		DEF_STRING("(drhouse_t1)"),		DEF_STRING(""),	0 },
	{	UT_AEON_BACKUP,	ADDR_EXARCH,	MSG_EXARCH_REMOVE_VOLUME,	DEF_STRING("(drhouse_vol03)"),		DEF_STRING(""),	0 },

	//	Hexe -----------------------------------------------------------------------------------------------------------

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"\\\"Hello, World!\\\"\")"),		DEF_STRING("Hello, World!"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"nil\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"true\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"0\")"),		DEF_STRING("0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"0x10\")"),		DEF_STRING("16"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"-10\")"),		DEF_STRING("-10"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"-2147483648\")"),		DEF_STRING("-2147483648"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"2147483647\")"),		DEF_STRING("2147483647"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"0x80000000\")"),		DEF_STRING("-2147483648"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"0x7fffffff\")"),		DEF_STRING("2147483647"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"1.2\")"),		DEF_STRING("1.2"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\".2\")"),		DEF_STRING("0.20000000000000001"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"+1.2\")"),		DEF_STRING("1.2"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"-1.2\")"),		DEF_STRING("-1.2"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"1.2e1\")"),		DEF_STRING("12.0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"1.2e-1\")"),		DEF_STRING("0.12"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"1.2e+2\")"),		DEF_STRING("120.0"),	0 },

	//	Comments

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"1//2\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"1/*test*/\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(/*divide*// 10 2)\")"),		DEF_STRING("5"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(quote 1/2)\")"),		DEF_STRING("1/2"),	0 },

	//	Quote

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(quote foo)\")"),		DEF_STRING("foo"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"'foo\")"),		DEF_STRING("foo"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"''foo\")"),		DEF_STRING("(quote foo)"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"'123\")"),		DEF_STRING("123"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"'(hello world)\")"),		DEF_STRING("(hello world)"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"'('hello world)\")"),		DEF_STRING("((quote hello) world)"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"'((hello) world)\")"),		DEF_STRING("((hello) world)"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"'nil\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"''nil\")"),		DEF_STRING("(quote nil)"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"'true\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 'nil nil)\")"),		DEF_STRING("true"),	0 },

	//	Test some basic math operations

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(+ 1 1)\")"),		DEF_STRING("2"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(+)\")"),		DEF_STRING("0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(+ 1)\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(+ 1 1 1 1 1 1 1 1)\")"),		DEF_STRING("8"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(-)\")"),		DEF_STRING("0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(- 1)\")"),		DEF_STRING("-1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(- 2 1)\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(- 8 1 1 1 1 1 1 1 1)\")"),		DEF_STRING("0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(- -10 5)\")"),		DEF_STRING("-15"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(*)\")"),		DEF_STRING("0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(* 1)\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(* 1 1)\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(* 5 5 5)\")"),		DEF_STRING("125"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(/)\")"),		DEF_STRING("0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(/ 2)\")"),		DEF_STRING("0.5"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(/ 10 5)\")"),		DEF_STRING("2"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(/ 100 5 4)\")"),		DEF_STRING("5"),	0 },

	//	Comparisons

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(=)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 1 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 1 2)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 1 1 1 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 0 1 1 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 1 1 1 0)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"a\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"A\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"a\\\" \\\"A\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"b\\\" \\\"A\\\")\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"a\\\" \\\"A\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"1\\\" 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"0.1\\\" 0.1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"0.100\\\" 0.1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"1e-1\\\" 0.1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= 1 \\\"1\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"asdf\\\" 0)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"asdf\\\" \\\"asdf\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= \\\"ASDF\\\" \\\"asdf\\\")\")"),		DEF_STRING("true"),	0 },

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 0)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< -1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 1 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 1 2)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< -1 2)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 1 2 3 4 5 6 7)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 1 1 2 3 4 5 6 7)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 1 2 3 4 5 6 7 0)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< 8 1 2 3 4 5 6 7)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< \\\"\\\")\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< \\\"a\\\")\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< \\\"a\\\" \\\"a\\\")\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< \\\"a\\\" \\\"b\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< \\\"a\\\" \\\"B\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< \\\"a\\\" \\\"B\\\" \\\"c\\\")\")"),		DEF_STRING("true"),	0 },

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 0)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> -1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 1 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 2 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 2 -1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 7 6 5 4 3 2 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 7 6 5 5 4 3 2 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 0 7 6 5 4 3 2 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> 7 6 5 4 3 2 1 8)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> \\\"\\\")\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> \\\"a\\\" \\\"a\\\")\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> \\\"b\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> \\\"B\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> \\\"c\\\" \\\"B\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<=)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= 0)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= -1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= 1 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= 1 2)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= -1 2)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= 1 1 2 3 4 4 5 6 7 7)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= 1 2 3 4 5 6 7 0)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= 8 1 2 3 4 5 6 7)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"a\\\")\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"a\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"a\\\" \\\"b\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"a\\\" \\\"B\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"a\\\" \\\"B\\\" \\\"c\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"a\\\" \\\"d\\\" \\\"c\\\")\")"),		DEF_STRING("nil"),	0 },

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>=)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 0)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= -1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 1 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 2 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 2 -1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 7 7 6 5 4 4 3 2 1 1)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 0 7 6 5 4 3 2 1)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= 7 6 5 4 3 2 1 8)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= \\\"\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= \\\"a\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= \\\"b\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= \\\"B\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(>= \\\"c\\\" \\\"B\\\" \\\"a\\\")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(<= \\\"a\\\" \\\"d\\\" \\\"c\\\")\")"),		DEF_STRING("nil"),	0 },

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= nil nil)\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= nil "")\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= nil (list))\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= '(1 2 3) '(1 2 3))\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= '(1 3 3) '(1 2 3))\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= nil '(1 2 3))\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(!= nil '(1 2 3))\")"),		DEF_STRING("true"),	0 },

	//	Min/Max

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max 1)\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max 1 2)\")"),		DEF_STRING("2"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max 3 1 2)\")"),	DEF_STRING("3"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max 3 1.5 2 3.5)\")"),	DEF_STRING("3.5"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max -1 1.0 -1.0)\")"),	DEF_STRING("1.0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max -1 \\\"1.0\\\" -1.0)\")"),	DEF_STRING("1.0"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max 10 1.2 -11)\")"),	DEF_STRING("10"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max nil 1)\")"),	DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max nil -11)\")"),	DEF_STRING("-11"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(max (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-22T0:0:0.0\\\"))\")"),	DEF_STRING("#2012-01-22T00:00:00.0000"),	0 },

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min 1)\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min 1 2)\")"),		DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min 3 1 2)\")"),	DEF_STRING("1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min 3 1.5 2 3.5)\")"),	DEF_STRING("1.5"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min -1 1.0 -1.0)\")"),	DEF_STRING("-1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min -1 \\\"1.0\\\" -1.0)\")"),	DEF_STRING("-1"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min 10 1.2 -11)\")"),	DEF_STRING("-11"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min nil 1)\")"),	DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min nil -11)\")"),	DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(min (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-22T0:0:0.0\\\"))\")"),	DEF_STRING("#2012-01-17T00:00:00.0000"),	0 },

	//	DateTime

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= (dateTime \\\"2012-01-17T0:0:0.0\\\"))\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= (dateTime \\\"2012-01-17T0:0:0.0\\\") nil)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-17T0:0:0.0\\\"))\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-01T0:0:0.0\\\"))\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(= (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-17T0:0:0.0\\\"))\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< nil (dateTime \\\"2012-01-17T0:0:0.0\\\"))\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< nil (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-17T0:0:0.1\\\"))\")"),		DEF_STRING("true"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(< (dateTime \\\"2012-01-17T0:0:0.0\\\") nil)\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> nil (dateTime \\\"2012-01-17T0:0:0.0\\\"))\")"),		DEF_STRING("nil"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(> (dateTime \\\"2012-01-17T0:0:0.1\\\") (dateTime \\\"2012-01-17T0:0:0.0\\\"))\")"),		DEF_STRING("true"),	0 },

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(dateTimeSpan (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-17T1:0:0.0\\\") )\")"),		DEF_STRING("3600"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(dateTimeSpan (dateTime \\\"2012-01-17T0:0:0.0\\\") (dateTime \\\"2012-01-16T23:0:0.0\\\") )\")"),		DEF_STRING("-3600"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(dateTimeSpan (dateTime \\\"2012-01-17T0:0:0.0\\\") 3600 )\")"),		DEF_STRING("#2012-01-17T01:00:00.0000"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(dateTimeSpan (dateTime \\\"2012-01-17T0:0:0.0\\\") -3600 )\")"),		DEF_STRING("#2012-01-16T23:00:00.0000"),	0 },

	//	Infinite precision integers

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"+2147483648\")"),		DEF_STRING("2147483648"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"-2147483649\")"),		DEF_STRING("-2147483649"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"2147583647\")"),			DEF_STRING("2147583647"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"2147383647\")"),			DEF_STRING("2147383647"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"0x100000000\")"),		DEF_STRING("4294967296"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(+ 0x100000000 0x100000000)\")"),		DEF_STRING("8589934592"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(- 0x400000000 0x123456789)\")"),		DEF_STRING("12293150839"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(+ 2000000000 2000000000)\")"),		DEF_STRING("4000000000"),	0 },
//	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(* 2147383647 2147383647)\")"),			DEF_STRING("4611686014132420609"),	0 },
//	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(/ 0x400000000 0x123456789)\")"),		DEF_STRING("3"),	0 },

	//	Invocation

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(invoke 'Aeon.getTables)\")"),		DEF_STRING("-"),	0 },

	//	Error

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(error 'Error 'Description)\")"),		DEF_STRING("-"),	0 },

	//	Control Flow

	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(block nil 5)\")"),		DEF_STRING("5"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(block (a) (set! a 5))\")"),		DEF_STRING("5"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(block (a b) (set! a 5) (set! b 5) (+ a b))\")"),		DEF_STRING("10"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(block ((a 5)) (+ a 5))\")"),		DEF_STRING("10"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(block ((a 5) (b (+ a 1))) b)\")"),		DEF_STRING("6"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(block ((a 5)) (block nil a))\")"),		DEF_STRING("5"),	0 },
	{	UT_HEXE,	ADDR_HEXE,		MSG_HEXE_RUN,				DEF_STRING("(\"(block ((a 5)) (block ((a 1)) a))\")"),		DEF_STRING("1"),	0 },

	//	Hexe -----------------------------------------------------------------------------------------------------------

	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"Text.\\\" 'hexeText)\")"),					DEF_STRING("<p>Text.</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"Line1\\nLine2\\\" 'hexeText)\")"),			DEF_STRING("<p>Line1<br/>Line2</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"Line1\\nLine2\\n\\nPara2\\\" 'hexeText)\")"),	DEF_STRING("<p>Line1<br/>Line2</p><p>Para2</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"Text **bold**.\\\" 'hexeText)\")"),		DEF_STRING("<p>Text <b>bold</b>.</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"Text //italic//.\\\" 'hexeText)\")"),		DEF_STRING("<p>Text <i>italic</i>.</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"==H1==\\nText.\\\" 'hexeText)\")"),			DEF_STRING("<h1>H1</h1><p>Text.</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"===H2===\\nText.\\\" 'hexeText)\")"),		DEF_STRING("<h2>H2</h2><p>Text.</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"P1\\n* B1\\n* B2\\nP2\\\" 'hexeText)\")"),	DEF_STRING("<p>P1</p><ul><li>B1</li><li>B2</li></ul><p>P2</p>"),	0 },
	{	UT_HEXE_TEXT,	ADDR_HEXE,	MSG_HEXE_RUN,				DEF_STRING("(\"(html \\\"P1\\n# N1\\n# N2\\nP2\\\" 'hexeText)\")"),		DEF_STRING("<p>P1</p><ol><li>N1</li><li>N2</li></ol><p>P2</p>"),	0 },
	};

int CUnitTestSession::m_iTestMessageListCount = SIZEOF_STATIC_ARRAY(CUnitTestSession::m_TestMessageList);

void CDrHouseEngine::MsgUnitTest (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgUnitTest
//
//	Runs one or more unit tests

	{
	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	StartSession(Msg, new CUnitTestSession);
	}

bool CUnitTestSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Process a message for the session

	{
	int iTest = m_Messages[m_iPos];

	//	Serialize the response

	CString sResponse;
	if (Msg.dPayload.GetBasicType() == CDatum::typeString)
		sResponse = Msg.dPayload;
	else if (Msg.dPayload.GetBasicType() == CDatum::typeIntegerIP)
		sResponse = Msg.dPayload.AsString();
	else
		{
		CBuffer Buffer(4096);
		Msg.dPayload.Serialize(CDatum::EFormat::AEONScript, Buffer);

		sResponse = CString(Buffer.GetPointer(), Buffer.GetLength());
		}

	//	Compare the response

	bool bSuccess;
	CString sCorrectResponse = CString(m_TestMessageList[iTest].pReply, m_TestMessageList[iTest].iReplyLen);
	if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
		{
		if (strEquals(sCorrectResponse, STR_TIMEOUT_EXPECTED))
			{
			SendMessageReplyProgress(strPattern("%s...pass", m_TestMessageList[iTest].sMsg));
			bSuccess = true;
			}
		else
			{
			SendMessageReplyProgress(strPattern("%s: %s -> TIMEOUT", m_TestMessageList[iTest].sMsg, CString(m_TestMessageList[iTest].pPayload, m_TestMessageList[iTest].iPayloadLen)));
			bSuccess = false;
			}
		}
	else if (IsError(Msg))
		{
		if (strEquals(sCorrectResponse, STR_ERROR_EXPECTED))
			{
			SendMessageReplyProgress(strPattern("%s...pass", m_TestMessageList[iTest].sMsg));
			bSuccess = true;
			}
		else
			{
			SendMessageReplyProgress(strPattern("%s: %s -> %s: %s", m_TestMessageList[iTest].sMsg, CString(m_TestMessageList[iTest].pPayload, m_TestMessageList[iTest].iPayloadLen), Msg.sMsg, sResponse));
			bSuccess = false;
			}
		}
	else if (sCorrectResponse.IsEmpty())
		{
		if (strEquals(Msg.sMsg, MSG_OK))
			SendMessageReplyProgress(strPattern("%s...pass", m_TestMessageList[iTest].sMsg));
		else
			SendMessageReplyProgress(strPattern("%s...%s", m_TestMessageList[iTest].sMsg, sResponse));

		bSuccess = true;
		}
	else if (strEquals(sCorrectResponse, STR_RESPONSE_ONLY))
		{
		SendMessageReplyProgress(strPattern("%s...responded", m_TestMessageList[iTest].sMsg));
		bSuccess = true;
		}
	else if (strEquals(sResponse, sCorrectResponse))
		{
		SendMessageReplyProgress(strPattern("%s...%s", 
				m_TestMessageList[iTest].sMsg,
				STR_PASS));
		bSuccess = true;
		}
	else
		{
		SendMessageReplyProgress(strPattern("%s: %s -> %s", 
				m_TestMessageList[iTest].sMsg,
				CString(m_TestMessageList[iTest].pPayload, m_TestMessageList[iTest].iPayloadLen),
				strPattern(STR_FAIL_PATTERN, sResponse)));
		bSuccess = false;
		}

	//	Keep track of successes

	if (bSuccess)
		m_iPass++;

	//	See if we need to abort

	bool bAbort = (!bSuccess && (m_TestMessageList[iTest].dwFlags & FLAG_ABORT_ON_FAIL));

	//	If we're not done, continue processing

	if (!bAbort && ++m_iPos < m_Messages.GetCount())
		{
		SendMessageCommand(m_iPos, Msg.dwTicket);
		return true;
		}

	//	Otherwise we're done; Send a summary of our tests.

	SendMessageReply(MSG_REPLY_DATA, strPattern("Pass: %d  Fail: %d", m_iPass, m_Messages.GetCount() - m_iPass));

	//	Session done

	return false;
	}

bool CUnitTestSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start the session

	{
	int i, j;

	//	If we don't have a parameter, then add all messages in order

	if (Msg.dPayload.GetCount() == 0)
		{
		for (i = 0; i < m_iTestMessageListCount; i++)
			m_Messages.Insert(i);
		}

	//	If we have a single parameter and it is "debug" then 
	//	we run all tests with no timeout

	else if (Msg.dPayload.GetCount() == 1 && strEquals(Msg.dPayload.GetElement(0), STR_DEBUG))
		{
		for (i = 0; i < m_iTestMessageListCount; i++)
			m_Messages.Insert(i);

		m_iTimeout = 0;
		}

	//	Otherwise, loop over all parameters and add the messages for
	//	each unit test specified.

	else
		{
		for (i = 0; i < Msg.dPayload.GetCount(); i++)
			{
			const CString &sUnitTest = Msg.dPayload.GetElement(i);

			//	Add all the messages for this unit test

			for (j = 0; j < m_iTestMessageListCount; j++)
				if (strEquals(sUnitTest, m_TestMessageList[j].sUnitTest))
					m_Messages.Insert(j);
			}
		}

	//	If we have no messages, then we're done

	if (m_Messages.GetCount() == 0)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, STR_NO_TEST_SPECIFIED);
		return false;
		}

	//	Set the position to the message

	m_iPos = 0;
	m_iPass = 0;

	//	Send out the first message

	SendMessageCommand(m_iPos, dwTicket);

	//	Done

	return true;
	}

void CUnitTestSession::SendMessageCommand (int iPos, DWORD dwTicket)

//	SendMessageCommand
//
//	Sends the given message

	{
	int iTest = m_Messages[iPos];

	//	Parse the payload
	//	NOTE: We use CHexeDocument so that we can parse lambda expression into
	//	code blocks.

	CMemoryBuffer Buffer(const_cast<char*>(m_TestMessageList[iTest].pPayload), m_TestMessageList[iTest].iPayloadLen);
	CDatum dPayload;
	if (Buffer.GetLength() > 0 && !CHexeDocument::ParseData(Buffer, &dPayload))
		{
		GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern(ERR_INVALID_PAYLOAD, CString(m_TestMessageList[iTest].pPayload, m_TestMessageList[iTest].iPayloadLen)));
		return;
		}

	//	Send it

	ISessionHandler::SendMessageCommand(m_TestMessageList[iTest].sAddr,
			m_TestMessageList[iTest].sMsg,
			GenerateAddress(PORT_DRHOUSE_COMMAND),
			dPayload,
			m_iTimeout);
	}
