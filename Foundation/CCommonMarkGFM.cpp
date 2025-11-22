//	CCommonMarkGFM.cpp
//
//	CCommonMarkGFM Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#include "cmark-gfm.h"
#include "cmark-gfm-core-extensions.h"

DECLARE_CONST_STRING(EXTENSION_AUTOLINK,				"autolink");
DECLARE_CONST_STRING(EXTENSION_TASKLIST,				"tasklist");
DECLARE_CONST_STRING(EXTENSION_TABLE,					"table");

class CCMarkDoc
	{
	public:

		CCMarkDoc (cmark_node*&& Src) noexcept : m_doc(Src)
			{
			Src = NULL;
			}

		CCMarkDoc (const CCMarkDoc& Src) = delete;
		CCMarkDoc (CCMarkDoc&& Src) noexcept : m_doc(Src.m_doc)
			{
			Src.m_doc = NULL;
			}

		~CCMarkDoc () { CleanUp(); }

		CCMarkDoc& operator= (const CCMarkDoc& Src) = delete;
		CCMarkDoc& operator= (CCMarkDoc&& Src) noexcept { m_doc = Src.m_doc; Src.m_doc = NULL; return *this; }

		void WriteToHTML (IByteStream& Output, const CCommonMarkGFM::SOptions& Options)
			{
			if (!m_doc)
				throw CException(errFail);

			int iOptions = 0;
			if (Options.bSourcePos) iOptions |= CMARK_OPT_SOURCEPOS;
			if (Options.bSmartQuotes) iOptions |= CMARK_OPT_SMART;
			if (Options.bFootnotes) iOptions |= CMARK_OPT_FOOTNOTES;

			char* html = cmark_render_html(m_doc, iOptions, NULL);
			Output.Write(html, strlen(html));
			free(html);
			}

	private:

		void CleanUp ()
			{
			if (m_doc)
				{
				cmark_node_free(m_doc);
				m_doc = NULL;
				}
			}

		cmark_node* m_doc = NULL;
	};

class CCMarkParser
	{
	public:

		CCMarkParser (const TArray<CString>& Extensions = TArray<CString>())
			{
			cmark_gfm_core_extensions_ensure_registered();

			m_parser = cmark_parser_new(CMARK_OPT_DEFAULT);

			for (int i = 0; i < Extensions.GetCount(); i++)
				{
				cmark_syntax_extension* ext = cmark_find_syntax_extension((LPCSTR)Extensions[i]);
				if (ext)
					{
					cmark_parser_attach_syntax_extension(m_parser, ext);
					}
				}
			}

		CCMarkParser (const CCMarkParser& Src) = delete;
		CCMarkParser (CCMarkParser&& Src) noexcept : m_parser(Src.m_parser) { Src.m_parser = NULL; }

		~CCMarkParser () { CleanUp(); }

		CCMarkParser& operator= (const CCMarkParser& Src) = delete;
		CCMarkParser& operator= (CCMarkParser&& Src) noexcept { m_parser = Src.m_parser; Src.m_parser = NULL; return *this; }

		CCMarkDoc Parse (const IMemoryBlock& Text)
			{
			if (!m_parser)
				throw CException(errFail);

			cmark_node* doc = NULL;
			cmark_parser_feed(m_parser, Text.GetPointer(), Text.GetLength());
			doc = cmark_parser_finish(m_parser);

			return CCMarkDoc(std::move(doc));
			}

	private:

		void CleanUp ()
			{
			if (m_parser)
				{
				cmark_parser_free(m_parser);
				m_parser = NULL;
				}
			}

		cmark_parser* m_parser = NULL;
	};

CString CCommonMarkGFM::SetCheckbox (CStringView sText, int iLine, bool bChecked)

//	SetCheckbox
//
//	Set a checkbox in a markdown document at the given line (1-based).

	{
	const char* pPos = sText.GetParsePointer();
	const char* pEndPos = pPos + sText.GetLength();

	//	Advance to the first character of the line.

	int iCurLine = 1;
	while (pPos < pEndPos && iCurLine < iLine)
		{
		if (*pPos == '\n')
			iCurLine++;

		pPos++;
		}

	if (pPos == pEndPos)
		return CString(sText);

	//	Advance to the bracket.

	while (pPos < pEndPos && *pPos != '[' && *pPos != '\n')
		pPos++;

	if (*pPos != '[')
		return CString(sText);

	pPos++;

	//	Write out everything before the bracket

	CStringBuffer Output;
	Output.Write(sText.GetParsePointer(), pPos - sText.GetParsePointer());

	//	Write out the checkbox

	Output.WriteChar(bChecked ? 'x' : ' ');
	pPos++;

	//	Write out the remaining text.

	Output.Write(pPos, pEndPos - pPos);

	//	Done

	return CString(std::move(Output));
	}

CString CCommonMarkGFM::SetCheckboxOnLine (CStringView sLine, bool bChecked)

//	SetCheckboxOnLine
//
//	Set a checkbox in a markdown document on the given line.

	{
	if (sLine.IsEmpty())
		return NULL_STR;

	CString sResult = CString(sLine);
	char* pPos = sResult.GetPointer();
	char* pEndPos = pPos + sResult.GetLength();
	while (pPos < pEndPos)
		{
		if (*pPos == '[' && pPos + 3 < pEndPos && pPos[2] == ']' && pPos[3] == ' ')
			{
			if (bChecked)
				{
				pPos[1] = 'x';
				}
			else
				{
				pPos[1] = ' ';
				}
			break;
			}

		pPos++;
		}

	return sResult;
	}

bool CCommonMarkGFM::ToHTML (const IMemoryBlock& Input, IByteStream& Output, const SOptions& Options, CString* retsError)

//	ToHTML
//
//	Convert to HTML.

	{
	TArray<CString> Extensions;
	Extensions.Insert(EXTENSION_AUTOLINK);
	Extensions.Insert(EXTENSION_TABLE);
	Extensions.Insert(EXTENSION_TASKLIST);

	CCMarkParser Parser(Extensions);
	CCMarkDoc Doc = Parser.Parse(Input);
	Doc.WriteToHTML(Output, Options);
	return true;
	}

