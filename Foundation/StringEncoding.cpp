//	StringEncoding.cpp
//
//	String encoding utilities
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_STRING_ENCODING_LT,	"lt");
DECLARE_CONST_STRING(STR_STRING_ENCODING_GT,	"gt");
DECLARE_CONST_STRING(STR_STRING_ENCODING_AMP,	"amp");
DECLARE_CONST_STRING(STR_STRING_ENCODING_QUOT,	"quot");
DECLARE_CONST_STRING(STR_STRING_ENCODING_APOS,	"apos");
DECLARE_CONST_STRING(STR_STRING_ENCODING_UNKNOWN_ENCODING_TYPE,	"Unknown encoding type.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_BINARY_LENGTH,	"Invalid binary length.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_UTF_8_BYTE_SEQUENCE,	"Invalid UTF-8 byte sequence.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_UTF_16_BYTE_LENGTH,	"Invalid UTF-16 byte length.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_UTF_16_SURROGATE_PAIR,	"Invalid UTF-16 surrogate pair.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_UTF_32_BYTE_LENGTH,	"Invalid UTF-32 byte length.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_UTF_32_CODE_POINT,	"Invalid UTF-32 code point.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_ASCII_BYTE_SEQUENCE,	"Invalid ASCII byte sequence.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_UNKNOWN_CHARACTER_SET,	"Unknown character set.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_INVALID_UTF_8_STRING,	"Invalid UTF-8 string.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_CHARACTER_CANNOT_BE_ENCODED_AS_ASCII,	"Character cannot be encoded as ASCII.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_CHARACTER_CANNOT_BE_ENCODED_AS_LATIN1,	"Character cannot be encoded as Latin1.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_CHARACTER_CANNOT_BE_ENCODED_AS_WINDOWS_1252,	"Character cannot be encoded as Windows1252.");
DECLARE_CONST_STRING(STR_STRING_ENCODING_UTF8,	"UTF8");
DECLARE_CONST_STRING(STR_STRING_ENCODING_UTF16_BE,	"UTF16BE");
DECLARE_CONST_STRING(STR_STRING_ENCODING_UTF16_LE,	"UTF16LE");
DECLARE_CONST_STRING(STR_STRING_ENCODING_UTF32_BE,	"UTF32BE");
DECLARE_CONST_STRING(STR_STRING_ENCODING_UTF32_LE,	"UTF32LE");
DECLARE_CONST_STRING(STR_STRING_ENCODING_ASCII,	"ASCII");
DECLARE_CONST_STRING(STR_STRING_ENCODING_LATIN1,	"Latin1");
DECLARE_CONST_STRING(STR_STRING_ENCODING_WINDOWS_1252,	"Windows1252");
DECLARE_CONST_STRING(STR_STRING_ENCODING_URL_COMPONENT,	"urlComponent");
DECLARE_CONST_STRING(STR_STRING_ENCODING_URL_PATH_SEGMENT,	"urlPathSegment");
DECLARE_CONST_STRING(STR_STRING_ENCODING_URL_QUERY_VALUE,	"urlQueryValue");
DECLARE_CONST_STRING(STR_STRING_ENCODING_FORM_COMPONENT,	"formComponent");
DECLARE_CONST_STRING(STR_STRING_ENCODING_JSON_STRING_LITERAL,	"jsonStringLiteral");
DECLARE_CONST_STRING(STR_STRING_ENCODING_JSON_STRING_CONTENT,	"jsonStringContent");
DECLARE_CONST_STRING(STR_STRING_ENCODING_C_STRING_LITERAL,	"cStringLiteral");
DECLARE_CONST_STRING(STR_STRING_ENCODING_C_STRING_CONTENT,	"cStringContent");
DECLARE_CONST_STRING(STR_STRING_ENCODING_HTML_TEXT,	"htmlText");
DECLARE_CONST_STRING(STR_STRING_ENCODING_HTML_ATTRIBUTE,	"htmlAttribute");
DECLARE_CONST_STRING(STR_STRING_ENCODING_BASE64,	"base64");
DECLARE_CONST_STRING(STR_STRING_ENCODING_BASE64_URL,	"base64URL");
DECLARE_CONST_STRING(STR_STRING_ENCODING_HEX,	"hex");

