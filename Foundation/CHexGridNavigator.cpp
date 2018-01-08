//	CHexGridNavigator.cpp
//
//	CHexGridNavigator class
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>

const double POINT_IDENTITY_EPSILON =			1.0e-05;
const double POINT_IDENTITY_EPSILON2 =			POINT_IDENTITY_EPSILON * POINT_IDENTITY_EPSILON;

struct SOffset
	{
	int x;
	int y;
	};

static SOffset HEX_NEIGHBORS_EVEN[6] = 
	{
		{  0, +1 }, { +1,  0 }, { +1, -1 }, {  0, -1 }, { -1, -1 }, { -1,  0 }
	};

static SOffset HEX_NEIGHBORS_ODD[6] =
	{
		{  0, +1 }, { +1, +1 }, { +1,  0 }, {  0, -1 }, { -1,  0 }, { -1, +1 }
	};

static SOffset HEX_CORNERS[6] =
	{
		{ -1, +1 }, { +1, +1 }, { +2,  0 }, { +1, -1 }, { -1, -1 }, { -2,  0 }
	};

CHexGridNavigator::CHexGridNavigator (const CHexGridBase &Grid) :
		m_Grid(Grid)

//	CHexGridNavigator constructor

	{
	}

bool CHexGridNavigator::FindNextOutlinePoint (SHexInfo *&pHex, int &iEdge, SHexInfo *pStopHex, int iStopEdge, const TArray<CVector2D> &HexPoints, CVector2D *retvNextPoint)

//	FindNextOutlinePoint
//
//	Looks the iNextSite and its neighbors to find the next connection to vNextPoint.
//	Updates both iNextSite and vNextPoint.
//
//	Returns FALSE when we hit vEndPoint.

	{
	//	See if the next (clockwise) edge is also unmarked; if it is, then we return it.

	int iNextEdge = (iEdge + 1) % 6;
	if (!IsEdgeMarked(pHex, iNextEdge))
		{
		MarkEdge(pHex, iNextEdge);

		//	Add the point

		CVector2D vCenter = m_Grid.HexXYToPoint(pHex->xHex, pHex->yHex);
		*retvNextPoint = vCenter + HexPoints[iNextEdge];

		//	Update

		iEdge = iNextEdge;
		return true;
		}

	//	Otherwise we need to navigate to our neighbor

	else
		{
		SHexInfo *pNeighbor = pHex->pNeighbor[iNextEdge];
		if (pNeighbor == NULL)
			return false;

		int iNeighborEdge = (iNextEdge + 4) % 6;
		MarkEdge(pNeighbor, iNeighborEdge);

		//	Add the point

		CVector2D vCenter = m_Grid.HexXYToPoint(pNeighbor->xHex, pNeighbor->yHex);
		*retvNextPoint = vCenter + HexPoints[iNeighborEdge];

		//	Update

		pHex = pNeighbor;
		iEdge = iNeighborEdge;
		return true;
		}
	}

int CHexGridNavigator::FindStartEdge (SHexInfo &Hex)

//	FindStartEdge
//
//	Returns an unmarked edge for the given hex. -1 if not found.

	{
	int i;

	for (i = 0; i < 6; i++)
		if (!IsEdgeMarked(&Hex, i))
			return i;

	return -1;
	}

void CHexGridNavigator::GetOutlines (TArray<CPolygon2D> *retResult)

