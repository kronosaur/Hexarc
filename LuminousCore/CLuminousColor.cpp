//	CLuminousColor.cpp
//
//	CLuminousColor Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(COLOR_ALERT_CAUTION,			"AlertCaution");
DECLARE_CONST_STRING(COLOR_ALERT_ERROR,				"AlertError");
DECLARE_CONST_STRING(COLOR_ALERT_SUCCESS,			"AlertSuccess");
DECLARE_CONST_STRING(COLOR_ALERT_WARNING,			"AlertWarning");
DECLARE_CONST_STRING(COLOR_AQUA,					"Aqua");
DECLARE_CONST_STRING(COLOR_BLUE,					"Blue");
DECLARE_CONST_STRING(COLOR_CERULEAN,				"Cerulean");
DECLARE_CONST_STRING(COLOR_GREEN,					"Green");
DECLARE_CONST_STRING(COLOR_LIME,					"Lime");
DECLARE_CONST_STRING(COLOR_NEUTRAL,					"Neutral");
DECLARE_CONST_STRING(COLOR_ORANGE,					"Orange");
DECLARE_CONST_STRING(COLOR_PRIMARY,					"Primary");
DECLARE_CONST_STRING(COLOR_PURPLE,					"Purple");
DECLARE_CONST_STRING(COLOR_RED,						"Red");
DECLARE_CONST_STRING(COLOR_SECONDARY,				"Secondary");
DECLARE_CONST_STRING(COLOR_YELLOW,					"Yellow");

DECLARE_CONST_STRING(VARIANT_FADED,					"Faded");
DECLARE_CONST_STRING(VARIANT_INK,					"Ink");
DECLARE_CONST_STRING(VARIANT_PAPER,					"Paper");

DECLARE_CONST_STRING(VARIANT_10,					"10");
DECLARE_CONST_STRING(VARIANT_20,					"20");
DECLARE_CONST_STRING(VARIANT_30,					"30");
DECLARE_CONST_STRING(VARIANT_40,					"40");
DECLARE_CONST_STRING(VARIANT_50,					"50");
DECLARE_CONST_STRING(VARIANT_60,					"60");
DECLARE_CONST_STRING(VARIANT_70,					"70");
DECLARE_CONST_STRING(VARIANT_80,					"80");
DECLARE_CONST_STRING(VARIANT_90,					"90");

const CLuminousColor CLuminousColor::Null;

bool CLuminousColor::operator== (const CLuminousColor &Src) const

//	CLuminousColor operator==

	{
	if (m_iType != Src.m_iType)
		return false;

	switch (m_iType)
		{
		case EType::Clear:
		case EType::None:
			break;

		case EType::Solid:
			if (m_rgbColor != Src.m_rgbColor)
				return false;
			break;

		case EType::Theme:
			if (m_iThemeColor != Src.m_iThemeColor)
				return false;

			if (m_iThemeColorVariant != Src.m_iThemeColorVariant)
				return false;
			break;

		default:
			throw CException(errFail);
		}

	return true;
	}

CString CLuminousColor::ComposeThemeColorID (const CString& sThemeColor, EVariant iVariant)

//	ComposeThemeColorID
//
//	Returns a theme color ID.

	{
	switch (iVariant)
		{
		case EVariant::Normal:
			return sThemeColor;

		case EVariant::Faded:
			return strPattern("%s_%s", sThemeColor, VARIANT_FADED);

		case EVariant::Ink:
			return strPattern("%s_%s", sThemeColor, VARIANT_INK);

		case EVariant::Paper:
			return strPattern("%s_%s", sThemeColor, VARIANT_PAPER);

		default:
			throw CException(errFail);
		}
	}

CLuminousColor::ETheme CLuminousColor::GetThemeColor (EVariant& retiVariant) const

//	GetThemeColor
//
//	Returns the theme color if this is a theme color.

	{
	switch (m_iType)
		{
		case EType::Theme:
			retiVariant = m_iThemeColorVariant;
			return m_iThemeColor;

		default:
			{
			retiVariant = EVariant::Normal;
			return ETheme::None;
			}
		}
	}

CString CLuminousColor::GetThemeColorID () const

//	GetThemeColorID
//
//	Returns the theme color ID (or NULL if not a theme color).

	{
	if (m_iType != EType::Theme)
		return NULL_STR;

	switch (m_iThemeColor)
		{
		case ETheme::AlertCaution:
			return ComposeThemeColorID(COLOR_ALERT_CAUTION, m_iThemeColorVariant);

		case ETheme::AlertError:
			return ComposeThemeColorID(COLOR_ALERT_ERROR, m_iThemeColorVariant);

		case ETheme::AlertSuccess:
			return ComposeThemeColorID(COLOR_ALERT_SUCCESS, m_iThemeColorVariant);

		case ETheme::AlertWarning:
			return ComposeThemeColorID(COLOR_ALERT_WARNING, m_iThemeColorVariant);

		case ETheme::Aqua:
			return ComposeThemeColorID(COLOR_AQUA, m_iThemeColorVariant);

		case ETheme::Blue:
			return ComposeThemeColorID(COLOR_BLUE, m_iThemeColorVariant);

		case ETheme::Cerulean:
			return ComposeThemeColorID(COLOR_CERULEAN, m_iThemeColorVariant);

		case ETheme::Green:
			return ComposeThemeColorID(COLOR_GREEN, m_iThemeColorVariant);

		case ETheme::Lime:
			return ComposeThemeColorID(COLOR_LIME, m_iThemeColorVariant);

		case ETheme::Orange:
			return ComposeThemeColorID(COLOR_ORANGE, m_iThemeColorVariant);

		case ETheme::Primary:
			return ComposeThemeColorID(COLOR_PRIMARY, m_iThemeColorVariant);

		case ETheme::Purple:
			return ComposeThemeColorID(COLOR_PURPLE, m_iThemeColorVariant);

		case ETheme::Red:
			return ComposeThemeColorID(COLOR_RED, m_iThemeColorVariant);

		case ETheme::Secondary:
			return ComposeThemeColorID(COLOR_SECONDARY, m_iThemeColorVariant);

		case ETheme::Yellow:
			return ComposeThemeColorID(COLOR_YELLOW, m_iThemeColorVariant);

		case ETheme::Neutral_Paper:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_PAPER);

		case ETheme::Neutral_10:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_10);

		case ETheme::Neutral_20:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_20);

		case ETheme::Neutral_30:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_30);

		case ETheme::Neutral_40:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_40);

		case ETheme::Neutral_50:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_50);

		case ETheme::Neutral_60:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_60);

		case ETheme::Neutral_70:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_70);

		case ETheme::Neutral_80:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_80);

		case ETheme::Neutral_90:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_90);

		case ETheme::Neutral_Ink:
			return strPattern("%s_%s", COLOR_NEUTRAL, VARIANT_INK);

		default:
			return NULL_STR;
		}
	}

