//	CPolygonIntersector.cpp
//
//	CPolygonIntersector class
//	Copyright (c) 2014 GridWhale Corporation. All Rights Reserved.
//
//	Based On:
//	HIDDEN SURFACE REMOVAL USING POLYGON AREA SORTING
//	by Kevin Weiler and Peter Atherton
//
//	http://en.wikipedia.org/wiki/Weiler%E2%80%93Atherton
//	https://www.cs.drexel.edu/~david/Classes/CS430/Lectures/L-05_Polygons.6.pdf

#include "stdafx.h"
#include <Math.h>
#include "PolygonIntersect.h"

const double POINT_IDENTITY_EPSILON = 0.00001;
const double POINT_IDENTITY_EPSILON2 = POINT_IDENTITY_EPSILON * POINT_IDENTITY_EPSILON;
const double INTERSECT_EPSILON = 1e-10;
const double INTERSECT_LIMIT = 1.0 - INTERSECT_EPSILON;

CPolygonIntersector::CPolygonIntersector (void)

//	CPolygonIntersector constructor

	{
	}

void CPolygonIntersector::AccumulateNonIntersecting (SVertexInfo *pPoly, TArray<SVertexInfo *> *retResult)

//	AccumulateNonIntersecting
//
//	Add the first vertex of any contour in pPoly that does not have intersections
//	(this could include the main contour).

	{
	SVertexInfo *pHole = pPoly;
	while (pHole)
		{
		bool bHasIntersection = false;
		SVertexInfo *pVertex = pHole;
		do
			{
			if (pVertex->dwFlags & flagIntersection)
				{
				bHasIntersection = true;
				break;
				}

			pVertex = pVertex->pNext;
			}
		while (pVertex != pHole);

		//	If we don't have an intersection, then add this hole to the list.

		if (!bHasIntersection)
			retResult->Insert(pHole);

		//	Next hole

		pHole = pHole->pNextPoly;
		}
	}

void CPolygonIntersector::AccumulateNonIntersectingHoles (SVertexInfo *pPoly, TArray<SVertexInfo *> *retResult)

//	AccumulateNonIntersectingHoles
//
//	Add the first vertex of any hole in pPoly that does not have intersections.

	{
	SVertexInfo *pHole = pPoly->pNextPoly;
	while (pHole)
		{
		bool bHasIntersection = false;
		SVertexInfo *pVertex = pHole;
		do
			{
			if (pVertex->dwFlags & flagIntersection)
				{
				bHasIntersection = true;
				break;
				}

			pVertex = pVertex->pNext;
			}
		while (pVertex != pHole);

		//	If we don't have an intersection, then add this hole to the list.

		if (!bHasIntersection)
			retResult->Insert(pHole);

		//	Next hole

		pHole = pHole->pNextPoly;
		}
	}

void CPolygonIntersector::AddHoleToPolygons (TArray<CPolygon2D> &Result, CPolygon2D &Hole)

//	AddHoleToPolygon
//
//	Attaches the given hole (destructively) to whichever polygon in Result
//	contains it.

	{
	int i;

	if (Hole.GetVertexCount() < 3)
		return;

	for (i = 0; i < Result.GetCount(); i++)
		if (Result[i].PointIntersects(Hole.GetVertex(0), false))
			{
			Result[i].AddHoleHandoff(Hole);
			return;
			}
	}

void CPolygonIntersector::AddHoles (CPolygon2D *pDest, const TArray<SVertexInfo *> &Holes)

//	AddHoles
//
//	If there are any holes that are enclosed by pDest, then add them.

	{
	int i, j;

	for (i = 0; i < Holes.GetCount(); i++)
		{
		SVertexInfo *pHole = Holes[i];

		//	If one of the points of this hole is inside pDest, then they all are
		//	because we've previously established that all these holes are non-
		//	intersecting.

		if (pDest->PointIntersects(pHole->vPos, false))
			{
			//	Count the points in this hole

			int iCount = 0;
			SVertexInfo *pVertex = pHole;
			do
				{
				iCount++;
				pVertex = pVertex->pNext;
				}
			while (pVertex != pHole);

			//	Allocate a new polygon

			CPolygon2D NewHole(iCount);
			j = 0;
			pVertex = pHole;
			do
				{
				NewHole.SetVertex(j++, pVertex->vPos);
				pVertex = pVertex->pNext;
				}
			while (pVertex != pHole);

			//	Add it to the destination

			pDest->AddHoleHandoff(NewHole);
			}
		}
	}

void CPolygonIntersector::AddHolesAsPolygons (const CPolygon2D &OriginalPoly, const TArray<SVertexInfo *> &Holes, TArray<CPolygon2D> *retResult)

//	AddHolesAsPolygons
//
//	For any hole in the list that is inside OriginalPoly (we assume that there
//	are not segment intersections), we add it as a new polygon to retResult.

	{
	int i;

	for (i = 0; i < Holes.GetCount(); i++)
		{
		SVertexInfo *pHole = Holes[i];

		//	If one of the points of this hole is inside pDest, then they all are
		//	because we've previously established that all these holes are non-
		//	intersecting.

		if ((pHole->dwFlags & flagHole) 
				&& OriginalPoly.PointIntersects(pHole->vPos, false))
			{
			CPolygon2D *pNewPoly = retResult->Insert();
			CreatePolygonFromVertexList(pHole, pNewPoly);
			}
		}
	}

void CPolygonIntersector::AddPolygonAndHoles (const CPolygon2D &Poly, const CPolygon2D &HoleSource, TArray<CPolygon2D> *retResult)

//	AddPolygonAndHoles
//
//	Adds the given polygon and attaches any enclosed holes from HoleSource.

	{
	int i;

	CPolygon2D *pNewPoly = retResult->Insert();
	*pNewPoly = Poly;

	for (i = 0; i < HoleSource.GetHoleCount(); i++)
		{
		const CPolygon2D &Hole = HoleSource.GetHole(i);
		if (Hole.GetVertexCount() >= 3 && Poly.PointIntersects(Hole.GetVertex(0), false))
			pNewPoly->AddHole(Hole);
		}
	}

CPolygonIntersector::SVertexInfo *CPolygonIntersector::AllocVertex (const CVector2D &vPos, DWORD dwFlags, SVertexInfo *pNext, SVertexInfo *pPrev)

//	AllocVertex
//
//	Allocate a vertex

	{
	ASSERT((dwFlags & 0x40) == 0);

	//	If we've exceeded our allocation, then we need to do something

	if (m_Vertices.GetCount() >= m_iVertexAlloc)
		{
		int i;
		int iClipVertexCount = 0;
		for (i = 0; i < m_Clip.GetCount(); i++)
			iClipVertexCount += GetTotalVertexCount(*m_Clip[i]);

		throw CException(errFail, 
				strPattern("Did not allocate enough vertices. Alloc = %d; Subject Vertices = %d; Clip Polys = %d; Clip Vertices = %d.",
						m_iVertexAlloc,
						(m_pSubject ? GetTotalVertexCount(*m_pSubject) : 0),
						m_Clip.GetCount(),
						iClipVertexCount));
		}

	//	Otherwise, continue

	else
		{
		SVertexInfo *pNewVertex = m_Vertices.Insert();
		pNewVertex->vPos = vPos;
		pNewVertex->dwFlags = dwFlags;
		pNewVertex->pNext = pNext;
		pNewVertex->pPrev = pPrev;
		pNewVertex->pNextPoly = NULL;
		pNewVertex->pIntersect = NULL;

		return pNewVertex;
		}
	}

