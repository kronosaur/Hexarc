//	CLuminousColor.cpp
//
//	CLuminousColor Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

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
			if (m_rgbColor != Src.m_rgbColor)
				return false;
			break;

		default:
			throw CException(errFail);
		}

	return true;
	}