//	GetOutlines
//
//	Returns 0 or more outlines for the selected hexes.

	{
	int i, j;

	retResult->DeleteAll();

	//	Short-circuit

	if (m_Selection.GetCount() == 0)
		return;

	//	Prepare. Mark all edges that we DO NOT need to process and leave the
	//	rest unmarked.

	InitEdgesToProcess();

	//	Generate points for each of the corners of the hexagon. The point for 
	//	the nth neighbor is the counter-clockwise point of the nth edge.

	TArray<CVector2D> HexPoints;
	HexPoints.InsertEmpty(6);

	double xCorner = 0.5 * m_Grid.GetRadius();
	double yCorner = 0.5 * m_Grid.GetDY();

	for (i = 0; i < 6; i++)
		{
		HexPoints[i].SetX(HEX_CORNERS[i].x * xCorner);
		HexPoints[i].SetY(HEX_CORNERS[i].y * yCorner);
		}

#ifdef DEBUG_HEX_GRIDS
	for (i = 0; i < m_Selection.GetCount(); i++)
		{
		SHexInfo &Hex = m_Selection[i];
		if (AreAllEdgesMarked(Hex))
			{
			CVector2D vCenter = m_Grid.HexXYToPoint(Hex.xHex, Hex.yHex);

			TArray<CVector2D> Points;
			Points.InsertEmpty(6);
			for (j = 0; j < 6; j++)
				Points[j] = vCenter + HexPoints[j];

			CPolygon2D *pPoly = retResult->Insert();
			pPoly->TakeHandoff(Points);
			}
		}

	return;
#endif

	//	Keep track of all outlines

	TArray<CPolygon2D> Outlines;

	//	Keep looping until we've processed all hexes

	while (true)
		{
		TArray<CVector2D> Points;

		//	Pick an unmarked hex to start a new outline.

		int iStart = -1;
		for (i = 0; i < m_Selection.GetCount(); i++)
			if (!AreAllEdgesMarked(m_Selection[i]))
				{
				iStart = i;
				break;
				}

		//	If none, then we're done

		if (iStart == -1)
			break;

		SHexInfo &StartHex = m_Selection[iStart];

		//	Pick the first edge on the site that we can find

		int iStartEdge = FindStartEdge(StartHex);
		if (iStartEdge == -1)
			{
			ASSERT(false);
			continue;
			}

		//	Mark the hex and compute the edge

		MarkEdge(&StartHex, iStartEdge);
		CVector2D vCenter = m_Grid.HexXYToPoint(StartHex.xHex, StartHex.yHex);
		CVector2D vFrom = vCenter + HexPoints[iStartEdge];

		//	Add both edge points to the array

		Points.Insert(vFrom);

		//	Given a site and point, find the next site and point until we get 
		//	back to From.

		SHexInfo *pNextHex = &StartHex;
		int iNextEdge = iStartEdge;
		CVector2D vNextPoint;
		while (true)
			{
			if (!FindNextOutlinePoint(pNextHex, iNextEdge, &StartHex, iStartEdge, HexPoints, &vNextPoint))
				break;

			if (pNextHex == &StartHex && iNextEdge == iStartEdge)
				break;

			Points.Insert(vNextPoint);
			}

		//	We should now have a complete outline, so add it to our list.

		CPolygon2D *pNewOutline = Outlines.Insert();
		pNewOutline->TakeHandoff(Points);
		}

	//	Now process all the outlines and figure out if any are holes

	for (i = 0; i < Outlines.GetCount(); i++)
		{
		if (Outlines[i].GetVertexCount() == 0)
			continue;

		for (j = 0; j < Outlines.GetCount(); j++)
			{
			if (i == j || Outlines[j].GetVertexCount() == 0)
				continue;

			//	If j is inside i, then it is a hole. This will also remove all
			//	of the points from the j polygon so we never test it again.

			if (Outlines[i].PointIntersects(Outlines[j].GetVertex(0), false))
				Outlines[i].AddHoleHandoff(Outlines[j]);
			}
		}

	//	Return result

	for (i = 0; i < Outlines.GetCount(); i++)
		if (Outlines[i].GetVertexCount() >= 3)
			{
			CPolygon2D *pNewPoly = retResult->Insert();
			pNewPoly->TakeHandoff(Outlines[i]);
			}
	}

void CHexGridNavigator::InitEdgesToProcess (void)

//	InitEdgesToProcess
//
//	Leaves all edges that need to be processed unmarked, and marks all other
//	edges.

	{
	int i, j;

	for (i = 0; i < m_Selection.GetCount(); i++)
		{
		SHexInfo &Hex = m_Selection[i];
		if (Hex.iNeighborCount == 6)
			Hex.dwFlags |= flagAllEdgeMarks;
		else
			{
			Hex.dwFlags &= ~flagAllEdgeMarks;

			//	Loop over all edges and mark the ones that lead to a selected
			//	hex.

			for (j = 0; j < 6; j++)
				if (Hex.pNeighbor[j])
					MarkEdge(&Hex, j);
			}
		}
	}

void CHexGridNavigator::MarkEdge (SHexInfo *pHex, int iEdge)

//	MarkEdge
//
//	Marks the given edge

	{
	pHex->dwFlags |= (flagEdge0Marked << iEdge);
	}

void CHexGridNavigator::SelectHexes (const TArray<int> &HexList)

//	SelectHexes
//
//	Selects the given hexes (by index)

	{
	int i, j;

	m_Selection.DeleteAll();
	m_Selection.InsertEmpty(HexList.GetCount());

	m_SelectionSet.ClearAll();

	//	Keep a temporary map of index to entry

	TSortMap<int, int> IndexToSelection;

	//	Add each of the hexes in the selection

	for (i = 0; i < HexList.GetCount(); i++)
		{
		SHexInfo &Hex = m_Selection[i];

		Hex.iIndex = HexList[i];
		m_Grid.IndexToHexXY(Hex.iIndex, &Hex.xHex, &Hex.yHex);

		Hex.dwFlags = (((Hex.xHex % 2) == 0) ? 0 : flagOdd);
		Hex.iNeighborCount = 0;

		m_SelectionSet.Set(Hex.iIndex);
		IndexToSelection.Insert(Hex.iIndex, i);
		}

	//	Now loop over all hexes in selection and add its neighbors (looking
	//	only at other hexes in the selection).

	for (i = 0; i < m_Selection.GetCount(); i++)
		{
		SHexInfo &Hex = m_Selection[i];
		SOffset *pOffsets = ((Hex.dwFlags & flagOdd) ? HEX_NEIGHBORS_ODD : HEX_NEIGHBORS_EVEN);

		for (j = 0; j < 6; j++)
			{
			int xNeighbor = Hex.xHex + pOffsets[j].x;
			int yNeighbor = Hex.yHex + pOffsets[j].y;
			int iNeighbor = m_Grid.HexXYToIndex(xNeighbor, yNeighbor);

			//	Skip neighbors that are off the grid or not part of the 
			//	selection.

			if (iNeighbor == -1 || !m_SelectionSet.IsSet(iNeighbor))
				{
				Hex.pNeighbor[j] = NULL;
				continue;
				}

			//	Add this neighbor

			Hex.pNeighbor[j] = &m_Selection[*IndexToSelection.GetAt(iNeighbor)];
			Hex.iNeighborCount++;
			}
		}
	}
