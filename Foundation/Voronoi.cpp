//	Voronoi.cpp
//
//	Voronoi Functions
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "VoronoiGenerator.h"

void mathVoronoiEx (const TArray<CVector2D> &Points, TArray<SVoronoiEdge> *retSegments, double rWidth, double rHeight)

//	mathVoronoiEx
//
//	Returns all the Voronoi segments for the given points.

	{
	int i;

	//	Short-circuit

	retSegments->DeleteAll();
	if (Points.GetCount() <= 0)
		return;

	//	Keep track of bounds

	double rMinX = -0.5 * rWidth;
	double rMaxX = 0.5 * rWidth;
	double rMinY = -0.5 * rHeight;
	double rMaxY = 0.5 * rHeight;

	//	The generator expects a sharded array of points.

	VFLOAT *pInputX = new VFLOAT [Points.GetCount()];
	VFLOAT *pInputY = new VFLOAT [Points.GetCount()];
	for (i = 0; i < Points.GetCount(); i++)
		{
		pInputX[i] = Points[i].X();
		pInputY[i] = Points[i].Y();

		if (Points[i].X() < rMinX) rMinX = Points[i].X();
		if (Points[i].X() > rMaxX) rMaxX = Points[i].X();
		if (Points[i].Y() < rMinY) rMinY = Points[i].Y();
		if (Points[i].Y() > rMaxY) rMaxY = Points[i].Y();
		}

	VoronoiDiagramGenerator Generator;
	Generator.generateVoronoi(pInputX, pInputY, Points.GetCount(), rMinX, rMaxX, rMinY, rMaxY, 0.0, false);

	//	Get the output

	int iEdgeCount = Generator.getEdgeCount();
	retSegments->InsertEmpty(iEdgeCount);
	
	Generator.resetIterator();

	VFLOAT x1, y1, x2, y2;
	int iSite1, iSite2;
	i = 0;
	while (Generator.getNext(x1, y1, x2, y2, &iSite1, &iSite2))
		{
		ASSERT(i < retSegments->GetCount());

		SVoronoiEdge &NewEdge = retSegments->GetAt(i);
		NewEdge.Edge.SetFrom(CVector2D(x1, y1));
		NewEdge.Edge.SetTo(CVector2D(x2, y2));
		NewEdge.iSite1 = iSite1;
		NewEdge.iSite2 = iSite2;

		i++;
		}

	//	Done

	delete [] pInputX;
	delete [] pInputY;
	}

void mathVoronoi (const TArray<CVector2D> &Points, TArray<CLine2D> *retSegments, double rWidth, double rHeight)

//	mathVoronoi
//
//	Returns all the Voronoi segments for the given points.

	{
	int i;

	//	Short-circuit

	retSegments->DeleteAll();
	if (Points.GetCount() <= 0)
		return;

	//	Keep track of bounds

	double rMinX = -0.5 * rWidth;
	double rMaxX = 0.5 * rWidth;
	double rMinY = -0.5 * rHeight;
	double rMaxY = 0.5 * rHeight;

	//	The generator expects a sharded array of points.

	VFLOAT *pInputX = new VFLOAT [Points.GetCount()];
	VFLOAT *pInputY = new VFLOAT [Points.GetCount()];
	for (i = 0; i < Points.GetCount(); i++)
		{
		pInputX[i] = Points[i].X();
		pInputY[i] = Points[i].Y();

		if (Points[i].X() < rMinX) rMinX = Points[i].X();
		if (Points[i].X() > rMaxX) rMaxX = Points[i].X();
		if (Points[i].Y() < rMinY) rMinY = Points[i].Y();
		if (Points[i].Y() > rMaxY) rMaxY = Points[i].Y();
		}

	VoronoiDiagramGenerator Generator;
	Generator.generateVoronoi(pInputX, pInputY, Points.GetCount(), rMinX, rMaxX, rMinY, rMaxY, 0.0, false);

	//	Get the output

	int iEdgeCount = Generator.getEdgeCount();
	retSegments->InsertEmpty(iEdgeCount);
	
	Generator.resetIterator();

	VFLOAT x1, y1, x2, y2;
	i = 0;
	while (Generator.getNext(x1, y1, x2, y2))
		{
		ASSERT(i < retSegments->GetCount());

		retSegments->GetAt(i) = CLine2D(CVector2D(x1, y1), CVector2D(x2, y2));
		i++;
		}

	//	Done

	delete [] pInputX;
	delete [] pInputY;
	}
