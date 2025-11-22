//	GridLangParser.h
//
//	GridLang Internals
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#pragma once

enum class EGridLangToken
	{
	Null,
	Identifier,					//	abc
	String,						//	"foo"
	Integer,					//	123
	HexInteger,					//	0x123
	Real,						//	123.4

	OpenParen,					//	(
	CloseParen,					//	)
	OpenBrace,					//	{
	CloseBrace,					//	}
	OpenBracket,				//	[
	CloseBracket,				//	]

	SemiColon,					//	;
	Colon,						//	:
	Dot,						//	.
	Comma,						//	,
	SingleQuote,				//	'

	DoubleDot,					//	..
	TripleDot,					//	...

	Plus,						//	+
	Minus,						//	-
	Star,						//	*
	Slash,						//	/
	PlusEquals,					//	+=
	MinusEquals,				//	-=
	StarEquals,					//	*=
	SlashEquals,				//	/=
	PercentEquals,				//	%=
	CaretEquals,				//	^=
	AmpEquals,					//	&=
	BackSlash,					//	backslash
	Equals,						//	=
	EqualEquals,				//	==
	LessThan,					//	<
	LessThanEquals,				//	<=
	GreaterThan,				//	>
	GreaterThanEquals,			//	>=
	NotEquals,					//	!=
	EqualEqualEquals,			//	===
	NotEqualEquals,				//	!==
	And,						//	&&
	Or,							//	||
	Arrow,						//	->

	Bang,						//	!
	At,							//	@
	Hash,						//	#
	Dollar,						//	$
	Percent,					//	%
	Caret,						//	^
	Amp,						//	&
	Question,					//	?
	Bar,						//	|
	Tilde,						//	~
	BackQuote,					//	`

	OpenBangBlock,				//	<!
	CloseBangBlock,				//	!>

	LeftLongArrow,				//	<--
	RightLongArrow,				//	-->
	Line,						//	-----

	Error,
	};

class CGridLangParser
	{
	public:

		struct SToken
			{
			EGridLangToken iToken;
			CString sToken;
			};

		struct SPos
			{
			CString sSourceFile;
			int iLine = 0;
			int iChar = 1;
			};

		CGridLangParser (const CString& sSourceFilename, const IMemoryBlock &Stream) :
				m_sSourceFile(sSourceFilename),
				m_pPos(Stream.GetPointer()),
				m_pPosEnd(Stream.GetPointer() + Stream.GetLength()),
				m_pLineStart(Stream.GetPointer())
			{ }

		CString ComposeError (const CString &sError) const;
		SPos GetPos () const { return { m_sSourceFile, m_iLine, m_iChar }; }
		EGridLangToken GetToken () const { return m_iToken; }
		const CString &GetTokenValue () const { return m_sTokenValue; }
		bool HasToken () const;
		EGridLangToken NextToken ();
		EGridLangToken PeekToken (CString *retsValue = NULL) const;
		TArray<SToken> PeekTokens (int iCount = 1) const;

	private:

		static constexpr int DEFAULT_TAB_SIZE = 4;

		CGridLangParser () { }

		int CalcCurLineIndent () const;
		int GetCharPos () const { return (int)(m_pPos - m_pLineStart) + 1; }
		static CString ParseBasicString (const CString& sValue);
		static CString ParseBlockString (const CString& sValue, int deIndentColumns);
		EGridLangToken SetToken (EGridLangToken iToken, const CString &sToken = NULL_STR);

		const char *m_pPos = NULL;
		const char *m_pPosEnd = NULL;
		CString m_sSourceFile;					//	Source file name
		int m_iLine = 1;						//	Current line number
		int m_iChar = 1;						//	Current character number
		const char *m_pLineStart = NULL;		//	Start of current line.
		EGridLangToken m_iToken = EGridLangToken::Null;
		CString m_sTokenValue;
	};
