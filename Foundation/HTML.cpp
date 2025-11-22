//	HTML.cpp
//
//	HTML utilities
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void htmlWriteAttributeValue (const CString &sText, IByteStream &Output)

//	htmlWriteAttributeValue
//
//	Writes attribute value text, escaping all appropriate characters.

	{
	char *pPos = sText.GetParsePointer();
	char *pPosEnd = pPos + sText.GetLength();

	char *pStart = pPos;

	while (pPos < pPosEnd)
		{
		if (*pPos == '<')
			{
			Output.Write(pStart, pPos - pStart);
			pPos++;
			pStart = pPos;

			Output.Write("&lt;", 4);
			}
		else if (*pPos == '>')
			{
			Output.Write(pStart, pPos - pStart);
			pPos++;
			pStart = pPos;

			Output.Write("&gt;", 4);
			}
		else if (*pPos == '&')
			{
			Output.Write(pStart, pPos - pStart);
			pPos++;
			pStart = pPos;

			Output.Write("&amp;", 5);
			}
		else if (*pPos == '"')
			{
			Output.Write(pStart, pPos - pStart);
			pPos++;
			pStart = pPos;

			Output.Write("&quot;", 6);
			}
		else if (*pPos == '\'')
			{
			Output.Write(pStart, pPos - pStart);
			pPos++;
			pStart = pPos;

			Output.Write("&apos;", 6);
			}
		else if (*pPos == '`')
			{
			Output.Write(pStart, pPos - pStart);
			pPos++;
			pStart = pPos;

			Output.Write("&#096;", 6);
			}
		else
            //  LATER: Should check for lower-ASCII characters
			pPos++;
		}

	Output.Write(pStart, pPos - pStart);
	}

CString htmlWriteAttributeValue (const CString &sText)

//	htmlWriteAttributeValue
//
//	Writes text, escaping all appropriate HTML characters.

	{
	CStringBuffer Output;
	htmlWriteAttributeValue(sText, Output);
	return Output;
	}

void htmlWriteText (const char* pPos, const char* pPosEnd, IByteStream& Output)

//	htmlWriteText
//
//	Writes text, escaping all appropriate HTML characters.
//
//	NOTE: This is only for the text content of HTML element; for escaping
//	attribute values, use htmlWriteAttributeValue

	{
	const char *pStart = pPos;

	while (pPos < pPosEnd)
		{
		switch (*pPos)
			{
			case '<':
				Output.Write(pStart, (int)(pPos - pStart));
				pPos++;
				pStart = pPos;

				Output.Write("&lt;", 4);
				break;

			case '>':
				Output.Write(pStart, (int)(pPos - pStart));
				pPos++;
				pStart = pPos;

				Output.Write("&gt;", 4);
				break;

			case '&':
				Output.Write(pStart, (int)(pPos - pStart));
				pPos++;
				pStart = pPos;

				Output.Write("&amp;", 5);
				break;

			default:
				//	Most control codes (other than whitespace) are illegal,
				//	so we strip them out.

				if ((BYTE)*pPos <= 0x08
						|| *pPos == 0x0b
						|| *pPos == 0x0c
						|| ((BYTE)*pPos >= 0x0e && (BYTE)*pPos <= 0x1f))
					{
					Output.Write(pStart, (int)(pPos - pStart));
					pPos++;
					pStart = pPos;
					}

				//	Otherwise, keep parsing

				else
					pPos++;
				break;
			}
		}

	Output.Write(pStart, (int)(pPos - pStart));
	}

void htmlWriteText (const CString& sText, IByteStream& Output)

//	htmlWriteText
//
//	Writes text, escaping all appropriate HTML characters.
//
//	NOTE: This is only for the text content of HTML element; for escaping
//	attribute values, use htmlWriteAttributeValue

	{
	const char *pPos = sText.GetParsePointer();
	const char *pPosEnd = pPos + sText.GetLength();
	htmlWriteText(pPos, pPosEnd, Output);
	}

CString htmlWriteText (const CString &sText)

//	htmlWriteText
//
//	Writes text, escaping all appropriate HTML characters.

	{
	CStringBuffer Output;
	htmlWriteText(sText, Output);
	return Output;
	}
