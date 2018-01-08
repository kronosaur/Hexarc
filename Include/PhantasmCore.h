//	PhantasmCore.h
//
//	Phantasm header file
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

typedef double Metric;

class CImagePlane
	{
	public:
		CImagePlane (int cxWidth = 0, int cyHeight = 0);
		virtual ~CImagePlane (void);

		bool AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc,
						   int *xDest, int *yDest,
						   int *cxWidth, int *cyHeight) const;
		bool AdjustScaledCoords (Metric *xSrc, Metric *ySrc, int cxSrc, int cySrc,
								 Metric xSrcInc, Metric ySrcInc,
								 int *xDest, int *yDest,
								 int *cxDest, int *cyDest);

		inline const RECT &GetClipRect (void) const { return m_rcClip; }
		inline int GetHeight (void) const { return m_cyHeight; }
		inline int GetWidth (void) const { return m_cxWidth; }
		void ResetClipRect (void);
		void SetClipRect (const RECT &rcClip);

	protected:
		int m_cxWidth;
		int m_cyHeight;

		RECT m_rcClip;
	};
