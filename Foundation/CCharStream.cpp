//	CCharStream.cpp
//
//	CCharStream class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

CCharStream::CCharStream (void) : 
		m_pStream(NULL), 
		m_iStreamLeft(0),
		m_chChar('\0'), 
		m_iLine(0)

//	CCharStream constructor

	{
	}

CString CCharStream::ComposeError (const CString &sError)

//	ComposeError
//
//	Composes an error string at the current parse location

	{
	return strPattern("(%d) %s", m_iLine, sError);
	}

bool CCharStream::Init (IByteStream &Stream, int iStartingLine)

//	Init
//
//	Initializes the character stream

	{
	m_pStream = &Stream;
	m_iStreamLeft = Stream.GetStreamLength() - Stream.GetPos();

	m_iLine = iStartingLine;
	ReadChar();

	return true;
	}

bool CCharStream::ParseQuotedString (DWORD dwFlags, CString *retsString)

//	ParseQuotedString
//
//	Parses a quoted UTF8 string with the following escape codes:
//
//	\"
//	\\
//	\/
//	\b
//	\f
//	\n
//	\r
//	\t
//	\uXXXX
//
//	We start with m_chChar at the first quote and end with m_chChar
//	at the first character after the end quote.
//
//	Returns FALSE if there is an invalid character

	{
	CStringBuffer String;

	//	Skip the open quote

	ReadChar();

	//	Keep looping

	while (m_chChar != '\"' && m_chChar != '\0')
		{
		if (m_chChar == '\\')
			{
			ReadChar();
			switch (m_chChar)
				{
				case '\"':
					String.Write("\"", 1);
					break;

				case '\\':
					String.Write("\\", 1);
					break;

				case '/':
					String.Write("/", 1);
					break;

				case 'b':
					String.Write("\b", 1);
					break;

				case 'f':
					String.Write("\f", 1);
					break;

				case 'n':
					String.Write("\n", 1);
					break;

				case 'r':
					String.Write("\r", 1);
					break;

				case 't':
					String.Write("\t", 1);
					break;

				case 'u':
					{
					char szBuffer[5];
					szBuffer[0] = ReadChar();
					szBuffer[1] = ReadChar();
					szBuffer[2] = ReadChar();
					szBuffer[3] = ReadChar();
					szBuffer[4] = '\0';

					DWORD dwHex = (DWORD)strParseIntOfBase(szBuffer, 16, (int)'?');
					CString sChar = strEncodeUTF8Char((UTF32)dwHex);
					String.Write(sChar);
					break;
					}

				default:
					return false;
				}
			}
		else if (strIsWhitespace(&m_chChar))
			String.Write(&m_chChar, 1);
		else if (strIsASCIIControl(&m_chChar))
			return false;
		else
			String.Write(&m_chChar, 1);

		ReadChar();
		}

	//	If we hit the end, then we have an error (unterminated string).

	if (m_chChar == '\0')
		return false;

	//	Otherwise, read the next char (skipping the end quote)

	ReadChar();

	//	Done

	if (retsString)
		retsString->TakeHandoff(String);

	return true;
	}

void CCharStream::ParseToEndOfLine (CString *retsLine)

//	ParseToEndOfLine
//
//	Parses to the end of the line and returns the line (excluding the end-of-line)
//	characters. Leaves m_chChar at the first character of the next line.

	{
	//	Optimize the case where we don't care about the line

	if (retsLine == NULL)
		{
		while (m_chChar != '\r' && m_chChar != '\n' && m_chChar != '\0')
			ReadChar();
		}

	//	Parse until the end of line

	else
		{
		CStringBuffer Line;

		//	Copy the line

		while (m_chChar != '\r' && m_chChar != '\n' && m_chChar != '\0')
			{
			Line.Write(&m_chChar, 1);
			ReadChar();
			}

		//	Done

		retsLine->TakeHandoff(Line);
		}

	//	Skip the end of line if there are double characters

	if (m_chChar == '\r')
		{
		if (ReadChar() == '\n')
			ReadChar();
		}
	else if (m_chChar == '\n')
		{
		if (ReadChar() == 'r')
			ReadChar();
		}
	}

void CCharStream::ParseWhitespace (void)

//	ParseWhitespace
//
//	Consumes whitespace and leaves m_chChar at the first non-whitespace character.

	{
	while (m_chChar == ' ' || m_chChar == '\t' || m_chChar == '\r' || m_chChar == '\n')
		ReadChar();
	}

char CCharStream::ReadChar (void)

//	ReadChar
//
//	Read a character from the stream
	
	{
	//	If we're done, nothing

	if (m_iStreamLeft == 0)
		m_chChar = '\0';
	else
		{
		m_chChar = m_pStream->ReadChar();
		m_iStreamLeft--;

		if (m_chChar == '\n')
			m_iLine++;
		}
	
	return m_chChar;
	}

void CCharStream::RefreshStream (void)

//	RefreshStream
//
//	Reinitializes the stream

	{
	m_iStreamLeft = m_pStream->GetStreamLength() - m_pStream->GetPos();
	}

void CCharStream::UnreadChar (int iChars)

//	UnreadChar
//
//	Put a character back on the stream

	{
	if (iChars > 0)
		{
		int iNewPos = Max(0, m_pStream->GetPos() - iChars);
		m_pStream->Seek(iNewPos);
		m_iStreamLeft += iChars;
		m_chChar = '\0';
		}
	}
