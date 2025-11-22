//	CLuminousLineStyle.cpp
//
//	CLuminousLineStyle Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

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

CLuminousLineStyle CLuminousLineStyle::CreateFromStream (IByteStream& Stream)
	{
	CLuminousLineStyle Result;
	Result.m_Color.Read(Stream);
	Result.m_rLineWidth = Stream.ReadDouble();
	Result.m_iLineCap = (ELineCap)Stream.ReadInt();
	Result.m_iLineJoin = (ELineJoin)Stream.ReadInt();
	Result.m_rMiterLimit = Stream.ReadDouble();

	return Result;
	}

void CLuminousLineStyle::Write (IByteStream& Stream) const
	{
	m_Color.Write(Stream);
	Stream.Write(m_rLineWidth);
	Stream.Write((int)m_iLineCap);
	Stream.Write((int)m_iLineJoin);
	Stream.Write(m_rMiterLimit);
	}