namespace
	{
	//	Undefined Windows-1252 bytes map to U+FFFD so decoded strings always
	//	satisfy Foundation's UTF-8 invariant.

	constexpr UTF32 WINDOWS_1252_HIGH[] =
		{
		0x20ac, 0xfffd, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
		0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0xfffd, 0x017d, 0xfffd,
		0xfffd, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
		0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0xfffd, 0x017e, 0x0178,
		};

	UTF32 DecodeWindows1252 (BYTE byValue)
		{
		if (byValue >= 0x80 && byValue <= 0x9f)
			return WINDOWS_1252_HIGH[byValue - 0x80];
		else
			return byValue;
		}

	bool EncodeWindows1252 (UTF32 dwCodePoint, BYTE *retbyValue)
		{
		if (dwCodePoint <= 0x7f || (dwCodePoint >= 0xa0 && dwCodePoint <= 0xff))
			{
			if (retbyValue) *retbyValue = (BYTE)dwCodePoint;
			return true;
			}

		for (int i = 0; i < SIZEOF_STATIC_ARRAY(WINDOWS_1252_HIGH); i++)
			if (WINDOWS_1252_HIGH[i] != 0xfffd && WINDOWS_1252_HIGH[i] == dwCodePoint)
				{
				if (retbyValue) *retbyValue = (BYTE)(0x80 + i);
				return true;
				}

		return false;
		}

	bool DecodeUTF8CharStrict (const char *&pPos, const char *pEndPos, UTF32 *retdwCodePoint)
		{
		if (pPos >= pEndPos)
			return false;

		const BYTE *pSrc = (const BYTE *)pPos;
		int iLength;
		UTF32 dwCodePoint;
		if ((pSrc[0] & 0x80) == 0)
			{
			iLength = 1;
			dwCodePoint = pSrc[0];
			}
		else if ((pSrc[0] & 0xe0) == 0xc0)
			{
			iLength = 2;
			dwCodePoint = pSrc[0] & 0x1f;
			}
		else if ((pSrc[0] & 0xf0) == 0xe0)
			{
			iLength = 3;
			dwCodePoint = pSrc[0] & 0x0f;
			}
		else if ((pSrc[0] & 0xf8) == 0xf0)
			{
			iLength = 4;
			dwCodePoint = pSrc[0] & 0x07;
			}
		else
			return false;

		if (pPos + iLength > pEndPos)
			return false;

		for (int i = 1; i < iLength; i++)
			{
			if ((pSrc[i] & 0xc0) != 0x80)
				return false;

			dwCodePoint = (dwCodePoint << 6) | (pSrc[i] & 0x3f);
			}

		if ((iLength == 2 && dwCodePoint < 0x80)
				|| (iLength == 3 && dwCodePoint < 0x800)
				|| (iLength == 4 && dwCodePoint < 0x10000)
				|| (dwCodePoint >= 0xd800 && dwCodePoint <= 0xdfff)
				|| dwCodePoint > 0x10ffff)
			return false;

		pPos += iLength;
		if (retdwCodePoint) *retdwCodePoint = dwCodePoint;
		return true;
		}

	bool ValidateUTF8 (const CString &sValue)
		{
		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();
		while (pPos < pEndPos)
			{
			if (!DecodeUTF8CharStrict(pPos, pEndPos, NULL))
				return false;
			}

		return true;
		}

	void WriteCodeUnit16 (CStringBuffer &Output, WORD wValue, bool bBigEndian)
		{
		if (bBigEndian)
			{
			Output.WriteChar((char)(BYTE)(wValue >> 8));
			Output.WriteChar((char)(BYTE)(wValue & 0xff));
			}
		else
			{
			Output.WriteChar((char)(BYTE)(wValue & 0xff));
			Output.WriteChar((char)(BYTE)(wValue >> 8));
			}
		}

	void WriteCodeUnit32 (CStringBuffer &Output, UTF32 dwValue, bool bBigEndian)
		{
		if (bBigEndian)
			{
			Output.WriteChar((char)(BYTE)((dwValue >> 24) & 0xff));
			Output.WriteChar((char)(BYTE)((dwValue >> 16) & 0xff));
			Output.WriteChar((char)(BYTE)((dwValue >> 8) & 0xff));
			Output.WriteChar((char)(BYTE)(dwValue & 0xff));
			}
		else
			{
			Output.WriteChar((char)(BYTE)(dwValue & 0xff));
			Output.WriteChar((char)(BYTE)((dwValue >> 8) & 0xff));
			Output.WriteChar((char)(BYTE)((dwValue >> 16) & 0xff));
			Output.WriteChar((char)(BYTE)((dwValue >> 24) & 0xff));
			}
		}

	WORD ReadCodeUnit16 (const BYTE *pPos, bool bBigEndian)
		{
		return (bBigEndian ? ((WORD)pPos[0] << 8) | pPos[1] : ((WORD)pPos[1] << 8) | pPos[0]);
		}

	UTF32 ReadCodeUnit32 (const BYTE *pPos, bool bBigEndian)
		{
		if (bBigEndian)
			return ((UTF32)pPos[0] << 24) | ((UTF32)pPos[1] << 16) | ((UTF32)pPos[2] << 8) | pPos[3];
		else
			return ((UTF32)pPos[3] << 24) | ((UTF32)pPos[2] << 16) | ((UTF32)pPos[1] << 8) | pPos[0];
		}

	bool IsValidCodePoint (UTF32 dwCodePoint)
		{
		return (dwCodePoint <= 0x10ffff && !(dwCodePoint >= 0xd800 && dwCodePoint <= 0xdfff));
		}

	int HexValue (char chChar)
		{
		if (chChar >= '0' && chChar <= '9')
			return chChar - '0';
		else if (chChar >= 'a' && chChar <= 'f')
			return chChar - 'a' + 10;
		else if (chChar >= 'A' && chChar <= 'F')
			return chChar - 'A' + 10;
		else
			return -1;
		}

	bool ReadHexFixed (const char *&pPos, const char *pEndPos, int iCount, UTF32 *retdwValue)
		{
		if (pPos + iCount > pEndPos)
			return false;

		UTF32 dwValue = 0;
		for (int i = 0; i < iCount; i++)
			{
			int iDigit = HexValue(*pPos++);
			if (iDigit == -1)
				return false;

			dwValue = (dwValue << 4) | (UTF32)iDigit;
			}

		if (retdwValue) *retdwValue = dwValue;
		return true;
		}

	void WritePercentEncodedByte (CStringBuffer &Output, BYTE byValue)
		{
		Output.WriteChar('%');
		Output.WriteChar(strEncodeHexDigit(byValue / 16));
		Output.WriteChar(strEncodeHexDigit(byValue % 16));
		}

	bool IsURLUnreserved (char chChar)
		{
		return ((chChar >= 'a' && chChar <= 'z')
				|| (chChar >= 'A' && chChar <= 'Z')
				|| (chChar >= '0' && chChar <= '9')
				|| chChar == '-'
				|| chChar == '.'
				|| chChar == '_'
				|| chChar == '~');
		}

	CString EncodeURLComponent (const CString &sValue, bool bSpaceAsPlus)
		{
		CStringBuffer Output;

		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();
		const char *pStart = pPos;
		while (pPos < pEndPos)
			{
			if (IsURLUnreserved(*pPos))
				pPos++;
			else if (*pPos == ' ' && bSpaceAsPlus)
				{
				Output.Write(pStart, (int)(pPos - pStart));
				Output.WriteChar('+');
				pPos++;
				pStart = pPos;
				}
			else
				{
				Output.Write(pStart, (int)(pPos - pStart));
				WritePercentEncodedByte(Output, (BYTE)*pPos);
				pPos++;
				pStart = pPos;
				}
			}

		Output.Write(pStart, (int)(pPos - pStart));
		return CString::CreateFromHandoff(Output);
		}

	bool DecodeURLComponent (const CString &sValue, bool bPlusAsSpace, CString *retsResult, CString *retsError)
		{
		CStringBuffer Output;

		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();
		const char *pStart = pPos;
		while (pPos < pEndPos)
			{
			if (*pPos == '+' && bPlusAsSpace)
				{
				Output.Write(pStart, (int)(pPos - pStart));
				Output.WriteChar(' ');
				pPos++;
				pStart = pPos;
				}
			else if (*pPos == '%')
				{
				Output.Write(pStart, (int)(pPos - pStart));
				pPos++;

				if (pPos + 2 > pEndPos)
					{
					if (retsError) *retsError = strPattern("Invalid percent escape: %s", sValue);
					return false;
					}

				int iH = HexValue(pPos[0]);
				int iL = HexValue(pPos[1]);
				if (iH == -1 || iL == -1)
					{
					if (retsError) *retsError = strPattern("Invalid percent escape: %s", sValue);
					return false;
					}

				Output.WriteChar((char)(BYTE)((iH << 4) | iL));
				pPos += 2;
				pStart = pPos;
				}
			else
				pPos++;
			}

		Output.Write(pStart, (int)(pPos - pStart));

		CString sResult = CString::CreateFromHandoff(Output);
		if (!ValidateUTF8(sResult))
			{
			if (retsError) *retsError = strPattern("Invalid UTF-8 after URL decoding: %s", sValue);
			return false;
			}

		if (retsResult) *retsResult = std::move(sResult);
		return true;
		}

	void WriteJSONChar (CStringBuffer &Output, char chChar)
		{
		switch (chChar)
			{
			case '"': Output.Write("\\\"", 2); break;
			case '\\': Output.Write("\\\\", 2); break;
			case '\b': Output.Write("\\b", 2); break;
			case '\f': Output.Write("\\f", 2); break;
			case '\n': Output.Write("\\n", 2); break;
			case '\r': Output.Write("\\r", 2); break;
			case '\t': Output.Write("\\t", 2); break;
			default:
				if ((BYTE)chChar < 0x20)
					Output.Write(strPattern("\\u%04x", (BYTE)chChar));
				else
					Output.WriteChar(chChar);
				break;
			}
		}

	void WriteCStringChar (CStringBuffer &Output, char chChar)
		{
		switch (chChar)
			{
			case '"': Output.Write("\\\"", 2); break;
			case '\\': Output.Write("\\\\", 2); break;
			case '\a': Output.Write("\\a", 2); break;
			case '\b': Output.Write("\\b", 2); break;
			case '\f': Output.Write("\\f", 2); break;
			case '\n': Output.Write("\\n", 2); break;
			case '\r': Output.Write("\\r", 2); break;
			case '\t': Output.Write("\\t", 2); break;
			case '\v': Output.Write("\\v", 2); break;
			default:
				if ((BYTE)chChar < 0x20 || (BYTE)chChar == 0x7f)
					Output.Write(strPattern("\\x%02x", (BYTE)chChar));
				else
					Output.WriteChar(chChar);
				break;
			}
		}

	CString EncodeQuotedContent (const CString &sValue, bool bJSON)
		{
		CStringBuffer Output;

		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();
		while (pPos < pEndPos)
			{
			if (bJSON)
				WriteJSONChar(Output, *pPos++);
			else
				WriteCStringChar(Output, *pPos++);
			}

		return CString::CreateFromHandoff(Output);
		}

	bool DecodeQuotedContent (const CString &sValue, bool bJSON, bool bLiteral, CString *retsResult, CString *retsError)
		{
		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();

		if (bLiteral)
			{
			if (pPos >= pEndPos || *pPos != '"')
				{
				if (retsError) *retsError = strPattern("Expected quoted string literal: %s", sValue);
				return false;
				}

			pPos++;
			pEndPos--;
			if (pEndPos < pPos || *pEndPos != '"')
				{
				if (retsError) *retsError = strPattern("Expected quoted string literal: %s", sValue);
				return false;
				}
			}

		CStringBuffer Output;
		while (pPos < pEndPos)
			{
			char chChar = *pPos++;
			if (chChar == '"')
				{
				if (retsError) *retsError = strPattern("Unescaped quote in string content: %s", sValue);
				return false;
				}
			else if (bJSON && strIsASCIIControl(&chChar))
				{
				if (retsError) *retsError = strPattern("Invalid control character in JSON string: %s", sValue);
				return false;
				}
			else if (chChar != '\\')
				{
				Output.WriteChar(chChar);
				continue;
				}
			else if (pPos >= pEndPos)
				{
				if (retsError) *retsError = strPattern("Invalid escape sequence: %s", sValue);
				return false;
				}
			else
				{
				char chEsc = *pPos++;
				switch (chEsc)
					{
					case '"': Output.WriteChar('"'); break;
					case '\\': Output.WriteChar('\\'); break;
					case '/': Output.WriteChar('/'); break;
					case 'a':
						if (bJSON) goto InvalidEscape;
						Output.WriteChar('\a');
						break;
					case 'b': Output.WriteChar('\b'); break;
					case 'f': Output.WriteChar('\f'); break;
					case 'n': Output.WriteChar('\n'); break;
					case 'r': Output.WriteChar('\r'); break;
					case 't': Output.WriteChar('\t'); break;
					case 'v':
						if (bJSON) goto InvalidEscape;
						Output.WriteChar('\v');
						break;
					case 'x':
						{
						if (bJSON) goto InvalidEscape;
						UTF32 dwValue;
						if (!ReadHexFixed(pPos, pEndPos, 2, &dwValue))
							goto InvalidEscape;
						Output.WriteChar((char)(BYTE)dwValue);
						break;
						}
					case 'u':
						{
						UTF32 dwValue;
						if (!ReadHexFixed(pPos, pEndPos, 4, &dwValue))
							goto InvalidEscape;

						if (dwValue >= 0xd800 && dwValue <= 0xdbff)
							{
							if (pPos + 6 > pEndPos || pPos[0] != '\\' || pPos[1] != 'u')
								goto InvalidEscape;

							pPos += 2;
							UTF32 dwLow;
							if (!ReadHexFixed(pPos, pEndPos, 4, &dwLow) || dwLow < 0xdc00 || dwLow > 0xdfff)
								goto InvalidEscape;

							dwValue = 0x10000 + (((dwValue - 0xd800) << 10) | (dwLow - 0xdc00));
							}
						else if (dwValue >= 0xdc00 && dwValue <= 0xdfff)
							goto InvalidEscape;

						strEncodeUTF8Char(dwValue, Output);
						break;
						}
					case 'U':
						{
						if (bJSON) goto InvalidEscape;
						UTF32 dwValue;
						if (!ReadHexFixed(pPos, pEndPos, 8, &dwValue) || !IsValidCodePoint(dwValue))
							goto InvalidEscape;
						strEncodeUTF8Char(dwValue, Output);
						break;
						}
					default:
InvalidEscape:
						if (retsError) *retsError = strPattern("Invalid escape sequence: %s", sValue);
						return false;
					}
				}
			}

		if (retsResult) *retsResult = CString::CreateFromHandoff(Output);
		return true;
		}

	bool DecodeHTMLEntity (const char *&pPos, const char *pEndPos, CStringBuffer &Output)
		{
		const char *pStart = pPos;
		const char *pSemi = pPos;
		while (pSemi < pEndPos && *pSemi != ';')
			pSemi++;

		if (pSemi == pEndPos)
			{
			Output.WriteChar('&');
			pPos = pStart;
			return true;
			}

		CString sEntity(pStart, (int)(pSemi - pStart));
		pPos = pSemi + 1;

		if (strEquals(sEntity, STR_STRING_ENCODING_LT))
			Output.WriteChar('<');
		else if (strEquals(sEntity, STR_STRING_ENCODING_GT))
			Output.WriteChar('>');
		else if (strEquals(sEntity, STR_STRING_ENCODING_AMP))
			Output.WriteChar('&');
		else if (strEquals(sEntity, STR_STRING_ENCODING_QUOT))
			Output.WriteChar('"');
		else if (strEquals(sEntity, STR_STRING_ENCODING_APOS))
			Output.WriteChar('\'');
		else if (sEntity.GetLength() > 1 && *sEntity.GetParsePointer() == '#')
			{
			const char *pNum = sEntity.GetParsePointer() + 1;
			int iBase = 10;
			if (pNum < sEntity.GetParsePointer() + sEntity.GetLength() && (*pNum == 'x' || *pNum == 'X'))
				{
				pNum++;
				iBase = 16;
				}

			bool bFailed;
			int iValue = strParseIntOfBase(pNum, iBase, 0, NULL, &bFailed);
			if (bFailed || !IsValidCodePoint((UTF32)iValue))
				return false;

			strEncodeUTF8Char((UTF32)iValue, Output);
			}
		else
			{
			Output.WriteChar('&');
			Output.Write(sEntity);
			Output.WriteChar(';');
			}

		return true;
		}

	bool DecodeHTML (const CString &sValue, CString *retsResult, CString *retsError)
		{
		CStringBuffer Output;
		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();
		const char *pStart = pPos;

		while (pPos < pEndPos)
			{
			if (*pPos == '&')
				{
				Output.Write(pStart, (int)(pPos - pStart));
				pPos++;
				if (!DecodeHTMLEntity(pPos, pEndPos, Output))
					{
					if (retsError) *retsError = strPattern("Invalid HTML character reference: %s", sValue);
					return false;
					}
				pStart = pPos;
				}
			else
				pPos++;
			}

		Output.Write(pStart, (int)(pPos - pStart));
		if (retsResult) *retsResult = CString::CreateFromHandoff(Output);
		return true;
		}

	bool ValidateBase64 (const CString &sValue, bool bURL, CString *retsError)
		{
		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();
		bool bPadding = false;

		while (pPos < pEndPos)
			{
			char chChar = *pPos++;
			bool bValid = ((chChar >= 'A' && chChar <= 'Z')
					|| (chChar >= 'a' && chChar <= 'z')
					|| (chChar >= '0' && chChar <= '9')
					|| (bURL ? (chChar == '-' || chChar == '_') : (chChar == '+' || chChar == '/')));

			if (chChar == '=')
				bPadding = true;
			else if (bPadding || !bValid)
				{
				if (retsError) *retsError = strPattern("Invalid Base64 string: %s", sValue);
				return false;
				}
			}

		return true;
		}

	bool DecodeBase64ToString (const CString &sValue, bool bURL, CString *retsResult, CString *retsError)
		{
		if (!ValidateBase64(sValue, bURL, retsError))
			return false;

		CString sInput = sValue;
		int iRemainder = sInput.GetLength() % 4;
		if (iRemainder != 0)
			{
			if (!bURL || iRemainder == 1)
				{
				if (retsError) *retsError = strPattern("Invalid Base64 string: %s", sValue);
				return false;
				}

			CStringBuffer Padded;
			Padded.Write(sInput);
			for (int i = iRemainder; i < 4; i++)
				Padded.WriteChar('=');
			sInput = CString::CreateFromHandoff(Padded);
			}

		try
			{
			CBuffer Input(sInput);
			CBase64Decoder Decoder(&Input);

			CStringBuffer Output;
			Output.Write(Decoder, Decoder.GetStreamLength());

			CString sResult = CString::CreateFromHandoff(Output);
			if (!ValidateUTF8(sResult))
				{
				if (retsError) *retsError = strPattern("Invalid UTF-8 after Base64 decoding: %s", sValue);
				return false;
				}

			if (retsResult) *retsResult = std::move(sResult);
			return true;
			}
		catch (...)
			{
			if (retsError) *retsError = strPattern("Invalid Base64 string: %s", sValue);
			return false;
			}
		}

	bool DecodeHexToString (const CString &sValue, CString *retsResult, CString *retsError)
		{
		if ((sValue.GetLength() % 2) != 0)
			{
			if (retsError) *retsError = strPattern("Invalid hex string: %s", sValue);
			return false;
			}

		const char *pPos = sValue.GetParsePointer();
		const char *pEndPos = pPos + sValue.GetLength();
		while (pPos < pEndPos)
			{
			if (HexValue(*pPos++) == -1)
				{
				if (retsError) *retsError = strPattern("Invalid hex string: %s", sValue);
				return false;
				}
			}

		try
			{
			CBuffer Input(sValue);
			CHexDecoder Decoder(Input);

			CStringBuffer Output;
			Output.Write(Decoder, Decoder.GetStreamLength());

			CString sResult = CString::CreateFromHandoff(Output);
			if (!ValidateUTF8(sResult))
				{
				if (retsError) *retsError = strPattern("Invalid UTF-8 after hex decoding: %s", sValue);
				return false;
				}

			if (retsResult) *retsResult = std::move(sResult);
			return true;
			}
		catch (...)
			{
			if (retsError) *retsError = strPattern("Invalid hex string: %s", sValue);
			return false;
			}
		}
	}

