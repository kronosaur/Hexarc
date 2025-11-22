//	CLuminousColor.cpp
//
//	CLuminousColor Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

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

DECLARE_CONST_STRING(ROLE_PAPER, "Paper");
DECLARE_CONST_STRING(ROLE_LIGHT, "Light");
DECLARE_CONST_STRING(ROLE_HIGHLIGHT, "Highlight");
DECLARE_CONST_STRING(ROLE_OVERLAY, "Overlay");
DECLARE_CONST_STRING(ROLE_ACCENT, "Accent");
DECLARE_CONST_STRING(ROLE_MARKER, "Marker");
DECLARE_CONST_STRING(ROLE_SOLID, "Solid");
DECLARE_CONST_STRING(ROLE_SOLID_TEXT, "SolidText");
DECLARE_CONST_STRING(ROLE_EMPHASIS, "Emphasis");
DECLARE_CONST_STRING(ROLE_CONTRAST, "Contrast");
DECLARE_CONST_STRING(ROLE_DARK, "Dark");
DECLARE_CONST_STRING(ROLE_INK, "Ink");
DECLARE_CONST_STRING(ROLE_ERROR, "Error");
DECLARE_CONST_STRING(ROLE_ERROR_TEXT, "ErrorText");
DECLARE_CONST_STRING(ROLE_WARNING, "Warning");
DECLARE_CONST_STRING(ROLE_WARNING_TEXT, "WarningText");
DECLARE_CONST_STRING(ROLE_LINK, "Link");
DECLARE_CONST_STRING(ROLE_LINK_TEXT, "LinkText");
DECLARE_CONST_STRING(ROLE_LINK_HOVER, "LinkHover");
DECLARE_CONST_STRING(ROLE_LINK_HOVER_TEXT, "LinkHoverText");

DECLARE_CONST_STRING(SHADE_PRIMARY,					"Primary");
DECLARE_CONST_STRING(SHADE_SECONDARY,				"Secondary");
DECLARE_CONST_STRING(SHADE_TERTIARY,				"Tertiary");
DECLARE_CONST_STRING(SHADE_GRAYSCALE,				"Grayscale");
DECLARE_CONST_STRING(SHADE_RED,						"Red");
DECLARE_CONST_STRING(SHADE_ORANGE,					"Orange");
DECLARE_CONST_STRING(SHADE_YELLOW,					"Yellow");
DECLARE_CONST_STRING(SHADE_GREEN,					"Green");
DECLARE_CONST_STRING(SHADE_BLUE,					"Blue");
DECLARE_CONST_STRING(SHADE_PURPLE,					"Purple");
DECLARE_CONST_STRING(SHADE_PINK,					"Pink");

DECLARE_CONST_STRING(VARIANT_FADED,					"Faded");
DECLARE_CONST_STRING(VARIANT_INK,					"Ink");
DECLARE_CONST_STRING(VARIANT_PAPER,					"Paper");
DECLARE_CONST_STRING(VARIANT_TEXT,					"Text");

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
			if (m_Value.rgbColor != Src.m_Value.rgbColor)
				return false;
			break;

		case EType::Theme:
			if (m_Value.ThemeColor.iShade != Src.m_Value.ThemeColor.iShade)
				return false;

			if (m_Value.ThemeColor.iToneIndex != Src.m_Value.ThemeColor.iToneIndex)
				return false;

			break;

		case EType::Role:
			if (m_Value.iRole != Src.m_Value.iRole)
				return false;
			break;

		default:
			throw CException(errFail);
		}

	return true;
	}

CString CLuminousColor::ComposeThemeColorID (CStringView sShade, int iToneIndex)

//	ComposeThemeColorID
//
//	Compose a theme color ID

	{
	return strPattern("%s_%02d", sShade, ToTone(iToneIndex));
	}

void CLuminousColor::Copy (const CLuminousColor& Src)

