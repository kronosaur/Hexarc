//	CLuminousLineStyle.cpp
//
//	CLuminousLineStyle Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

const CLuminousLineStyle CLuminousLineStyle::Null;

bool CLuminousLineStyle::operator== (const CLuminousLineStyle &Src) const

//	CLuminousLineStyle operator==

	{
	return (m_Color == Src.m_Color
			&& m_rLineWidth == Src.m_rLineWidth
			&& m_iLineCap == Src.m_iLineCap
			&& m_iLineJoin == Src.m_iLineJoin
			&& m_rMiterLimit == Src.m_rMiterLimit);
	}
