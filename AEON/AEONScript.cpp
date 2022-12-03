//	AEONScript.cpp
//
//	Methods for converting to/from AEONScript
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	value
//		array
//		comment
//		datetime
//		number
//		string
//		struct
//
//		TRUE
//		NIL
//
//	array
//		( )
//		( elements )
//
//	char
//		\"
//		\\
//		\/
//		\b
//		\f
//		\n
//		\r
//		\t
//		\u-four-hex-digits
//
//	chars
//		char
//		char chars
//
//	comment
//		//charsEOL
//		/*chars*/
//
//	datetime
//		#yyyy-mm-ddThh:mm:ss.millisec
//
//	digits
//		digit
//		digit digits
//
//	elements
//		value
//		value elements
//
//	int
//		digit
//		digit1-9 digits
//		- digit
//		- digit1-9 digits
//
//	members
//		pair
//		pair members
//
//	number
//		int
//
//	pair
//		string : value
//
//	string
//		""
//		"chars"
//		chars-that-don't-need-quotes
//
//	struct
//		{ }
//		{ members }
//
//	external
//		[typename:base64-data]
//		[typename:struct]

#include "stdafx.h"

DECLARE_CONST_STRING(STR_NAN,							"nan")
DECLARE_CONST_STRING(STR_NIL,							"nil")
DECLARE_CONST_STRING(STR_NULL,							"null")
DECLARE_CONST_STRING(STR_TRUE,							"true")

DECLARE_CONST_STRING(TYPENAME_ANNOTATED,				"annotated");
DECLARE_CONST_STRING(TYPENAME_BINARY,					"binary")
DECLARE_CONST_STRING(TYPENAME_BINARY_FILE,				"binaryFile")
DECLARE_CONST_STRING(TYPENAME_DATATYPE,					"datatype")
DECLARE_CONST_STRING(TYPENAME_ENUM,						"enum")
DECLARE_CONST_STRING(TYPENAME_ERROR,					"error")
DECLARE_CONST_STRING(TYPENAME_HEXE_ERROR,				"hexeError")
DECLARE_CONST_STRING(TYPENAME_IMAGE32,					"image32")
DECLARE_CONST_STRING(TYPENAME_IP_INTEGER,				"ipInteger")
DECLARE_CONST_STRING(TYPENAME_TABLE,					"table");
DECLARE_CONST_STRING(TYPENAME_TABLE_REF,				"tableRef");
DECLARE_CONST_STRING(TYPENAME_TEXT_LINES,				"textLines");
DECLARE_CONST_STRING(TYPENAME_TIME_SPAN,				"timeSpan")
DECLARE_CONST_STRING(TYPENAME_VECTOR_2D,				"vector2D")

DECLARE_CONST_STRING(ERR_COLON_EXPECTED,				"Colon expected in struct: %s.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_TOKEN_IN_STRUCT,	"Unexpected token in struct: %s.")

const int MAX_IN_MEMORY_SIZE =							4 * 1024 * 1024;

size_t CDatum::CalcSerializeSizeAEONScript (EFormat iFormat) const

//	CalcSerializeSizeAEONScript
//
//	Computes the size of the serialization (in bytes).
//
//	NOTE: We return an approximation, not an accurage value, because otherwise 
//	we would have to traverse the entire data.

	{
	size_t TotalSize = 0;

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				TotalSize = 3;	//	"nil"
			else
				{
				//	We approximate the size.

				const CString &sString = raw_GetString();
				TotalSize = sString.GetLength();
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
							TotalSize = 3;	//	"nan"
							break;

						case CONST_TRUE:
							TotalSize = 4;	//	"true"
							break;

						default:
							ASSERT(false);
						}
					break;
					}

				case AEON_NUMBER_INTEGER:
					TotalSize = 6;
					break;

				case AEON_NUMBER_ENUM:
					TotalSize = 100;
					break;

				case AEON_NUMBER_DOUBLE:
					TotalSize = 8;
					break;

				default:
					ASSERT(false);
				}
			break;

		case AEON_TYPE_COMPLEX:
			TotalSize = raw_GetComplex()->CalcSerializeSizeAEONScript(iFormat);
			break;

		default:
			ASSERT(false);
		}

	return TotalSize;
	}

