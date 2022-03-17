//	CString.cpp
//
//	CString class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	A CString is either NULL or a pointer to an allocated block
//	of the following form:
//
//					int		Length (in bytes)
//	m_pString ->	char	First char of string
//					char	Second char of string
//					char	...
//					char	Last char of string
//					char	'\0' (always NULL-terminated)
//
//	Length does NOT include the null termination.
//
//	If Length is negative then it means that m_pString is a 
//	literal and should not be freed.
//
//	NOTES
//
//	*	Max size of a string is 2^31 - 1.
//
//	*	All string methods support embedded NULLs.
//
//	*	Excepted as noted, string methods do not require
//		UTF-8 encoding.

#include "stdafx.h"
#include <utility>

static const char *PRIVATE_CONS = "";
const DWORD STR_FLAG_LITERAL =		0x00000001;

const CString NULL_STR;

DECLARE_CONST_STRING(STR_NAN,							"NaN")

static char MIN_INTEGER_DIGITS[] = "2147483648";
static char MAX_INTEGER_DIGITS[] = "2147483647";

char CString::BLANK_STRING[] = "\0\0\0\0\0";

CString::CString (LPCSTR pStr)

//	CString constructor
//
//	pStr must be a NULL terminated sequence of bytes.

	{
	Init(pStr, strLength(pStr));
	}

CString::CString (LPCSTR pStr, int iLen)

//	CString constructor
//
//	pStr must be a sequence of bytes at least iLen long
//	If pStr is NULL and iLen > 0 then we allocate an uninitialized string

	{
	ASSERT(iLen >= 0);
	Init(pStr, iLen);
	}

CString::CString (LPCSTR pStr, DWORD iLen)

//	CString constructor
//
//	pStr must be a sequence of bytes at least iLen long
//	If pStr is NULL and iLen > 0 then we allocate an uninitialized string

	{
	ASSERT(iLen >= 0);
	ASSERT(iLen < MAXINT32);
	Init(pStr, (int)iLen);
	}

CString::CString (LPCSTR pStr, size_t iLen)

//	CString constructor
//
//	pStr must be a sequence of bytes at least iLen long
//	If pStr is NULL and iLen > 0 then we allocate an uninitialized string

	{
	ASSERT(iLen >= 0);
	ASSERT(iLen < MAXINT32);
	Init(pStr, (int)iLen);
	}

CString::CString (LPCSTR pStr, std::ptrdiff_t iLen)

//	CString constructor
//
//	pStr must be a sequence of bytes at least iLen long
//	If pStr is NULL and iLen > 0 then we allocate an uninitialized string

	{
	ASSERT(iLen >= 0);
	ASSERT(iLen < MAXINT32);
	Init(pStr, (int)iLen);
	}

CString::CString (int iLen)

//	CString constructor

	{
	ASSERT(iLen >= 0);
	Init(NULL, iLen);
	}

CString::CString (DWORD iLen)

//	CString constructor

	{
	ASSERT(iLen >= 0);
	ASSERT(iLen < MAXINT32);
	Init(NULL, (int)iLen);
	}

CString::CString (size_t iLen)

//	CString constructor

	{
	ASSERT(iLen >= 0);
	ASSERT(iLen < MAXINT32);
	Init(NULL, (int)iLen);
	}

CString::CString (std::ptrdiff_t iLen)

//	CString constructor

	{
	ASSERT(iLen >= 0);
	ASSERT(iLen < MAXINT32);
	Init(NULL, (int)iLen);
	}

CString::CString (const CString &sStr)

//	CString constructor

	{
	if (sStr.IsLiteral())
		m_pString = sStr.m_pString;
	else
		Init(sStr, sStr.GetLength());
	}

CString::CString (CStringBuffer &&Src) noexcept

//	CString move constructor

	{
	m_pString = Src.Handoff();
	}

CString::CString (LPSTR pStr, int iLen, bool bLiteral)

//	CString constructor

	{
	if (bLiteral)
		{
		m_pString = (LPSTR)pStr;
		ASSERT(IsLiteral());
		}
	else
		Init((LPSTR)pStr, iLen);
	}

CString::CString (LPSTR pStr, DWORD iLen, bool bLiteral)

//	CString constructor

	{
	ASSERT(iLen < MAXINT32);

	if (bLiteral)
		{
		m_pString = (LPSTR)pStr;
		ASSERT(IsLiteral());
		}
	else
		Init((LPSTR)pStr, (int)iLen);
	}

CString::CString (LPSTR pStr, size_t iLen, bool bLiteral)

//	CString constructor

	{
	ASSERT(iLen < MAXINT32);

	if (bLiteral)
		{
		m_pString = (LPSTR)pStr;
		ASSERT(IsLiteral());
		}
	else
		Init((LPSTR)pStr, (int)iLen);
	}

CString::CString (LPSTR pStr, std::ptrdiff_t iLen, bool bLiteral)

//	CString constructor

	{
	ASSERT(iLen < MAXINT32);

	if (bLiteral)
		{
		m_pString = (LPSTR)pStr;
		ASSERT(IsLiteral());
		}
	else
		Init((LPSTR)pStr, (int)iLen);
	}

CString::CString (LPTSTR pStr, int iLen)

//	CString constructor

	{
	m_pString = CreateBufferFromUTF16(pStr, iLen);
	}

CString::CString (LPTSTR pStr, DWORD iLen)

//	CString constructor

	{
	ASSERT(iLen < MAXINT32);
	m_pString = CreateBufferFromUTF16(pStr, (int)iLen);
	}

CString::CString (LPTSTR pStr, size_t iLen)

//	CString constructor

	{
	ASSERT(iLen < MAXINT32);
	m_pString = CreateBufferFromUTF16(pStr, (int)iLen);
	}

CString::CString (LPTSTR pStr, std::ptrdiff_t iLen)

//	CString constructor

	{
	ASSERT(iLen < MAXINT32);
	m_pString = CreateBufferFromUTF16(pStr, (int)iLen);
	}

CString::CString (const CString16 &sStr)

//	CString constructor

	{
	m_pString = CreateBufferFromUTF16(sStr, sStr.GetLength());
	}

CString::~CString (void)

//	CString destructor

	{
	if (!IsLiteral())
		delete [] GetBuffer();
	}

CString &CString::operator= (LPSTR pStr)

//	CString operator =

	{
	CleanUp();
	Init(pStr, strLength(pStr));
	return *this;
	}

CString &CString::operator= (const CString &sStr)

//	CString operator =

	{
	if (sStr.IsLiteral())
		{
		CleanUp();
		m_pString = sStr.m_pString;
		}
	else
		{
		//	First copy the buffer. We do this because if we assign a string
		//	to ourselves we might end up freeing our string before we
		//	get a chance to copy it.

		LPSTR pString = sStr.CopyBuffer();

		//	Clean up

		CleanUp();

		//	Done

		m_pString = pString;
		}

	return *this;
	}

CString &CString::operator+= (const CString &sStr)

//	CString operator +=

	{
	int iStrLen1 = GetLength();
	int iStrLen2 = sStr.GetLength();

	//	Edge conditions

	int iFinalLen = iStrLen1 + iStrLen2;
	if (iStrLen2 == 0)
		return *this;
	else if (iStrLen1 == 0)
		{
		LPSTR pString = sStr.CopyBuffer();
		CleanUp();
		m_pString = pString;
		return *this;
		}

	//	Create a buffer large enough to hold both string

	LPSTR pBuffer = new char[sizeof(int) + iFinalLen + 1];
	*(int *)pBuffer = iFinalLen;

	//	Copy the original string

	utlMemCopy(m_pString, pBuffer + sizeof(int), iStrLen1);

	//	Copy the string being appended

	utlMemCopy(sStr.m_pString, pBuffer + sizeof(int) + iStrLen1, iStrLen2);
	
	//	Clean up

	CleanUp();

	//	Assign

	m_pString = pBuffer + sizeof(int);
	m_pString[iFinalLen] = '\0';

	return *this;
	}

CString CString::operator + (const CString &sStr) const

//	CString operator +

	{
	int iStrLen1 = GetLength();
	int iStrLen2 = sStr.GetLength();

	//	Edge conditions

	int iFinalLen = iStrLen1 + iStrLen2;
	if (iStrLen2 == 0)
		return *this;
	else if (iStrLen1 == 0)
		return sStr;

	//	Create a buffer large enough to hold both string

	LPSTR pBuffer = new char[sizeof(int) + iFinalLen + 1];
	*(int *)pBuffer = iFinalLen;

	//	Copy the original string

	utlMemCopy(m_pString, pBuffer + sizeof(int), iStrLen1);

	//	Copy the string being appended

	utlMemCopy(sStr.m_pString, pBuffer + sizeof(int) + iStrLen1, iStrLen2 + 1);

	//	Done (this private constructor just takes the string buffer that we
	//	allocated without making a copy).

	return CString(pBuffer + sizeof(int), PRIVATE_CONS);
	}

void CString::CleanUp (void)

//	CleanUp
//
//	Free the string

	{
	if (!IsLiteral())
		{
		delete [] GetBuffer();
		m_pString = NULL;
		}
	}

LPSTR CString::CopyBuffer (void) const

//	CopyBuffer
//
//	Returns a copy of m_pString for this string

	{
	if (!IsLiteral())
		{
		int iBufferLen = GetBufferLength();
		LPSTR pNewBuffer = new char [iBufferLen];
		utlMemCopy(GetBuffer(), pNewBuffer, iBufferLen);
		return pNewBuffer + sizeof(int);
		}
	else
		return m_pString;
	}

