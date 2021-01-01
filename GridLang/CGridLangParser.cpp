//	CGridLangParser.cpp
//
//	CGridLangParser Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(ERR_UNEXPECTED_CHARACTER,			"Unexpected character '%s'");
DECLARE_CONST_STRING(ERR_NO_CLOSE_QUOTE,				"Quoted string started on line %d never ended.");

CString CGridLangParser::ComposeError (const CString &sError) const

//	ComposeError
//
//	Compose an error.

	{
	return strPattern("ERROR(%d): %s", m_iLine, sError);
	}

bool CGridLangParser::HasToken () const

//	HasToken
//
//	Returns TRUE if we have a token.

	{
	const char *pPos = m_pPos;
	while (pPos < m_pPosEnd)
		{
		if (*pPos == '#')
			{
			while (pPos < m_pPosEnd && *pPos != '\n')
				pPos++;
			}
		else if (strIsWhitespace(pPos))
			pPos++;
		else
			{
			return true;
			}
		}

	return false;
	}

EGridLangToken CGridLangParser::PeekToken (CString *retsValue) const

//	PeekToken
//
//	Peeks ahead.

	{
	CGridLangParser Peeker;
	Peeker.m_pPos = m_pPos;
	Peeker.m_pPosEnd = m_pPosEnd;

	Peeker.NextToken();
	if (retsValue)
		*retsValue = Peeker.GetTokenValue();

	return Peeker.GetToken();
	}

EGridLangToken CGridLangParser::NextToken ()