void CPolygonIntersector::CalcInsideOutsideIntersections (SVertexInfo *pPoly)

//	CalcInsideOutsideIntersections
//
//	Mark intersections as either inside or outside.

	{
	int i;

	TArray<SVertexInfo *> Intersections;
	Intersections.GrowToFit(m_iIntersectCount);

	SVertexInfo *pFrom = pPoly;
	do
		{
		SVertexInfo *pTo = pFrom->pNext;

		//	If this is an intersection, handle it

		if (pFrom->dwFlags & flagIntersection)
			{
			ASSERT(pFrom->pIntersect);

			//	Switch to the intersection in the intersecting polygon

			SVertexInfo *pOther = pFrom->pIntersect;

			//	Get the next point on the intersecting polygon's path

			CVector2D vPos = pOther->pNext->vPos;

			//	See if vPos is left or right. If left of line, then we are outside.

			DWORD dwFlag;
			CLine2D::EOpResult iResult = CLine2D::IsLeftOfLine(pFrom->vPos, pTo->vPos, vPos);

			//	If we are collinear, then check the previous point

			if (iResult == CLine2D::resultCollinear)
				{
				CVector2D vPrevPos = pOther->pPrev->vPos;
				iResult = CLine2D::IsLeftOfLine(pFrom->pPrev->vPos, pFrom->vPos, vPrevPos);

				//	If we're still collinear then just mark it because we'll probably 
				//	need to remove the vertex.

				if (iResult == CLine2D::resultCollinear)
					dwFlag = flagInside | flagCollinear;

				//	Since this is the previous point, we reverse the sense.

				else
					dwFlag = (iResult == CLine2D::resultOK ? flagInside : flagOutside);
				}

			//	Otherwise set the flag. To the left means outside.

			else
				dwFlag = (iResult == CLine2D::resultOK ? flagOutside : flagInside);

			//	Mark both intersections

			pFrom->dwFlags |= dwFlag;
			pOther->dwFlags |= dwFlag;

			//	Remember the intersection

			Intersections.Insert(pFrom);
			}

		//	Next

		pFrom = pTo;
		}
	while (pFrom != pPoly);

	//	Make sure that all our intersections alternate (in/out)

	bool bFixup = true;

	while (bFixup)
		{
		bFixup = false;

		for (i = 0; i < Intersections.GetCount(); i++)
			{
			SVertexInfo *pPrev = Intersections[(i - 1 + Intersections.GetCount()) % Intersections.GetCount()];
			SVertexInfo *pFrom = Intersections[i];
			SVertexInfo *pTo = Intersections[(i + 1) % Intersections.GetCount()];

			//	If this is the last intersection, or if this intersection
			//	is collinear, then remove it.

			if (m_iIntersectCount == 1 || pFrom->dwFlags & flagCollinear)
				{
				ASSERT(pFrom->pIntersect->pIntersect == pFrom);
				RemoveIntersection(pFrom);
				m_iIntersectCount--;
				Intersections.Delete(i);
				i--;
				bFixup = true;
				}

			//	If we conflict with the previous intersection and we are a vertex
			//	then delete ourselves

			else if ((pFrom->dwFlags & flagOriginal)
					&& !(pTo->dwFlags & flagCollinear)
					&& (pFrom->dwFlags & flagOutside) == (pPrev->dwFlags & flagOutside))
				{
				ASSERT(pFrom->pIntersect->pIntersect == pFrom);
				RemoveIntersection(pFrom);
				m_iIntersectCount--;
				Intersections.Delete(i);
				i--;
				bFixup = true;
				}
					
			//	If two intersections have the same polarity, then remove one of them.
			//	(But if the next intersection is collinear, then skip it since we'll get
			//	to it).

			else if (!(pTo->dwFlags & flagCollinear)
					&& (pFrom->dwFlags & flagOriginal)
					&& (pFrom->dwFlags & flagOutside) == (pTo->dwFlags & flagOutside))
				{
				ASSERT(pFrom->pIntersect->pIntersect == pFrom);
				RemoveIntersection(pFrom);
				m_iIntersectCount--;
				Intersections.Delete(i);
				i--;
				bFixup = true;
				}
			}
		}

	//	Recurse to holes

	if (pPoly->pNextPoly)
		CalcInsideOutsideIntersections(pPoly->pNextPoly);
	}

void CPolygonIntersector::CollapseIntersections (SVertexInfo *pPoly)

//	CollapseIntersections
//
//	Looks for intersections that are on top of a vertex and removes them.

	{
	//	Do a second pass and collapse intersections that fall on a vertex.

	SVertexInfo *pFrom = pPoly;
	while (true)
		{
		SVertexInfo *pTo = pFrom->pNext;

		//	If we are an original vertex and the next vertex is a non-original 
		//	intersection that is right on top of us, then we collapse it.

		if ((pFrom->dwFlags & flagOriginal)
				&& (pTo->dwFlags & flagIntersection)
				&& !(pTo->dwFlags & flagOriginal)
				&& pTo->rX <= INTERSECT_EPSILON)
			{
			ASSERT(pTo->pIntersect->pIntersect == pTo);

			//	This vertex becomes the intersection

			if (!(pFrom->dwFlags & flagIntersection))
				pFrom->dwFlags |= flagIntersection;
			else
				{
				SVertexInfo *pDelete = pFrom->pIntersect;
				pDelete->pPrev->pNext = pDelete->pNext;
				pDelete->pNext->pPrev = pDelete->pPrev;

				m_iIntersectCount--;
				}

			//	Make sure our partner intersection points to us.

			pFrom->pIntersect = pTo->pIntersect;
			pTo->pIntersect->pIntersect = pFrom;

			//	Remove the next vertex

			pFrom->pNext = pTo->pNext;
			pTo->pNext->pPrev = pFrom;

			pTo->dwFlags |= flagDeleted;
			pTo->pNext = NULL;
			pTo->pPrev = NULL;

			//	Stay on this vertex and continue

			continue;
			}

		//	Otherwise, if the next vertex is an original vertex and we're
		//	right on top of it, then we delete ourselves

		else if ((pTo->dwFlags & flagOriginal)
				&& (pFrom->dwFlags & flagIntersection)
				&& !(pFrom->dwFlags & flagOriginal)
				&& pFrom->rX >= INTERSECT_LIMIT)
			{
			ASSERT(pFrom->pIntersect->pIntersect == pFrom);

			//	The next vertex becomes the intersection

			if (!(pTo->dwFlags & flagIntersection))
				pTo->dwFlags |= flagIntersection;
			else
				{
				SVertexInfo *pDelete = pTo->pIntersect;
				pDelete->pPrev->pNext = pDelete->pNext;
				pDelete->pNext->pPrev = pDelete->pPrev;

				m_iIntersectCount--;
				}

			pTo->pIntersect = pFrom->pIntersect;
			pFrom->pIntersect->pIntersect = pTo;

			//	Remove ourselves

			pFrom->pPrev->pNext = pTo;
			pTo->pPrev = pFrom->pPrev;

			pFrom->dwFlags |= flagDeleted;
			pFrom->pNext = NULL;
			pFrom->pPrev = NULL;

			//	Back up one and continue

			pFrom = pTo->pPrev;
			continue;
			}

		//	Next

		if (pTo != pPoly)
			pFrom = pTo;
		else
			break;
		}

	//	Now collapse any holes vertices

	if (pPoly->pNextPoly)
		CollapseIntersections(pPoly->pNextPoly);
	}