//	Copy
//
//	Copy from source

	{
	m_iType = Src.m_iType;
	switch (m_iType)
		{
		case EType::Clear:
		case EType::None:
			break;

		case EType::Solid:
			m_Value.rgbColor = Src.m_Value.rgbColor;
			break;

		case EType::Theme:
			m_Value.ThemeColor = Src.m_Value.ThemeColor;
			break;

		case EType::Role:
			m_Value.iRole = Src.m_Value.iRole;
			break;

		default:
			throw CException(errFail);
		}
	}

CLuminousColor CLuminousColor::CreateFromStream (IByteStream& Stream)

//	CreateFromStream
//
//	Reads from a stream.

	{
	DWORD dwType = Stream.ReadDWORD();
	switch (dwType)
		{
		case TYPE_NONE:
		case TYPE_CLEAR:
			return CLuminousColor();

		case TYPE_SOLID:
			{
			DWORD dwColor = Stream.ReadDWORD();
			return CLuminousColor(CRGBA32(dwColor));
			}

		case TYPE_THEME_V1:
			{
			CString sThemeID = CString::Deserialize(Stream);
			return ParseThemeColor(sThemeID);
			}

		case TYPE_THEME:
			{
			CLuminousColor Result;
			Result.m_iType = EType::Theme;
			Result.m_Value.ThemeColor = DecodeThemeColor(Stream.ReadDWORD());
			return Result;
			}

		case TYPE_ROLE:
			return CLuminousColor(DecodeRole(Stream.ReadDWORD()));

		default:
			return CLuminousColor();
		}
	}

CLuminousColor::ERole CLuminousColor::DecodeRole (DWORD dwValue)

//	DecodeRole
//
//	Decodes a role

	{
	switch (dwValue)
		{
		case ROLE_NONE_ID:
			return ERole::None;

		case ROLE_PAPER_ID:
			return ERole::Paper;

		case ROLE_LIGHT_ID:
			return ERole::Light;

		case ROLE_HIGHLIGHT_ID:
			return ERole::Highlight;

		case ROLE_OVERLAY_ID:
			return ERole::Overlay;

		case ROLE_ACCENT_ID:
			return ERole::Accent;

		case ROLE_MARKER_ID:
			return ERole::Marker;

		case ROLE_SOLID_ID:
			return ERole::Solid;

		case ROLE_SOLID_TEXT_ID:
			return ERole::SolidText;

		case ROLE_EMPHASIS_ID:
			return ERole::Emphasis;

		case ROLE_CONTRAST_ID:
			return ERole::Contrast;

		case ROLE_DARK_ID:
			return ERole::Dark;

		case ROLE_INK_ID:
			return ERole::Ink;

		case ROLE_ERROR_ID:
			return ERole::Error;

		case ROLE_ERROR_TEXT_ID:
			return ERole::ErrorText;

		case ROLE_WARNING_ID:
			return ERole::Warning;

		case ROLE_WARNING_TEXT_ID:
			return ERole::WarningText;

		case ROLE_LINK_ID:
			return ERole::Link;

		case ROLE_LINK_TEXT_ID:
			return ERole::LinkText;

		case ROLE_LINK_HOVER_ID:
			return ERole::LinkHover;

		case ROLE_LINK_HOVER_TEXT_ID:
			return ERole::LinkHoverText;

		default:
			return ERole::None;
		}
	}

CLuminousColor::EShade CLuminousColor::DecodeShade (DWORD dwValue)

//	DecodeShade
//
//	Decodes a shade

	{
	switch (dwValue & SHADE_MASK)
		{
		case SHADE_NONE_ID:
			return EShade::None;

		case SHADE_PRIMARY_ID:
			return EShade::Primary;

		case SHADE_SECONDARY_ID:
			return EShade::Secondary;

		case SHADE_TERTIARY_ID:
			return EShade::Tertiary;

		case SHADE_GRAYSCALE_ID:
			return EShade::Grayscale;

		case SHADE_RED_ID:
			return EShade::Red;

		case SHADE_ORANGE_ID:
			return EShade::Orange;

		case SHADE_YELLOW_ID:
			return EShade::Yellow;

		case SHADE_GREEN_ID:
			return EShade::Green;

		case SHADE_BLUE_ID:
			return EShade::Blue;

		case SHADE_PURPLE_ID:
			return EShade::Purple;

		case SHADE_PINK_ID:
			return EShade::Pink;

		default:
			return EShade::None;
		}
	}