bool strDecodeFrom (const CString &sValue, EStringEncodingType iEncoding, CString *retsResult, CString *retsError)
	{
	switch (iEncoding)
		{
		case EStringEncodingType::urlComponent:
		case EStringEncodingType::urlPathSegment:
		case EStringEncodingType::urlQueryValue:
			return DecodeURLComponent(sValue, false, retsResult, retsError);

		case EStringEncodingType::formComponent:
			return DecodeURLComponent(sValue, true, retsResult, retsError);

		case EStringEncodingType::jsonStringLiteral:
			return DecodeQuotedContent(sValue, true, true, retsResult, retsError);

		case EStringEncodingType::jsonStringContent:
			return DecodeQuotedContent(sValue, true, false, retsResult, retsError);

		case EStringEncodingType::cStringLiteral:
			return DecodeQuotedContent(sValue, false, true, retsResult, retsError);

		case EStringEncodingType::cStringContent:
			return DecodeQuotedContent(sValue, false, false, retsResult, retsError);

		case EStringEncodingType::htmlText:
		case EStringEncodingType::htmlAttribute:
			return DecodeHTML(sValue, retsResult, retsError);

		case EStringEncodingType::base64:
			return DecodeBase64ToString(sValue, false, retsResult, retsError);

		case EStringEncodingType::base64URL:
			return DecodeBase64ToString(sValue, true, retsResult, retsError);

		case EStringEncodingType::hex:
			return DecodeHexToString(sValue, retsResult, retsError);

		default:
			if (retsError) *retsError = STR_STRING_ENCODING_UNKNOWN_ENCODING_TYPE;
			return false;
		}
	}