LPSTR CString::CreateBufferFromUTF16 (LPTSTR pStr, int iLen)

//	CreateBufferFromUTF16
//
//	Creates a buffer that has been converted from UTF-16.

	{
	if (iLen == -1)
		iLen = CString16::CalcLength(pStr);

	//	Handle NULL input

	if (iLen == 0)
		return NULL;

	//	We optimistically assume that the UTF-8 buffer
	//	will be half the size of the UTF-16 buffer.

	int iNewBufferLen = iLen;
	LPSTR pNewBuffer = new char [sizeof(int) + iNewBufferLen + 1];
	int iResult = ::WideCharToMultiByte(CP_UTF8, 0, pStr, iLen, pNewBuffer + sizeof(int), iNewBufferLen, NULL, NULL);

	//	Deal with failure

	if (iResult == 0)
		{
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
			delete [] pNewBuffer;

			//	Figure out how big the buffer should be and allocate appropriately
			//	And redo the conversion.

			iNewBufferLen = ::WideCharToMultiByte(CP_UTF8, 0, pStr, iLen, NULL, 0, NULL, NULL);
			if (iNewBufferLen == 0)
				return NULL;

			pNewBuffer = new char [sizeof(int) + iNewBufferLen + 1];
			iResult = ::WideCharToMultiByte(CP_UTF8, 0, pStr, iLen, pNewBuffer + sizeof(int), iNewBufferLen, NULL, NULL);
			}
		else
			{
			delete [] pNewBuffer;
			return NULL;
			}
		}

	//	Store the length

	*(int *)(pNewBuffer) = iResult;
	pNewBuffer += sizeof(int);

	//	Null-terminate the end

	ASSERT(iResult <= iNewBufferLen);
	pNewBuffer[iResult] = _T('\0');

	//	Done

	return pNewBuffer;
	}

CString CString::CreateFromHandoff (CStringBuffer &Buffer)

//	CreateFromHandoff
//
//	Creates a string from a CStringBuffer by handing off the buffer

	{
	return CString(Buffer.Handoff(), PRIVATE_CONS);
	}

CString CString::Deserialize (IByteStream &Stream)

//	Deserialize
//
//	Deserialize a binary format

	{
	//	Read the length

	DWORD dwLength;
	if (Stream.Read(&dwLength, sizeof(DWORD)) != sizeof(DWORD))
		return NULL_STR;

	//	Read the string

	if (dwLength > 0 && dwLength < 0x80000000)
		{
		CString sStr((int)dwLength);
		int iActualLen = Stream.Read(sStr.m_pString, dwLength);

		if (iActualLen < (int)dwLength)
			utlMemSet(sStr.m_pString + iActualLen, (int)(dwLength - iActualLen), '?');

		int iPad = AlignUp((int)dwLength, (int)sizeof(DWORD)) - dwLength;
		if (iPad)
			{
			DWORD dwPad;
			Stream.Read(&dwPad, iPad);
			}

		return CString(sStr.Handoff(), PRIVATE_CONS);
		}
	else
		return NULL_STR;
	}

CString CString::DeserializeJSON (IByteStream &Stream)

//	DeserializeJSON
//
//	Reads a JSON string (up to the next double-quote or the end of the buffer).

	{
	CStringBuffer Result;

	int iPos = Stream.GetPos();
	int iMaxPos = Stream.GetStreamLength();
	bool bEscape = false;

	while (iPos < iMaxPos)
		{
		char chChar;
		Stream.Read(&chChar, 1);
		iPos++;

		if (bEscape)
			{
			switch (chChar)
				{
				case '\"':
					Result.Write("\"", 1);
					break;

				case '\\':
					Result.Write("\\", 1);
					break;

				case '/':
					Result.Write("/", 1);
					break;

				case 'b':
					Result.Write("\b", 1);
					break;

				case 'f':
					Result.Write("\f", 1);
					break;

				case 'n':
					Result.Write("\n", 1);
					break;

				case 'r':
					Result.Write("\r", 1);
					break;

				case 't':
					Result.Write("\t", 1);
					break;

				case 'u':
					{
					if (iPos + 4 > iMaxPos)
						return CString::CreateFromHandoff(Result);

					char szBuffer[5];
					Stream.Read(&szBuffer[0], 4);
					iPos += 4;
					szBuffer[4] = '\0';

					DWORD dwHex = (DWORD)strParseIntOfBase(szBuffer, 16, (int)'?');
					CString sChar = strEncodeUTF8Char((UTF32)dwHex);
					Result.Write(sChar);
					break;
					}

				default:
					return CString::CreateFromHandoff(Result);
				}

			bEscape = false;
			}
		else if (chChar == '\"')
			break;
		else if (chChar == '\\')
			bEscape = true;
		else
			Result.Write(&chChar, 1);
		}

	//	Done

	return CString::CreateFromHandoff(Result);
	}

int CString::GetLength (void) const

//	GetLength
//
//	Returns the byte length of the string (excluding the NULL terminator)

	{
	if (m_pString)
		return Abs(GetLengthParameter());
	else
		return 0;
	}

CString CString::IncrementTrailingNumber (const CString &sStr)

//	IncrementTrailingNumber
//
//	If a string has a trailing integer, we increment it. Otherwise, we add the
//	integer 2.

	{
	if (sStr.IsEmpty())
		return NULL_STR;

	const char *pPos = sStr.GetParsePointer();
	const char *pNumber = pPos + sStr.GetLength();
	while (pNumber > pPos && strIsDigit(pNumber - 1))
		pNumber--;

	CString sNumber(pNumber);
	if (sNumber.IsEmpty() || strOverflowsInteger32(sNumber))
		return strPattern("%s2", sStr);
	else
		{
		int iNumber = strParseInt(sNumber, 1);
		if (iNumber == INT_MAX)
			return strPattern("%s2", sStr);

		return strPattern("%s%d", CString(pPos, (pNumber - pPos)), iNumber + 1);
		}
	}

void CString::Init (LPCSTR pStr, int iLen)

//	Init
//
//	Initialize the string

	{
	if (pStr || iLen > 0)
		{
		LPSTR pBuffer = new char[sizeof(int) + iLen + 1];
		*(int *)pBuffer = iLen;
		m_pString = pBuffer + sizeof(int);

		if (pStr)
			utlMemCopy(pStr, m_pString, iLen);

		m_pString[iLen] = '\0';
		}
	else
		m_pString = NULL;	//	NULL
	}

bool CString::IsWhitespace (void) const

//	IsWhitespace
//
//	Returns TRUE if the string is completely whitespace (or empty).

	{
	char *pPos = GetParsePointer();
	char *pPosEnd = pPos + GetLength();

	while (pPos < pPosEnd)
		{
		if (!strIsWhitespace(pPos))
			return false;

		pPos++;
		}

	return true;
	}

void CString::Serialize (IByteStream &Stream) const

//	Serialize
//
//	Serialize to a binary format

	{
	//	Write out the length

	DWORD dwLength = GetLength();
	Stream.Write(&dwLength, sizeof(DWORD));

	//	Write out the string

	if (dwLength > 0)
		{
		Stream.Write(m_pString, dwLength);

		//	Pad to a DWORD

		int iPad = AlignUp((int)dwLength, (int)sizeof(DWORD)) - dwLength;
		if (iPad)
			{
			DWORD dwPad = 0;
			Stream.Write(&dwPad, iPad);
			}
		}
	}

void CString::SerializeJSON (IByteStream &Stream) const

//	SerializeJSON
//
//	Serializes to JSON format (not including surrounding
//	double-quotes).

	{
	char *pPos = m_pString;

	//	Handle some edge conditions

	if (GetLength() == 0)
		return;

	//	Keep looping until we're done

	bool bEscapeNextSlash = false;
	char *pStart = pPos;
	while (*pPos != '\0')
		{
		//	If we have a < then we need to escape the next character if it is
		//	a slash. This prevents close tag injections inside <script> element.

		if (*pPos == '<')
			{
			bEscapeNextSlash = true;
			pPos++;
			}

		//	Escape a slash

		else if (*pPos == '/' && bEscapeNextSlash)
			{
			//	Write out what we've got so far

			Stream.Write(pStart, pPos - pStart);

			//	Escape the character

			Stream.Write("\\/", 2);

			//	Next

			bEscapeNextSlash = false;
			pPos++;
			pStart = pPos;
			}

		//	Look for characters that we need to escape

		else if (*pPos == '\\' || *pPos == '\"' || strIsASCIIControl(pPos))
			{
			//	Write out what we've got so far

			Stream.Write(pStart, pPos - pStart);

			//	Escape the character

			switch (*pPos)
				{
				case '\"':
					Stream.Write("\\\"", 2);
					break;

				case '\\':
					Stream.Write("\\\\", 2);
					break;

				case '\b':
					Stream.Write("\\b", 2);
					break;

				case '\f':
					Stream.Write("\\f", 2);
					break;

				case '\n':
					Stream.Write("\\n", 2);
					break;

				case '\r':
					Stream.Write("\\r", 2);
					break;

				case '\t':
					Stream.Write("\\t", 2);
					break;

				default:
					Stream.Write(strPattern("\\u%04x", (DWORD)*pPos));
					break;
				}

			bEscapeNextSlash = false;
			pPos++;
			pStart = pPos;
			}
		else
			{
			bEscapeNextSlash = false;
			pPos++;
			}
		}

	//	Write out the remainder

	Stream.Write(pStart, pPos - pStart);
	}