bool CDatum::DeserializeAEONScript (IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum)

//	DeserializeAEONScript
//
//	Deserialize from AEONScript

	{
	CCharStream CharStream;
	CharStream.Init(Stream);

	CAEONScriptParser Parse(&CharStream, pExtension);
	if (!Parse.ParseDatum(retDatum))
		return false;

	//	If we're not at the end of the stream, backup one because the parser
	//	reads one extra character (to decide that it has reached the end).

	if (Stream.GetPos() < Stream.GetStreamLength())
		CharStream.UnreadChar();

	//	Done

	return true;
	}

void CDatum::SerializeAEONScript (EFormat iFormat, IByteStream &Stream) const

//	SerializeAEONScript
//
//	Serializes to AEONScript

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				Stream.Write("nil", 3);
			else
				{
				const CString &sString = raw_GetString();
				bool bQuote = CAEONScriptParser::HasSpecialChars(sString);
				if (bQuote)
					Stream.Write("\"", 1);
				sString.SerializeJSON(Stream);
				if (bQuote)
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

				case AEON_NUMBER_ENUM:
					SerializeEnum(iFormat, Stream);
					break;

				case AEON_NUMBER_DOUBLE:
					{
					CString sNumber = strFromDouble(raw_GetDouble());
					Stream.Write(sNumber);
					break;
					}

				default:
					ASSERT(false);
				}
			break;

		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->Serialize(iFormat, Stream);
			break;

		default:
			ASSERT(false);
		}
	}

void CDatum::SerializeEnum (EFormat iFormat, IByteStream &Stream) const
	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("[", 1);
			Stream.Write(TYPENAME_ENUM);
			Stream.Write(":", 1);

			CString sInt = strFromInt((int)HIDWORD(m_dwData));
			Stream.Write(sInt);
			Stream.Write(" ", 1);

			DWORD dwDatatypeID = GetNumberIndex();
			CDatum dType = CAEONTypes::Get(dwDatatypeID);

			dType.Serialize(iFormat, Stream);
			Stream.Write("]", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("[", 1);
			int iValue = (int)HIDWORD(m_dwData);
			CString sInt = strFromInt(iValue);
			Stream.Write(sInt);
			Stream.Write(",", 1);

			DWORD dwDatatypeID = GetNumberIndex();
			CDatum dType = CAEONTypes::Get(dwDatatypeID);
			const IDatatype& Datatype = dType;
			int iIndex = Datatype.FindMemberByOrdinal(iValue);

			Stream.Write("\"", 1);
			if (iIndex == -1)
				Stream.Write(STR_NULL);
			else
				Stream.Write(Datatype.GetMember(iIndex).sName);
			Stream.Write("\"", 1);

			Stream.Write("]", 1);
			break;
			}

		default:
			ASSERT(false);
		}
	}

//	CAEONScriptParser ----------------------------------------------------------------

bool CAEONScriptParser::HasSpecialChars (const CString &sString)

