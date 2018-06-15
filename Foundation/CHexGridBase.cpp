//	CHexGridBase.cpp
//
//	CHexGridBase class
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.
//
//    ____        ____
//   /    \      /    \
//	/  0   \____/  2   \
//	\-1,1  /    \ 1,1  /
//	 \____/  1   \____/
//	 /    \ 0,1  /    \
//	/  3   \____/  5   \
//	\-1,0  /    \ 1,0  /
//	 \____/  4   \____/
//	 /    \ 0,0  /    \
//	/  6   \____/  8   \
//  \-1,-1 /    \ 1,-1 /
//   \____/  7   \____/
//        \ 0,-1 /    
//         \____/      


#include "stdafx.h"
#include <Math.h>

CHexGridBase::CHexGridBase (double rRadius, double rWidth, double rHeight)

//	CHexGridBase constructor

	{
	Init(rRadius, rWidth, rHeight);
	}

bool CHexGridBase::GetHexesInRange (const CVector2D &vPos, double rRange, TArray<int> *retHexes) const

//	GetHexesInRange
//
//	Returns a list of hex indices that are in range of the given point.

	{
	ASSERT(rRange >= 0.0);

	//	Convert to hex x,y coordinates

	int xCenter;
	int yCenter;
	PointToHexXY(vPos, &xCenter, &yCenter);

	//	Figure out the range in hexes (we add a little bit to account for 
	//	points that might be at the edge of one hex.

	int xRange = (int)((rRange + m_rRadius) / m_rDX);
	int yRange = (int)((rRange + m_rRadius) / m_rDY) + 1;	//	Add 1 because sometimes of odd/even hexes
	int cxCount = 1 + 2 * xRange;
	int cyCount = 1 + 2 * yRange;

	//	Size the result appropriately

	retHexes->DeleteAll();
	retHexes->GrowToFit(cxCount * cyCount);

	//	Loop over all hexes in range and add the ones that are in the circle

	double rRange2 = rRange * rRange;

	int y = yCenter - yRange;
	int yEnd = y + cyCount;
	while (y < yEnd)
		{
		int x = xCenter - xRange;
		int xEnd = x + cxCount;
		while (x < xEnd)
			{
			//	If we're a valid hex and we're within the specified range, then
			//	we add the hex index to the result list.

			int iHexIndex = HexXYToIndex(x, y);
			if (iHexIndex != -1
					&& vPos.Distance2(HexXYToPoint(x, y)) <= rRange2)
				retHexes->Insert(iHexIndex);

			x++;
			}

		y++;
		}

	return (retHexes->GetCount() > 0);
	}

void CHexGridBase::GetOutlines (const TArray<int> &HexList, TArray<CPolygon2D> *retResult) const

//	GetOutline
//
//	Returns a list of polygons sufficient to outline the given list of hexes.

	{
	CHexGridNavigator Navigator(*this);
	Navigator.SelectHexes(HexList);
	Navigator.GetOutlines(retResult);
	}

bool CHexGridBase::HasMore (SIterator &i, EIterationTypes iType) const

//	HasMore
//
//	Returns TRUE if SelectNext will succeed. If this return FALSE, SelectNext is
//	undefined.

	{
	switch (iType)
		{
		case iterateIndex:
			return (i.iHexIndex + 1 < m_iCount);

		default:
			return false;
		}
	}

int CHexGridBase::HexXYToIndex (int x, int y) const

//	HexXYToIndex
//
//	Converts from x, y hex coordinates to a hex index. If the coordinates are
//	out of range, we return -1.

	{
	if (x > m_cxHexHalfCount || x < -m_cxHexHalfCount
			|| y > m_cyHexHalfCount || y < -m_cyHexHalfCount)
		return -1;

	return m_iCenterHex + (y * m_cxHexCount) + x;
	}

CVector2D CHexGridBase::HexXYToPoint (int x, int y) const

