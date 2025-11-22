//	CDatumFormat.cpp
//
//	CDatumFormat class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_COMMA,						"comma");
DECLARE_CONST_STRING(FIELD_LEADING,						"leading");
DECLARE_CONST_STRING(FIELD_MAX_DIGITS,					"maxDigits");
DECLARE_CONST_STRING(FIELD_MIN_DIGITS,					"minDigits");
DECLARE_CONST_STRING(FIELD_PERCENT,						"percent");
DECLARE_CONST_STRING(FIELD_TRAILING,					"trailing");
DECLARE_CONST_STRING(FIELD_ZEROS,						"zeros");

CDatum CDatumFormat::AsDatum (const CStringFormat& Format)

//	AsDatum
//
//	Encodes as a datum for interpretation by the client.

	{
	CDatum dResult(CDatum::typeStruct);

	if (Format.GetClauseCount() == 0)
		return dResult;

	const CStringFormat::SNumberFormat& NumFmt = Format.GetClause(0).Format;	 //	We only support one clause for now

	if (!NumFmt.sLeading.IsEmpty())
		dResult.SetElement(FIELD_LEADING, NumFmt.sLeading);

	if (!NumFmt.sTrailing.IsEmpty())
		dResult.SetElement(FIELD_TRAILING, NumFmt.sTrailing);

	if (NumFmt.iZeroPadding != 0)
		dResult.SetElement(FIELD_ZEROS, NumFmt.iZeroPadding);

	if (NumFmt.iMinSignificantDigits != 0)
		dResult.SetElement(FIELD_MIN_DIGITS, NumFmt.iMinSignificantDigits);

	if (NumFmt.iMaxSignificantDigits != 0)
		dResult.SetElement(FIELD_MAX_DIGITS, NumFmt.iMaxSignificantDigits);

	if (NumFmt.bCommaSeparators)
		dResult.SetElement(FIELD_COMMA, true);

	if (NumFmt.bPercent)
		dResult.SetElement(FIELD_PERCENT, true);

	return dResult;
	}

CString CDatumFormat::FormatParams (CStringView sValue, CDatum dParams)

//	FormatParams
//
//	Given a string with placeholders for parameters, and a struct of parameters,
//	we return a resulting string.
//
//	Parameters are specified with {param} or {param:format}. {{ and }} are 
//	escape codes.

	{
	bool bUseOrdinal = !dParams.IsStruct();

	CStringBuffer Output;
	const char* pPos = sValue.GetParsePointer();
	const char* pPosEnd = pPos + sValue.GetLength();

	while (pPos < pPosEnd)
		{
		if (*pPos == '{')
			{
			pPos++;
			if (*pPos == '{')
				{
				Output.WriteChar(*pPos);
				}
			else
				{
				const char* pStart = pPos;
				while (*pPos != '\0' && *pPos != '}' && *pPos != ':')
					pPos++;

				CString sArg(pStart, pPos - pStart);

				CString sFormat;
				if (*pPos == ':')
					{
					pPos++;
					pStart = pPos;
					while (*pPos != '\0' && *pPos != '}')
						pPos++;

					sFormat = CString(pStart, pPos - pStart);
					}

				CDatum dValue = (bUseOrdinal ? dParams.GetElement(strToInt(sArg)) : dParams.GetElement(sArg));
				Output.Write(dValue.Format(sFormat));
				}
			}
		else if (*pPos == '}')
			{
			Output.WriteChar(*pPos);
			if (pPos[1] == '}')
				pPos++;
			}
		else
			{
			Output.WriteChar(*pPos);
			}

		pPos++;
		}

	return CString(std::move(Output));
	}

CString CDatumFormat::FormatParamsByOrdinal (CStringView sValue, CHexeStackEnv& LocalEnv)

//	FormatParamsByOrdinal
//
//	Given a string with placeholders and an array of parameters, we return a
//	resulting string.
//
//	NOTE: Ordinals are 0-based, but the first element of dParams is ignored
//	because it is the this pointer (string).

	{
	CStringBuffer Output;
	const char* pPos = sValue.GetParsePointer();
	const char* pPosEnd = pPos + sValue.GetLength();
	int iNextOrdinal = 0;

	while (pPos < pPosEnd)
		{
		if (*pPos == '{')
			{
			pPos++;
			if (*pPos == '{')
				{
				Output.WriteChar(*pPos);
				}
			else
				{
				const char* pStart = pPos;
				while (*pPos != '\0' && *pPos != '}' && *pPos != ':')
					pPos++;

				CString sArg(pStart, pPos - pStart);

				CString sFormat;
				if (*pPos == ':')
					{
					pPos++;
					pStart = pPos;
					while (*pPos != '\0' && *pPos != '}')
						pPos++;

					sFormat = CString(pStart, pPos - pStart);
					}

				int iOrdinal;
				if (sArg.IsEmpty())
					iOrdinal = iNextOrdinal++;
				else
					{
					iOrdinal = strToInt(sArg, -1);
					if (iOrdinal == -1)
						{
						iOrdinal = iNextOrdinal++;
						if (sFormat.IsEmpty())
							sFormat = sArg;
						}
					}

				CDatum dValue = LocalEnv.GetArgument(iOrdinal + 1);
				Output.Write(dValue.Format(sFormat));
				}
			}
		else if (*pPos == '}')
			{
			Output.WriteChar(*pPos);
			if (pPos[1] == '}')
				pPos++;
			}
		else
			{
			Output.WriteChar(*pPos);
			}

		pPos++;
		}

	return CString(std::move(Output));
	}

