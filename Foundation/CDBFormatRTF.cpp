//	CDBFormatRTF.cpp
//
//	CDBFormatRTF class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

void CDBFormatRTF::WriteEoF (IByteStream &Stream)
	{
	Stream.Write(CString("\r\n}"));
	}

void CDBFormatRTF::WriteHeader (IByteStream &Stream)
	{
	Stream.Write(CString("{\\rtf1\\ansi\\ansicpg1252\\pard\\plain "));
	}

void CDBFormatRTF::WriteText (IByteStream &Stream, const CString &sText)
	{
	const char *pPos = sText.GetParsePointer();
	const char *pStart = pPos;

	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '\\':
				Stream.WriteChar('\\');
				Stream.WriteChar('\\');
				pPos++;
				break;

			case '{':
				Stream.WriteChar('\\');
				Stream.WriteChar('{');
				pPos++;
				break;

			case '}':
				Stream.WriteChar('\\');
				Stream.WriteChar('}');
				pPos++;
				break;

			case '\n':
				Stream.Write(CString("\r\n\\par "));
				pPos++;
				break;

			case '\r':
				pPos++;
				break;

			default:
				if (strIsASCIIHigh(pPos))
					{
					UTF32 dwCode = strParseUTF8Char(&pPos, sText.GetParsePointer() + sText.GetLength());
					if (dwCode & 0xffff0000)
						Stream.WriteChar('?');
					else
						{
						int iCode = (int)(short)LOWORD(dwCode);
						Stream.Write(strPattern("\\u%d?", iCode));
						}
					}
				else
					{
					Stream.WriteChar(*pPos);
					pPos++;
					}
				break;
			}
		}
	}
