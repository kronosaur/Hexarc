//	TImageBlt.h
//
//	Template for blt'ing
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

template <class PAINTER, class BLENDER> class TImageBlt
	{
	public:
		TImageBlt (void) { }
		TImageBlt (const PAINTER &Painter) :
				m_Painter(Painter)
			{ }

		void Blt (CRGBA32Image &Dest, int xDest, int yDest, const CRGBA32Image &Src, int xSrc = 0, int ySrc = 0, int cxSrc = -1, int cySrc = -1)
			{
			if (!AdjustCoords(Dest, xDest, yDest, Src, xSrc, ySrc, cxSrc, cySrc))
				return;

			CRGBA32 *pSrcRow = Src.GetPixelPos(xSrc, ySrc);
			CRGBA32 *pSrcRowEnd = Src.GetPixelPos(xSrc, ySrc + cySrc);
			CRGBA32 *pDestRow = Dest.GetPixelPos(xDest, yDest);

			while (pSrcRow < pSrcRowEnd)
				{
				CRGBA32 *pSrc = pSrcRow;
				CRGBA32 *pSrcEnd = pSrc + cxSrc;
				CRGBA32 *pDest = pDestRow;

				START_ROW(pSrc, pDest);

				while (pSrc < pSrcEnd)
					{
					CRGBA32 rgbResult = FILTER(*pSrc, pDest);
					BYTE byAlpha = rgbResult.GetAlpha();

					if (byAlpha == 0x00)
						;
					else if (byAlpha == 0xff)
						*pDest = BLENDER::Copy(*pDest, rgbResult);
					else
						*pDest = BLENDER::Blend(*pDest, rgbResult);

					pDest++;
					pSrc++;
					}

				pSrcRow = Src.NextRow(pSrcRow);
				pDestRow = Dest.NextRow(pDestRow);
				}
			}

		void BltScaled (CRGBA32Image &Dest, int xDest, int yDest, int cxDest, int cyDest, const CRGBA32Image &Src, int xSrc, int ySrc, int cxSrc, int cySrc)
			{
			if (cxSrc == -1) cxSrc = Src.GetWidth();
			if (cySrc == -1) cySrc = Src.GetHeight();

			if (cxDest <= 0 || cyDest <= 0 || cxSrc <= 0 || cySrc <= 0)
				return;

			//	Compute the increment on the source to cover the entire destination

			Metric xSrcInc = (Metric)cxSrc / (Metric)cxDest;
			Metric ySrcInc = (Metric)cySrc / (Metric)cyDest;

			//	Make sure we're in bounds

			Metric xSrcStart = (Metric)xSrc;
			Metric ySrcStart = (Metric)ySrc;
			if (!Dest.AdjustScaledCoords(&xSrcStart, &ySrcStart, Src.GetWidth(), Src.GetHeight(), 
					xSrcInc, ySrcInc,
					&xDest, &yDest,
					&cxDest, &cyDest))
				return;

			//	Do the blt

			CRGBA32 *pDestRow = Dest.GetPixelPos(xDest, yDest);
			CRGBA32 *pDestRowEnd = Dest.GetPixelPos(xDest, yDest + cyDest);

			Metric y = ySrcStart;
			while (pDestRow < pDestRowEnd)
				{
				CRGBA32 *pDestPos = pDestRow;
				CRGBA32 *pDestPosEnd = pDestPos + cxDest;

				CRGBA32 *pSrcRow = Src.GetPixelPos((int)xSrcStart, (int)y);
				Metric xOffset = 0.0;

				while (pDestPos < pDestPosEnd)
					{
					CRGBA32 rgbResult = FILTER(*(pSrcRow + (int)xOffset), pDestPos);
					BYTE byAlpha = rgbResult.GetAlpha();

					if (byAlpha == 0x00)
						;
					else if (byAlpha == 0xff)
						*pDestPos = BLENDER::Copy(*pDestPos, rgbResult);
					else
						*pDestPos = BLENDER::Blend(*pDestPos, rgbResult);

					pDestPos++;
					xOffset += xSrcInc;
					}

				y += ySrcInc;
				pDestRow = Dest.NextRow(pDestRow);
				}
			}

		void BltTransformed (CRGBA32Image &Dest, Metric rX, Metric rY, Metric rScaleX, Metric rScaleY, Metric rRotation, const CRGBA32Image &Src, Metric rSrcX, Metric rSrcY, Metric rSrcWidth, Metric rSrcHeight)
			{
			CXForm2D SrcToDest;
			CXForm2D DestToSrc;
			RECT rcDest;
			if (!CalcBltTransform(rX, rY, rScaleX, rScaleY, mathRadiansToDegrees(rRotation), rSrcX, rSrcY, rSrcWidth, rSrcHeight, &SrcToDest, &DestToSrc, &rcDest))
				return;

			//	Bounds check on the destination

			int xDest = rcDest.left;
			int yDest = rcDest.top;
			int cxDest = RectWidth(rcDest);
			int cyDest = RectHeight(rcDest);
			if (!Dest.AdjustCoords(NULL, NULL, 0, 0, &xDest, &yDest, &cxDest, &cyDest))
				return;

			int xSrcEnd = xSrc + cxSrc;
			int ySrcEnd = ySrc + cySrc;

			//	Compute vectors that move us by 1 pixel

			CVector2D vOrigin = DestToSrc.Transform(CVector2D(0.0, 0.0));
			CVector2D vIncX = DestToSrc.Transform(CVector2D(1.0, 0.0)) - vOrigin;
			CVector2D vIncY = DestToSrc.Transform(CVector2D(0.0, 1.0)) - vOrigin;

			int iRowHeight = Src.GetPixelPos(0, 1) - Src.GetPixelPos(0, 0);

			//	Loop over every pixel in the destination

			CVector2D vSrcRow = DestToSrc.Transform(CVector2D(xDest, yDest));
			CRGBA32 *pDestRow = Dest.GetPixelPos(xDest, yDest);
			CRGBA32 *pDestRowEnd = Dest.GetPixelPos(xDest, yDest + cyDest);
			while (pDestRow < pDestRowEnd)
				{
				CVector2D vSrcPos = vSrcRow;
				CRGBA32 *pDestPos = pDestRow;
				CRGBA32 *pDestPosEnd = pDestRow + cxDest;
				while (pDestPos < pDestPosEnd)
					{
					int xSrcPos = (int)vSrcPos.GetX();
					int ySrcPos = (int)vSrcPos.GetY();

					if (xSrcPos >= xSrc && xSrcPos + 1 < xSrcEnd
							&& ySrcPos >= ySrc && ySrcPos + 1< ySrcEnd)
						{
						CRGBA32 *pSrcPos = Src.GetPixelPos(xSrcPos, ySrcPos);

						CRGBA32 rgbA = FILTER(*pSrcPos, pDestPos);
						BYTE byAlpha = rgbA.GetAlpha();

						if (byAlpha == 0x00)
							NULL;
						else
							{
							CRGBA32 rgbB = FILTER(*(pSrcPos + iRowHeight), pDestPos);
							CRGBA32 rgbC = FILTER(*(pSrcPos + 1), pDestPos);
							CRGBA32 rgbD = FILTER(*(pSrcPos + iRowHeight + 1), pDestPos);

							Metric xf = vSrcPos.GetX() - (Metric)(xSrcPos);
							Metric yf = vSrcPos.GetY() - (Metric)(ySrcPos);

							Metric ka = (1.0 - xf) * (1.0 - yf);
							Metric kb = (1.0 - xf) * yf;
							Metric kc = xf * (1.0 - yf);
							Metric kd = xf * yf;

							DWORD red = (DWORD)(ka * rgbA.GetRed()
									+ kb * rgbB.GetRed()
									+ kc * rgbC.GetRed()
									+ kd * rgbD.GetRed());

							DWORD green = (DWORD)(ka * rgbA.GetGreen()
									+ kb * rgbB.GetGreen()
									+ kc * rgbC.GetGreen()
									+ kd * rgbD.GetGreen());

							DWORD blue = (DWORD)(ka * rgbA.GetBlue()
									+ kb * rgbB.GetBlue()
									+ kc * rgbC.GetBlue()
									+ kd * rgbD.GetBlue());

							*pDestPos = BLENDER::BlendPreMult(*pDestPos, CRGBA32((BYTE)red, (BYTE)green, (BYTE)blue, byAlpha));
							}
						}

					//	Next

					vSrcPos = vSrcPos + vIncX;
					pDestPos++;
					}

				//	Next row

				vSrcRow = vSrcRow + vIncY;
				pDestRow = Dest.NextRow(pDestRow);
				}
			}

		void Copy (CRGBA32Image &Dest, int xDest, int yDest, const CRGBA32Image &Src, int xSrc = 0, int ySrc = 0, int cxSrc = -1, int cySrc = -1)
			{
			if (!AdjustCoords(Dest, xDest, yDest, Src, xSrc, ySrc, cxSrc, cySrc))
				return;

			CRGBA32 *pSrcRow = Src.GetPixelPos(xSrc, ySrc);
			CRGBA32 *pSrcRowEnd = Src.GetPixelPos(xSrc, ySrc + cySrc);
			CRGBA32 *pDestRow = Dest.GetPixelPos(xDest, yDest);

			while (pSrcRow < pSrcRowEnd)
				{
				CRGBA32 *pSrc = pSrcRow;
				CRGBA32 *pSrcEnd = pSrc + cxSrc;
				CRGBA32 *pDest = pDestRow;

				START_ROW(pSrc, pDest);

				while (pSrc < pSrcEnd)
					{
					*pDest = FILTER(*pSrc, pDest);

					pDest++;
					pSrc++;
					}

				pSrcRow = Src.NextRow(pSrcRow);
				pDestRow = Dest.NextRow(pDestRow);
				}
			}

	private:

		static bool AdjustCoords (const CRGBA32Image &Dest, int &xDest, int &yDest, const CRGBA32Image &Src, int &xSrc, int &ySrc, int &cxSrc, int &cySrc)
			{
			if (cxSrc == -1) cxSrc = Src.GetWidth();
			if (cySrc == -1) cySrc = Src.GetHeight();

			if (!Dest.AdjustCoords(&xSrc, &ySrc, Src.GetWidth(), Src.GetHeight(), 
					&xDest, &yDest,
					&cxSrc, &cySrc))
				return false;

			return true;
			}

		static bool CalcBltTransform (Metric rX,
							   Metric rY,
							   Metric rScaleX,
							   Metric rScaleY,
							   Metric rRotation,
							   Metric rSrcX,
							   Metric rSrcY,
							   Metric rSrcWidth,
							   Metric rSrcHeight,
							   CXForm2D *retSrcToDest, 
							   CXForm2D *retDestToSrc, 
							   RECT *retrcDest)
			{
			//	Eliminate some simple edge conditions

			if (rSrcWidth <= 0.0 || rSrcHeight <= 0.0 || rScaleX <= 0.0 || rScaleY <= 0.0)
				return false;

			//	Compute the center of the source in source coordinates

			Metric rSrcCenterX = rSrcX + (0.5 * rSrcWidth);
			Metric rSrcCenterY = rSrcY + (0.5 * rSrcHeight);

			//	Create a transform from source coords to destination

			*retSrcToDest 
					//	First move the source origin to the center
					= CXForm2D(xformTranslate, -rSrcCenterX, -rSrcCenterY)

					//	Next, scale up the source
					* CXForm2D(xformScale, rScaleX, rScaleY)

					//	Then rotate
					* CXForm2D(xformRotate, -rRotation)

					//	Now move to the proper location
					* CXForm2D(xformTranslate, rX, rY);

			//	Now create the inverse transfor (from dest to source)

			*retDestToSrc = CXForm2D(xformTranslate, -rX, -rY)
					* CXForm2D(xformRotate, rRotation)
					* CXForm2D(xformScale, 1.0 / rScaleX, 1.0 / rScaleY)
					* CXForm2D(xformTranslate, rSrcCenterX, rSrcCenterY);

			//	Transform the four corners of the source to see where they
			//	end up on the destination

			CVector2D vSrcUL = retSrcToDest->Transform(CVector2D(rSrcX, rSrcY));
			CVector2D vSrcUR = retSrcToDest->Transform(CVector2D(rSrcX + rSrcWidth, rSrcY));
			CVector2D vSrcLL = retSrcToDest->Transform(CVector2D(rSrcX, rSrcY + rSrcHeight));
			CVector2D vSrcLR = retSrcToDest->Transform(CVector2D(rSrcX + rSrcWidth, rSrcY + rSrcHeight));

			//	Now figure out the axis-aligned box in which the source will
			//	be in destination coordinates

			Metric rLeft = Min(Min(vSrcUL.GetX(), vSrcUR.GetX()), Min(vSrcLL.GetX(), vSrcLR.GetX()));
			Metric rRight = Max(Max(vSrcUL.GetX(), vSrcUR.GetX()), Max(vSrcLL.GetX(), vSrcLR.GetX()));
			Metric rTop = Min(Min(vSrcUL.GetY(), vSrcUR.GetY()), Min(vSrcLL.GetY(), vSrcLR.GetY()));
			Metric rBottom = Max(Max(vSrcUL.GetY(), vSrcUR.GetY()), Max(vSrcLL.GetY(), vSrcLR.GetY()));

			//	Generate the rect

			retrcDest->left = (int)rLeft;
			retrcDest->right = (int)mathRound(rLeft + (rRight - rLeft));
			retrcDest->top = (int)rTop;
			retrcDest->bottom = (int)mathRound(rTop + (rBottom - rTop));

			//	Done

			return (RectWidth(*retrcDest) > 0 ) && (RectHeight(*retrcDest) > 0);
			}

		inline CRGBA32 FILTER (CRGBA32 rgbSrc, CRGBA32 *pDest) const { return m_Painter.Filter(rgbSrc, pDest); }
		inline CRGBA32 GET_PIXEL (int x, int y) const { return m_Painter.GetPixelAt(x, y); }
		inline void START_ROW (CRGBA32 *pSrc, CRGBA32 *pDest) { m_Painter.StartRow(pSrc, pDest); }

		PAINTER m_Painter;
	};

template <class BLENDER> class TSolidImageBlt : public TImageBlt<CPainterSolid, BLENDER>
	{
	public:
		TSolidImageBlt (void) { }
		TSolidImageBlt (const CPainterSolid &Painter) : TImageBlt<CPainterSolid, BLENDER>(Painter)
			{ }
	};