int CPolygonIntersector::CompareVertexIntersections (void *pCtx, const SVertexInfo *&Key1, const SVertexInfo *&Key2)
	{
	if (Key1->rX > Key2->rX)
		return 1;
	else if (Key1->rX < Key2->rX)
		return -1;
	else
		{
		//	Reverse order precedence (because we add intersections backwards).

		if (Key1->iOrder > Key2->iOrder)
			return -1;
		else if (Key1->iOrder < Key2->iOrder)
			return 1;
		else
			return 0;
		}
	}

void CPolygonIntersector::CreatePolygonFromVertexList (SVertexInfo *pPoly, CPolygon2D *retResult)

//	CreatePolygonFromVertexList
//
//	Creates a new polygon from the given vertex list.

	{
	int j;

	//	Count the points in this hole

	int iCount = 0;
	SVertexInfo *pVertex = pPoly;
	do
		{
		iCount++;
		pVertex = pVertex->pNext;
		}
	while (pVertex != pPoly);

	//	Allocate a new polygon

	CPolygon2D NewPoly(iCount);
	j = 0;
	pVertex = pPoly;
	do
		{
		NewPoly.SetVertex(j++, pVertex->vPos);
		pVertex = pVertex->pNext;
		}
	while (pVertex != pPoly);

	//	Add it to the destination

	retResult->TakeHandoff(NewPoly);
	}

CPolygonIntersector::SVertexInfo *CPolygonIntersector::CreateVertexList (const CPolygon2D &Src, DWORD dwFlags)

//	CreateVertexList
//
//	Creates a list of vertices for the given polygon.
//
//	1.	pNext connects to the next vertex in order. For normal polygons, this is
//		always clockwise. For holes, this is always counter-clockwise. The 
//		connection is circular (the last vertex points back to the first).
//
//	2.	pPrev is a backwards link (in the opposite order).
//
//	3.	The first vertex has a pNextPoly link which points to the first vertex
//		of the first hole (if any). NULL if none. In turn, the first vertex of
//		a hole has a pNextPoly that points to the next hole. NULL if none.
//
//	4.	The first vertex always has the flagStart flag (even for holes).
//
//	5.	All vertices of a hole have flagHole.

	{
	int i;

	int iVertex;
	int iEnd;
	int iDir;
	int iCount = Src.GetVertexCount();

	ERotationDirections iDesiredOrder = ((dwFlags & flagHole) ? dirCounterClockwise : dirClockwise);

	if (Src.GetVertexOrder() == iDesiredOrder)
		{
		iVertex = 0;
		iEnd = iCount - 1;
		iDir = 1;
		}
	else
		{
		iVertex = iCount - 1;
		iEnd = 0;
		iDir = -1;
		}

	SVertexInfo *pFirst = NULL;
	SVertexInfo *pPrev = NULL;
	while (true)
		{
		SVertexInfo *pNewVertex = AllocVertex(Src.GetVertex(iVertex), dwFlags | flagOriginal, NULL, pPrev);
		if (pNewVertex == NULL)
			throw CException(errFail);

		if (pFirst == NULL)
			{
			pFirst = pNewVertex;
			pNewVertex->dwFlags |= flagStart;
			}

		if (pPrev)
			pPrev->pNext = pNewVertex;

		//	Next

		pPrev = pNewVertex;
		if (iVertex != iEnd)
			iVertex += iDir;
		else
			break;
		}

	if (pPrev)
		{
		pPrev->pNext = pFirst;
		if (pFirst)
			pFirst->pPrev = pPrev;
		}

	//	Recurse and add any holes

	for (i = 0; i < Src.GetHoleCount(); i++)
		{
		ASSERT((dwFlags & flagHole) == 0);

		SVertexInfo *pHole = CreateVertexList(Src.GetHole(i), dwFlags | flagHole);

		//	Attach to the main polygon

		pHole->pNextPoly = pFirst->pNextPoly;
		pFirst->pNextPoly = pHole;
		}

	//	Done

	return pFirst;
	}

CPolygonIntersector::EIntersectionTypes CPolygonIntersector::GetContainmentRelationship (const CPolygon2D &Subject, const CPolygon2D &Clip)

//	GetContainmentRelationship
//
//	If the two polygons have no intersecting segments, this function returns 
//	whether one is contained within the other.

	{
	int i;

	ASSERT(Subject.GetVertexCount() > 0);
	ASSERT(Clip.GetVertexCount() > 0);

	//	If one of the points in the subject is inside the clip, then assume that
	//	they all are (since there are no intersections).

	if (Clip.PointIntersects(Subject.GetVertex(0), false))
		return intersectSubjectInsideClip;

	//	If one of the points in the clip is inside the subject, then assume that
	//	they all are.

	else if (Subject.PointIntersects(Clip.GetVertex(0), false))
		return intersectClipInsideSubject;

	//	Otherwise, we need to do a couple more tests.

	else
		{
		//	See if any of the points in Subject is inside of clip.

		for (i = 1; i < Subject.GetVertexCount(); i++)
			if (Clip.PointIntersects(Subject.GetVertex(i), false))
				{
				if (Clip.HasVertex(Subject.GetVertex(i), POINT_IDENTITY_EPSILON2))
					continue;

				return intersectSubjectInsideClip;
				}

		//	Otherwise, see if any of the points in Clip are inside the
		//	subject.

		for (i = 1; i < Clip.GetVertexCount(); i++)
			if (Subject.PointIntersects(Clip.GetVertex(i), false))
				{
				if (Subject.HasVertex(Clip.GetVertex(i), POINT_IDENTITY_EPSILON2))
					continue;

				return intersectClipInsideSubject;
				}

		//	Otherwise we're either disjoint or equal. We count the number of 
		//	points that we have in common. If we have at least 3 points in common
		//	then we're equal.

		if (m_iVertexMatches >= 3)
			return intersectEquals;
		else
			return intersectDisjoint;
		}
	}

void CPolygonIntersector::GetIntersections (SVertexInfo *pPoly, EIntersectionTypes iType, TArray<SVertexInfo *> *retOutside)

//	GetIntersections
//
//	We traverse the vertices of pPoly. At each intersection we decide whether
//	the path on the other polygon leads inside (right) or outside (left) of 
//	pPoly. We return the appropriate intersections.

	{
	retOutside->DeleteAll();
	retOutside->GrowToFit(m_iIntersectCount);

	while (pPoly)
		{
		SVertexInfo *pFrom = pPoly;
		do
			{
			SVertexInfo *pTo = pFrom->pNext;

			//	If this is an intersection, handle it

			if (pFrom->dwFlags & flagIntersection)
				{
				ASSERT(pFrom->pIntersect);

				EIntersectionTypes iIntersectType = ((pFrom->dwFlags & flagOutside) ? intersectOutside : intersectInside);
				if (iIntersectType == iType)
					retOutside->Insert(pFrom);
				}

			//	Next

			pFrom = pTo;
			}
		while (pFrom != pPoly);

		//	Next contour

		pPoly = pPoly->pNextPoly;
		}
	}

