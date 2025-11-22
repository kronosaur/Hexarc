//	CLuminousFontStyle.cpp
//
//	CLuminousLineStyle Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(DEFAULT_TYPEFACE,				"sans-serif");

DECLARE_CONST_STRING(FONT_BOLD,						"bold");
DECLARE_CONST_STRING(FONT_ITALIC,					"italic");
DECLARE_CONST_STRING(FONT_NORMAL,					"normal");
DECLARE_CONST_STRING(FONT_OBLIQUE,					"oblique");

DECLARE_CONST_STRING(UNIT_PX,						"px");

const CLuminousFontStyle CLuminousFontStyle::Null;

CString CLuminousFontStyle::AsHTML () const

//	AsHTML
//
//	Returns an HTML/CSS font property.

	{
	return strPattern("%s%s%s %s",
			(GetStyle() != EType::Normal ? strPattern("%s ", GetStyleAsID()) : NULL_STR),
			(GetWeight() != WEIGHT_NORMAL ? strPattern("%d ", GetWeight()) : NULL_STR),
			GetSizeAsHTML(),
			(GetTypeface().IsEmpty() ? DEFAULT_TYPEFACE : GetTypeface())
			);
	}

CString CLuminousFontStyle::GetID (EType iStyle)

//	GetID
//
//	Returns an ID.

	{
	switch (iStyle)
		{
		case EType::Italic:
			return FONT_ITALIC;

		case EType::Normal:
			return FONT_NORMAL;

		case EType::Oblique:
			return FONT_OBLIQUE;

		default:
			throw CException(errFail);
		}
	}

CString CLuminousFontStyle::GetSizeAsHTML () const

//	GetSizeAsHTML
//
//	Returns the size (and optionally line height) as an HTML/CSS property.

	{
	CString sSize = strFromDouble(m_rSize, 2);
	if (m_rLineHeight > 0.0 && m_rLineHeight != DEFAULT_LINE_HEIGHT)
		return strPattern("%spx/%s", sSize, strFromDouble(m_rLineHeight, 2));
	else
		return strPattern("%spx", sSize);
	}

CLuminousFontStyle CLuminousFontStyle::ParseHTML (const CString& sValue)

//	ParseHTML
//
//	Parses a HTML/CSS font property.

	{
	CLuminousFontStyle Result;

	TArray<CString> Tokens = CHTML::ParseCSSProperty(sValue);
	int iToken = 0;
	bool bFoundSize = false;

	while (iToken < Tokens.GetCount() && !bFoundSize)
		{
		//	Is this a font size?

		double rSize;
		double rLineHeight;
		if (CLuminousFontStyle::ParseSize(Tokens[iToken], &rSize, &rLineHeight))
			{
			Result.SetSize(rSize);
			if (rLineHeight > 0.0)
				Result.SetLineHeight(rLineHeight);
			iToken++;
			bFoundSize = true;
			break;
			}

		//	Is this a font style?

		CLuminousFontStyle::EType iFontStyle = CLuminousFontStyle::ParseStyle(Tokens[iToken]);
		if (iFontStyle != CLuminousFontStyle::EType::Unknown)
			{
			Result.SetStyle(iFontStyle);
			iToken++;
			continue;
			}

		//	Is this a font weight?

		int iWeight = CLuminousFontStyle::ParseWeight(Tokens[iToken]);
		if (iWeight != -1)
			{
			Result.SetWeight(iWeight);
			iToken++;
			continue;
			}
		}

	//	Once we've found the size, the remaining token is the font family.

	if (bFoundSize && iToken < Tokens.GetCount())
		{
		Result.SetTypeface(Tokens[iToken]);
		iToken++;
		}

	//	Success!

	return Result;
	}

bool CLuminousFontStyle::ParseSize (const CString& sValue, double* retrSize, double* retiLineHeight)

//	ParseSize
//
//	Parse a size of the form:
//
//	16px
//	16px/1.4
//
//	We return FALSE if we don't match these patterns. If line height is not 
//	specifed, we return DEFAULT_LINE_HEIGHT.

	{
	const char *pPos = sValue.GetParsePointer();
	int iSize = strParseInt(pPos, -1, &pPos);
	if (iSize == -1)
		return false;

	//	Parse the unit

	const char *pStart = pPos;
	while (*pPos != '\0' && *pPos != '/')
		pPos++;

	CString sUnit(pStart, pPos - pStart);
	if (sUnit.IsEmpty())
		return false;
	else if (strEqualsNoCase(sUnit, UNIT_PX))
		;
	else
		return false;

	//	If we have a line height, parse it

	double rLineHeight = DEFAULT_LINE_HEIGHT;
	if (*pPos == '/')
		{
		pPos++;
		rLineHeight = strParseDouble(pPos, 0.0, &pPos);
		if (rLineHeight <= 0.0)
			return false;
		}

	//	Done

	if (retrSize)
		*retrSize = (double)iSize;

	if (retiLineHeight)
		*retiLineHeight = rLineHeight;

	return true;
	}

CLuminousFontStyle::EType CLuminousFontStyle::ParseStyle (const CString& sValue)

//	ParseStyle
//
//	Parses a style, or returns Unknown if no match.

	{
	if (strEqualsNoCase(sValue, FONT_ITALIC))
		return EType::Italic;
	else if (strEqualsNoCase(sValue, FONT_NORMAL))
		return EType::Normal;
	else if (strEqualsNoCase(sValue, FONT_OBLIQUE))
		return EType::Oblique;
	else
		return EType::Unknown;
	}

int CLuminousFontStyle::ParseWeight (const CString& sValue)

//	ParseWeight
//
//	Parses a weight, or returns -1 if no match.

	{
	if (strEqualsNoCase(sValue, FONT_BOLD))
		return 700;
	else if (strEqualsNoCase(sValue, FONT_NORMAL))
		return 400;
	else
		{
		const char *pPos = sValue.GetParsePointer();
		int iWeight = strParseInt(pPos, -1, &pPos);
		if (iWeight != -1 && *pPos == '\0')
			return iWeight;
		else
			return -1;
		}
	}
