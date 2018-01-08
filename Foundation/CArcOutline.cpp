//	CArcOutline.cpp
//
//	CArcOutline class
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>

void CArcOutline::AddCircle (const CCircle2D &NewCircle)

//	AddCircle
//
//	Adds a circle to the outline.

	{
	int i;

	//	Add the new circle.

	SCircle *pNewCircle = m_Circles.Insert();
	pNewCircle->vCenter = NewCircle.GetCenter();
	pNewCircle->rRadius = NewCircle.GetRadius();

	//	Loop over all circles and see if we intersect any of them

	for (i = 0; i < m_Circles.GetCount() - 1; i++)
		{
		SCircle *pCircle = &m_Circles[i];

		//	Intersect with this circle

		SArc Arc1;
		SArc Arc2;
		EResultTypes iResult = Intersect(pCircle, pNewCircle, &Arc1, &Arc2);

		//	If there is no intersection, continue.

		if (iResult == resultNoIntersection)
			continue;

		//	If pCircle is overlapped by the new circle, then we just delete it.

		else if (iResult == resultOverlapped1)
			{
			pCircle->bDelete = true;
			break;
			}

		//	If the new circle is overlapped by this circle, then we delete the
		//	new circle.

		else if (iResult == resultOverlapped2)
			{
			pNewCircle->bDelete = true;
			break;
			}

		//	Otherwise we combine the arcs

		else
			{
			CombineArc(pCircle, Arc1);
			CombineArc(pNewCircle, Arc2);
			}
		}
	}

void CArcOutline::CombineArc (SCircle *pCircle, const SArc &Arc)