int CPolygonIntersector::GetTotalVertexCount (const CPolygon2D &Poly)

//	GetTotalVertexCount
//
//	Returns the total number of vertices in the polygon, including all the
//	holes.

	{
	int i;
	int iTotal = Poly.GetVertexCount();

	for (i = 0; i < Poly.GetHoleCount(); i++)
		iTotal += Poly.GetHole(i).GetVertexCount();

	return iTotal;
	}

void CPolygonIntersector::InsertIntersections (SVertexInfo *pClip, SVertexInfo *pSubject)

//	InsertIntersections
//
//	Loop over all line segments in the clip polygon and add intersection
//	points with the subject polygon.
//
//	NOTE: We also initialize m_iIntersectCount;

	{
	m_iIntersectCount = 0;
	m_iVertexMatches = 0;

	//	Loop over all line segments in the clip polygon (including the holes)

	SVertexInfo *pFrom = pClip;
	do
		{
		SVertexInfo *pTo = pFrom->pNext;
		ASSERT(pTo);

		//	Loop over all subject line segments (including the holes)

		SVertexInfo *pSubjectFrom = pSubject;
		do
			{
			SVertexInfo *pSubjectTo = pSubjectFrom->pNext;
			ASSERT(pSubjectTo);

			//	If we intersect, then we need to add a point.

			CVector2D vIntersect;
			double rClipPos;
			double rSubjectPos;
			CLine2D::EOpResult iResult = CLine2D::CalcIntersection(pFrom->vPos, pTo->vPos, pSubjectFrom->vPos, pSubjectTo->vPos, &vIntersect, &rClipPos, &rSubjectPos);

			//	Skip intersections at the end of a line (since we'll get the same
			//	intersection at the beginning of the next segment).
			//
			//	NOTE: We don't do epsilon because otherwise we'd have count epsilon
			//	less than 0 when checking the next segment.

			if ((iResult == CLine2D::resultOK)
					&& (rClipPos < 1.0 && rSubjectPos < 1.0))
				{
				//	We create two intersection points (one for each polygon) but
				//	they only count as one.

				m_iIntersectCount++;

				//	Create a new intersection point and link it (temporarily) to
				//	the pIntersection field of the from vertex of this line 
				//	segment.

				ASSERT(pFrom->dwFlags & flagClip);
				DWORD dwClipFlags = (pFrom->dwFlags & flagHole) | flagClip | flagIntersection;
				SVertexInfo *pNewClipIntersect = AllocVertex(vIntersect, dwClipFlags, pFrom->pIntersect, pFrom);

				pFrom->pIntersect = pNewClipIntersect;
				pNewClipIntersect->rX = rClipPos;

				//	Now do the same for the subject polygon

				ASSERT(pSubjectFrom->dwFlags & flagSubject);
				DWORD dwSubjectFlags = (pSubjectFrom->dwFlags & flagHole) | flagSubject | flagIntersection;
				SVertexInfo *pNewSubjectIntersect = AllocVertex(vIntersect, dwSubjectFlags, pSubjectFrom->pIntersect, pSubjectFrom);

				pSubjectFrom->pIntersect = pNewSubjectIntersect;
				pNewSubjectIntersect->rX = rSubjectPos;

				//	Connect the two together (so we can transition from one to
				//	the other).

				pNewClipIntersect->pIntersect = pNewSubjectIntersect;
				pNewSubjectIntersect->pIntersect = pNewClipIntersect;
				}

			//	If the result is collinear then we check to see if we have overlapping
			//	vertices (we need to know so we can later check if we have identical
			//	polygons).

			else if (iResult == CLine2D::resultCollinear)
				{
				//	We only need to check one point against another (since we're checking all
				//	permutations).

				if (pFrom->vPos.IsEqualTo(pSubjectFrom->vPos, POINT_IDENTITY_EPSILON2))
					m_iVertexMatches++;
				}

			//	Next

			pSubjectFrom = pSubjectTo;
			if (pSubjectFrom->dwFlags & flagStart)
				pSubjectFrom = pSubjectFrom->pNextPoly;
			}
		while (pSubjectFrom);

		//	Next

		pFrom = pTo;
		if (pFrom->dwFlags & flagStart)
			pFrom = pFrom->pNextPoly;
		}
	while (pFrom);

	//	Now we need to add the newly created intersection points to their
	//	respective loops making sure to add the points in order.

	ResolveIntersections(pClip);
	ResolveIntersections(pSubject);
	CollapseIntersections(pClip);
	CollapseIntersections(pSubject);
	CalcInsideOutsideIntersections(pSubject);

#ifdef DEBUG_POLYGON_INTERSECT
	if ((m_iIntersectCount % 2) != 0)
		{
		m_pCurClip->OutputPolygon(CString("Clip"));
		m_pSubject->OutputPolygon(CString("Subject"));
		}
#endif
	}

CPolygon2D::EOpResult CPolygonIntersector::Intersect (TArray<CPolygon2D> *retResult)

//	Intersect
//
//	Intersects this (subject) polygon with the list of clipping polygons and 
//	optionally returns a list of resulting polygons. If the list is empty, then
//	there is no intersection.

	{
	int i;

	//	Start empty

	retResult->DeleteAll();

	//	Accumulate polygons for each clip polygon

	for (i = 0; i < m_Clip.GetCount(); i++)
		{
		m_pCurClip = m_Clip[i];
		Intersect(*m_pCurClip, retResult);
		}

	//	If the result is empty, then nothing

	return (retResult->GetCount() > 0 ? CPolygon2D::resultOK : CPolygon2D::resultEmpty);
	}

CPolygon2D::EOpResult CPolygonIntersector::Intersect (const CPolygon2D &Clip, TArray<CPolygon2D> *retResult)