CLuminousColor::SThemeColor CLuminousColor::DecodeThemeColor (DWORD dwValue)

//	DecodeThemeColor
//
//	Decodes a theme color

	{
	SThemeColor Result;
	Result.iShade = DecodeShade(dwValue);
	Result.iToneIndex = (int)(dwValue & TONE_MASK);
	return Result;
	}

DWORD CLuminousColor::EncodeRole (ERole iRole)

//	EncodeRole
//
//	Encodes a role
	{
	switch (iRole)
		{
		case ERole::None:
			return ROLE_NONE_ID;

		case ERole::Paper:
			return ROLE_PAPER_ID;

		case ERole::Light:
			return ROLE_LIGHT_ID;

		case ERole::Highlight:
			return ROLE_HIGHLIGHT_ID;

		case ERole::Overlay:
			return ROLE_OVERLAY_ID;

		case ERole::Accent:
			return ROLE_ACCENT_ID;

		case ERole::Marker:
			return ROLE_MARKER_ID;

		case ERole::Solid:
			return ROLE_SOLID_ID;

		case ERole::SolidText:
			return ROLE_SOLID_TEXT_ID;

		case ERole::Emphasis:
			return ROLE_EMPHASIS_ID;

		case ERole::Contrast:
			return ROLE_CONTRAST_ID;

		case ERole::Dark:
			return ROLE_DARK_ID;

		case ERole::Ink:
			return ROLE_INK_ID;

		case ERole::Error:
			return ROLE_ERROR_ID;

		case ERole::ErrorText:
			return ROLE_ERROR_TEXT_ID;

		case ERole::Warning:
			return ROLE_WARNING_ID;

		case ERole::WarningText:
			return ROLE_WARNING_TEXT_ID;

		case ERole::Link:
			return ROLE_LINK_ID;

		case ERole::LinkText:
			return ROLE_LINK_TEXT_ID;

		case ERole::LinkHover:
			return ROLE_LINK_HOVER_ID;

		case ERole::LinkHoverText:
			return ROLE_LINK_HOVER_TEXT_ID;

		default:
			throw CException(errFail);
		}
	}

DWORD CLuminousColor::EncodeShade (EShade iShade)

//	EncodeShade
//
//	Encodes a shade

	{
	switch (iShade)
		{
		case EShade::None:
			return SHADE_NONE_ID;

		case EShade::Primary:
			return SHADE_PRIMARY_ID;

		case EShade::Secondary:
			return SHADE_SECONDARY_ID;

		case EShade::Tertiary:
			return SHADE_TERTIARY_ID;

		case EShade::Grayscale:
			return SHADE_GRAYSCALE_ID;

		case EShade::Red:
			return SHADE_RED_ID;

		case EShade::Orange:
			return SHADE_ORANGE_ID;

		case EShade::Yellow:
			return SHADE_YELLOW_ID;

		case EShade::Green:
			return SHADE_GREEN_ID;

		case EShade::Blue:
			return SHADE_BLUE_ID;

		case EShade::Purple:
			return SHADE_PURPLE_ID;

		case EShade::Pink:
			return SHADE_PINK_ID;

		default:
			throw CException(errFail);
		}
	}

DWORD CLuminousColor::EncodeThemeColor (EShade iShade, int iToneIndex)

//	EncodeThemeColor
//
//	Encodes a theme color.

	{
	return EncodeShade(iShade) | ((DWORD)iToneIndex & TONE_MASK);
	}

CRGBA32 CLuminousColor::GetSolidColor (const CLuminousColorTheme& Theme) const

//	GetSolidColor
//
//	Return the color RGB value.

	{
	switch (m_iType)
		{
		case CLuminousColor::EType::None:
			return CRGBA32::Null();

		case CLuminousColor::EType::Role:
			return Theme.GetColor(m_Value.iRole);

		case CLuminousColor::EType::Solid:
			return GetSolidColor();

		case CLuminousColor::EType::Theme:
			return Theme.GetColor(m_Value.ThemeColor.iShade, m_Value.ThemeColor.iToneIndex);

		default:
			throw CException(errFail);
		}
	}

