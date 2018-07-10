//	CHexeMarkupEvaluator.cpp
//
//	CHexeMarkupEvaluator class
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(HEADER_LOCATION,					"location")

DECLARE_CONST_STRING(LIBRARY_HYPERION,					"hyperion")
DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")
DECLARE_CONST_STRING(LIBRARY_SESSION_CTX,				"sessionCtx")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_BODY_BUILDER,	"sessionHTTPBodyBuilder")
DECLARE_CONST_STRING(LIBRARY_SESSION_HTTP_REQUEST,		"sessionHTTPRequest")

DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html")

DECLARE_CONST_STRING(PI_ELSE,							"else")
DECLARE_CONST_STRING(PI_ENDIF,							"endif")
DECLARE_CONST_STRING(PI_EVAL,							"eval")
DECLARE_CONST_STRING(PI_FILE,							"file")
DECLARE_CONST_STRING(PI_HEADER,							"header")
DECLARE_CONST_STRING(PI_IF,								"if")
DECLARE_CONST_STRING(PI_REDIRECT,						"redirect")
DECLARE_CONST_STRING(PI_XML,							"xml")

DECLARE_CONST_STRING(STR_OK,							"OK")
DECLARE_CONST_STRING(STR_MOVED_PERMANENTLY,				"Moved Permanently")

DECLARE_CONST_STRING(ERR_PROCESSING_HEADER,				"ERROR processing header directive: %s.")
DECLARE_CONST_STRING(ERR_NO_HEADER_FIELD,				"ERROR: Unable to parse header field: %s.")

CHexeMarkupEvaluator::CHexeMarkupEvaluator (void) : m_pProcess(NULL)

//	CHexeMarkupEvaluator constructor

	{
	}

CHexeMarkupEvaluator::~CHexeMarkupEvaluator (void)

//	CHexeMarkupEvaluator destructor

	{
	CleanUp();
	}

void CHexeMarkupEvaluator::AddHeader (const CString &sField, CDatum dValue)

//	AddHeader
//
//	Adds the header

	{
	CHTTPMessage::SHeader *pNewHeader;

	switch (dValue.GetBasicType())
		{
		case CDatum::typeString:
			pNewHeader = m_Headers.Insert();
			pNewHeader->sField = sField;
			pNewHeader->sValue = dValue;
			break;

		default:
			pNewHeader = m_Headers.Insert();
			pNewHeader->sField = sField;
			pNewHeader->sValue = dValue.AsString();
		}
	}

void CHexeMarkupEvaluator::CleanUp (void)

//	CleanUp
//
//	Reset all member data

	{
	if (m_pProcess)
		{
		delete m_pProcess;
		m_pProcess = NULL;
		}

	m_dwResponseCode = http_OK;
	m_sResponseMsg = STR_OK;
	m_Headers.DeleteAll();
	m_Output.SetLength(0);
	}

bool CHexeMarkupEvaluator::ComposeResponse (SHTTPRequestCtx &Ctx)

//	ComposeResponse
//
//	We're done with the evaluation and the result is in m_Output;
//	Compose a response message.

	{
	int i;

	IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);

	//	NOTE: We default to HTML, but if the user added a Content-Type header 
	//	then we'll use that instead.

	pBody->DecodeFromBuffer(MEDIA_TYPE_HTML, m_Output);

	//	Prepare response

	Ctx.iStatus = pstatResponseReady;
	Ctx.Response.InitResponse(m_dwResponseCode, m_sResponseMsg);
	Ctx.Response.SetBody(pBody);

	//	Additional headers

	for (i = 0; i < m_Headers.GetCount(); i++)
		Ctx.Response.AddHeader(m_Headers[i].sField, m_Headers[i].sValue);

	return true;
	}

bool CHexeMarkupEvaluator::EvalContinues (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult)

//	EvalContinues
//
//	Continues with evaluation after an RPC call

	{
	CDatum dResult;
	CHexeProcess::ERunCodes iRun = m_pProcess->RunContinues(CSimpleEngine::MessageToHexeResult(RPCResult), &dResult);

	//	Output the result of the last processing instruction. This can return
	//	FALSE if we need to return (e.g., we have a file to load).

	if (!ProcessResult(Ctx, iRun, dResult))
		return false;

	//	Now continue parsing

	return ParseUntilRPC(Ctx);
	}

