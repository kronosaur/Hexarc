//	CAeonDiagnostics.cpp
//
//	AeonDB diagnostics
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_INDEX,	"index");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_GROUP,	"group");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_VALUE,	"value");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_MISSING_MSG_FIELD,	"Missing msg field.");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_NIL,	"nil");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_ROW,	"row");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_KEY,	"key");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_LIMITS,	"limits");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_OPTIONS,	"options");
DECLARE_CONST_STRING(STR_CAEON_DIAGNOSTICS_UNNAMED,	"unnamed");

DECLARE_CONST_STRING(ADDRESS_DIAGNOSTICS,				"AeonDiagnostics")

DECLARE_CONST_STRING(FIELD_EXPECT_MSG,					"expectMsg")
DECLARE_CONST_STRING(FIELD_EXPECT_PAYLOAD,				"expectPayload")
DECLARE_CONST_STRING(FIELD_EXPECT_PAYLOAD_NIL,			"expectPayloadNil")
DECLARE_CONST_STRING(FIELD_COUNT,						"count")
DECLARE_CONST_STRING(FIELD_EXPECT_RESULT_COUNT,			"expectResultCount")
DECLARE_CONST_STRING(FIELD_KEY_PREFIX,					"keyPrefix")
DECLARE_CONST_STRING(FIELD_LABEL,						"label")
DECLARE_CONST_STRING(FIELD_MSG,							"msg")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_PAYLOAD,						"payload")
DECLARE_CONST_STRING(FIELD_PERF,						"perf")
DECLARE_CONST_STRING(FIELD_REPEAT,						"repeat")
DECLARE_CONST_STRING(FIELD_START,						"start")
DECLARE_CONST_STRING(FIELD_STEPS,						"steps")
DECLARE_CONST_STRING(FIELD_TABLE,						"table")
DECLARE_CONST_STRING(FIELD_VALUE_BYTES,					"valueBytes")
DECLARE_CONST_STRING(FIELD_VIEW,						"view")

DECLARE_CONST_STRING(FILE_DIAGNOSTICS,					"AeonDiagnostics.ars")
DECLARE_CONST_STRING(FOLDER_AEONDB,						"AeonDb")
DECLARE_CONST_STRING(FOLDER_DIAGNOSTICS,				"Diagnostics")
DECLARE_CONST_STRING(FOLDER_GRIDWHALE,					"GridWhale")
DECLARE_CONST_STRING(FOLDER_TEST_ROOT,					"AeonDbDiagnostics")

DECLARE_CONST_STRING(MACHINE_NAME,						"DiagnosticsMachine")
DECLARE_CONST_STRING(MODULE_NAME,						"AeonDB")

DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")
DECLARE_CONST_STRING(MSG_AEON_FLUSH_DB,					"Aeon.flushDb")
DECLARE_CONST_STRING(MSG_AEON_GET_ROWS,					"Aeon.getRows")
DECLARE_CONST_STRING(MSG_AEON_GET_VALUE,				"Aeon.getValue")
DECLARE_CONST_STRING(MSG_AEON_INSERT,					"Aeon.insert")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PERF_GET_RANGE,					"getRange")
DECLARE_CONST_STRING(PERF_INSERT_RANGE,					"insertRange")
DECLARE_CONST_STRING(PERF_MESSAGE,						"message")
DECLARE_CONST_STRING(PERF_SCAN_ROWS,					"scanRows")