CString CLuminousColor::GetThemeColorID () const

//	GetThemeColorID
//
//	Returns the theme color ID (or NULL if not a theme color).

	{
	switch (m_iType)
		{
		case EType::Role:
			switch (m_Value.iRole)
				{
				case ERole::Paper:
					return ROLE_PAPER;
				case ERole::Light:
					return ROLE_LIGHT;
				case ERole::Highlight:
					return ROLE_HIGHLIGHT;
				case ERole::Overlay:
					return ROLE_OVERLAY;
				case ERole::Accent:
					return ROLE_ACCENT;
				case ERole::Marker:
					return ROLE_MARKER;
				case ERole::Solid:
					return ROLE_SOLID;
				case ERole::SolidText:
					return ROLE_SOLID_TEXT;
				case ERole::Emphasis:
					return ROLE_EMPHASIS;
				case ERole::Contrast:
					return ROLE_CONTRAST;
				case ERole::Dark:
					return ROLE_DARK;
				case ERole::Ink:
					return ROLE_INK;
				case ERole::Error:
					return ROLE_ERROR;
				case ERole::ErrorText:
					return ROLE_ERROR_TEXT;
				case ERole::Warning:
					return ROLE_WARNING;
				case ERole::WarningText:
					return ROLE_WARNING_TEXT;
				case ERole::Link:
					return ROLE_LINK;
				case ERole::LinkText:
					return ROLE_LINK_TEXT;
				case ERole::LinkHover:
					return ROLE_LINK_HOVER;
				case ERole::LinkHoverText:
					return ROLE_LINK_HOVER_TEXT;
				default:
					return NULL_STR;
				}

		case EType::Theme:
			{
			switch (m_Value.ThemeColor.iShade)
				{
				case EShade::Primary:
					return ComposeThemeColorID(SHADE_PRIMARY, m_Value.ThemeColor.iToneIndex);

				case EShade::Secondary:
					return ComposeThemeColorID(SHADE_SECONDARY, m_Value.ThemeColor.iToneIndex);

				case EShade::Tertiary:
					return ComposeThemeColorID(SHADE_TERTIARY, m_Value.ThemeColor.iToneIndex);

				case EShade::Grayscale:
					return ComposeThemeColorID(SHADE_GRAYSCALE, m_Value.ThemeColor.iToneIndex);

				case EShade::Red:
					return ComposeThemeColorID(SHADE_RED, m_Value.ThemeColor.iToneIndex);

				case EShade::Orange:
					return ComposeThemeColorID(SHADE_ORANGE, m_Value.ThemeColor.iToneIndex);

				case EShade::Yellow:
					return ComposeThemeColorID(SHADE_YELLOW, m_Value.ThemeColor.iToneIndex);

				case EShade::Green:
					return ComposeThemeColorID(SHADE_GREEN, m_Value.ThemeColor.iToneIndex);

				case EShade::Blue:
					return ComposeThemeColorID(SHADE_BLUE, m_Value.ThemeColor.iToneIndex);

				case EShade::Purple:
					return ComposeThemeColorID(SHADE_PURPLE, m_Value.ThemeColor.iToneIndex);

				case EShade::Pink:
					return ComposeThemeColorID(SHADE_PINK, m_Value.ThemeColor.iToneIndex);

				default:
					return NULL_STR;
				}
			}

		default:
			return NULL_STR;
		}
	}

CLuminousColor CLuminousColor::ParseShade (EShade iShade, CStringView sTone, const CLuminousColor& Default)

//	ParseShade
//
//	Parses the shade and tone.

	{
	int iTone = strToInt(sTone, -1);
	if (iTone < 10 || iTone > 100)
		return Default;

	return CLuminousColor(iShade, iTone);
	}

CLuminousColor CLuminousColor::ParseShadeCompatible (EShade iShade, CStringView sTone, const CLuminousColor& Default)

