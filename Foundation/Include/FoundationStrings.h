//	FoundationStrings.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

#include <cstddef>

class CString16;
class CStringBuffer;
class IMemoryBlock;
struct SStringValidate;

//	Small a with acute: á
#define UTF8a1	"\xc3\xa1"

//	Small a with tilde: ã
#define UTF8a2	"\xc3\xa3"

class CString
	{
	public:
		CString (void) : m_pString(NULL) { }
		CString (const CString &sStr);
		CString (LPCSTR pStr);
		CString (LPCSTR pStr, int iLen);
		CString (LPCSTR pStr, DWORD iLen);
		CString (LPCSTR pStr, size_t iLen);
		CString (LPCSTR pStr, std::ptrdiff_t iLen);
		CString (LPSTR pStr, int iLen, bool bLiteral);
		CString (LPSTR pStr, DWORD iLen, bool bLiteral);
		CString (LPSTR pStr, size_t iLen, bool bLiteral);
		CString (LPSTR pStr, std::ptrdiff_t iLen, bool bLiteral);
		CString (LPTSTR pStr, int iLen);
		CString (LPTSTR pStr, DWORD iLen);
		CString (LPTSTR pStr, size_t iLen);
		CString (LPTSTR pStr, std::ptrdiff_t iLen);
		CString (const CString16 &sStr);
		CString (CString &&Src) noexcept { m_pString = Src.m_pString; Src.m_pString = NULL; }
		CString (CStringBuffer &&Src) noexcept;

		explicit CString (int iLen);
		explicit CString (DWORD iLen);
		explicit CString (size_t iLen);
		explicit CString (std::ptrdiff_t iLen);

		~CString (void);

		static CString CreateFromHandoff (CStringBuffer &Buffer);
		static CString Deserialize (IByteStream &Stream);
		static CString DeserializeJSON (IByteStream &Stream);

		operator LPSTR () const { return GetParsePointer(); }

		CString &operator= (LPSTR pStr);
		CString &operator= (const CString &sStr);
		CString &operator= (CString &&Src) noexcept { TakeHandoff(Src); return *this; }
		CString &operator+= (const CString &sStr);
		CString &operator+= (const IMemoryBlock& Value);
		CString operator + (const CString &sStr) const;

		int Find (char chChar, int iStartAt = 0) const;
		int GetLength (void) const;
		char *GetPointer (void) const { return m_pString; }
		char *GetParsePointer (void) const { return (m_pString ? m_pString : &BLANK_STRING[4]); }
		LPSTR Handoff (void) { LPSTR pTemp = m_pString; m_pString = NULL; return pTemp; }
		bool IsEmpty (void) const { return (GetLength() == 0); }
		bool IsLiteral (void) const { return (m_pString == NULL || GetLengthParameter() < 0); }
		bool IsWhitespace (void) const;
		void Serialize (IByteStream &Stream) const;
		void SerializeJSON (IByteStream &Stream) const;
		void SetLength (int iLength);
		void TakeHandoff (CString &sStr);
		void TakeHandoff (CStringBuffer &Buffer);
		void TakeHandoffLPSTR (LPSTR pStr) { CleanUp(); m_pString = pStr; }
		bool Validate (const SStringValidate &Options) const;

		static CString IncrementTrailingNumber (const CString &sStr);
		static int UTF8CharSize (const char *pStartPos, const char *pEndPos);

	private:
		CString (LPSTR pString, const char *pPrivate) { m_pString = pString; }

		void CleanUp (void);
		LPSTR CopyBuffer (void) const;
		static LPSTR CreateBufferFromUTF16 (LPTSTR pStr, int iLen);
		LPSTR GetBuffer (void) const { return (m_pString - sizeof(int)); }
		int GetBufferLength (void) const { return (sizeof(int) + GetLength() + 1); }
		int GetLengthParameter (void) const { return *((int *)(m_pString - sizeof(int))); }
		void Init (LPCSTR pStr, int iLen);
		void SetLengthParameter (int iLength) { *((int *)(m_pString - sizeof(int))) = iLength; }

		LPSTR m_pString;

		static char BLANK_STRING[];
	};

