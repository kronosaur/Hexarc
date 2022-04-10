//	JSON.cpp
//
//	Methods for converting to/from JSON
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

class CJSONParser
	{
	public:
		CJSONParser (IByteStream &Stream) : m_Stream(Stream), m_chChar('\0') { }
		bool ParseDatum (CDatum *retDatum);

	private:
		enum ETokens
			{
			tkError,
			tkDatum,
			tkCloseBracket,
			tkCloseBrace,
			tkColon,
			tkComma,
			};

		ETokens ParseArray (CDatum *retDatum);
		ETokens ParseInteger (int *retiValue);
		ETokens ParseLiteral (CDatum *retDatum);
		ETokens ParseNumber (CDatum *retDatum);
		ETokens ParseString (CDatum *retDatum);
		ETokens ParseStruct (CDatum *retDatum);
		ETokens ParseToken (CDatum *retDatum);
		char ReadChar (void);

		IByteStream &m_Stream;
		char m_chChar;
	};

DECLARE_CONST_STRING(STR_AEON_SENTINEL,					"AEON2011:");
DECLARE_CONST_STRING(TYPENAME_IP_INTEGER,				"ipInteger");
DECLARE_CONST_STRING(TYPENAME_TABLE_REF,				"tableRef");

DECLARE_CONST_STRING(STR_INFINITY,						"\"Infinity\"");
DECLARE_CONST_STRING(STR_NAN,							"\"NaN\"");

const int MAX_IN_MEMORY_SIZE =							4 * 1024 * 1024;

bool CDatum::DeserializeJSON (IByteStream &Stream, CDatum *retDatum)

//	DeserializeJSON
//
//	Deserialize from JSON

	{
	CJSONParser Parse(Stream);
	return Parse.ParseDatum(retDatum);
	}

void CDatum::SerializeJSON (IByteStream &Stream) const

//	SerializeJSON
//
//	Serializes to JSON

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				Stream.Write("null", 4);
			else
				{
				Stream.Write("\"", 1);
				raw_GetString().SerializeJSON(Stream);
				Stream.Write("\"", 1);
				}
			break;

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							Stream.Write("nan", 3);
							break;
							
						case CONST_TRUE:
							Stream.Write("true", 4);
							break;

						default:
							ASSERT(false);
						}
					break;
					}

				case AEON_NUMBER_INTEGER:
					{
					CString sInt = strFromInt((int)HIDWORD(m_dwData));
					Stream.Write(sInt);
					break;
					}

				case AEON_NUMBER_DOUBLE:
					{
					double rValue = raw_GetDouble();
					if (isnan(rValue))
						Stream.Write(STR_NAN);
					else if (isinf(rValue))
						Stream.Write(STR_INFINITY);
					else
						Stream.Write(strFromDouble(rValue));
					break;
					}

				default:
					ASSERT(false);
				}
			break;

		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->Serialize(EFormat::JSON, Stream);
			break;

		default:
			ASSERT(false);
		}
	}

//	CJSONParser ----------------------------------------------------------------

CJSONParser::ETokens CJSONParser::ParseArray (CDatum *retDatum)

