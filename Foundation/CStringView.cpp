//	CStringView.cpp
//
//	CStringView class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CStringView::Serialize (IByteStream& Stream) const

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

void CStringView::SerializeJSON (IByteStream& Stream, LPCSTR pStr, int iLength)

//	SerializeJSON
//
//	Serializes to JSON format (not including surrounding
//	double-quotes).

	{
	const char *pPos = pStr;

	//	Handle some edge conditions

	if (iLength == 0)
		return;

	//	Keep looping until we're done

	bool bEscapeNextSlash = false;
	const char *pStart = pPos;
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

		else if (*pPos == '\\' || *pPos == '"' || strIsASCIIControl(pPos))
			{
			//	Write out what we've got so far

			Stream.Write(pStart, pPos - pStart);

			//	Escape the character

			switch (*pPos)
				{
				case '"':
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

