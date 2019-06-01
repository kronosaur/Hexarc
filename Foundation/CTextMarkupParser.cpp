//	CTextMarkupParser.cpp
//
//	CTextMarkupParser Class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

static char SPECIAL_CHARS[] = "-_*/#=[<{}>]\'\"\\|~`";

inline bool IsURLStartDelimeter (char chChar) { return (chChar == ' ' || chChar == '\t' || chChar == '('); }

CTextMarkupParser::CTextMarkupParser (void) :
		m_pPos(""),
		m_pPosEnd(m_pPos)

//	CTextMarkupParser constructor

	{
	}

char CTextMarkupParser::GetTokenChar (void) const

//	GetTokenChar
//
//	Returns the character for the current token

	{
	switch (m_iToken)
		{
		case tokenNone:
		case tokenError:
		case tokenEoS:
		case tokenText:
		case tokenURL:
			return '\0';

		case tokenLineEnd:
			return '\n';

		case tokenSingleQuote:
			return '\'';

		case tokenDoubleQuote:
			return '\"';

		case tokenSlash:
			return '/';

		case tokenStar:
			return '*';

		case tokenHash:
			return '#';

		case tokenEquals:
			return '=';

		case tokenOpenBracket:
			return '[';

		case tokenCloseBracket:
			return ']';

		case tokenOpenBrace:
			return '{';

		case tokenCloseBrace:
			return '}';

		case tokenLessThan:
			return '<';

		case tokenGreaterThan:
			return '>';

		case tokenDash:
			return '-';

		case tokenUnderscore:
			return '_';

		case tokenBackslash:
			return '\\';

		case tokenVerticalBar:
			return '|';

		case tokenTilde:
			return '~';

		case tokenBackQuote:
			return '`';

		default:
			return '\0';
		}
	}

bool CTextMarkupParser::IsBasicLink (const char *pPos, const char *pPosEnd)

//	IsBasicLink
//
//	Returns TRUE if pPos points to the beginning of a well-known URL scheme,
//	such as http: or https:

	{
	//	Start with 'h'

	if (pPos >= pPosEnd || (*pPos != 'h' && *pPos != 'H'))
		return false;

	//	't'

	pPos++;
	if (pPos >= pPosEnd || (*pPos != 't' && *pPos != 'T'))
		return false;

	//	't'

	pPos++;
	if (pPos >= pPosEnd || (*pPos != 't' && *pPos != 'T'))
		return false;

	//	'p'

	pPos++;
	if (pPos >= pPosEnd || (*pPos != 'p' && *pPos != 'P'))
		return false;

	//	':' or 's'

	pPos++;
	if (pPos >= pPosEnd || (*pPos != ':' && *pPos != 's' && *pPos != 'S'))
		return false;

	//	If we're at 's' then the next must be a colon

	if (*pPos == 's' || *pPos == 'S')
		{
		pPos++;
		if (pPos >= pPosEnd || (*pPos != ':'))
			return false;
		}

	//	Two slashes

	pPos++;
	if (pPos >= pPosEnd || (*pPos != '/'))
		return false;

	pPos++;
	if (pPos >= pPosEnd || (*pPos != '/'))
		return false;

	//	Done

	return true;
	}

bool CTextMarkupParser::IsEntity (const char *pPos, const char *pPosEnd)

//	IsEntity
//
//	Returns TRUE if pPos is pointing to an HTML entity.

	{
	//	Start with '&'

	if (pPos >= pPosEnd || (*pPos != '&'))
		return false;

	//	Parse valid characters until we get either a semi-colon, or and invalid
	//	character.

	pPos++;
	bool bValid = false;
	while (pPos < pPosEnd)
		{
		if (*pPos == ';')
			{
			bValid = true;
			break;
			}
		else if ((*pPos >= 'A' && *pPos <='Z')
				|| (*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= '0' && *pPos <= '9')
				|| (*pPos == '#'))
			pPos++;
		else
			break;
		}

	return bValid;
	}

bool CTextMarkupParser::IsSpecialChar (char chChar)

