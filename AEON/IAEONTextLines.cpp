//	IAEONTextLines.cpp
//
//	IAEONTextLines class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

int IAEONTextLines::Compare (const IAEONTextLines& Src) const

//	Compare
//
//	-1:		If this < Src
//	0:		If this == Src
//	1:		If this > Src

	{
	if (GetLineCount() == Src.GetLineCount())
		{
		for (int i = 0; i < GetLineCount(); i++)
			{
			int iComp = ::KeyCompare(GetLine(i), Src.GetLine(i));
			if (iComp != 0)
				return iComp;
			}

		return 0;
		}
	else if (GetLineCount() < Src.GetLineCount())
		return -1;
	else
		return 1;
	}

int IAEONTextLines::CompareNoCase (const IAEONTextLines& Src) const

//	CompareNoCase
//
//	-1:		If this < Src
//	0:		If this == Src
//	1:		If this > Src

	{
	if (GetLineCount() == Src.GetLineCount())
		{
		for (int i = 0; i < GetLineCount(); i++)
			{
			int iComp = ::KeyCompareNoCase(GetLine(i), Src.GetLine(i));
			if (iComp != 0)
				return iComp;
			}

		return 0;
		}
	else if (GetLineCount() < Src.GetLineCount())
		return -1;
	else
		return 1;
	}