//	Intersect
//
//	Intersects this (subject) polygon with the given clipping polygon and 
//	optionally returns a list of resulting polygons. If the list is empty, then
//	there is no intersection.
//
//	Based On:
//	HIDDEN SURFACE REMOVAL USING POLYGON AREA SORTING
//	by Kevin Weiler and Peter Atherton

	{
	int i;

	//	Deal with some degenerate conditions

	if (m_pSubject == NULL || m_pSubject->GetVertexCount() < 3 || Clip.GetVertexCount() < 3)
		return CPolygon2D::resultEmpty;

	//	If the subject is completely outside the bounds of the clip region,
	//	then we're done. [This is just an optimization; there is no guarantee
	//	that all non-overlapping polygons will exit here.]

	CRect2D ClipBounds;
	Clip.CalcBoundingRect(&ClipBounds);

	CRect2D SubjectBounds;
	m_pSubject->CalcBoundingRect(&SubjectBounds);

	if (!SubjectBounds.RectIntersects(ClipBounds))
		return CPolygon2D::resultEmpty;

	//	Initialize the vertex array. This will allocate enough vertex info cells
	//	to do the intersection.

	InitVerticesArray();

	//	Create a chain of vertices for the subject and the clip polygon in
	//	clockwise order.

	SVertexInfo *pSubject = CreateVertexList(*m_pSubject, flagSubject);
	SVertexInfo *pClip = CreateVertexList(Clip, flagClip);

	//	Loop over all line segments in the clip polygon and add intersection
	//	points with the subject polygon.

	InsertIntersections(pClip, pSubject);

	//	If there are no intersection, then see if we are disjoint or if one 
	//	polygon is completely inside the other.

	if (m_iIntersectCount == 0)
		{
		switch (GetContainmentRelationship(*m_pSubject, Clip))
			{
			case intersectSubjectInsideClip:
			case intersectEquals:
				{
				//	If the clip polygon has holes then we might need to add them
				//	to the resulting polygon.

				if (Clip.GetHoleCount() > 0)
					{
					AddPolygonAndHoles(*m_pSubject, Clip, retResult);
					return CPolygon2D::resultOK;
					}

				//	Otherwise, we return the subject intact.

				else
					{
					retResult->Insert(*m_pSubject);
					return CPolygon2D::resultSubjectPoly;
					}
				}

			case intersectClipInsideSubject:
				{
				//	Check to see if we need to add holes from the subject to the
				//	clip result.

				if (m_pSubject->GetHoleCount() > 0)
					{
					AddPolygonAndHoles(Clip, *m_pSubject, retResult);
					return CPolygon2D::resultOK;
					}
				else
					{
					retResult->Insert(Clip);
					return CPolygon2D::resultClipPoly;
					}
				}

			default:
				return CPolygon2D::resultEmpty;
			}
		}

	//	Make a list of non-intersecting holes from both the subject and the clip
	//	polygons.

	TArray<SVertexInfo *> Holes;
	AccumulateNonIntersectingHoles(pClip, &Holes);
	AccumulateNonIntersectingHoles(pSubject, &Holes);

	//	Get a list of outside intersections. This will be the set of vertices
	//	that we'll use to start each of the resulting polygons.

	TArray<SVertexInfo *> Outside;
	GetIntersections(pSubject, intersectOutside, &Outside);

	//	Loop over all outside vertices and create a polygon.

	for (i = 0; i < Outside.GetCount(); i++)
		{
		SVertexInfo *pStart = Outside[i];

		//	If we already processed this vertex (i.e., it is already part of
		//	a resulting polygon, then we skip).

		if (pStart->dwFlags & flagProcessed)
			continue;

		//	Prepare an array. We make sure we have enough room for at least as
		//	many points in the original, plus all intersections.

		TArray<CVector2D> NewPoints;
		NewPoints.GrowToFit(m_pSubject->GetVertexCount() + m_iIntersectCount);

		//	Loop until we get back to the starting point

		TraceOutline(pStart, opIntersect, &NewPoints);

		//	Add a polygon

		if (NewPoints.GetCount() >= 3)
			{
			CPolygon2D *pNewPoly = retResult->Insert();
			pNewPoly->TakeHandoff(NewPoints);

			//	Add any holes that we need

			AddHoles(pNewPoly, Holes);
			}
		}

	//	Done

	return CPolygon2D::resultOK;
	}

void CPolygonIntersector::InitVerticesArray (void)

//	InitVerticesArray
//
//	Initializes the vertices array.

	{
	int i;

	//	Figure out how many vertices we need for all the clipping polygons

	int iTotalClipVertices = 0;
	for (i = 0; i < m_Clip.GetCount(); i++)
		iTotalClipVertices += GetTotalVertexCount(*m_Clip[i]);

	//	Figure out how many vertices we need for all the subject polygons

	int iTotalSubjectVertices = (m_pSubject ? GetTotalVertexCount(*m_pSubject) : 0);

	//	Initialize our vertex list for the worst-case scenario. We need the
	//	array to be large enough to hold all vertices for the subject, the clip,
	//	and any intersections.

	m_iVertexAlloc = 10 * (iTotalClipVertices + iTotalSubjectVertices);

	m_Vertices.DeleteAll();
	m_Vertices.GrowToFit(m_iVertexAlloc);
	}

#ifdef DEBUG
void CPolygonIntersector::OutputVertexList (SVertexInfo *pPoly)
	{
	SVertexInfo *pVert = pPoly;
	do
		{
		CString sType = strPattern("%s%s%s%s%s",
				((pVert->dwFlags & flagClip) ? CString("clip ") : CString("subject ")),
				((pVert->dwFlags & flagHole) ? CString("hole ") : NULL_STR),
				((pVert->dwFlags & flagIntersection) ? ((pVert->dwFlags & flagOutside) ? CString("outside ") : CString("inside ")) : NULL_STR),
				((pVert->dwFlags & flagOriginal) ? CString("vertex ") : NULL_STR),
				((pVert->dwFlags & flagIntersection) ? CString("intersect ") : NULL_STR));

		printf("%p: [%.12f,%.12f] %s (%x) [%p <-]\n", pVert, pVert->vPos.X(), pVert->vPos.Y(), (LPSTR)sType, pVert->dwFlags, pVert->pPrev);

		pVert = pVert->pNext;
		}
	while (pVert != pPoly);

	//	Loop over holes

	SVertexInfo *pHolePoly = pPoly->pNextPoly;
	while (pHolePoly)
		{
		OutputVertexList(pHolePoly);
		pHolePoly = pHolePoly->pNextPoly;
		}
	}
#endif

CPolygonIntersector::SVertexInfo *CPolygonIntersector::RemoveIntersection (SVertexInfo *pIntersect)

//	RemoveIntersection
//
//	Removes the given intersection vertex from both contours. Returns the vertex
//	that used to point to the removed vertex.

	{
	SVertexInfo *pPrev = pIntersect->pPrev;

	//	If this is an original vertex then just remove the intersection flag.

	if (pIntersect->dwFlags & flagOriginal)
		pIntersect->dwFlags &= ~flagIntersection;

	//	Otherwise we remove the vertex

	else
		{
		pPrev->pNext = pIntersect->pNext;
		pIntersect->pNext->pPrev = pPrev;

		pIntersect->dwFlags |= flagDeleted;
		pIntersect->pNext = NULL;
		pIntersect->pPrev = NULL;
		}

	//	Remove the other side. [We clear out pIntersect so that we don't recurse
	//	back to this side.]

	if (pIntersect->pIntersect)
		{
		SVertexInfo *pOther = pIntersect->pIntersect;
		pIntersect->pIntersect = NULL;

		if (pOther->pIntersect)
			{
			ASSERT(pOther->pIntersect == pIntersect);
			RemoveIntersection(pOther);
			}
		}

	//	Done

	return pPrev;
	}

void CPolygonIntersector::ResolveIntersections (SVertexInfo *pPoly)

