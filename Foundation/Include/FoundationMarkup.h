//	FoundationMarkup.h
//
//	Foundation header file
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IHexeTextExtensions
	{
	public:

		virtual ~IHexeTextExtensions (void) { }

		virtual CString ProcessLink (const CString &sLink, const TArray<CString> &Params) { return sLink; }
		virtual CString ProcessTemplate (const CString &sTemplate, const TSortMap<CString, CString> &Params) { return sTemplate; }
	};

class CTextMarkupParser
	{
	public:
		enum EExtensionTypes
			{
			typeNone,
			typeLink,
			typeTemplate,
			};

		struct SExtensionDesc
			{
			SExtensionDesc (void) : iType(typeNone) { }

			EExtensionTypes iType;
			CString sName;
			TSortMap<CString, CString> Params;
			};

		enum ETokens
			{
			tokenNone,
			tokenError,
			tokenEoS,

			tokenText,
			tokenLineEnd,
			tokenEntity,
			tokenEscapeText,
			tokenURL,

			tokenSingleQuote,
			tokenDoubleQuote,
			tokenSlash,
			tokenStar,
			tokenHash,
			tokenEquals,
			tokenOpenBracket,
			tokenCloseBracket,
			tokenOpenBrace,
			tokenCloseBrace,
			tokenLessThan,
			tokenGreaterThan,
			tokenDash,
			tokenUnderscore,
			tokenBackslash,
			tokenVerticalBar,
			tokenTilde,
			tokenBackQuote,
			};

		CTextMarkupParser (void);

		inline ETokens GetToken (void) const { return m_iToken; }
		char GetTokenChar (void) const;
		inline int GetTokenCount (void) const { return m_iTokenCount; }
		inline const CString &GetTokenString (void) const { return m_sToken; }
		void ParseExtensions (TArray<SExtensionDesc> *retExtensions);
		ETokens ParseNextToken (void);
		inline char PeekNextChar (void) const { return (m_pPos < m_pPosEnd ? *m_pPos : '\0'); }
		ETokens PeekNextToken (CString *retsToken = NULL, int *retiTokenCount = NULL);
		void RestoreParser (CTextMarkupParser &Saved);
		void SaveParser (CTextMarkupParser *retSaved);
		void SetInput (char *pPos, char *pPosEnd);

		static bool IsBasicLink (char *pPos, char *pPosEnd);
		static bool IsEntity (char *pPos, char *pPosEnd);

	private:
		bool IsSpecialChar (char chChar);
		ETokens TokenFromSpecialChar (char chChar);

		char *m_pPos;
		char *m_pPosEnd;

		ETokens m_iToken;
		int m_iTokenCount;
		CString m_sToken;
	};

class CHexeTextProcessor
	{
	public:
		CHexeTextProcessor (void);

		bool ConvertToHTML (const IMemoryBlock &Input, IByteStream &Output, CString *retsError);
		inline void SetExtensions (IHexeTextExtensions *pExtensions) { m_pExtensions = pExtensions; }
		inline void SetValues (const TArray<CString> &Values) { m_pValues = &Values; }

	private:
		enum EListTypes
			{
			listOrdered,
			listUnordered,
			};

		void BlockQuoteToHTML (int iLevel);
		inline void CloseAllStyles (void) { if (m_bInLink) CloseStyles(m_InLinkStyleStack); else CloseStyles(m_StyleStack); }
		void CloseStyles (TArray<CTextMarkupParser::ETokens> &StyleStack);
		void CodeToHTML (int iIndentLevel = 0);
		bool FindTextEnd (CTextMarkupParser::ETokens iToken);
		void HeaderToHTML (void);
		bool IsHeaderEnd (void);
		inline bool IsStyleInStack (CTextMarkupParser::ETokens iToken) { if (m_bInLink) return m_InLinkStyleStack.Find(iToken); else return m_StyleStack.Find(iToken); }
		inline bool IsTopStyle (CTextMarkupParser::ETokens iToken) { if (m_bInLink) return (m_InLinkStyleStack.GetCount() > 0 && m_InLinkStyleStack[m_InLinkStyleStack.GetCount() - 1] == iToken); else return (m_StyleStack.GetCount() > 0 && m_StyleStack[m_StyleStack.GetCount() - 1] == iToken); }
		void LinkToHTML (void);
		void ListToHTML (EListTypes iListType, int iItemLevel);
		void OutputRepeatingChar (char chChar, int iCount);
		void ParagraphToHTML (void);
		inline void PopStyle (void) { if (m_bInLink) m_InLinkStyleStack.Delete(m_InLinkStyleStack.GetCount() - 1); else m_StyleStack.Delete(m_StyleStack.GetCount() - 1); }
		inline void PushStyle (CTextMarkupParser::ETokens iToken) { if (m_bInLink) m_InLinkStyleStack.Insert(iToken); else m_StyleStack.Insert(iToken); }
		void TemplateToHTML (void);
		void TextToHTML (bool bSkipWhitespace = false, char chTerminator = '\0');

		CTextMarkupParser m_Parser;
		IByteStream *m_pOutput;
		TArray<CTextMarkupParser::ETokens> m_StyleStack;
		int m_iCurListLevel;				//	Current list level
		int m_iCurBlockQuoteLevel;			//	Block quote level
		IHexeTextExtensions *m_pExtensions;
		const TArray<CString> *m_pValues;
		int m_iCurValue;					//	Current extension value

		bool m_bInLink;
		TArray<CTextMarkupParser::ETokens> m_InLinkStyleStack;

		bool m_bOLTemplate;					//	If TRUE, we've outputted the special # template
		bool m_bULTemplate;					//	If TRUE, we've outputted the special * template
	};