//	ParseArray
//
//	Parse an array

	{
	//	Skip the open bracket

	m_chChar = ReadChar();

	//	Create a new array

	CComplexArray *pArray = new CComplexArray;

	//	Parse elements

	while (true)
		{
		CDatum dElement;
		ETokens iToken = ParseToken(&dElement);
		if (iToken == tkCloseBracket)
			break;
		else if (iToken == tkComma)
			;
		else if (iToken == tkDatum)
			{
			//	See if this is an encoded AEON complex datum

			if (pArray->GetCount() == 0 
					&& dElement.GetBasicType() == CDatum::typeString 
					&& strStartsWith(dElement, STR_AEON_SENTINEL))
				{
				//	Get the typename

				char *pPos = ((const CString &)dElement).GetParsePointer() + STR_AEON_SENTINEL.GetLength();
				char *pStart = pPos;
				while (*pPos != ':' && *pPos != '\0')
					pPos++;

				if (*pPos == ':')
					{
					CString sTypename(pStart, pPos - pStart);

					//	Lookup the typename and create the proper complex datum

					IComplexDatum *pDatum = NULL;
					if (strEquals(sTypename, TYPENAME_IP_INTEGER))
						pDatum = new CComplexInteger;
					else if (strEquals(sTypename, TYPENAME_TABLE_REF))
						pDatum = new CAEONTable;
					else
						{
						IComplexFactory *pFactory;
						if (CDatum::FindExternalType(sTypename, &pFactory))
							pDatum = pFactory->Create();
						}

					//	Deserialize

					if (pDatum)
						{
						//	We no longer need the array because we will return a 
						//	complex datum (or an error).

						delete pArray;

						//	Next token

						iToken = ParseToken(&dElement);

						//	The remaining elements are the data.

						TArray<CDatum> Serialized;
						while (iToken != tkCloseBracket)
							{
							//	Skip commas

							if (iToken == tkComma)
								;

							//	Elements

							else if (iToken == tkDatum)
								{
								//	Add the element

								Serialized.Insert(dElement);
								}

							//	Error

							else
								{
								delete pDatum;
								return tkError;
								}

							//	Next

							iToken = ParseToken(&dElement);
							}

						//	Let the base class deserialize

						if (!pDatum->DeserializeJSON(sTypename, Serialized))
							{
							delete pDatum;
							return tkError;
							}

						//	Done!

						*retDatum = CDatum(pDatum);
						return tkDatum;
						}
					}
				}

			//	Otherwise, just add the element

			pArray->Insert(dElement);
			}
		else
			{
			delete pArray;
			return tkError;
			}
		}

	//	Done

	*retDatum = CDatum(pArray);
	return tkDatum;
	}

bool CJSONParser::ParseDatum (CDatum *retDatum)

//	ParseDatum
//
//	Parses the next datum. Returns TRUE if successful and FALSE
//	if there was a parsing error.
//
//	We leave m_chChar on the first character
//	AFTER the datum (or on the character that caused an error).

	{
	if (m_chChar == '\0')
		m_chChar = ReadChar();

	return (ParseToken(retDatum) == tkDatum);
	}

CJSONParser::ETokens CJSONParser::ParseInteger (int *retiValue)

//	ParseInteger
//
//	Parse an integer

	{
	int iValue = 0;
	int iSign = 1;

	while (true)
		{
		if (m_chChar == '-')
			iSign = -1;
		else if (m_chChar >= '0' && m_chChar <= '9')
			iValue = (iValue * 10) + (int)(m_chChar - '0');
		else
			break;

		m_chChar = ReadChar();
		}

	//	Done

	*retiValue = iSign * iValue;
	return tkDatum;
	}

CJSONParser::ETokens CJSONParser::ParseLiteral (CDatum *retDatum)

//	ParseLiteral
//
//	Parse a literal

	{
	if (m_chChar == 'f')
		{
		m_chChar = ReadChar();
		if (m_chChar == 'a')
			{
			m_chChar = ReadChar();
			if (m_chChar == 'l')
				{
				m_chChar = ReadChar();
				if (m_chChar == 's')
					{
					m_chChar = ReadChar();
					if (m_chChar == 'e')
						{
						m_chChar = ReadChar();
						*retDatum = CDatum();
						return tkDatum;
						}
					}
				}
			}
		}
	else if (m_chChar == 'n')
		{
		m_chChar = ReadChar();
		if (m_chChar == 'u')
			{
			m_chChar = ReadChar();
			if (m_chChar == 'l')
				{
				m_chChar = ReadChar();
				if (m_chChar == 'l')
					{
					m_chChar = ReadChar();
					*retDatum = CDatum();
					return tkDatum;
					}
				}
			}
		}
	else if (m_chChar == 't')
		{
		m_chChar = ReadChar();
		if (m_chChar == 'r')
			{
			m_chChar = ReadChar();
			if (m_chChar == 'u')
				{
				m_chChar = ReadChar();
				if (m_chChar == 'e')
					{
					m_chChar = ReadChar();
					*retDatum = CDatum(true);
					return tkDatum;
					}
				}
			}
		}

	//	Error

	return tkError;
	}

CJSONParser::ETokens CJSONParser::ParseNumber (CDatum *retDatum)

