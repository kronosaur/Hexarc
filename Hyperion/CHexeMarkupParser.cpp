//	CHexeMarkupParser.cpp
//
//	CHexeMarkupParser class
//	Copyright (c) 2012 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ELEMENT_SCRIPT,					"<script>")

DECLARE_CONST_STRING(ERR_INVALID_DIRECTIVE,				"Invalid character in directive.")
DECLARE_CONST_STRING(ERR_INVALID_TAG,					"Invalid character in tag.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_CLOSE_ANGLE_BRACKET,	"Unexpected character: '>'.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_OPEN_ANGLE_BRACKET,	"Unexpected character: '<'.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_EOS,				"Unexpected end of stream.")

void CHexeMarkupParser::Init (const char *pPos, int iLength)

//	Init
//
//	Initialize parser

	{
	m_pPos = pPos;
	m_pPosEnd = pPos + iLength;

	m_iToken = tkEoS;
	m_sKey = NULL_STR;
	m_sValue = NULL_STR;
	}

CHexeMarkupParser::Tokens CHexeMarkupParser::ParseToken (CString *retsKey, CString *retsValue)

//	ParseToken
//
//	Returns a token

	{
	m_iToken = tkEoS;
	m_sKey = NULL_STR;
	m_sValue = NULL_STR;

	States iState = ((m_pPos && m_pPos < m_pPosEnd) ? stateStart : stateEnd);

	while (iState != stateEnd)
		{
		switch (iState)
			{
			//	Start of token. Figure out what we've got.

			case stateStart:
				if (*m_pPos == '<')
					{
					iState = stateOpenAngleBracket;
					m_pStart = m_pPos++;
					}
				else if (*m_pPos == '>')
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_CLOSE_ANGLE_BRACKET;
					}
				else
					{
					if (m_bCDATAText)
						{
						iState = stateTextCDATA;
						m_bCDATAText = false;
						}
					else
						iState = stateText;

					m_pStart = m_pPos++;
					}
				break;

			//	We've accepted "<". Now look for the next character to see what
			//	kind of element this is.

			case stateOpenAngleBracket:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '/')
					{
					iState = stateCloseTag;
					m_pPos++;
					}
				else if (*m_pPos == '?')
					{
					iState = statePIStart;
					m_pPos++;
					}
				else if (*m_pPos == '!')
					{
					iState = stateDirectiveStart;
					m_pPos++;
					}
				else
					{
					iState = stateOpenTag;
					m_pPos++;
					}
				break;

			//	We've accepted "<xyz". Look for the end of the tag.

			case stateOpenTag:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '/')
					{
					iState = stateCloseElement;
					m_pPos++;
					}
				else if (*m_pPos == '>')
					{
					iState = stateEnd;
					m_iToken = tkStartTag;
					m_sValue = CString(m_pStart, (int)(++m_pPos - m_pStart));

					//	If this is a script tag, then the next text is CDATA

					if (strEquals(strToLower(m_sValue), ELEMENT_SCRIPT))
						m_bCDATAText = true;
					}
				else if (*m_pPos == '<')
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_OPEN_ANGLE_BRACKET;
					}
				else
					m_pPos++;
				break;

			//	We've accepted "<xyz/". Look for the final close.

			case stateCloseElement:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '>')
					{
					iState = stateEnd;
					m_iToken = tkEmptyElement;
					m_sValue = CString(m_pStart, (int)(++m_pPos - m_pStart));
					}
				else
					{
					iState = stateOpenTag;
					m_pPos++;
					}
				break;

			//	We've accepted "</". Now we need to look for the terminating
			//	close angle bracket.

			case stateCloseTag:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '>')
					{
					iState = stateEnd;
					m_iToken = tkEndTag;
					m_sValue = CString(m_pStart, (int)(++m_pPos - m_pStart));
					}
				else
					m_pPos++;
				break;

			//	We've accepted "<?". Now look for the processing instruction
			//	tag.

			case statePIStart:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (strIsWhitespace(m_pPos) || strIsASCIIControl(m_pPos) || (*m_pPos != ':' && *m_pPos != '_' && strIsASCIISymbol(m_pPos)))
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_INVALID_TAG;
					}
				else
					{
					iState = statePITag;
					m_pStart = m_pPos++;
					}
				break;

			//	We've accepted "<?xyz". Continue until we find whitespace.

			case statePITag:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (strIsWhitespace(m_pPos))
					{
					iState = statePIBody;
					m_sKey = CString(m_pStart, (int)(m_pPos - m_pStart));
					m_pStart = ++m_pPos;
					}
				else if (*m_pPos == '?')
					{
					iState = statePIQuestion;
					m_sKey = strToLower(CString(m_pStart, (int)(m_pPos - m_pStart)));
					m_pStart = m_pPos++;
					}
				else
					m_pPos++;
				break;

			//	We've accepted "<?xyx abc".

			case statePIBody:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '?')
					{
					iState = statePIQuestion;
					m_pPos++;
					}
				else
					m_pPos++;
				break;

			//	We've accepted "<?xyz abc?".

			case statePIQuestion:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '>')
					{
					iState = stateEnd;
					m_iToken = tkPI;
					m_sValue = CString(m_pStart, (int)((m_pPos - 1) - m_pStart));
					m_pPos++;
					}
				else
					{
					iState = statePIBody;
					m_pPos++;
					}
				break;

			//	We've accepted "<!". Now look for the rest.

			case stateDirectiveStart:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '-')
					{
					iState = stateCommentFirst;
					m_pPos++;
					}
				else
					{
					iState = stateDirective;
					m_pPos++;
					}
				break;

			//	We've accepted "<!xyz". Look for the end

			case stateDirective:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '>')
					{
					iState = stateEnd;
					m_iToken = tkDirective;
					m_sValue = CString(m_pStart, (int)(++m_pPos - m_pStart));
					}
				else
					m_pPos++;
				break;

			//	We've accepted "<!-". Now see if this is a comment.

			case stateCommentFirst:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '-')
					{
					iState = stateComment;
					m_pPos++;
					}
				else
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_INVALID_DIRECTIVE;
					}
				break;

			//	We've accepted "<!--xyz". Look for the end of the comment.

			case stateComment:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '-')
					{
					iState = stateCommentEndFirst;
					m_pPos++;
					}
				else
					m_pPos++;
				break;

			//	We've accepted "<!--xyz-". Look for end of comment

			case stateCommentEndFirst:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '-')
					{
					iState = stateCommentEndSecond;
					m_pPos++;
					}
				else
					{
					iState = stateComment;
					m_pPos++;
					}
				break;

			//	We've accepted "<!--xyz--". Look for the final close.

			case stateCommentEndSecond:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkError;
					m_sValue = ERR_UNEXPECTED_EOS;
					}
				else if (*m_pPos == '>')
					{
					iState = stateEnd;
					m_iToken = tkComment;
					m_sValue = CString(m_pStart, (int)(++m_pPos - m_pStart));
					}
				else if (*m_pPos == '-')
					m_pPos++;
				else
					{
					iState = stateComment;
					m_pPos++;
					}
				break;

			//	We're parsing text. Continue until the end bracket.

			case stateText:
				if (m_pPos == m_pPosEnd || *m_pPos == '<')
					{
					iState = stateEnd;
					m_iToken = tkText;
					m_sValue = CString(m_pStart, (int)(m_pPos - m_pStart));
					}
				else
					m_pPos++;
				break;

			case stateTextCDATA:
				if (m_pPos == m_pPosEnd)
					{
					iState = stateEnd;
					m_iToken = tkText;
					m_sValue = CString(m_pStart, (int)(m_pPos - m_pStart));
					}
				else if (*m_pPos == '<')
					{
					//	Peek ahead to see if we've got an end of element tag. If
					//	so then we've reached the end.

					if (m_pPos + 1 >= m_pPosEnd || m_pPos[1] == '/')
						{
						iState = stateEnd;
						m_iToken = tkText;
						m_sValue = CString(m_pStart, (int)(m_pPos - m_pStart));
						}

					//	Otherwise, continue

					else
						m_pPos++;
					}
				else
					m_pPos++;
				break;

			default:
				ASSERT(false);
			}
		}

	//	Done

	if (retsKey)
		*retsKey = m_sKey;

	if (retsValue)
		*retsValue = m_sValue;

	return m_iToken;
	}