bool strDecodeToString (const void *pData, int iLength, ECharSetType iCharSet, CString *retsResult, CString *retsError)
	{
	if (iLength < 0)
		{
		if (retsError) *retsError = STR_STRING_ENCODING_INVALID_BINARY_LENGTH;
		return false;
		}

	const BYTE *pPos = (const BYTE *)pData;
	const BYTE *pEndPos = pPos + iLength;
	CStringBuffer Output;

	switch (iCharSet)
		{
		case ECharSetType::UTF8:
			{
			CString sResult((const char *)pData, iLength);
			if (!ValidateUTF8(sResult))
				{
				if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_8_BYTE_SEQUENCE;
				return false;
				}

			if (retsResult) *retsResult = std::move(sResult);
			return true;
			}

		case ECharSetType::UTF16BE:
		case ECharSetType::UTF16LE:
			{
			if ((iLength % 2) != 0)
				{
				if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_16_BYTE_LENGTH;
				return false;
				}

			bool bBigEndian = (iCharSet == ECharSetType::UTF16BE);
			while (pPos < pEndPos)
				{
				WORD wValue = ReadCodeUnit16(pPos, bBigEndian);
				pPos += 2;

				if (wValue >= 0xd800 && wValue <= 0xdbff)
					{
					if (pPos >= pEndPos)
						{
						if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_16_SURROGATE_PAIR;
						return false;
						}

					WORD wLow = ReadCodeUnit16(pPos, bBigEndian);
					pPos += 2;
					if (wLow < 0xdc00 || wLow > 0xdfff)
						{
						if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_16_SURROGATE_PAIR;
						return false;
						}

					UTF32 dwCodePoint = 0x10000 + (((UTF32)(wValue - 0xd800) << 10) | (wLow - 0xdc00));
					strEncodeUTF8Char(dwCodePoint, Output);
					}
				else if (wValue >= 0xdc00 && wValue <= 0xdfff)
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_16_SURROGATE_PAIR;
					return false;
					}
				else
					strEncodeUTF8Char(wValue, Output);
				}

			if (retsResult) *retsResult = CString::CreateFromHandoff(Output);
			return true;
			}

		case ECharSetType::UTF32BE:
		case ECharSetType::UTF32LE:
			{
			if ((iLength % 4) != 0)
				{
				if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_32_BYTE_LENGTH;
				return false;
				}

			bool bBigEndian = (iCharSet == ECharSetType::UTF32BE);
			while (pPos < pEndPos)
				{
				UTF32 dwCodePoint = ReadCodeUnit32(pPos, bBigEndian);
				pPos += 4;

				if (!IsValidCodePoint(dwCodePoint))
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_32_CODE_POINT;
					return false;
					}

				strEncodeUTF8Char(dwCodePoint, Output);
				}

			if (retsResult) *retsResult = CString::CreateFromHandoff(Output);
			return true;
			}

		case ECharSetType::ASCII:
			while (pPos < pEndPos)
				{
				if (*pPos > 0x7f)
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_ASCII_BYTE_SEQUENCE;
					return false;
					}

				Output.WriteChar((char)*pPos++);
				}

			if (retsResult) *retsResult = CString::CreateFromHandoff(Output);
			return true;

		case ECharSetType::Latin1:
			while (pPos < pEndPos)
				strEncodeUTF8Char((UTF32)*pPos++, Output);

			if (retsResult) *retsResult = CString::CreateFromHandoff(Output);
			return true;

		case ECharSetType::Windows1252:
			while (pPos < pEndPos)
				strEncodeUTF8Char(DecodeWindows1252(*pPos++), Output);

			if (retsResult) *retsResult = CString::CreateFromHandoff(Output);
			return true;

		default:
			if (retsError) *retsError = STR_STRING_ENCODING_UNKNOWN_CHARACTER_SET;
			return false;
		}
	}