//	CombineArc
//
//	Combines the arc with the set of arcs in the circle. Leaves the circle as
//	the intersection of all arcs with the given one.

	{
	int i;

	//	If the destination circle has already been deleted, then
	//	no need to do anything.

	if (pCircle->bDelete)
		return;

	//	If we have no arcs then it means that the circle is still whole. We just
	//	take the arc.

	if (pCircle->Arcs.GetCount() == 0)
		pCircle->Arcs.Insert(Arc);

	//	Otherwise we loop over all arcs and intersect

	else
		{
		TArray<SArc> NewArcs;
		bool b0Cross = (Arc.rStart > Arc.rEnd);

		for (i = 0; i < pCircle->Arcs.GetCount(); i++)
			{
			SArc &Arc2 = pCircle->Arcs[i];
			bool b0Cross2 = (Arc2.rStart > Arc2.rEnd);

			//	If neither arc crosses 0, then we compare

			if (!b0Cross && !b0Cross2)
				{
				if (Arc.rStart <= Arc2.rStart)
					{
					//	arc1:	|----|
					//	arc2:			|----------|

					if (Arc.rEnd <= Arc2.rStart)
						{
						Arc2.rStart = 0.0;
						Arc2.rEnd = 0.0;
						}

					//	arc1:	|-------------------|
					//	arc2:		|----------|

					else if (Arc.rEnd >= Arc2.rEnd)
						;

					//	arc1:	|---------|
					//	arc2:		|----------|

					else
						Arc2.rEnd = Arc.rEnd;
					}
				else
					{
					//	arc1:			|----------|
					//	arc2:	|----|

					if (Arc.rStart >= Arc2.rEnd)
						{
						Arc2.rStart = 0.0;
						Arc2.rEnd = 0.0;
						}

					//	arc1:		|----------|
					//	arc2:	|-------------------|

					else if (Arc.rEnd <= Arc2.rEnd)
						Arc2 = Arc;

					//	arc1:		|----------|
					//	arc2:	|---------|

					else
						Arc2.rStart = Arc.rStart;
					}
				}

			//	If both cross 0

			else if (b0Cross && b0Cross2)
				{
				if (Arc2.rEnd < Arc.rEnd)
					{
					//	arc1:	|-------|		|---|
					//	arc2:	|--| |--------------|

					if (Arc2.rStart < Arc.rEnd)
						{
						SArc *pNewArc = NewArcs.Insert();
						pNewArc->rStart = Arc.rStart;
						pNewArc->rEnd = Arc2.rEnd;

						Arc2.rEnd = Arc.rEnd;
						}

					//	arc1:	|-------|		|---|
					//	arc2:	|--|		|-------|
					//
					//	OR
					//
					//	arc1:	|-------|		|---|
					//	arc2:	|--|		     |--|

					else
						{
						Arc2.rStart = Max(Arc.rStart, Arc2.rStart);
						Arc2.rEnd = Min(Arc.rEnd, Arc2.rEnd);
						}
					}
				else if (Arc2.rEnd < Arc.rStart)
					{
					//	arc1:	|-------|		|---|
					//	arc2:	|---------| |-------|
					//
					//	OR
					//
					//	arc1:	|-------|		|---|
					//	arc2:	|---------|      |--|

					Arc2.rStart = Max(Arc.rStart, Arc2.rStart);
					Arc2.rEnd = Min(Arc.rEnd, Arc2.rEnd);
					}
				else
					{
					//	arc1:	|-------|	  |-----|
					//	arc2:	|---------------| |-|

					SArc *pNewArc = NewArcs.Insert();
					pNewArc->rStart = Arc.rStart;
					pNewArc->rEnd = Arc2.rEnd;

					Arc2.rEnd = Arc.rEnd;
					}
				}

			//	Arc1 crosses 0

			else if (b0Cross)
				{
				if (Arc2.rStart < Arc.rEnd)
					{
					//	arc1:	|-------|		|---|
					//	arc2:		|--|

					if (Arc2.rEnd <= Arc.rEnd)
						;

					//	arc1:	|-------|		|---|
					//	arc2:		|-------|

					else if (Arc2.rEnd < Arc.rStart)
						Arc2.rEnd = Arc.rEnd;

					//	arc1:	|-------|		|---|
					//	arc2:		|-------------|

					else
						{
						SArc *pNewArc = NewArcs.Insert();
						pNewArc->rStart = Arc.rStart;
						pNewArc->rEnd = Arc2.rEnd;

						Arc2.rEnd = Arc.rEnd;
						}
					}
				else
					{
					//	arc1:	|-------|		|---|
					//	arc2:					 |-|

					if (Arc2.rStart >= Arc.rStart)
						;

					//	arc1:	|-------|		|---|
					//	arc2:				|--|

					else if (Arc2.rEnd <= Arc.rStart)
						{
						Arc2.rStart = 0.0;
						Arc2.rEnd = 0.0;
						}

					//	arc1:	|-------|		|---|
					//	arc2:				|-----|

					else
						Arc2.rStart = Arc.rStart;
					}
				}

			//	Arc2 crosses 0

			else
				{
				if (Arc.rStart < Arc2.rEnd)
					{
					//	arc1:		|--|
					//	arc2:	|-------|		|---|

					if (Arc.rEnd <= Arc2.rEnd)
						Arc2 = Arc;

					//	arc1:		|-------|
					//	arc2:	|-------|		|---|

					else if (Arc.rEnd < Arc2.rStart)
						Arc2.rStart = Arc.rStart;

					//	arc1:		|-------------|
					//	arc2:	|-------|		|---|

					else
						{
						SArc *pNewArc = NewArcs.Insert();
						pNewArc->rStart = Arc.rStart;
						pNewArc->rEnd = Arc2.rEnd;

						Arc2.rEnd = Arc.rEnd;
						}
					}
				else
					{
					//	arc1:					 |-|
					//	arc2:	|-------|		|---|

					if (Arc.rStart >= Arc2.rStart)
						Arc2 = Arc;

					//	arc1:				|--|
					//	arc2:	|-------|		|---|

					else if (Arc.rEnd <= Arc2.rStart)
						{
						Arc2.rStart = 0.0;
						Arc2.rEnd = 0.0;
						}

					//	arc1:				|-----|
					//	arc2:	|-------|		|---|

					else
						Arc2.rEnd = Arc.rEnd;
					}
				}
			}

		//	Now delete any arcs that we zeroed out.

		for (i = 0; i < pCircle->Arcs.GetCount(); i++)
			{
			if (pCircle->Arcs[i].rStart == pCircle->Arcs[i].rEnd)
				{
				pCircle->Arcs.Delete(i);
				i--;
				}
			}

		//	Add any additional arcs

		for (i = 0; i < NewArcs.GetCount(); i++)
			pCircle->Arcs.Insert(NewArcs[i]);

		//	If we have no more arcs then we delete the circle

		if (pCircle->Arcs.GetCount() == 0)
			pCircle->bDelete = true;
		}
	}

void CArcOutline::GetSortedArcList (TArray<CArc2D> *retList)

