//	FoundationGitHubMarkdown.h
//
//	Foundation header file
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IHexeTextExtensions;

class CCommonMarkGFM
	{
	public:

		struct SOptions
			{
			bool bSourcePos = false;
			bool bSmartQuotes = false;
			bool bFootnotes = false;
			};

		static CString SetCheckbox (CStringView sText, int iLine, bool bChecked);
		static CString SetCheckboxOnLine (CStringView sLine, bool bChecked);
		static bool ToHTML (const IMemoryBlock& Input, IByteStream& Output, const SOptions& Options, CString* retsError = NULL);
	};
