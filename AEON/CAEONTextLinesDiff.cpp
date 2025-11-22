//	CAEONTextLinesDiff.cpp
//
//	CAEONTextLinesDiff class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CAEONTextLinesDiff::CAEONTextLinesDiff (CDatum dDiff)

//	CAEONTextLinesDiff constructor

	{
	m_Diff.GrowToFit(dDiff.GetCount());
	for (int i = 0; i < dDiff.GetCount(); i++)
		{
		CDatum dEntry = dDiff.GetElement(i);
		CDatum dType = dEntry.GetElement(0);
		CDatum dParam = dEntry.GetElement(1);
		char chType = *(dType.AsStringView()).GetParsePointer();

		switch (chType)
			{
			case 'D':
				m_Diff.Insert({ EType::Delete, dParam });
				break;

			case 'I':
				m_Diff.Insert({ EType::Insert, dParam });
				break;

			case 'M':
				m_Diff.Insert({ EType::Same, dParam });
				break;

			default:
				break;
			}
		}
	}

void CAEONTextLinesDiff::Mark ()

//	Mark
//
//	Mark data in use
	
	{
	for (int i = 0; i < m_Diff.GetCount(); i++) m_Diff[i].dParam.Mark();
	}