//	HexXYToPoint
//
//	Converts from a hex coordinate to a position.

	{
	return CVector2D(x * m_rDX, 
			(((x % 2) == 0) 
				? y * m_rDY 
				: (y * m_rDY) + m_rHalfDY)
				);
	}

void CHexGridBase::IndexToHexXY (int iIndex, int *retx, int *rety) const

//	IndexToHexXY
//
//	Returns the hex x,y coordinate of the given hex.

	{
	*rety = (iIndex / m_cxHexCount) - m_cyHexHalfCount;
	*retx = (iIndex % m_cxHexCount) - m_cxHexHalfCount;
	}

CVector2D CHexGridBase::IndexToPoint (int iIndex) const

//	IndexToPoint
//
//	Returns the center of the given hex.

	{
	int xHex;
	int yHex;
	IndexToHexXY(iIndex, &xHex, &yHex);
	return HexXYToPoint(xHex, yHex);
	}

void CHexGridBase::Init (double rRadius, double rWidth, double rHeight)

//	Init
//
//	Initialize

	{
	ASSERT(rRadius > 0.0);
	ASSERT(rWidth > 0.0);
	ASSERT(rHeight > 0.0);

	m_rRadius = rRadius;
	m_rHalfRadius = 0.5 * m_rRadius;

	m_rDX = 1.5 * m_rRadius;
	m_rDY = rRadius * SQRT_3;
	m_rHalfDY = 0.5 * m_rDY;

	m_rWidth = rWidth;
	m_rHeight = rHeight;

	//	Compute the number of hexes

	m_cxHexHalfCount = (int)ceil((0.5 * rWidth) / m_rDX);
	m_cyHexHalfCount = (int)ceil((0.5 * rHeight) / m_rDY);
	m_cxHexCount = 1 + (2 * m_cxHexHalfCount);
	m_cyHexCount = 1 + (2 * m_cyHexHalfCount);
	m_iCount = m_cxHexCount * m_cyHexCount;

	m_iCenterHex = (m_cyHexHalfCount * m_cxHexCount) + m_cxHexHalfCount;
	}

bool CHexGridBase::IsNeighbor (int xFrom, int yFrom, int xTo, int yTo) const

//	IsNeighbor
//
//	Returns TRUE if these two cells are neighbors.

	{
	int xDiff = xTo - xFrom;
	if (xDiff > 1 || xDiff < -1)
		return false;

	int yDiff = yTo - yFrom;
	if (xDiff == 0)
		return (yDiff >= -1 && yDiff <= 1);
	else if ((xFrom % 2) == 0)
		return (yDiff == 0 || yDiff == -1);
	else
		return (yDiff == 0 || yDiff == 1);
	}

void CHexGridBase::PointToHexXY (const CVector2D &vPos, int *retx, int *rety) const

