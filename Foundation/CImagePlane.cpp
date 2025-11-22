//	CImagePlane.cpp
//
//	CImagePlane class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CImagePlane::CImagePlane (int cxWidth, int cyHeight) :
		m_cxWidth(cxWidth),
		m_cyHeight(cyHeight)

//	CGImagePlane constructor

	{
	ASSERT(m_cxWidth >= 0);
	ASSERT(m_cyHeight >= 0);

	m_rcClip.left = 0;
	m_rcClip.top = 0;
	m_rcClip.right = 0;
	m_rcClip.bottom = 0;
	}

CImagePlane::~CImagePlane (void)

//	CImagePlane destructor

	{
	}

bool CImagePlane::AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc,
								 int *xDest, int *yDest,
								 int *cxWidth, int *cyHeight) const

//	AdjustCoords
//
//	Make sure that the coordinates are in range and adjust
//	them if they are not.

	{
	if (xSrc && *xSrc < 0)
		{
		*cxWidth += *xSrc;
		*xDest -= *xSrc;
		*xSrc = 0;
		}

	if (ySrc && *ySrc < 0)
		{
		*cyHeight += *ySrc;
		*yDest -= *ySrc;
		*ySrc = 0;
		}

	if (*xDest < m_rcClip.left)
		{
		*cxWidth += (*xDest - m_rcClip.left);
		if (xSrc) *xSrc -= (*xDest - m_rcClip.left);
		*xDest = m_rcClip.left;
		}

	if (*yDest < m_rcClip.top)
		{
		*cyHeight += (*yDest - m_rcClip.top);
		if (ySrc) *ySrc -= (*yDest - m_rcClip.top);
		*yDest = m_rcClip.top;
		}

	*cxWidth = Min(*cxWidth, (int)m_rcClip.right - *xDest);
	if (xSrc)
		*cxWidth = Min(*cxWidth, cxSrc - *xSrc);

	*cyHeight = Min(*cyHeight, (int)m_rcClip.bottom - *yDest);
	if (ySrc)
		*cyHeight = Min(*cyHeight, cySrc - *ySrc);

	return (*cxWidth > 0 && *cyHeight > 0);
	}

bool CImagePlane::AdjustScaledCoords (Metric *xSrc, Metric *ySrc, int cxSrc, int cySrc,
									   Metric xSrcInc, Metric ySrcInc,
									   int *xDest, int *yDest,
									   int *cxDest, int *cyDest)

//	AdjustCoords
//
//	Make sure that the coordinates are in range and adjust
//	them if they are not.

	{
	if (xSrc && *xSrc < 0.0)
		{
		*cxDest += (int)(*xSrc / xSrcInc);
		*xDest -= (int)(*xSrc / xSrcInc);
		*xSrc = 0.0;
		}

	if (ySrc && *ySrc < 0)
		{
		*cyDest += (int)(*ySrc / ySrcInc);
		*yDest -= (int)(*ySrc / ySrcInc);
		*ySrc = 0.0;
		}

	if (*xDest < m_rcClip.left)
		{
		*cxDest += (*xDest - m_rcClip.left);
		if (xSrc) *xSrc -= (*xDest - m_rcClip.left);
		*xDest = m_rcClip.left;
		}

	if (*yDest < m_rcClip.top)
		{
		*cyDest += (*yDest - m_rcClip.top);
		if (ySrc) *ySrc -= (*yDest - m_rcClip.top);
		*yDest = m_rcClip.top;
		}

	*cxDest = Min(*cxDest, (int)(m_rcClip.right - *xDest));
	if (xSrc)
		*cxDest = Min(*cxDest, (int)((cxSrc - *xSrc) / xSrcInc));

	*cyDest = Min(*cyDest, (int)(m_rcClip.bottom - *yDest));
	if (ySrc)
		*cyDest = Min(*cyDest, (int)((cySrc - *ySrc) / ySrcInc));

	return (*cxDest > 0 && *cyDest > 0);
	}

void CImagePlane::ResetClipRect (void)

//	ResetClipRect
//
//	Clears the clip rect

	{
	m_rcClip.left = 0;
	m_rcClip.top = 0;
	m_rcClip.right = m_cxWidth;
	m_rcClip.bottom = m_cyHeight;
	}

void CImagePlane::SetClipRect (const RECT &rcClip)

//	SetClipRect
//
//	Sets the clip rect

	{
	m_rcClip.left = Min(Max(0, (int)rcClip.left), m_cxWidth);
	m_rcClip.top = Min(Max(0, (int)rcClip.top), m_cyHeight);
	m_rcClip.right = Min((int)Max(m_rcClip.left, rcClip.right), m_cxWidth);
	m_rcClip.bottom = Min((int)Max(m_rcClip.top, rcClip.bottom), m_cyHeight);
	}
