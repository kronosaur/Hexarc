//	CXLSXFTable.cpp
//
//	CXLSXFTable class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

std::initializer_list<CXLSXFTable::SXF> CXLSXFTable::m_Defaults = {

	//	First 15 entries are default styles

	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},
	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		NO_PARENT,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
		FLAG_LOCKED | FLAG_STYLE
		},

	//	This is the default cell XF

	{	CXLSFontTable::DEFAULT_FONT_ID,		FORMAT_GENERAL,		0,	EAlign::General,	EVerticalAlign::Top,	0,	0,	EReadOrder::Context,
		SBorder(),	SBorder(),	SBorder(),	SBorder(),		EDiagonal::None,	SBorder(),	EPattern::None,		CXLSColorPalette::DEFAULT_BLACK,	CXLSColorPalette::DEFAULT_WHITE,
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

	//	Add the last XF entry, because this is the cell default.

	int *pIndex = m_Index.SetAt(m_Table[15]);
	*pIndex = 15;
	}

CXLSXFTable::SXF CXLSXFTable::CreateXF (int iFormat, const CDBFormatDesc &Desc)

//	CreateXF
//
//	Creates an XF definition.

	{
	SXF XF = GetXF(DEFAULT_CELL_XF);
	XF.iFormat = iFormat;
	XF.iFont = m_Fonts.GetFont(Desc);

	return XF;
	}

int CXLSXFTable::GetXF (int iFormat, const CDBFormatDesc &Desc)

//	GetXF
//
//	Returns an XF index for the appropriate format. We add a new one, if 
//	necessary.

	{
	SXF XF = CreateXF(iFormat, Desc);

	//	Check the index.

	bool bNew;
	int *pXF = m_Index.SetAt(XF, &bNew);
	if (bNew)
		{
		*pXF = m_Table.GetCount();
		m_Table.Insert(XF);
		}

	return *pXF;
	}

int KeyCompare (const CXLSXFTable::SXF &Key1, const CXLSXFTable::SXF &Key2)

//	KeyCompare
//
//	Case-sensitive, non-locale specific comparison
//
//	0 if Key1 == Key2
//	1 if Key1 > Key2
//	-1 if Key1 < Key2

	{
	if (Key1.iFont > Key2.iFont)
		return 1;
	else if (Key1.iFont < Key2.iFont)
		return -1;
	else if (Key1.iFormat > Key2.iFormat)
		return 1;
	else if (Key1.iFormat < Key2.iFormat)
		return -1;
	else if (Key1.iParentXF > Key2.iParentXF)
		return 1;
	else if (Key1.iParentXF < Key2.iParentXF)
		return -1;
	else if ((int)Key1.iAlign > (int)Key2.iAlign)
		return 1;
	else if ((int)Key1.iAlign < (int)Key2.iAlign)
		return -1;
	else if ((int)Key1.iVerticalAlign > (int)Key2.iVerticalAlign)
		return 1;
	else if ((int)Key1.iVerticalAlign < (int)Key2.iVerticalAlign)
		return -1;
	else if (Key1.iRotation > Key2.iRotation)
		return 1;
	else if (Key1.iRotation < Key2.iRotation)
		return -1;
	else if (Key1.iIndentLevel > Key2.iIndentLevel)
		return 1;
	else if (Key1.iIndentLevel < Key2.iIndentLevel)
		return -1;
	else if ((int)Key1.iReadOrder > (int)Key2.iReadOrder)
		return 1;
	else if ((int)Key1.iReadOrder < (int)Key2.iReadOrder)
		return -1;
	else if (int iCompare = ::KeyCompare(Key1.LeftBorder, Key2.LeftBorder))
		return iCompare;
	else if (int iCompare = ::KeyCompare(Key1.RightBorder, Key2.RightBorder))
		return iCompare;
	else if (int iCompare = ::KeyCompare(Key1.TopBorder, Key2.TopBorder))
		return iCompare;
	else if (int iCompare = ::KeyCompare(Key1.BottomBorder, Key2.BottomBorder))
		return iCompare;
	else if ((int)Key1.iDiagonal > (int)Key2.iDiagonal)
		return 1;
	else if ((int)Key1.iDiagonal < (int)Key2.iDiagonal)
		return -1;
	else if (int iCompare = ::KeyCompare(Key1.DiagonalBorder, Key2.DiagonalBorder))
		return iCompare;
	else if ((int)Key1.iFillPattern> (int)Key2.iFillPattern)
		return 1;
	else if ((int)Key1.iFillPattern < (int)Key2.iFillPattern)
		return -1;
	else if (Key1.iPatternColor > Key2.iPatternColor)
		return 1;
	else if (Key1.iPatternColor < Key2.iPatternColor)
		return -1;
	else if (Key1.iPatternBackColor > Key2.iPatternBackColor)
		return 1;
	else if (Key1.iPatternBackColor < Key2.iPatternBackColor)
		return -1;
	else if (Key1.dwFlags > Key2.dwFlags)
		return 1;
	else if (Key1.dwFlags < Key2.dwFlags)
		return -1;
	else
		return 0;
	}

int KeyCompare (const CXLSXFTable::SBorder &Key1, const CXLSXFTable::SBorder &Key2)

//	KeyCompare
//
//	Case-sensitive, non-locale specific comparison
//
//	0 if Key1 == Key2
//	1 if Key1 > Key2
//	-1 if Key1 < Key2

	{
	if ((int)Key1.iStyle > (int)Key2.iStyle)
		return 1;
	else if ((int)Key1.iStyle > (int)Key2.iStyle)
		return -1;
	else if (Key1.iColor > Key2.iColor)
		return 1;
	else if (Key1.iColor < Key2.iColor)
		return -1;
	else
		return 0;
	}

