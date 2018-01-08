//	CHexeTextFunctionProcessor.cpp
//
//	CHexeTextFunctionProcessor class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_HEXE_PROC,				"hexeTextFunctionProcessor")
const CString &CHexeTextFunctionProcessor::StaticGetTypename (void) { return TYPENAME_HEXE_PROC; }

DECLARE_CONST_STRING(FIELD_PARSE_TEMPLATE,				"parseTemplate")
DECLARE_CONST_STRING(FIELD_TEXT,						"text")
DECLARE_CONST_STRING(FIELD_0,							"0")

DECLARE_CONST_STRING(FORMAT_HEXE_TEXT,					"hexetext")
DECLARE_CONST_STRING(FORMAT_HTML,						"html")

DECLARE_CONST_STRING(ERR_UNKNOWN_FORMAT,				"Unknown markup format: %s")

CHexeTextFunctionProcessor::CHexeTextFunctionProcessor (CDatum dInput, const CString &sFormat, CDatum dParams) :
		m_dInput(dInput),
		m_sFormat(strToLower(sFormat)),
		m_dParams(dParams)

//	CHexeTextFunctionProcessor constructor

	{
	}

void CHexeTextFunctionProcessor::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dInput.Mark();
	m_dParams.Mark();
	}

bool CHexeTextFunctionProcessor::Process (CDatum dSelf, CDatum *retResult)

//	Process
//
//	Process the text markup. We return under the following conditions:
//
//	1.	If processing has completed, we return TRUE and retResult has
//		the result (html text).
//
//	2.	If we need to call a lambda function to continue processing, we return
//		FALSE and retResult has the proper calling parameters (ready for
//		Compute.
//
//	3.	If there is an error we return FALSE and retResult is an error string.

	{
	CStringBuffer Input((const CString &)m_dInput);

	if (strEquals(m_sFormat, FORMAT_HEXE_TEXT))
		{
		//	We start by looking for all templates and links because we may need 
		//	to compute those before we can do anything.

		CTextMarkupParser Parser;
		Parser.SetInput(Input.GetPointer(), Input.GetPointer() + Input.GetLength());
		Parser.ParseExtensions(&m_Extensions);
		m_Results.InsertEmpty(m_Extensions.GetCount());
		m_iCurResult = 0;

		//	Process all extensions

		while (m_iCurResult < m_Extensions.GetCount())
			{
			if (!ProcessExtension(dSelf, m_Extensions[m_iCurResult], &m_Results[m_iCurResult], retResult))
				return false;

			m_iCurResult++;
			}

		//	Now compose

		return ProcessHexeText(retResult);
		}
	else if (strEquals(m_sFormat, FORMAT_HTML))
		{
		CStringBuffer Output;
		CString sError;
		if (!CHexeTextMarkup::EscapeHTML(Input, m_dParams, Output, &sError))
			{
			CHexeError::Create(NULL_STR, sError, retResult);
			return false;
			}

		CDatum::CreateStringFromHandoff(Output, retResult);
		return true;
		}
	else
		{
		CHexeError::Create(NULL_STR, strPattern(ERR_UNKNOWN_FORMAT, m_sFormat), retResult);
		return false;
		}

	return true;
	}

bool CHexeTextFunctionProcessor::ProcessContinues (CDatum dSelf, CDatum dResult, CDatum *retResult)

//	ProcessContinues
//
//	Process continues. This is called after we made a function call. The result
//	of the function call is in dResult. We return under the same conditions as 
//	Process.

	{
	//	Add the result to our list

	m_Results[m_iCurResult] = dResult.AsString();
	m_iCurResult++;

	//	Continue processing

	while (m_iCurResult < m_Extensions.GetCount())
		{
		if (!ProcessExtension(dSelf, m_Extensions[m_iCurResult], &m_Results[m_iCurResult], retResult))
			return false;

		m_iCurResult++;
		}

	//	We have all results, so put it all together.

	return ProcessHexeText(retResult);
	}

bool CHexeTextFunctionProcessor::ProcessExtension (CDatum dSelf, CTextMarkupParser::SExtensionDesc &Desc, CString *retsText, CDatum *retResult)

//	ProcessExtension
//
//	Get the value of the extension from parameters. This function returns TRUE
//	if we have the data (retResult is the data). If we need to call a function
//	to determine the value of the extension, we return FALSE and the calling
//	parameters are in retResult.

	{
	switch (Desc.iType)
		{
		case CTextMarkupParser::typeLink:
			{
			CString sLinkName;
			if (!Desc.Params.Find(FIELD_TEXT, &sLinkName))
				{
				if (!Desc.Params.Find(FIELD_0, &sLinkName))
					sLinkName = Desc.sName;
				}

			//	We always convert to a link

			if (!sLinkName.IsEmpty())
				*retsText = strPattern("<a href='%s'>%s</a>", htmlWriteAttributeValue(Desc.sName), htmlWriteText(sLinkName));
			else
				*retsText = strPattern("[[%s]]", htmlWriteText(sLinkName));
			return true;
			}

		case CTextMarkupParser::typeTemplate:
			{
			if (Desc.sName.IsEmpty())
				{
				*retsText = NULL_STR;
				return true;
				}

			//	Look for a function to process the template

			CDatum dFunc = m_dParams.GetElement(FIELD_PARSE_TEMPLATE);
			if (!dFunc.IsNil())
				{
				CComplexArray *pArgs = new CComplexArray;
				pArgs->Append(Desc.sName);
				pArgs->Append(CDatum(new CComplexStruct(Desc.Params)));

				CComplexArray *pResult = new CComplexArray;
				pResult->Append(dFunc);
				pResult->Append(CDatum(pArgs));
				pResult->Append(dSelf);

				*retResult = CDatum(pResult);
				return false;
				}
			else
				{
				*retsText = Desc.sName;
				return true;
				}

			break;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool CHexeTextFunctionProcessor::ProcessHexeText (CDatum *retResult)

//	ProcessHexeText
//
//	Now that we've initialized all templates and links, we process the full text.

	{
	CStringBuffer Input((const CString &)m_dInput);
	CStringBuffer Output;

	CString sError;
	CHexeTextProcessor Processor;
	Processor.SetValues(m_Results);
	if (!Processor.ConvertToHTML(Input, Output, &sError))
		{
		CHexeError::Create(NULL_STR, sError, retResult);
		return false;
		}

	CDatum::CreateStringFromHandoff(Output, retResult);
	return true;
	}
