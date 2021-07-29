//	CXLSFontTable.cpp
//
//	CXLSFontTable class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

std::initializer_list<CXLSFontTable::SFont> CXLSFontTable::m_Defaults = {
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	false,	false,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	false,	false,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	false,	false,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	false,	false,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Unknown,	"",			0,			0,				false,	false,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
	{	EFamily::Swiss,		"Calibri",	POINTS_11,	WEIGHT_NORMAL,	false,	false,	EUnderline::None, EEscapement::None, CRGBA32(0, 0, 0),	ECharacterSet::ANSILatin },
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