//	PointToHexXY
//
//	Convert from a point the a hex x,y coordinate

	{
	int xHex;
	int yHex;

	//	We figure out which column we're in:
	//
	//	        ____
	//	       /    \
	//	  ____/  B   \
	//	 /    \ 1,0  /
	//	/  A   \____/
	//	\ 0,0  /    \
	//	 \____/  C   \
	//	      \ 1,-1 /
	//	       \____/
	//	[][][][][][][]
	//	-2   0  +2  +4
	//	  -1  +1  +3

	int xColumn = (int)floor(vPos.X() / m_rHalfRadius);

	//	Divide by 3 to get a hex offset
	//
	//	[][][][][][][]
	//	-1   0  +1  +1
	//	   0   0  +1

	int xHexA = (xColumn >= -1 ? (xColumn + 1) / 3 : (xColumn - 1) / 3);

	//	Figure out which column we are within hex A:
	//
	//	  ____
	//	 /    \
	//	/  A   \
	//	\      /
	//	 \____/
	//	      
	//	  [][][]
	//	   0 1 2

	int xSubColumn = (xColumn + 1) - (xHexA * 3);

	//	For subcolumns 0 and 1 we are guaranteed to be in hex A:

	if (xSubColumn != 2)
		{
		xHex = xHexA;

		//	For even columns we're offset vertically because we have
		//	a center hex.

		if ((xHexA % 2) == 0)
			yHex = (int)floor((vPos.Y() + m_rHalfDY) / m_rDY);
		else
			yHex = (int)floor(vPos.Y() / m_rDY);
		}

	//	Otherwise we are in one of three possible hexes:
	//
	//	        ____
	//	       /    \
	//	  ____/  B   \   ___
	//	 /    \ 1,0  /
	//	/  A   \____/    ___1
	//	\ 0,0  /    \
	//	 \____/  C   \   ___0
	//	      \ 1,-1 /
	//	       \____/
	//	      []
	//	       2

	else
		{
		int yRow = ((xHexA % 2) == 0 ? (int)floor(vPos.Y() / m_rHalfDY) + 1 : (int)floor(vPos.Y() / m_rHalfDY));
		int yHexA = (yRow >= 0 ? yRow / 2 : (yRow - 1) / 2);

		//	Compute the x offset from the left of the column

		double xOffset = vPos.X() - (xColumn * m_rHalfRadius);

		//	Compute the y offset from the bottom of hex A

		double yOffset = vPos.Y() - ((yHexA * m_rDY) + ((xHexA % 2) == 0 ? -m_rHalfDY : 0.0));

		//	Figure out which of the three regions we're in:
		//
		//	\
		//	 \ B
		//	  \
		//	   \
		//	 A  \
		//	    /
		//	   /
		//	  / C
		//	 /
		//	/

		if (yOffset <= m_rHalfDY)
			{
			int xHexC = xHexA + 1;
			int yHexC = ((xHexA % 2) == 0 ? yHexA - 1 : yHexA);

			double yLine = xOffset * SQRT_3;
			if (yOffset > yLine)
				{
				xHex = xHexA;
				yHex = yHexA;
				}
			else
				{
				xHex = xHexC;
				yHex = yHexC;
				}

			}
		else
			{
			int xHexB = xHexA + 1;
			int yHexB = ((xHexA % 2) == 0 ? yHexA : yHexA + 1);

			double yLine = (m_rHalfRadius - xOffset) * SQRT_3;
			if (yOffset - m_rHalfDY > yLine)
				{
				xHex = xHexB;
				yHex = yHexB;
				}
			else
				{
				xHex = xHexA;
				yHex = yHexA;
				}
			}
		}

	//	Done

	*retx = xHex;
	*rety = yHex;
	}

int CHexGridBase::PointToIndex (const CVector2D &vPos) const

//	PointToIndex
//
//	Converts from a real point to a hex index. Returns -1 if the point is out of
//	range.

	{
	int xHex, yHex;
	PointToHexXY(vPos, &xHex, &yHex);
	return HexXYToIndex(xHex, yHex);
	}

void CHexGridBase::Reset (SIterator &i, EIterationTypes iType) const

//	Reset
//
//	Resets the iterator.

	{
	switch (iType)
		{
		case iterateIndex:
			i.iHexIndex = -1;
			break;
		}
	}

void CHexGridBase::SelectNext (SIterator &i, EIterationTypes iType) const

//	SelectNext
//
//	Selects the next hex.

	{
	switch (iType)
		{
		case iterateIndex:
			if (i.iHexIndex == -1)
				{
				i.iHexIndex = 0;
				IndexToHexXY(0, &i.xHex, &i.yHex);
				i.vHex = HexXYToPoint(i.xHex, i.yHex);
				}
			else
				{
				i.iHexIndex++;
				i.xHex++;
				if (i.xHex > m_cxHexHalfCount)
					{
					i.xHex = -m_cxHexHalfCount;
					i.yHex++;
					i.vHex = HexXYToPoint(i.xHex, i.yHex);
					}
				else
					i.vHex.SetX(i.vHex.X() + m_rDX);
				}
			break;
		}
	}
