//	CLuminousShadowStyle.cpp
//
//	CLuminousShadowStyle Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

const CLuminousShadowStyle CLuminousShadowStyle::Null;

bool CLuminousShadowStyle::operator== (const CLuminousShadowStyle &Src) const

//	CLuminousShadowStyle operator==

	{
	return (m_Color == Src.m_Color
			&& m_rBlur == Src.m_rBlur
			&& m_rOffsetX == Src.m_rOffsetX
			&& m_rOffsetY == Src.m_rOffsetY);
	}
