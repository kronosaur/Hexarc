//	CDBFormatRTF.cpp
//
//	CDBFormatRTF class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_CDBFORMAT_RTF_CARRIAGE_RETURN_NEWLINE,	"\r\n}");
DECLARE_CONST_STRING(STR_CDBFORMAT_RTF_CARRIAGE_RETURN_TF1_BACKSLASH_ANSI_BACKSLASH_ANSICPG1252_BACKSLASH_PARD_B_AB5992A3,	"{\\rtf1\\ansi\\ansicpg1252\\pard\\plain ");
DECLARE_CONST_STRING(STR_CDBFORMAT_RTF_CARRIAGE_RETURN_NEWLINE_BACKSLASH_PAR,	"\r\n\\par ");

void CDBFormatRTF::WriteEoF (IByteStream &Stream)
	{
	Stream.Write(STR_CDBFORMAT_RTF_CARRIAGE_RETURN_NEWLINE);
	}

void CDBFormatRTF::WriteHeader (IByteStream &Stream)
	{
	Stream.Write(STR_CDBFORMAT_RTF_CARRIAGE_RETURN_TF1_BACKSLASH_ANSI_BACKSLASH_ANSICPG1252_BACKSLASH_PARD_B_AB5992A3);
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
				Stream.Write(STR_CDBFORMAT_RTF_CARRIAGE_RETURN_NEWLINE_BACKSLASH_PAR);
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