//	GetSortedArcList
//
//	Generates a list of arcs. The list is sorted so that each arc continues to 
//	next arc around the outline.

	{
	int i, j;

	//	Create a linked list of all unsorted arcs.

	SArcEntry *pUnsortedList = NULL;
	for (i = 0; i < m_Circles.GetCount(); i++)
		{
		SCircle *pCircle = &m_Circles[i];

		//	If deleted, then skip

		if (pCircle->bDelete)
			NULL;

		//	If no arcs, then this is a complete circle (we automatically
		//	put it in the result

		else if (pCircle->Arcs.GetCount() == 0)
			{
			CArc2D *pNewArc = retList->Insert();
			pNewArc->SetCenter(pCircle->vCenter);
			pNewArc->SetRadius(pCircle->rRadius);
			}

		//	Otherwise, add each of the arcs to the unsorted list.

		else
			{
			for (j = 0; j < pCircle->Arcs.GetCount(); j++)
				{
				SArcEntry *pNewArc = new SArcEntry;
				pNewArc->pCircle = pCircle;
				pNewArc->pArc = &pCircle->Arcs[j];

				//	Compute the arc end-points

				pNewArc->vStart = pCircle->vCenter + CVector2D::FromPolar(pNewArc->pArc->rStart, pCircle->rRadius);
				pNewArc->vEnd = pCircle->vCenter + CVector2D::FromPolar(pNewArc->pArc->rEnd, pCircle->rRadius);

				//	Add to the beginning of the unsorted list.
				//	(Since it is unsorted, we don't care.)

				pNewArc->pNext = pUnsortedList;
				pUnsortedList = pNewArc;
				}
			}
		}

	//	While we have stuff in the unsorted list, process it into the sorted
	//	list.

	while (pUnsortedList)
		{
		const double EPSILON = 0.001;

		//	Start a list of sorted arcs.

		SArcEntry *pSortedList = NULL;
		SArcEntry *pSortedListEnd = NULL;
		CVector2D vSortedEnd;

		//	Add arcs to the sorted list until we can't add any more

		while (true)
			{
			//	Look for something to add to the sorted list.

			SArcEntry *pPrevArc = NULL;
			SArcEntry *pArcToAdd = pUnsortedList;
			while (pArcToAdd)
				{
				//	If the sorted list is empty then we can add anything.

				if (pSortedListEnd == NULL)
					break;

				//	Otherwise, see if this arc is adjacent to the current
				//	end point

				else if (Abs(vSortedEnd.X() - pArcToAdd->vStart.X()) < EPSILON
						&& Abs(vSortedEnd.Y() - pArcToAdd->vStart.Y()) < EPSILON)
					break;

				//	Else continue searching

				pPrevArc = pArcToAdd;
				pArcToAdd = pArcToAdd->pNext;
				}

			//	If we found something to add...

			if (pArcToAdd)
				{
				//	Remove the arc from the unsorted list.

				if (pPrevArc)
					pPrevArc->pNext = pArcToAdd->pNext;
				else
					pUnsortedList = pArcToAdd->pNext;

				//	Add to sorted list

				if (pSortedListEnd)
					pSortedListEnd->pNext = pArcToAdd;
				else
					pSortedList = pArcToAdd;

				pSortedListEnd = pArcToAdd;
				pArcToAdd->pNext = NULL;

				//	Remember the end point

				vSortedEnd = pArcToAdd->vEnd;

				//	Keep looping
				}

			//	Otherwise, we're done

			else
				break;
			}

		//	Now add the sorted list to the result

		SArcEntry *pEntry = pSortedList;
		while (pEntry)
			{
			//	Add to result

			CArc2D *pNewArc = retList->Insert();
			pNewArc->SetCenter(pEntry->pCircle->vCenter);
			pNewArc->SetRadius(pEntry->pCircle->rRadius);
			pNewArc->SetStart(pEntry->pArc->rStart);
			pNewArc->SetEnd(pEntry->pArc->rEnd);

			//	Next

			SArcEntry *pToDelete = pEntry;
			pEntry = pEntry->pNext;

			//	Delete the entry

			delete pToDelete;
			}
		}
	}

CArcOutline::EResultTypes CArcOutline::Intersect (SCircle *pC1, SCircle *pC2, SArc *retC1Arc, SArc *retC2Arc)

//	Intersect
//
//	Returns TRUE if the two circles intersect. If so, we return the two 
//	intersection points.

	{
	CVector2D vDist = pC2->vCenter - pC1->vCenter;
	double rDist2 = vDist.Length2();

	double rMaxDist = pC1->rRadius + pC2->rRadius;

	//	If 0, then the circles are on the same spot.

	if (rDist2 == 0.0)
		{
		//	Either C1 overlaps C2 or C2 overlaps C1.

		if (pC1->rRadius < pC2->rRadius)
			return resultOverlapped1;
		else
			return resultOverlapped2;
		}

	//	No intersection

	if (rDist2 >= (rMaxDist * rMaxDist))
		return resultNoIntersection;

	//	We intersect. Compute by how much

	double rDist = sqrt(rDist2);

	//	If the distance + radius1 <= radiu2 then circle1 is fully overlapped.

	if (rDist + pC1->rRadius <= pC2->rRadius)
		return resultOverlapped1;

	//	Same for the other circle

	if (rDist + pC2->rRadius <= pC1->rRadius)
		return resultOverlapped2;

	//	Compute the distance from the center of circle1 to the mid-point of
	//	the intersection.
	//
	//	See: http://mathworld.wolfram.com/Circle-CircleIntersection.html

	double rIntersection = ((rDist * rDist) - (pC2->rRadius * pC2->rRadius) + (pC1->rRadius * pC1->rRadius)) / (2.0 * rDist);

	//	Compute the angle from the intersection axis to the arc

	double rA1 = acos(rIntersection / pC1->rRadius);

	//	Compute the angle of the axis from circle1 to circle2.

	double rAxis = vDist.Polar();

	//	Compute the starting and ending angles of the arc of circle 1

	retC1Arc->rStart = mathAngleMod(rAxis + rA1);
	retC1Arc->rEnd = mathAngleMod(rAxis - rA1);

	//	Compute angle for second circle (negative numerator because the point is
	//	towards C1).

	double rA2 = acos((rIntersection - rDist) / pC2->rRadius);

	//	Now the arcs

	retC2Arc->rStart = mathAngleMod(rAxis - rA2);
	retC2Arc->rEnd = mathAngleMod(rAxis + rA2);

	//	Done

	return resultIntersect;
	}
