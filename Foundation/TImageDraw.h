//	TImageDraw.h
//
//	Template for drawing various shapes.
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

template <class PAINTER, class BLENDER> class TImageDraw
	{
	public:
		TImageDraw (void) { }
		TImageDraw (const PAINTER &Painter) :
				m_Painter(Painter)
			{ }

		void Rectangle (CRGBA32Image &Dest, int xDest, int yDest, int cxWidth, int cyHeight)
			{
			if (!AdjustCoords(Dest, xDest, yDest, cxWidth, cyHeight))
				return;

			CRGBA32 *pDestRow = Dest.GetPixelPos(xDest, yDest);
			CRGBA32 *pDestRowEnd = Dest.GetPixelPos(xDest, yDest + cyHeight);
			int ySrc = 0;

			while (pDestRow < pDestRowEnd)
				{
				CRGBA32 *pDest = pDestRow;
				CRGBA32 *pDestEnd = pDestRow + cxWidth;
				int xSrc = 0;

				while (pDest < pDestEnd)
					{
					CRGBA32 rgbResult = GET_PIXEL(xSrc, ySrc);
					BYTE byAlpha = rgbResult.GetAlpha();

					if (byAlpha == 0x00)
						;
					else if (byAlpha == 0xff)
						*pDest = BLENDER::Copy(*pDest, rgbResult);
					else
						*pDest = BLENDER::Blend(*pDest, rgbResult);

					pDest++;
					xSrc++;
					}

				pDestRow = Dest.NextRow(pDestRow);
				ySrc++;
				}
			}

	private:

		static bool AdjustCoords (const CRGBA32Image &Dest, int &xDest, int &yDest, int &cxWidth, int &cyHeight)
			{
			return Dest.AdjustCoords(NULL, NULL, 0, 0, 
					&xDest, &yDest,
					&cxWidth, &cyHeight);
			}

		inline CRGBA32 FILTER (CRGBA32 rgbSrc, CRGBA32 *pDest) const { return m_Painter.Filter(rgbSrc, pDest); }
		inline CRGBA32 GET_PIXEL (int x, int y) const { return m_Painter.GetPixelAt(x, y); }
		inline void START_ROW (CRGBA32 *pSrc, CRGBA32 *pDest) { m_Painter.StartRow(pSrc, pDest); }

		PAINTER m_Painter;
	};

template <class BLENDER> class TSolidImageDraw : public TImageDraw<CPainterSolid, BLENDER>
	{
	public:
		TSolidImageDraw (void) { }
		TSolidImageDraw (const CPainterSolid &Painter) : TImageDraw<CPainterSolid, BLENDER>(Painter)
			{ }
	};

