//	HexeMarkup.h
//
//	HexeMarkup Implementation
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

struct SHTTPRequestCtx;

class CHexeMarkupParser
	{
	public:
		enum Tokens
			{
			tkError,
			tkEoS,

			tkText,							//	Value is entire text; key is null.
			tkEmptyElement,					//	Value is simple XML element; key is null.
			tkStartTag,						//	Value open XML element; key is null.
			tkEndTag,						//	Value close XML element; key is null.
			tkPI,							//	Value is contents of processing instruction; key is tag
			tkComment,						//	Value is entire comment
			tkDirective,					//	Value is entire element
			};

		CHexeMarkupParser (void) : m_pPos(NULL), m_pPosEnd(NULL), m_bCDATAText(false), m_iToken(tkEoS) { }

		void Init (char *pPos, int iLength);
		Tokens ParseToken (CString *retsKey, CString *retsValue);

	private:
		enum States
			{
			stateStart,						//	Start

			stateCloseElement,				//	<xyz/
			stateCloseTag,					//	</
			stateComment,					//	<!--
			stateCommentEndFirst,			//	<!--xyz-
			stateCommentEndSecond,			//	<!--xyz--
			stateCommentFirst,				//	<!-
			stateDirective,					//	<!xyz
			stateDirectiveStart,			//	<!
			stateOpenAngleBracket,			//	<
			stateOpenTag,					//	<xyz...
			statePIBody,					//	<?xyz abc...
			statePIQuestion,				//	<?xyz abc?
			statePIStart,					//	<?
			statePITag,						//	<?xyz...
			stateText,						//	plain text
			stateTextCDATA,					//	CDATA text

			stateEnd,						//	Done
			};

		char *m_pPos;						//	Current parsing position
		char *m_pPosEnd;

		bool m_bCDATAText;					//	If TRUE, then parse text as CDATA

		Tokens m_iToken;
		CString m_sKey;
		CString m_sValue;

		char *m_pStart;
	};

class CHexeMarkupEvaluator
	{
	public:
		CHexeMarkupEvaluator (void);
		~CHexeMarkupEvaluator (void);

		bool EvalContinues (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult);
		bool EvalInit (SHTTPRequestCtx &Ctx, 
					   const CHexeProcess &ProcessTemplate, 
					   const CHexeSecurityCtx &SecurityCtx,
					   const TArray<CDatum> &HexeDefinitions,
					   CDatum dFileDesc, 
					   CDatum dData);
		inline void Mark (void) { m_dFileDesc.Mark(); m_dData.Mark(); m_dMarkup.Mark(); if (m_pProcess) m_pProcess->Mark(); }

	private:
		enum TagTypes
			{
			tagNone,
			tagEval,
			tagIf,
			tagFile,
			tagHeader,
			tagRedirect,
			};

		void AddHeader (const CString &sField, CDatum dValue);
		void CleanUp (void);
		bool ComposeResponse (SHTTPRequestCtx &Ctx);
		void OutputDatum (CDatum dValue);
		void OutputResult (CDatum dResult);
		bool ParseUntilRPC (SHTTPRequestCtx &Ctx);
		bool ProcessDirective (SHTTPRequestCtx &Ctx, const CString &sDirective, const CString &sValue);
		bool ProcessEval (SHTTPRequestCtx &Ctx, TagTypes iDirective, const CString &sCode);
		bool ProcessHeader (SHTTPRequestCtx &Ctx, CDatum dResult);
		bool ProcessResult (SHTTPRequestCtx &Ctx, CHexeProcess::ERunCodes iRun, CDatum dResult);

		CDatum m_dFileDesc;
		CDatum m_dData;
		CDatum m_dMarkup;
		CHexeProcess *m_pProcess;			//	Hexe process to evaluate code

		//	Input
		CHexeMarkupParser m_Parser;			//	Parser
		int m_iIfLevel;						//	<?if ...> nesting level (0 = none)
		int m_iIfLevelEnd;					//	Start including again when we reach this level.

		//	Output
		DWORD m_dwResponseCode;
		CString m_sResponseMsg;
		TArray<CHTTPMessage::SHeader> m_Headers;			//	Additional headers
		CStringBuffer m_Output;				//	Cumulative output

		//	Context while waiting for async request
		TagTypes m_iProcessing;
	};