void CString::SetLength (int iLength)

//	SetLength
//
//	Set the length of the string. If we're making the string longer, any
//	additional characters are uninitialized.

	{
	if (IsLiteral())
		{
		Init(NULL, iLength);
		}
	else if (m_pString)
		{
		int iCurrentLen = GetLengthParameter();
		if (iCurrentLen > iLength)
			{
			SetLengthParameter(iLength);
			m_pString[iLength] = '\0';
			}
		else if (iCurrentLen < iLength)
			{
			//	Create a buffer large enough to hold the result

			LPSTR pBuffer = new char[sizeof(int) + iLength + 1];
			*(int *)pBuffer = iLength;

			//	Copy the original string

			utlMemCopy(m_pString, pBuffer + sizeof(int), iCurrentLen);

			//	Clean up

			CleanUp();

			//	Assign

			m_pString = pBuffer + sizeof(int);
			m_pString[iLength] = '\0';
			}
		}
	else
		{
		if (iLength > 0)
			Init(NULL, iLength);
		}
	}

void CString::TakeHandoff (CString &sStr)

//	TakeHandoff
//
//	Takes ownership of the sStr's buffer. Empties out sStr

	{
	CleanUp();
	m_pString = sStr.m_pString;
	sStr.m_pString = NULL;
	}

void CString::TakeHandoff (CStringBuffer &Buffer)

//	TakeHandoff
//
//	Takes ownership of the buffer

	{
	CleanUp();
	m_pString = Buffer.Handoff();
	}

//	Functions

bool strASCIICharInSet (char *pPos, const CString &sSet)

//	strASCIICharInSet
//
//	Returns TRUE if the given 7-bit ASCII character is in the given set

	{
	char *pSet = sSet.GetParsePointer();
	char *pSetEnd = pSet + sSet.GetLength();

	while (pSet < pSetEnd)
		if (*pPos == *pSet++)
			return true;

	return false;
	}

CString strCapitalize (const CString &sString)

//	strCapitalize
//
//	Capitalizes the string

	{
	const char *pSrc = sString.GetParsePointer();
	const char *pSrcEnd = pSrc + sString.GetLength();

	//	Get the first character

	const char *pPos = pSrc;
	UTF32 dwFirstChar = strParseUTF8Char(&pPos, pSrcEnd);

	//	If already upper-case then we're done

	UTF32 dwUpper = strToUpperChar(dwFirstChar);
	if (dwUpper == dwFirstChar)
		return sString;

	//	Try to convert in place

	int iOldLen = strGetUTF8EncodeLength(dwFirstChar);
	int iNewLen = strGetUTF8EncodeLength(dwUpper);
	if (iOldLen == iNewLen)
		{
		CString sResult = sString;
		CBuffer Buffer(sResult.GetParsePointer(), sResult.GetLength(), false);
		strEncodeUTF8Char(dwUpper, Buffer);
		return sResult;
		}

	//	Otherwise, we need to allocate a new buffer

	else
		{
		CStringBuffer Result;
		strEncodeUTF8Char(dwUpper, Result);
		Result.Write(pSrc + iOldLen, sString.GetLength() - iOldLen);
		return CString::CreateFromHandoff(Result);
		}
	}

CString strClean (const CString &sText, DWORD dwFlags)

//	strClean
//
//	Clean up a string in the following ways:
//
//	*	Removes any leading or trailing whitespace
//	*	Removes/converts any control characters
//	*	Collapses multiple space characters into one

	{
	//	Short circuit.

	if (sText.IsEmpty())
		return sText;

	//	Result will always be shorter

	CString sResult(sText.GetLength());

	char *pSrc = sText.GetParsePointer();
	char *pSrcEnd = pSrc + sText.GetLength();
	char *pDest = sResult.GetParsePointer();

	bool bFoundNonSpace = false;
	bool bLastCharWasSpace = false;
	bool bFoundAlpha = false;
	bool bFoundAlphaNumeric = false;

	while (pSrc < pSrcEnd)
		{
		switch (*pSrc)
			{
			case '\t':
			case '\r':
			case '\n':
			case ' ':
				if (!bLastCharWasSpace && bFoundNonSpace)
					{
					if (dwFlags & STC_FLAG_NO_SPACES)
						*pDest++ = '_';
					else
						*pDest++ = ' ';
					bLastCharWasSpace = true;
					}
				break;

			default:
				if (strIsASCIIControl(pSrc))
					NULL;
				else
					{
					if (strIsASCIIAlpha(pSrc))
						{
						bFoundAlpha = true;
						bFoundAlphaNumeric = true;
						}
					else if (strIsASCIIAlphaNumeric(pSrc))
						bFoundAlphaNumeric = true;

					*pDest++ = *pSrc;
					bLastCharWasSpace = false;
					bFoundNonSpace = true;
					}
				break;
			}

		pSrc++;
		}

	//	Compute length of final string

	int iNewLen = (int)(pDest - sResult.GetParsePointer());
	if (bLastCharWasSpace)
		iNewLen--;

	//	If we didn't find any alpha characters and we want at least one then we fail

	if ((dwFlags & STC_FLAG_MUST_HAVE_ALPHA) && !bFoundAlpha)
		return NULL_STR;
	else if ((dwFlags & STC_FLAG_MUST_HAVE_ALPHANUMERIC) && !bFoundAlphaNumeric)
		return NULL_STR;

	//	Done

	sResult.SetLength(iNewLen);
	return sResult;
	}

bool strEndsWith (const CString &sString, const CString &sPartial)

//	strEndsWith
//
//	Returns TRUE if sString ends with sPartial

	{
	char *pString = (LPSTR)sString;
	char *pPartial = (LPSTR)sPartial;
	int iLen = sPartial.GetLength();

	//	Some edge conditions

	if (pPartial == NULL || iLen == 0)
		return true;
	else if (sString.GetLength() < iLen)
		return false;

	//	Match

	pString += (sString.GetLength() - iLen);

	char *pPartialEnd = pPartial + iLen;
	while (pPartial < pPartialEnd)
		if (*pPartial++ != *pString++)
			return false;

	return true;
	}

bool strEquals (const char *pKey1, const char *pKey2)

//	strEquals

	{
	if (pKey1 == pKey2)
		return true;
	else if (pKey1 == NULL)
		return false;
	else if (pKey2 == NULL)
		return false;
	else
		{
		const char *pPos1 = pKey1;
		const char *pPos2 = pKey2;
		while (*pPos1 != '\0' && *pPos2 != '\0')
			{
			if (*pPos1 != *pPos2)
				return false;

			pPos1++;
			pPos2++;
			}

		return (*pPos1 == *pPos2);
		}
	}

bool strEquals (const CString &sKey1, const CString &sKey2)

//	strEquals
//
//	Case-sensitive, non-locale specific comparison.
//	NOTE: Some clients rely on the fact that we can compare strings even if they
//	have embedded 0s.

	{
	//	Must be the same length

	int iKey1Len = sKey1.GetLength();
	int iKey2Len = sKey2.GetLength();
	if (iKey1Len != iKey2Len)
		return false;

	//	Prepare

	char *pPos1 = (LPSTR)sKey1;
	char *pPos1End = pPos1 + iKey1Len;
	char *pPos2 = (LPSTR)sKey2;
	char *pPos2End = pPos2 + iKey2Len;

	//	Handle NULL

	if (pPos1 == NULL)
		return (pPos2 == NULL || pPos2 == pPos2End);
	else if (pPos2 == NULL)
		return (pPos1 == pPos1End);

	//	Compare

	while (pPos1 < pPos1End && pPos2 < pPos2End)
		{
		if (*pPos1 != *pPos2)
			return false;

		pPos1++;
		pPos2++;
		}

	return (pPos1 == pPos1End && pPos2 == pPos2End);
	}

bool strEqualsNoCase (const CString &sKey1, const CString &sKey2)

//	strEqualsNoCase
//
//	Case-insensitive, non-locale specific comparison.
//	NOTE: Some clients rely on the fact that we can compare strings even if they
//	have embedded 0s.

	{
	//	Must be the same length

	int iKey1Len = sKey1.GetLength();
	int iKey2Len = sKey2.GetLength();
	if (iKey1Len != iKey2Len)
		return false;

	//	Prepare

	const char *pPos1 = (LPSTR)sKey1;
	const char *pPos1End = pPos1 + iKey1Len;
	const char *pPos2 = (LPSTR)sKey2;
	const char *pPos2End = pPos2 + iKey2Len;

	//	Handle NULL

	if (pPos1 == NULL)
		return (pPos2 == NULL || pPos2 == pPos2End);
	else if (pPos2 == NULL)
		return (pPos1 == pPos1End);

	//	Compare

	while (pPos1 < pPos1End && pPos2 < pPos2End)
		{
		UTF32 dwCodePoint1 = strParseUTF8Char(&pPos1, pPos1End);
		UTF32 dwCodePoint2 = strParseUTF8Char(&pPos2, pPos2End);

		if (strToLowerChar(dwCodePoint1) != strToLowerChar(dwCodePoint2))
			return false;
		}

	return (pPos1 == pPos1End && pPos2 == pPos2End);
	}

CString strEscapePrintf (const CString &sString)

