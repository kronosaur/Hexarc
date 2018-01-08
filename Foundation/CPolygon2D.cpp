//	CPolygon2D.cpp
//
//	CPolygon2D class
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>
#include "PolygonIntersect.h"

CPolygon2D::CPolygon2D (int iPoints)

//	CPolygon2D constructor

	{
	ASSERT(iPoints >= 0);
	m_vPoints.InsertEmpty(iPoints);
	}

CPolygon2D::CPolygon2D (const CPolygon2D &Src)

//	CPolygon2D constructor

	{
	Copy(Src);
	}

CPolygon2D::CPolygon2D (const TArray<CVector2D> &vPoints) :
		m_vPoints(vPoints)

//	CPolygon2D constructor

	{
	}

CPolygon2D::~CPolygon2D (void)

//	CPolygon2D destructor

	{
	CleanUp();
	}

CPolygon2D &CPolygon2D::operator = (const CPolygon2D &Src)

//	CPolygon2D operator =

	{
	CleanUp();
	Copy(Src);
	return *this;
	}

void CPolygon2D::AddHole (const CPolygon2D &HolePoly)

//	AddHole
//
//	Adds the given polygon as a hole. NOTE: We assume that the hole is completely
//	enclosed by this polygon and that it does not overlap any other holes.

	{
	if (HolePoly.GetVertexCount() < 3)
		return;

	CPolygon2D *pNewHole = new CPolygon2D;
	m_Holes.Insert(pNewHole);

	pNewHole->m_vPoints = HolePoly.m_vPoints;
	}

void CPolygon2D::AddHoleHandoff (CPolygon2D &HolePoly)

//	AddHoleHandoff
//
//	Adds the given polygon as a hole. NOTE: We assume that the hole is completely
//	enclosed by this polygon and that it does not overlap any other holes.

	{
	if (HolePoly.GetVertexCount() < 3)
		return;

	CPolygon2D *pNewHole = new CPolygon2D;
	m_Holes.Insert(pNewHole);

	pNewHole->m_vPoints.TakeHandoff(HolePoly.m_vPoints);

	//	We don't take any holes in HolePoly (no need to recurse because any
	//	polygons inside our hole can just be separate polygons).
	}

void CPolygon2D::CalcBoundingRect (CRect2D *retRect) const

//	CalcBoundingRect
//
//	Computes a bounding rect for the polygon.
//	NOTE: Undefined for empty polygons.

	{
	int i;

	//	Short-circuit

	if (m_vPoints.GetCount() == 0)
		{
		*retRect = CRect2D();
		return;
		}

	CVector2D vLL(m_vPoints[0]);
	CVector2D vUR(m_vPoints[0]);

	for (i = 1; i < m_vPoints.GetCount(); i++)
		{
		const CVector2D &vPoint = m_vPoints[i];

		if (vPoint.X() > vUR.X())
			vUR.SetX(vPoint.X());
		else if (vPoint.X() < vLL.X())
			vLL.SetX(vPoint.X());

		if (vPoint.Y() > vUR.Y())
			vUR.SetY(vPoint.Y());
		else if (vPoint.Y() < vLL.Y())
			vLL.SetY(vPoint.Y());
		}

	retRect->SetLL(vLL);
	retRect->SetUR(vUR);
	}

void CPolygon2D::CleanUp (void)

//	CleanUp
//
//	Clean up

	{
	int i;

	for (i = 0; i < m_Holes.GetCount(); i++)
		delete m_Holes[i];

	m_Holes.DeleteAll();
	}

void CPolygon2D::Copy (const CPolygon2D &Src)

//	Copy
//
//	Copy from Src. We assume that we are empty

	{
	int i;

	m_vPoints = Src.m_vPoints;

	m_Holes.InsertEmpty(Src.m_Holes.GetCount());
	for (i = 0; i < Src.m_Holes.GetCount(); i++)
		{
		CPolygon2D *pNewHole = new CPolygon2D(*Src.m_Holes[i]);
		m_Holes[i] = pNewHole;
		}
	}

