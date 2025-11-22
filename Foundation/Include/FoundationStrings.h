//	FoundationStrings.h
//
//	Foundation header file
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

#include <cstddef>

class CString;
class CString16;
class CStringBuffer;
class CStringView;
class IMemoryBlock;
struct SStringValidate;

//	Small a with acute: á
#define UTF8a1	"\xc3\xa1"

//	Small a with tilde: ã
#define UTF8a2	"\xc3\xa3"

typedef DWORD UTF32;

class CStringSlice
	{
	public:
		CStringSlice () { }

		CStringSlice (const char* pPos, int iLength) :
				m_pStr(pPos),
				m_iLength(iLength)
			{ }

		CStringSlice (const char* pPos, size_t iLength) :
				m_pStr(pPos),
				m_iLength((int)iLength)
			{ }

		CStringSlice (const char* pPos, std::ptrdiff_t iLength) :
				m_pStr(pPos),
				m_iLength((int)iLength)
			{ }

		operator LPCSTR () const { return GetPointer(); }

		int GetLength () const { return m_iLength; }
		const char* GetPointer () const { return m_pStr; }
		bool IsEmpty () const { return (m_iLength == 0); }

	private:
		const char* m_pStr = "";
		int m_iLength = 0;
	};

class CStringView
	{
	public:

		enum class ECharClass 
			{
			Null = 0,				//	Null character

			Normal,					//	Normal character
			Whitespace,				//	Whitespace character
			NonPrintable,			//	Non-printable character
			};

		inline CStringView ();

		static CStringView FromCStringPtr (LPCSTR pStr) { return CStringView(pStr); }

		inline operator const CString & () const;
		operator LPCSTR () const { return GetParsePointer(); }
		operator CStringSlice () const { return CStringSlice(GetParsePointer(), GetLength()); }

		inline CString Clean (DWORD dwFlags = 0) const;
		LPCSTR GetParsePointer (void) const { return (m_pString ? m_pString : &BLANK_STRING[4]); }
		LPCSTR GetPointer (void) const { return m_pString; }
		int GetLength () const { return (m_pString ? Abs(GetLengthParameter()) : 0); }
		bool IsEmpty () const { return (GetLength() == 0); }
		bool IsLiteral (void) const { return (m_pString == NULL || GetLengthParameter() < 0); }
		void Serialize (IByteStream &Stream) const;
		void SerializeJSON (IByteStream& Stream) const { SerializeJSON(Stream, m_pString, GetLength()); }

		static ECharClass GetCharClass (UTF32 dwCodePoint);
		static void SerializeJSON (IByteStream& Stream, LPCSTR pStr, int iLength);

	private:

		CStringView (LPCSTR pString) : m_pString(pString) { }
		int GetLengthParameter (void) const { return *((int *)(m_pString - sizeof(int))); }

		LPCSTR m_pString;

		static const char BLANK_STRING[];
	};

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
		CString (LPCSTR pStr, int iLen, bool bLiteral);
		CString (LPCSTR pStr, DWORD iLen, bool bLiteral);
		CString (LPCSTR pStr, size_t iLen, bool bLiteral);
		CString (LPCSTR pStr, std::ptrdiff_t iLen, bool bLiteral);
		CString (LPTSTR pStr, int iLen);
		CString (LPTSTR pStr, DWORD iLen);
		CString (LPTSTR pStr, size_t iLen);
		CString (LPTSTR pStr, std::ptrdiff_t iLen);
		CString (const CString16 &sStr);
		CString (CStringView Src);
		CString (CString &&Src) noexcept { m_pString = Src.m_pString; Src.m_pString = NULL; }
		CString (CStringBuffer &&Src) noexcept;

		explicit CString (int iLen);
		explicit CString (DWORD iLen);
		explicit CString (size_t iLen);
		explicit CString (std::ptrdiff_t iLen);

		~CString (void);

		static CString Concatenate (const CString &sStr1, const CString &sStr2);
		static CString CreateFromHandoff (CStringBuffer &Buffer);
		static CString Deserialize (IByteStream &Stream);
		static CString DeserializeJSON (IByteStream &Stream);

		operator LPSTR () const { return GetParsePointer(); }
		operator CStringView () const { return CStringView::FromCStringPtr(m_pString); }
		operator CStringSlice () const { return CStringSlice(GetParsePointer(), GetLength()); }

		CString &operator= (LPSTR pStr);
		CString &operator= (const CString &sStr);
		CString &operator= (CString &&Src) noexcept { TakeHandoff(Src); return *this; }
		CString &operator+= (const CString &sStr);
		CString &operator+= (const IMemoryBlock& Value);
		CString &operator+= (const CStringSlice& Src);
		CString operator + (const CString &sStr) const;

		CString AsBase64 () const;
		CStringView AsView () const { return CStringView::FromCStringPtr(m_pString); }
		size_t CalcSerializeSize () const { return GetLength() + sizeof(DWORD) + 1; }
		CString Clean (DWORD dwFlags = 0) const { return Clean(GetParsePointer(), GetLength(), dwFlags); }
		CString DecodedBase64 () const;
		int Find (char chChar, int iStartAt = 0) const;
		int GetLength (void) const;
		char *GetPointer (void) const { return m_pString; }
		char *GetParsePointer (void) const { return (m_pString ? m_pString : &BLANK_STRING[4]); }
		LPSTR Handoff (void) { LPSTR pTemp = m_pString; m_pString = NULL; return pTemp; }
		bool IsEmpty (void) const { return (GetLength() == 0); }
		bool IsLiteral (void) const { return (m_pString == NULL || GetLengthParameter() < 0); }
		bool IsWhitespace (void) const;
		void Serialize (IByteStream& Stream) const;
		void SerializeJSON (IByteStream &Stream) const { CStringView::SerializeJSON(Stream, m_pString, GetLength()); }
		void SetLength (int iLength);
		void SetLength (size_t iLength) { return SetLength((int)iLength); }
		void SetLength (std::ptrdiff_t iLength) { return SetLength((int)iLength); }
		CString StripChar (char chChar) const;
		void TakeHandoff (CString &sStr);
		void TakeHandoff (CStringBuffer &Buffer);
		void TakeHandoffLPSTR (LPSTR pStr) { CleanUp(); m_pString = pStr; }
		bool Validate (const SStringValidate &Options) const;

		static CString Clean (LPCSTR pStr, int iLength, DWORD dwFlags = 0);
		static CString EncodeBase36 (DWORDLONG dwValue);
		static CString EncodeBase64 (const BYTE* pData, int iLength, DWORD dwFlags = 0);
		static CString EncodeHex (const BYTE* pData, int iLength, DWORD dwFlags = 0);
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

