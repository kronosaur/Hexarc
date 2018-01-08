//	HexeTextImpl.h
//
//	Hexe header file
//	Copyright (c) 2013 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Hexe.h

#pragma once

class CHexeTextMarkup
	{
	public:
		static bool ConvertToHTML (const IMemoryBlock &Input, const CString &sFormat, CDatum dParams, IByteStream &Output, CString *retsError);
		static bool EscapeHTML (const IMemoryBlock &Template, CDatum dStruct, IByteStream &Output, CString *retsError = NULL);
		static CString FormatString (CDatum dArgList);
		static void WriteHTMLContent (CDatum dValue, IByteStream &Output);
	};

class CHexeTextFunctionProcessor : public TExternalDatum<CHexeTextFunctionProcessor>
	{
	public:
		CHexeTextFunctionProcessor (CDatum dInput, const CString &sFormat, CDatum dParams);
		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, CDatum *retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, CDatum *retResult);

	protected:
		virtual void OnMarked (void);

	private:
		bool ProcessExtension (CDatum dSelf, CTextMarkupParser::SExtensionDesc &Desc, CString *retsText, CDatum *retResult);
		bool ProcessHexeText (CDatum *retResult);

		CDatum m_dInput;
		CString m_sFormat;
		CDatum m_dParams;

		TArray<CTextMarkupParser::SExtensionDesc> m_Extensions;
		TArray<CString> m_Results;
		int m_iCurResult;
	};