//	IsSpecialChar
//
//	Returns TRUE if this is a special character

	{
	char *pPos = SPECIAL_CHARS;
	while (*pPos != '\0' && *pPos != chChar)
		pPos++;

	return (*pPos != '\0');
	}

void CTextMarkupParser::ParseExtensions (TArray<SExtensionDesc> *retExtensions)

//	ParseExtensions
//
//	Generates a list of extensions in the current text

	{
	enum EStates
		{
		stateStart,
		stateFoundOpenBracket,
		stateFoundOpenLink,
		stateFoundOpenBasicLink,
		stateFoundLinkName,
		stateFoundCloseBracket,
		stateFoundCloseBasicLinkBracket,
		stateFoundOpenBrace,
		stateFoundOpenTemplate,
		stateFoundTemplateName,
		stateFoundKeyName,
		stateFoundValueName,
		stateFoundCloseBrace,
		};

	EStates iState = stateStart;
	SExtensionDesc *pCurrent = NULL;
	const char *pStart;
	EStates iOldState;
	int iCurParam;
	CString sCurKey;
	while (m_pPos < m_pPosEnd)
		{
		switch (iState)
			{
			case stateStart:
				if (*m_pPos == '[')
					iState = stateFoundOpenBracket;
				else if (*m_pPos == '{')
					iState = stateFoundOpenBrace;
				break;

			case stateFoundOpenBracket:
				if (*m_pPos == '[')
					{
					//	If the link starts with well-known URL schemes (http: or https:) 
					//	then we let the base parser handle it.

					if (IsBasicLink(m_pPos + 1, m_pPosEnd))
						iState = stateFoundOpenBasicLink;

					//	Otherwise we add this to our list of extensions

					else
						{
						iState = stateFoundOpenLink;
						pCurrent = retExtensions->Insert();
						pCurrent->iType = typeLink;
						pStart = m_pPos + 1;
						}
					}
				else if (*m_pPos == '{')
					iState = stateFoundOpenBrace;
				else
					iState = stateStart;
				break;

			case stateFoundOpenBasicLink:
				if (*m_pPos == ']')
					iState = stateFoundCloseBasicLinkBracket;
				break;

			case stateFoundCloseBasicLinkBracket:
				if (*m_pPos == ']')
					iState = stateStart;
				else
					iState = stateFoundOpenBasicLink;
				break;

			case stateFoundOpenLink:
				if (*m_pPos == ']')
					{
					iOldState = iState;
					iState = stateFoundCloseBracket;
					}
				else if (*m_pPos == '|')
					{
					pCurrent->sName = CString(pStart, (int)(m_pPos - pStart));
					iState = stateFoundLinkName;
					iCurParam = 0;
					pStart = m_pPos + 1;
					}
				break;

			case stateFoundLinkName:
				if (*m_pPos == ']')
					{
					iOldState = iState;
					iState = stateFoundCloseBracket;
					}
				else if (*m_pPos == '|')
					{
					pCurrent->Params.Insert(strFromInt(iCurParam), CString(pStart, (int)(m_pPos - pStart)));
					iCurParam++;
					pStart = m_pPos + 1;
					}
				break;

			case stateFoundCloseBracket:
				if (*m_pPos == ']')
					{
					if (iOldState == stateFoundOpenLink)
						pCurrent->sName = CString(pStart, (int)(m_pPos - pStart) - 1);
					else
						pCurrent->Params.Insert(strFromInt(iCurParam), CString(pStart, (int)(m_pPos - pStart) - 1));

					iState = stateStart;
					}
				else
					iState = iOldState;
				break;

			case stateFoundOpenBrace:
				if (*m_pPos == '{')
					{
					iState = stateFoundOpenTemplate;
					pCurrent = retExtensions->Insert();
					pCurrent->iType = typeTemplate;
					pStart = m_pPos + 1;
					}
				else if (*m_pPos == '[')
					iState = stateFoundOpenBracket;
				else
					iState = stateStart;
				break;

			case stateFoundOpenTemplate:
				if (*m_pPos == '}')
					{
					iOldState = iState;
					iState = stateFoundCloseBrace;
					}
				else if (*m_pPos == '|')
					{
					pCurrent->sName = CString(pStart, (int)(m_pPos - pStart));
					iState = stateFoundTemplateName;
					iCurParam = 0;
					pStart = m_pPos + 1;
					}
				break;

			case stateFoundTemplateName:
				if (*m_pPos == '}')
					{
					iOldState = iState;
					iState = stateFoundCloseBrace;
					}
				else if (*m_pPos == '=')
					{
					sCurKey = CString(pStart, (int)(m_pPos - pStart));
					if (sCurKey.IsEmpty())
						sCurKey = strFromInt(iCurParam);

					pStart = m_pPos + 1;
					iState = stateFoundKeyName;
					}
				else if (*m_pPos == '|')
					{
					pCurrent->Params.Insert(strFromInt(iCurParam), CString(pStart, (int)(m_pPos - pStart)));
					iCurParam++;
					pStart = m_pPos + 1;
					}
				break;

			case stateFoundKeyName:
				if (*m_pPos == '}')
					{
					iOldState = iState;
					iState = stateFoundCloseBrace;
					}
				else if (*m_pPos == '|')
					{
					pCurrent->Params.Insert(sCurKey, CString(pStart, (int)(m_pPos - pStart)));
					iCurParam++;
					pStart = m_pPos + 1;
					iState = stateFoundTemplateName;
					}
				break;

			case stateFoundCloseBrace:
				if (*m_pPos == '}')
					{
					if (iOldState == stateFoundOpenTemplate)
						pCurrent->sName = CString(pStart, (int)(m_pPos - pStart) - 1);
					else if (iOldState == stateFoundTemplateName)
						pCurrent->Params.Insert(strFromInt(iCurParam), CString(pStart, (int)(m_pPos - pStart) - 1));
					else if (iOldState == stateFoundKeyName)
						pCurrent->Params.Insert(sCurKey, CString(pStart, (int)(m_pPos - pStart) - 1));

					iState = stateStart;
					}
				else
					iState = iOldState;
				break;
			}

		m_pPos++;
		}
	}