extern const CString NULL_STR;

//	Constant strings

template <int l> struct SConstString
	{
	int iLength;
	char pString[l];
	};

#define DECLARE_CONST_STRING(label,string)	\
	static SConstString<sizeof(string)> g_p##label = { 0-(int)(sizeof(string)-1), string }; \
	static const CString label (g_p##label##.pString, -1, true); 

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

inline char strEncodeHexDigit (BYTE byValue)
	{
	if (byValue >= 0 && byValue <= 9)
		return '0' + byValue;
	else if (byValue >= 10 && byValue <= 15)
		return 'A' + (byValue - 10);
	else
		return '?';
	}

bool strEndsWith (const CString &sString, const CString &sPartial);
bool strEndsWithNoCase (const CString &sString, const CString &sPartial);
bool strEquals (const CString &sKey1, const CString &sKey2);
bool strEquals (const char *pKey1, const char *pKey2);
inline bool strEquals (CStringView sKey1, CStringView sKey2) { return strEquals((const CString&)sKey1, (const CString&)sKey2); }
inline bool strEquals (CStringView sKey1, const CString& sKey2) { return strEquals((const CString&)sKey1, sKey2); }
inline bool strEquals (const CString& sKey1, CStringView sKey2) { return strEquals(sKey1, (const CString&)sKey2); }
bool strEqualsNoCase (const char* pKey1, int iKey1Len, const char* pKey2, int iKey2Len);
inline bool strEqualsNoCase (CStringSlice sKey1, CStringSlice sKey2) { return strEqualsNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
inline bool strEqualsNoCase (const CString& sKey1, const CString& sKey2) { return strEqualsNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
inline bool strEqualsNoCase (CStringView sKey1, CStringView sKey2) { return strEqualsNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
inline bool strEqualsNoCase (CStringView sKey1, const CString& sKey2) { return strEqualsNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
inline bool strEqualsNoCase (const CString& sKey1, CStringView sKey2) { return strEqualsNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
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
bool strIsASCIISymbol (const char *pPos);
inline bool strIsDigit (const char *pPos) { return (*pPos >= '0' && *pPos <= '9'); }
inline bool strIsHexDigit (const char *pPos) { return ((*pPos >= '0' && *pPos <= '9') || (*pPos >= 'a' && *pPos <= 'f') || (*pPos >= 'A' && *pPos <= 'F')); }
bool strIsHexNumber (const char* pPos, int iLength);
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
bool strStartsWithNoCase (CStringSlice sString, CStringSlice sPartial);
CString strSubString (CStringSlice sString, int iStart, int iLen = -1);
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
bool strIsWhitespaceChar (UTF32 dwCodePoint);
UTF32 strParseUTF8Char (const char **iopPos, const char *pEndPos);
UTF32 strToLowerChar (UTF32 dwCodePoint);
UTF32 strToUpperChar (UTF32 dwCodePoint);

inline bool strIsASCIIAlphaNumeric (const char *pPos) { return (strIsASCIIAlpha(pPos) || strIsDigit(pPos)); }

//	Inlines

inline CStringView::CStringView () : m_pString(NULL) { }
inline CStringView::operator const CString & () const { return *(const CString *)&m_pString; }
inline CString CStringView::Clean (DWORD dwFlags) const { return CString::Clean(GetParsePointer(), GetLength(), dwFlags); }

//	Pattern functions

typedef void (*PATTERNHOOKPROC)(LPVOID pCtx, LPCSTR &pPos, LPVOID *&pArgs, IByteStream *pStream);

CString strPattern (LPCSTR pLine, ...);
CString strPatternEx (LPCSTR pLine, PATTERNHOOKPROC pfHook, LPVOID pCtx, ...);

