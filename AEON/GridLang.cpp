//	GridLang.cpp
//
//	Methods for converting to/from GridLang literals
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_NAN,							"nan");

void CDatum::SerializeGridLang (IByteStream &Stream) const

//	SerializeGridLang
//
//	Serializes to a GridLang literal

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			Stream.Write("null", 4);
			break;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					Stream.Write("false", 5);
					break;

				case VALUE_TRUE:
					Stream.Write("true", 4);
					break;

				default:
					ASSERT(false);
					break;
				}
			break;
			}

		case TYPE_INT32:
			{
			CString sInt = strFromInt(DecodeInt32(m_dwData));
			Stream.Write(sInt);
			break;
			}

		case TYPE_ENUM:
			SerializeEnum(EFormat::GridLang, Stream);
			break;

		case TYPE_STRING:
			WriteGridLangString(Stream, DecodeString(m_dwData));
			break;

		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).Serialize(EFormat::GridLang, Stream);
			break;

		case TYPE_ROW_REF:
			CAEONRowRefImpl::Serialize(m_dwData, EFormat::GridLang, Stream);
			break;

		case TYPE_NAN:
			Stream.Write(STR_NAN);
			break;

		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			Stream.Write(STR_NAN);
			break;

		default:
			{
			double rValue = DecodeDouble(m_dwData);
			if (isnan(rValue))
				Stream.Write(STR_NAN);
			else if (isinf(rValue))
				Stream.Write(STR_NAN);
			else
				Stream.Write(strFromDouble(rValue));
			break;
			}
		}
	}

void CDatum::WriteGridLangIdentifier (IByteStream& Stream, CStringView sString)
	{
	//	Check to see if this string is a valid GridLang identifier. If not, we write it out
	//	as a string.

	const char* pPos = sString.GetParsePointer();
	const char* pEndPos = pPos + sString.GetLength();

	bool bIsIdentifier = strIsASCIIAlpha(pPos) || *pPos == '_';
	pPos++;
	while (pPos < pEndPos)
		{
		if (strIsASCIIAlpha(pPos) || strIsDigit(pPos) || *pPos == '_')
			{ }
		else
			{
			bIsIdentifier = false;
			break;
			}

		pPos++;
		}

	if (bIsIdentifier)
		Stream.Write(sString);
	else
		WriteGridLangString(Stream, sString);
	}

void CDatum::WriteGridLangString (IByteStream& Stream, CStringView sString)
	{
	Stream.WriteChar('"');

	const char* pPos = sString.GetParsePointer();
	const char* pEndPos = pPos + sString.GetLength();
	const char* pStart = pPos;

	while (true)
		{
		switch (*pPos)
			{
			//	NOTE: CString string are always null-terminated. But note that
			//	we assume that strings are UTF-8. For non-UTF-8 string we should
			//	use binary blobs.

			case '\0':
				Stream.Write(pStart, pPos - pStart);
				Stream.WriteChar('"');
				return;

			case '\"':
				Stream.Write(pStart, pPos - pStart);
				pPos++;
				pStart = pPos;
				Stream.WriteChar('"', 2);
				break;

			default:
				pPos++;
				break;
			}
		}
	}

