//	CLuminousPath2D.cpp
//
//	CLuminousPath2D Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

const CLuminousPath2D CLuminousPath2D::Null;

bool CLuminousPath2D::operator== (const CLuminousPath2D &Src) const

//	CLuminousPath2D operator ==

	{
	if (m_Path.GetCount() != Src.m_Path.GetCount())
		return false;

	for (int i = 0; i < m_Path.GetCount(); i++)
		{
		if (m_Path[i] != Src.m_Path[i])
			return false;
		}

	return true;
	}

void CLuminousPath2D::Arc (const CVector2D& vCenter, double rRadius, double rStartAngle, double rEndAngle, bool bCounterClockwise)

//	Arc
//
//	Adds a new arc to the path.

	{
	auto& NewSegment = *m_Path.Insert();
	if (bCounterClockwise)
		NewSegment.iType = ESegmentType::ArcCounterClockwise;
	else
		NewSegment.iType = ESegmentType::ArcClockwise;

	NewSegment.A = vCenter;
	NewSegment.B = CVector2D(rRadius, 0.0);
	NewSegment.C = CVector2D(rStartAngle, rEndAngle);
	}

void CLuminousPath2D::ArcTo (const CVector2D& v1stTangent, const CVector2D& v2ndTangent, double rRadius)

//	ArcTo
//
//	Adds an arc from the current path position.

	{
	auto& NewSegment = *m_Path.Insert();
	NewSegment.iType = ESegmentType::ArcTo;
	NewSegment.A = v1stTangent;
	NewSegment.B = v2ndTangent;
	NewSegment.C = CVector2D(rRadius, 0.0);
	}

void CLuminousPath2D::ClosePath ()

//	ClosePath
//
//	Closes the path.

	{
	auto& NewSegment = *m_Path.Insert();
	NewSegment.iType = ESegmentType::ClosePath;
	}

void CLuminousPath2D::GetArc (int iIndex, CVector2D& retCenter, double& retRadius, double& retStartAngle, double& retEndAngle, bool& retCounterClockwise) const

//	GetArc
//
//	Get arc parameters.

	{
	if (iIndex < 0 || iIndex >= m_Path.GetCount() 
			|| (m_Path[iIndex].iType != ESegmentType::ArcClockwise && (m_Path[iIndex].iType != ESegmentType::ArcCounterClockwise)))
		throw CException(errFail);

	retCenter = m_Path[iIndex].A;
	retRadius = m_Path[iIndex].B.X();
	retStartAngle = m_Path[iIndex].C.X();
	retEndAngle = m_Path[iIndex].C.Y();
	retCounterClockwise = (m_Path[iIndex].iType == ESegmentType::ArcCounterClockwise);
	}

void CLuminousPath2D::GetArcTo (int iIndex, CVector2D& retTangent1, CVector2D& retTangent2, double& retRadius) const

//	GetArcTo
//
//	Get arcTo parameters.

	{
	if (iIndex < 0 || iIndex >= m_Path.GetCount() 
			|| m_Path[iIndex].iType != ESegmentType::ArcTo)
		throw CException(errFail);

	retTangent1 = m_Path[iIndex].A;
	retTangent2 = m_Path[iIndex].B;
	retRadius = m_Path[iIndex].C.X();
	}

void CLuminousPath2D::GetRect (int iIndex, CVector2D& retUL, CVector2D& retLR) const

//	GetRect
//
//	Get rect parameters.

	{
	if (iIndex < 0 || iIndex >= m_Path.GetCount() 
			|| m_Path[iIndex].iType != ESegmentType::Rect)
		throw CException(errFail);

	retUL = m_Path[iIndex].A;
	retLR = m_Path[iIndex].B;
	}

void CLuminousPath2D::LineTo (const CVector2D& vPos)

//	LineTo
//
//	Add to the path.

	{
	auto& NewSegment = *m_Path.Insert();
	NewSegment.iType = ESegmentType::LineTo;
	NewSegment.A = vPos;
	}

void CLuminousPath2D::MoveTo (const CVector2D& vPos)

//	MoveTo
//
//	Moves the current path position.

	{
	auto& NewSegment = *m_Path.Insert();
	NewSegment.iType = ESegmentType::MoveTo;
	NewSegment.A = vPos;
	}

void CLuminousPath2D::Rect (const CVector2D& vUL, const CVector2D& vLR)

//	Rect
//
//	Adds a rectangle path.

	{
	auto& NewSegment = *m_Path.Insert();
	NewSegment.iType = ESegmentType::Rect;
	NewSegment.A = vUL;
	NewSegment.B = vLR;
	}