DECLARE_CONST_STRING(STR_DIAGNOSTICS_HEADER,			"AeonDB Diagnostics\n")
DECLARE_CONST_STRING(STR_CANT_FIND_DIAGNOSTICS,			"Unable to find diagnostics file.")
DECLARE_CONST_STRING(STR_CANT_INIT_STORAGE,				"Unable to initialize diagnostics storage: %s")
DECLARE_CONST_STRING(STR_CANT_LOAD_DIAGNOSTICS,			"Unable to load diagnostics file %s: %s")
DECLARE_CONST_STRING(STR_CANT_START_ENGINE,				"Unable to start AeonDB diagnostics: %s")
DECLARE_CONST_STRING(STR_EXPECTED_GOT,					"Expected %s, got %s")
DECLARE_CONST_STRING(STR_EXPECTED_PAYLOAD,				"Expected payload %s, got %s")
DECLARE_CONST_STRING(STR_EXPECTED_RESULT_COUNT,			"Expected %d result elements, got %d")
DECLARE_CONST_STRING(STR_INVALID_PERF_STEP,				"Unknown performance step: %s")
DECLARE_CONST_STRING(STR_NO_REPLY,						"No reply from message %s.")
DECLARE_CONST_STRING(STR_PASS_COUNT,					"%d tests completed.\n0 failures.\n")
DECLARE_CONST_STRING(STR_PERF_NO_LABEL,					"unnamed performance step")
DECLARE_CONST_STRING(STR_STEP_FAILED,					"FAIL: %s / step %d: %s\n")

class CAeonDiagnosticsProcessCtx : public IArchonProcessCtx
	{
	public:
		struct SReply
			{
			CString sMsg;
			CDatum dPayload;
			SArchonMessage OriginalMsg;
			};

		CAeonDiagnosticsProcessCtx (void)
			{
			m_Transporter.Boot(this);
			}

		bool GetLastReply (SReply *retReply) const
			{
			if (m_Replies.GetCount() == 0)
				return false;

			if (retReply)
				*retReply = m_Replies[m_Replies.GetCount() - 1];

			return true;
			}

		void ResetReplies (void) { m_Replies.DeleteAll(); }

		//	IArchonProcessCtx
		virtual void AddEventRequest (const CString &sName, COSObject &Event, IArchonMessagePort *pPort, const CString &sMsg, DWORD dwTicket) override { }
		virtual void AddPort (const CString &sPort, IArchonMessagePort *pPort) override { m_Transporter.AddLocalPort(sPort, pPort); }
		virtual void AddVirtualPort (const CString &sPort, const CString &sAddress, DWORD dwFlags) override { m_Transporter.AddVirtualPort(sPort, sAddress, dwFlags); }
		virtual bool AuthenticateMachine (const CString &sMachineName, const CIPInteger &Key) override { return false; }
		virtual CMessagePort *Bind (const CString &sAddr) override { return m_Transporter.Bind(sAddr); }
		virtual CString GenerateAbsoluteAddress (const CString &sAddress) override { return m_Transporter.GenerateAbsoluteAddress(sAddress); }
		virtual CString GenerateAddress (const CString &sPort) override { return CMessageTransporter::GenerateAddress(sPort, MODULE_NAME, MACHINE_NAME); }
		virtual CString GenerateMachineAddress (const CString &sMachineName, const CString &sPort) override { return CMessageTransporter::GenerateAddress(sPort, MODULE_NAME, sMachineName); }
		virtual const CString &GetMachineName (void) const override { return MACHINE_NAME; }
		virtual CMnemosynthDb &GetMnemosynth (void) override { return m_MnemosynthDb; }
		virtual const CString &GetModuleName (void) const override { return MODULE_NAME; }
		virtual const SFileVersionInfo& GetModuleVersion (void) const override { return m_Version; }
		virtual CMessageTransporter &GetTransporter (void) override { return m_Transporter; }
		virtual void InitiateShutdown (void) override { }
		virtual bool IsCentralModule (void) override { return true; }
		virtual bool IsOnArcologyPrime () const override { return true; }
		virtual void Log (const CString &sMsg, const CString &sText) override
			{
			if (strEquals(sMsg, MSG_LOG_ERROR))
				printf("ERROR: %s\n", (LPCSTR)sText);
			}
		virtual void LogBlackBox (const CString &sText) override { }
		virtual CDatum MnemosynthRead (const CString &sCollection, const CString &sKey, SWatermark *retWatermark = NULL) const override { return CDatum(); }
		virtual void MnemosynthReadCollection (const CString &sCollection, TArray<CString> *retKeys, SWatermark *retWatermark = NULL) const override { if (retKeys) retKeys->DeleteAll(); }
		virtual bool MnemosynthWrite (const CString &sCollection, const CString &sKey, CDatum dValue, const SWatermark &Watermark = NULL_WATERMARK) override { return true; }
		virtual void OnMnemosynthDbModified (CDatum dLocalUpdates) override { }
		virtual bool ReadBlackBox (const CString &sFind, int iLines, TArray<CString> *retLines) override { if (retLines) retLines->DeleteAll(); return true; }
		virtual void ReportVolumeFailure (const CString &sFilespec, const CString &sOperation = NULL_STR) override
			{
			if (sOperation.IsEmpty())
				printf("Volume failure: %s\n", (LPCSTR)sFilespec);
			else
				printf("Volume failure: %s [%s]\n", (LPCSTR)sFilespec, (LPCSTR)sOperation);
			}
		virtual bool SendMessage (const CString &sAddress, const SArchonMessage &Msg) override
			{
			CMessagePort *pPort = Bind(sAddress);
			return (pPort ? pPort->SendMessage(Msg) : true);
			}
		virtual bool SendMessageCommand (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, DWORD dwTicket, CDatum dPayload) override { return true; }
		virtual void SendMessageReply (const CString &sReplyMsg, CDatum dPayload, const SArchonMessage &OriginalMsg) override
			{
			SReply *pReply = m_Replies.Insert();
			pReply->sMsg = sReplyMsg;
			pReply->dPayload = dPayload;
			pReply->OriginalMsg = OriginalMsg;
			}
		virtual void TranspaceDownload (const CString &sAddress, const CString &sReplyAddr, DWORD dwTicket, CDatum dDownloadDesc, const CHexeSecurityCtx *pSecurityCtx) override { }

	private:
		TArray<SReply> m_Replies;
		CMnemosynthDb m_MnemosynthDb;
		CMessageTransporter m_Transporter;
		SFileVersionInfo m_Version;
	};

