//	CHexeTextProcessor.cpp
//
//	CHexeTextProcessor Class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	FORMAT
//
//	Bold Text
//
//	'''bold text'''
//	**bold text**
//
//	Italic Text
//
//	''italic text''
//	//italic text//
//
//	Strikethrough
//
//	Headers
//
//	== H1 ==
//	=== H2 ===
//	...
//
//	Lists
//
//	* bullet
//	  * sub-bullet
//
//	# ordinals
//    # sub-ordinal
//
//	[[link]]
//	[[link|link-text]]
//
//	{{template}}
//	{{template|param0|param1|...}}
//	{{template|key1=value1|key2=value2|...}}

#include "stdafx.h"

DECLARE_CONST_STRING(ENTITY_NBSP,						"&nbsp;");

DECLARE_CONST_STRING(HTML_A_OPEN_PATTERN,				"<a href='%s'>");
DECLARE_CONST_STRING(HTML_A_CLOSE,						"</a>");
DECLARE_CONST_STRING(HTML_B_CLOSE,						"</b>");
DECLARE_CONST_STRING(HTML_B_OPEN,						"<b>");
DECLARE_CONST_STRING(HTML_BLOCK_QUOTE_CLOSE,			"</blockquote>");
DECLARE_CONST_STRING(HTML_BLOCK_QUOTE_OPEN,				"<blockquote>");
DECLARE_CONST_STRING(HTML_BR,							"<br/>");
DECLARE_CONST_STRING(HTML_CODE_CLOSE,					"</code>");
DECLARE_CONST_STRING(HTML_CODE_OPEN,					"<code>");
DECLARE_CONST_STRING(HTML_HX_CLOSE,						"</h%d>");
DECLARE_CONST_STRING(HTML_HX_OPEN,						"<h%d>");
DECLARE_CONST_STRING(HTML_I_CLOSE,						"</i>");
DECLARE_CONST_STRING(HTML_I_OPEN,						"<i>");
DECLARE_CONST_STRING(HTML_LI_CLOSE,						"</li>");
DECLARE_CONST_STRING(HTML_LI_OPEN,						"<li>");
DECLARE_CONST_STRING(HTML_OL_CLOSE,						"</ol>");
DECLARE_CONST_STRING(HTML_OL_OPEN,						"<ol>");
DECLARE_CONST_STRING(HTML_P_CLOSE,						"</p>");
DECLARE_CONST_STRING(HTML_P_OPEN,						"<p>");
DECLARE_CONST_STRING(HTML_PRE_CODE_CLOSE,				"</code></pre>");
DECLARE_CONST_STRING(HTML_PRE_CODE_OPEN,				"<pre><code>");
DECLARE_CONST_STRING(HTML_STRIKE_CLOSE,					"</strike>");
DECLARE_CONST_STRING(HTML_STRIKE_OPEN,					"<strike>");
DECLARE_CONST_STRING(HTML_UL_CLOSE,						"</ul>");
DECLARE_CONST_STRING(HTML_UL_OPEN,						"<ul>");

DECLARE_CONST_STRING(SPECIAL_TEMPLATE_OL,				"#");
DECLARE_CONST_STRING(SPECIAL_TEMPLATE_UL,				"*");

DECLARE_CONST_STRING(URL_HTTP_PREFIX,					"http:");
DECLARE_CONST_STRING(URL_HTTPS_PREFIX,					"https:");

const int MAX_HEADER_LEVEL =							6;

CHexeTextProcessor::CHexeTextProcessor (void)

//	CHexeTextProcessor constructor

	{
	}

void CHexeTextProcessor::BlockQuoteToHTML (int iLevel)

//	BlockQuoteToHTML
//
//	Converts a block quote

	{
	//	If we're at the proper level, then we just output a paragraph

	if (m_iCurBlockQuoteLevel == iLevel)
		{
		bool bInsideP = false;
		m_Parser.ParseNextToken();

		bool bDone = false;
		while (!bDone)
			{
			switch (m_Parser.GetToken())
				{
				case CTextMarkupParser::tokenText:
					{
					int iTildeCount;

					//	If we've got a single character followed by a tilde, then this is a
					//	nested code block, so we leave this paragraph.

					if (m_Parser.GetTokenString().GetLength() == 1 
							&& (*m_Parser.GetTokenString().GetParsePointer() == ' ' || *m_Parser.GetTokenString().GetParsePointer() == '\t')
							&& m_Parser.PeekNextToken(NULL, &iTildeCount) == CTextMarkupParser::tokenTilde
							&& iTildeCount >= 3)
						{
						m_Parser.ParseNextToken();
						bDone = true;
						break;
						}

					//	Otherwise, parse text

					else
						{
						//	Open paragraph, if necessary

						if (!bInsideP)
							{
							m_pOutput->Write(HTML_P_OPEN);
							bInsideP = true;
							}

						//	Text

						TextToHTML(true);
						if (m_Parser.GetToken() == CTextMarkupParser::tokenLineEnd)
							{
							int iLines = m_Parser.GetTokenCount();

							//	If we only have one line then stay in the block quote and see if
							//	we can stay in the same paragraph

							if (iLines == 1)
								{
								m_Parser.ParseNextToken();
								if (m_Parser.GetToken() == CTextMarkupParser::tokenGreaterThan
										&& m_Parser.GetTokenCount() == m_iCurBlockQuoteLevel
										&& (m_Parser.PeekNextChar() == ' ' || m_Parser.PeekNextChar() == '\t'))
									{
									m_Parser.ParseNextToken();
									if (m_Parser.GetTokenString().GetLength() > 1)
										m_pOutput->Write(HTML_BR);
									}
								else
									bDone = true;
								}

							//	If two lines, then leave the paragraph, but stay in the block quote

							else if (iLines == 2)
								{
								m_Parser.ParseNextToken();
								bDone = true;
								}

							//	Otherwise, we're done with the paragraph and blockquote.

							else
								bDone = true;
							}
						}

					break;
					}

				default:
					bDone = true;
				}
			}

		if (bInsideP)
			m_pOutput->Write(HTML_P_CLOSE);
		}

	//	If we need to add a single level then parse the whole list while we
	//	have entries at the same level or higher.

	else
		{
		m_iCurBlockQuoteLevel++;

		//	Output block quote start

		m_pOutput->Write(HTML_BLOCK_QUOTE_OPEN);

		//	Output paragraphs elements

		bool bDone = false;
		while (!bDone)
			{
			switch (m_Parser.GetToken())
				{
				case CTextMarkupParser::tokenGreaterThan:
					if (m_Parser.GetTokenCount() >= m_iCurBlockQuoteLevel)
						BlockQuoteToHTML(m_Parser.GetTokenCount());
					else
						bDone = true;
					break;

				case CTextMarkupParser::tokenTilde:
					if (m_Parser.GetTokenCount() >= 3)
						{
						CodeToHTML(m_iCurBlockQuoteLevel + 1);
						if (m_Parser.GetToken() == CTextMarkupParser::tokenLineEnd && m_Parser.GetTokenCount() <= 2)
							m_Parser.ParseNextToken();
						}
					else
						bDone = true;
					break;

				default:
					bDone = true;
				}
			}

		m_pOutput->Write(HTML_BLOCK_QUOTE_CLOSE);

		m_iCurBlockQuoteLevel--;
		}
	}

