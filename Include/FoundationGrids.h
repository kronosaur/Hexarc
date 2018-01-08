//	FoundationGrids.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

template <class VALUE> class TDynamicGrid
	{
	public:
		TDynamicGrid (void) : 
				m_cxWidth(0),
				m_cyHeight(0),
				m_xOrigin(0),
				m_yOrigin(0),
				m_pArray(NULL),
				m_bInitWithNullValue(false)
			{
			}

		~TDynamicGrid (void)
			{
			if (m_pArray)
				delete [] m_pArray;
			}

		void DeleteAll (void)
			{
			if (m_pArray)
				{
				delete [] m_pArray;
				m_pArray = NULL;
				}

			m_cxWidth = 0;
			m_cyHeight = 0;
			}

		const VALUE &GetAt (int x, int y) const
			{
			VALUE *pLoc = GetLocation(x, y);
			return (pLoc ? *pLoc : m_NullValue);
			}

		int GetCount (void) const
			{
			return m_cxWidth * m_cyHeight;
			}

		const VALUE &Get (int iIndex) const
			{
			return m_pArray[iIndex];
			}

		void SetAt (int x, int y, const VALUE &Value)
			{
			//	Convert to array coordinates

			int xArray = x + m_xOrigin;
			int yArray = y + m_yOrigin;

			//	If we're in bounds then we just set the value

			if (xArray >= 0 && xArray < m_cxWidth && yArray >= 0 && yArray < m_cyHeight)
				{
				m_pArray[yArray * m_cxWidth + xArray] = Value;
				return;
				}

			//	Otherwise we need to reallocate. Compute the new values.

			int xNewOrigin;
			int yNewOrigin;
			int cxNewWidth;
			int cyNewHeight;

			CalcAllocation(xArray, yArray, &cxNewWidth, &cyNewHeight, &xNewOrigin, &yNewOrigin);

			//	Allocate a new array

			Allocate(cxNewWidth, cyNewHeight, xNewOrigin, yNewOrigin);

			//	Try again

			xArray = x + m_xOrigin;
			yArray = y + m_yOrigin;

			m_pArray[yArray * m_cxWidth + xArray] = Value;
			}

		void SetNullValue (const VALUE &Null)
			{
			m_NullValue = Null;
			m_bInitWithNullValue = true;
			}

	private:
		void Allocate (int cxNewWidth, int cyNewHeight, int xNewOrigin, int yNewOrigin)
			{
			//	Allocate a new array

			VALUE *pNewArray = new VALUE [cxNewWidth * cyNewHeight];

			//	Copy over the old array

			if (m_pArray)
				{
				VALUE *pSrcLine = m_pArray;
				VALUE *pSrcLineEnd = pSrcLine + (m_cyHeight * m_cxWidth);

				int yDest = yNewOrigin - m_yOrigin;
				VALUE *pDestLine = pNewArray + (yDest * cxNewWidth);

				while (pSrcLine < pSrcLineEnd)
					{
					VALUE *pSrc = pSrcLine;
					VALUE *pSrcEnd = pSrcLine + m_cxWidth;

					int xDest = xNewOrigin - m_xOrigin;
					VALUE *pDest = pDestLine + xDest;

					while (pSrc < pSrcEnd)
						*pDest++ = *pSrc++;

					pSrcLine += m_cxWidth;
					pDestLine += cxNewWidth;
					}

				//	Initialize new regions

				if (m_bInitWithNullValue)
					{
					//	Top

					int yDest = yNewOrigin - m_yOrigin;
					if (yDest > 0)
						{
						VALUE *pDest = pNewArray;
						VALUE *pDestEnd = pDest + (yDest * cxNewWidth);

						while (pDest < pDestEnd)
							*pDest++ = m_NullValue;
						}

					//	Bottom

					int cyDest = cyNewHeight - (yDest + m_cyHeight);
					if (cyDest > 0)
						{
						VALUE *pDest = pNewArray + ((yDest + m_cyHeight) * cxNewWidth);
						VALUE *pDestEnd = pNewArray + (cyNewHeight * cxNewWidth);

						while (pDest < pDestEnd)
							*pDest++ = m_NullValue;
						}

					//	Left & right

					int xDest = xNewOrigin - m_xOrigin;
					int cxDest = cxNewWidth - (xDest + m_cxWidth);
					if (xDest > 0 || cxDest > 0)
						{
						VALUE *pDestLine = pNewArray + (yDest * cxNewWidth);
						VALUE *pDestLineEnd = pNewArray + ((yDest + m_cyHeight) * cxNewWidth);

						while (pDestLine < pDestLineEnd)
							{
							if (xDest > 0)
								{
								VALUE *pDest = pDestLine;
								VALUE *pDestEnd = pDest + xDest;

								while (pDest < pDestEnd)
									*pDest++ = m_NullValue;
								}

							if (cxDest > 0)
								{
								VALUE *pDest = pDestLine + xDest + m_cxWidth;
								VALUE *pDestEnd = pDest + cxDest;

								while (pDest < pDestEnd)
									*pDest++ = m_NullValue;
								}

							pDestLine += cxNewWidth;
							}
						}
					}
				}

			//	If we don't have an original array then we can easily initialize
			//	the new array.

			else if (m_bInitWithNullValue)
				{
				VALUE *pDest = pNewArray;
				VALUE *pDestEnd = pNewArray + (cyNewHeight * cxNewWidth);

				while (pDest < pDestEnd)
					*pDest++ = m_NullValue;
				}

			//	Switch

			if (m_pArray)
				delete [] m_pArray;

			m_pArray = pNewArray;
			m_cxWidth = cxNewWidth;
			m_cyHeight = cyNewHeight;
			m_xOrigin = xNewOrigin;
			m_yOrigin = yNewOrigin;
			}

		void CalcAllocation (int xArray, int yArray, int *retcxWidth, int *retcyHeight, int *retxOrigin, int *retyOrigin) const
			{
			if (m_pArray == NULL)
				{
				*retxOrigin = -xArray;
				*retcxWidth = 1;
				}
			else if (xArray < 0)
				{
				*retxOrigin = m_xOrigin - xArray;
				*retcxWidth = m_cxWidth - xArray;
				}
			else if (xArray >= m_cxWidth)
				{
				*retxOrigin = m_xOrigin;
				*retcxWidth = xArray + 1;
				}
			else
				{
				*retxOrigin = m_xOrigin;
				*retcxWidth = m_cxWidth;
				}

			if (m_pArray == NULL)
				{
				*retyOrigin = -yArray;
				*retcyHeight = 1;
				}
			else if (yArray < 0)
				{
				*retyOrigin = m_yOrigin - yArray;
				*retcyHeight = m_cyHeight - yArray;
				}
			else if (yArray >= m_cyHeight)
				{
				*retyOrigin = m_yOrigin;
				*retcyHeight = yArray + 1;
				}
			else
				{
				*retyOrigin = m_yOrigin;
				*retcyHeight = m_cyHeight;
				}
			}

		VALUE *GetLocation (int x, int y) const
			{
			//	Convert to array coordinates

			int xArray = x + m_xOrigin;
			int yArray = y + m_yOrigin;

			//	Bounds check

			if (xArray < 0 || xArray >= m_cxWidth || yArray < 0 || yArray >= m_cyHeight)
				return NULL;

			return &m_pArray[yArray * m_cxWidth + xArray];
			}

		int m_cxWidth;						//	Width of allocated array
		int m_cyHeight;						//	Height of allocated array
		int m_xOrigin;						//	Origin X
		int m_yOrigin;						//	Origin y

		VALUE *m_pArray;
		VALUE m_NullValue;					//	Returned when out of range
		bool m_bInitWithNullValue;			//	If TRUE, we use null value to initialize
	};