static CString StripTrailingPathSeparator (const CString &sPath)
	{
	const char *pPath = sPath.GetParsePointer();
	int iLength = sPath.GetLength();

	while (iLength > 0
			&& (pPath[iLength - 1] == '\\' || pPath[iLength - 1] == '/')
			&& !(iLength == 3 && pPath[1] == ':'))
		iLength--;

	return (iLength == sPath.GetLength() ? sPath : CString(pPath, iLength));
	}

static void AddDiagnosticsCandidates (const CString &sRoot, TArray<CString> *retCandidates)
	{
	CString sPath = StripTrailingPathSeparator(sRoot);

	for (int i = 0; i < 5 && !sPath.IsEmpty(); i++)
		{
		retCandidates->Insert(fileAppend(fileAppend(sPath, FOLDER_DIAGNOSTICS), FILE_DIAGNOSTICS));
		retCandidates->Insert(fileAppend(fileAppend(fileAppend(sPath, FOLDER_AEONDB), FOLDER_DIAGNOSTICS), FILE_DIAGNOSTICS));

		CString sParent = StripTrailingPathSeparator(fileGetPath(sPath));
		if (strEquals(sParent, sPath))
			break;

		sPath = sParent;
		}
	}

static bool FindDiagnosticsFile (CString *retsFilespec)
	{
	TArray<CString> Candidates;

	AddDiagnosticsCandidates(fileGetWorkingDirectory(), &Candidates);
	AddDiagnosticsCandidates(fileGetPath(fileGetExecutableFilespec()), &Candidates);

	for (int i = 0; i < Candidates.GetCount(); i++)
		if (fileExists(Candidates[i]))
			{
			if (retsFilespec)
				*retsFilespec = Candidates[i];

			return true;
			}

	return false;
	}

static CString GetDiagnosticsStorageRoot (void)
	{
	return fileAppend(fileAppend(fileGetTempPath(), FOLDER_GRIDWHALE), FOLDER_TEST_ROOT);
	}

static bool InitStorageRoot (const CString &sStorageRoot, CString *retsError)
	{
	if (fileExists(sStorageRoot) && !filePathDelete(sStorageRoot, FPD_FLAG_RECURSIVE))
		{
		if (retsError)
			*retsError = strPattern("Unable to delete previous diagnostics storage: %s", sStorageRoot);
		return false;
		}

	if (!filePathCreate(sStorageRoot))
		{
		if (retsError)
			*retsError = strPattern("Unable to create diagnostics storage: %s", sStorageRoot);
		return false;
		}

	return true;
	}