typedef DWORD UTF32;

//	Constant strings

template <int l> struct SConstString
	{
	int iLength;
	char pString[l];
	};

#define DECLARE_CONST_STRING(label,string)	\
	static SConstString<sizeof(string)> g_p##label = { 0-(int)(sizeof(string)-1), string }; \
	static const CString label (g_p##label##.pString, -1, true); 

extern const CString NULL_STR;

template <int l> class TFixedString
	{
	public:
		TFixedString (void) :
				m_iLength(0)
			{
			m_szString[0] = '\0';
			}

		operator CString () const { return (m_iLength == 0 ? NULL_STR : CString(m_szString, m_iLength, true)); }
		operator LPSTR () const { return m_szString; }

		TFixedString<l> &operator= (const CString &sStr)
			{
			LPSTR pSrc = sStr;
			LPSTR pDest = m_szString;
			int iLen = Min(sStr.GetLength(), l);
			LPSTR pSrcEnd = pSrc + iLen;

			while (pSrc < pSrcEnd)
				*pDest++ = *pSrc++;

			*pDest = '\0';
			m_iLength = -iLen;

			return *this;
			}

		int GetLength (void) const { return -m_iLength; }

	private:
		int m_iLength;
		char m_szString[l+1];
	};

//	Helpers

struct SStringValidate
	{
	CString sForbiddenChars;
	CString sForbiddenLeadingChars;
	int iMinPrintableChars = 0;
	bool bAllowTabs = false;
	bool bAllowCRLF = false;
	bool bAllowASCIICtrlChars = false;
	bool bAllowLeadingWhitespace = false;
	bool bAllowTrailingWhitespace = false;
	bool bAllowRepeatedWhitespace = false;
	};

//	UTF-16 strings

class CString16
	{
	public:
		CString16 (void) { }
		CString16 (LPCTSTR pStr, int iLen);
		CString16 (LPCTSTR pStr, size_t iLen = -1);
		CString16 (LPSTR pStr);
		CString16 (const CString &sStr);

		explicit CString16 (size_t iLen);

		~CString16 (void);

		CString16 &operator= (LPSTR pStr);
		CString16 &operator= (const CString &sStr);
		CString16 &operator= (const CString16 &sStr);
		operator LPTSTR () const { return m_pString; }

		static int CalcLength (LPCTSTR pStr);
		int GetLength (void) const { return CalcLength(m_pString); }
		bool IsASCII () const;
		bool IsEmpty (void) const { return GetLength() == 0; }

	private:
		static LPTSTR CreateUTF16BufferFromUTF8 (LPSTR pStr, int iLen);

		LPTSTR m_pString = NULL;
	};

//	String functions

bool strASCIICharInSet (char *pPos, const CString &sSet);
CString strCapitalize (const CString &sString);

constexpr DWORD STC_FLAG_MUST_HAVE_ALPHA =			0x00000001;
constexpr DWORD STC_FLAG_MUST_HAVE_ALPHANUMERIC =	0x00000002;
constexpr DWORD STC_FLAG_NO_SPACES =				0x00000004;
CString strClean (const CString &sText, DWORD dwFlags = 0);

bool strEndsWith (const CString &sString, const CString &sPartial);
bool strEndsWithNoCase (const CString &sString, const CString &sPartial);
bool strEquals (const CString &sKey1, const CString &sKey2);
bool strEquals (const char *pKey1, const char *pKey2);
bool strEqualsNoCase (const CString &sKey1, const CString &sKey2);
CString strEscapePrintf (const CString &sString);
int strFind (const CString &sString, const CString &sStringToFind);
int strFindNoCase (const CString &sString, const CString &sStringToFind);

const DWORD FORMAT_LEADING_ZERO =			0x00000001;
const DWORD FORMAT_THOUSAND_SEPARATOR =		0x00000002;
const DWORD FORMAT_UNSIGNED =				0x00000004;
CString strFormatInteger (int iValue, int iMinFieldWidth = -1, DWORD dwFlags = 0);

CString strFromDouble (double rValue, int iDecimals = 17);
CString strFromInt (int iInteger, bool bSigned = true);
CString strFromIntOfBase (int iInteger, int iBase, bool bSigned = true, bool bUppercase = false);
inline bool strIsASCIIAlpha (const char *pPos) { return (*pPos >= 'a' && *pPos <= 'z') || (*pPos >= 'A' && *pPos <= 'Z'); }
inline bool strIsASCIIControl (const char *pPos) { return ((BYTE)*pPos <= (BYTE)0x1f) || *pPos == 0x7f; }
inline bool strIsASCIIHigh (const char *pPos) { return ((BYTE)*pPos >= (BYTE)0x80); }
bool strIsASCIISymbol (char *pPos);
inline bool strIsDigit (const char *pPos) { return (*pPos >= '0' && *pPos <= '9'); }
bool strIsInt (const CString &sString, int *retiValue = NULL);
bool strIsTitleCapitalWord (const CString &sWord);
inline bool strIsWhitespace (char chChar) { return (chChar == ' ' || chChar == '\t' || chChar == '\r' || chChar == '\n'); }
inline bool strIsWhitespace (const char *pPos) { return strIsWhitespace(*pPos); }
int strLength (LPCSTR pStr);
CString strOrdinal (int iOrdinal);
bool strOverflowsInteger32 (const CString &sValue);
double strParseDouble (const char *pStart, double rNullResult, const char **retpEnd = NULL, bool *retbNullValue = NULL);
int strParseHexChar (char chChar, int iNullResult = 0);
int strParseInt (const char *pStart, int iNullResult = 0, const char **retpEnd = NULL, bool *retbNullValue = NULL);
int strParseIntOfBase (const char *pStart, int iBase, int iNullResult = 0, const char **retpEnd = NULL, bool *retbNullValue = NULL);
CString strRepeat (char chChar, int iCount);
bool strStartsWith (const CString &sString, const CString &sPartial);
bool strStartsWithNoCase (const CString &sString, const CString &sPartial);
CString strSubString (const CString &sString, int iStart, int iLen = -1);
double strToDouble (const CString &sString);
inline int strToInt (const CString &sString, int iFailResult = 0, bool *retbFailed = NULL) { return strParseInt(sString, iFailResult, NULL, retbFailed); }
CString strToLower (const CString &sString);
CString strToSimilarMatch (const CString &sString);
CString strToTitleCase (const CString &sString);
CString strToUpper (const CString &sString);

CString strEncodeUTF8Char (UTF32 dwCodePoint);
void strEncodeUTF8Char (UTF32 dwCodePoint, IByteStream &Stream);
void strEncodeUTF8Char (UTF32 dwCodePoint, char *pPos, char *pPosEnd);
inline int strGetUTF8EncodeLength (UTF32 dwCodePoint) { return (dwCodePoint <= 0x007f ? 1 : (dwCodePoint <= 0x07ff ? 2 : (dwCodePoint <= 0xffff ? 3 : 4))); }
inline bool strIsAlphaNumeric (UTF32 dwCodePoint) { return (::IsCharAlphaNumericW(LOWORD(dwCodePoint)) ? true : false); }
inline bool strIsASCII (UTF32 dwCodePoint) { return (dwCodePoint < 0x7f); }
bool strIsPrintableChar (UTF32 dwCodePoint);
UTF32 strParseUTF8Char (const char **iopPos, const char *pEndPos);
UTF32 strToLowerChar (UTF32 dwCodePoint);
UTF32 strToUpperChar (UTF32 dwCodePoint);

inline bool strIsASCIIAlphaNumeric (const char *pPos) { return (strIsASCIIAlpha(pPos) || strIsDigit(pPos)); }

//	Pattern functions

typedef void (*PATTERNHOOKPROC)(LPVOID pCtx, LPCSTR &pPos, LPVOID *&pArgs, IByteStream *pStream);

CString strPattern (LPCSTR pLine, ...);
CString strPatternEx (LPCSTR pLine, PATTERNHOOKPROC pfHook, LPVOID pCtx, ...);

