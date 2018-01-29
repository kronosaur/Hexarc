//	FoundationMathHexGrid.h
//
//	Foundation header file
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CHexGridBase
	{
	public:
		CHexGridBase (double rRadius, double rWidth, double rHeight);

		inline int GetCount (void) const { return m_iCount; }
		inline double GetDX (void) const { return m_rDX; }
		inline double GetDY (void) const { return m_rDY; }
		inline double GetHeight (void) const { return m_rHeight; }
		bool GetHexesInRange (const CVector2D &vPos, double rRange, TArray<int> *retHexes) const;
		void GetOutlines (const TArray<int> &HexList, TArray<CPolygon2D> *retResult);
		inline double GetRadius (void) const { return m_rRadius; }
		inline double GetWidth (void) const { return m_rWidth; }
		inline bool HexXYInRange (int x, int y) const { return (x <= m_cxHexHalfCount && x >= -m_cxHexHalfCount	&& y <= m_cyHexHalfCount && y >= -m_cyHexHalfCount); }
		CVector2D HexXYToPoint (int x, int y) const;
		int HexXYToIndex (int x, int y) const;
		void IndexToHexXY (int iIndex, int *retx, int *rety) const;
		CVector2D IndexToPoint (int iIndex) const;
		void PointToHexXY (const CVector2D &vPos, int *retx, int *rety) const;
		int PointToIndex (const CVector2D &vPos) const;

	private:
		void Init (double rRadius, double rWidth, double rHeight);

		double m_rRadius;					//	Radius of each hex
		double m_rHalfRadius;

		double m_rDX;						//	Distance between hex centers (horizontally)
		double m_rHalfDX;
		double m_rDY;						//	Distance between hex centers (vertically)
		double m_rHalfDY;

		int m_iCount;						//	Total number of hexes
		int m_cxHexCount;					//	Number of hexes across (always odd because always a center hex)
		int m_cyHexCount;					//	Number of hexes up and down	(always odd)
		int m_iCenterHex;					//	Index of center hex
		int m_cxHexHalfCount;
		int m_cyHexHalfCount;

		double m_rWidth;					//	Total width of grid (centered on 0.0)
		double m_rHeight;					//	Total height of grid (centered on 0.0)
	};

template <class VALUE> class THexGrid : public CHexGridBase
	{
	public:
		THexGrid (double rRadius, double rWidth, double rHeight) : CHexGridBase(rRadius, rWidth, rHeight),
				m_pHexes(NULL)
			{
			AllocArray();
			}

		~THexGrid (void)
			{
			CleanUp();
			}

		void FillAll (const VALUE &Value)
			{
			int i;

			for (i = 0; i < GetCount(); i++)
				m_pHexes[i] = Value;
			}

		VALUE &GetAt (int iIndex) { return m_pHexes[iIndex]; }

		VALUE *PointToHex (const CVector2D &vPos)
			{
			int iIndex = PointToIndex(vPos);
			if (iIndex == -1)
				return NULL;

			return &m_pHexes[iIndex];
			}

	private:
		void AllocArray (void)
			{
			ASSERT(m_pHexes == NULL);
			m_pHexes = new VALUE [GetCount()];
			}

		void CleanUp (void)
			{
			if (m_pHexes)
				{
				delete [] m_pHexes;
				m_pHexes = NULL;
				}
			}

		VALUE *m_pHexes;
	};

class CHexGridNavigator
	{
	public:
		CHexGridNavigator (const CHexGridBase &Grid);

		void GetOutlines (TArray<CPolygon2D> *retResult);
		void SelectHexes (const TArray<int> &HexList);

	private:
		enum EFlags
			{
			flagOdd =			0x00000001,	//	Odd hex
			flagMarked =		0x00000002,	//	Marked hex

			flagEdge0Marked =	0x00000004,
			flagEdge1Marked =	0x00000008,
			flagEdge2Marked =	0x00000010,
			flagEdge3Marked =	0x00000020,
			flagEdge4Marked =	0x00000040,
			flagEdge5Marked =	0x00000080,

			flagAllEdgeMarks =	0x000000FC,
			};

		struct SHexInfo
			{
			int iIndex;
			int xHex;
			int yHex;

			DWORD dwFlags;
			int iNeighborCount;				//	Number of neighbors
			SHexInfo *pNeighbor[6];			//	Valid neighbors
			};

		inline bool AreAllEdgesMarked (SHexInfo &Hex) const { return (Hex.dwFlags & flagAllEdgeMarks) == flagAllEdgeMarks; }
		bool FindNextOutlinePoint (SHexInfo *&pHex, int &iEdge, SHexInfo *pStopHex, int iStopEdge, const TArray<CVector2D> &HexPoints, CVector2D *retvNextPoint);
		int FindStartEdge (SHexInfo &Hex);
		void InitEdgesToProcess (void);
		inline bool IsEdgeMarked (SHexInfo *pHex, int iEdge) { return (pHex->dwFlags & (flagEdge0Marked << iEdge)) != 0; }
		void MarkEdge (SHexInfo *pHex, int iEdge);

		const CHexGridBase &m_Grid;

		TArray<SHexInfo> m_Selection;
		CLargeSet m_SelectionSet;
	};