CTextMarkupParser::ETokens CTextMarkupParser::ParseNextToken (void)

//	ParseNextToken
//
//	Parses and returns the next token.

	{
	enum EStates
		{
		stateStart,
		stateDone,

		stateSpecialChar,
		stateCR,
		stateLF,
		stateLineEnd,
		stateText,
		stateEscapeChar,
		stateEscapeChar2,

		stateText_urlCheck,

		stateURL_h0,
		stateURL_t1,
		stateURL_t2,
		stateURL_p3,
		stateURL_s4,
		stateURL_colon,
		stateURL_slash1,
		stateURL_slash2,
		stateURL_remainder,
		stateURL_lastChar,

		stateEntity,
		stateEntity_done,
		};

	//	Reset state

	m_iToken = tokenEoS;
	m_iTokenCount = 0;
	m_sToken = NULL_STR;
	const char *pStart;
	char chSpecialChar;
	int iPeek = 0;

	//	Parse

	EStates iState = stateStart;
	while (iState != stateDone)
		{
		switch (iState)
			{
			case stateStart:
				{
				if (m_pPos == m_pPosEnd)
					{
					m_iToken = tokenEoS;
					iState = stateDone;
					break;
					}

				switch (*m_pPos)
					{
					case '\n':
						iState = stateLF;
						break;

					case '\r':
						iState = stateCR;
						break;

					case '\\':
						iState = stateEscapeChar;
						break;

					default:
						if (IsSpecialChar(*m_pPos))
							{
							chSpecialChar = *m_pPos;
							m_iTokenCount = 1;
							iState = stateSpecialChar;
							}
						else if (*m_pPos == 'h' || *m_pPos == 'H')
							{
							iState = stateURL_h0;
							pStart = m_pPos;
							}
						else if (IsURLStartDelimeter(*m_pPos))
							{
							iState = stateText_urlCheck;
							pStart = m_pPos;
							}
						else if (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd))
							{
							iState = stateEntity;
							pStart = m_pPos;
							}
						else
							{
							iState = stateText;
							pStart = m_pPos;
							}
						break;
					}

				break;
				}

			case stateText:
				{
				if (m_pPos == m_pPosEnd
						|| IsSpecialChar(*m_pPos)
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (IsURLStartDelimeter(*m_pPos))
					iState = stateText_urlCheck;

				break;
				}

			case stateText_urlCheck:
				{
				if (m_pPos == m_pPosEnd
						|| IsSpecialChar(*m_pPos)
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd))
						|| IsBasicLink(m_pPos, m_pPosEnd))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (IsURLStartDelimeter(*m_pPos))
					{ }
				else
					iState = stateText;

				break;
				}

			case stateEscapeChar:
				if (m_pPos < m_pPosEnd)
					{
					m_iToken = tokenEscapeText;
					m_sToken = CString(m_pPos, 1);
					iState = stateEscapeChar2;
					}
				else
					iState = stateDone;
				break;

			case stateEscapeChar2:
				iState = stateDone;
				break;

			case stateSpecialChar:
				if (m_pPos == m_pPosEnd	|| *m_pPos != chSpecialChar)
					{
					m_iToken = TokenFromSpecialChar(chSpecialChar);
					iState = stateDone;
					}
				else
					m_iTokenCount++;
				break;

			case stateCR:
				{
				if (m_pPos == m_pPosEnd)
					{
					m_iToken = tokenLineEnd;
					m_iTokenCount++;
					iState = stateDone;
					}
				else if (*m_pPos == '\r')
					{
					m_iTokenCount += 2;
					iState = stateLineEnd;
					}
				else if (*m_pPos == '\n')
					{
					m_iTokenCount++;
					iState = stateLineEnd;
					}
				else
					{
					m_iToken = tokenLineEnd;
					m_iTokenCount++;
					iState = stateDone;
					}

				break;
				}

			case stateLF:
				{
				if (m_pPos == m_pPosEnd)
					{
					m_iToken = tokenLineEnd;
					m_iTokenCount++;
					iState = stateDone;
					}
				else if (*m_pPos == '\r')
					{
					m_iTokenCount++;
					iState = stateLineEnd;
					}
				else if (*m_pPos == '\n')
					{
					m_iTokenCount += 2;
					iState = stateLineEnd;
					}
				else
					{
					m_iToken = tokenLineEnd;
					m_iTokenCount++;
					iState = stateDone;
					}

				break;
				}

			case stateLineEnd:
				{
				if (m_pPos == m_pPosEnd)
					{
					m_iToken = tokenLineEnd;
					iState = stateDone;
					}
				else if (*m_pPos == '\r')
					iState = stateCR;
				else if (*m_pPos == '\n')
					iState = stateLF;
				else
					{
					m_iToken = tokenLineEnd;
					iState = stateDone;
					}

				break;
				}

			//	Entities

			case stateEntity:
				if (m_pPos == m_pPosEnd)
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == ';')
					iState = stateEntity_done;
				else if ((*m_pPos >= 'A' && *m_pPos <= 'Z')
						 || (*m_pPos >= 'a' && *m_pPos <= 'z')
						 || (*m_pPos >= '0' && *m_pPos <= '9')
						 || (*m_pPos == '#'))
					{ }
				else
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				break;

			case stateEntity_done:
				{
				m_iToken = tokenEntity;
				m_sToken = CString(pStart, (int)(m_pPos - pStart));
				iState = stateDone;
				break;
				}

			//	URLs

			case stateURL_h0:
				if (m_pPos == m_pPosEnd
						|| IsSpecialChar(*m_pPos)
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == 't' || *m_pPos == 'T')
					iState = stateURL_t1;
				else
					iState = stateText;
				break;

			case stateURL_t1:
				if (m_pPos == m_pPosEnd
						|| IsSpecialChar(*m_pPos)
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == 't' || *m_pPos == 'T')
					iState = stateURL_t2;
				else
					iState = stateText;
				break;

			case stateURL_t2:
				if (m_pPos == m_pPosEnd
						|| IsSpecialChar(*m_pPos)
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == 'p' || *m_pPos == 'P')
					iState = stateURL_p3;
				else
					iState = stateText;
				break;

			case stateURL_p3:
				if (m_pPos == m_pPosEnd
						|| IsSpecialChar(*m_pPos)
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == ':')
					iState = stateURL_colon;
				else if (*m_pPos == 's' || *m_pPos == 'S')
					iState = stateURL_s4;
				else
					iState = stateText;
				break;

			case stateURL_s4:
				if (m_pPos == m_pPosEnd
						|| IsSpecialChar(*m_pPos)
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == ':')
					iState = stateURL_colon;
				else
					iState = stateText;
				break;

			case stateURL_colon:
				if (m_pPos == m_pPosEnd
						|| (*m_pPos != '/' && IsSpecialChar(*m_pPos))
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == '/')
					iState = stateURL_slash1;
				else
					iState = stateText;
				break;

			case stateURL_slash1:
				if (m_pPos == m_pPosEnd
						|| (*m_pPos != '/' && IsSpecialChar(*m_pPos))
						|| *m_pPos == '\n'
						|| *m_pPos == '\r'
						|| (*m_pPos == '&' && IsEntity(m_pPos, m_pPosEnd)))
					{
					m_iToken = tokenText;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == '/')
					iState = stateURL_remainder;
				else
					iState = stateText;
				break;

			case stateURL_remainder:
				if (m_pPos == m_pPosEnd)
					{
					m_iToken = tokenURL;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				else if (*m_pPos == '.'
						|| *m_pPos == ','
						|| *m_pPos == '?'
						|| *m_pPos == '!'
						|| *m_pPos == ';'
						|| *m_pPos == ':'
						|| *m_pPos == ')')
					{
					//	This set of characters is valid inside an URL, but we 
					//	ignore it if it's the last character (since it is likely to 
					//	be punctuation instead).

					iPeek = 1;
					iState = stateURL_lastChar;
					}
				else if ((*m_pPos >= 'A' && *m_pPos <= 'Z')
						|| (*m_pPos >= 'a' && *m_pPos <= 'z')
						|| (*m_pPos >= '0' && *m_pPos <= '9')
						|| *m_pPos == '-'
						|| *m_pPos == '_'
						|| *m_pPos == '~'
						|| *m_pPos == ':'
						|| *m_pPos == '/'
						|| *m_pPos == '#'
						|| *m_pPos == '@'
						|| *m_pPos == '$'
						|| *m_pPos == '&'
						|| *m_pPos == '('
						|| *m_pPos == '*'
						|| *m_pPos == '+'
						|| *m_pPos == '='
						|| *m_pPos == '%')

						//	NOTE: Brackets are normally allowed in a URL, but we 
						//	disallow them here since we use them as a terminator.
						//
						//	We also disallow single-quotes, which could be used
						//	escape out of an href attribute.
					{ }
				else
					{
					m_iToken = tokenURL;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				break;

			case stateURL_lastChar:
				//	If we hit the end, then we exclude the last character

				if (m_pPos == m_pPosEnd)
					{
					m_pPos -= iPeek;
					m_iToken = tokenURL;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}

				//	If we have another ambiguous character, then continue peeking

				else if (*m_pPos == '.'
						|| *m_pPos == ','
						|| *m_pPos == '?'
						|| *m_pPos == '!'
						|| *m_pPos == ';'
						|| *m_pPos == ':'
						|| *m_pPos == ')')
					iPeek++;

				//	If we have a valid URL character then we're still inside an URL.

				else if ((*m_pPos >= 'A' && *m_pPos <= 'Z')
						|| (*m_pPos >= 'a' && *m_pPos <= 'z')
						|| (*m_pPos >= '0' && *m_pPos <= '9')
						|| *m_pPos == '-'
						|| *m_pPos == '_'
						|| *m_pPos == '~'
						|| *m_pPos == ':'
						|| *m_pPos == '/'
						|| *m_pPos == '#'
						|| *m_pPos == '@'
						|| *m_pPos == '$'
						|| *m_pPos == '&'
						|| *m_pPos == '('
						|| *m_pPos == '*'
						|| *m_pPos == '+'
						|| *m_pPos == '='
						|| *m_pPos == '%')
					iState = stateURL_remainder;

				//	Otherwise, we're done with the URL

				else
					{
					m_pPos -= iPeek;
					m_iToken = tokenURL;
					m_sToken = CString(pStart, (int)(m_pPos - pStart));
					iState = stateDone;
					}
				break;
			}

		if (iState != stateDone)
			m_pPos++;
		}

	//	Done

	return m_iToken;
	}