//	ParseNumber
//
//	Parse a JSON number

	{
	CStringBuffer NumberStr;
	int iOffset = 0;

	//	Parse the integer part

	int iSign = 1;
	if (m_chChar == '-')
		{
		iSign = -1;

		NumberStr.Write(&m_chChar, 1);
		m_chChar = ReadChar();
		iOffset++;
		}

	int iIntOffset = iOffset;
	while (m_chChar >= '0' && m_chChar <= '9')
		{
		NumberStr.Write(&m_chChar, 1);
		m_chChar = ReadChar();
		iOffset++;
		}

	if (iIntOffset == iOffset)
		//	Invalid integer
		return tkError;

	//	Do we have a fractional part?

	int iFracOffset = -1;
	if (m_chChar == '.')
		{
		NumberStr.Write(&m_chChar, 1);
		m_chChar = ReadChar();
		iOffset++;

		iFracOffset = iOffset;

		//	Check for NAN

		if (m_chChar == '#')
			{
			while (m_chChar == '#' || (m_chChar >= 'A' && m_chChar <= 'Z'))
				{
				NumberStr.Write(&m_chChar, 1);
				m_chChar = ReadChar();
				iOffset++;
				}
			}
		else
			{
			while (m_chChar >= '0' && m_chChar <= '9')
				{
				NumberStr.Write(&m_chChar, 1);
				m_chChar = ReadChar();
				iOffset++;
				}
			}

		if (iFracOffset == iOffset)
			//	Invalid float
			return tkError;
		}

	//	Do we have an exponential part?

	int iExpOffset = -1;
	int iExpSign = 1;
	if (m_chChar == 'e' || m_chChar == 'E')
		{
		NumberStr.Write(&m_chChar, 1);
		m_chChar = ReadChar();
		iOffset++;

		if (m_chChar == '+')
			{
			NumberStr.Write(&m_chChar, 1);
			m_chChar = ReadChar();
			iOffset++;
			}
		else if (m_chChar == '-')
			{
			NumberStr.Write(&m_chChar, 1);
			m_chChar = ReadChar();
			iOffset++;

			iExpSign = -1;
			}

		iExpOffset = iOffset;
		while (m_chChar >= '0' && m_chChar <= '9')
			{
			NumberStr.Write(&m_chChar, 1);
			m_chChar = ReadChar();
			iOffset++;
			}

		if (iExpOffset == iOffset)
			//	Invalid exponent
			return tkError;
		}

	//	LATER: For now we just do everything with atof. Later we can be more
	//	careful	about parsing integers separately.

	CString sNumber = CString::CreateFromHandoff(NumberStr);
	double rNumber = strToDouble(sNumber);

	int iNumber = (int)rNumber;
	if ((double)iNumber == rNumber)
		*retDatum = CDatum(iNumber);
	else
		*retDatum = CDatum(rNumber);

	return tkDatum;
	}

CJSONParser::ETokens CJSONParser::ParseString (CDatum *retDatum)

