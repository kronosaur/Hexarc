//	CHexeTextMarkup.cpp
//
//	CHexeTextMarkup class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FORMAT_HEXE_TEXT,					"hexetext");
DECLARE_CONST_STRING(FORMAT_HTML,						"html");

DECLARE_CONST_STRING(ERR_UNEXPECTED_END_OF_TEMPLATE,	"Unexpected end of template.");
DECLARE_CONST_STRING(ERR_UNKNOWN_FORMAT,				"Unknown markup format: %s");

bool CHexeTextMarkup::ConvertToHTML (const IMemoryBlock &Input, const CString &sFormat, CDatum dParams, IByteStream &Output, CString *retsError)

//	ConvertToHTML
//
//	Converts from the given markup format to HTML

	{
	CString sFormatKey = strToLower(sFormat);

	if (strEquals(sFormatKey, FORMAT_HEXE_TEXT))
		{
		CHexeTextProcessor Processor;
		if (!Processor.ConvertToHTML(Input, Output, retsError))
			return false;
		}
	else if (strEquals(sFormatKey, FORMAT_HTML))
		return EscapeHTML(Input, dParams, Output, retsError);
	else
		{
		*retsError = strPattern(ERR_UNKNOWN_FORMAT, sFormat);
		return false;
		}

	return true;
	}

bool CHexeTextMarkup::EscapeHTML (const IMemoryBlock &Template, CDatum dStruct, IByteStream &Output, CString *retsError)

//	EscapeHTML
//
//	Looks for any text of the form:
//
//	%param%
//
//	And looks up param in dStruct as a key. The value is then escaped for HTML and
//	replaced.
//
//	Returns TRUE if successful; FALSE if error.

	{
	char *pPos = Template.GetPointer();
	char *pPosEnd = pPos + Template.GetLength();
	char *pStart = pPos;

	while (pPos < pPosEnd)
		{
		if (*pPos == '%')
			{
			Output.Write(pStart, pPos - pStart);
			pPos++;

			if (pPos == pPosEnd)
				{
				if (retsError)
					*retsError = ERR_UNEXPECTED_END_OF_TEMPLATE;
				return false;
				}

			//	Escape %

			if (*pPos == '%')
				{
				Output.Write("%", 1);
				pPos++;
				}

			//	Get the parameter name

			else
				{
				pStart = pPos;
				while (pPos < pPosEnd && *pPos != '%')
					pPos++;

				if (pPos == pPosEnd)
					{
					if (retsError)
						*retsError = ERR_UNEXPECTED_END_OF_TEMPLATE;
					return false;
					}

				CString sParam(pStart, pPos - pStart);

				//	Write out the value of the parameter

				WriteHTMLContent(dStruct.GetElement(sParam), Output);

				//	Continue

				pPos++;
				pStart = pPos;
				}
			}
		else
			pPos++;
		}

	Output.Write(pStart, pPos - pStart);

	//	Create the result

	return true;
	}

CString CHexeTextMarkup::FormatString (CDatum dArgList)

