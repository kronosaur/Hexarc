//	CXLSXFTable.cpp
//
//	CXLSXFTable class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

std::initializer_list<CXLSXFTable::SXF> CXLSXFTable::m_Defaults = {

	//	First 15 entries are default styles

	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED | FLAG_STYLE
		},

	//	This is the default cell XF

	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		0,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CRGBA32(0, 0, 0),	CRGBA32(255, 255, 255),
		FLAG_LOCKED
		},
};

CXLSXFTable::CXLSXFTable ()

//	CXLSXFTable constructor

	{
	m_Table.GrowToFit((int)m_Defaults.size());
	for (const auto &Entry : m_Defaults)
		{
		m_Table.Insert(Entry);
		}
	}