//	strEscapePrintf
//
//	Converts all embedded % characters into double-% so that we can output 
//	through printf (etc) without error.

	{
	//	Optimistically assume that there are no % chars.

	char *pPos = sString.GetParsePointer();
	char *pPosEnd = pPos + sString.GetLength();
	while (pPos < pPosEnd)
		{
		if (*pPos == '%')
			break;

		pPos++;
		}

	//	If we did not find a % then we're done

	if (pPos == pPosEnd)
		return sString;

	//	We need to convert

	CStringBuffer Buffer;
	pPos = sString.GetParsePointer();
	pPosEnd = pPos + sString.GetLength();
	char *pStart = pPos;
	while (pPos < pPosEnd)
		{
		if (*pPos == '%')
			{
			if (pStart != pPos)
				Buffer.Write(pStart, pPos - pStart);

			Buffer.Write("%%", 2);

			pPos++;
			pStart = pPos;
			}
		else
			pPos++;
		}

	if (pStart != pPos)
		Buffer.Write(pStart, pPos - pStart);

	//	Done

	return CString::CreateFromHandoff(Buffer);
	}

int strFind (const CString &sString, const CString &sStringToFind)

//	strFind
//
//	Case-sensitive find.

	{
	char *pPos1Start = (LPSTR)sString;
	char *pPos1 = pPos1Start;
	char *pPos1End = pPos1 + sString.GetLength();
	char *pPos2 = (LPSTR)sStringToFind;
	char *pPos2End = pPos2 + sStringToFind.GetLength();

	//	Handle NULL

	if (pPos1 == NULL || pPos1 == pPos1End)
		return -1;
	else if (pPos2 == NULL || pPos2 == pPos2End)
		return 0;

	//	Look

	char *pPos1Last = pPos1End - sStringToFind.GetLength();
	while (pPos1 <= pPos1Last)
		{
		if (*pPos1 == *pPos2)
			{
			//	See how far we match

			char *pTarget = pPos1 + 1;
			char *pToFind = pPos2 + 1;
			while (pToFind < pPos2End && *pTarget == *pToFind)
				{
				pTarget++;
				pToFind++;
				}

			//	Found it?

			if (pToFind == pPos2End)
				return (int)(pPos1 - pPos1Start);
			}

		//	Next

		pPos1++;
		}

	//	Not found

	return -1;
	}

int strFindNoCase (const CString &sString, const CString &sStringToFind)

//	strFindNoCase
//
//	Case-insensitive find.

	{
	const char *pPos1Start = (LPSTR)sString;
	const char *pPos1 = pPos1Start;
	const char *pPos1End = pPos1 + sString.GetLength();
	const char *pPos2 = (LPSTR)sStringToFind;
	const char *pPos2End = pPos2 + sStringToFind.GetLength();

	//	Handle NULL

	if (pPos1 == NULL || pPos1 == pPos1End)
		return -1;
	else if (pPos2 == NULL || pPos2 == pPos2End)
		return 0;

	const char *pFirstPos = pPos2;
	UTF32 dwFirstCodePoint = strParseUTF8Char(&pFirstPos, pPos2End);

	//	Look

	const char *pPos1Last = pPos1End - sStringToFind.GetLength();
	while (pPos1 <= pPos1Last)
		{
		const char *pAdvance = pPos1;
		UTF32 dwCodePoint1 = strParseUTF8Char(&pAdvance, pPos1End);

		if (strToLowerChar(dwCodePoint1) == strToLowerChar(dwFirstCodePoint))
			{
			bool bFound = true;

			//	See how far we match

			const char *pTarget = pPos1;
			const char *pToFind = pPos2;
			while (pToFind < pPos2End)
				{
				UTF32 dwCodePoint1 = strParseUTF8Char(&pTarget, pPos1End);
				UTF32 dwCodePoint2 = strParseUTF8Char(&pToFind, pPos2End);

				if (strToLowerChar(dwCodePoint1) != strToLowerChar(dwCodePoint2))
					{
					bFound = false;
					break;
					}
				}

			//	Found it?

			if (bFound)
				return (int)(pPos1 - pPos1Start);
			}

		pPos1 = pAdvance;
		}

	//	Not found

	return -1;
	}

CString strFormatInteger (int iValue, int iMinFieldWidth, DWORD dwFlags)

//	strFormatInteger
//
//	Converts an integer to a string

	{
	int i;

	DWORD dwRadix = 10;
	bool bNegative = (iValue < 0) && !(dwFlags & FORMAT_UNSIGNED);
	DWORD dwValue = (bNegative ? (DWORD)(-iValue) : (DWORD)iValue);

	//	Convert to new base (we end up in reverse order)

	DWORD Result[100];
	int iDigitCount = 0;
	DWORD dwRemainder = dwValue;

	do
		{
		Result[iDigitCount] = (dwRemainder % dwRadix);

		iDigitCount++;
		dwRemainder = dwRemainder / dwRadix;
		}
	while (dwRemainder > 0);

	//	Compute the length of the result (not counting padding,
	//	but including thousands separators).

	int iResultLength = iDigitCount;
	if (dwFlags & FORMAT_THOUSAND_SEPARATOR)
		iResultLength += (iDigitCount - 1) / 3;

	if (bNegative)
		iResultLength += 1;

	//	Total length

	int iTotalLength = (iMinFieldWidth == -1 ? iResultLength : Max(iResultLength, iMinFieldWidth));

	//	Allocate result string

	CString sResult(iTotalLength);
	char *pPos = sResult.GetPointer();

	//	Padding

	char *pEndPos = pPos + (iTotalLength - iResultLength);
	while (pPos < pEndPos)
		*pPos++ = ((dwFlags & FORMAT_LEADING_ZERO) ? '0' : ' ');

	//	Sign

	if (bNegative)
		*pPos++ = '-';

	//	Write the result backwards

	char *pDigitPos = sResult.GetPointer() + (iTotalLength - 1);
	for (i = 0; i < iDigitCount; i++)
		{
		if ((dwFlags & FORMAT_THOUSAND_SEPARATOR) && i > 0 && (i % 3) == 0)
			*pDigitPos-- = ',';

		*pDigitPos-- = (char)('0' + Result[i]);
		}

	//	Done

	return sResult;
	}

CString strFromDouble (double rValue, int iDecimals)

//	strFromDouble
//
//	Converts a double to a string

	{
	CString sResult(_CVTBUFSIZE + 1);
	char *pPos = sResult.GetParsePointer();

	//	-1 means we adjust

	if (iDecimals <= 0)
		{
		CStringBuffer Dest;

		//	If necessary adjust decimal output based on the size of the number.

		if (iDecimals == -1)
			{
			double rValueAbs = Abs(rValue);

			if (rValueAbs >= 100000000.0)
				{
				if (_gcvt_s(pPos, sResult.GetLength(), rValue, 8) != 0)
					return STR_NAN;

				//	Figure out how much we wrote

				int iLen = (int)strlen(pPos);
				sResult.SetLength(iLen);
				return sResult;
				}
			else if (rValueAbs >= 0.00001)
				iDecimals = 6;
			else if (rValueAbs >= 0.000001)
				iDecimals = 7;
			else if (rValueAbs >= 0.0000001)
				iDecimals = 8;
			else
				{
				if (_gcvt_s(pPos, sResult.GetLength(), rValue, 8) != 0)
					return STR_NAN;

				//	Figure out how much we wrote

				int iLen = (int)strlen(pPos);
				sResult.SetLength(iLen);
				return sResult;
				}
			}
		else if (iDecimals < -1)
			iDecimals = -iDecimals - 1;

		int iDec, iSign;
		if (_fcvt_s(pPos, sResult.GetLength(), rValue, iDecimals, &iDec, &iSign))
			return STR_NAN;

		char *pSrc = sResult.GetParsePointer();

		//	Now generate the final string based on parameters.

		if (iSign)
			Dest.WriteChar('-');

		if (iDec == 0)
			{
			if (iDecimals == 0)
				{
				Dest.WriteChar('0');
				}
			else
				{
				Dest.WriteChar('0');
				Dest.WriteChar('.');

				Dest.Write(pSrc, iDecimals);
				}
			}
		else if (iDec <= 0)
			{
			Dest.WriteChar('0');
			Dest.WriteChar('.');
			if (iDec)
				Dest.WriteChar('0', -iDec);

			int iLen = (int)strlen(pSrc);
			if (iLen)
				{
				while (iLen > 0 && pSrc[iLen - 1] == '0')
					iLen--;

				Dest.Write(pSrc, iLen);
				}
			else
				Dest.WriteChar('0');
			}
		else
			{
			Dest.Write(pSrc, iDec);
			pSrc += iDec;
			if (iDecimals > 0)
				{
				Dest.WriteChar('.');

				while (iDecimals > 0 && pSrc[iDecimals - 1] == '0')
					iDecimals--;

				if (iDecimals)
					Dest.Write(pSrc, iDecimals);
				else
					Dest.WriteChar('0');
				}
			}

		//	Done

		return CString::CreateFromHandoff(Dest);
		}

	//	Otherwise, iDecimals to total number of significant digits

	else
		{
		if (_gcvt_s(pPos, sResult.GetLength(), rValue, iDecimals) != 0)
			return STR_NAN;

		//	Figure out how much we wrote

		int iLen = (int)strlen(pPos);

		//	If we have a '.' followed by nothing, then we need to add a digit.
		//	JSON and others do not like a number like "2.e-002"

		char *pEdit = pPos;
		char *pPosEnd = pPos + iLen;
		while (pEdit < pPosEnd)
			{
			if (*pEdit == '.')
				{
				if (pEdit + 1 == pPosEnd || pEdit[1] == 'e' || pEdit[1] == 'E')
					{
					char *pSlide = pPosEnd - 1;
					char *pSlideEnd = pEdit + 1;

					while (pSlide >= pSlideEnd)
						{
						pSlide[1] = *pSlide;
						pSlide--;
						}

					pEdit[1] = '0';

					pPosEnd++;
					iLen++;
					*pPosEnd = '\0';
					}
				break;
				}

			pEdit++;
			}

		sResult.SetLength(iLen);
		return sResult;
		}
	}