//	NextToken
//
//	Parses the next token and returns it. If we hit the end of stream, we return
//	Null.

	{
	enum class EState
		{
		Start,

		Comment,
		Identifier,
		QuotedString,
		OtherString,

		Number0,
		Number,
		NumberHex,
		NumberFloat,
		NumberExponent,
		NumberExponentValue,

		Equals,
		Amp,
		Bar,
		Bang,
		GreaterThan,
		LessThan,

		Done,
		};

	EState iState = EState::Start;
	const char *pStart = NULL;
	int iStartLine = 0;

	while (iState != EState::Done)
		{
		switch (iState)
			{
			case EState::Start:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Null);
					}

				//	If whitespace, continue.

				else if (*m_pPos == ' ' || *m_pPos == '\t' || *m_pPos == '\r')
					{ }

				//	If line-break, keep line count.

				else if (*m_pPos == '\n')
					m_iLine++;

				//	Comment

				else if (*m_pPos == '#')
					iState = EState::Comment;

				//	Quoted string

				else if (*m_pPos == '\"')
					{
					pStart = m_pPos + 1;
					iStartLine = m_iLine;
					iState = EState::QuotedString;
					}

				//	Identifier

				else if (::strIsASCIIAlpha(m_pPos) || *m_pPos == '_')
					{
					pStart = m_pPos;
					iState = EState::Identifier;
					}

				//	Number

				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					pStart = m_pPos;
					if (*m_pPos == '0')
						iState = EState::Number0;
					else
						iState = EState::Number;
					}

				//	Else it's a symbol of some sort

				else
					{
					switch (*m_pPos)
						{
						case '!':
							iState = EState::Bang;
							break;

						case '@':
							return SetToken(EGridLangToken::At, CString(m_pPos++, 1));

						case '$':
							return SetToken(EGridLangToken::Dollar, CString(m_pPos++, 1));

						case '%':
							return SetToken(EGridLangToken::Percent, CString(m_pPos++, 1));

						case '^':
							return SetToken(EGridLangToken::Caret, CString(m_pPos++, 1));

						case '&':
							iState = EState::Amp;
							break;

						case '*':
							return SetToken(EGridLangToken::Star, CString(m_pPos++, 1));

						case '(':
							return SetToken(EGridLangToken::OpenParen, CString(m_pPos++, 1));

						case ')':
							return SetToken(EGridLangToken::CloseParen, CString(m_pPos++, 1));

						case '-':
							return SetToken(EGridLangToken::Minus, CString(m_pPos++, 1));

						case '+':
							return SetToken(EGridLangToken::Plus, CString(m_pPos++, 1));

						case '=':
							iState = EState::Equals;
							break;

						case '{':
							return SetToken(EGridLangToken::OpenBrace, CString(m_pPos++, 1));

						case '}':
							return SetToken(EGridLangToken::CloseBrace, CString(m_pPos++, 1));

						case '[':
							return SetToken(EGridLangToken::OpenBracket, CString(m_pPos++, 1));

						case ']':
							return SetToken(EGridLangToken::CloseBracket, CString(m_pPos++, 1));

						case '|':
							iState = EState::Bar;
							break;

						case '\\':
							return SetToken(EGridLangToken::BackSlash, CString(m_pPos++, 1));

						case ':':
							return SetToken(EGridLangToken::Colon, CString(m_pPos++, 1));

						case ';':
							return SetToken(EGridLangToken::SemiColon, CString(m_pPos++, 1));

						case '<':
							iState = EState::LessThan;
							break;

						case '>':
							iState = EState::GreaterThan;
							break;

						case ',':
							return SetToken(EGridLangToken::Comma, CString(m_pPos++, 1));

						case '.':
							return SetToken(EGridLangToken::Dot, CString(m_pPos++, 1));

						case '?':
							return SetToken(EGridLangToken::Question, CString(m_pPos++, 1));

						case '/':
							return SetToken(EGridLangToken::Slash, CString(m_pPos++, 1));

						case '~':
							return SetToken(EGridLangToken::Tilde, CString(m_pPos++, 1));

						case '`':
							return SetToken(EGridLangToken::BackQuote, CString(m_pPos++, 1));

						default:
							pStart = m_pPos;
							iState = EState::OtherString;
							break;
						}
					}

				break;
				}

			case EState::Comment:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Null);
					}
				else if (*m_pPos == '\n')
					{
					m_iLine++;
					iState = EState::Start;
					}

				break;
				}

			case EState::Identifier:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Identifier, CString(pStart, m_pPos - pStart));
					}
				else if (::strIsASCIIAlphaNumeric(m_pPos) || *m_pPos == '_')
					{
					}
				else
					{
					return SetToken(EGridLangToken::Identifier, CString(pStart, m_pPos - pStart));
					}

				break;
				}

			case EState::QuotedString:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Error, strPattern(ERR_NO_CLOSE_QUOTE, iStartLine));
					}
				else if (*m_pPos == '\"')
					{
					SetToken(EGridLangToken::String, CString(pStart, m_pPos - pStart));
					m_pPos++;
					return EGridLangToken::String;
					}

				break;
				}

			case EState::OtherString:
				{
				if (m_pPos == m_pPosEnd || strIsWhitespace(m_pPos))
					{
					return SetToken(EGridLangToken::String, CString(pStart, m_pPos - pStart));
					}

				break;
				}

			case EState::Number0:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Integer, CString(pStart, m_pPos - pStart));
					}
				else if (*m_pPos == 'x' || *m_pPos == 'X')
					{
					iState = EState::NumberHex;
					}
				else if (*m_pPos == '.')
					{
					iState = EState::NumberFloat;
					}
				else if (*m_pPos == 'e' || *m_pPos == 'E')
					{
					iState = EState::NumberExponent;
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					iState = EState::Number;
					}
				else
					{
					return SetToken(EGridLangToken::Integer, CString(pStart, m_pPos - pStart));
					}
				break;
				}

			case EState::Number:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Integer, CString(pStart, m_pPos - pStart));
					}
				else if (*m_pPos == '.')
					{
					iState = EState::NumberFloat;
					}
				else if (*m_pPos == 'e' || *m_pPos == 'E')
					{
					iState = EState::NumberExponent;
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					}
				else
					{
					return SetToken(EGridLangToken::Integer, CString(pStart, m_pPos - pStart));
					}
				break;
				}

			case EState::NumberHex:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::HexInteger, CString(pStart, m_pPos - pStart));
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					}
				else if (*m_pPos >= 'a' && *m_pPos <= 'f')
					{
					}
				else if (*m_pPos >= 'A' && *m_pPos <= 'F')
					{
					}
				else
					{
					return SetToken(EGridLangToken::HexInteger, CString(pStart, m_pPos - pStart));
					}
				break;
				}

			case EState::NumberFloat:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Real, CString(pStart, m_pPos - pStart));
					}
				else if (*m_pPos == 'e' || *m_pPos == 'E')
					{
					iState = EState::NumberExponent;
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					}
				else
					{
					return SetToken(EGridLangToken::Real, CString(pStart, m_pPos - pStart));
					}
				break;
				}

			case EState::NumberExponent:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Real, CString(pStart, m_pPos - pStart));
					}
				else if (*m_pPos == '+' || *m_pPos == '-')
					{
					iState = EState::NumberExponentValue;
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					iState = EState::NumberExponentValue;
					}
				else
					{
					return SetToken(EGridLangToken::Real, CString(pStart, m_pPos - pStart));
					}
				break;
				}

			case EState::NumberExponentValue:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Real, CString(pStart, m_pPos - pStart));
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					}
				else
					{
					return SetToken(EGridLangToken::Real, CString(pStart, m_pPos - pStart));
					}
				break;
				}

			case EState::Amp:
				{
				if (*m_pPos == '&')
					return SetToken(EGridLangToken::And, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Amp, CString(m_pPos - 1, 1));
				}

			case EState::Bang:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::NotEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Bang, CString(m_pPos - 1, 1));
				}

			case EState::Bar:
				{
				if (*m_pPos == '|')
					return SetToken(EGridLangToken::Or, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Bar, CString(m_pPos - 1, 1));
				}

			case EState::Equals:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::EqualEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Equals, CString(m_pPos - 1, 1));
				}

			case EState::GreaterThan:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::GreaterThanEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::GreaterThan, CString(m_pPos - 1, 1));
				}

			case EState::LessThan:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::LessThanEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::LessThan, CString(m_pPos - 1, 1));
				}

			default:
				throw CException(errFail);
			}

		m_pPos++;
		}

	return m_iToken;
	}

EGridLangToken CGridLangParser::SetToken (EGridLangToken iToken, const CString &sToken)

//	SetToken
//
//	Sets the current token.

	{
	m_iToken = iToken;
	m_sTokenValue = sToken;
	return iToken;
	}