bool CHexeMarkupEvaluator::EvalInit (SHTTPRequestCtx &Ctx,
									 const CHexeProcess &ProcessTemplate, 
									 const CHexeSecurityCtx &SecurityCtx,
									 const TArray<CDatum> &HexeDefinitions,
									 CDatum dFileDesc,
									 CDatum dData)

//	EvalInit
//
//	Initializes evaluation. Return TRUE if the message is ready; FALSE if we 
//	need to wait for an RPC result.

	{
	CleanUp();

	//	We need to keep the input data (otherwise it might get freed out from
	//	under us, and we keep a pointer to it in the parser).

	m_dFileDesc = dFileDesc;
	m_dData = dData;

	//	Initialize the process that we'll use for evaluation

	m_pProcess = new CHexeProcess;

	//	Clone from the template

	m_pProcess->InitFrom(ProcessTemplate);

	//	Set some context

	m_pProcess->SetLibraryCtx(LIBRARY_HYPERION, Ctx.pSession->GetEngine());
	m_pProcess->SetLibraryCtx(LIBRARY_SESSION, Ctx.pSession);
	m_pProcess->SetLibraryCtx(LIBRARY_SESSION_CTX, &Ctx);
	m_pProcess->SetLibraryCtx(LIBRARY_SESSION_HTTP_BODY_BUILDER, Ctx.pBodyBuilder);
	m_pProcess->SetLibraryCtx(LIBRARY_SESSION_HTTP_REQUEST, &Ctx.Request);
	m_pProcess->SetSecurityCtx(SecurityCtx);

	//	Definitions

	CString sError;
	if (!m_pProcess->LoadHexeDefinitions(HexeDefinitions, &sError))
		{
		m_Output.Write(strPattern("<!DOCTYPE html><html lang='en'><body><h1>%s</h1></body></html>", htmlWriteText(sError)));
		return ComposeResponse(Ctx);
		}

	//	Initialize parsing

	const CString &sData = dData;
	m_Parser.Init(sData.GetParsePointer(), sData.GetLength());

	//	Initialize state

	m_iProcessing = tagNone;
	m_iIfLevel = 0;
	m_iIfLevelEnd = 0;

	//	Parse until we need to process an RPC message

	return ParseUntilRPC(Ctx);
	}

void CHexeMarkupEvaluator::OutputDatum (CDatum dValue)

//	OutputDatum
//
//	Outputs a datum to the resulting HTML page. NOTE: We expect the values to be
//	HTML compatible (i.e., caller is responsible for escaping).

	{
	int i;

	if (dValue.GetBasicType() == CDatum::typeArray)
		{
		for (i = 0; i < dValue.GetCount(); i++)
			OutputDatum(dValue.GetElement(i));
		}
	else
		{
		m_Output.Write(dValue.AsString());
		}
	}

void CHexeMarkupEvaluator::OutputResult (CDatum dResult)

//	OutputResult
//
//	Outputs the result of a processing instruction into the result stream.

	{
	OutputDatum(dResult);
	}

bool CHexeMarkupEvaluator::ParseUntilRPC (SHTTPRequestCtx &Ctx)

