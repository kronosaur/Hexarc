//	CLispParser.cpp
//
//	CLispParser class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_INVALID_CHARACTER,			"Invalid character: '%s'.")
DECLARE_CONST_STRING(ERR_INVALID_COMMENT,			"Error parsing comment.")
DECLARE_CONST_STRING(ERR_INVALID_STRING,			"Error parsing quoted string.")

bool CLispParser::ParseCommentCPP (void)

//	ParseCommentCPP
//
//	m_chChar == '/'
//	Process a C++ style comment

	{
	//	Skip '/'

	m_pStream->ReadChar();

	//	If another slash, then skip until end of line

	//	Read until the end of the line

	m_pStream->ParseToEndOfLine();

	//	Done

	return true;
	}

bool CLispParser::ParseCommentSemi (void)

//	ParseCommentSemi
//
//	m_chChar == ';'
//	Process until the end of the line.

	{
	//	Skip ';'

	m_pStream->ReadChar();

	//	Read until the end of the line

	m_pStream->ParseToEndOfLine();

	//	Done

	return true;
	}

CLispParser::ETokens CLispParser::ParseIdentifier (CDatum *retdDatum)

//	ParseIdentifier
//
//	Parses an identifier

	{
	enum EStates
		{
		stateStart,

		stateHexNumber,
		stateIdentifier,
		stateNumber,
		stateNumberStartingWith0,
		stateRealDecimal,
		stateRealExponent,
		stateRealExponentNumber,
		stateRealExponentSign,
		stateSignedNumber,
		};

	EStates iState = stateStart;
	CStringBuffer Identifier;
	bool bHasLeadingSign = false;

	//	Keep parsing until we reach the end of the identifier

	char chChar = m_pStream->GetChar();
	while (chChar != ' '
			&& chChar != '('
			&& chChar != ')'
			&& chChar != '{'
			&& chChar != '}'
			&& chChar != '['
			&& chChar != ']'
			&& chChar != '\''
			&& chChar != '"'
			&& chChar != '`'
			&& chChar != ','
			&& chChar != ';'
			&& chChar != ':'
			&& !strIsWhitespace(&chChar)
			&& !strIsASCIIControl(&chChar))
		{
		//	If this is a slash then see if it is the start of a comment. If so,
		//	then we're done with the identifier (note: we can't get here if
		//	the first character is a slash, so we are guaranteed that the
		//	identifier is non-null).

		if (chChar == '/')
			{
			chChar = m_pStream->ReadChar();
			
			//	A second slash means a comment until the end of the line
			//	A * means a C style comment block

			if (chChar == '/' || chChar == '*')
				{
				//	Backup as if we had stopped at the first slash and
				//	exit the loop.

				m_pStream->UnreadChar();
				m_pStream->UnreadChar();
				m_pStream->ReadChar();
				break;
				}

			//	Otherwise the slash was part of the identifier

			else
				{
				iState = stateIdentifier;
				Identifier.Write("/", 1);
				continue;
				}
			}

		//	Keep parsing characters, keeping track of what we've got.

		switch (iState)
			{
			case stateStart:
				if (chChar == '0')
					iState = stateNumberStartingWith0;
				else if (chChar >= '1' && chChar <= '9')
					iState = stateNumber;
				else if (chChar == '.')
					iState = stateRealDecimal;
				else if (chChar == '-' || chChar == '+')
					{
					bHasLeadingSign = true;
					iState = stateSignedNumber;
					}
				else
					iState = stateIdentifier;
				break;

			case stateNumber:
				if (chChar == '.')
					iState = stateRealDecimal;
				else if (chChar == 'e' || chChar == 'E')
					iState = stateRealExponent;
				else if (chChar >= '0' && chChar <= '9')
					;
				else
					iState = stateIdentifier;
				break;

			case stateNumberStartingWith0:
				if (chChar == 'x' || chChar == 'X')
					iState = stateHexNumber;
				else if (chChar >= '0' && chChar <= '9')
					iState = stateNumber;
				else if (chChar == '.')
					iState = stateRealDecimal;
				else if (chChar = 'e' || chChar == 'E')
					iState = stateRealExponent;
				else
					iState = stateIdentifier;
				break;

			case stateHexNumber:
				if (chChar >= '0' && chChar <= '9')
					;
				else if (chChar >= 'a' && chChar <= 'f')
					;
				else if (chChar >= 'A' && chChar <= 'F')
					;
				else
					iState = stateIdentifier;
				break;

			case stateRealDecimal:
				if (chChar >= '0' && chChar <= '9')
					;
				else if (chChar == 'e' || chChar == 'E')
					iState = stateRealExponent;
				else
					iState = stateIdentifier;
				break;

			case stateRealExponent:
				if (chChar == '+' || chChar == '-')
					iState = stateRealExponentSign;
				else if (chChar >= '0' && chChar <= '9')
					iState = stateRealExponentNumber;
				else
					iState = stateIdentifier;
				break;

			case stateRealExponentNumber:
				if (chChar >= '0' && chChar <= '9')
					;
				else
					iState = stateIdentifier;
				break;

			case stateRealExponentSign:
				if (chChar >= '0' && chChar <= '9')
					iState = stateRealExponentNumber;
				else
					iState = stateIdentifier;
				break;

			case stateSignedNumber:
				if (chChar == '.')
					iState = stateRealDecimal;
				else if (chChar >= '0' && chChar <= '9')
					iState = stateNumber;
				else
					iState = stateIdentifier;
				break;

			case stateIdentifier:
				break;

			default:
				ASSERT(false);
			}

		//	Keep writing it

		Identifier.Write(&chChar, 1);
		chChar = m_pStream->ReadChar();
		}

	//	Figure out what kind of literal we have

	switch (iState)
		{
		//	If we're in any of these partial states then it means that we did
		//	not parse a full number, so we just have a plain identifier.

		case stateIdentifier:
		case stateRealExponent:
		case stateRealExponentSign:
		case stateSignedNumber:
			CDatum::CreateStringFromHandoff(Identifier, retdDatum);
			return tkIdentifier;

		//	This is the integer 0

		case stateNumberStartingWith0:
			*retdDatum = CDatum(0);
			return tkIntegerDatum;

		//	This is a plain integer, but we have to figure out if it fits in
		//	32 bits or not.

		case stateNumber:
			{
			int iDigits = Identifier.GetLength() - (bHasLeadingSign ? 1 : 0);

			if (iDigits > 10
					|| (iDigits == 10 && strOverflowsInteger32(Identifier)))
				{
				CIPInteger Value;
				Value.InitFromString(Identifier);
				CDatum::CreateIPIntegerFromHandoff(Value, retdDatum);
				return tkIPIntegerDatum;
				}
			else
				{
				int iValue = strParseInt(Identifier.GetPointer(), 0, NULL, NULL);
				*retdDatum = CDatum(iValue);
				return tkIntegerDatum;
				}
			}

		case stateHexNumber:
			{
			int iDigits = Identifier.GetLength() - 2;

			if (iDigits > 8)
				{
				CIPInteger Value;
				Value.InitFromString(Identifier);
				CDatum::CreateIPIntegerFromHandoff(Value, retdDatum);
				return tkIPIntegerDatum;
				}
			else
				{
				int iValue = strParseInt(Identifier.GetPointer(), 0, NULL, NULL);
				*retdDatum = CDatum(iValue);
				return tkIntegerDatum;
				}
			}

		case stateRealDecimal:
		case stateRealExponentNumber:
			{
			*retdDatum = CDatum(strToDouble(Identifier));
			return tkFloatDatum;
			}

		default:
			ASSERT(false);
			return tkError;
		}

/*
	CStringBuffer Identifier;
	bool bIsDecimalInteger = true;
	bool bIsHexInteger = true;
	bool bIsRealNumber = true;
	bool bHasLeadingNegative = false;
	bool bHasLeadingSign = false;
	bool bHasDecimalPoint = false;
	bool bHasExponent = false;

	//	Keep parsing until we reach the end of the identifier

	char chChar = m_pStream->GetChar();
	while (chChar != ' '
			&& chChar != '('
			&& chChar != ')'
			&& chChar != '{'
			&& chChar != '}'
			&& chChar != '['
			&& chChar != ']'
			&& chChar != '\''
			&& chChar != '"'
			&& chChar != '`'
			&& chChar != ','
			&& chChar != ';'
			&& chChar != ':'
			&& !strIsWhitespace(&chChar)
			&& !strIsASCIIControl(&chChar))
		{
		//	If this is a slash then see if it is the start of a comment. If so,
		//	then we're done with the identifier (note: we can't get here if
		//	the first character is a slash, so we are guaranteed that the
		//	identifier is non-null).

		if (chChar == '/')
			{
			chChar = m_pStream->ReadChar();
			
			//	A second slash means a comment until the end of the line
			//	A * means a C style comment block

			if (chChar == '/' || chChar == '*')
				{
				//	Backup as if we had stopped at the first slash

				m_pStream->UnreadChar();
				m_pStream->UnreadChar();
				m_pStream->ReadChar();
				break;
				}

			//	Otherwise the slash was part of the identifier

			else
				{
				Identifier.Write("/", 1);
				bIsDecimalInteger = false;
				bIsHexInteger = false;
				bIsRealNumber = false;
				continue;
				}
			}

		//	If this is the first character the we check some things

		if (Identifier.GetLength() == 0)
			{
			if (chChar == '-')
				{
				bHasLeadingNegative = true;
				bHasLeadingSign = true;
				}
			else if (chChar == '+')
				bHasLeadingSign = true;

			if (chChar != '0')
				bIsHexInteger = false;
			}

		//	If this is the second character and it is not an x, then it can't
		//	be a hex integer

		else if (bIsHexInteger && Identifier.GetLength() == 1)
			{
			if (chChar != 'x' && chChar != 'X')
				bIsHexInteger = false;
			}

		//	If we have any character that is not a number, then it can't be a
		//	decimal integer

		if (bIsDecimalInteger && (chChar < '0' || chChar > '9') 
				&& (chChar != '-' || Identifier.GetLength() > 0) 
				&& (chChar != '+' || Identifier.GetLength() > 0))
			bIsDecimalInteger = false;

		//	If we have any character outside the hex range, then it can't be a
		//	hex integer

		if (bIsHexInteger && (chChar < '0' || chChar > '9') && (chChar < 'A' || chChar > 'F') && (chChar < 'a' || chChar > 'f')
				&& (chChar != 'x' || Identifier.GetLength() != 1)
				&& (chChar != 'X' || Identifier.GetLength() != 1))
			bIsHexInteger = false;

		//	If we have any character outside IEEE number, then it can't be a
		//	real number

		if (bIsRealNumber && (chChar < '0' || chChar > '9')
				&& (chChar != '.')
				&& (chChar != 'e')
				&& (chChar != 'E')
				&& (chChar != '+')
				&& (chChar != '-'))
			bIsRealNumber = false;

		//	Real numbers cannot have more than one decimal point

		if (bIsRealNumber && chChar == '.')
			{
			if (bHasDecimalPoint)
				bIsRealNumber = false;
			else
				bHasDecimalPoint = true;
			}

		//	Keep writing it

		Identifier.Write(&chChar, 1);
		chChar = m_pStream->ReadChar();
		}

	//	If an integer, calculate how many digits

	int iDigits;
	if (bIsHexInteger && Identifier.GetLength() > 2)
		iDigits = Identifier.GetLength() - 2;
	else if (bIsDecimalInteger)
		iDigits = Identifier.GetLength() - (bHasLeadingSign ? 1 : 0);
	else
		iDigits = 0;

	//	If we have a hex number greater than 8 then we have to
	//	use a CIPInteger. If we have a decimal number greater than 10 digits
	//	then we also have to use a CIPInteger.

	if ((bIsHexInteger && iDigits > 8)
			|| (bIsDecimalInteger && iDigits > 10)
			|| (bIsDecimalInteger && iDigits == 10 && strOverflowsInteger32(Identifier)))
		{
		CIPInteger Value;
		Value.InitFromString(Identifier);
		CDatum::CreateIPIntegerFromHandoff(Value, retdDatum);
		return tkIPIntegerDatum;
		}

	//	Otherwise, if we have an decimal or hex integer, convert it
	//	to a 32-bit value.

	else if ((bIsDecimalInteger || bIsHexInteger) && iDigits > 0)
		{
		int iValue = strParseInt(Identifier.GetPointer(), 0, NULL, NULL);
		*retdDatum = CDatum(iValue);
		return tkIntegerDatum;
		}

	//	Otherwise, this is regular identifier

	else
		{
		CDatum::CreateStringFromHandoff(Identifier, retdDatum);
		return tkIdentifier;
		}

*/
	}

