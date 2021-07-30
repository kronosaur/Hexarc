//	CXLSFontTable.cpp
//
//	CXLSFontTable class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

std::initializer_list<CXLSFontTable::SFont> CXLSFontTable::m_Defaults = {
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	EItalic::None,	EStrikeout::None,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	EItalic::None,	EStrikeout::None,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	EItalic::None,	EStrikeout::None,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	EItalic::None,	EStrikeout::None,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Unknown,	"",			0,			0,				EItalic::None,	EStrikeout::None,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	EItalic::None,	EStrikeout::None,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
};

CXLSFontTable::CXLSFontTable ()

//	CXLSFontTable constructor

	{
	m_Table.GrowToFit((int)m_Defaults.size());
	for (const auto &Entry : m_Defaults)
		{
		m_Table.Insert(Entry);
		}
	}

CXLSFontTable::SFont CXLSFontTable::Create (const CDBFormatDesc &Desc)

//	Create
//
//	Creates a font entry based on a descriptor.

	{
	//	Start with defaults.

	SFont Result = m_Table[0];

	//	Modify based on descriptor.

	const CRGBA32 &rgbColor = Desc.GetColor();
	if (!rgbColor.IsNull())
		Result.Color = rgbColor;

	return Result;
	}

bool CXLSFontTable::Equals (const SFont &Src, const SFont &Dest)

//	Equals
//
//	Returns TRUE if Src is equal to Dest.

	{
	if (Src.iFamily != Dest.iFamily)
		return false;

	if (!strEquals(Src.sName, Dest.sName))
		return false;

	if (Src.iSize != Dest.iSize)
		return false;

	if (Src.iWeight != Dest.iWeight)
		return false;

	if (Src.iItalic != Dest.iItalic)
		return false;

	if (Src.iStrikeout != Dest.iStrikeout)
		return false;

	if (Src.iUnderline != Dest.iUnderline)
		return false;

	if (Src.iEscapement != Dest.iEscapement)
		return false;

	if (Src.Color != Dest.Color)
		return false;

	if (Src.iCharacterSet != Dest.iCharacterSet)
		return false;

	return true;
	}

int CXLSFontTable::GetFont (const CDBFormatDesc &Desc)

//	GetFont
//
//	Returns the index to a font matching the given descriptor. We create a new
//	entry, if necessary.

	{
	if (Desc.IsEmpty())
		return DEFAULT_FONT_ID;

	SFont DesiredFont = Create(Desc);
	for (int i = 0; i < m_Table.GetCount(); i++)
		if (Equals(DesiredFont, m_Table[i]))
			return i;

	int iNewIndex = m_Table.GetCount();
	m_Table.Insert(DesiredFont);
	return iNewIndex;
	}