CTextMarkupParser::ETokens CTextMarkupParser::PeekNextToken (CString *retsToken, int *retiTokenCount)

//	PeekNextToken
//
//	Returns the next token without advancing the current parse position

	{
	//	Save all state

	const char *pOldPos = m_pPos;
	const char *pOldPosEnd = m_pPosEnd;
	ETokens iOldToken = m_iToken;
	int iOldTokenCount = m_iTokenCount;

	//	We keep the original pointer in sOldToken because others might have
	//	a pointer to it and we don't want to change it.

	CString sOldToken;
	sOldToken.TakeHandoff(m_sToken);
	m_sToken = sOldToken;

	//	Peek

	ETokens iPeekToken = ParseNextToken();
	if (retsToken)
		*retsToken = m_sToken;
	if (retiTokenCount)
		*retiTokenCount = m_iTokenCount;

	//	Restore

	m_pPos = pOldPos;
	m_pPosEnd = pOldPosEnd;
	m_iToken = iOldToken;
	m_iTokenCount = iOldTokenCount;
	m_sToken.TakeHandoff(sOldToken);

	//	Done

	return iPeekToken;
	}

void CTextMarkupParser::RestoreParser (CTextMarkupParser &Saved)

//	RestoreParser
//
//	Restores the parser state

	{
	m_pPos = Saved.m_pPos;
	m_pPosEnd = Saved.m_pPosEnd;
	m_iToken = Saved.m_iToken;
	m_iTokenCount = Saved.m_iTokenCount;
	m_sToken.TakeHandoff(Saved.m_sToken);
	}