CString strFromInt (int iInteger, bool bSigned)

//	strFromInt
//
//	Converts an integer to a string

	{
	char szString[256];
	int iLen;

	if (bSigned)
		iLen = sprintf_s(szString, sizeof(szString), "%d", iInteger);
	else
		iLen = sprintf_s(szString, sizeof(szString), "%u", iInteger);

	return CString(szString, iLen);
	}

CString strFromIntOfBase (int iInteger, int iBase, bool bSigned, bool bUppercase)

//	strFromIntOfBase
//
//	Converts to a string using the given base 

	{
	//	Too lazy to do this the right way. If I ever need base 7
	//	numbers I'll fix this.

	ASSERT(iBase == 10 || iBase == 16);

	switch (iBase)
		{
		case 10:
			return strFromInt(iInteger, bSigned);

		case 16:
			{
			char szString[256];
			int iLen;

			if (bUppercase)
				iLen = sprintf_s(szString, sizeof(szString), "%X", (DWORD)iInteger);
			else
				iLen = sprintf_s(szString, sizeof(szString), "%x", (DWORD)iInteger);

			return CString(szString, iLen);
			}

		default:
			return CString();
		}
	}

bool strIsASCIISymbol (char *pPos)

//	strIsASCIISymbol
//
//	Returns TRUE if the character is an ASCII symbol

	{
	switch (*pPos)
		{
		case '!':
		case '\"':
		case '#':
		case '$':
		case '%':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '-':
		case '.':
		case '/':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case '[':
		case '\\':
		case ']':
		case '^':
		case '_':
		case '`':
		case '{':
		case '|':
		case '}':
		case '~':
			return true;

		default:
			return false;
		}
	}

bool strIsInt (const CString &sString, int *retiValue)

//	strIsInt
//
//	Returns TRUE if the string can be completely parsed into an integer.
//	We support any value accepted by strParseInt.

	{
	bool bFailed;
	const char *pPos = sString.GetParsePointer();
	int iValue = strParseInt(pPos, 0, &pPos, &bFailed);
	if (bFailed || *pPos != '\0')
		return false;

	if (retiValue)
		*retiValue = iValue;

	return true;
	}

int strLength (LPCSTR pStr)

//	strLength
//
//	Returns the length of the string in bytes
//	(excluding the terminating NULL)

	{
	if (pStr == NULL)
		return 0;

	LPCSTR pStart = pStr;
	while (*pStr != '\0')
		pStr++;

	return (int)(pStr - pStart);
	}

CString strOrdinal (int iOrdinal)

//	strOrdinal
//
//	Returns an ordinal: "1st", "2nd", "3rd", etc.

	{
	int iOnes = (iOrdinal % 10);
	int iTeens = (iOrdinal % 100);

	if (iOnes == 1 && iTeens != 11)
		return strPattern("%dst", iOrdinal);
	else if (iOnes == 2 && iTeens != 12)
		return strPattern("%dnd", iOrdinal);
	else if (iOnes == 3 && iTeens != 13)
		return strPattern("%drd", iOrdinal);
	else
		return strPattern("%dth", iOrdinal);
	}

bool strOverflowsInteger32 (const CString &sValue)

//	strOverflowsInteger32
//
//	Returns TRUE if the given string is a string of digits that does not fit
//	in a signed 32-bit integer.

	{
	int i;
	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();

	//	See if we've got a sign

	bool bNegative;
	if (*pPos == '-')
		{
		pPos++;
		bNegative = true;
		}
	else if (*pPos == '+')
		{
		pPos++;
		bNegative = false;
		}
	else
		bNegative = false;

	//	See how many digit we've got

	while (*pPos == '0')
		pPos++;

	int iSignificantDigits = (int)(pPosEnd - pPos);
	if (iSignificantDigits < 10)
		return false;
	else if (iSignificantDigits > 10)
		return true;

	//	Continue

	if (bNegative)
		{
		for (i = 0; i < 10; i++)
			{
			if (pPos[i] < MIN_INTEGER_DIGITS[i])
				return false;
			else if (pPos[i] > MIN_INTEGER_DIGITS[i])
				return true;
			}

		//	If we get this far then we're at the limit

		return false;
		}
	else
		{
		for (i = 0; i < 10; i++)
			{
			if (pPos[i] < MAX_INTEGER_DIGITS[i])
				return false;
			else if (pPos[i] > MAX_INTEGER_DIGITS[i])
				return true;
			}

		//	If we get this far then we're at the limit

		return false;
		}
	}

double strParseDouble (const char *pStart, double rNullResult, const char **retpEnd, bool *retbNullValue)

//  strParseDouble
//
//  Parses a double precision value.

	{
	const char *pPos = pStart;

	//  Skip any leading whitespace

	while (strIsWhitespace(pPos))
		pPos++;

	//  We copy what we've got to a buffer, because we're going to use atof.

	const int MAX_SIZE = 64;
	const char *pSrc = pPos;
	char szBuffer[MAX_SIZE];
	char *pDest = szBuffer;
	char *pDestEnd = szBuffer + MAX_SIZE;

	while (pDest < pDestEnd
			&& ((*pSrc >= '0' && *pSrc <= '9')
				|| *pSrc == '+'
				|| *pSrc == '-'
				|| *pSrc == '.'
				|| *pSrc == 'e'
				|| *pSrc == 'E'))
		*pDest++ = *pSrc++;

	if (retpEnd) *retpEnd = pSrc;

	//  If we hit the end of the buffer, or we didn't find any valid characters,
	//  then we fail.

	if (pDest == pDestEnd
			|| pDest == szBuffer)
		{
		if (retbNullValue) *retbNullValue = true;
		return rNullResult;
		}

	//  Null-terminate our buffer

	*pDest++ = '\0';

	//  Success 

	if (retbNullValue) *retbNullValue = false;
	return atof(szBuffer);
	}

int strParseHexChar (char chChar, int iNullResult)

//	strParseHexChar
//
//	Returns the value of a hexadecimal character.

	{
	switch (chChar)
		{
		case 'a':
		case 'A':
			return 10;

		case 'b':
		case 'B':
			return 11;

		case 'c':
		case 'C':
			return 12;

		case 'd':
		case 'D':
			return 13;

		case 'e':
		case 'E':
			return 14;

		case 'f':
		case 'F':
			return 15;

		default:
			if (chChar >= '0' && chChar <='9')
				return (int)(chChar - '0');
			else
				return iNullResult;
		}
	}

int strParseInt (const char *pStart, int iNullResult, const char **retpEnd, bool *retbNullValue)

//	strParseInt
//
//	pStart: Start parsing. Skips any leading whitespace
//	iNullResult: If there are no valid numbers, returns this value
//	retpEnd: Returns the character at which we stopped parsing
//	retbNullValue: Returns true if there are no valid numbers.

	{
	const char *pPos;
	bool bNegative;
	bool bFoundNumber;
	bool bHex;
	int iInt;

	//	Edge-cases

	if (pStart == NULL)
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pStart;

		return iNullResult;
		}

	//	Preset

	if (retbNullValue)
		*retbNullValue = false;

	pPos = pStart;
	bNegative = false;
	bFoundNumber = false;
	bHex = false;
	iInt = 0;

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r')
		pPos++;

	//	If NULL, then we're done

	if (*pPos == '\0')
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	If negative, remember it

	if (*pPos == '-')
		{
		bNegative = true;
		pPos++;
		}
	else if (*pPos == '+')
		pPos++;

	//	See if this is a hex number

	if (*pPos == '0')
		{
		pPos++;
		bFoundNumber = true;

		//	If the next character is x (or X) then we've got
		//	a Hex number

		if (*pPos == 'x' || *pPos == 'X')
			{
			pPos++;
			bHex = true;
			}
		}

	//	Keep parsing

	if (bHex)
		{
		DWORD dwInt = 0;

		while (*pPos != '\0' 
				&& ((*pPos >= '0' && *pPos <= '9') 
					|| (*pPos >= 'a' && *pPos <='f')
					|| (*pPos >= 'A' && *pPos <= 'F')))
			{
			if (*pPos >= '0' && *pPos <= '9')
				dwInt = 16 * dwInt + (*pPos - '0');
			else if (*pPos >= 'A' && *pPos <= 'F')
				dwInt = 16 * dwInt + (10 + (*pPos - 'A'));
			else
				dwInt = 16 * dwInt + (10 + (*pPos - 'a'));

			pPos++;
			}

		iInt = (int)dwInt;
		}
	else
		{
		while (*pPos != '\0' && *pPos >= '0' && *pPos <= '9')
			{
			iInt = 10 * iInt + (*pPos - '0');
			pPos++;
			bFoundNumber = true;
			}
		}

	//	Done?

	if (!bFoundNumber)
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	Done!

	if (bNegative)
		iInt = -iInt;

	if (retpEnd)
		*retpEnd = pPos;

	return iInt;
	}