static bool GetExpectedPayload (CDatum dStep, CDatum *retdPayload)
	{
	return dStep.FindElement(FIELD_EXPECT_PAYLOAD, retdPayload);
	}

static bool SendEngineMessage (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, const CString &sMsg, CDatum dPayload, int iTicket, CAeonDiagnosticsProcessCtx::SReply *retReply, CString *retsError)
	{
	SArchonMessage Msg;
	Msg.sMsg = sMsg;
	Msg.sReplyAddr = ADDRESS_DIAGNOSTICS;
	Msg.dwTicket = iTicket;
	Msg.dPayload = dPayload;

	ProcessCtx.ResetReplies();

	CArchonMessageList List;
	List.Insert(Msg);
	Engine.ProcessMessages(List);

	if (!ProcessCtx.GetLastReply(retReply))
		{
		if (retsError)
			*retsError = strPattern(STR_NO_REPLY, sMsg);
		return false;
		}

	return true;
	}

static CDatum CreateKey (const CString &sPrefix, int iIndex)
	{
	CDatum dKey(CDatum::typeArray);
	dKey.Append(CDatum(strPattern("%s%08d", sPrefix, iIndex)));
	return dKey;
	}

static CString CreateValueBytes (int iBytes)
	{
	if (iBytes <= 0)
		return NULL_STR;

	CStringBuffer Buffer;
	Buffer.WriteChar('x', iBytes);
	return CString(Buffer);
	}

static CDatum CreateRowData (int iIndex, const CString &sValue)
	{
	CComplexStruct *pData = new CComplexStruct;
	pData->SetElement(STR_CAEON_DIAGNOSTICS_INDEX, CDatum(iIndex));
	pData->SetElement(STR_CAEON_DIAGNOSTICS_GROUP, CDatum(strPattern("g%03d", iIndex % 100)));
	if (!sValue.IsEmpty())
		pData->SetElement(STR_CAEON_DIAGNOSTICS_VALUE, CDatum(sValue));

	return CDatum(pData);
	}

static CDatum CreateTableAndView (CDatum dStep)
	{
	CString sTable = dStep.GetElement(FIELD_TABLE).AsString();
	CString sView = dStep.GetElement(FIELD_VIEW).AsString();

	if (sView.IsEmpty())
		return CDatum(sTable);

	CDatum dTableAndView(CDatum::typeArray);
	dTableAndView.Append(CDatum(sTable));
	dTableAndView.Append(CDatum(sView));
	return dTableAndView;
	}

static void PrintPerfResult (const CString &sLabel, int iOps, DWORDLONG dwElapsed)
	{
	double rOpsPerSec = (dwElapsed > 0 ? (1000.0 * (double)iOps / (double)dwElapsed) : 0.0);
	printf("PERF: %s: %d ops in %I64u ms (%.1f ops/sec)\n", (LPCSTR)sLabel, iOps, dwElapsed, rOpsPerSec);
	}

static bool ExecuteStep (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, CDatum dStep, int iStep, CString *retsError)
	{
	CString sMsg = dStep.GetElement(FIELD_MSG).AsString();
	if (sMsg.IsEmpty())
		{
		if (retsError)
			*retsError = STR_CAEON_DIAGNOSTICS_MISSING_MSG_FIELD;
		return false;
		}

	CString sExpectedMsg = dStep.GetElement(FIELD_EXPECT_MSG).AsString();
	if (sExpectedMsg.IsEmpty())
		sExpectedMsg = MSG_OK;

	CAeonDiagnosticsProcessCtx::SReply Reply;
	if (!SendEngineMessage(Engine, ProcessCtx, sMsg, dStep.GetElement(FIELD_PAYLOAD), iStep + 1, &Reply, retsError))
		return false;

	if (!strEquals(Reply.sMsg, sExpectedMsg))
		{
		if (retsError)
			*retsError = strPattern(STR_EXPECTED_GOT, sExpectedMsg, Reply.sMsg);
		return false;
		}

	CDatum dExpectedPayload;
	if (dStep.GetElement(FIELD_EXPECT_PAYLOAD_NIL).AsBool() && !Reply.dPayload.IsNil())
		{
		if (retsError)
			*retsError = strPattern(STR_EXPECTED_PAYLOAD, STR_CAEON_DIAGNOSTICS_NIL, Reply.dPayload.AsString());
		return false;
		}

	if (GetExpectedPayload(dStep, &dExpectedPayload) && !Reply.dPayload.OpIsEqual(dExpectedPayload))
		{
		if (retsError)
			*retsError = strPattern(STR_EXPECTED_PAYLOAD, dExpectedPayload.AsString(), Reply.dPayload.AsString());
		return false;
		}

	return true;
	}