//	ResolveIntersections
//
//	We assume that we have a set of intersection vertices attached to line
//	segments in pPoly. We sort them appropriately and insert them into pPoly
//	in order.
//
//	The result is a valid polygon with intersection points.

	{
	int i;

	ASSERT((pPoly->dwFlags & flagIntResolved) == 0);
	pPoly->dwFlags |= flagIntResolved;

	//	Loop over all line segments in the polygon

	SVertexInfo *pFrom = pPoly;
	do
		{
		SVertexInfo *pTo = pFrom->pNext;
		SVertexInfo *pFirstIntersect = pFrom->pIntersect;

		//	If we no intersection points then nothing to do

		if (pFirstIntersect == NULL)
			NULL;

		//	If we have a single intersection point then we can just insert it
		//	without sorting.

		else if (pFirstIntersect->pNext == NULL)
			{
			pFirstIntersect->pPrev = pFrom;
			pFirstIntersect->pNext = pTo;

			pTo->pPrev = pFirstIntersect;
			pFrom->pNext = pFirstIntersect;

			ASSERT((pFrom->dwFlags & flagIntersection) == 0);
			pFrom->pIntersect = NULL;
			}

		//	Otherwise we have to sort them

		else
			{
			//	Compute the distance from each intersection point to the from
			//	vertex.

			int iCount = 0;
			SVertexInfo *pVert = pFirstIntersect;
			TArray<SVertexInfo *> Sorted;
			while (pVert)
				{
				pVert->iOrder = iCount++;
				Sorted.Insert(pVert);

				pVert = pVert->pNext;
				}

			//	Sort them in an array

			Sorted.Sort((void *)NULL, (TArray<SVertexInfo *>::COMPAREPROC)CompareVertexIntersections, AscendingSort);

			//	Now link them together

			SVertexInfo *pPrev = pFrom;
			for (i = 0; i < iCount; i++)
				{
				SVertexInfo *pNewIntersect = Sorted[i];

				pNewIntersect->pPrev = pPrev;
				pPrev = pNewIntersect;

				if (i < iCount - 1)
					pNewIntersect->pNext = Sorted[i + 1];
				else
					pNewIntersect->pNext = pTo;
				}

			//	Link first and last

			pFrom->pNext = Sorted[0];
			pTo->pPrev = pPrev;

			ASSERT((pFrom->dwFlags & flagIntersection) == 0);
			pFrom->pIntersect = NULL;
			}

		//	Next

		pFrom = pTo;
		}
	while (pFrom != pPoly);

	//	Now resolve any holes

	if (pPoly->pNextPoly)
		ResolveIntersections(pPoly->pNextPoly);
	}

void CPolygonIntersector::SetClipRegion (const CPolygon2D &Clip)

//	GetClipRegion
//
//	Sets the clip region to a single polygon

	{
	m_Clip.DeleteAll();
	m_Clip.GrowToFit(1);

	if (Clip.GetVertexCount() >= 3)
		m_Clip.Insert(&Clip);
	}

void CPolygonIntersector::SetClipRegion (const TArray<CPolygon2D> &Clip)

//	SetClipRegion
//
//	Sets the clip region to an array of polygons

	{
	int i;

	m_Clip.DeleteAll();
	m_Clip.GrowToFit(Clip.GetCount());

	for (i = 0; i < Clip.GetCount(); i++)
		{
		if (Clip[i].GetVertexCount() >= 3)
			m_Clip.Insert(&Clip[i]);
		}
	}

void CPolygonIntersector::SetSubjectRegion (const CPolygon2D &Subject)

//	SetSubjectRegion
//
//	Sets the subject region

	{
	m_pSubject = &Subject;
	}

CPolygon2D::EOpResult CPolygonIntersector::Subtract (TArray<CPolygon2D> *retResult)

//	Subtract
//
//	Subtracts the clip polygons from the subject

	{
	int i, j, k;

	//	Start with the subject polygon

	const CPolygon2D *pOldSubject = m_pSubject;
	TArray<const CPolygon2D *> SubjectList;
	SubjectList.Insert(m_pSubject);

	//	This list accumulates new polygons created by the subtraction

	TArray<CPolygon2D *> NewPolygons;

	//	Accumulate polygons for each clip polygon

	for (i = 0; i < m_Clip.GetCount(); i++)
		{
		TArray<const CPolygon2D *> NewSubjectList;

		//	For each polygon in the subject list, subtract this clipping polygon

		m_pCurClip = m_Clip[i];
		for (j = 0; j < SubjectList.GetCount(); j++)
			{
			TArray<CPolygon2D> Result;

			m_pSubject = SubjectList[j];
			CPolygon2D::EOpResult iResult = Subtract(*m_pCurClip, &Result);

			//	If the result is empty then we continue (without adding this subject
			//	poly back to the list.

			if (iResult == CPolygon2D::resultEmpty)
				NULL;

			//	If the result is the intact subject, then we just add it back to the
			//	list.

			else if (iResult == CPolygon2D::resultSubjectPoly)
				NewSubjectList.Insert(m_pSubject);

			//	Otherwise we take all the polygons from the result and add them

			else
				{
				for (k = 0; k < Result.GetCount(); k++)
					{
					CPolygon2D *pAlloc = new CPolygon2D;
					NewPolygons.Insert(pAlloc);

					pAlloc->TakeHandoff(Result[k]);
					NewSubjectList.Insert(pAlloc);
					}
				}
			}

		//	If the new subject list is empty, then we're done!

		if (NewSubjectList.GetCount() == 0)
			{
#ifdef DEBUG_SUBTRACT_BUG
			m_pCurClip->OutputPolygon(CString("Clip"));
			m_pSubject->OutputPolygon(CString("Subject"));
#endif
			NewPolygons.DeleteAllAndFreeValues();
			return CPolygon2D::resultEmpty;
			}

		//	Otherwise, we overwrite the subject list and keep looping to the next
		//	clipping polygon.

		SubjectList = NewSubjectList;
		}

	//	The result is the subject list

	retResult->DeleteAll();
	retResult->GrowToFit(SubjectList.GetCount());
	for (i = 0; i < SubjectList.GetCount(); i++)
		retResult->Insert(*SubjectList[i]);

	NewPolygons.DeleteAllAndFreeValues();
	return CPolygon2D::resultOK;
	}

CPolygon2D::EOpResult CPolygonIntersector::Subtract (const CPolygon2D &Clip, TArray<CPolygon2D> *retResult)