int strParseIntOfBase (const char *pStart, int iBase, int iNullResult, const char **retpEnd, bool *retbNullValue)

//	strParseIntOfBase
//
//	Parses an integer of the given base

	{
	const char *pPos;
	bool bNegative;
	bool bFoundNumber;
	int iInt;

	//	Edge-cases

	if (pStart == NULL)
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pStart;

		return iNullResult;
		}

	//	Preset

	if (retbNullValue)
		*retbNullValue = false;

	pPos = pStart;
	bNegative = false;
	bFoundNumber = false;
	iInt = 0;

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r')
		pPos++;

	//	If NULL, then we're done

	if (*pPos == '\0')
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	If negative, remember it

	if (*pPos == '-')
		{
		bNegative = true;
		pPos++;
		}
	else if (*pPos == '+')
		pPos++;

	//	See if this is a hex number

	if (*pPos == '0')
		{
		pPos++;
		bFoundNumber = true;

		//	If the next character is x (or X) then we've got
		//	a Hex number

		if (*pPos == 'x' || *pPos == 'X')
			{
			pPos++;
			iBase = 16;
			}
		}

	//	Now parse for numbers

	DWORD dwInt = 0;

	while (*pPos != '\0' 
			&& ((*pPos >= '0' && *pPos <= '9') 
				|| (*pPos >= 'a' && *pPos <='f')
				|| (*pPos >= 'A' && *pPos <= 'F')))
		{
		if (*pPos >= '0' && *pPos <= '9')
			dwInt = (DWORD)iBase * dwInt + (*pPos - '0');
		else if (*pPos >= 'A' && *pPos <= 'F')
			dwInt = (DWORD)iBase * dwInt + (10 + (*pPos - 'A'));
		else
			dwInt = (DWORD)iBase * dwInt + (10 + (*pPos - 'a'));

		pPos++;
		bFoundNumber = true;
		}

	iInt = (int)dwInt;

	//	Done?

	if (!bFoundNumber)
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	Done!

	if (bNegative)
		iInt = -iInt;

	if (retpEnd)
		*retpEnd = pPos;

	return iInt;
	}

//	Pattern format:
//
//	%type
//
//	where type is one of the following:
//
//	%	Evaluates to a single percent sign
//
//	s	Argument is a CString. The string is substituted
//
//	d	Argument is a signed 32-bit integer. The number is substituted
//		a count in front of this type will add leading zeros
//
//	D	Argument is a signed 32-bit integer. Comma separators are added
//		at 1000 marks.
//
//	x	Argument is an unsigned 32-bit integer expressed as hexadecimal
//		A count in front of this type will add leading zeros
//
//	p	If the last integer argument not 1, then 's' is substituted.
//		This is used to pluralize words in the English language.

CString strPatternEngine (LPCSTR pPattern, LPVOID *pArgs, PATTERNHOOKPROC pfHook, LPVOID pCtx)

//	strPatternEngine
//
//	Returns a string with a pattern substitution

	{
	CMemoryBuffer Stream(4096);
	LPCSTR pPos = pPattern;
	LPCSTR pRunStart;
	int iRunLength;
	int iLastInteger = 1;

	//	Start

	pRunStart = pPos;
	iRunLength = 0;

	//	Loop

	while (*pPos != '\0')
		{
		if (*pPos == '%')
			{
			int iFieldSize = 0;
			bool bZeroPad = false;

			//	Save out what we've got til now

			if (iRunLength > 0)
				{
				Stream.Write(pRunStart, iRunLength);
				iRunLength = 0;
				}

			pPos++;

			//	Check some optional flags

			if (*pPos == '0')
				{
				bZeroPad = true;
				pPos++;
				}

			//	Read the field size, if any

			iFieldSize = strParseInt(pPos, 0, &pPos);

			//	Check the actual pattern code

			if (*pPos == 's')
				{
				LPSTR *pParam = (LPSTR *)pArgs;

				Stream.Write(*pParam, strLength(*pParam));

				pArgs++;
				pPos++;
				}
			else if (*pPos == 'd' || *pPos == 'D')
				{
				int *pInt = (int *)pArgs;
				CString sNew;

				sNew = strFromInt(*pInt, true);
				if (sNew.GetLength() < iFieldSize)
					Stream.WriteChar((bZeroPad ? '0' : ' '), iFieldSize - sNew.GetLength());

				if (*pPos == 'd')
					Stream.Write(sNew);
				else
					{
					int iRun = sNew.GetLength() % 3;
					if (iRun == 0)
						iRun = 3;
					LPSTR pSrc = sNew;

					bool bComma = false;
					while (*pSrc != '\0')
						{
						if (bComma)
							Stream.Write(",", 1);

						Stream.Write(pSrc, iRun);
						pSrc += iRun;
						iRun = 3;
						bComma = true;
						}
					}

				//	Remember the last integer

				iLastInteger = *pInt;

				//	Next

				pArgs++;
				pPos++;
				}
			else if (*pPos == 'x' || *pPos == 'X')
				{
				DWORD *pInt = (DWORD *)pArgs;
				CString sNew;

				sNew = strFromIntOfBase(*pInt, 16, false, (*pPos == 'X'));
				if (sNew.GetLength() < iFieldSize)
					Stream.WriteChar('0', iFieldSize - sNew.GetLength());

				Stream.Write(sNew);

				//	Remember the last integer

				iLastInteger = (int)*pInt;

				//	Next

				pArgs++;
				pPos++;
				}
			else if (*pPos == 'p')
				{
				if (iLastInteger != 1)
					Stream.Write("s", 1);

				pPos++;
				}
			else if (*pPos == 'c')
				{
				int *pInt = (int *)pArgs;
				char chChar = (char)(*pInt);
				Stream.Write(&chChar, 1);

				pArgs++;
				pPos++;
				}
			else if (*pPos == '%')
				{
				Stream.Write("%", 1);

				pPos++;
				}
			else if (*pPos == '@')
				{
				pPos++;
				if (pfHook)
					(*pfHook)(pCtx, pPos, pArgs, &Stream);
				}

			pRunStart = pPos;
			iRunLength = 0;
			}
		else
			{
			iRunLength++;
			pPos++;
			}
		}

	//	Save out the last run

	if (iRunLength > 0)
		Stream.Write(pRunStart, iRunLength);

	//	Convert the stream to a string

	return CString(Stream.GetPointer(), Stream.GetLength());
	}

CString strPattern (LPCSTR pLine, ...)

//	strPattern
//
//	Substitutes patterns

	{
	char *pArgs;
	pArgs = (char *) &pLine + sizeof(pLine);
	return strPatternEngine(pLine, (void **)pArgs, NULL, NULL);
	}

CString strPatternEx (LPCSTR pLine, PATTERNHOOKPROC pfHook, LPVOID pCtx, ...)

//	strPatternEx
//
//	Substitutes patterns

	{
	char *pArgs;
	pArgs = (char *) &pLine + sizeof(pLine) + sizeof(pfHook) + sizeof(pCtx);
	return strPatternEngine(pLine, (void **)pArgs, pfHook, pCtx);
	}

CString strRepeat (char chChar, int iCount)

//	strRepeat
//
//	Creates a repeating string.

	{
	if (iCount <= 0)
		return NULL_STR;

	CString sString(iCount);
	char *pPos = sString.GetParsePointer();
	char *pPosEnd = pPos + iCount;
	while (pPos < pPosEnd)
		*pPos++ = chChar;

	return sString;
	}

void strSplit (const CString &sString, const CString &sSeparators, TArray<CString> *retResult, int iMaxCount, DWORD dwFlags)

