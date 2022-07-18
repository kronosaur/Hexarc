//	CLuminousFillStyle.cpp
//
//	CLuminousFillStyle Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

const CLuminousFillStyle CLuminousFillStyle::Null;

bool CLuminousFillStyle::operator== (const CLuminousFillStyle &Src) const

//	CLuminousFillStyle operator==

	{
	if (m_Color != Src.m_Color)
		return false;

	return true;
	}