//	ParseShade
//
//	Parses the shade and tone.

	{
	if (sTone.IsEmpty())
		return CLuminousColor(iShade, 60);
	else if (strEquals(sTone, VARIANT_PAPER))
		return CLuminousColor(iShade, 10);
	else if (strEquals(sTone, VARIANT_TEXT))
		return CLuminousColor(iShade, 10);
	else if (strEquals(sTone, VARIANT_FADED))
		return CLuminousColor(iShade, 80);
	else if (strEquals(sTone, VARIANT_INK))
		return CLuminousColor(iShade, 100);
	else
		{
		int iTone = strToInt(sTone, -1);
		if (iTone < 10 || iTone > 100)
			return Default;

		return CLuminousColor(iShade, iTone);
		}
	}

CLuminousColor CLuminousColor::ParseThemeColor (const CString& sValue, const CLuminousColor& Default)

//	ParseThemeColor
//
//	Parses a theme color. We support the following:
//
//	Primary_10 to Primary_100
//	Secondary_10 to Secondary_100
//	Tertiary_10 to Tertiary_100
//	Grayscale_10 to Grayscale_100
//	Red_10 to Red_100
//	Orange_10 to Orange_100
//	Yellow_10 to Yellow_100
//	Green_10 to Green_100
//	Blue_10 to Blue_100
//	Purple_10 to Purple_100
//	Pink_10 to Pink_100
//
//	Paper
//	Highlight
//	Overlay
//	Accent
//	Marker
//	Solid
//	SolidText
//	Emphasis
//	Contrast
//	Dark
//	Ink
//	Error
//	ErrorText
//	Warning
//	WarningText
//	Link
//	LinkText
//	LinkHover
//	LinkHoverText
//
//	LEGACY:
//
//	Neutral_Paper
//	Neutral_10 to Neutral_90
//	Neutral_Ink
//
//	Primary (+ _Faded, _Ink, _Paper, _Text)
//	Secondary (+ _Faded, _Ink, _Paper, _Text)
//	Aqua (+ _Faded, _Ink, _Paper, _Text)
//	Blue (+ _Faded, _Ink, _Paper, _Text)
//	Cerulean (+ _Faded, _Ink, _Paper, _Text)
//	Green (+ _Faded, _Ink, _Paper, _Text)
//	Lime (+ _Faded, _Ink, _Paper, _Text)
//	Orange (+ _Faded, _Ink, _Paper, _Text)
//	Purple (+ _Faded, _Ink, _Paper, _Text)
//	Red (+ _Faded, _Ink, _Paper, _Text)
//	Yellow (+ _Faded, _Ink, _Paper, _Text)
//	AlertCaution (+ _Faded, _Ink, _Paper, _Text)
//	AlertError (+ _Faded, _Ink, _Paper, _Text)
//	AlertSuccess (+ _Faded, _Ink, _Paper, _Text)
//	AlertWarning (+ _Faded, _Ink, _Paper, _Text)

	{
	if (sValue.IsEmpty())
		return Default;

	//	Handle shades and tones

	else if (strStartsWith(sValue, SHADE_PRIMARY))
		return ParseShadeCompatible(EShade::Primary, strSubString(sValue, SHADE_PRIMARY.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_SECONDARY))
		return ParseShadeCompatible(EShade::Secondary, strSubString(sValue, SHADE_SECONDARY.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_TERTIARY))
		return ParseShade(EShade::Tertiary, strSubString(sValue, SHADE_TERTIARY.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_GRAYSCALE))
		return ParseShade(EShade::Grayscale, strSubString(sValue, SHADE_GRAYSCALE.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_RED))
		return ParseShadeCompatible(EShade::Red, strSubString(sValue, SHADE_RED.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_ORANGE))
		return ParseShadeCompatible(EShade::Orange, strSubString(sValue, SHADE_ORANGE.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_YELLOW))
		return ParseShadeCompatible(EShade::Yellow, strSubString(sValue, SHADE_YELLOW.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_GREEN))
		return ParseShadeCompatible(EShade::Green, strSubString(sValue, SHADE_GREEN.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_BLUE))
		return ParseShadeCompatible(EShade::Blue, strSubString(sValue, SHADE_BLUE.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_PURPLE))
		return ParseShadeCompatible(EShade::Purple, strSubString(sValue, SHADE_PURPLE.GetLength() + 1), Default);
	else if (strStartsWith(sValue, SHADE_PINK))
		return ParseShade(EShade::Pink, strSubString(sValue, SHADE_PINK.GetLength() + 1), Default);

	//	Handle roles

	else if (strEquals(sValue, ROLE_PAPER))
		return CLuminousColor(ERole::Paper);
	else if (strEquals(sValue, ROLE_LIGHT))
		return CLuminousColor(ERole::Light);
	else if (strEquals(sValue, ROLE_HIGHLIGHT))
		return CLuminousColor(ERole::Highlight);
	else if (strEquals(sValue, ROLE_OVERLAY))
		return CLuminousColor(ERole::Overlay);
	else if (strEquals(sValue, ROLE_ACCENT))
		return CLuminousColor(ERole::Accent);
	else if (strEquals(sValue, ROLE_MARKER))
		return CLuminousColor(ERole::Marker);
	else if (strEquals(sValue, ROLE_SOLID))
		return CLuminousColor(ERole::Solid);
	else if (strEquals(sValue, ROLE_SOLID_TEXT))
		return CLuminousColor(ERole::SolidText);
	else if (strEquals(sValue, ROLE_EMPHASIS))
		return CLuminousColor(ERole::Emphasis);
	else if (strEquals(sValue, ROLE_CONTRAST))
		return CLuminousColor(ERole::Contrast);
	else if (strEquals(sValue, ROLE_DARK))
		return CLuminousColor(ERole::Dark);
	else if (strEquals(sValue, ROLE_INK))
		return CLuminousColor(ERole::Ink);
	else if (strEquals(sValue, ROLE_ERROR))
		return CLuminousColor(ERole::Error);
	else if (strEquals(sValue, ROLE_ERROR_TEXT))
		return CLuminousColor(ERole::ErrorText);
	else if (strEquals(sValue, ROLE_WARNING))
		return CLuminousColor(ERole::Warning);
	else if (strEquals(sValue, ROLE_WARNING_TEXT))
		return CLuminousColor(ERole::WarningText);
	else if (strEquals(sValue, ROLE_LINK))
		return CLuminousColor(ERole::Link);
	else if (strEquals(sValue, ROLE_LINK_TEXT))
		return CLuminousColor(ERole::LinkText);
	else if (strEquals(sValue, ROLE_LINK_HOVER))
		return CLuminousColor(ERole::LinkHover);
	else if (strEquals(sValue, ROLE_LINK_HOVER_TEXT))
		return CLuminousColor(ERole::LinkHoverText);

	//	Neutral gets treated specially.

	if (strStartsWithNoCase(sValue, COLOR_NEUTRAL))
		{
		CString sVariant = strSubString(sValue, COLOR_NEUTRAL.GetLength() + 1);
		if (strEqualsNoCase(sVariant, VARIANT_PAPER))
			return CLuminousColor(EShade::Grayscale, 10);
		else if (strEqualsNoCase(sVariant, VARIANT_10))
			return CLuminousColor(EShade::Grayscale, 10);
		else if (strEqualsNoCase(sVariant, VARIANT_20))
			return CLuminousColor(EShade::Grayscale, 20);
		else if (strEqualsNoCase(sVariant, VARIANT_30))
			return CLuminousColor(EShade::Grayscale, 30);
		else if (strEqualsNoCase(sVariant, VARIANT_40))
			return CLuminousColor(EShade::Grayscale, 40);
		else if (strEqualsNoCase(sVariant, VARIANT_50))
			return CLuminousColor(EShade::Grayscale, 50);
		else if (strEqualsNoCase(sVariant, VARIANT_60))
			return CLuminousColor(EShade::Grayscale, 60);
		else if (strEqualsNoCase(sVariant, VARIANT_70))
			return CLuminousColor(EShade::Grayscale, 70);
		else if (strEqualsNoCase(sVariant, VARIANT_80))
			return CLuminousColor(EShade::Grayscale, 80);
		else if (strEqualsNoCase(sVariant, VARIANT_90))
			return CLuminousColor(EShade::Grayscale, 90);
		else if (strEqualsNoCase(sVariant, VARIANT_INK))
			return CLuminousColor(EShade::Grayscale, 100);
		else
			return Default;
		}

	//	Other colors

	else
		{
		EShade iShade = EShade::None;

		if (strStartsWithNoCase(sValue, COLOR_ALERT_CAUTION))
			iShade = EShade::Yellow;
		else if (strStartsWithNoCase(sValue, COLOR_ALERT_ERROR))
			iShade = EShade::Red;
		else if (strStartsWithNoCase(sValue, COLOR_ALERT_SUCCESS))
			iShade = EShade::Green;
		else if (strStartsWithNoCase(sValue, COLOR_ALERT_WARNING))
			iShade = EShade::Orange;
		else if (strStartsWithNoCase(sValue, COLOR_AQUA))
			iShade = EShade::Blue;
		else if (strStartsWithNoCase(sValue, COLOR_BLUE))
			iShade = EShade::Blue;
		else if (strStartsWithNoCase(sValue, COLOR_CERULEAN))
			iShade = EShade::Blue;
		else if (strStartsWithNoCase(sValue, COLOR_GREEN))
			iShade = EShade::Green;
		else if (strStartsWithNoCase(sValue, COLOR_LIME))
			iShade = EShade::Green;
		else if (strStartsWithNoCase(sValue, COLOR_ORANGE))
			iShade = EShade::Orange;
		else if (strStartsWithNoCase(sValue, COLOR_PRIMARY))
			iShade = EShade::Primary;
		else if (strStartsWithNoCase(sValue, COLOR_PURPLE))
			iShade = EShade::Purple;
		else if (strStartsWithNoCase(sValue, COLOR_RED))
			iShade = EShade::Red;
		else if (strStartsWithNoCase(sValue, COLOR_SECONDARY))
			iShade = EShade::Secondary;
		else if (strStartsWithNoCase(sValue, COLOR_YELLOW))
			iShade = EShade::Yellow;
		else
			return Default;

		const char *pPos = sValue.GetParsePointer();
		while (*pPos != '_' && *pPos != '\0')
			pPos++;

		if (*pPos == '\0')
			return CLuminousColor(iShade, 60);
		else
			{
			CString sVariant = CString(pPos + 1);
			if (strEqualsNoCase(sVariant, VARIANT_FADED))
				return CLuminousColor(iShade, 80);
			else if (strEqualsNoCase(sVariant, VARIANT_INK))
				return CLuminousColor(iShade, 100);
			else if (strEqualsNoCase(sVariant, VARIANT_PAPER))
				return CLuminousColor(iShade, 10);
			else if (strEqualsNoCase(sVariant, VARIANT_TEXT))
				return CLuminousColor(iShade, 10);
			else
				return CLuminousColor(iShade, 60);
			}
		}
	}

void CLuminousColor::Write (IByteStream& Stream) const

//	Write
//
//	Write to a stream.

	{
	switch (m_iType)
		{
		case EType::None:
			Stream.Write(TYPE_NONE);
			break;

		case EType::Clear:
			Stream.Write(TYPE_CLEAR);
			break;

		case EType::Solid:
			Stream.Write(TYPE_SOLID);
			Stream.Write(m_Value.rgbColor.AsDWORD());
			break;

		case EType::Theme:
			Stream.Write(TYPE_THEME);
			Stream.Write(EncodeThemeColor(m_Value.ThemeColor.iShade, m_Value.ThemeColor.iToneIndex));
			break;

		case EType::Role:
			Stream.Write(TYPE_ROLE);
			Stream.Write(EncodeRole(m_Value.iRole));
			break;

		default:
			throw CException(errFail);
		}
	}