template <class VALUE> class TSparseGrid
	{
	public:
		TSparseGrid (int iBlockSize = 20) : 
				m_iBlockSize(iBlockSize),
				m_iHalfBlock(iBlockSize / 2),
				m_iMinusHalfBlock(-iBlockSize / 2),
				m_iBlockSizeMinus1(iBlockSize - 1),
				m_iHalfBlockMinus1((iBlockSize / 2) - 1),
				m_iHalfBlockPlus1((iBlockSize / 2) + 1),
				m_bInitWithNullValue(false)
			{
			ASSERT(iBlockSize > 0);
			ASSERT((iBlockSize % 2) == 0);

			m_BlockGrid.SetNullValue(NULL);
			}

		~TSparseGrid (void)
			{
			DeleteAll();
			}

		void DeleteAll (void)
			{
			int i;

			for (i = 0; i < m_BlockGrid.GetCount(); i++)
				{
				VALUE *pBlock = m_BlockGrid.Get(i);
				if (pBlock)
					delete [] pBlock;
				}

			m_BlockGrid.DeleteAll();
			}

		const VALUE &GetAt (int x, int y) const
			{
			//	Split up by quadrant so we minimize computations

			if (x >= m_iMinusHalfBlock)
				{
				int xNormal = x + m_iHalfBlock;
				int xBlock = xNormal / m_iBlockSize;

				if (y >= m_iMinusHalfBlock)
					{
					int yNormal = y + m_iHalfBlock;
					int yBlock = yNormal / m_iBlockSize;

					VALUE *pBlock = m_BlockGrid.GetAt(xBlock, yBlock);
					if (pBlock == NULL)
						return m_NullValue;

					int xRel = xNormal % m_iBlockSize;
					int yRel = yNormal % m_iBlockSize;
					return pBlock[yRel * m_iBlockSize + xRel];
					}
				else
					{
					int yBlock = (y - m_iHalfBlockMinus1) / m_iBlockSize;

					VALUE *pBlock = m_BlockGrid.GetAt(xBlock, yBlock);
					if (pBlock == NULL)
						return m_NullValue;

					int xRel = xNormal % m_iBlockSize;
					int yRel = m_iBlockSizeMinus1 + ((y + m_iHalfBlockPlus1) % m_iBlockSize);
					return pBlock[yRel * m_iBlockSize + xRel];
					}
				}
			else
				{
				int xBlock = (x - m_iHalfBlockMinus1) / m_iBlockSize;

				if (y >= m_iMinusHalfBlock)
					{
					int yNormal = y + m_iHalfBlock;
					int yBlock = yNormal / m_iBlockSize;

					VALUE *pBlock = m_BlockGrid.GetAt(xBlock, yBlock);
					if (pBlock == NULL)
						return m_NullValue;

					int xRel = m_iBlockSizeMinus1 + ((x + m_iHalfBlockPlus1) % m_iBlockSize);
					int yRel = yNormal % m_iBlockSize;
					return pBlock[yRel * m_iBlockSize + xRel];
					}
				else
					{
					int yBlock = (y - m_iHalfBlockMinus1) / m_iBlockSize;

					VALUE *pBlock = m_BlockGrid.GetAt(xBlock, yBlock);
					if (pBlock == NULL)
						return m_NullValue;

					int xRel = m_iBlockSizeMinus1 + ((x + m_iHalfBlockPlus1) % m_iBlockSize);
					int yRel = m_iBlockSizeMinus1 + ((y + m_iHalfBlockPlus1) % m_iBlockSize);
					return pBlock[yRel * m_iBlockSize + xRel];
					}
				}
			}

		void SetAt (int x, int y, const VALUE &Value)
			{
			int xNormal = x + m_iHalfBlock;
			int xBlock;
			int xRel;
			if (xNormal >= 0)
				{
				xBlock = xNormal / m_iBlockSize;
				xRel = xNormal % m_iBlockSize;
				}
			else
				{
				xBlock = (x - m_iHalfBlockMinus1) / m_iBlockSize;
				xRel = m_iBlockSizeMinus1 + ((x + m_iHalfBlockPlus1) % m_iBlockSize);
				}

			int yNormal = y + m_iHalfBlock;
			int yBlock;
			int yRel;
			if (yNormal >= 0)
				{
				yBlock = yNormal / m_iBlockSize;
				yRel = yNormal % m_iBlockSize;
				}
			else
				{
				yBlock = (y - m_iHalfBlockMinus1) / m_iBlockSize;
				yRel = m_iBlockSizeMinus1 + ((y + m_iHalfBlockPlus1) % m_iBlockSize);
				}

			//	Get the block, allocating if necessary

			VALUE *pBlock = m_BlockGrid.GetAt(xBlock, yBlock);
			if (pBlock == NULL)
				pBlock = AllocBlock(xBlock, yBlock);

			//	Set the cell

			pBlock[yRel * m_iBlockSize + xRel] = Value;
			}

		void SetNullValue (const VALUE &Value)
			{
			m_NullValue = Value;
			m_bInitWithNullValue = true;
			}

	private:
		VALUE *AllocBlock (int xBlock, int yBlock)
			{
			int iSize = m_iBlockSize * m_iBlockSize;
			VALUE *pBlock = new VALUE [iSize];

			//	Initialize, if necessary

			if (m_bInitWithNullValue)
				{
				VALUE *pPos = pBlock;
				VALUE *pPosEnd = pPos + iSize;

				while (pPos < pPosEnd)
					*pPos++ = m_NullValue;
				}

			m_BlockGrid.SetAt(xBlock, yBlock, pBlock);

			return pBlock;
			}

		void GetBlockCoord (int x, int y, int *retxBlock, int *retyBlock, int *retxRel, int *retyRel)
			{
			int xOffset = (x + m_iHalfBlock);
			if (xOffset >= 0)
				{
				*retxBlock = xOffset / m_iBlockSize;
				*retxRel = xOffset % m_iBlockSize;
				}
			else
				{
				*retxBlock = (xOffset - (m_iBlockSize - 1)) / m_iBlockSize;
				*retxRel = (m_iBlockSize - 1) + ((xOffset + 1) % m_iBlockSize);
				}


			if (x >= 0)
				{
				*retxBlock = (x + m_iHalfBlock) / m_iBlockSize;
				*retxRel = (x + m_iHalfBlock) % m_iBlockSize;
				}
			else
				{
				*retxBlock = (x - m_iHalfBlock + 1) / m_iBlockSize;
				*retxRel = m_iBlockSize - ((x + m_iHalfBlock) % m_iBlockSize);
				}

			int xOffset = (x >= 0 ? x + m_iHalfBlock : x - m_iHalfBlock + 1);
			*retxBlock = xOffset / m_iBlockSize;

			int yOffset = (y >= 0 ? y + m_iHalfBlock : y - m_iHalfBlock + 1);
			*retyBlock = yOffset / m_iBlockSize;

			*retxRel = xOffset % m_iBlockSize;
			}

		int m_iBlockSize;
		int m_iHalfBlock;
		int m_iMinusHalfBlock;
		int m_iBlockSizeMinus1;
		int m_iHalfBlockMinus1;
		int m_iHalfBlockPlus1;

		TDynamicGrid<VALUE *> m_BlockGrid;
		VALUE m_NullValue;
		bool m_bInitWithNullValue;
	};
