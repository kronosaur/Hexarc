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

CString CDatumFormat::FormatDouble (double rValue, CDatum dFormat)

//	FormatDouble
//
//	Formats a double from the client-side descriptor returned by AsDatum.

	{
	if (dFormat.GetBasicType() == CDatum::typeString)
		return CStringFormat(dFormat.AsString()).FormatDouble(rValue);
	else if (!dFormat.IsStruct())
		return CDatum(rValue).AsString();

	CString sLeading = dFormat.GetElement(FIELD_LEADING).AsString();
	CString sTrailing = dFormat.GetElement(FIELD_TRAILING).AsString();
	int iZeros = Max(0, (int)dFormat.GetElement(FIELD_ZEROS));
	int iMinDigits = Max(0, (int)dFormat.GetElement(FIELD_MIN_DIGITS));
	int iMaxDigits = Max(iMinDigits, (int)dFormat.GetElement(FIELD_MAX_DIGITS));

	//	Reconstitute a CStringFormat pattern with the same number-format fields.
	//	A leading comma is sufficient to enable grouping; CStringFormat derives
	//	the actual comma positions from the number of integer digits.

	CStringBuffer Format;
	Format.Write(sLeading);
	if (dFormat.GetElement(FIELD_COMMA).AsBool())
		Format.WriteChar(',');

	if (iZeros > 0)
		for (int i = 0; i < iZeros; i++)
			Format.WriteChar('0');
	else
		Format.WriteChar('#');

	if (iMaxDigits > 0)
		{
		Format.WriteChar('.');
		for (int i = 0; i < iMinDigits; i++)
			Format.WriteChar('0');

		for (int i = iMinDigits; i < iMaxDigits; i++)
			Format.WriteChar('#');
		}

	Format.Write(sTrailing);

	//	AsDatum preserves the percent character in either the leading or trailing
	//	text. Be defensive about hand-built descriptors which set only the flag.

	if (dFormat.GetElement(FIELD_PERCENT).AsBool()
			&& strFind(sLeading, CString("%")) == -1
			&& strFind(sTrailing, CString("%")) == -1)
		rValue *= 100.0;

	return CStringFormat(CString::CreateFromHandoff(Format)).FormatDouble(rValue);
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