//	ParseUntilRPC
//
//	Parses the input until we need to process and RPC request. Returns FALSE if
//	we need RPC; TRUE otherwise.

	{
	while (true)
		{
		CString sKey;
		CString sValue;
		CHexeMarkupParser::Tokens iToken = m_Parser.ParseToken(&sKey, &sValue);

		//	If error, emit token and return

		if (iToken == CHexeMarkupParser::tkError)
			{
			m_Output.Write(sValue);
			return ComposeResponse(Ctx);
			}

		//	Are we done? Then compose the response.

		else if (iToken == CHexeMarkupParser::tkEoS)
			return ComposeResponse(Ctx);

		//	If we're excluding content then continue

		else if (m_iIfLevelEnd > 0)
			{
			if (iToken == CHexeMarkupParser::tkPI)
				{
				if (strEquals(sKey, PI_ELSE))
					{
					if (m_iIfLevel == m_iIfLevelEnd)
						m_iIfLevelEnd = 0;
					}
				else if (strEquals(sKey, PI_ENDIF))
					{
					if (m_iIfLevel == m_iIfLevelEnd)
						m_iIfLevelEnd = 0;

					m_iIfLevel--;
					}
				else if (strEquals(sKey, PI_IF))
					m_iIfLevel++;
				}

			//	Otherwise, continue parsing and ignoring.
			}

		//	If this is a processing instruction, then handle it.

		else if (iToken == CHexeMarkupParser::tkPI)
			{
			//	Interpret the instruction

			if (!ProcessDirective(Ctx, sKey, sValue))
				return false;

			//	Continue parsing
			}

		//	Otherwise we just emit the token to the output

		else
			m_Output.Write(sValue);
		}
	}

bool CHexeMarkupEvaluator::ProcessDirective (SHTTPRequestCtx &Ctx, const CString &sDirective, const CString &sValue)

//	ProcessDirective
//
//	Process the given directive. Returns TRUE if processing should continue;
//	FALSE if we need RPC.

	{
	//	Interpret the instruction

	if (strEquals(sDirective, PI_EVAL))
		{
		if (!ProcessEval(Ctx, tagEval, sValue))
			return false;
		}

	//	If statement. This may be nested.

	else if (strEquals(sDirective, PI_IF))
		{
		if (!ProcessEval(Ctx, tagIf, sValue))
			return false;
		}

	//	Handle endif

	else if (strEquals(sDirective, PI_ELSE))
		{
		//	Error if no matching if-block

		if (m_iIfLevel == 0)
			{
			m_Output.Write(CString("Mismatched <?else?>"));
			return true;
			}

		m_iIfLevelEnd = m_iIfLevel;
		}

	else if (strEquals(sDirective, PI_ENDIF))
		{
		//	Error if no matching if-block

		if (m_iIfLevel == 0)
			{
			m_Output.Write(CString("Mismatched <?endif?>"));
			return true;
			}

		m_iIfLevel--;
		}

	else if (strEquals(sDirective, PI_FILE))
		{
		if (!ProcessEval(Ctx, tagFile, sValue))
			return false;
		}

	else if (strEquals(sDirective, PI_HEADER))
		{
		if (!ProcessEval(Ctx, tagHeader, sValue))
			return false;
		}

	else if (strEquals(sDirective, PI_REDIRECT))
		{
		if (!ProcessEval(Ctx, tagRedirect, sValue))
			return false;
		}

	else if (strEquals(sDirective, PI_XML))
		m_Output.Write(strPattern("<?xml %s ?>", sValue));
	else
		m_Output.Write(strPattern("<span>Invalid processing instruction: %s.</span>", sDirective));

	//	Done

	return true;
	}

bool CHexeMarkupEvaluator::ProcessEval (SHTTPRequestCtx &Ctx, TagTypes iDirective, const CString &sCode)

//	ProcessEval
//
//	Process an evaluation.

	{
	//	Remember what we're processing in case we need RPC.

	m_iProcessing = iDirective;

	//	Compile the body into a function call

	CDatum dCode;
	CString sError;
	if (!CHexeDocument::ParseLispExpression(sCode, &dCode, &sError))
		{
		m_Output.Write(strPattern("ERROR: %s", sError));
		return true;
		}

	//	Run

	CDatum dResult;
	CHexeProcess::ERunCodes iRun = m_pProcess->Run(dCode, &dResult);

	//	Process the result

	if (!ProcessResult(Ctx, iRun, dResult))
		return false;

	//	Done

	return true;
	}

bool CHexeMarkupEvaluator::ProcessHeader (SHTTPRequestCtx &Ctx, CDatum dResult)

