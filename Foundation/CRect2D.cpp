//	CRect2D.cpp
//
//	CRect2D class
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>

void CRect2D::Expand (double rValue)

//	Expand
//
//	Expands the rect by the given value

	{
	m_vLL = CVector2D(m_vLL.X() - rValue, m_vLL.Y() - rValue);
	m_vUR = CVector2D(m_vUR.X() + rValue, m_vUR.Y() + rValue);
	}

bool CRect2D::LineIntersects (const CVector2D &vA, const CVector2D &vB) const

//	LineIntersects
//
//	Returns TRUE if the given line segment intersects.

	{
	//	First check to see if either of the end points is inside the rect. If
	//	so then we definitely intersect.

	if (PointIntersects(vA) || PointIntersects(vB))
		return true;

	//	Otherwise check to see if we intersect any of the lines of the rect.
	//	(This handles the case where both endpoints are outside the rect but
	//	they overlap a corner.)

	CVector2D vUL(m_vLL.X(), m_vUR.Y());
	CVector2D vLR(m_vUR.X(), m_vLL.Y());
	return (mathLinesIntersect(vA, vB, m_vLL, vUL)
			|| mathLinesIntersect(vA, vB, vUL, m_vUR)
			|| mathLinesIntersect(vA, vB, m_vUR, vLR)
			|| mathLinesIntersect(vA, vB, vLR, m_vLL));
	}

bool CRect2D::PointIntersects (const CVector2D &vA) const

//	PointIntersects
//
//	Returns TRUE if the point is inside the given rect.

	{
	return (vA.X() >= m_vLL.X()
			&& vA.X() < m_vUR.X()
			&& vA.Y() >= m_vLL.Y()
			&& vA.Y() < m_vUR.Y());
	}

bool CRect2D::RectIntersects (const CRect2D &Rect) const

//	RectIntersects
//
//	Returns TRUE if the two rects intersect

	{
	return (m_vUR.X() > Rect.m_vLL.X()
			&& m_vLL.X() < Rect.m_vUR.X()
			&& m_vUR.Y() > Rect.m_vLL.Y()
			&& m_vLL.Y() < Rect.m_vUR.Y());
	}

void CRect2D::Union (const CRect2D &Rect)

//	Union
//
//	Grows the rect to encompase the given rect

	{
	if (Rect.m_vUR.X() > m_vUR.X())
		m_vUR.SetX(Rect.m_vUR.X());
	if (Rect.m_vUR.Y() > m_vUR.Y())
		m_vUR.SetY(Rect.m_vUR.Y());
	if (Rect.m_vLL.X() < m_vLL.X())
		m_vLL.SetX(Rect.m_vLL.X());
	if (Rect.m_vLL.Y() < m_vLL.Y())
		m_vLL.SetY(Rect.m_vLL.Y());
	}