//	ParseString
//
//	Parse a JSON string

	{
	CMemoryBuffer Stream(4096);
	CComplexBinaryFile *pBigData = NULL;

	//	Skip the open quote

	m_chChar = ReadChar();

	//	Keep looping

	while (m_chChar != '\"' && m_chChar != '\0')
		{
		if (m_chChar == '\\')
			{
			m_chChar = ReadChar();
			switch (m_chChar)
				{
				case '\"':
					Stream.Write("\"", 1);
					break;

				case '\\':
					Stream.Write("\\", 1);
					break;

				case '/':
					Stream.Write("/", 1);
					break;

				case 'b':
					Stream.Write("\b", 1);
					break;

				case 'f':
					Stream.Write("\f", 1);
					break;

				case 'n':
					Stream.Write("\n", 1);
					break;

				case 'r':
					Stream.Write("\r", 1);
					break;

				case 't':
					Stream.Write("\t", 1);
					break;

				case 'u':
					{
					char szBuffer[5];
					szBuffer[0] = ReadChar();
					szBuffer[1] = ReadChar();
					szBuffer[2] = ReadChar();
					szBuffer[3] = ReadChar();
					szBuffer[4] = '\0';

					DWORD dwHex = (DWORD)strParseIntOfBase(szBuffer, 16, (int)'?');
					CString sChar = strEncodeUTF8Char((UTF32)dwHex);
					Stream.Write(sChar);
					break;
					}

				default:
					{
					if (pBigData)
						delete pBigData;
					return tkError;
					}
				}
			}
		else
			Stream.Write(&m_chChar, 1);

		m_chChar = ReadChar();

		//	If this string has gotten big, then start saving to disk

		if (Stream.GetLength() > MAX_IN_MEMORY_SIZE)
			{
			if (pBigData == NULL)
				pBigData = new CComplexBinaryFile;

			pBigData->Append(Stream);
			Stream.SetLength(0);
			}
		}

	//	Append any remaining data

	if (pBigData)
		pBigData->Append(Stream);

	//	If we hit the end, then we have an error

	if (m_chChar == '\0')
		{
		if (pBigData)
			delete pBigData;
		return tkError;
		}

	//	Otherwise, read the next char

	m_chChar = ReadChar();

	//	Done. We return either a memory block or a binary file blob.

	if (pBigData)
		{
#ifdef DEBUG
		printf("JSON string converted to CComplexBinaryFile.\n");
#endif
		*retDatum = CDatum(pBigData);
		}
	else
		*retDatum = CDatum(CString(Stream.GetPointer(), Stream.GetLength()));

	return tkDatum;
	}

CJSONParser::ETokens CJSONParser::ParseStruct (CDatum *retDatum)

//	ParseStruct
//
//	Parse a struct

	{
	//	Skip the open brace

	m_chChar = ReadChar();

	//	Create a new array

	CComplexStruct *pStruct = new CComplexStruct;

	//	Parse elements

	while (true)
		{
		CDatum dElement;
		ETokens iToken = ParseToken(&dElement);
		if (iToken == tkCloseBrace)
			break;
		else if (iToken == tkComma)
			continue;
		else if (iToken == tkDatum)
			{
			CDatum dValue;

			//	Parse the colon

			iToken = ParseToken(&dValue);
			if (iToken != tkColon)
				{
				delete pStruct;
				return tkError;
				}

			//	Parse the value

			iToken = ParseToken(&dValue);
			if (iToken != tkDatum)
				{
				delete pStruct;
				return tkError;
				}

			//	Add

			pStruct->SetElement(dElement, dValue);
			}
		else
			{
			delete pStruct;
			return tkError;
			}
		}

	//	Done

	*retDatum = CDatum(pStruct);
	return tkDatum;
	}

CJSONParser::ETokens CJSONParser::ParseToken (CDatum *retDatum)

//	ParseToken
//
//	Parse a datum or token. We expect that m_chChar is initialized
//	at the first character for us to parse. 
//
//	We leave m_chChar on the first character
//	AFTER the token (or on the character that caused an error).

	{
	//	Skip Parse whitespace

	while (m_chChar == ' ' || m_chChar == '\t' || m_chChar == '\r' || m_chChar == '\n')
		m_chChar = ReadChar();

	//	Parse token

	switch (m_chChar)
		{
		case '\0':
			//	Unexpected end of file
			return tkError;

		case ':':
			m_chChar = ReadChar();
			return tkColon;

		case '{':
			return ParseStruct(retDatum);

		case '}':
			m_chChar = ReadChar();
			return tkCloseBrace;

		case '[':
			return ParseArray(retDatum);

		case ']':
			m_chChar = ReadChar();
			return tkCloseBracket;

		case ',':
			m_chChar = ReadChar();
			return tkComma;

		case '\"':
			return ParseString(retDatum);

		default:
			{
			if (m_chChar == '-' || (m_chChar >= '0' && m_chChar <= '9'))
				return ParseNumber(retDatum);
			else
				return ParseLiteral(retDatum);
			}
		}
	}

char CJSONParser::ReadChar (void)

//	ReadChar
//
//	Reads the next character. Returns '\0' if we reached the end

	{
	char chChar;
	int iRead = m_Stream.Read(&chChar, 1);
	return (iRead > 0 ? chChar : '\0');
	}