void CPolygon2D::DeleteHole (int iIndex)

//	DeleteHole
//
//	Deletes the given hole

	{
	ASSERT(iIndex >= 0 && iIndex < m_Holes.GetCount());

	delete m_Holes[iIndex];
	m_Holes.Delete(iIndex);
	}

void CPolygon2D::Expand (double rExpansion, CPolygon2D *retResult, bool bNoValidityCheck) const

//	Expand
//
//	Expands the polygon by the given distance (if negative, we contract instead).

	{
	int i;
	int iCount = GetVertexCount();

	//	Short-circuit

	*retResult = CPolygon2D();
	if (iCount < 3)
		return;

	//	We always want to traverse the polygon in clockwise order, so if the
	//	vertices are counter-clockwise we traverse backwards.

	int iStart;
	int iEnd;
	int iDir;
	if (GetVertexOrder() == dirClockwise)
		{
		iStart = 0;
		iEnd = iCount;
		iDir = 1;
		}
	else
		{
		iStart = iCount - 1;
		iEnd = -1;
		iDir = -1;
		}

	//	The half-width is the expansion distance

	bool bUseOuter = (rExpansion > 0);
	double rHalfWidth = Abs(rExpansion);

	//	Compute a point for each vertex

	TArray<CVector2D> Points;
	Points.InsertEmpty(iCount);

	int iDest;
	for (i = iStart, iDest = 0; i != iEnd; i += iDir, iDest++)
		{
		const CVector2D &vFrom = m_vPoints[(i - iDir + iCount) % iCount];
		const CVector2D &vTo = m_vPoints[(i + iDir + iCount) % iCount];

		CLine2D::CalcCornerPoints(vFrom, m_vPoints[i], vTo, rHalfWidth,
				(bUseOuter ? NULL : &Points[iDest]),
				(bUseOuter ? &Points[iDest] : NULL));

		//	If we're contracting and this point is NOT inside the original shape, 
		//	then we won't end up with a valid polygon, so we abort.

		if (!bNoValidityCheck
				&& !bUseOuter 
				&& !PointIntersects(Points[iDest], false))
			return;
		}

	//	Done with main outline

	retResult->TakeHandoff(Points);

	//	If we have any holes, then we add those too (but we reverse the 
	//	expansion to a contraction, or vice versa).

	for (i = 0; i < GetHoleCount(); i++)
		{
		CPolygon2D NewHole;
		m_Holes[i]->Expand(-rExpansion, &NewHole);
		if (NewHole.GetVertexCount() >= 3)
			retResult->AddHoleHandoff(NewHole);
		}
	}

bool CPolygon2D::FindVertex (const CVector2D &vPos, double rEpsilon2, int *retiIndex) const