static bool ExecutePerfInsertRange (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, CDatum dStep, const CString &sLabel, CString *retsError)
	{
	CString sTable = dStep.GetElement(FIELD_TABLE).AsString();
	CString sKeyPrefix = dStep.GetElement(FIELD_KEY_PREFIX).AsString();
	if (sKeyPrefix.IsEmpty())
		sKeyPrefix = STR_CAEON_DIAGNOSTICS_ROW;

	int iStart = (int)dStep.GetElement(FIELD_START);
	int iCount = (int)dStep.GetElement(FIELD_COUNT);
	int iValueBytes = (int)dStep.GetElement(FIELD_VALUE_BYTES);
	CString sValue = CreateValueBytes(iValueBytes);

	DWORDLONG dwStart = ::sysGetTickCount64();
	for (int i = 0; i < iCount; i++)
		{
		CDatum dPayload(CDatum::typeArray);
		dPayload.Append(CDatum(sTable));
		dPayload.Append(CreateKey(sKeyPrefix, iStart + i));
		dPayload.Append(CreateRowData(iStart + i, sValue));

		CAeonDiagnosticsProcessCtx::SReply Reply;
		if (!SendEngineMessage(Engine, ProcessCtx, MSG_AEON_INSERT, dPayload, i + 1, &Reply, retsError))
			return false;

		if (!strEquals(Reply.sMsg, MSG_OK))
			{
			if (retsError)
				*retsError = strPattern(STR_EXPECTED_GOT, MSG_OK, Reply.sMsg);
			return false;
			}
		}

	DWORDLONG dwElapsed = ::sysGetTicksElapsed(dwStart);
	PrintPerfResult(sLabel, iCount, dwElapsed);
	return true;
	}

static bool ExecutePerfGetRange (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, CDatum dStep, const CString &sLabel, CString *retsError)
	{
	CString sTable = dStep.GetElement(FIELD_TABLE).AsString();
	CString sKeyPrefix = dStep.GetElement(FIELD_KEY_PREFIX).AsString();
	if (sKeyPrefix.IsEmpty())
		sKeyPrefix = STR_CAEON_DIAGNOSTICS_ROW;

	int iStart = (int)dStep.GetElement(FIELD_START);
	int iCount = (int)dStep.GetElement(FIELD_COUNT);

	DWORDLONG dwStart = ::sysGetTickCount64();
	for (int i = 0; i < iCount; i++)
		{
		CDatum dPayload(CDatum::typeArray);
		dPayload.Append(CDatum(sTable));
		dPayload.Append(CreateKey(sKeyPrefix, iStart + i));

		CAeonDiagnosticsProcessCtx::SReply Reply;
		if (!SendEngineMessage(Engine, ProcessCtx, MSG_AEON_GET_VALUE, dPayload, i + 1, &Reply, retsError))
			return false;

		if (!strEquals(Reply.sMsg, MSG_REPLY_DATA))
			{
			if (retsError)
				*retsError = strPattern(STR_EXPECTED_GOT, MSG_REPLY_DATA, Reply.sMsg);
			return false;
			}
		else if (Reply.dPayload.IsNil())
			{
			if (retsError)
				*retsError = strPattern("Missing row for key %s.", CreateKey(sKeyPrefix, iStart + i).AsString());
			return false;
			}
		}

	DWORDLONG dwElapsed = ::sysGetTicksElapsed(dwStart);
	PrintPerfResult(sLabel, iCount, dwElapsed);
	return true;
	}