//	ProcessHeader
//
//	Outputs the given header

	{
	//	Check for error

	if (dResult.IsError())
		{
		m_Output.Write(strPattern(ERR_PROCESSING_HEADER, dResult.AsString()));
		return true;
		}

	//	Processing depends on result type

	switch (dResult.GetBasicType())
		{
		case CDatum::typeNil:
			return true;

		//	If this is a string or anything else, we expect both field and value
		//	are in the same string and we need to parse it.

		default:
			{
			CString sData = dResult.AsString();

			//	Parse into field and value

			char *pPos = sData.GetParsePointer();
			while (strIsWhitespace(pPos))
				pPos++;

			//	Look for the field name

			char *pStart = pPos;
			while (*pPos != ':' && *pPos != '\0')
				pPos++;

			CString sField(pStart, pPos - pStart);
			if (sField.IsEmpty())
				{
				m_Output.Write(strPattern(ERR_NO_HEADER_FIELD, sData));
				return true;
				}

			//	Look for the value

			CString sValue;
			if (*pPos == ':')
				{
				pPos++;
				while (strIsWhitespace(pPos))
					pPos++;

				sValue = CString(pPos);
				}

			//	Done

			CHTTPMessage::SHeader *pNewHeader = m_Headers.Insert();
			pNewHeader->sField = sField;
			pNewHeader->sValue = sValue;
			}
		}

	return true;
	}

bool CHexeMarkupEvaluator::ProcessResult (SHTTPRequestCtx &Ctx, CHexeProcess::ERunCodes iRun, CDatum dResult)

//	ProcessResult
//
//	Process the result of an evaluation. Returns TRUE if processing should 
//	continue; FALSE if we need RPC or are done processing.

	{
	//	If we have more async calls then return

	if (iRun == CHexeProcess::runAsyncRequest)
		{
		Ctx.iStatus = pstatRPCReady;
		Ctx.sRPCAddr = dResult.GetElement(0);
		Ctx.RPCMsg.sMsg = dResult.GetElement(1);
		Ctx.RPCMsg.dPayload = dResult.GetElement(2);
		Ctx.RPCMsg.dwTicket = 0;
		Ctx.RPCMsg.sReplyAddr = NULL_STR;

		return false;
		}

	//	Otherwise, process the result based on the directive that we're
	//	evaluating.

	bool bResult = true;
	switch (m_iProcessing)
		{
		case tagEval:
			OutputDatum(dResult);
			break;

		case tagFile:
			//	If this is an error, then we return with 404

			if (iRun == CHexeProcess::runError)
				{
				Ctx.iStatus = pstatResponseReady;
				Ctx.Response.InitResponse(http_NOT_FOUND, dResult.AsString());
				bResult = false;
				}

			//	If the result is a list then we expect a fileDesc and fileData.

			else if (dResult.GetCount() >= 2)
				{
				Ctx.iStatus = pstatFileDataReady;
				Ctx.dFileDesc = dResult.GetElement(0);
				Ctx.dFileData = dResult.GetElement(1);
				Ctx.AdditionalHeaders = m_Headers;
				bResult = false;
				}

			//	Otherwise we expect a filePath

			else
				{
				Ctx.iStatus = pstatFilePathReady;
				Ctx.sFilePath = dResult;
				Ctx.AdditionalHeaders = m_Headers;
				bResult = false;
				}
			break;

		case tagHeader:
			bResult = ProcessHeader(Ctx, dResult);
			break;

		case tagIf:
			m_iIfLevel++;
			if (dResult.IsNil())
				m_iIfLevelEnd = m_iIfLevel;
			break;

		case tagRedirect:
			//	If this is an error, then we return with 404

			if (iRun == CHexeProcess::runError)
				{
				Ctx.iStatus = pstatResponseReady;
				Ctx.Response.InitResponse(http_NOT_FOUND, dResult.AsString());
				bResult = false;
				}

			//	Otherwise, we expect a string containing the new address.

			else if (!dResult.IsNil())
				{
				m_dwResponseCode = http_MOVED_PERMANENTLY;
				m_sResponseMsg = STR_MOVED_PERMANENTLY;

				AddHeader(HEADER_LOCATION, dResult);
				}

			break;

		default:
			ASSERT(false);
		}

	m_iProcessing = tagNone;
	return bResult;
	}
