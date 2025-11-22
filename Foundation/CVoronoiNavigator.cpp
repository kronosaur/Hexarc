//	CVoronoiNavigator.cpp
//
//	CVoronoiNavigator class
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const DWORD ID_NO_SELECTION =					0xffffffff;
const double POINT_IDENTITY_EPSILON =			1.0e-05;
const double POINT_IDENTITY_EPSILON2 =			POINT_IDENTITY_EPSILON * POINT_IDENTITY_EPSILON;

inline bool AreAllEdgesSet (DWORD dwEdgeFlags, int iCount) { DWORD dwMask = (1 << iCount) - 1; return (dwEdgeFlags & dwMask) == dwMask; }
inline bool IsEdgeSet (DWORD dwEdgeFlags, int iEdge) { return (dwEdgeFlags & (1 << iEdge)) != 0; }

CVoronoiNavigator::CVoronoiNavigator (CVoronoiTessellation &Graph) :
		m_Graph(Graph)

//	CVoronoiNavigator constructor

	{
	}

bool CVoronoiNavigator::FindNextEdge (int iSite, CVector2D &vNextPoint, int *retiEdge)

//	FindNextEdge
//
//	Looks for an edge on the given site that connection to vNextPoint.
//
//	Returns FALSE if we could not find a point.

	{
	int i;
	DWORD dwSelIndex = m_Graph.GetSiteData(iSite);
	ASSERT(dwSelIndex != ID_NO_SELECTION);
	DWORD dwEdgeFlags = m_Selection[dwSelIndex].dwEdgesProcessed;

	for (i = 0; i < m_Graph.GetNeighborCount(iSite); i++)
		{
		if (!IsEdgeSet(dwEdgeFlags, i))
			{
			int iNeighbor = m_Graph.GetNeighbor(iSite, i);
			const CLine2D &vEdge = m_Graph.GetNeighborEdge(iSite, i);

			if (vNextPoint.IsEqualTo(vEdge.From(), POINT_IDENTITY_EPSILON2))
				{
				vNextPoint = vEdge.To();
				*retiEdge = i;
				return true;
				}
			else if (vNextPoint.IsEqualTo(vEdge.To(), POINT_IDENTITY_EPSILON2))
				{
				vNextPoint = vEdge.From();
				*retiEdge = i;
				return true;
				}
			}
		}

	return false;
	}

bool CVoronoiNavigator::FindNextOutlinePoint (int &iNextSite, CVector2D &vNextPoint, const CVector2D &vEndPoint, int *retiEdge)

//	FindNextOutlinePoint
//
//	Looks the iNextSite and its neighbors to find the next connection to vNextPoint.
//	Updates both iNextSite and vNextPoint.
//
//	Returns FALSE when we hit vEndPoint.

	{
	int i;

	if (FindNextEdge(iNextSite, vNextPoint, retiEdge))
		{
		//	If we hit the end point then we're done

		if (vEndPoint.IsEqualTo(vNextPoint, POINT_IDENTITY_EPSILON2))
			return false;

		//	Otherwise, we found the point

		return true;
		}

	//	If we got this far then we need to continue with a neighbor of the site

	for (i = 0; i < m_Graph.GetNeighborCount(iNextSite); i++)
		{
		int iNeighbor = m_Graph.GetNeighbor(iNextSite, i);
		DWORD dwNeighborSel = m_Graph.GetSiteData(iNeighbor);
		if (dwNeighborSel != ID_NO_SELECTION
				&& !m_Selection[dwNeighborSel].bMarked)
			{
			if (FindNextEdge(iNeighbor, vNextPoint, retiEdge))
				{
				//	If we hit the end point then we're done

				if (vEndPoint.IsEqualTo(vNextPoint, POINT_IDENTITY_EPSILON2))
					return false;

				//	Otherwise, we found the point

				iNextSite = iNeighbor;
				return true;
				}
			}
		}

	//	This means we're done

	return false;
	}

int CVoronoiNavigator::FindStartEdge (int iSiteIndex)

//	FindStartEdge
//
//	Finds an edge to an unmarked site in the selection.

	{
	int i;
	DWORD dwSelIndex = m_Graph.GetSiteData(iSiteIndex);
	ASSERT(dwSelIndex != ID_NO_SELECTION);
	DWORD dwEdgeFlags = m_Selection[dwSelIndex].dwEdgesProcessed;

	for (i = 0; i < m_Graph.GetNeighborCount(iSiteIndex); i++)
		{
		if (!IsEdgeSet(dwEdgeFlags, i))
			return i;
		}

	//	Not found

	return -1;
	}

void CVoronoiNavigator::GetOutlines (TArray<CPolygon2D> *retResult)