CLispParser::ETokens CLispParser::ParseStringLiteral (CDatum *retdDatum)

//	ParseStringLiteral
//
//	m_chChar == '"'
//	Parse until close quote.

	{
	CString sString;

	if (!m_pStream->ParseQuotedString(0, &sString))
		{
		*retdDatum = CDatum(ComposeError(ERR_INVALID_STRING));
		return tkError;
		}

	CDatum::CreateStringFromHandoff(sString, retdDatum);

	//	Done

	return tkStringDatum;
	}

CLispParser::ETokens CLispParser::ParseToken (CDatum *retdDatum)

//	ParseToken
//
//	Parse the token. We end with parse position at the first character
//	after the token.

	{
	bool bDone = false;

	while (!bDone)
		{
		//	Skip Parse whitespace

		m_pStream->ParseWhitespace();

		//	Parse token

		switch (m_pStream->GetChar())
			{
			case '\0':
				//	Unexpected end of file
				m_iToken = tkEOF;
				m_dToken = CDatum();
				bDone = true;
				break;

			case '(':
				m_iToken = tkOpenParen;
				m_dToken = CDatum();
				m_pStream->ReadChar();
				bDone = true;
				break;

			case ')':
				m_iToken = tkCloseParen;
				m_dToken = CDatum();
				m_pStream->ReadChar();
				bDone = true;
				break;

			case '{':
				m_iToken = tkOpenBrace;
				m_dToken = CDatum();
				m_pStream->ReadChar();
				bDone = true;
				break;

			case '}':
				m_iToken = tkCloseBrace;
				m_dToken = CDatum();
				m_pStream->ReadChar();
				bDone = true;
				break;

			case '\'':
				m_iToken = tkQuote;
				m_dToken = CDatum();
				m_pStream->ReadChar();
				bDone = true;
				break;

			case ';':
				if (!ParseCommentSemi())
					{
					m_iToken = tkError;
					m_dToken = CDatum(ComposeError(ERR_INVALID_COMMENT));
					bDone = true;
					break;
					}

				//	Loop and get another token
				break;

			case ':':
				m_iToken = tkColon;
				m_dToken = CDatum();
				m_pStream->ReadChar();
				bDone = true;
				break;

			case '/':
				{
				//	Read the next character

				char chChar = m_pStream->ReadChar();
				if (chChar == '/')
					{
					m_pStream->ParseToEndOfLine();

					//	Loop and get some more
					break;
					}
				else if (chChar == '*')
					{
					while (true)
						{
						chChar = m_pStream->ReadChar();
						if (chChar == '*')
							{
							chChar = m_pStream->ReadChar();
							if (chChar == '/')
								{
								m_pStream->ReadChar();

								//	Loop and get some more;
								break;
								}
							else if (chChar == '\0')
								{
								m_iToken = tkEOF;
								m_dToken = CDatum();
								bDone = true;
								break;
								}
							}
						else if (chChar == '\0')
							{
							m_iToken = tkEOF;
							m_dToken = CDatum();
							bDone = true;
							break;
							}
						}
					break;
					}

				//	Otherwise, put the characters back and read an identifier

				else
					{
					m_pStream->UnreadChar();
					m_pStream->UnreadChar();
					m_pStream->ReadChar();
					m_iToken = ParseIdentifier(&m_dToken);
					bDone = true;
					break;
					}

				break;
				}

			case '"':
				m_iToken = ParseStringLiteral(&m_dToken);
				bDone = true;
				break;

			//	These characters are reserved for future use
			case ',':
			case '`':
			case '[':
			case ']':
				{
				char chChar = m_pStream->GetChar();
				m_iToken = tkError;
				m_dToken = CDatum(ComposeError(strPattern(ERR_INVALID_CHARACTER, CString(&chChar, 1))));
				bDone = true;
				break;
				}

			default:
				m_iToken = ParseIdentifier(&m_dToken);
				bDone = true;
				break;
			}
		}

	if (retdDatum)
		*retdDatum = m_dToken;

	return m_iToken;
	}

