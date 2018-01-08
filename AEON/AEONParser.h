//	AEONParser.h
//
//	AEON parse class
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

#pragma once

class CAEONScriptParser
	{
	public:
		enum ETokens
			{
			tkEOF,
			tkError,
			tkDatum,
			tkCloseParen,
			tkCloseBrace,
			tkColon,
			tkComma,
			};

		CAEONScriptParser (IByteStream &Stream) : m_Stream(Stream) { m_chChar = ReadChar(); }
		bool ParseDatum (CDatum *retDatum);
		ETokens ParseToken (CDatum *retDatum);

		static bool HasSpecialChars (const CString &sString);

	private:
		ETokens ParseArray (CDatum *retDatum);
		ETokens ParseDateTime (CDatum *retDatum);
		ETokens ParseInteger (int *retiValue);
		bool ParseComment (void);
		ETokens ParseLiteral (CDatum *retDatum);
		ETokens ParseNumber (CDatum *retDatum);
		ETokens ParseString (CDatum *retDatum);
		ETokens ParseStruct (CDatum *retDatum);
		char ReadChar (void);

		IByteStream &m_Stream;
		char m_chChar;
	};