//	strSplit
//
//	Split the string

	{
	enum EStates
		{
		stateStart,
		stateItem,

		stateExpectCR,
		stateExpectLF,
		};

	//	Short circuit.

	retResult->DeleteAll();
	if (sString.IsEmpty() || iMaxCount == 0)
		return;

	//	Prepare some stuff

	const char *pPos = sString.GetParsePointer();
	const char *pPosEnd = pPos + sString.GetLength();

	const char *pSep = sSeparators.GetParsePointer();
	const char *pSepEnd = pSep + sSeparators.GetLength();

	bool bDone = false;
	const char *pStart = pPos;
	int iCount = 0;
	EStates iState = stateStart;

	bool bForceLowercase = ((dwFlags & SSP_FLAG_FORCE_LOWERCASE) == SSP_FLAG_FORCE_LOWERCASE);

	//	We optimize a few special cases. If we want line separation then
	//	we ignore the sSeparators list.

	if (dwFlags & SSP_FLAG_LINE_SEPARATOR)
		{
		while (!bDone)
			{
			switch (iState)
				{
				case stateStart:
					if (pPos == pPosEnd)
						bDone = true;
					else if (*pPos == '\r')
						{
						retResult->Insert(NULL_STR);
						if (iMaxCount != -1 && ++iCount == iMaxCount)
							bDone = true;

						iState = stateExpectLF;
						pPos++;
						}
					else if (*pPos == '\n')
						{
						retResult->Insert(NULL_STR);
						if (iMaxCount != -1 && ++iCount == iMaxCount)
							bDone = true;

						iState = stateExpectCR;
						pPos++;
						}
					else
						{
						iState = stateItem;
						pPos++;
						}
					break;

				case stateItem:
					if (pPos == pPosEnd || *pPos == '\r' || *pPos == '\n')
						{
						if (bForceLowercase)
							retResult->Insert(strToLower(CString(pStart, pPos - pStart)));
						else
							retResult->Insert(CString(pStart, pPos - pStart));

						if (pPos == pPosEnd || (iMaxCount != -1 && ++iCount == iMaxCount))
							bDone = true;
						else if (*pPos == '\r')
							{
							pPos++;
							iState = stateExpectLF;
							}
						else
							{
							pPos++;
							iState = stateExpectCR;
							}
						}
					else
						pPos++;
					break;

				case stateExpectCR:
					if (pPos == pPosEnd)
						{
						retResult->Insert(NULL_STR);
						if (iMaxCount != -1 && ++iCount == iMaxCount)
							bDone = true;
						}
					else if (*pPos == '\r')
						{
						pPos++;
						pStart = pPos;
						iState = stateStart;
						}
					else if (*pPos == '\n')
						{
						retResult->Insert(NULL_STR);
						if (iMaxCount != -1 && ++iCount == iMaxCount)
							bDone = true;

						pPos++;
						iState = stateExpectCR;
						}
					else
						{
						pStart = pPos;
						pPos++;
						iState = stateItem;
						}
					break;

				case stateExpectLF:
					if (pPos == pPosEnd)
						{
						retResult->Insert(NULL_STR);
						if (iMaxCount != -1 && ++iCount == iMaxCount)
							bDone = true;
						}
					else if (*pPos == '\n')
						{
						pPos++;
						pStart = pPos;
						iState = stateStart;
						}
					else if (*pPos == '\r')
						{
						retResult->Insert(NULL_STR);
						if (iMaxCount != -1 && ++iCount == iMaxCount)
							bDone = true;

						pPos++;
						iState = stateExpectLF;
						}
					else
						{
						pStart = pPos;
						pPos++;
						iState = stateItem;
						}
					break;

				default:
					ASSERT(false);
					return;
				}
			}
		}

	//	Handle alphanumeric splits

	else if ((dwFlags & SSP_FLAG_ALPHANUMERIC_ONLY) && sSeparators.IsEmpty())
		{
		while (!bDone)
			{
			switch (iState)
				{
				case stateStart:
					if (pPos == pPosEnd)
						bDone = true;

					//	If we're ASCII alphanumeric, then we're in a word

					else if (strIsASCIIAlphaNumeric(pPos))
						{
						pStart = pPos++;
						iState = stateItem;
						}

					//	Otherwise, if we're lower ASCII, then this is a non-alphanumeric

					else if (!strIsASCIIHigh(pPos))
						pPos++;

					//	Otherwise we need to handle a Unicode code point

					else
						{
						const char *pPosAdvance = pPos;
						UTF32 dwCodePoint = strParseUTF8Char(&pPosAdvance, pPosEnd);
						if (strIsAlphaNumeric(dwCodePoint))
							{
							pStart = pPos;
							iState = stateItem;
							}

						pPos = pPosAdvance;
						}
					break;

				case stateItem:
					{
					bool bWordDone = false;
					const char *pPosAdvance = pPos + 1;
					if (pPos == pPosEnd)
						bWordDone = true;

					else if (strIsASCIIAlphaNumeric(pPos))
						;

					else if (!strIsASCIIHigh(pPos))
						bWordDone = true;

					else
						{
						pPosAdvance = pPos;
						bWordDone = !strIsAlphaNumeric(strParseUTF8Char(&pPosAdvance, pPosEnd));
						}

					if (bWordDone)
						{
						if (bForceLowercase)
							retResult->Insert(strToLower(CString(pStart, pPos - pStart)));
						else
							retResult->Insert(CString(pStart, pPos - pStart));

						if (pPos == pPosEnd || (iMaxCount != -1 && ++iCount == iMaxCount))
							bDone = true;
						else
							{
							pPos = pPosAdvance;
							iState = stateStart;
							}
						}
					else
						pPos = pPosAdvance;
					break;
					}

				default:
					ASSERT(false);
					return;
				}
			}
		}

	//	Handle whitespace with no other separators

	else if ((dwFlags & SSP_FLAG_WHITESPACE_SEPARATOR) && sSeparators.IsEmpty())
		{
		while (!bDone)
			{
			switch (iState)
				{
				case stateStart:
					if (pPos == pPosEnd)
						bDone = true;
					else if (!strIsWhitespace(pPos))
						{
						pStart = pPos++;
						iState = stateItem;
						}
					else
						pPos++;
					break;

				case stateItem:
					if (pPos == pPosEnd || strIsWhitespace(pPos))
						{
						if (bForceLowercase)
							retResult->Insert(strToLower(CString(pStart, pPos - pStart)));
						else
							retResult->Insert(CString(pStart, pPos - pStart));

						if (pPos == pPosEnd || (iMaxCount != -1 && ++iCount == iMaxCount))
							bDone = true;
						else
							{
							pPos++;
							iState = stateStart;
							}
						}
					else
						pPos++;
					break;

				default:
					ASSERT(false);
					return;
				}
			}
		}

	//	If we have no separators then we split up by character.

	else if (sSeparators.IsEmpty())
		{
		if (bForceLowercase)
			{
			while (pPos < pPosEnd)
				{
				retResult->Insert(strToLower(CString(pPos, 1)));

				if (iMaxCount != -1 && ++iCount == iMaxCount)
					break;

				pPos++;
				}
			}
		else
			{
			while (pPos < pPosEnd)
				{
				retResult->Insert(CString(pPos, 1));

				if (iMaxCount != -1 && ++iCount == iMaxCount)
					break;

				pPos++;
				}
			}
		}

	//	If we have a single separator then we optimize that.

	else if (sSeparators.GetLength() == 1 && !(dwFlags & SSP_FLAG_WHITESPACE_SEPARATOR))
		{
		char chSep = *pSep;
		bool bNoEmpty = ((dwFlags & SSP_FLAG_NO_EMPTY_ITEMS) == SSP_FLAG_NO_EMPTY_ITEMS);

		while (!bDone)
			{
			switch (iState)
				{
				case stateStart:
					if (pPos == pPosEnd)
						bDone = true;
					else if (*pPos == chSep)
						{
						if (bNoEmpty)
							pPos++;
						else
							{
							retResult->Insert(NULL_STR);

							if (iMaxCount != -1 && ++iCount == iMaxCount)
								bDone = true;
							else
								pPos++;
							}
						}
					else
						{
						pStart = pPos++;
						iState = stateItem;
						}
					break;

				case stateItem:
					if (pPos == pPosEnd || *pPos == chSep)
						{
						if (bForceLowercase)
							retResult->Insert(strToLower(CString(pStart, pPos - pStart)));
						else
							retResult->Insert(CString(pStart, pPos - pStart));

						if (pPos == pPosEnd || (iMaxCount != -1 && ++iCount == iMaxCount))
							bDone = true;
						else
							{
							pPos++;
							iState = stateStart;
							}
						}
					else
						pPos++;
					break;

				default:
					ASSERT(false);
					return;
				}
			}
		}

	//	Otherwise we do it the long way.

	else
		{
		bool bNoEmpty = ((dwFlags & SSP_FLAG_NO_EMPTY_ITEMS) == SSP_FLAG_NO_EMPTY_ITEMS);
		bool bWhitespace = ((dwFlags & SSP_FLAG_WHITESPACE_SEPARATOR) == SSP_FLAG_WHITESPACE_SEPARATOR);

		while (!bDone)
			{
			//	Figure out if the character is a separator

			bool bIsSeparator;
			if (pPos == pPosEnd)
				bIsSeparator = true;
			else if (bWhitespace)
				{
				bIsSeparator = false;
				pSep = sSeparators.GetParsePointer();
				while (pSep < pSepEnd)
					{
					if (*pSep == *pPos || strIsWhitespace(pPos))
						{
						bIsSeparator = true;
						break;
						}

					pSep++;
					}
				}
			else
				{
				bIsSeparator = false;
				pSep = sSeparators.GetParsePointer();
				while (pSep < pSepEnd)
					{
					if (*pSep == *pPos)
						{
						bIsSeparator = true;
						break;
						}

					pSep++;
					}
				}

			//	State machine

			switch (iState)
				{
				case stateStart:
					if (pPos == pPosEnd)
						bDone = true;
					else if (bIsSeparator)
						{
						if (bNoEmpty)
							pPos++;
						else
							{
							retResult->Insert(NULL_STR);

							if (iMaxCount != -1 && ++iCount == iMaxCount)
								bDone = true;
							else
								pPos++;
							}
						}
					else
						{
						pStart = pPos++;
						iState = stateItem;
						}
					break;

				case stateItem:
					if (pPos == pPosEnd || bIsSeparator)
						{
						if (bForceLowercase)
							retResult->Insert(strToLower(CString(pStart, pPos - pStart)));
						else
							retResult->Insert(CString(pStart, pPos - pStart));

						if (pPos == pPosEnd || (iMaxCount != -1 && ++iCount == iMaxCount))
							bDone = true;
						else
							{
							pPos++;
							iState = stateStart;
							}
						}
					else
						pPos++;
					break;

				default:
					ASSERT(false);
					return;
				}
			}
		}
	}

