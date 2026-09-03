//	FoundationParsers.h
//
//	Foundation header file
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CCSVParser
	{
	public:
		CCSVParser (IByteStream64 &Stream) : m_Stream(Stream), m_chCur(Stream.ReadChar()) { }

		static ECharSetType DetectCharSet (IByteStream64 &Stream);
		const TArray<CString> &GetHeader (void) const { return m_Header; }
		bool HasMore (void) const { return (GetCurChar() != '\0'); }
		bool ParseHeader (CString *retsError) { return ParseRow(m_Header, retsError); }
		bool ParseRow (TArray<CString> *retRow = NULL, CString *retsError = NULL);
		bool ParseRow (TArray<CString> &Row, CString *retsError = NULL)
			{ return ParseRow(&Row, retsError); }
		void Reset () { m_Stream.Seek(0); m_chCur = m_Stream.ReadChar(); m_bAtStart = true; }
		void SetCharSet (ECharSetType iCharSet) { m_iCharSet = iCharSet; }
		void SetDelimiter (const char chDelimiter) { m_chDelimiter = chDelimiter; }

	private:
		bool AppendValue (CBuffer &Value, TArray<CString> *retRow, CString *retsError);
		char GetCurChar (void) const { return m_chCur; }
		char GetNextChar (void) { m_chCur = m_Stream.ReadChar(); return m_chCur; }
		void ParseBOM (void);

		IByteStream64 &m_Stream;
		ECharSetType m_iCharSet = ECharSetType::UTF8;
		TArray<CString> m_Header;

		bool m_bAtStart = true;
		char m_chCur = '\0';
		char m_chDelimiter = ',';
	};
