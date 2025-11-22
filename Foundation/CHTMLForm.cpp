//	CHTMLForm.cpp
//
//	CHTMLFOrm class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.
//
//	The encoding is as follows:
//
//	1.	All spaces are converted to '+' (in both keys and values)
//	2.	All non-alphanumerics are URL encoded (e.g., %2B).
//	3.	Keys and values are separated by '='
//	4.	Key value pairs are separated by '&'
//	5.	Duplicate keys are allowed.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_ESCAPE_CONS,					"%%%02X")

DECLARE_CONST_STRING(MEDIA_WWW_FORM_URL_ENCODED,		"application/x-www-form-urlencoded")

void CHTMLForm::AddField (const CString &sKey, const CString &sValue)

//	AddField
//
//	Adds a field to the form

	{
	SField *pField = m_Fields.Insert();
	pField->sKey = sKey;
	pField->sValue = sValue;
	}

bool CHTMLForm::DecodeFromBuffer (const CString &sMediaType, const IMemoryBlock &Buffer)

//	DecodeFromBuffer
//
//	Initializes the form from the given encoded buffer

	{
	//	LATER: Not yet implemented
	return false;
	}

bool CHTMLForm::EncodeText (IByteStream &Stream, const CString &sText) const

//	EncodeText
//
//	Encodes the text to output buffer

	{
	char *pPos = sText.GetParsePointer();
	char *pEndPos = pPos + sText.GetLength();

	while (pPos < pEndPos)
		{
		if ((*pPos >= 'A' && *pPos <= 'Z')
				|| (*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= '0' && *pPos <= '9'))
			{
			Stream.Write(pPos, 1);
			}
		else if (*pPos == ' ')
			{
			Stream.Write("+", 1);
			}
		else
			{
			CString sEscape = strPattern("%%%02X", (DWORD)*pPos);
			Stream.Write(sEscape);
			}

		pPos++;
		}

	return true;
	}

bool CHTMLForm::EncodeToBuffer (IByteStream &Stream, DWORD dwOffset, DWORD dwSize) const

//	EncodeToBuffer
//
//	Encodes the form to the buffer

	{
	int i;

	for (i = 0; i < m_Fields.GetCount(); i++)
		{
		//	Output field separator

		if (i != 0)
			Stream.Write("&", 1);

		//	Output key

		if (!EncodeText(Stream, m_Fields[i].sKey))
			return false;

		//	Output key/value separator

		Stream.Write("=", 1);

		//	Output value

		if (!EncodeText(Stream, m_Fields[i].sValue))
			return false;
		}

	return true;
	}

DWORD CHTMLForm::GetEncodedTextLength (const CString &sText) const

//	GetEncodedTextLength
//
//	Returns the length that the given text would be encoded (in bytes)

	{
	char *pPos = sText.GetParsePointer();
	char *pEndPos = pPos + sText.GetLength();

	DWORD dwCount = 0;
	while (pPos < pEndPos)
		{
		if ((*pPos >= 'A' && *pPos <= 'Z')
				|| (*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= '0' && *pPos <= '9')
				|| *pPos == ' ')
			dwCount++;
		else
			dwCount += 3;

		pPos++;
		}

	return dwCount;
	}

DWORD CHTMLForm::GetMediaLength (void) const

//	GetMediaLength
//
//	Returns the size of the resulting encoded buffer in bytes

	{
	int i;

	DWORD dwCount = 0;
	for (i = 0; i < m_Fields.GetCount(); i++)
		{
		if (i != 0)
			dwCount++;

		dwCount += GetEncodedTextLength(m_Fields[i].sKey) + 1 + GetEncodedTextLength(m_Fields[i].sValue);
		}

	return dwCount;
	}

const CString &CHTMLForm::GetMediaType (void) const

//	GetMediaType
//
//	Returns the media type

	{
	return MEDIA_WWW_FORM_URL_ENCODED;
	}