//	FormatString
//
//	Formats a string assuming the first argument is a printf style format descriptor.

	{
	//	Must have at least one value.

	if (dArgList.GetCount() < 1)
		return NULL_STR;

	//	First parameter is a printf style pattern

	int iArg = 0;
	const CString &sPattern = dArgList.GetElement(iArg++);

	//	Compose

	CStringBuffer Stream;
	LPCSTR pPos = sPattern.GetParsePointer();
	LPCSTR pRunStart;
	int iRunLength;
	int iLastInteger = 1;

	//	Start

	pRunStart = pPos;
	iRunLength = 0;

	//	Loop

	while (*pPos != '\0')
		{
		if (*pPos == '%')
			{
			//	Save out what we've got til now

			if (iRunLength > 0)
				{
				Stream.Write(pRunStart, iRunLength);
				iRunLength = 0;
				}

			pPos++;

			//	Check some optional flags

			bool bZeroPad = false;
			if (*pPos == '0')
				{
				bZeroPad = true;
				pPos++;
				}

			//	Read the field size, if any

			int iFieldSize = strParseInt(pPos, 0, &pPos);

			//	If we have a . then we want to set decimal precision

			int iDecimalSize = 6;
			if (*pPos == '.')
				{
				pPos++;
				iDecimalSize = strParseInt(pPos, 6, &pPos);
				}

			//	Check the actual pattern code

			if (*pPos == 's')
				{
				CDatum dArg = dArgList.GetElement(iArg++);

				Stream.Write(dArg.AsString());

				pPos++;
				}
			else if (*pPos == 'd' || *pPos == 'D')
				{
				CDatum dArg = dArgList.GetElement(iArg++);
				int iValue = (int)dArg;
				CString sNew;

				sNew = strFromInt(iValue, true);
				if (sNew.GetLength() < iFieldSize)
					Stream.WriteChar((bZeroPad ? '0' : ' '), iFieldSize - sNew.GetLength());

				if (*pPos == 'd')
					Stream.Write(sNew);
				else
					{
					int iRun = sNew.GetLength() % 3;
					if (iRun == 0)
						iRun = 3;
					LPSTR pSrc = sNew;

					bool bComma = false;
					while (*pSrc != '\0')
						{
						if (bComma)
							Stream.Write(",", 1);

						Stream.Write(pSrc, iRun);
						pSrc += iRun;
						iRun = 3;
						bComma = true;
						}
					}

				//	Remember the last integer

				iLastInteger = iValue;

				//	Next

				pPos++;
				}
			else if (*pPos == 'x' || *pPos == 'X')
				{
				CDatum dArg = dArgList.GetElement(iArg++);
				DWORD dwValue = (DWORD)dArg;
				CString sNew;

				sNew = strFromIntOfBase(dwValue, 16, false, (*pPos == 'X'));
				if (sNew.GetLength() < iFieldSize)
					Stream.WriteChar('0', iFieldSize - sNew.GetLength());

				Stream.Write(sNew);

				//	Remember the last integer

				iLastInteger = (int)dwValue;

				//	Next

				pPos++;
				}
			else if (*pPos == 'f' || *pPos == 'F')
				{
				CDatum dArg = dArgList.GetElement(iArg++);
				double rValue = (double)dArg;

				CString sNew(_CVTBUFSIZE);
				int iDecimalOut;
				int iSignOut;
				int err = _fcvt_s(sNew.GetPointer(), sNew.GetLength(), rValue, iDecimalSize, &iDecimalOut, &iSignOut);
				if (err != 0)
					sNew = CString("ERROR");

				//	Write out the sign

				if (iSignOut != 0)
					Stream.WriteChar('-');

				//	If the decimal point is not positive, we need to write out zeroes

				if (iDecimalOut <= 0)
					{
					Stream.WriteChar('0');
					Stream.WriteChar('.');
					Stream.WriteChar('0', iDecimalOut);
					}

				//	Otherwise, write out digits to the left of the decimal point

				else
					{
					Stream.Write(sNew.GetPointer(), iDecimalOut);
					Stream.WriteChar('.');
					}

				//	Now write the characters to the right of the decimal point

				int iSkip = Max(0, iDecimalOut);
				int iDecimalCount = Min(sNew.GetLength() - iSkip, iDecimalSize);
				Stream.Write(sNew.GetPointer() + iSkip, iDecimalCount);

				//	Done

				pPos++;
				}
			else if (*pPos == 'p')
				{
				if (iLastInteger != 1)
					Stream.Write("s", 1);

				pPos++;
				}
			else if (*pPos == 'c')
				{
				CDatum dArg = dArgList.GetElement(iArg++);
				char chChar = (char)(int)dArg;
				Stream.Write(&chChar, 1);

				pPos++;
				}
			else if (*pPos == '%')
				{
				Stream.Write("%", 1);

				pPos++;
				}

			pRunStart = pPos;
			iRunLength = 0;
			}
		else
			{
			iRunLength++;
			pPos++;
			}
		}

	//	Save out the last run

	if (iRunLength > 0)
		Stream.Write(pRunStart, iRunLength);

	//	Convert the stream to a string

	return CString::CreateFromHandoff(Stream);
	}

void CHexeTextMarkup::WriteHTMLContent (CDatum dValue, IByteStream &Output)

//	WriteHTMLContent
//
//	Writes the datum as HTML (escaping as appropriate).

	{
	int i;

	if (dValue.GetBasicType() == CDatum::typeArray)
		{
		for (i = 0; i < dValue.GetCount(); i++)
			WriteHTMLContent(dValue.GetElement(i), Output);
		}
	else
		htmlWriteText(dValue.AsString(), Output);
	}
