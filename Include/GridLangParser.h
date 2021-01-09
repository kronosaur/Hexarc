//	GridLangParser.h
//
//	GridLang Internals
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

enum class EGridLangToken
	{
	Null,
	Identifier,
	String,
	Integer,
	HexInteger,
	Real,

	OpenParen,
	CloseParen,
	OpenBrace,
	CloseBrace,
	OpenBracket,
	CloseBracket,

	SemiColon,
	Colon,
	Dot,
	Comma,
	SingleQuote,

	Plus,
	Minus,
	Star,
	Slash,
	BackSlash,
	Equals,
	EqualEquals,
	LessThan,
	LessThanEquals,
	GreaterThan,
	GreaterThanEquals,
	NotEquals,
	And,
	Or,

	Bang,
	At,
	Hash,
	Dollar,
	Percent,
	Caret,
	Amp,
	Question,
	Bar,
	Tilde,
	BackQuote,

	Error,
	};

class CGridLangParser
	{
	public:
		CGridLangParser (IMemoryBlock &Stream) :
				m_pPos(Stream.GetPointer()),
				m_pPosEnd(Stream.GetPointer() + Stream.GetLength())
			{ }

		CString ComposeError (const CString &sError) const;
		EGridLangToken GetToken () const { return m_iToken; }
		const CString &GetTokenValue () const { return m_sTokenValue; }
		bool HasToken () const;
		EGridLangToken NextToken ();
		EGridLangToken PeekToken (CString *retsValue = NULL) const;

	private:
		CGridLangParser () { }

		EGridLangToken SetToken (EGridLangToken iToken, const CString &sToken = NULL_STR);

		const char *m_pPos = NULL;
		const char *m_pPosEnd = NULL;
		int m_iLine = 1;						//	Current line number
		EGridLangToken m_iToken = EGridLangToken::Null;
		CString m_sTokenValue;
	};