//	Subtract
//
//	Subtracts the given drill polygon from the subect.

	{
	int i;

	//	Deal with some degenerate conditions

	if (Clip.GetVertexCount() < 3)
		return CPolygon2D::resultSubjectPoly;

	if (m_pSubject == NULL || m_pSubject->GetVertexCount() < 3)
		return CPolygon2D::resultEmpty;

	//	If the subject is completely outside the bounds of the clip region,
	//	then we're done. [This is just an optimization; there is no guarantee
	//	that all non-overlapping polygons will exit here.]

	CRect2D ClipBounds;
	Clip.CalcBoundingRect(&ClipBounds);

	CRect2D SubjectBounds;
	m_pSubject->CalcBoundingRect(&SubjectBounds);

	if (!SubjectBounds.RectIntersects(ClipBounds))
		return CPolygon2D::resultSubjectPoly;

	//	Initialize the vertex array. This will allocate enough vertex info cells
	//	to do the intersection.

	InitVerticesArray();

	//	Create a chain of vertices for the subject and the clip polygon in
	//	clockwise order.

	SVertexInfo *pSubject = CreateVertexList(*m_pSubject, flagSubject);
	SVertexInfo *pClip = CreateVertexList(Clip, flagClip);

	//	Loop over all line segments in the clip polygon and add intersection
	//	points with the subject polygon.

	InsertIntersections(pClip, pSubject);

	//	If there are no intersection, then see if we are disjoint or if one 
	//	polygon is completely inside the other.

	if (m_iIntersectCount == 0)
		{
		switch (GetContainmentRelationship(*m_pSubject, Clip))
			{
			case intersectEquals:
			case intersectSubjectInsideClip:
				{
				//	If the clip polygon has holes then check to see if the subject
				//	polygon is inside any of the holes.

				if (Clip.GetHoleCount() > 0 && Clip.PointIntersectsHole(m_pSubject->GetVertex(0)))
					return CPolygon2D::resultSubjectPoly;

				//	Otherwise, we return the empty set

				else
					return CPolygon2D::resultEmpty;
				}

			case intersectClipInsideSubject:
				{
				//	If the clip is inside one of the subject's holes, then nothing
				//	happens (we return the subject)

				if (m_pSubject->GetHoleCount() > 0 && m_pSubject->PointIntersectsHole(Clip.GetVertex(0)))
					return CPolygon2D::resultSubjectPoly;

				//	Otherwise, the clip polygon becomes a hole in the subject

				else
					{
					CPolygon2D *pNewPoly = retResult->Insert();
					*pNewPoly = *m_pSubject;
					pNewPoly->AddHole(Clip);

					//	If the clip has holes, then they become separate polygons

					for (i = 0; i < Clip.GetHoleCount(); i++)
						retResult->Insert(Clip.GetHole(i));

					return CPolygon2D::resultOK;
					}
				}

			default:
				return CPolygon2D::resultSubjectPoly;
			}
		}

	//	Make a list of non-intersecting contours from both the subject and the clip
	//	polygons.

	TArray<SVertexInfo *> ClipContours;
	AccumulateNonIntersecting(pClip, &ClipContours);
	TArray<SVertexInfo *> SubjectContours;
	AccumulateNonIntersecting(pSubject, &SubjectContours);

	//	Get a list of inside intersections. This will be the set of vertices
	//	that we'll use to start each of the resulting polygons.

	TArray<SVertexInfo *> Inside;
	GetIntersections(pSubject, intersectInside, &Inside);

	//	Loop over all inside vertices and create a series of polygons

	TArray<CPolygon2D> NewPoly;
	for (i = 0; i < Inside.GetCount(); i++)
		{
		SVertexInfo *pStart = Inside[i];

		//	If we already processed this vertex (i.e., it is already part of
		//	a resulting polygon, then we skip).

		if (pStart->dwFlags & flagProcessed)
			continue;

		//	Prepare an array. We make sure we have enough room for at least as
		//	many points in the original, plus all intersections.

		TArray<CVector2D> NewPoints;
		NewPoints.GrowToFit(m_pSubject->GetVertexCount() + m_iIntersectCount);

		//	Loop until we get back to the starting point, adding the appropriate
		//	points as we go.

		TraceOutline(pStart, opSubtract, &NewPoints);

		//	Add a polygon

		if (NewPoints.GetCount() >= 3)
			{
			CPolygon2D *pNewPoly = NewPoly.Insert();
			pNewPoly->TakeHandoff(NewPoints);
			}
		}

	//	For all the new polygons, some are going to be holes and some are going
	//	to be new polygons. First we add all the new contours that are proper
	//	polygons.

	for (i = 0; i < NewPoly.GetCount(); i++)
		{
		if (NewPoly[i].GetVertexOrder() == dirClockwise)
			{
			CPolygon2D *pNewPoly = retResult->Insert();
			pNewPoly->TakeHandoff(NewPoly[i]);
			}
		}

	//	Add non-intersecting subjects also

	for (i = 0; i < SubjectContours.GetCount(); i++)
		if (!(SubjectContours[i]->dwFlags & flagHole))
			{
			CPolygon2D *pNewPoly = retResult->Insert();
			CreatePolygonFromVertexList(SubjectContours[i], pNewPoly);
			}

	//	Now we need to attack any holes to their corresponding polygons

	for (i = 0; i < NewPoly.GetCount(); i++)
		if (NewPoly[i].GetVertexCount() > 0)
			AddHoleToPolygons(*retResult, NewPoly[i]);

	for (i = 0; i < SubjectContours.GetCount(); i++)
		if (SubjectContours[i]->dwFlags & flagHole)
			{
			CPolygon2D NewHole;
			CreatePolygonFromVertexList(SubjectContours[i], &NewHole);
			AddHoleToPolygons(*retResult, NewHole);
			}

	//	Holes in the clip polygon should be added as a new polygons (only if
	//	they intersect the original subject)

	AddHolesAsPolygons(*m_pSubject, ClipContours, retResult);

	//	Done

	return CPolygon2D::resultOK;
	}

void CPolygonIntersector::TraceOutline (SVertexInfo *pStart, EOpTypes iOp, TArray<CVector2D> *retResult)

//	TraceOutline
//
//	Follows a vertex outline and adds the points to the result.

	{
#ifdef DEBUG_POLYGON_INTERSECT
	int iCount = 0;
#endif

	//	Loop until we get back to the starting point, adding the appropriate
	//	points as we go.

	SVertexInfo *pLastAdded = NULL;
	bool bFlip = false;
	SVertexInfo *pVert = pStart;
	do
		{
		//	Add this point to the list, but only if it is not the same as 
		//	a previously added point. The latter can happen if an intersection
		//	happens at a point.
		//
		//	Also, mark it so we know that we've processed it.

		if (pLastAdded == NULL || pLastAdded->vPos.Distance2(pVert->vPos) > POINT_IDENTITY_EPSILON2)
			{
			retResult->Insert(pVert->vPos);
			pLastAdded = pVert;
			}

		pVert->dwFlags |= flagProcessed;

		if (pVert->pIntersect)
			pVert->pIntersect->dwFlags |= flagProcessed;

		//	If this is an intersection, flip to the other polygon

		if (bFlip && (pVert->pIntersect))
			pVert = pVert->pIntersect;
		else
			bFlip = true;

		//	Our path depends on the operation

		switch (iOp)
			{
			//	For subtract, follow clip polygon in reverse order

			case opSubtract:
				if (pVert->dwFlags & flagClip)
					pVert = pVert->pPrev;
				else
					pVert = pVert->pNext;
				break;

			//	Otherwise, we always follow in forward order

			default:
				pVert = pVert->pNext;
			}

#ifdef DEBUG_POLYGON_INTERSECT
		if (++iCount > 5 * (GetTotalVertexCount(*m_pSubject) + GetTotalVertexCount(*m_pCurClip)))
			{
			m_pCurClip->OutputPolygon(CString("Clip"));
			m_pSubject->OutputPolygon(CString("Subject"));
			break;
			}
#endif
		}
	while (pVert != pStart && pVert->pIntersect != pStart);
	}

CPolygon2D::EOpResult CPolygonIntersector::Union (CPolygon2D *retResult)

