//	LogSearch.cpp
//
//	Log search messages
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_AFTER,						"after")
DECLARE_CONST_STRING(FIELD_BEFORE,						"before")
DECLARE_CONST_STRING(FIELD_CASE,						"case")
DECLARE_CONST_STRING(FIELD_COMPLETE,					"complete")
DECLARE_CONST_STRING(FIELD_ID,							"id")
DECLARE_CONST_STRING(FIELD_LINES_SEARCHED,				"linesSearched")
DECLARE_CONST_STRING(FIELD_MAX_RESULTS,					"maxResults")
DECLARE_CONST_STRING(FIELD_RESULTS,						"results")

DECLARE_CONST_STRING(MSG_DRHOUSE_PROCESS_LOG_SEARCH,	"DrHouse.processLogSearch")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(ERR_BAD_DATE,						"Unable to parse date: %s.")
DECLARE_CONST_STRING(ERR_BUSY,							"Unable to search the log files at this time.")
DECLARE_CONST_STRING(ERR_WTF,							"Non sequitur. Your facts are uncoordinated.")

void CDrHouseEngine::MsgCreateLogSearch (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgCreateLogSearch
//
//	Diagnostics.createLogSearch {search} [{options}]
//
//	options:
//
//		maxResults: max number of results
//		after: only results on or after this date
//		before: only results on or before this data

	{
	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get the search

	CString sSearch = Msg.dPayload.GetElement(0).AsString();

	//	Get options

	CDatum dOptions = Msg.dPayload.GetElement(1);
	CBlackBoxProcessor::SOptions Options;

	//	Start with max results

	CDatum dMaxResult = dOptions.GetElement(FIELD_MAX_RESULTS);
	if (!dMaxResult.IsNil())
		Options.iMaxLines = (int)dMaxResult;

	//	After

	CDatum dAfter = dOptions.GetElement(FIELD_AFTER);
	if (dAfter.GetBasicType() == CDatum::typeInteger32)
		Options.After = timeSubtractTime(CDateTime(CDateTime::Now), CTimeSpan(Max(0, (int)dAfter), 0));
	else if (dAfter.GetBasicType() == CDatum::typeString)
		{
		if (!CDateTime::Parse(CDateTime::formatAuto, dAfter.AsStringView(), &Options.After))
			{
			SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_BAD_DATE, dAfter.AsString()), Msg);
			return;
			}
		}

	//	Before

	CDatum dBefore = dOptions.GetElement(FIELD_BEFORE);
	if (dBefore.GetBasicType() == CDatum::typeInteger32)
		Options.After = timeSubtractTime(CDateTime(CDateTime::Now), CTimeSpan(Max(0, (int)dBefore), 0));
	else if (dBefore.GetBasicType() == CDatum::typeString)
		{
		if (!CDateTime::Parse(CDateTime::formatAuto, dBefore.AsStringView(), &Options.Before))
			{
			SendMessageReply(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_BAD_DATE, dBefore.AsString()), Msg);
			return;
			}
		}

	//	Case-sensitive

	CDatum dCase = dOptions.GetElement(FIELD_CASE);
	if (!dCase.IsNil())
		Options.bNoCase = false;

	//	Create the search (we let the search engine validate parameters, 
	//	including the search string).

	DWORD dwID;
	CString sError;
	if (!m_BlackBoxProcessor.CreateSession(sSearch, Options, &dwID, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Send ourselves a message so that we process the session. We it as a 
	//	separate message because it might take longer than the timeout.
	//	Clients are responsible for polling until the search is done.

	SArchonMessage ProcessMsg;
	ProcessMsg.sMsg = MSG_DRHOUSE_PROCESS_LOG_SEARCH;
	if (!SendMessage(ProcessMsg))
		{
		//	Should never happen
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_BUSY, Msg);
		return;
		}

	//	Return the search ID

	SendMessageReply(MSG_REPLY_DATA, CDatum(dwID), Msg);
	}

void CDrHouseEngine::MsgGetLogSearch (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetLogSearch
//
//	Diagnostics.getLogSearch {searchID}
//
//	We return a structure with the following fields:
//
//	complete: True if the results are complete
//	linesSearched: The number of lines that we searched.
//	results: An array of lines.

	{
	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	//	Get the ID

	DWORD dwID = Msg.dPayload.GetElement(0);

	//	Get the results

	DWORD dwLinesSearched;
	CDatum dResult;
	CString sError;
	if (!m_BlackBoxProcessor.GetResults(dwID, &dwLinesSearched, &dResult, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
		return;
		}

	//	Compose a response

	CDatum dResponse(CDatum::typeStruct);
	if (!dResult.IsNil())
		dResponse.SetElement(FIELD_COMPLETE, CDatum(true));

	dResponse.SetElement(FIELD_ID, dwID);
	dResponse.SetElement(FIELD_LINES_SEARCHED, dwLinesSearched);
	
	//	If the result is just the constant True, then it means that we found no
	//	results.

	if (dResult.GetBasicType() == CDatum::typeTrue)
		dResponse.SetElement(FIELD_RESULTS, CDatum());
	else
		dResponse.SetElement(FIELD_RESULTS, dResult);

	//	Repond

	SendMessageReply(MSG_REPLY_DATA, dResponse, Msg);
	}

void CDrHouseEngine::MsgProcessLogSearch (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgProcessLogSearch
//
//	DrHouse.processLogSearch

	{
	//	This message should only come from internal code, so we must not have
	//	a security ctx.

	if (pSecurityCtx)
		{
		//	Should never happen
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_WTF, Msg);
		return;
		}
		
	//	Process the next session, if there is one. If we did some work, then
	//	resend the message in case there is another session.

	if (m_BlackBoxProcessor.Process() == CBlackBoxProcessor::resultContinue)
		{
		SArchonMessage ProcessMsg;
		ProcessMsg.sMsg = MSG_DRHOUSE_PROCESS_LOG_SEARCH;
		SendMessage(ProcessMsg);
		}

	//	Done
	}