static bool ExecutePerfMessage (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, CDatum dStep, const CString &sLabel, int iStep, CString *retsError)
	{
	CString sMsg = dStep.GetElement(FIELD_MSG).AsString();
	if (sMsg.IsEmpty())
		{
		if (retsError)
			*retsError = STR_CAEON_DIAGNOSTICS_MISSING_MSG_FIELD;
		return false;
		}

	CString sExpectedMsg = dStep.GetElement(FIELD_EXPECT_MSG).AsString();
	if (sExpectedMsg.IsEmpty())
		sExpectedMsg = MSG_OK;

	DWORDLONG dwStart = ::sysGetTickCount64();

	CAeonDiagnosticsProcessCtx::SReply Reply;
	if (!SendEngineMessage(Engine, ProcessCtx, sMsg, dStep.GetElement(FIELD_PAYLOAD), iStep + 1, &Reply, retsError))
		return false;

	DWORDLONG dwElapsed = ::sysGetTicksElapsed(dwStart);

	if (!strEquals(Reply.sMsg, sExpectedMsg))
		{
		if (retsError)
			*retsError = strPattern(STR_EXPECTED_GOT, sExpectedMsg, Reply.sMsg);
		return false;
		}

	PrintPerfResult(sLabel, 1, dwElapsed);
	return true;
	}

static bool ExecutePerfScanRows (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, CDatum dStep, const CString &sLabel, int iStep, CString *retsError)
	{
	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(CreateTableAndView(dStep));

	CDatum dKey = dStep.GetElement(STR_CAEON_DIAGNOSTICS_KEY);
	if (!dKey.IsNil())
		dPayload.Append(dKey);

	CDatum dLimits = dStep.GetElement(STR_CAEON_DIAGNOSTICS_LIMITS);
	if (!dLimits.IsNil())
		{
		if (dKey.IsNil())
			dPayload.Append(CDatum());
		dPayload.Append(dLimits);
		}

	CDatum dOptions = dStep.GetElement(STR_CAEON_DIAGNOSTICS_OPTIONS);
	if (!dOptions.IsNil())
		{
		if (dKey.IsNil())
			dPayload.Append(CDatum());
		if (dLimits.IsNil())
			dPayload.Append(CDatum());
		dPayload.Append(dOptions);
		}

	int iRepeat = (int)dStep.GetElement(FIELD_REPEAT);
	if (iRepeat <= 0)
		iRepeat = 1;

	DWORDLONG dwStart = ::sysGetTickCount64();

	for (int i = 0; i < iRepeat; i++)
		{
		CAeonDiagnosticsProcessCtx::SReply Reply;
		if (!SendEngineMessage(Engine, ProcessCtx, MSG_AEON_GET_ROWS, dPayload, iStep + 1, &Reply, retsError))
			return false;

		if (!strEquals(Reply.sMsg, MSG_REPLY_DATA))
			{
			if (retsError)
				*retsError = strPattern(STR_EXPECTED_GOT, MSG_REPLY_DATA, Reply.sMsg);
			return false;
			}

		int iExpectedResultCount = (int)dStep.GetElement(FIELD_EXPECT_RESULT_COUNT);
		if (iExpectedResultCount > 0 && Reply.dPayload.GetCount() != iExpectedResultCount)
			{
			if (retsError)
				*retsError = strPattern(STR_EXPECTED_RESULT_COUNT, iExpectedResultCount, Reply.dPayload.GetCount());
			return false;
			}
		}

	DWORDLONG dwElapsed = ::sysGetTicksElapsed(dwStart);

	PrintPerfResult(sLabel, Max(1, (int)dStep.GetElement(FIELD_COUNT)) * iRepeat, dwElapsed);
	return true;
	}