bool strEncodeAs (const CString &sValue, EStringEncodingType iEncoding, CString *retsResult, CString *retsError)
	{
	switch (iEncoding)
		{
		case EStringEncodingType::urlComponent:
		case EStringEncodingType::urlPathSegment:
		case EStringEncodingType::urlQueryValue:
			if (retsResult) *retsResult = EncodeURLComponent(sValue, false);
			return true;

		case EStringEncodingType::formComponent:
			if (retsResult) *retsResult = EncodeURLComponent(sValue, true);
			return true;

		case EStringEncodingType::jsonStringLiteral:
			if (retsResult) *retsResult = strPattern("\"%s\"", EncodeQuotedContent(sValue, true));
			return true;

		case EStringEncodingType::jsonStringContent:
			if (retsResult) *retsResult = EncodeQuotedContent(sValue, true);
			return true;

		case EStringEncodingType::cStringLiteral:
			if (retsResult) *retsResult = strPattern("\"%s\"", EncodeQuotedContent(sValue, false));
			return true;

		case EStringEncodingType::cStringContent:
			if (retsResult) *retsResult = EncodeQuotedContent(sValue, false);
			return true;

		case EStringEncodingType::htmlText:
			if (retsResult) *retsResult = htmlWriteText(sValue);
			return true;

		case EStringEncodingType::htmlAttribute:
			if (retsResult) *retsResult = htmlWriteAttributeValue(sValue);
			return true;

		case EStringEncodingType::base64:
			if (retsResult) *retsResult = CString::EncodeBase64((const BYTE *)sValue.GetParsePointer(), sValue.GetLength());
			return true;

		case EStringEncodingType::base64URL:
			if (retsResult) *retsResult = CString::EncodeBase64((const BYTE *)sValue.GetParsePointer(), sValue.GetLength(), CBase64Encoder::FLAG_BASE64_URL);
			return true;

		case EStringEncodingType::hex:
			if (retsResult) *retsResult = CString::EncodeHex((const BYTE *)sValue.GetParsePointer(), sValue.GetLength());
			return true;

		default:
			if (retsError) *retsError = STR_STRING_ENCODING_UNKNOWN_ENCODING_TYPE;
			return false;
		}
	}

