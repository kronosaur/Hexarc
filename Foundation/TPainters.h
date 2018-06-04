//	TPainters.h
//
//	Templates for various pixel painters and filters
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CPainterSolid
	{
	public:
		CPainterSolid (void) { }
		CPainterSolid (CRGBA32 rgbColor) : m_rgbColor(rgbColor)
			{ }

		inline CRGBA32 Filter (CRGBA32 rgbSrc, CRGBA32 *pDest) const { return rgbSrc; }
		inline CRGBA32 GetPixelAt (int x, int y) const { return m_rgbColor; }
		inline void StartRow (CRGBA32 *pSrc, CRGBA32 *pDest) { }

	private:
		CRGBA32 m_rgbColor = CRGBA32(255, 255, 255);
	};