//	GetOutlines
//
//	Returns combined outlines for selected sites.

	{
	int i, j;

	retResult->DeleteAll();

	//	Short-circuit

	if (m_Selection.GetCount() == 0)
		return;

	//	Loop over all selected sites and mark the ones that are fully surrounded
	//	by selected sites. [The unmarked sites are the edges that need to be
	//	processed.]

	InitEdgesToProcess();

	//	Keep track of outlines

	TArray<CPolygon2D> Outlines;

	//	Keep looping until we've processed all hexes

	while (true)
		{
		TArray<CVector2D> Points;

		//	Pick an unmarked site to start a new outline.

		int iStart = -1;
		for (i = 0; i < m_Selection.GetCount(); i++)
			if (!m_Selection[i].bMarked)
				{
				iStart = i;
				break;
				}

		//	If none, then we're done

		if (iStart == -1)
			break;

		//	Pick the first edge on the site that we can find

		int iStartEdge = FindStartEdge(m_Selection[iStart].iIndex);
		if (iStartEdge == -1)
			{
			ASSERT(false);
			m_Selection[iStart].bMarked = true;
			continue;
			}

		//	Mark and retrieve the edge

		const CLine2D &vStartEdge = m_Graph.GetNeighborEdge(m_Selection[iStart].iIndex, iStartEdge);
		SetEdge(m_Selection[iStart], iStartEdge);

		//	Add both endpoints to the array

		Points.Insert(vStartEdge.From());
		Points.Insert(vStartEdge.To());

		//	Given a site and point, find the next site and point until we get 
		//	back to From.

		int iNextSite = m_Selection[iStart].iIndex;
		CVector2D vNextPoint = vStartEdge.To();
		int iEdge;
		while (FindNextOutlinePoint(iNextSite, vNextPoint, vStartEdge.From(), &iEdge))
			{
			Points.Insert(vNextPoint);

			//	Mark the edge as processed

			DWORD dwSelIndex = m_Graph.GetSiteData(iNextSite);
			ASSERT(dwSelIndex != ID_NO_SELECTION);
			SetEdge(m_Selection[dwSelIndex], iEdge);
			}

		//	We should now have a complete outline, so add it to our list.

		CPolygon2D *pNewOutline = Outlines.Insert();
		pNewOutline->TakeHandoff(Points);
		}

	//	Now that we have a list of outlines we need to figure out if any of them
	//	are holes.

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

	//	Done

	for (i = 0; i < Outlines.GetCount(); i++)
		if (Outlines[i].GetVertexCount() >= 3)
			{
			CPolygon2D *pNewPoly = retResult->Insert();
			pNewPoly->TakeHandoff(Outlines[i]);
			}
	}

void CVoronoiNavigator::InitEdgesToProcess (void)

//	InitEdgesToProcess
//
//	Initializes the set of edges that we need to process.

	{
	int i, j;

	for (i = 0; i < m_Selection.GetCount(); i++)
		{
		m_Selection[i].dwEdgesProcessed = 0;
		m_Selection[i].bMarked = false;

		//	For each edge, see if we need to process it.

		int iSiteIndex = m_Selection[i].iIndex;
		for (j = 0; j < m_Graph.GetNeighborCount(iSiteIndex); j++)
			{
			int iNeighborIndex = m_Graph.GetNeighbor(iSiteIndex, j);

			//	If this neighbor part of the selection then we mark it as 
			//	already processed.

			if (m_Graph.GetSiteData(iNeighborIndex) != ID_NO_SELECTION)
				SetEdge(m_Selection[i], j);
			}
		}
	}

bool CVoronoiNavigator::IsSiteEdge (int iSiteIndex)

//	IsSiteEdge
//
//	Returns TRUE if site is NOT completely surrounded by selected sites.

	{
	int i;

	//	Loop over all neighbors of this site

	for (i = 0; i < m_Graph.GetNeighborCount(iSiteIndex); i++)
		{
		int iNeighborIndex = m_Graph.GetNeighbor(iSiteIndex, i);

		//	If this neighbor is not part of the selection, then we are clearly
		//	an edge.

		if (m_Graph.GetSiteData(iNeighborIndex) == ID_NO_SELECTION)
			return true;
		}

	//	If all our neighbors are part of the selection, then we are clearly not
	//	an edge.

	return false;
	}

void CVoronoiNavigator::SelectSites (const TArray<int> &SiteList)

//	SelectSites
//
//	Selects the given sites.

	{
	int i;

	for (i = 0; i < m_Graph.GetSiteCount(); i++)
		m_Graph.SetSiteData(i, ID_NO_SELECTION);

	m_Selection.DeleteAll();
	m_Selection.InsertEmpty(SiteList.GetCount());

	for (i = 0; i < SiteList.GetCount(); i++)
		{
		//	Map from our selection data to the site index

		m_Selection[i].iIndex = SiteList[i];

		//	Map from the site to our selection data

		m_Graph.SetSiteData(SiteList[i], i);
		}
	}

void CVoronoiNavigator::SetEdge (SSiteInfo &Site, int iEdge)

//	SetEdge
//
//	Sets the edge for the given site

	{
	Site.dwEdgesProcessed |= (1 << iEdge);
	if (AreAllEdgesSet(Site.dwEdgesProcessed, m_Graph.GetNeighborCount(Site.iIndex)))
		Site.bMarked = true;
	}