//	HasSpecialChars
//
//	Returns TRUE if the string has any special characters that
//	must be placed inside quotes.

	{
	char *pPos = sString.GetParsePointer();
	char *pEndPos = pPos + sString.GetLength();

	//	Blank strings always need to be quoted.

	if (pPos == pEndPos)
		return true;

	//	If we start with a number, then we need quotes

	if (strIsDigit(pPos))
		return true;

	//	Check for reserved characters

	while (pPos < pEndPos)
		{
		//	We check for alpha first because it is faster (and most common)

		if (strIsASCIIAlpha(pPos))
			;

		//	Underscores and periods are OK

		else if (*pPos == '_' || *pPos == '.')
			;

		//	Otherwise...

		else if (*pPos == ' ' || strIsASCIIControl(pPos) || strIsASCIISymbol(pPos))
			return true;

		pPos++;
		}

	//	If the string is null, nil, nan, or true, then we need to quote it, 
	//	otherwise we will interpret it as an identifier.

	const char *pFirst = sString.GetParsePointer();
	if (*pFirst == 'n' || *pFirst == 'N')
		{
		if (strEqualsNoCase(sString, STR_NAN))
			return true;
		else if (strEqualsNoCase(sString, STR_NIL))
			return true;
		else if (strEqualsNoCase(sString, STR_NULL))
			return true;
		}
	else if (*pFirst == 't' || *pFirst == 'T')
		{
		if (strEqualsNoCase(sString, STR_TRUE))
			return true;
		}

	return false;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseArray (CDatum *retDatum)

//	ParseArray
//
//	Parse an array

	{
	//	See if the extension will handle it...

	if (m_pExtension && m_pExtension->ParseAEONArray(*m_pStream, retDatum))
		{
		if (retDatum->IsError())
			return tkError;
		else
			return tkDatum;
		}

	//	Skip the open paren

	m_pStream->ReadChar();

	//	Create a new array

	CComplexArray *pArray = new CComplexArray;

	//	Parse elements

	while (true)
		{
		CDatum dElement;
		ETokens iToken = ParseToken(&dElement);
		if (iToken == tkCloseParen)
			break;
		else if (iToken == tkDatum)
			pArray->Insert(dElement);
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

bool CAEONScriptParser::ParseComment (void)

//	ParseComment
//
//	Current character is the first '/'. We consume characters until we reach
//	the end of the comment (we leave the character at the first AFTER we've
//	found the end of line).
//
//	Returns FALSE if we find an error.

	{
	//	Skip the first slash

	char chChar = m_pStream->ReadChar();

	//	If we have a second slash, parse until the end of the line

	if (chChar == '/')
		{
		//	Read until we get an end of line

		do
			{
			chChar = m_pStream->ReadChar();
			}
		while (chChar != '\0' && chChar != '\n' && chChar != '\r');
		}

	//	If we have a * then parse the end of the comment block

	else if (chChar == '*')
		{
		bool bFoundStar = false;
		bool bFoundCommentEnd = false;

		do
			{
			chChar = m_pStream->ReadChar();

			if (chChar == '\0')
				break;
			else if (chChar == '/' && bFoundStar)
				break;

			bFoundStar = (chChar == '*');
			}
		while (true);

		//	If we found the end, consume one more character

		if (chChar == '/')
			chChar = m_pStream->ReadChar();
		}

	//	Otherwise, an error

	else
		return false;

	//	Done

	return true;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseDateTime (CDatum *retDatum)

//	ParseDateTime
//
//	Parse a dateTime

	{
	//	Parse each component

	int iYear;
	m_pStream->ReadChar();
	ParseInteger(&iYear);
	if (m_pStream->GetChar() != '-' || iYear < 1 || iYear > 30827)
		return tkError;

	int iMonth;
	m_pStream->ReadChar();
	ParseInteger(&iMonth);
	if (m_pStream->GetChar() != '-' || iMonth < 1 || iMonth > 12)
		return tkError;

	int iDay;
	m_pStream->ReadChar();
	ParseInteger(&iDay);
	if (m_pStream->GetChar() != 'T' || iDay < 1 || iDay > CDateTime::GetDaysInMonth(iMonth, iYear))
		return tkError;

	int iHour;
	m_pStream->ReadChar();
	ParseInteger(&iHour);
	if (m_pStream->GetChar() != ':' || iHour < 0 || iHour > 23)
		return tkError;

	int iMinute;
	m_pStream->ReadChar();
	ParseInteger(&iMinute);
	if (m_pStream->GetChar() != ':' || iMinute < 0 || iMinute > 59)
		return tkError;

	int iSecond;
	m_pStream->ReadChar();
	ParseInteger(&iSecond);
	if (m_pStream->GetChar() != '.' || iSecond < 0 || iSecond > 59)
		return tkError;

	int iMillisecond;
	m_pStream->ReadChar();
	ParseInteger(&iMillisecond);
	if (iMillisecond < 0 || iMillisecond > 999)
		return tkError;

	//	Create a new DateTime

	CDateTime DateTime;
	DateTime.SetDate(iDay, iMonth, iYear);
	DateTime.SetTime(iHour, iMinute, iSecond, iMillisecond);
	CComplexDateTime *pDateTime = new CComplexDateTime(DateTime);

	//	Done

	*retDatum = CDatum(pDateTime);
	return tkDatum;
	}

bool CAEONScriptParser::ParseDatum (CDatum *retDatum)

//	ParseDatum
//
//	Parses the next datum. Returns TRUE if successful and FALSE
//	if there was a parsing error.
//
//	We leave m_chChar on the first character
//	AFTER the datum (or on the character that caused an error).

	{
	ETokens iToken = ParseToken(retDatum);
	if (iToken == tkEOF)
		{
		if (retDatum)
			*retDatum = CDatum();

		return true;
		}
	else if (iToken == tkDatum)
		return true;
	else
		return false;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseExternal (CDatum *retDatum)

//	ParseExternal
//
//	Parses an external type

	{
	//	Skip the open bracket

	m_pStream->ReadChar();

	//	Read the typename

	CDatum dTypename;
	ETokens iToken = ParseToken(&dTypename);
	if (iToken != tkDatum)
		return tkError;

	//	Parse the colon

	if (ParseToken(retDatum) != tkColon)
		return tkError;

	//	Create the object based on the typename

	IComplexDatum *pDatum;
	if (strEquals(dTypename, TYPENAME_IP_INTEGER))
		pDatum = new CComplexInteger;
	else if (strEquals(dTypename, TYPENAME_ANNOTATED))
		pDatum = new CAEONAnnotated;
	else if (strEquals(dTypename, TYPENAME_BINARY))
		pDatum = new CComplexBinary;
	else if (strEquals(dTypename, TYPENAME_BINARY_FILE))
		pDatum = new CComplexBinaryFile;
	else if (strEquals(dTypename, TYPENAME_DATATYPE))
		pDatum = new CComplexDatatype(NULL);
	else if (strEquals(dTypename, TYPENAME_ERROR)
			|| strEquals(dTypename, TYPENAME_HEXE_ERROR))
		pDatum = new CAEONError;
	else if (strEquals(dTypename, TYPENAME_IMAGE32))
		pDatum = new CComplexImage32;
	else if (strEquals(dTypename, TYPENAME_TABLE)
			|| strEquals(dTypename, TYPENAME_TABLE_REF))
		pDatum = new CAEONTable;
	else if (strEquals(dTypename, TYPENAME_TEXT_LINES))
		pDatum = new CAEONLines;
	else if (strEquals(dTypename, TYPENAME_TIME_SPAN))
		pDatum = new CAEONTimeSpan;
	else if (strEquals(dTypename, TYPENAME_VECTOR_2D))
		pDatum = new CAEONVector2D;
	else if (strEquals(dTypename, TYPENAME_ENUM))
		{
		CDatum dInt;
		if (ParseToken(&dInt) != tkDatum)
			return tkError;

		CDatum dType;
		if (ParseToken(&dType) != tkDatum || dType.GetBasicType() != CDatum::typeDatatype)
			return tkError;

		*retDatum = CDatum::CreateEnum((int)dInt, dType);

		//	Parse the closing bracket

		if (ParseToken(retDatum) != tkCloseBracket)
			return tkError;

		return tkDatum;
		}
	else
		{
		IComplexFactory *pFactory;
		if (!CDatum::FindExternalType(dTypename, &pFactory))
			return tkError;

		pDatum = pFactory->Create();
		}

	//	Deserialize the object

	if (!pDatum->DeserializeAEONScript(CDatum::EFormat::AEONScript, dTypename, m_pStream))
		return tkError;

	//	Parse the closing bracket

	if (ParseToken(retDatum) != tkCloseBracket)
		return tkError;

	//	Done

	*retDatum = CDatum(pDatum);
	return tkDatum;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseInteger (int *retiValue)

//	ParseInteger
//
//	Parse an integer

	{
	bool bFirstChar = true;
	int iValue = 0;
	int iSign = 1;

	char chChar = m_pStream->GetChar();
	while (true)
		{
		if (chChar == '-')
			{
			if (bFirstChar)
				iSign = -1;
			else
				break;
			}
		else if (chChar >= '0' && chChar <= '9')
			iValue = (iValue * 10) + (int)(chChar - '0');
		else
			break;

		chChar = m_pStream->ReadChar();
		bFirstChar = false;
		}

	//	Done

	*retiValue = iSign * iValue;
	return tkDatum;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseLiteral (CDatum *retDatum)

//	ParseLiteral
//
//	Parse a literal

	{
	CMemoryBuffer Stream(4096);

	char chChar = m_pStream->GetChar();
	do
		{
		Stream.Write(&chChar, 1);
		chChar = m_pStream->ReadChar();
		}
	while (chChar != ' ' && chChar != ',' && chChar != ':' && chChar != ')' && chChar != '}' && chChar != ']' && chChar != '\r' && chChar != '\n' && chChar != '\t' && chChar != '\0');

	CString sLiteral(Stream.GetPointer(), Stream.GetLength());
	if (strEquals(strToLower(sLiteral), STR_NIL))
		*retDatum = CDatum();
	else if (strEquals(strToLower(sLiteral), STR_NULL))
		*retDatum = CDatum();
	else if (strEquals(strToLower(sLiteral), STR_NAN))
		*retDatum = CDatum::CreateNaN();
	else if (strEquals(strToLower(sLiteral), STR_TRUE))
		*retDatum = CDatum(true);
	else
		*retDatum = CDatum(sLiteral);

	return tkDatum;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseNumber (CDatum *retDatum)

//	ParseNumber
//
//	Parse a JSON number

	{
	char chChar = m_pStream->GetChar();
	CStringBuffer NumberStr;
	int iOffset = 0;

	//	Parse the integer part

	int iSign = 1;
	if (chChar == '-')
		{
		iSign = -1;

		NumberStr.Write(&chChar, 1);
		chChar = m_pStream->ReadChar();
		iOffset++;
		}

	int iIntOffset = iOffset;
	while (chChar >= '0' && chChar <= '9')
		{
		NumberStr.Write(&chChar, 1);
		chChar = m_pStream->ReadChar();
		iOffset++;
		}

	if (iIntOffset == iOffset)
		//	Invalid integer
		return tkError;

	//	Do we have a fractional part?

	int iFracOffset = -1;
	if (chChar == '.')
		{
		NumberStr.Write(&chChar, 1);
		chChar = m_pStream->ReadChar();
		iOffset++;

		iFracOffset = iOffset;

		//	Check for NAN

		if (chChar == '#')
			{
			while (chChar == '#' || (chChar >= 'A' && chChar <= 'Z'))
				{
				NumberStr.Write(&chChar, 1);
				chChar = m_pStream->ReadChar();
				iOffset++;
				}
			}

		//	Normal

		else
			{
			while (chChar >= '0' && chChar <= '9')
				{
				NumberStr.Write(&chChar, 1);
				chChar = m_pStream->ReadChar();
				iOffset++;
				}
			}

		//	Make sure there is a factional part

		if (iFracOffset == iOffset)
			{
			if (chChar != 'e' && chChar != 'E')
				//	Invalid float
				return tkError;

			iFracOffset = -1;
			}
		}

	//	Do we have an exponential part?

	int iExpOffset = -1;
	int iExpSign = 1;
	if (chChar == 'e' || chChar == 'E')
		{
		NumberStr.Write(&chChar, 1);
		chChar = m_pStream->ReadChar();
		iOffset++;

		if (chChar == '+')
			{
			NumberStr.Write(&chChar, 1);
			chChar = m_pStream->ReadChar();
			iOffset++;
			}
		else if (chChar == '-')
			{
			NumberStr.Write(&chChar, 1);
			chChar = m_pStream->ReadChar();
			iOffset++;

			iExpSign = -1;
			}

		iExpOffset = iOffset;
		while (chChar >= '0' && chChar <= '9')
			{
			NumberStr.Write(&chChar, 1);
			chChar = m_pStream->ReadChar();
			iOffset++;
			}

		if (iExpOffset == iOffset)
			//	Invalid exponent
			return tkError;
		}

	//	If we have neither a fractional part nor an exponential part, then just
	//	parse as an integer.

	if (iFracOffset == -1 && iExpOffset == -1)
		{
		CString sNumber = CString::CreateFromHandoff(NumberStr);
		int iLen = sNumber.GetLength();
		if (strOverflowsInteger32(sNumber))
			{
			CIPInteger Value;
			Value.InitFromString(sNumber);
			CDatum::CreateIPIntegerFromHandoff(Value, retDatum);
			}
		else
			*retDatum = CDatum(strToInt(sNumber, 0));
		}
	else
		{
		//	LATER: For now we just do everything with atof. Later we can be more
		//	careful about parsing integers separately.

		CString sNumber = CString::CreateFromHandoff(NumberStr);
		*retDatum = CDatum(strToDouble(sNumber));
		}

	return tkDatum;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseString (CDatum *retDatum)

//	ParseString
//
//	Parse a JSON string

	{
	CMemoryBuffer Stream(4096);
	CComplexBinaryFile *pBigData = NULL;

	//	Skip the open quote

	char chChar = m_pStream->ReadChar();

	//	Keep looping

	while (chChar != '\"' && chChar != '\0')
		{
		if (chChar == '\\')
			{
			chChar = m_pStream->ReadChar();
			switch (chChar)
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
					szBuffer[0] = m_pStream->ReadChar();
					szBuffer[1] = m_pStream->ReadChar();
					szBuffer[2] = m_pStream->ReadChar();
					szBuffer[3] = m_pStream->ReadChar();
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
		else if (strIsASCIIControl(&chChar))
			{
			if (pBigData)
				delete pBigData;
			return tkError;
			}
		else
			Stream.Write(&chChar, 1);

		chChar = m_pStream->ReadChar();

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

	if (chChar == '\0')
		{
		if (pBigData)
			delete pBigData;
		return tkError;
		}

	//	Otherwise, read the next char

	m_pStream->ReadChar();

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

CAEONScriptParser::ETokens CAEONScriptParser::ParseStruct (CDatum *retDatum)

//	ParseStruct
//
//	Parse a struct

	{
	//	Skip the open brace

	m_pStream->ReadChar();

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
				if (iToken == tkError)
					*retDatum = dValue;
				else
					*retDatum = strPattern(ERR_COLON_EXPECTED, dValue.AsString());
				delete pStruct;
				return tkError;
				}

			//	Parse the value

			iToken = ParseToken(&dValue);
			if (iToken != tkDatum)
				{
				if (iToken == tkError)
					*retDatum = dValue;
				delete pStruct;
				return tkError;
				}

			//	Add

			pStruct->SetElement(dElement, dValue);
			}
		else if (iToken == tkError)
			{
			*retDatum = dElement;
			delete pStruct;
			return tkError;
			}
		else
			{
			*retDatum = strPattern(ERR_UNEXPECTED_TOKEN_IN_STRUCT, dElement.AsString());
			delete pStruct;
			return tkError;
			}
		}

	//	Done

	*retDatum = CDatum(pStruct);
	return tkDatum;
	}

CAEONScriptParser::ETokens CAEONScriptParser::ParseToken (CDatum *retDatum)

//	ParseToken
//
//	Parse a datum or token. We expect that m_chChar is initialized
//	at the first character for us to parse. 
//
//	We leave m_chChar on the first character
//	AFTER the token (or on the character that caused an error).

	{
	while (true)
		{
		char chChar = m_pStream->GetChar();

		//	Skip Parse whitespace

		while (chChar == ' ' || chChar == '\t' || chChar == '\r' || chChar == '\n')
			chChar = m_pStream->ReadChar();

		//	Parse token

		switch (chChar)
			{
			case '\0':
				//	Unexpected end of file
				return tkEOF;

			case ':':
				chChar = m_pStream->ReadChar();
				return tkColon;

			case ',':
				chChar = m_pStream->ReadChar();
				return tkComma;

			case '{':
				return ParseStruct(retDatum);

			case '}':
				chChar = m_pStream->ReadChar();
				return tkCloseBrace;

			case '(':
				return ParseArray(retDatum);

			case ')':
				chChar = m_pStream->ReadChar();
				return tkCloseParen;

			case '[':
				return ParseExternal(retDatum);

			case ']':
				chChar = m_pStream->ReadChar();
				return tkCloseBracket;

			case '/':
				if (!ParseComment())
					return tkError;

				//	Loop and get another token
				break;

			case '\"':
				return ParseString(retDatum);

			case '#':
				return ParseDateTime(retDatum);

			default:
				{
				if (chChar == '-' || (chChar >= '0' && chChar <= '9'))
					return ParseNumber(retDatum);
				else
					return ParseLiteral(retDatum);
				}
			}
		}
	}

