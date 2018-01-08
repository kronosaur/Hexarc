//	CLine2D.cpp
//
//	CLine2D class
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>

void CLine2D::CalcCornerPoints (const CVector2D &From, const CVector2D &Center, const CVector2D &To, double rHalfWidth, CVector2D *retInner, CVector2D *retOuter)

//	CalcCornerPoints
//
//	Given two line segments joined at a common point (Center) and whose 
//	endpoints are From and To, we return two points, colinear with Center,
//	which define the corner points (where the line bends) for a line of
//	the given width.

	{
	//	Imagine two line segment: AB and BC (where A = From, B = Center,
	//	C = To).
	//
	//	GreatAngle is the angle formed from the line BC to BA

	double rBCAngle = (To - Center).Polar();
	double rBAAngle = (From - Center).Polar();
	double rGreatAngle = mathAngleDiff(rBCAngle, rBAAngle);

	//	CornerAngle is the angle from B (Center) to the outer corner. If is
	//	halfway between BC and BA.

	double rCornerAngle = mathAngleMod(rBCAngle + (0.5 * rGreatAngle));

	//	If GreatAngle is PI then A, B, and C are colinear

	if (rGreatAngle == PI || rGreatAngle == 0.0)
		{
		CVector2D ToCorner = CVector2D::FromPolar(rCornerAngle, rHalfWidth);
		if (retOuter)
			*retOuter = Center + ToCorner;
		if (retInner)
			*retInner = Center - ToCorner;
		}

	//	Otherwise, we need some more trig to compute the points

	else
		{
		//	Imagine a line parallel to BC and rHalfWidth away in the outer
		//	direction. This line is the outer outline. Imagine a point P
		//	on that line such that BCP form a right-triangle with B at the
		//	right-angle.
		//
		//	That is, the line BP is tangential to BC.
		//
		//	Thus the angle of the BP line is just 90 degrees (PI/2) more than
		//	the BC angle.
		//
		//	NOTE: If GreatAngle is less than 180 degrees then we have a
		//	concave angle. In that case, we flip P to a line parallel to BA.
		//	Everything else will work out the same.

		double rBPAngle = (rGreatAngle > PI ? rBCAngle + HALF_PI : rBAAngle - HALF_PI);

		//	Extending this parallel line will eventually get us to the corner,
		//	which we'll call point X. Thus BPX form a right-triangle with P at
		//	the right-angle and BX as the hypotenuse.
		//
		//	We eventually want to find the length of BX, so we try to figure out
		//	the angle formed by BXP.
		//
		//	We can easily compute the PBX angle because BX is along the 
		//	CornerAngle line.

		double rPBXAngle = mathAngleDiff(rBPAngle, rCornerAngle);

		//	Since we have a right-triangle, the BXP angle is just 90 - PBX.

		double rBXPAngle = HALF_PI - rPBXAngle;

		//	Now we can compute the distance from BX using SOH-CAH-TOA.

		double rSinBXP = sin(rBXPAngle);
		double rBX = (rSinBXP != 0.0 ? (rHalfWidth / rSinBXP) : rHalfWidth);

		//	We're done! The outer angle is positive; the inner one negative.

		CVector2D ToCorner = CVector2D::FromPolar(rCornerAngle, rBX);
		if (retOuter)
			*retOuter = Center + ToCorner;
		if (retInner)
			*retInner = Center - ToCorner;
		}
	}

double CLine2D::CalcDistanceToPoint (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, CVector2D *retvNearest)