void CTextMarkupParser::SaveParser (CTextMarkupParser *retSaved)

//	SaveParser
//
//	Saves the current parser state in the given variable.

	{
	retSaved->m_pPos = m_pPos;
	retSaved->m_pPosEnd = m_pPosEnd;
	retSaved->m_iToken = m_iToken;
	retSaved->m_iTokenCount = m_iTokenCount;

	retSaved->m_sToken.TakeHandoff(m_sToken);
	m_sToken = retSaved->m_sToken;
	}

void CTextMarkupParser::SetInput (const char *pPos, const char *pPosEnd)

//	SetInput
//
//	Sets the input string.

	{
	//	Skip all leading whitespace

	while (pPos < pPosEnd && strIsWhitespace(pPos))
		pPos++;

	//	Skip all trailing whitespace

	while (pPosEnd > pPos && strIsWhitespace(pPosEnd - 1))
		pPosEnd--;

	//	Input

	m_pPos = pPos;
	m_pPosEnd = pPosEnd;
	}

CTextMarkupParser::ETokens CTextMarkupParser::TokenFromSpecialChar (char chChar)

//	TokenFromSpecialChar
//
//	Returns the token from the special character

	{
	switch (chChar)
		{
		case '-':
			return tokenDash;

		case '_':
			return tokenUnderscore;

		case '\'':
			return tokenSingleQuote;

		case '\"':
			return tokenDoubleQuote;

		case '/':
			return tokenSlash;

		case '*':
			return tokenStar;

		case '#':
			return tokenHash;

		case '=':
			return tokenEquals;

		case '[':
			return tokenOpenBracket;

		case ']':
			return tokenCloseBracket;

		case '{':
			return tokenOpenBrace;

		case '}':
			return tokenCloseBrace;

		case '<':
			return tokenLessThan;

		case '>':
			return tokenGreaterThan;

		case '\\':
			return tokenBackslash;

		case '|':
			return tokenVerticalBar;

		case '~':
			return tokenTilde;

		case '`':
			return tokenBackQuote;

		default:
			return tokenError;
		}
	}
