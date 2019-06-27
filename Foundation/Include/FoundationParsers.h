//	FoundationParsers.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CCSVParser
	{
	public:
		CCSVParser (IByteStream64 &Stream) : m_Stream(Stream), m_chCur(Stream.ReadChar()) { }

		const TArray<CString> &GetHeader (void) const { return m_Header; }
		bool HasMore (void) const { return (GetCurChar() != '\0'); }
		bool ParseHeader (CString *retsError) { return ParseRow(m_Header, retsError); }
		bool ParseRow (TArray<CString> &Row, CString *retsError = NULL);
		void SetUTF8Format (void) { m_iFormat = formatUTF8; }

	private:
		enum EFormat
			{
			formatUnknown,
			formatError,

			formatNone,
			formatUTF8,
			formatUTF16_BigEndian,
			formatUTF16_LittleEndian,
			};

		char GetCurChar (void) const { return m_chCur; }
		char GetNextChar (void) { m_chCur = m_Stream.ReadChar(); return m_chCur; }
		EFormat ParseBOM (void);
		void ParseToOpenQuote (void);

		IByteStream64 &m_Stream;
		EFormat m_iFormat = formatUnknown;
		TArray<CString> m_Header;

		char m_chCur;
	};