bool strEncodeToBinary (const CString &sValue, ECharSetType iCharSet, CStringBuffer *retBuffer, CString *retsError)
	{
	if (retBuffer == NULL)
		return false;

	const char *pPos = sValue.GetParsePointer();
	const char *pEndPos = pPos + sValue.GetLength();

	switch (iCharSet)
		{
		case ECharSetType::UTF8:
			if (!ValidateUTF8(sValue))
				{
				if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_8_STRING;
				return false;
				}

			retBuffer->Write(sValue);
			return true;

		case ECharSetType::UTF16BE:
		case ECharSetType::UTF16LE:
			{
			bool bBigEndian = (iCharSet == ECharSetType::UTF16BE);
			while (pPos < pEndPos)
				{
				UTF32 dwCodePoint;
				if (!DecodeUTF8CharStrict(pPos, pEndPos, &dwCodePoint))
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_8_STRING;
					return false;
					}

				if (dwCodePoint <= 0xffff)
					WriteCodeUnit16(*retBuffer, (WORD)dwCodePoint, bBigEndian);
				else
					{
					dwCodePoint -= 0x10000;
					WriteCodeUnit16(*retBuffer, (WORD)(0xd800 | (dwCodePoint >> 10)), bBigEndian);
					WriteCodeUnit16(*retBuffer, (WORD)(0xdc00 | (dwCodePoint & 0x03ff)), bBigEndian);
					}
				}

			return true;
			}

		case ECharSetType::UTF32BE:
		case ECharSetType::UTF32LE:
			{
			bool bBigEndian = (iCharSet == ECharSetType::UTF32BE);
			while (pPos < pEndPos)
				{
				UTF32 dwCodePoint;
				if (!DecodeUTF8CharStrict(pPos, pEndPos, &dwCodePoint))
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_8_STRING;
					return false;
					}

				WriteCodeUnit32(*retBuffer, dwCodePoint, bBigEndian);
				}

			return true;
			}

		case ECharSetType::ASCII:
			while (pPos < pEndPos)
				{
				UTF32 dwCodePoint;
				if (!DecodeUTF8CharStrict(pPos, pEndPos, &dwCodePoint))
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_8_STRING;
					return false;
					}
				else if (dwCodePoint > 0x7f)
					{
					if (retsError) *retsError = STR_STRING_ENCODING_CHARACTER_CANNOT_BE_ENCODED_AS_ASCII;
					return false;
					}

				retBuffer->WriteChar((char)(BYTE)dwCodePoint);
				}

			return true;

		case ECharSetType::Latin1:
			while (pPos < pEndPos)
				{
				UTF32 dwCodePoint;
				if (!DecodeUTF8CharStrict(pPos, pEndPos, &dwCodePoint))
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_8_STRING;
					return false;
					}
				else if (dwCodePoint > 0xff)
					{
					if (retsError) *retsError = STR_STRING_ENCODING_CHARACTER_CANNOT_BE_ENCODED_AS_LATIN1;
					return false;
					}

				retBuffer->WriteChar((char)(BYTE)dwCodePoint);
				}

			return true;

		case ECharSetType::Windows1252:
			while (pPos < pEndPos)
				{
				UTF32 dwCodePoint;
				if (!DecodeUTF8CharStrict(pPos, pEndPos, &dwCodePoint))
					{
					if (retsError) *retsError = STR_STRING_ENCODING_INVALID_UTF_8_STRING;
					return false;
					}

				BYTE byValue;
				if (!EncodeWindows1252(dwCodePoint, &byValue))
					{
					if (retsError) *retsError = STR_STRING_ENCODING_CHARACTER_CANNOT_BE_ENCODED_AS_WINDOWS_1252;
					return false;
					}

				retBuffer->WriteChar((char)byValue);
				}

			return true;

		default:
			if (retsError) *retsError = STR_STRING_ENCODING_UNKNOWN_CHARACTER_SET;
			return false;
		}
	}