//	CalcDistanceToPoint
//
//	Calculates the closest distance from point C to the line segment AB.

	{
	double rA, rB, rC;
	CalcLineEquation(vA, vB, &rA, &rB, &rC);

	//	Figure out the point on the line AB that is nearest to C.

	double rA2B2 = (rA * rA) + (rB * rB);
	if (rA2B2 == 0.0)
		{
		if (retvNearest)
			*retvNearest = vA;

		return (vA - vC).Length();
		}

	CVector2D vD((rB * (rB * vC.X() - rA * vC.Y()) - (rA * rC)) / rA2B2, (rA * (-rB * vC.X() + rA * vC.Y()) - (rB * rC)) / rA2B2);

	//	Now figure out the position of D in the line segment.

	CVector2D vBOffset = vB - vA;
	CVector2D vDOffset = vD - vA;

	//	If we're beyond B, then B is the closest point

	CVector2D vNearest;
	if ((vBOffset.X() > 0.0 && vDOffset.X() > vBOffset.X())
			|| (vBOffset.X() < 0.0 && vDOffset.X() < vBOffset.X()))
		vNearest = vB;

	//	Otherwise, if we're before A, then A is the closest point.

	else if ((vBOffset.X() > 0.0 && vDOffset.X() < 0.0)
			|| (vBOffset.X() < 0.0 && vDOffset.X() > 0.0))
		vNearest = vA;

	//	Otherwise we're in the segment

	else
		vNearest = vD;

	//	Done

	if (retvNearest)
		*retvNearest = vNearest;

	return (vC - vNearest).Length();
	}

CLine2D::EOpResult CLine2D::CalcIntersection (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, const CVector2D &vD, CVector2D *retIntersect, double *retAB, double *retCD, double rDenEpsilon)

//	CalcIntersection
//
//	Tests for intersection between two line segments and return the result, 
//	and optionally the intersection point.
//
//	retAB is the position of the intersection along the AB line (0 = A vertex; 1 = B vertex).
//	retCD is the position of the intersection along the CD line (0 = C vertex; 1 = B vertex).
//
//	See: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

	{
	double xAB = vB.X() - vA.X();
	double yAB = vB.Y() - vA.Y();
	double xCD = vD.X() - vC.X();
	double yCD = vD.Y() - vC.Y();

	double rDen = (xAB * yCD) - (yAB * xCD);
	if (Abs(rDen) <= rDenEpsilon)
		return resultCollinear;

	double xCA = vA.X() - vC.X();
	double yCA = vA.Y() - vC.Y();

	double rS = ((xAB * yCA) - (yAB * xCA)) / rDen;
	double rT = ((xCD * yCA) - (yCD * xCA)) / rDen;

	if (rS >= 0.0 && rS <= 1.0 && rT >= 0.0 && rT <= 1.0)
		{
		//	OK

		if (retIntersect)
			{
			retIntersect->SetX(vA.X() + (rT * xAB));
			retIntersect->SetY(vA.Y() + (rT * yAB));
			}

		if (retAB)
			*retAB = rT;

		if (retCD)
			*retCD = rS;

		return resultOK;
		}
	else
		return resultNone;
	}

void CLine2D::CalcLineEquation (const CVector2D &vP1, const CVector2D &vP2, double *retrA, double *retrB, double *retrC)

//	CalcLineEquation
//
//	Computes the line equation for a line passing through the two points:
//
//	a*x + b*y + c = 0
//
//	If vP1 and vP2 are the same, then we return 0 for all variables.

	{
	*retrA = (vP2.Y() - vP1.Y());
	*retrB = (vP1.X() - vP2.X());
	*retrC = (vP2.X() * vP1.Y()) - (vP1.X() * vP2.Y());
	}

bool CLine2D::IsCollinear (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, const CVector2D &vD, double rDenEpsilon)

//	IsCollinear
//
//	Returns TRUE if AB is collinear with CD

	{
	double xAB = vB.X() - vA.X();
	double yAB = vB.Y() - vA.Y();
	double xCD = vD.X() - vC.X();
	double yCD = vD.Y() - vC.Y();

	double rDen = (xAB * yCD) - (yAB * xCD);
	return (Abs(rDen) <= rDenEpsilon);
	}

CLine2D::EOpResult CLine2D::IsLeftOfLine (const CVector2D &vA, const CVector2D &vB, const CVector2D &vX)

//	IsLeftOfLine
//
//	Returns resultOK if point vX is to the left of a directional line starting at vA
//	and moving towards vB.
//
//	If vX collinear to AB then we return resultCollinear. Otherwise we return resultNone.
//
//	See: http://alienryderflex.com/point_left_of_ray/

	{
	double rW = (vX.Y() - vA.Y()) * (vB.X() - vA.X());
	double rZ = (vX.X() - vA.X()) * (vB.Y() - vA.Y());

	if (rW > rZ)
		return resultOK;
	else if (rW < rZ)
		return resultNone;
	else
		return resultCollinear;
	}