void CHexeTextProcessor::CloseStyles (TArray<CTextMarkupParser::ETokens> &StyleStack)

//	CloseStyles
//
//	Close all styles that haven't yet been closed.

	{
	int i;

	if (StyleStack.GetCount() > 0)
		{
		for (i = StyleStack.GetCount() - 1; i >= 0; i--)
			{
			switch (StyleStack[i])
				{
				case CTextMarkupParser::tokenStar:
					m_pOutput->Write(HTML_B_CLOSE);
					break;

				case CTextMarkupParser::tokenSlash:
					m_pOutput->Write(HTML_I_CLOSE);
					break;

				case CTextMarkupParser::tokenTilde:
					m_pOutput->Write(HTML_STRIKE_CLOSE);
					break;
				}
			}

		StyleStack.DeleteAll();
		}
	}

void CHexeTextProcessor::CodeToHTML (int iIndentLevel)

//	CodeToHTML
//
//	Converts a code block

	{
	int i;

	//	Skip all text in the opening line

	while (m_Parser.GetToken() != CTextMarkupParser::tokenLineEnd
			&& m_Parser.GetToken() != CTextMarkupParser::tokenEoS)
		m_Parser.ParseNextToken();

	//	Edge-cases

	if (m_Parser.GetToken() == CTextMarkupParser::tokenEoS)
		{
		m_pOutput->Write(HTML_PRE_CODE_OPEN);
		m_pOutput->Write(HTML_PRE_CODE_CLOSE);
		return;
		}
	else
		m_Parser.ParseNextToken();

	//	Start

	m_pOutput->Write(HTML_PRE_CODE_OPEN);

	//	Inside of code block

	bool bDone = false;
	bool bStartLine = true;
	int iCharsToSwallow = iIndentLevel;
	while (!bDone)
		{
		switch (m_Parser.GetToken())
			{
			case CTextMarkupParser::tokenText:
				{
				char *pPos = m_Parser.GetTokenString().GetParsePointer();
				char *pPosEnd = pPos + m_Parser.GetTokenString().GetLength();

				//	If we're starting a line, and we've got an indent, consume characters

				if (bStartLine && iCharsToSwallow > 0)
					{
					while (iCharsToSwallow > 0 && pPos < pPosEnd)
						{
						pPos++;
						iCharsToSwallow--;
						}

					//	If we've still got chars, then write them. Otherwise, we do 
					//	nothing (and we leave bStartLine true).

					if (pPos != pPosEnd)
						{
						htmlWriteText(pPos, pPosEnd, *m_pOutput);
						bStartLine = false;
						}
					}

				//	Otherwise, just write the line out.

				else
					{
					htmlWriteText(pPos, pPosEnd, *m_pOutput);
					bStartLine = false;
					}

				break;
				}

			case CTextMarkupParser::tokenURL:
				htmlWriteText(m_Parser.GetTokenString(), *m_pOutput);
				bStartLine = false;
				break;

			case CTextMarkupParser::tokenEntity:
				m_pOutput->Write(m_Parser.GetTokenString());
				bStartLine = false;
				break;

			case CTextMarkupParser::tokenEscapeText:
				m_pOutput->Write("\\", 1);
				m_pOutput->Write(m_Parser.GetTokenString());
				bStartLine = false;
				break;

			case CTextMarkupParser::tokenEquals:
			case CTextMarkupParser::tokenStar:
			case CTextMarkupParser::tokenHash:
			case CTextMarkupParser::tokenSlash:
			case CTextMarkupParser::tokenSingleQuote:
			case CTextMarkupParser::tokenDoubleQuote:
			case CTextMarkupParser::tokenOpenBracket:
			case CTextMarkupParser::tokenCloseBracket:
			case CTextMarkupParser::tokenOpenBrace:
			case CTextMarkupParser::tokenCloseBrace:
			case CTextMarkupParser::tokenLessThan:
			case CTextMarkupParser::tokenGreaterThan:
			case CTextMarkupParser::tokenDash:
			case CTextMarkupParser::tokenUnderscore:
			case CTextMarkupParser::tokenBackslash:
			case CTextMarkupParser::tokenVerticalBar:
			case CTextMarkupParser::tokenBackQuote:
				{
				//	Consume characters

				int iCharsLeft = m_Parser.GetTokenCount();
				if (bStartLine && iCharsToSwallow > 0)
					{
					int iConsume = Min(iCharsToSwallow, iCharsLeft);
					iCharsToSwallow -= iConsume;
					iCharsLeft -= iConsume;
					}

				//	Output characters

				if (iCharsLeft > 0)
					{
					OutputRepeatingChar(m_Parser.GetTokenChar(), iCharsLeft);
					bStartLine = false;
					}
				break;
				}

			case CTextMarkupParser::tokenLineEnd:
				for (i = 0; i < m_Parser.GetTokenCount(); i++)
					m_pOutput->Write("\r\n", 2);
				bStartLine = true;
				iCharsToSwallow = iIndentLevel;
				break;

			case CTextMarkupParser::tokenTilde:
				if (bStartLine && m_Parser.GetTokenCount() >= 3)
					{
					m_Parser.ParseNextToken();
					bDone = true;
					}
				else
					{
					OutputRepeatingChar(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
					bStartLine = false;
					}
				break;

			case CTextMarkupParser::tokenEoS:
				bDone = true;
				break;

			default:
				ASSERT(false);
				bDone = true;
				break;
			}

		//	Get the next token.

		if (!bDone)
			m_Parser.ParseNextToken();
		}

	//	End

	m_pOutput->Write(HTML_PRE_CODE_CLOSE);
	}

bool CHexeTextProcessor::ConvertToHTML (const IMemoryBlock &Input, IByteStream &Output, CString *retsError)

//	ConvertToHTML
//
//	Converts the input to HTML output.

	{
	m_pOutput = &Output;
	m_Parser.SetInput(Input.GetPointer(), Input.GetPointer() + Input.GetLength());
	m_Parser.ParseNextToken();
	m_bInLink = false;
	m_StyleStack.DeleteAll();
	m_InLinkStyleStack.DeleteAll();
	m_iCurListLevel = 0;
	m_iCurBlockQuoteLevel = 0;
	m_iCurValue = 0;

	//	For the special case where we have no input, we return an empty 
	//	paragraph.

	bool bDone = false;
	if (m_Parser.GetToken() == CTextMarkupParser::tokenEoS)
		{
		m_pOutput->Write(HTML_P_OPEN);
		m_pOutput->Write(HTML_P_CLOSE);
		bDone = true;
		}

	//	Parse

	while (!bDone)
		{
		switch (m_Parser.GetToken())
			{
			case CTextMarkupParser::tokenText:
			case CTextMarkupParser::tokenEntity:
			case CTextMarkupParser::tokenEscapeText:
			case CTextMarkupParser::tokenURL:
			case CTextMarkupParser::tokenSlash:
			case CTextMarkupParser::tokenSingleQuote:
			case CTextMarkupParser::tokenDoubleQuote:
			case CTextMarkupParser::tokenOpenBracket:
			case CTextMarkupParser::tokenCloseBracket:
			case CTextMarkupParser::tokenOpenBrace:
			case CTextMarkupParser::tokenCloseBrace:
			case CTextMarkupParser::tokenLessThan:
			case CTextMarkupParser::tokenDash:
			case CTextMarkupParser::tokenUnderscore:
			case CTextMarkupParser::tokenBackslash:
			case CTextMarkupParser::tokenVerticalBar:
			case CTextMarkupParser::tokenBackQuote:
				ParagraphToHTML();
				break;

			case CTextMarkupParser::tokenHash:
				if (strIsWhitespace(m_Parser.PeekNextChar()))
					ListToHTML(listOrdered, m_Parser.GetTokenCount());
				else
					ParagraphToHTML();
				break;

			case CTextMarkupParser::tokenStar:
				if (strIsWhitespace(m_Parser.PeekNextChar()))
					ListToHTML(listUnordered, m_Parser.GetTokenCount());
				else
					ParagraphToHTML();
				break;

			case CTextMarkupParser::tokenGreaterThan:
				if (strIsWhitespace(m_Parser.PeekNextChar()))
					BlockQuoteToHTML(m_Parser.GetTokenCount());
				else
					ParagraphToHTML();
				break;

			case CTextMarkupParser::tokenEquals:
				if (m_Parser.GetTokenCount() >= 2 && m_Parser.GetTokenCount() <= (MAX_HEADER_LEVEL + 1))
					HeaderToHTML();
				else
					ParagraphToHTML();
				break;

			case CTextMarkupParser::tokenTilde:
				if (m_Parser.GetTokenCount() >= 3 && !FindTextEnd(CTextMarkupParser::tokenTilde))
					CodeToHTML();
				else
					ParagraphToHTML();
				break;

			case CTextMarkupParser::tokenLineEnd:
				//	Skip
				m_Parser.ParseNextToken();
				break;

			case CTextMarkupParser::tokenError:
			case CTextMarkupParser::tokenEoS:
				bDone = true;
				break;
			}
		}

	//	Done

	m_pOutput = NULL;
	return true;
	}

bool CHexeTextProcessor::FindTextEnd (CTextMarkupParser::ETokens iToken)

//	FindTextEnd
//
//	Returns TRUE if we find the matching token before the end of the text line
//	or paragraph.

	{
	CTextMarkupParser Saved;
	m_Parser.SaveParser(&Saved);

	bool bFound = false;
	bool bReachedEnd = false;
	while (!bFound && !bReachedEnd)
		{
		CTextMarkupParser::ETokens iNext = m_Parser.ParseNextToken();
		if (iNext == iToken
				&& m_Parser.GetTokenCount() >= 2)
			bFound = true;
		else
			{
			switch (iNext)
				{
				case CTextMarkupParser::tokenEoS:
				case CTextMarkupParser::tokenLineEnd:
				case CTextMarkupParser::tokenError:
					bReachedEnd = true;
					break;
				}
			}
		}

	m_Parser.RestoreParser(Saved);

	return bFound;
	}

void CHexeTextProcessor::HeaderToHTML (void)

//	HeaderToHTML
//
//	Parses and outputs a header. Leaves the parser at the first token AFTER the
//	header.

	{
	int iLevel = m_Parser.GetTokenCount() - 1;
	m_pOutput->Write(strPattern(HTML_HX_OPEN, iLevel));
	m_Parser.ParseNextToken();

	TextToHTML(true, '=');
	m_Parser.ParseNextToken();

	m_pOutput->Write(strPattern(HTML_HX_CLOSE, iLevel));
	}

bool CHexeTextProcessor::IsHeaderEnd (void)

//	IsHeaderEnd
//
//	Returns TRUE if the next token is the end of a header.

	{
	bool bIsEnd = false;

	CTextMarkupParser Saved;
	m_Parser.SaveParser(&Saved);

	CTextMarkupParser::ETokens iNext = m_Parser.ParseNextToken();
	if (iNext == CTextMarkupParser::tokenLineEnd
			|| iNext == CTextMarkupParser::tokenEoS)
		bIsEnd = true;
	else if (iNext == CTextMarkupParser::tokenEquals && m_Parser.GetTokenCount() >= 2 && m_Parser.GetTokenCount() <= (MAX_HEADER_LEVEL + 1))
		{
		while (true)
			{
			iNext = m_Parser.ParseNextToken();
			if (iNext == CTextMarkupParser::tokenLineEnd
					|| iNext == CTextMarkupParser::tokenEoS)
				{
				bIsEnd = true;
				break;
				}
			else if (iNext == CTextMarkupParser::tokenText && m_Parser.GetTokenString().IsWhitespace())
				continue;
			else
				break;
			}
		}

	m_Parser.RestoreParser(Saved);

	//	Done

	return bIsEnd;
	}

void CHexeTextProcessor::LinkToHTML (void)

//	LinkToHTML
//
//	Processes a link. Leaves the parser at the last token.

	{
	m_Parser.ParseNextToken();

	//	If this is a basic link, then parse it.

	if (m_Parser.GetToken() == CTextMarkupParser::tokenURL)
		{
		//	Write tag open

		CString sURL = m_Parser.GetTokenString();
		m_pOutput->Write(strPattern(HTML_A_OPEN_PATTERN, strToXMLText(sURL)));

		//	Get the next token. If this is a vertical bar, then we have some 
		//	link text.

		if (m_Parser.ParseNextToken() == CTextMarkupParser::tokenVerticalBar)
			{
			m_Parser.ParseNextToken();

			m_bInLink = true;
			TextToHTML();
			m_bInLink = false;
			}

		//	Just write the URL as the link text

		else
			htmlWriteText(sURL, *m_pOutput);

		//	Otherwise, we're done

		m_pOutput->Write(HTML_A_CLOSE);
		}

	//	Otherwise, we need to ask an extension for help.

	else
		{
		CString sLink;
		TArray<CString> Params;

		//	Parse the link

		bool bDone = false;
		while (!bDone)
			{
			switch (m_Parser.GetToken())
				{
				case CTextMarkupParser::tokenEoS:
				case CTextMarkupParser::tokenCloseBracket:
				case CTextMarkupParser::tokenVerticalBar:
					bDone = true;
					break;

				default:
					if (m_Parser.GetToken() == CTextMarkupParser::tokenText)
						sLink = sLink + m_Parser.GetTokenString();
					else
						sLink = sLink + strRepeat(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
					break;
				}

			if (!bDone)
				m_Parser.ParseNextToken();
			}

		//	If we have a vertical bar, keep parsing

		if (m_Parser.GetToken() == CTextMarkupParser::tokenVerticalBar)
			{
			m_Parser.ParseNextToken();

			bDone = false;
			CString sCurrent;
			while (!bDone)
				{
				switch (m_Parser.GetToken())
					{
					case CTextMarkupParser::tokenEoS:
					case CTextMarkupParser::tokenCloseBracket:
					case CTextMarkupParser::tokenVerticalBar:
						if (!sCurrent.IsEmpty())
							{
							Params.Insert(sCurrent);
							sCurrent = NULL_STR;
							}

						if (m_Parser.GetToken() != CTextMarkupParser::tokenVerticalBar)
							bDone = true;
						break;

					default:
						if (m_Parser.GetToken() == CTextMarkupParser::tokenText)
							sCurrent = sCurrent + m_Parser.GetTokenString();
						else
							sCurrent = sCurrent + strRepeat(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
						break;
					}

				if (!bDone)
					m_Parser.ParseNextToken();
				}
			}

		//	Allow the extension to determine the output

		if (m_pValues)
			{
			if (m_iCurValue < m_pValues->GetCount())
				m_pOutput->Write(m_pValues->GetAt(m_iCurValue++));
			}
		else if (m_pExtensions)
			{
			//	NOTE: The extension is responsible for escaping any HTML characters.
			m_pOutput->Write(m_pExtensions->ProcessLink(sLink, Params));
			}
		}
	}

void CHexeTextProcessor::ListToHTML (EListTypes iListType, int iItemLevel)

//	ListToHTML
//
//	Handle a bullet list. Leaves the parser at the first token AFTER the bullet
//	list.

	{
	enum EItemState
		{
		stateNone,
		stateP,
		};

	//	If we're at the proper level, then we just output a line

	if (m_iCurListLevel == iItemLevel)
		{
		m_pOutput->Write(HTML_LI_OPEN);
		m_Parser.ParseNextToken();

		TextToHTML(true);
		if (m_Parser.GetToken() == CTextMarkupParser::tokenLineEnd && m_Parser.GetTokenCount() <= 2)
			m_Parser.ParseNextToken();

		m_pOutput->Write(HTML_LI_CLOSE);
		}

	//	If we need to add a single level then parse the whole list while we
	//	have entries at the same level or higher.

	else
		{
		bool bUnordered = (m_Parser.GetToken() == CTextMarkupParser::tokenStar);
		m_iCurListLevel++;

		//	If we've already outputted the list open tag through a template, then
		//	we don't need to do anything

		if (m_bOLTemplate && m_iCurListLevel == 1)
			{ }
		else if (m_bULTemplate && m_iCurListLevel == 1)
			{ }
		else
			m_pOutput->Write(bUnordered ? HTML_UL_OPEN : HTML_OL_OPEN);

		//	Either way, we're done with the flags

		m_bOLTemplate = false;
		m_bULTemplate = false;

		//	Output list elements

		EItemState iItemState = stateNone;
		bool bDone = false;
		while (!bDone)
			{
			switch (m_Parser.GetToken())
				{
				case CTextMarkupParser::tokenHash:
					if (iListType != listOrdered)
						bDone = true;
					else if (m_Parser.GetTokenCount() >= m_iCurListLevel)
						ListToHTML(iListType, m_Parser.GetTokenCount());
					else
						bDone = true;
					break;

				case CTextMarkupParser::tokenStar:
					if (iListType != listUnordered)
						bDone = true;
					else if (m_Parser.GetTokenCount() >= m_iCurListLevel)
						ListToHTML(iListType, m_Parser.GetTokenCount());
					else
						bDone = true;
					break;

				//	If we've got a line of text indented the correct number of spaces,
				//	then we continue to output text at the same level.

				case CTextMarkupParser::tokenText:
					{
					int iTildeCount;

					//	Count leading whitespace

					int iLeadingSpace = 0;
					char *pPos = m_Parser.GetTokenString().GetParsePointer();
					while (*pPos == ' ')
						{
						iLeadingSpace++;
						pPos++;
						}

					//	If we don't have the right amount, then we're done

					if (iLeadingSpace != m_iCurListLevel + 1)
						{
						//	If necessary, close our paragraph

						if (iItemState == stateP)
							{
							m_pOutput->Write(HTML_P_CLOSE);
							iItemState = stateNone;
							}

						bDone = true;
						}

					//	If we've got an indented code fence, then output a code block

					else if (*pPos == '\0' && m_Parser.PeekNextToken(NULL, &iTildeCount) == CTextMarkupParser::tokenTilde && iTildeCount >= 3)
						{
						m_Parser.ParseNextToken();
						CodeToHTML(iLeadingSpace);
						if (m_Parser.GetToken() == CTextMarkupParser::tokenLineEnd && m_Parser.GetTokenCount() <= 2)
							m_Parser.ParseNextToken();
						}

					//	Otherwise, we've got an indented paragraph

					else
						{
						//	If we're starting a new paragraph, output the right tag

						switch (iItemState)
							{
							case stateNone:
								m_pOutput->Write(HTML_P_OPEN);
								iItemState = stateP;
								break;
							}

						//	Write text

						TextToHTML(true);
						if (m_Parser.GetToken() == CTextMarkupParser::tokenLineEnd && m_Parser.GetTokenCount() == 1)
							{
							if (m_Parser.ParseNextToken() == CTextMarkupParser::tokenText)
								m_pOutput->Write(HTML_BR);
							}

						//	If the next token is not text, then close our paragraph

						if (m_Parser.GetToken() != CTextMarkupParser::tokenText)
							{
							m_pOutput->Write(HTML_P_CLOSE);
							iItemState = stateNone;
							}
						}

					break;
					}

				default:
					bDone = true;
				}
			}

		m_pOutput->Write(bUnordered ? HTML_UL_CLOSE : HTML_OL_CLOSE);

		m_iCurListLevel--;
		}
	}

void CHexeTextProcessor::OutputRepeatingChar (char chChar, int iCount)

//	OutputRepeatingChar
//
//	Outputs the same character for the given count.

	{
	int i;

	switch (chChar)
		{
		//	This is a special character in HTML

		case '<':
			for (i = 0; i < iCount; i++)
				m_pOutput->Write("&lt;", 4);
			break;

		//	We don't always have to escape this character in HTML, but we MUST
		//	when it is preceeded by ]]. Thus, we do it all the time.

		case '>':
			for (i = 0; i < iCount; i++)
				m_pOutput->Write("&gt;", 4);
			break;

		case '&':
			for (i = 0; i < iCount; i++)
				m_pOutput->Write("&amp;", 5);
			break;

		default:
			m_pOutput->WriteChar(chChar, iCount);
		}
	}

void CHexeTextProcessor::ParagraphToHTML (void)

//	ParagrphToHTML
//
//	Parses a paragraph. Leaves the parser pointing at the first token AFTER
//	the paragraph.

	{
	//	Start

	m_pOutput->Write(HTML_P_OPEN);

	//	Inside of paragraph

	bool bDone = false;
	bool bStartLine = true;
	while (!bDone)
		{
		switch (m_Parser.GetToken())
			{
			case CTextMarkupParser::tokenText:
			case CTextMarkupParser::tokenEntity:
			case CTextMarkupParser::tokenEscapeText:
			case CTextMarkupParser::tokenURL:
			case CTextMarkupParser::tokenSlash:
			case CTextMarkupParser::tokenSingleQuote:
			case CTextMarkupParser::tokenDoubleQuote:
			case CTextMarkupParser::tokenOpenBracket:
			case CTextMarkupParser::tokenCloseBracket:
			case CTextMarkupParser::tokenOpenBrace:
			case CTextMarkupParser::tokenCloseBrace:
			case CTextMarkupParser::tokenLessThan:
			case CTextMarkupParser::tokenDash:
			case CTextMarkupParser::tokenUnderscore:
			case CTextMarkupParser::tokenBackslash:
			case CTextMarkupParser::tokenVerticalBar:
			case CTextMarkupParser::tokenTilde:
			case CTextMarkupParser::tokenBackQuote:
				TextToHTML(bStartLine);
				bStartLine = false;
				break;

			case CTextMarkupParser::tokenHash:
			case CTextMarkupParser::tokenStar:
			case CTextMarkupParser::tokenGreaterThan:
				if (strIsWhitespace(m_Parser.PeekNextChar()))
					bDone = true;
				else
					TextToHTML();
				bStartLine = false;
				break;

			case CTextMarkupParser::tokenEquals:
				if (m_Parser.GetTokenCount() >= 2 && m_Parser.GetTokenCount() <= (MAX_HEADER_LEVEL + 1))
					bDone = true;
				else
					TextToHTML();
				bStartLine = false;
				break;

			case CTextMarkupParser::tokenLineEnd:
				{
				//	We always advance to the next token but we keep track of how
				//	many line ends.

				int iCount = m_Parser.GetTokenCount();
				m_Parser.ParseNextToken();

				//	If the next character is a star, a hash, or equals then we are
				//	done.
				//	(Or we have a double line end).

				if (iCount > 1
						|| (m_Parser.GetToken() == CTextMarkupParser::tokenStar && (m_Parser.PeekNextChar() == ' ' || m_Parser.PeekNextChar() == '\t'))
						|| (m_Parser.GetToken() == CTextMarkupParser::tokenHash && (m_Parser.PeekNextChar() == ' ' || m_Parser.PeekNextChar() == '\t'))
						|| (m_Parser.GetToken() == CTextMarkupParser::tokenGreaterThan && (m_Parser.PeekNextChar() == ' ' || m_Parser.PeekNextChar() == '\t'))
						|| (m_Parser.GetToken() == CTextMarkupParser::tokenTilde && m_Parser.GetTokenCount() >= 3)
						|| (m_Parser.GetToken() == CTextMarkupParser::tokenEquals 
							&& m_Parser.GetTokenCount() >= 2 
							&& m_Parser.GetTokenCount() <= (MAX_HEADER_LEVEL + 1)))
					bDone = true;

				//	If this is a line-end followed by whitespace and another 
				//	line-end, then swallow it, since it's a no-op.

				else if (m_Parser.GetToken() == CTextMarkupParser::tokenText
							&& m_Parser.GetTokenString().IsWhitespace()
							&& m_Parser.PeekNextToken() == CTextMarkupParser::tokenLineEnd)
					{ }

				//	Otherwise we stay in the paragraph and just add a line break

				else
					{
					m_pOutput->Write(HTML_BR);
					bStartLine = true;
					}

				break;
				}

			default:
				bDone = true;
				break;
			}
		}

	//	End

	m_pOutput->Write(HTML_P_CLOSE);
	}

void CHexeTextProcessor::TemplateToHTML (void)

//	TemplateToHTML
//
//	Processes a template. Leaves the parser at the last token.

	{
	CString sTemplate;
	TSortMap<CString, CString> Params;

	//	Parse the template name

	m_Parser.ParseNextToken();

	bool bDone = false;
	while (!bDone)
		{
		switch (m_Parser.GetToken())
			{
			case CTextMarkupParser::tokenEoS:
			case CTextMarkupParser::tokenCloseBrace:
			case CTextMarkupParser::tokenVerticalBar:
				bDone = true;
				break;

			default:
				if (m_Parser.GetToken() == CTextMarkupParser::tokenText)
					sTemplate = sTemplate + m_Parser.GetTokenString();
				else
					sTemplate = sTemplate + strRepeat(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
				break;
			}

		if (!bDone)
			m_Parser.ParseNextToken();
		}

	//	If we have a vertical bar, keep parsing

	if (m_Parser.GetToken() == CTextMarkupParser::tokenVerticalBar)
		{
		m_Parser.ParseNextToken();

		bDone = false;
		CString sKey;
		CString sValue;
		bool bParseKey = true;
		int iParam = 0;
		while (!bDone)
			{
			switch (m_Parser.GetToken())
				{
				case CTextMarkupParser::tokenEquals:
					bParseKey = false;
					break;

				case CTextMarkupParser::tokenEoS:
				case CTextMarkupParser::tokenCloseBrace:
				case CTextMarkupParser::tokenVerticalBar:
					if (bParseKey || sKey.IsEmpty())
						Params.SetAt(strFromInt(iParam), sKey);
					else
						Params.SetAt(sKey, sValue);

					sKey = NULL_STR;
					sValue = NULL_STR;
					iParam++;

					if (m_Parser.GetToken() != CTextMarkupParser::tokenVerticalBar)
						bDone = true;
					break;

				default:
					if (m_Parser.GetToken() == CTextMarkupParser::tokenText)
						{
						if (bParseKey)
							sKey = sKey + m_Parser.GetTokenString();
						else
							sValue = sValue + m_Parser.GetTokenString();
						}
					else
						{
						if (bParseKey)
							sKey = sKey + strRepeat(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
						else
							sValue = sValue + strRepeat(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
						}
					break;
				}

			if (!bDone)
				m_Parser.ParseNextToken();
			}
		}

	//	Handle some special templates

	if (strEquals(sTemplate, SPECIAL_TEMPLATE_OL))
		{
		if (m_iCurListLevel == 0 && m_iCurValue < m_pValues->GetCount())
			{
			m_pOutput->Write(m_pValues->GetAt(m_iCurValue++));
			m_bOLTemplate = true;
			}
		}
	else if (strEquals(sTemplate, SPECIAL_TEMPLATE_UL))
		{
		if (m_iCurListLevel == 0 && m_iCurValue < m_pValues->GetCount())
			{
			m_pOutput->Write(m_pValues->GetAt(m_iCurValue++));
			m_bULTemplate = true;
			}
		}
	else
		{
		//	Allow the extension to determine the output

		if (m_pValues)
			{
			if (m_iCurValue < m_pValues->GetCount())
				m_pOutput->Write(m_pValues->GetAt(m_iCurValue++));
			}
		else if (m_pExtensions)
			{
			//	NOTE: The extension is responsible for escaping any HTML characters.
			m_pOutput->Write(m_pExtensions->ProcessTemplate(sTemplate, Params));
			}
		}
	}

void CHexeTextProcessor::TextToHTML (bool bSkipWhitespace, char chTerminator)

//	TextToHTML
//
//	Output text until we reach the end of the line. We leave the end-of-line
//	token in the parser.

	{
	bool bDone = false;
	while (!bDone)
		{
		switch (m_Parser.GetToken())
			{
			case CTextMarkupParser::tokenText:
			case CTextMarkupParser::tokenEntity:
			case CTextMarkupParser::tokenEscapeText:
			case CTextMarkupParser::tokenURL:
				{
				char *pPos = m_Parser.GetTokenString().GetParsePointer();
				char *pPosEnd = pPos + m_Parser.GetTokenString().GetLength();

				//	Trim whitespace, if necessary

				if (!m_bInLink)
					{
					bool bStartsWithWS = strIsWhitespace(pPos);
					bool bEndsWithWS = strIsWhitespace(pPosEnd - 1);

					//	Trim leading spaces, if necessary

					if (bStartsWithWS && bSkipWhitespace)
						{
						while (pPos < pPosEnd && strIsWhitespace(pPos))
							pPos++;
						}

					//	Trim trailing spaces, if necessary

					if (bEndsWithWS 
							&& ((chTerminator == '=' && IsHeaderEnd())
								|| (chTerminator != '=' && m_Parser.PeekNextToken() == CTextMarkupParser::tokenLineEnd)))
						{
						while (pPosEnd > pPos && strIsWhitespace(pPosEnd - 1))
							pPosEnd--;
						}
					}

				//	Write out the string

				if (!m_bInLink 
						&& m_Parser.GetToken() == CTextMarkupParser::tokenURL
						&& !IsTopStyle(CTextMarkupParser::tokenTilde))
					{
					m_pOutput->Write(strPattern(HTML_A_OPEN_PATTERN, strToXMLText(m_Parser.GetTokenString())));
					htmlWriteText(m_Parser.GetTokenString(), *m_pOutput);
					m_pOutput->Write(HTML_A_CLOSE);
					}
				else if (m_Parser.GetToken() == CTextMarkupParser::tokenEntity)
					m_pOutput->Write(pPos, (int)(pPosEnd - pPos));
				else
					htmlWriteText(pPos, pPosEnd, *m_pOutput);
				break;
				}

			case CTextMarkupParser::tokenSlash:
				if (IsTopStyle(CTextMarkupParser::tokenTilde))
					m_pOutput->WriteChar('/', m_Parser.GetTokenCount());
				else if (m_Parser.GetTokenCount() >= 2 && IsTopStyle(CTextMarkupParser::tokenSlash))
					{
					m_pOutput->WriteChar('/', m_Parser.GetTokenCount() - 2);
					m_pOutput->Write(HTML_I_CLOSE);
					PopStyle();
					}
				else if (m_Parser.GetTokenCount() >= 2
						&& !strIsWhitespace(m_Parser.PeekNextChar())
						&& !IsStyleInStack(CTextMarkupParser::tokenSlash)
						&& FindTextEnd(CTextMarkupParser::tokenSlash))
					{
					PushStyle(CTextMarkupParser::tokenSlash);
					m_pOutput->Write(HTML_I_OPEN);
					m_pOutput->WriteChar('/', m_Parser.GetTokenCount() - 2);
					}
				else
					m_pOutput->WriteChar('/', m_Parser.GetTokenCount());
				break;

			case CTextMarkupParser::tokenStar:
				if (IsTopStyle(CTextMarkupParser::tokenTilde))
					m_pOutput->WriteChar('*', m_Parser.GetTokenCount());
				else if (m_Parser.GetTokenCount() >= 2 && IsTopStyle(CTextMarkupParser::tokenStar))
					{
					m_pOutput->WriteChar('*', m_Parser.GetTokenCount() - 2);
					m_pOutput->Write(HTML_B_CLOSE);
					PopStyle();
					}
				else if (m_Parser.GetTokenCount() >= 2
						&& !strIsWhitespace(m_Parser.PeekNextChar())
						&& !IsStyleInStack(CTextMarkupParser::tokenStar)
						&& FindTextEnd(CTextMarkupParser::tokenStar))
					{
					PushStyle(CTextMarkupParser::tokenStar);
					m_pOutput->Write(HTML_B_OPEN);
					m_pOutput->WriteChar('*', m_Parser.GetTokenCount() - 2);
					}
				else
					m_pOutput->WriteChar('*', m_Parser.GetTokenCount());
				break;

			case CTextMarkupParser::tokenTilde:
				if (m_Parser.GetTokenCount() >= 2 && IsTopStyle(CTextMarkupParser::tokenTilde))
					{
					m_pOutput->WriteChar('~', m_Parser.GetTokenCount() - 2);
					m_pOutput->Write(HTML_CODE_CLOSE);
					PopStyle();
					}
				else if (m_Parser.GetTokenCount() >= 2
						&& !strIsWhitespace(m_Parser.PeekNextChar())
						&& !IsStyleInStack(CTextMarkupParser::tokenTilde)
						&& FindTextEnd(CTextMarkupParser::tokenTilde))
					{
					PushStyle(CTextMarkupParser::tokenTilde);
					m_pOutput->Write(HTML_CODE_OPEN);
					m_pOutput->WriteChar('~', m_Parser.GetTokenCount() - 2);
					}
				else
					m_pOutput->WriteChar('~', m_Parser.GetTokenCount());
				break;

			case CTextMarkupParser::tokenBackQuote:
				if (IsTopStyle(CTextMarkupParser::tokenTilde))
					m_pOutput->WriteChar('`', m_Parser.GetTokenCount());
				else if (m_Parser.GetTokenCount() >= 2 && IsTopStyle(CTextMarkupParser::tokenBackQuote))
					{
					m_pOutput->WriteChar('`', m_Parser.GetTokenCount() - 2);
					m_pOutput->Write(HTML_STRIKE_CLOSE);
					PopStyle();
					}
				else if (m_Parser.GetTokenCount() >= 2
						&& !strIsWhitespace(m_Parser.PeekNextChar())
						&& !IsStyleInStack(CTextMarkupParser::tokenBackQuote)
						&& FindTextEnd(CTextMarkupParser::tokenBackQuote))
					{
					PushStyle(CTextMarkupParser::tokenBackQuote);
					m_pOutput->Write(HTML_STRIKE_OPEN);
					m_pOutput->WriteChar('`', m_Parser.GetTokenCount() - 2);
					}
				else
					m_pOutput->WriteChar('`', m_Parser.GetTokenCount());
				break;

			case CTextMarkupParser::tokenCloseBracket:
				if (m_bInLink)
					{
					m_pOutput->WriteChar(']', m_Parser.GetTokenCount() - 2);
					bDone = true;
					}
				else
					m_pOutput->WriteChar(']', m_Parser.GetTokenCount());
				break;

			case CTextMarkupParser::tokenOpenBrace:
				if (IsTopStyle(CTextMarkupParser::tokenTilde))
					OutputRepeatingChar(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
				else if (!m_bInLink && m_Parser.GetTokenCount() == 2)
					TemplateToHTML();
				else
					OutputRepeatingChar(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
				break;

			case CTextMarkupParser::tokenOpenBracket:
				if (IsTopStyle(CTextMarkupParser::tokenTilde))
					OutputRepeatingChar(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
				else if (!m_bInLink && m_Parser.GetTokenCount() == 2)
					LinkToHTML();
				else
					OutputRepeatingChar(m_Parser.GetTokenChar(), m_Parser.GetTokenCount());
				break;

			case CTextMarkupParser::tokenEoS:
			case CTextMarkupParser::tokenLineEnd:
				bDone = true;
				break;

			default:
				{
				char chTokenChar = m_Parser.GetTokenChar();

				if (chTokenChar == '='
						&& !m_bInLink
						&& chTokenChar == chTerminator
						&& m_Parser.GetTokenCount() >= 2
						&& m_Parser.GetTokenCount() <= (MAX_HEADER_LEVEL + 1)
						&& IsHeaderEnd()) 
					//	Skip
					NULL;
				else if (chTokenChar != '\0')
					OutputRepeatingChar(chTokenChar, m_Parser.GetTokenCount());
				break;
				}
			}

		//	Whatever we get, we don't need to skip leading whitespace anymore

		bSkipWhitespace = false;

		//	Get the next token.

		if (!bDone)
			m_Parser.ParseNextToken();
		}

	//	Close styles, in case the user forgot to terminate one of them. Note 
	//	that we have different stacks for in link vs. out of a link.

	CloseAllStyles();
	}