CLuminousColor CLuminousColor::ParseThemeColor (const CString& sValue, const CLuminousColor& Default)

//	ParseThemeColor
//
//	Parses a theme color.

	{
	//	Neutral gets treated specially.

	if (strStartsWithNoCase(sValue, COLOR_NEUTRAL))
		{
		CString sVariant = strSubString(sValue, COLOR_NEUTRAL.GetLength() + 1);
		if (strEqualsNoCase(sVariant, VARIANT_PAPER))
			return CLuminousColor(ETheme::Neutral_Paper);
		else if (strEqualsNoCase(sVariant, VARIANT_10))
			return CLuminousColor(ETheme::Neutral_10);
		else if (strEqualsNoCase(sVariant, VARIANT_20))
			return CLuminousColor(ETheme::Neutral_20);
		else if (strEqualsNoCase(sVariant, VARIANT_30))
			return CLuminousColor(ETheme::Neutral_30);
		else if (strEqualsNoCase(sVariant, VARIANT_40))
			return CLuminousColor(ETheme::Neutral_40);
		else if (strEqualsNoCase(sVariant, VARIANT_50))
			return CLuminousColor(ETheme::Neutral_50);
		else if (strEqualsNoCase(sVariant, VARIANT_60))
			return CLuminousColor(ETheme::Neutral_60);
		else if (strEqualsNoCase(sVariant, VARIANT_70))
			return CLuminousColor(ETheme::Neutral_70);
		else if (strEqualsNoCase(sVariant, VARIANT_80))
			return CLuminousColor(ETheme::Neutral_80);
		else if (strEqualsNoCase(sVariant, VARIANT_90))
			return CLuminousColor(ETheme::Neutral_90);
		else if (strEqualsNoCase(sVariant, VARIANT_INK))
			return CLuminousColor(ETheme::Neutral_Ink);
		else
			return Default;
		}

	//	Other colors

	else
		{
		ETheme iTheme;

		if (strStartsWithNoCase(sValue, COLOR_ALERT_CAUTION))
			iTheme = ETheme::AlertCaution;
		else if (strStartsWithNoCase(sValue, COLOR_ALERT_ERROR))
			iTheme = ETheme::AlertError;
		else if (strStartsWithNoCase(sValue, COLOR_ALERT_SUCCESS))
			iTheme = ETheme::AlertSuccess;
		else if (strStartsWithNoCase(sValue, COLOR_ALERT_WARNING))
			iTheme = ETheme::AlertWarning;
		else if (strStartsWithNoCase(sValue, COLOR_AQUA))
			iTheme = ETheme::Aqua;
		else if (strStartsWithNoCase(sValue, COLOR_BLUE))
			iTheme = ETheme::Blue;
		else if (strStartsWithNoCase(sValue, COLOR_CERULEAN))
			iTheme = ETheme::Cerulean;
		else if (strStartsWithNoCase(sValue, COLOR_GREEN))
			iTheme = ETheme::Green;
		else if (strStartsWithNoCase(sValue, COLOR_LIME))
			iTheme = ETheme::Lime;
		else if (strStartsWithNoCase(sValue, COLOR_ORANGE))
			iTheme = ETheme::Orange;
		else if (strStartsWithNoCase(sValue, COLOR_PRIMARY))
			iTheme = ETheme::Primary;
		else if (strStartsWithNoCase(sValue, COLOR_PURPLE))
			iTheme = ETheme::Purple;
		else if (strStartsWithNoCase(sValue, COLOR_RED))
			iTheme = ETheme::Red;
		else if (strStartsWithNoCase(sValue, COLOR_SECONDARY))
			iTheme = ETheme::Secondary;
		else if (strStartsWithNoCase(sValue, COLOR_YELLOW))
			iTheme = ETheme::Yellow;
		else
			return Default;

		const char *pPos = sValue.GetParsePointer();
		while (*pPos != '_' && *pPos != '\0')
			pPos++;

		if (*pPos == '\0')
			return CLuminousColor(iTheme);
		else
			{
			CString sVariant = CString(pPos + 1);
			if (strEqualsNoCase(sVariant, VARIANT_FADED))
				return CLuminousColor(iTheme, EVariant::Faded);
			else if (strEqualsNoCase(sVariant, VARIANT_INK))
				return CLuminousColor(iTheme, EVariant::Ink);
			else if (strEqualsNoCase(sVariant, VARIANT_PAPER))
				return CLuminousColor(iTheme, EVariant::Paper);
			else
				return CLuminousColor(iTheme);
			}
		}
	}