static bool ExecutePerformanceStep (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, CDatum dStep, int iStep, CString *retsError)
	{
	CString sPerf = dStep.GetElement(FIELD_PERF).AsString();
	CString sLabel = dStep.GetElement(FIELD_LABEL).AsString();
	if (sLabel.IsEmpty())
		sLabel = STR_PERF_NO_LABEL;

	if (strEquals(sPerf, PERF_INSERT_RANGE))
		return ExecutePerfInsertRange(Engine, ProcessCtx, dStep, sLabel, retsError);
	else if (strEquals(sPerf, PERF_GET_RANGE))
		return ExecutePerfGetRange(Engine, ProcessCtx, dStep, sLabel, retsError);
	else if (strEquals(sPerf, PERF_MESSAGE))
		return ExecutePerfMessage(Engine, ProcessCtx, dStep, sLabel, iStep, retsError);
	else if (strEquals(sPerf, PERF_SCAN_ROWS))
		return ExecutePerfScanRows(Engine, ProcessCtx, dStep, sLabel, iStep, retsError);
	else
		{
		if (retsError)
			*retsError = strPattern(STR_INVALID_PERF_STEP, sPerf);
		return false;
		}
	}

static bool ExecuteTest (CAeonEngine &Engine, CAeonDiagnosticsProcessCtx &ProcessCtx, CDatum dTest, int *retiTestCount, CString *retsError)
	{
	CString sName = dTest.GetElement(FIELD_NAME).AsString();
	if (sName.IsEmpty())
		sName = STR_CAEON_DIAGNOSTICS_UNNAMED;

	CDatum dSteps = dTest.GetElement(FIELD_STEPS);
	for (int i = 0; i < dSteps.GetCount(); i++)
		{
		CDatum dStep = dSteps.GetElement(i);
		bool bSuccess;
		if (!dStep.GetElement(FIELD_PERF).IsNil())
			bSuccess = ExecutePerformanceStep(Engine, ProcessCtx, dStep, i, retsError);
		else
			bSuccess = ExecuteStep(Engine, ProcessCtx, dStep, i, retsError);

		if (!bSuccess)
			{
			printf((LPCSTR)STR_STEP_FAILED, (LPCSTR)sName, i + 1, (LPCSTR)*retsError);
			return false;
			}

		if (retiTestCount)
			(*retiTestCount)++;
		}

	return true;
	}

int CAeonDiagnostics::RunDiagnostics (void)
	{
	printf((LPCSTR)STR_DIAGNOSTICS_HEADER);

	CString sDiagnosticsFilespec;
	if (!FindDiagnosticsFile(&sDiagnosticsFilespec))
		{
		printf("ERROR: %s\n", (LPCSTR)STR_CANT_FIND_DIAGNOSTICS);
		return 1;
		}

	CString sStorageRoot = GetDiagnosticsStorageRoot();
	CString sError;
	if (!InitStorageRoot(sStorageRoot, &sError))
		{
		printf("ERROR: ");
		printf((LPCSTR)STR_CANT_INIT_STORAGE, (LPCSTR)sError);
		printf("\n");
		return 1;
		}

	CAeonDiagnosticsProcessCtx ProcessCtx;
	CAeonEngine Engine;
	Engine.Boot(&ProcessCtx, 1);

	if (!Engine.InitDiagnostics(sStorageRoot, &sError))
		{
		printf("ERROR: ");
		printf((LPCSTR)STR_CANT_START_ENGINE, (LPCSTR)sError);
		printf("\n");
		return 1;
		}

	CDatum dTests;
	if (!CDatum::CreateFromFile(sDiagnosticsFilespec, CDatum::EFormat::AEONScript, &dTests, &sError))
		{
		printf("ERROR: ");
		printf((LPCSTR)STR_CANT_LOAD_DIAGNOSTICS, (LPCSTR)sDiagnosticsFilespec, (LPCSTR)sError);
		printf("\n");
		return 1;
		}

	int iTestCount = 0;
	for (int i = 0; i < dTests.GetCount(); i++)
		if (!ExecuteTest(Engine, ProcessCtx, dTests.GetElement(i), &iTestCount, &sError))
			return 1;

	printf((LPCSTR)STR_PASS_COUNT, iTestCount);
	return 0;
	}
