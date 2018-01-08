//	Geometry.cpp
//
//	Math functions and classes
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>

inline bool IntCCW (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC)

//	IntCCW
//
//	Returns TRUE if the three points are in counter-clockwise order

	{
	return (vC.Y() - vA.Y()) * (vB.X() - vA.X()) > (vB.Y() - vA.Y()) * (vC.X() - vA.X());
	}

bool mathLinesIntersect (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, const CVector2D &vD)

//	mathLinesIntersect
//
//	Returns TRUE if line segments AB and CD intersect.

	{
	return (IntCCW(vA, vC, vD) != IntCCW(vB, vC, vD))
			&& (IntCCW(vA, vB, vC) != IntCCW(vA, vB, vD));
	}