//	Union
//
//	Combines a single subject and a single clip region into one result. Or we
//	return resultNone if clip and subject are disjoint.

	{
	int i;

	//	Deal with some degenerate conditions

	if (m_Clip.GetCount() == 0)
		return CPolygon2D::resultEmpty;

	m_pCurClip = m_Clip[0];
	if (m_pCurClip == NULL || m_pCurClip->GetVertexCount() < 3)
		return CPolygon2D::resultEmpty;

	if (m_pSubject == NULL || m_pSubject->GetVertexCount() < 3)
		return CPolygon2D::resultEmpty;

	//	If the subject is completely outside the bounds of the clip region,
	//	then we're done. [This is just an optimization; there is no guarantee
	//	that all non-overlapping polygons will exit here.]

	CRect2D ClipBounds;
	m_pCurClip->CalcBoundingRect(&ClipBounds);

	CRect2D SubjectBounds;
	m_pSubject->CalcBoundingRect(&SubjectBounds);

	if (!SubjectBounds.RectIntersects(ClipBounds))
		return CPolygon2D::resultEmpty;

	//	Initialize the vertex array. This will allocate enough vertex info cells
	//	to do the intersection.

	InitVerticesArray();

	//	Create a chain of vertices for the subject and the clip polygon in
	//	clockwise order.

	SVertexInfo *pSubject = CreateVertexList(*m_pSubject, flagSubject);
	SVertexInfo *pClip = CreateVertexList(*m_pCurClip, flagClip);

	//	Loop over all line segments in the clip polygon and add intersection
	//	points with the subject polygon.

	InsertIntersections(pClip, pSubject);

	//	If there are no intersection, then see if we are disjoint or if one 
	//	polygon is completely inside the other.

	if (m_iIntersectCount == 0)
		{
		switch (GetContainmentRelationship(*m_pSubject, *m_pCurClip))
			{
			case intersectSubjectInsideClip:
			case intersectEquals:
				{
				//	If the clip polygon has holes then we might need to add them
				//	to the resulting polygon.

				if (m_pCurClip->GetHoleCount() > 0)
					return UnionHoles(*m_pCurClip, *m_pSubject, retResult);

				//	Otherwise, we return the clip intact

				else
					{
					*retResult = *m_pCurClip;
					return CPolygon2D::resultOK;
					}
				}

			case intersectClipInsideSubject:
				{
				//	Check to see if we need to add holes from the subject to the
				//	clip result.

				if (m_pSubject->GetHoleCount() > 0)
					return UnionHoles(*m_pSubject, *m_pCurClip, retResult);
				else
					{
					*retResult = *m_pSubject;
					return CPolygon2D::resultOK;
					}
				}

			default:
				return CPolygon2D::resultEmpty;
			}
		}

	//	Make a list of non-intersecting contours from both the subject and the clip
	//	polygons.

	TArray<SVertexInfo *> Contours;
	AccumulateNonIntersecting(pClip, &Contours);
	AccumulateNonIntersecting(pSubject, &Contours);

	//	Get a list of inside intersections. This will be the set of vertices
	//	that we'll use to start each of the resulting polygons.

	TArray<SVertexInfo *> Inside;
	GetIntersections(pSubject, intersectInside, &Inside);

	//	Loop over all vertices and create a polygon.

	TArray<CPolygon2D> NewPoly;
	for (i = 0; i < Inside.GetCount(); i++)
		{
		SVertexInfo *pStart = Inside[i];

		//	If we already processed this vertex (i.e., it is already part of
		//	a resulting polygon, then we skip).

		if (pStart->dwFlags & flagProcessed)
			continue;

		//	Prepare an array. We make sure we have enough room for at least as
		//	many points in the original, plus all intersections.

		TArray<CVector2D> NewPoints;
		NewPoints.GrowToFit(m_pSubject->GetVertexCount() + m_iIntersectCount);

		//	Loop until we get back to the starting point

		TraceOutline(pStart, opIntersect, &NewPoints);

		//	Add a polygon

		if (NewPoints.GetCount() >= 3)
			{
			CPolygon2D *pNewPoly = NewPoly.Insert();
			pNewPoly->TakeHandoff(NewPoints);
			}
		}

	//	Find the main contour and add it as the result (there should only be
	//	one).

	for (i = 0; i < NewPoly.GetCount(); i++)
		{
		if (NewPoly[i].GetVertexOrder() == dirClockwise)
			{
			retResult->TakeHandoff(NewPoly[i]);
			break;
			}
		}

	//	If we haven't found it yet, then look at the unattached contours.

	if (retResult->GetVertexCount() == 0)
		{
		for (i = 0; i < Contours.GetCount(); i++)
			{
			if (!(Contours[i]->dwFlags & flagHole))
				{
				CreatePolygonFromVertexList(Contours[i], retResult);
				break;
				}
			}
		}

	//	If not found, then we're done

	if (retResult->GetVertexCount() == 0)
		return CPolygon2D::resultEmpty;

	//	Attach holes to the main contour

	for (i = 0; i < NewPoly.GetCount(); i++)
		if (NewPoly[i].GetVertexCount() > 0)
			retResult->AddHoleHandoff(NewPoly[i]);

	for (i = 0; i < Contours.GetCount(); i++)
		if (Contours[i]->dwFlags & flagHole)
			{
			CPolygon2D NewHole;
			CreatePolygonFromVertexList(Contours[i], &NewHole);
			retResult->AddHoleHandoff(NewHole);
			}

	//	Done

	return CPolygon2D::resultOK;
	}

CPolygon2D::EOpResult CPolygonIntersector::UnionHoles (const CPolygon2D &Container, const CPolygon2D &Poly, CPolygon2D *retResult)

//	UnionHoles
//
//	Assuming that Poly is contained by Container (no intersections), combine them
//	and return the result.

	{
	int i, j;

	//	Loop over all holes in the container and see if Poly is inside any of them.

	for (i = 0; i < Container.GetHoleCount(); i++)
		{
		switch (GetContainmentRelationship(Container.GetHole(i), Poly))
			{
			//	This hole is inside the bounds of Poly.

			case intersectSubjectInsideClip:
			case intersectEquals:
				{
				//	If Poly has holes, then we need to see if there is any 
				//	containment.

				if (Poly.GetHoleCount() > 0)
					{
					for (j = 0; j < Poly.GetHoleCount(); j++)
						{
						switch (GetContainmentRelationship(Container.GetHole(i), Poly.GetHole(j)))
							{
							//	This container hole is inside the bounds of a Poly hole.
							//	We return the Container intact.

							case intersectSubjectInsideClip:
							case intersectEquals:
								*retResult = Container;
								return CPolygon2D::resultOK;

							//	This Poly hole is inside the Container hole, which means
							//	we've made the Container hole smaller.

							case intersectClipInsideSubject:
								*retResult = Container;
								retResult->DeleteHole(i);
								retResult->AddHole(Poly.GetHole(j));
								return CPolygon2D::resultOK;
	
							//	Otherwise, we're disjoint, which means that this poly hole
							//	overlaps solid Container.
							}
						}

					//	If we get this far, then all Poly holes overlap solid
					//	Container, so we just return the original container

					*retResult = Container;
					return CPolygon2D::resultOK;
					}

				//	Otherwise, the polygon covers up the hole, so we can return
				//	the original container minus this hole.

				else
					{
					*retResult = Container;
					retResult->DeleteHole(i);
					return CPolygon2D::resultOK;
					}

				break;
				}

			//	Poly is inside this hole. This means that the two are disjoint.

			case intersectClipInsideSubject:
				return CPolygon2D::resultEmpty;
			}
		}

	//	If we get this far then there is no relationship, which means we just return
	//	the container.

	*retResult = Container;
	return CPolygon2D::resultOK;
	}