bool strParseCharSetType (CStringView sValue, ECharSetType *retiType)
	{
	ECharSetType iType;
	if (strEqualsNoCase(sValue, STR_STRING_ENCODING_UTF8))
		iType = ECharSetType::UTF8;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_UTF16_BE))
		iType = ECharSetType::UTF16BE;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_UTF16_LE))
		iType = ECharSetType::UTF16LE;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_UTF32_BE))
		iType = ECharSetType::UTF32BE;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_UTF32_LE))
		iType = ECharSetType::UTF32LE;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_ASCII))
		iType = ECharSetType::ASCII;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_LATIN1))
		iType = ECharSetType::Latin1;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_WINDOWS_1252))
		iType = ECharSetType::Windows1252;
	else
		return false;

	if (retiType) *retiType = iType;
	return true;
	}

bool strParseStringEncodingType (CStringView sValue, EStringEncodingType *retiType)
	{
	EStringEncodingType iType;
	if (strEqualsNoCase(sValue, STR_STRING_ENCODING_URL_COMPONENT))
		iType = EStringEncodingType::urlComponent;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_URL_PATH_SEGMENT))
		iType = EStringEncodingType::urlPathSegment;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_URL_QUERY_VALUE))
		iType = EStringEncodingType::urlQueryValue;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_FORM_COMPONENT))
		iType = EStringEncodingType::formComponent;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_JSON_STRING_LITERAL))
		iType = EStringEncodingType::jsonStringLiteral;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_JSON_STRING_CONTENT))
		iType = EStringEncodingType::jsonStringContent;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_C_STRING_LITERAL))
		iType = EStringEncodingType::cStringLiteral;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_C_STRING_CONTENT))
		iType = EStringEncodingType::cStringContent;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_HTML_TEXT))
		iType = EStringEncodingType::htmlText;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_HTML_ATTRIBUTE))
		iType = EStringEncodingType::htmlAttribute;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_BASE64))
		iType = EStringEncodingType::base64;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_BASE64_URL))
		iType = EStringEncodingType::base64URL;
	else if (strEqualsNoCase(sValue, STR_STRING_ENCODING_HEX))
		iType = EStringEncodingType::hex;
	else
		return false;

	if (retiType) *retiType = iType;
	return true;
	}