bool strStartsWith (const CString &sString, const CString &sPartial)

//	strStartsWith
//
//	Returns TRUE if sString starts with sPartial

	{
	char *pString = (LPSTR)sString;
	char *pPartial = (LPSTR)sPartial;
	int iLen = sPartial.GetLength();

	//	Some edge conditions

	if (pPartial == NULL || iLen == 0)
		return true;
	else if (sString.GetLength() < iLen)
		return false;

	//	Match

	char *pPartialEnd = pPartial + iLen;
	while (pPartial < pPartialEnd)
		if (*pPartial++ != *pString++)
			return false;

	return true;
	}

bool strStartsWithNoCase (const CString &sString, const CString &sPartial)

//	strStartsWithNoCase
//
//	Case-insensitive, non-locale specific comparison.
//	NOTE: Some clients rely on the fact that we can compare strings even if they
//	have embedded 0s.

	{
	const char *pString = (LPSTR)sString;
	const char *pStringEnd = pString + sString.GetLength();
	const char *pPartial = (LPSTR)sPartial;
	const char *pPartialEnd = pPartial + sPartial.GetLength();

	//	Some edge conditions

	if (pPartial == NULL || pPartial == pPartialEnd)
		return true;

	//	Search

	while (pPartial < pPartialEnd && pString < pStringEnd)
		{
		UTF32 dwCodePoint1 = strParseUTF8Char(&pString, pStringEnd);
		UTF32 dwCodePoint2 = strParseUTF8Char(&pPartial, pPartialEnd);

		if (strToLowerChar(dwCodePoint1) != strToLowerChar(dwCodePoint2))
			return false;
		}

	//	Done

	return (pPartial == pPartialEnd);
	}

CString strSubString (const CString &sString, int iStart, int iLen)

//	strSubString
//
//	Returns a substring

	{
	char *pPos = (LPSTR)sString;
	int iStringLen = sString.GetLength();
	if (pPos == NULL || iStart >= iStringLen)
		return CString();

	if (iLen == -1)
		return CString(pPos + iStart, iStringLen - iStart);
	else
		return CString(pPos + iStart, Min(iLen, iStringLen - iStart));
	}

double strToDouble (const CString &sString)

//	strToDouble
//
//	Converts a string to double.

	{
	return atof((LPSTR)sString);
	}

CString strToLower (const CString &sString)

//	strToLower
//
//	Converts the string to lowercase

	{
	CMemoryBuffer Stream(4096);

	const char *pPos = (LPSTR)sString;
	const char *pEndPos = pPos + sString.GetLength();
	const char *pStart = pPos;
	while (pPos < pEndPos)
		{
		UTF32 dwCodePoint = strParseUTF8Char(&pPos, pEndPos);
		UTF32 dwLower = strToLowerChar(dwCodePoint);
		strEncodeUTF8Char(dwLower, Stream);
		}

	return CString(Stream.GetPointer(), Stream.GetLength());
	}

TArray<CString> strToLower (TArray<CString> &&List)

//	strToLower
//
//	Converts an array to lowercase

	{
	int i;

	for (i = 0; i < List.GetCount(); i++)
		List[i] = strToLower(List[i]);

	//	NOTE: move is required here since once an r-value is named, 
	//	the compiler can't assume that this is the only reference.
	//	See: http://stackoverflow.com/questions/13430831/return-by-rvalue-reference

	return std::move(List);
	}

CString strToSimilarMatch (const CString &sString)

//	strToSimilarMatch
//
//	Does the following conversions on the string (to generate a string that will
//	be equal to other similar string):
//
//	1.	Converted to lowercase
//	2.	Spaces removed

	{
	CString sConvert = strToLower(sString);
	CStringBuffer Buffer;

	char *pPos = (LPSTR)sConvert;
	char *pEndPos = pPos + sConvert.GetLength();
	while (pPos < pEndPos)
		{
		if (*pPos == ' ')
			;
		else
			Buffer.Write(pPos, 1);

		pPos++;
		}

	return CString::CreateFromHandoff(Buffer);
	}

bool strIsTitleCapitalWord (const CString &sWord)

//	strIsTitleCapitalWord
//
//	Returns TRUE if this is a word that we normally capitalize in titles. We
//	assume that sWord is always lowercase.

	{
	static CString TITLE_CAP_EXCEPTIONS_SHORT[] =
		{
		"a",
		"an",
		"and",
		"at",
		"by",
		"for",
		"in",
		"near",
		"not",
		"of",
		"on",
		"or",
		"the",
		"to",
		"under",
		"upon",
		"with",
		"without",
		};

	static int TITLE_CAP_EXCEPTIONS_SHORT_COUNT = sizeof(TITLE_CAP_EXCEPTIONS_SHORT) / sizeof(TITLE_CAP_EXCEPTIONS_SHORT[0]);

	if (sWord.GetLength() <= 4)
		{
		for (int i = 0; i < TITLE_CAP_EXCEPTIONS_SHORT_COUNT; i++)
			{
			if (strEquals(sWord, TITLE_CAP_EXCEPTIONS_SHORT[i]))
				return false;
			}
		}

	return true;
	}

CString strToTitleCase (const CString &sString)

//	strToTitleCase
//
//	Converts the string to title case.

	{
	//	First split into words and force to lowercase

	TArray<CString> Words;
	strSplit(sString, NULL_STR, &Words, -1, SSP_FLAG_WHITESPACE_SEPARATOR | SSP_FLAG_FORCE_LOWERCASE);

	//	Now join back, setting case appropriately.

	CStringBuffer Result;
	for (int i = 0; i < Words.GetCount(); i++)
		{
		bool bLastWord = (i == Words.GetCount() - 1);
		const char *pPos = Words[i].GetParsePointer();
		const char *pEndPos = pPos + Words[i].GetLength();

		//	Add a space, if necessary.

		if (i != 0)
			Result.WriteChar(' ');

		//	Should we capitalize this word?

		if (i == 0 || bLastWord || strIsTitleCapitalWord(Words[i]))
			{
			UTF32 dwCodePoint = strParseUTF8Char(&pPos, pEndPos);
			UTF32 dwNew = strToUpperChar(dwCodePoint);
			strEncodeUTF8Char(dwNew, Result);
			}

		//	Everything else is lowercase

		Result.Write(pPos, pEndPos - pPos);
		}

	return Result;
	}

CString strToUpper (const CString &sString)

//	strToUpper
//
//	Converts the string to UPPERCASE

	{
	CMemoryBuffer Stream(4096);

	const char *pPos = (LPSTR)sString;
	const char *pEndPos = pPos + sString.GetLength();
	const char *pStart = pPos;
	while (pPos < pEndPos)
		{
		UTF32 dwCodePoint = strParseUTF8Char(&pPos, pEndPos);
		UTF32 dwLower = strToUpperChar(dwCodePoint);
		strEncodeUTF8Char(dwLower, Stream);
		}

	return CString(Stream.GetPointer(), Stream.GetLength());
	}

//	Functions

int KeyCompare (const LPCSTR &pKey1, const LPCSTR &pKey2)

//	KeyCompare
//
//	Case-sensitive, non-locale specific comparison
//
//	0 if Key1 == Key2
//	1 if Key1 > Key2
//	-1 if Key1 < Key2

	{
	const char *pPos1 = pKey1;
	const char *pPos2 = pKey2;

	//	Handle NULL

	if (pPos1 == NULL)
		return ((pPos2 == NULL || *pPos2 == '\0') ? 0 : -1);
	else if (pPos2 == NULL)
		return ((*pPos1 == '\0') ? 0 : 1);

	//	Compare

	while (*pPos1 != '\0' && *pPos2 != '\0')
		{
		if (*pPos1 < *pPos2)
			return -1;
		else if (*pPos1 > *pPos2)
			return 1;
		else
			{
			pPos1++;
			pPos2++;
			}
		}

	if (*pPos1 == '\0' && *pPos2 == '\0')
		return 0;
	else if (*pPos1 == '\0')
		return -1;
	else
		return 1;
	}

int KeyCompareNoCase (const LPSTR &pKey1, int iKey1Len, const LPSTR &pKey2, int iKey2Len)

//	KeyCompareNoCase
//
//	Case-insensitive, non-locale specific comparison
//
//	0 if Key1 == Key2
//	1 if Key1 > Key2
//	-1 if Key1 < Key2

	{
	const char *pPos1 = pKey1;
	const char *pPos2 = pKey2;

	//	Handle NULL

	if (pPos1 == NULL)
		return ((pPos2 == NULL || *pPos2 == '\0') ? 0 : -1);
	else if (pPos2 == NULL)
		return ((*pPos1 == '\0') ? 0 : 1);

	//	Compare

	const char *pPos1End = pPos1 + iKey1Len;
	const char *pPos2End = pPos2 + iKey2Len;

	while (pPos1 < pPos1End && pPos2 < pPos2End)
		{
		UTF32 dwCodePoint1 = strToLowerChar(strParseUTF8Char(&pPos1, pPos1End));
		UTF32 dwCodePoint2 = strToLowerChar(strParseUTF8Char(&pPos2, pPos2End));

		if (dwCodePoint1 < dwCodePoint2)
			return -1;
		else if (dwCodePoint1 > dwCodePoint2)
			return 1;
		}

	if (pPos1 == pPos1End && pPos2 == pPos2End)
		return 0;
	else if (pPos1 == pPos1End)
		return -1;
	else
		return 1;
	}
