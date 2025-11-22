//	CGridLangParser.cpp
//
//	CGridLangParser Class
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_UNEXPECTED_CHARACTER,			"Unexpected character '%s'");
DECLARE_CONST_STRING(ERR_NO_CLOSE_QUOTE,				"Quoted string started on line %d never ended.");
DECLARE_CONST_STRING(ERR_UNEXPECTED_EOS,				"Unexpected end of stream.");

int CGridLangParser::CalcCurLineIndent () const

//	CalcIndent
//
//	Computes the indentation of the current line.

	{
	if (!m_pLineStart)
		return 0;

	int iIndent = 0;

	const char *pIndent = m_pLineStart;
	while (pIndent < m_pPos)
		{
		if (*pIndent == '\t')
			iIndent += DEFAULT_TAB_SIZE;
		else if (*pIndent == ' ')
			iIndent++;
		else if (*pIndent == '\r')
			{ }
		else
			break;

		pIndent++;
		}

	return iIndent;
	}

CString CGridLangParser::ComposeError (const CString &sError) const

//	ComposeError
//
//	Compose an error.

	{
	return strPattern("%s(%d,%d): %s", m_sSourceFile, m_iLine, m_iChar, sError);
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

CString CGridLangParser::ParseBasicString (const CString& sValue)

//	ParseBasicString
//
//	Parses a string in which a sequence of two double-quotes is treated as an
//	embedded double-quote character.

	{
	CString sResult(sValue.GetLength());
	char *pStartDest = sResult.GetPointer();
	const char *pSrc = sValue.GetParsePointer();
	const char *pSrcEnd = pSrc + sValue.GetLength();

	char *pDest = pStartDest;
	while (pSrc < pSrcEnd)
		{
		if (*pSrc == '"')
			{
			*pDest++ = *pSrc++;

			//	Skip second quote.

			pSrc++;
			}
		else
			{
			*pDest++ = *pSrc++;
			}
		}

	sResult.SetLength((int)(pDest - pStartDest));
	return sResult;
	}

CString CGridLangParser::ParseBlockString (const CString& sValue, int deIndentColumns)

//	ParseBlockString
//
//	Parse a block of text, which is a string that can span multiple lines.

	{
	CStringBuffer Output;
	const char* pSrc = sValue.GetParsePointer();
	const char* pSrcEnd = pSrc + sValue.GetLength();

	int indentCount = deIndentColumns;
	int quoteCount = 0;

	for (; pSrc < pSrcEnd; pSrc++) 
		{
		//	Handle line breaks

		if (*pSrc == '\n' || *pSrc == '\r') 
			{
			if (*pSrc == '\r' && pSrc[1] == '\n') 
				{
				pSrc++; // Skip the next character if it's \n
				}

			//	If we're at the end, then we skip the last line break.
			//	[We're at the end if all we have after this line break is just
			//	tabs or spaces.]

			const char* pLineEnd = pSrc + 1;
			while (pLineEnd < pSrcEnd && (*pLineEnd == ' ' || *pLineEnd == '\t'))
				pLineEnd++;

			if (pLineEnd == pSrcEnd)
				break;

			//	Otherwise, we write out a line break.

			Output.WriteChar('\n');
			indentCount = deIndentColumns;
			quoteCount = 0;
			}

		//	Count quotes

		else if (*pSrc == '"')
			{
			indentCount = 0;
			quoteCount++;
			if (pSrc[1] != '"') 
				{
				if (quoteCount >= 4) 
					{
					Output.WriteChar('"', quoteCount - 1);
					}
				else
					{
					Output.WriteChar('"', quoteCount);
					}
				quoteCount = 0;
				}
			}
		else if (*pSrc == '\t' && indentCount > 0)
			{
			indentCount = Max(0, indentCount - DEFAULT_TAB_SIZE);
			}
		else if (*pSrc == ' ' && indentCount > 0)
			{
			indentCount--;
			}

		//	Write the character

		else
			{
			indentCount = 0;
			Output.WriteChar(*pSrc);
			}
		}

	return CString(std::move(Output));
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

TArray<CGridLangParser::SToken> CGridLangParser::PeekTokens (int iCount) const

//	PeekToken
//
//	Peeks ahead.

	{
	TArray<SToken> Tokens;
	Tokens.GrowToFit(iCount);

	CGridLangParser Peeker;
	Peeker.m_pPos = m_pPos;
	Peeker.m_pPosEnd = m_pPosEnd;

	for (int i = 0; i < iCount; i++)
		{
		auto iToken = Peeker.NextToken();
		Tokens.Insert({ iToken, Peeker.GetTokenValue() });
		}

	return Tokens;
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
		BlockComment,
		BlockCommentHash,
		BlockCommentSlash,
		Identifier,
		QuotedStringStart,
		QuotedString,
		QuotedStringEscape,
		QuotedStringWithEscapes,
		BlockString,
		OtherString,

		Number0,
		Number,
		NumberHex,
		NumberFloat,
		NumberExponent,
		NumberExponentValue,

		Equals,
		EqualEquals,
		Amp,
		Bar,
		BarInBlockComment,
		Bang,
		BangEquals,
		Caret,
		Dot,
		GreaterThan,
		LessThan,
		Minus,
		Percent,
		Plus,
		Star,
		StarInBlockComment,
		Slash,
		Hash,
		HashInBlockComment,
		Line,

		Done,
		};

	EState iState = EState::Start;
	const char *pStart = NULL;
	int iStartLine = 0;
	int iBlockIndent = 0;

	while (iState != EState::Done)
		{
		if (*m_pPos == '\n' && m_pLineStart != m_pPos + 1)
			{
			m_iLine++;
			m_pLineStart = m_pPos + 1;
			}

		switch (iState)
			{
			case EState::Start:
				{
				if (m_pPos == m_pPosEnd)
					{
					m_iChar = GetCharPos();
					return SetToken(EGridLangToken::Null);
					}

				//	If whitespace, continue.

				else if (*m_pPos == ' ' || *m_pPos == '\t' || *m_pPos == '\r' || *m_pPos == '\n')
					{ }

				//	Quoted string

				else if (*m_pPos == '"')
					{
					m_iChar = GetCharPos();
					pStart = m_pPos + 1;
					iStartLine = m_iLine;
					iState = EState::QuotedStringStart;
					}

				//	Identifier (can't start with a number)

				else if (::strIsASCIIAlpha(m_pPos) || *m_pPos == '_' || *m_pPos == '$')
					{
					m_iChar = GetCharPos();
					pStart = m_pPos;
					iState = EState::Identifier;
					}

				//	Number

				else if (*m_pPos >= '0' && *m_pPos <= '9')
					{
					m_iChar = GetCharPos();
					pStart = m_pPos;
					if (*m_pPos == '0')
						iState = EState::Number0;
					else
						iState = EState::Number;
					}

				//	Else it's a symbol of some sort

				else
					{
					m_iChar = GetCharPos();

					switch (*m_pPos)
						{
						case '!':
							iState = EState::Bang;
							break;

						case '@':
							return SetToken(EGridLangToken::At, CString(m_pPos++, 1));

						case '#':
							iState = EState::Hash;
							break;

						case '%':
							iState = EState::Percent;
							break;

						case '^':
							iState = EState::Caret;
							break;

						case '&':
							iState = EState::Amp;
							break;

						case '*':
							iState = EState::Star;
							break;

						case '(':
							return SetToken(EGridLangToken::OpenParen, CString(m_pPos++, 1));

						case ')':
							return SetToken(EGridLangToken::CloseParen, CString(m_pPos++, 1));

						case '-':
							iState = EState::Minus;
							break;

						case '+':
							iState = EState::Plus;
							break;

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
							iState = EState::Dot;
							break;

						case '?':
							return SetToken(EGridLangToken::Question, CString(m_pPos++, 1));

						case '/':
							iState = EState::Slash;
							break;

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

			case EState::Hash:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Null);
					}
				else if (*m_pPos == '\n')
					{
					iState = EState::Start;
					}
				else
					iState = EState::Comment;

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
					iState = EState::Start;
					}

				break;
				}

			case EState::BlockComment:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Null);
					}
				else if (*m_pPos == '|')
					iState = EState::BarInBlockComment;

				break;
				}

			case EState::BarInBlockComment:
				{
				if (m_pPos == m_pPosEnd)
					return SetToken(EGridLangToken::Null);

				else if (*m_pPos == '\n')
					iState = EState::BlockComment;

				else if (*m_pPos == '>')
					iState = EState::Start;

				else
					iState = EState::BlockComment;

				break;
				}

			case EState::BlockCommentHash:
				{
				if (m_pPos == m_pPosEnd)
					return SetToken(EGridLangToken::Null);

				else if (*m_pPos == '#')
					iState = EState::HashInBlockComment;

				break;
				}

			case EState::HashInBlockComment:
				{
				if (m_pPos == m_pPosEnd)
					return SetToken(EGridLangToken::Null);

				else if (*m_pPos == '\n')
					iState = EState::BlockCommentHash;

				else if (*m_pPos == '>')
					iState = EState::Start;

				else
					iState = EState::BlockCommentHash;

				break;
				}

			case EState::BlockCommentSlash:
				{
				if (m_pPos == m_pPosEnd)
					return SetToken(EGridLangToken::Null);

				else if (*m_pPos == '*')
					iState = EState::StarInBlockComment;

				break;
				}

			case EState::StarInBlockComment:
				{
				if (m_pPos == m_pPosEnd)
					return SetToken(EGridLangToken::Null);

				else if (*m_pPos == '\n')
					iState = EState::BlockCommentSlash;

				else if (*m_pPos == '/')
					iState = EState::Start;

				else
					iState = EState::BlockCommentSlash;

				break;
				}

			case EState::Identifier:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Identifier, CString(pStart, m_pPos - pStart));
					}

				//	Can have embedded numbers.

				else if (::strIsASCIIAlphaNumeric(m_pPos) || *m_pPos == '_' || *m_pPos == '$')
					{
					}
				else
					{
					return SetToken(EGridLangToken::Identifier, CString(pStart, m_pPos - pStart));
					}

				break;
				}

			case EState::QuotedStringStart:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Error, strPattern(ERR_NO_CLOSE_QUOTE, iStartLine));
					}
				else if (*m_pPos == '"')
					{
					//	Another double-quote is either an escape or the first 
					//	two characters in a triple-quote block.

					if (m_pPos[1] == '"')
						{
						if (m_pPos[2] == '\r' || m_pPos[2] == '\n')
							{
							//	Remember the indentation of this line.

							iBlockIndent = CalcCurLineIndent();

							//	Place the current position after the CRLF

							m_pPos++;	//	Skip second quote
							m_pPos++;	//	Skip third quote
							if (m_pPos[1] == '\r' || m_pPos[1] == '\n')
								m_pPos++;	//	Skip CRLF
							m_iLine++;
							m_pLineStart = m_pPos + 1;

							pStart = m_pPos + 1;
							iStartLine = m_iLine;
							iState = EState::BlockString;
							}
						else
							iState = EState::QuotedStringEscape;
						}

					//	Otherwise, we're done with the string.

					else
						{
						SetToken(EGridLangToken::String, CString(pStart, m_pPos - pStart));
						m_pPos++;
						return EGridLangToken::String;
						}
					}
				else
					{
					iState = EState::QuotedString;
					}

				break;
				}

			case EState::QuotedString:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Error, strPattern(ERR_NO_CLOSE_QUOTE, iStartLine));
					}
				else if (*m_pPos == '"')
					{
					//	Another double-quote is an escape.

					if (m_pPos[1] == '"')
						{
						iState = EState::QuotedStringEscape;
						}

					//	Otherwise, we're done with the string.

					else
						{
						SetToken(EGridLangToken::String, CString(pStart, m_pPos - pStart));
						m_pPos++;
						return EGridLangToken::String;
						}
					}

				break;
				}

			case EState::QuotedStringEscape:
				{
				iState = EState::QuotedStringWithEscapes;
				break;
				}

			case EState::QuotedStringWithEscapes:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Error, strPattern(ERR_NO_CLOSE_QUOTE, iStartLine));
					}
				else if (*m_pPos == '"')
					{
					//	Another double-quote is an escape.

					if (m_pPos[1] == '"')
						{
						iState = EState::QuotedStringEscape;
						}

					//	Otherwise, we're done with the string.

					else
						{
						SetToken(EGridLangToken::String, ParseBasicString(CString(pStart, m_pPos - pStart)));
						m_pPos++;
						return EGridLangToken::String;
						}
					}

				break;
				}

			case EState::BlockString:
				{
				if (m_pPos == m_pPosEnd)
					{
					return SetToken(EGridLangToken::Error, strPattern(ERR_NO_CLOSE_QUOTE, iStartLine));
					}
				else if (*m_pPos == '"')
					{
					const char *pPos = m_pPos + 1;
					while (*pPos == '"')
						pPos++;

					int iQuoteCount = (int)(pPos - m_pPos);
					if (iQuoteCount == 3)
						{
						SetToken(EGridLangToken::String, ParseBlockString(CString(pStart, m_pPos - pStart), iBlockIndent));
						m_pPos += 3;
						return EGridLangToken::String;
						}
					else
						{
						//	Advance past the run of quotes, but include them
						//	in the block string.

						m_pPos += iQuoteCount - 1;
						}
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
				else if (*m_pPos == '.' && m_pPos[1] != '.')
					{
					iState = EState::NumberFloat;
					}
				else if (*m_pPos == 'e' || *m_pPos == 'E')
					{
					iState = EState::NumberExponent;
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9' || *m_pPos == '_')
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
				else if (*m_pPos == '.' && m_pPos[1] != '.')
					{
					iState = EState::NumberFloat;
					}
				else if (*m_pPos == 'e' || *m_pPos == 'E')
					{
					iState = EState::NumberExponent;
					}
				else if (*m_pPos >= '0' && *m_pPos <= '9' || *m_pPos == '_')
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
				else if (*m_pPos == '_')
					{
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
				else if (*m_pPos >= '0' && *m_pPos <= '9' || *m_pPos == '_')
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
				else if (*m_pPos == '=')
					return SetToken(EGridLangToken::AmpEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Amp, CString(m_pPos - 1, 1));
				}

			case EState::Bang:
				{
				if (*m_pPos == '=')
					{
					iState = EState::BangEquals;
					break;
					}
				else if (*m_pPos == '>')
					return SetToken(EGridLangToken::CloseBangBlock, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Bang, CString(m_pPos - 1, 1));
				}

			case EState::BangEquals:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::NotEqualEquals, CString(m_pPos++ - 2, 3));
				else
					return SetToken(EGridLangToken::NotEquals, CString(m_pPos - 2, 2));
				}

			case EState::Bar:
				{
				if (m_pPos == m_pPosEnd)
					return SetToken(EGridLangToken::Null);

				else if (*m_pPos == '\n')
					iState = EState::Start;

				else if (*m_pPos == '|')
					return SetToken(EGridLangToken::Or, CString(m_pPos++ - 1, 2));

				else
					iState = EState::Comment;

				break;
				}

			case EState::Caret:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::CaretEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Caret, CString(m_pPos - 1, 1));
				}

			case EState::Dot:
				{
				//	If another dot, then this is at least 2 dots.

				if (*m_pPos == '.')
					{
					//	3 dots?

					if (m_pPos < m_pPosEnd && m_pPos[1] == '.')
						{
						auto iToken = SetToken(EGridLangToken::TripleDot, CString(m_pPos - 1, 3));
						m_pPos += 2;
						return iToken;
						}
					else
						return SetToken(EGridLangToken::DoubleDot, CString(m_pPos++ - 1, 2));
					}
				else
					return SetToken(EGridLangToken::Dot, CString(m_pPos - 1, 1));
				}

			case EState::Equals:
				{
				if (*m_pPos == '=')
					{
					iState = EState::EqualEquals;
					break;
					}
				else
					return SetToken(EGridLangToken::Equals, CString(m_pPos - 1, 1));
				}

			case EState::EqualEquals:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::EqualEqualEquals, CString(m_pPos++ - 2, 3));
				else
					return SetToken(EGridLangToken::EqualEquals, CString(m_pPos - 2, 2));
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
				else if (*m_pPos == '|')
					{
					iState = EState::BlockComment;
					}
				else if (*m_pPos == '#')
					{
					iState = EState::BlockCommentHash;
					}
				else if (*m_pPos == '!')
					return SetToken(EGridLangToken::OpenBangBlock, CString(m_pPos++ - 1, 2));
				else if (*m_pPos == '-')
					{
					if (m_pPos < m_pPosEnd && m_pPos[1] == '-')
						{
						auto iToken = SetToken(EGridLangToken::LeftLongArrow, CString(m_pPos - 1, 3));
						m_pPos += 2;
						return iToken;
						}
					else
						return SetToken(EGridLangToken::LessThan, CString(m_pPos - 1, 1));
					}
				else
					return SetToken(EGridLangToken::LessThan, CString(m_pPos - 1, 1));
				break;
				}

			case EState::Minus:
				{
				if (*m_pPos == '>')
					return SetToken(EGridLangToken::Arrow, CString(m_pPos++ - 1, 2));

				else if (*m_pPos == '=')
					return SetToken(EGridLangToken::MinusEquals, CString(m_pPos++ - 1, 2));

				else if (*m_pPos == '-')
					{
					if (m_pPos < m_pPosEnd && m_pPos[1] == '>')
						{
						auto iToken = SetToken(EGridLangToken::RightLongArrow, CString(m_pPos - 1, 3));
						m_pPos += 2;
						return iToken;
						}
					else if (m_pPos < m_pPosEnd && m_pPos[1] == '-')
						iState = EState::Line;
					else
						return SetToken(EGridLangToken::Minus, CString(m_pPos - 1, 1));
					}
				else
					return SetToken(EGridLangToken::Minus, CString(m_pPos - 1, 1));

				break;
				}

			case EState::Percent:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::PercentEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Percent, CString(m_pPos - 1, 1));
				}

			case EState::Plus:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::PlusEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Plus, CString(m_pPos - 1, 1));
				}

			case EState::Star:
				{
				if (*m_pPos == '=')
					return SetToken(EGridLangToken::StarEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Star, CString(m_pPos - 1, 1));
				}

			case EState::Slash:
				{
				if (*m_pPos == '/')
					iState = EState::Comment;
				else if (*m_pPos == '*')
					iState = EState::BlockCommentSlash;
				else if (*m_pPos == '=')
					return SetToken(EGridLangToken::SlashEquals, CString(m_pPos++ - 1, 2));
				else
					return SetToken(EGridLangToken::Slash, CString(m_pPos - 1, 1));

				break;
				}

			case EState::Line:
				{
				if (m_pPos == m_pPosEnd)
					return SetToken(EGridLangToken::Null);
				else if (*m_pPos != '-')
					{
					iState = EState::Start;
					}
				break;
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