//	FindVertex
//
//	Returns TRUE (and optionally an index) if a vertex in this polygon is equal
//	to vPos (within the square root of rEpsilon2).
//
//	[Excludes any hole vertices.]

	{
	int i;

	for (i = 0; i < m_vPoints.GetCount(); i++)
		if (m_vPoints[i].IsEqualTo(vPos, rEpsilon2))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

void CPolygon2D::GetHoleListHandoff (TArray<CPolygon2D> *retResult)

//	GetHoleListHandoff
//
//	Hands-off the holes in this polygon to the result.

	{
	int i;

	retResult->DeleteAll();
	retResult->InsertEmpty(m_Holes.GetCount());

	for (i = 0; i < m_Holes.GetCount(); i++)
		{
		retResult->GetAt(i).TakeHandoff(m_Holes[i]->m_vPoints);
		delete m_Holes[i];
		}

	m_Holes.DeleteAll();
	}

ERotationDirections CPolygon2D::GetVertexOrder (void) const

//	GetVertexOrder
//
//	Returns the direction of the vertices, clockwise or counter-clockwise.
//
//	We figure this out by a determinant.
//	See: https://en.wikipedia.org/wiki/Curve_orientation#Orientation_of_a_simple_polygon

	{
	int i;

	if (m_vPoints.GetCount() < 3)
		return dirNone;

	//	Look for a the vertex with the smallest y coordinate out of all the smallest
	//	x coordinates.

	int iVB = 0;
	double rSmallestX = m_vPoints[0].X();
	double rSmallestY = m_vPoints[0].Y();
	for (i = 1; i < m_vPoints.GetCount(); i++)
		{
		if (m_vPoints[i].X() < rSmallestX
				|| (m_vPoints[i].X() == rSmallestX && m_vPoints[i].Y() < rSmallestY))
			{
			iVB = i;
			rSmallestX = m_vPoints[i].X();
			rSmallestY = m_vPoints[i].Y();
			}
		}

	//	We have three points, A, B, and C:

	const CVector2D &vA = m_vPoints[(iVB + m_vPoints.GetCount() - 1) % m_vPoints.GetCount()];
	const CVector2D &vB = m_vPoints[iVB];
	const CVector2D &vC = m_vPoints[(iVB + 1) % m_vPoints.GetCount()];

	//	Compute the determinant

	double rDet = (vB.X() * vC.Y() + vA.X() * vB.Y() + vA.Y() * vC.X()) - (vA.Y() * vB.X() + vB.Y() * vC.X() + vA.X() * vC.Y());
	if (rDet < 0.0)
		return dirClockwise;
	else
		return dirCounterClockwise;
	}

bool CPolygon2D::HasVertex (const CVector2D &vPos, double rEpsilon2) const

//	HasVertex
//
//	Returns TRUE if the given polygon has vPos as one of its vertices (including
//	hole vertices).

	{
	int i;

	if (FindVertex(vPos, rEpsilon2))
		return true;

	for (i = 0; i < m_Holes.GetCount(); i++)
		if (m_Holes[i]->FindVertex(vPos, rEpsilon2))
			return true;

	return false;
	}

CPolygon2D::EOpResult CPolygon2D::IntersectPolygon (const TArray<CPolygon2D> &ClipRegion, TArray<CPolygon2D> *retResult) const

//	IntersectPolygon
//
//	Intersects this (subject) polygon with an array of clip polygons

	{
	CPolygonIntersector Intersector;
	Intersector.SetClipRegion(ClipRegion);
	Intersector.SetSubjectRegion(*this);

	return Intersector.Intersect(retResult);
	}

CPolygon2D::EOpResult CPolygon2D::IntersectPolygon (const CPolygon2D &ClipPoly, TArray<CPolygon2D> *retResult) const

//	IntersectPolygon
//
//	Intersects this (subject) polygon with the given clipping polygon and 
//	optionally returns a list of resulting polygons. If the list is empty, then
//	there is no intersection.
//
//	Based On:
//	HIDDEN SURFACE REMOVAL USING POLYGON AREA SORTING
//	by Kevin Weiler and Peter Atherton

	{
	CPolygonIntersector Intersector;
	Intersector.SetClipRegion(ClipPoly);
	Intersector.SetSubjectRegion(*this);

	return Intersector.Intersect(retResult);
	}

bool CPolygon2D::IsSelfIntersecting (void) const

//	IsSelfIntersecting
//
//	Returns TRUE if the polygon segments are self-intersecting. We use a naive
//	O(n^2) test, but that should be OK for must small polygons.
//
//	NOTE: This will not detect overlapping segments, nor will it handle 0-length
//	(or tiny) segments.

	{
	int i, j;

	//	Short-circuit. Polygons with less than 4 sides cannot be self-
	//	intersecting.

	int iCount = m_vPoints.GetCount();
	if (iCount < 4)
		return false;

	//	Loop over all segments

	for (i = 0; i < iCount; i++)
		{
		int iB = (i + 1) % iCount;
		const CVector2D &vA = m_vPoints[i];
		const CVector2D &vB = m_vPoints[iB];

		//	We start at 2 because adjacent segments cannot intersect.

		for (j = i + 2; j < m_vPoints.GetCount(); j++)
			{
			int iD = (j + 1) % iCount;
			if (iD == i)
				continue;

			const CVector2D &vC = m_vPoints[j];
			const CVector2D &vD = m_vPoints[iD];

			if (mathLinesIntersect(vA, vB, vC, vD))
				return true;
			}
		}

	//	If we get this far then we do not intersect

	return false;
	}

bool CPolygon2D::LineIntersects (const CVector2D &vA, const CVector2D &vB) const

//	LineIntersects
//
//	Returns TRUE if the polygon intersects the given line segment.

	{
	int i, j;
	bool bAInside = false;
	bool bBInside = false;

	//	Loop over all line segments, including holes, starting with the outer
	//	countours.

	const TArray<CVector2D> *pPoints = &m_vPoints;
	int iNextHole = 0;

	while (true)
		{
		//	Loop over all line segments of this countour, checking for 
		//	intersection.

		for (i = 0, j = pPoints->GetCount() - 1; i < pPoints->GetCount(); j = i++)
			{
			const CVector2D &vFrom = pPoints->GetAt(i);
			const CVector2D &vTo = pPoints->GetAt(j);

			//	If we intersect one of the polygon edges, then we definitely intersect

			if (mathLinesIntersect(vA, vB, vFrom, vTo))
				return true;

			//	Track to see if point A is inside the polygon

			if ((vFrom.Y() >= vA.Y()) != (vTo.Y() >= vA.Y())
					&& (vA.X() <= (vTo.X() - vFrom.X()) * (vA.Y() - vFrom.Y()) / (vTo.Y() - vFrom.Y()) + vFrom.X()))
				bAInside = !bAInside;

			//	Track to see if point B is inside the polygon

			if ((vFrom.Y() >= vB.Y()) != (vTo.Y() >= vB.Y())
					&& (vB.X() <= (vTo.X() - vFrom.X()) * (vB.Y() - vFrom.Y()) / (vTo.Y() - vFrom.Y()) + vFrom.X()))
				bBInside = !bBInside;
			}

		//	Next hole (or done)

		if (iNextHole < m_Holes.GetCount())
			pPoints = &m_Holes[iNextHole++]->m_vPoints;
		else
			break;
		}

	return (bAInside || bBInside);
	}

#ifdef DEBUG
void CPolygon2D::OutputPolygon (const CString &sName) const
	{
	int i;

	printf("%s\n", (LPSTR)sName);
	for (i = 0; i < GetVertexCount(); i++)
		printf("'(%.20f %.20f)\n", GetVertex(i).X(), GetVertex(i).Y());

	//	Holes

	for (i = 0; i < GetHoleCount(); i++)
		GetHole(i).OutputPolygon(strPattern("%s Hole %d", sName, i));
	}
#endif

bool CPolygon2D::PointIntersects (const CVector2D &vA, bool bIncludeHoles) const

//	PointIntersects
//
//	Returns TRUE if the given point is inside the polygon

	{
	int i, j;
	bool bInside = false;

	for (i = 0, j = m_vPoints.GetCount() - 1; i < m_vPoints.GetCount(); j = i++)
		{
		if ((m_vPoints[i].Y() >= vA.Y()) != (m_vPoints[j].Y() >= vA.Y())
				&& (vA.X() <= (m_vPoints[j].X() - m_vPoints[i].X()) * (vA.Y() - m_vPoints[i].Y()) / (m_vPoints[j].Y() - m_vPoints[i].Y()) + m_vPoints[i].X()))
			bInside = !bInside;
		}

	//	If we're not inside the outer bounds, then we're done

	if (!bInside)
		return false;

	//	Otherwise, check to see if we're inside a hole. If so, then we're not
	//	inside.

	if (bIncludeHoles
			&& PointIntersectsHole(vA))
		return false;

	return true;
	}

bool CPolygon2D::PointIntersectsHole (const CVector2D &vA) const

//	PointIntersectsHole
//
//	Returns TRUE if the given point is inside any of the polygon's holes

	{
	int i;

	for (i = 0; i < m_Holes.GetCount(); i++)
		if (m_Holes[i]->PointIntersects(vA, false))
			return true;

	return false;
	}

void CPolygon2D::SetCanonicalVertexOrder (void)

//	SetCanonicalVertexOrder
//
//	Make sure that the main contour is clockwise and any holes are counter-
//	clockwise.

	{
	int i;

	SetVertexOrder(dirClockwise);
	for (i = 0; i < m_Holes.GetCount(); i++)
		m_Holes[i]->SetVertexOrder(dirCounterClockwise);
	}

void CPolygon2D::SetVertexOrder (ERotationDirections iDirection)

//	SetVertexOrder
//
//	Sets the order of the vertices.

	{
	ASSERT(iDirection != dirNone);

	ERotationDirections iCurDir = GetVertexOrder();
	if (iCurDir == dirNone)
		return;

	if (iCurDir != iDirection)
		m_vPoints.Reverse();
	}

void CPolygon2D::Simplify (double rMinSegment, CPolygon2D *retResult) const

//	Simplify
//
//	Removes any segments shorter than rMinSegment.

	{
	int i;

	//	Short-circuit.

	*retResult = *this;
	int iCount = retResult->m_vPoints.GetCount();
	if (iCount < 3 || rMinSegment <= 0.0)
		return;

	double rMinSegment2 = rMinSegment * rMinSegment;

	//	Loop over all segments

	for (i = 0; i < retResult->GetVertexCount(); i++)
		{
		int iB = (i + 1) % retResult->GetVertexCount();
		const CVector2D &vA = retResult->m_vPoints[i];
		const CVector2D &vB = retResult->m_vPoints[iB];

		//	If this segment is too short, then delete it.

		if (vA.Distance2(vB) < rMinSegment2)
			{
			retResult->m_vPoints.Delete(iB);
			i--;
			}
		}
	}

CPolygon2D::EOpResult CPolygon2D::SubtractPolygon (const CPolygon2D &DrillRegion, TArray<CPolygon2D> *retResult) const

//	SubtractPolygon
//
//	Generates a resulting set of polygons after subtracting the drill polygons
//	from this polygon.

	{
	CPolygonIntersector Intersector;
	Intersector.SetClipRegion(DrillRegion);
	Intersector.SetSubjectRegion(*this);

	return Intersector.Subtract(retResult);
	}

CPolygon2D::EOpResult CPolygon2D::SubtractPolygon (const TArray<CPolygon2D> &DrillRegion, TArray<CPolygon2D> *retResult) const

//	SubtractPolygon
//
//	Generates a resulting set of polygons after subtracting the drill polygons
//	from this polygon.

	{
	CPolygonIntersector Intersector;
	Intersector.SetClipRegion(DrillRegion);
	Intersector.SetSubjectRegion(*this);

	return Intersector.Subtract(retResult);
	}

CPolygon2D::EOpResult CPolygon2D::SubtractPolygon (const TArray<const CPolygon2D *> &DrillRegion, TArray<CPolygon2D> *retResult) const

//	SubtractPolygon
//
//	Generates a resulting set of polygons after subtracting the drill polygons
//	from this polygon.

	{
	int i;

	CPolygonIntersector Intersector;
	Intersector.SetSubjectRegion(*this);

	for (i = 0; i < DrillRegion.GetCount(); i++)
		Intersector.InsertClipRegion(*DrillRegion[i]);

	return Intersector.Subtract(retResult);
	}

CPolygon2D::EOpResult CPolygon2D::UnionPolygon (const CPolygon2D &Poly, CPolygon2D *retResult) const

//	UnionPolygon
//
//	Generates a union of Poly and this polygon and returns the result. If the 
//	two polygons are disjoint, then we return resultEmpty and retResult is
//	undefined.

	{
	CPolygonIntersector Intersector;
	Intersector.SetSubjectRegion(*this);
	Intersector.SetClipRegion(Poly);

	return Intersector.Union(retResult);
	}
