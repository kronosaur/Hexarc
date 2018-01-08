//	FoundationMathGeometry.h
//
//	Foundation header file
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

const double PI = 3.14159265358979;
const double HALF_PI = 0.5 * PI;
const double TAU = 2.0 * PI;

const double SQRT_3 = sqrt(3.0);

const double DBL_INFINITY = 1.7976931348623158e+308;	//	DBL_MAX

enum ERotationDirections
	{
	dirNone,

	dirClockwise,
	dirCounterClockwise,
	};

//	2D Objects -----------------------------------------------------------------

template <class VALUE> class TVector2D
	{
	public:
		inline TVector2D (void);
		TVector2D (VALUE x, VALUE y) : m_x(x), m_y(y) { }

		inline bool operator == (const TVector2D<VALUE> &vA) const { return (m_x == vA.m_x && m_y == vA.m_y); }
		inline TVector2D operator - (void) const { return TVector2D(-m_x, -m_y); }
		inline TVector2D operator + (const TVector2D<VALUE> &vA) const { return TVector2D(m_x + vA.m_x, m_y + vA.m_y); }
		inline TVector2D operator - (const TVector2D<VALUE> &vA) const { return TVector2D(m_x - vA.m_x, m_y - vA.m_y); }
		inline TVector2D operator * (VALUE a) const { return TVector2D(m_x * a, m_y * a); }
		inline TVector2D operator / (VALUE a) const { return TVector2D(m_x / a, m_y / a); }

		inline VALUE Distance2 (const TVector2D<VALUE> &vA) const { double rX = (vA.m_x - m_x); double rY = (vA.m_y - m_y); return (rX * rX + rY * rY); }
		inline VALUE Dot (const TVector2D<VALUE> &vA) const { return m_x * vA.m_x + m_y * vA.m_y; }
		inline bool IsEqualTo (const TVector2D<VALUE> &vA, double rEpsilon2) const { return (Distance2(vA) < rEpsilon2); }
		inline VALUE Length (void) const { return sqrt(m_x * m_x + m_y * m_y); }
		inline VALUE Length2 (void) const { return (m_x * m_x + m_y * m_y); }
		inline TVector2D Orthogonal (ERotationDirections iDir = dirCounterClockwise) const { if (iDir == dirClockwise) return TVector2D(m_y, -m_x); else return TVector2D(-m_y, m_x); }
		inline TVector2D Perp (void) const { return TVector2D(-m_y, m_x); }
		VALUE Polar (VALUE *retrRadius = NULL) const;
		void Randomize (VALUE xMin, VALUE xMax, VALUE yMin, VALUE yMax);
		inline TVector2D Rotation (double rAngle) const
			{
			double rSin = sin(rAngle);
			double rCos = cos(rAngle);
			return TVector2D((m_x * rCos) - (m_y * rSin), (m_x * rSin) + (m_y * rCos));
			}
		inline void SetX (VALUE x) { m_x = x; }
		inline void SetY (VALUE y) { m_y = y; }
		inline TVector2D ToPolar (void) { return ToPolar(*this); }
		inline TVector2D ToXY (void) { return FromPolar(*this); }
		inline TVector2D Unit (void) const { VALUE rLength = Length(); if (rLength == 0) return TVector2D(); else return TVector2D(m_x / rLength, m_y / rLength); }
		inline VALUE X (void) const { return m_x; }
		inline VALUE Y (void) const { return m_y; }

		static TVector2D FromPolar (const TVector2D<VALUE> &vA) { return TVector2D(vA.Y() * cos(vA.X()), vA.Y() * sin(vA.X())); }
		static TVector2D FromPolar (VALUE Angle, VALUE Radius) { return TVector2D(Radius * cos(Angle), Radius * sin(Angle)); }
		static TVector2D ToPolar (const TVector2D<VALUE> &vA) { double rRadius; double rAngle = vA.Polar(&rRadius); return TVector2D(rAngle, rRadius); }

	private:
		VALUE m_x;
		VALUE m_y;
	};

typedef TVector2D<double> CVector2D;

template <> inline TVector2D<int>::TVector2D (void) : m_x(0), m_y(0) { }
template <> inline TVector2D<double>::TVector2D (void) : m_x(0.0), m_y(0.0) { }

template <class VALUE> const TVector2D<VALUE> operator * (const VALUE &a, const TVector2D<VALUE> &vB) { return TVector2D<VALUE>(a * vB.X(), a * vB.Y()); }

//	Transform class

enum XFormType
	{
	xformIdentity,

	xformTranslate,
	xformScale,
	xformRotate,
	};

class CXForm2D
	{
	public:
		CXForm2D (void);
		CXForm2D (XFormType type);
		CXForm2D (XFormType type, double rX, double rY);
		CXForm2D (XFormType type, const CVector2D &vVector);
		CXForm2D (XFormType type, double rAngle);

		void Transform (double x, double y, double *retx, double *rety) const;
		void Transform (TArray<CVector2D> *Points) const;
		CVector2D Transform (const CVector2D &vVector) const;

	private:
		double m_Xform[3][3];

	friend const CXForm2D operator* (const CXForm2D &op1, const CXForm2D &op2);
	};

const CXForm2D operator* (const CXForm2D &op1, const CXForm2D &op2);

//	Geometric Shapes

class CLine2D
	{
	public:
		enum EOpResult
			{
			resultNone,						//	No intersection
			resultOK,						//	Intersection point returned
			resultCollinear,				//	Lines are collinear (mulitple intersections)
			};

		CLine2D (void)
			{ }

		CLine2D (const CVector2D &vFrom, const CVector2D &vTo) :
				m_vFrom(vFrom),
				m_vTo(vTo)
			{ }

		const CVector2D &From (void) const { return m_vFrom; }
		const CVector2D &To (void) const { return m_vTo; }
		void SetFrom (const CVector2D &vVector) { m_vFrom = vVector; }
		void SetTo (const CVector2D &vVector) { m_vTo = vVector; }

		static void CalcCornerPoints (const CVector2D &From, const CVector2D &Center, const CVector2D &To, double rHalfWidth, CVector2D *retInner, CVector2D *retOuter);
		static double CalcDistanceToPoint (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, CVector2D *retvNearest = NULL);
		static EOpResult CalcIntersection (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, const CVector2D &vD, CVector2D *retIntersect = NULL, double *retAB = NULL, double *retCD = NULL, double rDenEpsilon = 0.0);
		static void CalcLineEquation (const CVector2D &vP1, const CVector2D &vP2, double *retrA, double *retrB, double *retrC);
		static bool IsCollinear (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, const CVector2D &vD, double rDenEpsilon = 0.0);
		static EOpResult IsLeftOfLine (const CVector2D &vA, const CVector2D &vB, const CVector2D &vX);

	private:
		CVector2D m_vFrom;
		CVector2D m_vTo;
	};

class CCircle2D
	{
	public:
		CCircle2D (void) :
				m_rRadius(0.0)
			{ }

		CCircle2D (const CVector2D &vCenter, double rRadius) :
				m_vCenter(vCenter),
				m_rRadius(rRadius)
			{
			ASSERT(m_rRadius >= 0.0);
			}

		inline const CVector2D &GetCenter (void) const { return m_vCenter; }
		inline double GetRadius (void) const { return m_rRadius; }

	private:
		CVector2D m_vCenter;
		double m_rRadius;
	};

class CRect2D
	{
	public:
		CRect2D (void)
			{ }

		CRect2D (const CVector2D &vUR, const CVector2D &vLL) :
				m_vUR(vUR),
				m_vLL(vLL)
			{ }

		void Expand (double rValue);
		inline double GetHeight (void) const { return (m_vUR.Y() - m_vLL.Y()); }
		inline const CVector2D &GetLL (void) const { return m_vLL; }
		inline const CVector2D &GetUR (void) const { return m_vUR; }
		inline double GetWidth (void) const { return (m_vUR.X() - m_vLL.X()); }
		bool LineIntersects (const CVector2D &vA, const CVector2D &vB) const;
		bool PointIntersects (const CVector2D &vA) const;
		bool RectIntersects (const CRect2D &Rect) const;
		inline void SetLL (const CVector2D &vLL) { m_vLL = vLL; }
		inline void SetUR (const CVector2D &vUR) { m_vUR = vUR; }
		void Union (const CRect2D &Rect);

	private:
		CVector2D m_vUR;
		CVector2D m_vLL;
	};

class CPolygon2D
	{
	public:
		enum EOpResult
			{
			resultOK,
			resultEmpty,					//	Result is an empty polygon
			resultClipPoly,					//	Result is the clipping polygon
			resultSubjectPoly,				//	Result is the subject polygon
			};

		CPolygon2D (int iPoints = 0);
		CPolygon2D (const TArray<CVector2D> &vPoints);
		CPolygon2D (const CPolygon2D &Src);
		~CPolygon2D (void);

		CPolygon2D &operator= (const CPolygon2D &Src);

		void AddHole (const CPolygon2D &HolePoly);
		void AddHoleHandoff (CPolygon2D &HolePoly);
		void CalcBoundingRect (CRect2D *retRect) const;
		void DeleteHole (int iIndex);
		void Expand (double rExpansion, CPolygon2D *retResult, bool bNoValidityCheck = false) const;
		bool FindVertex (const CVector2D &vPos, double rEpsilon2, int *retiIndex = NULL) const;
		const CPolygon2D &GetHole (int iIndex) const { return *m_Holes[iIndex]; }
		inline int GetHoleCount (void) const { return m_Holes.GetCount(); }
		void GetHoleListHandoff (TArray<CPolygon2D> *retResult);
		inline const CVector2D &GetVertex (int iIndex) const { return m_vPoints[iIndex]; }
		inline int GetVertexCount (void) const { return m_vPoints.GetCount(); }
		ERotationDirections GetVertexOrder (void) const;
		bool HasVertex (const CVector2D &vPos, double rEpsilon2) const;
		EOpResult IntersectPolygon (const CPolygon2D &ClipPoly, TArray<CPolygon2D> *retResult = NULL) const;
		EOpResult IntersectPolygon (const TArray<CPolygon2D> &ClipRegion, TArray<CPolygon2D> *retResult = NULL) const;
		inline bool IsEmpty (void) const { return (m_vPoints.GetCount() == 0); }
		bool IsSelfIntersecting (void) const;
		bool LineIntersects (const CVector2D &vA, const CVector2D &vB) const;
		bool PointIntersects (const CVector2D &vA, bool bIncludeHoles = true) const;
		bool PointIntersectsHole (const CVector2D &vA) const;
		void SetCanonicalVertexOrder (void);
		inline void SetVertex (int iIndex, const CVector2D &vPoint) { m_vPoints[iIndex] = vPoint; }
		void SetVertexOrder (ERotationDirections iDirection);
		void Simplify (double rMinSegment, CPolygon2D *retResult) const;
		EOpResult SubtractPolygon (const CPolygon2D &DrillRegion, TArray<CPolygon2D> *retResult = NULL) const;
		EOpResult SubtractPolygon (const TArray<const CPolygon2D *> &DrillRegion, TArray<CPolygon2D> *retResult = NULL) const;
		EOpResult SubtractPolygon (const TArray<CPolygon2D> &DrillRegion, TArray<CPolygon2D> *retResult = NULL) const;
		inline void TakeHandoff (TArray<CVector2D> &Points) { m_vPoints.TakeHandoff(Points); }
		inline void TakeHandoff (CPolygon2D &Poly) { m_vPoints.TakeHandoff(Poly.m_vPoints); m_Holes.TakeHandoff(Poly.m_Holes); }
		EOpResult UnionPolygon (const CPolygon2D &Poly, CPolygon2D *retResult = NULL) const;

#ifdef DEBUG
		void OutputPolygon (const CString &sName = NULL_STR) const;
#endif

	private:
		void CleanUp (void);
		void Copy (const CPolygon2D &Src);

		TArray<CVector2D> m_vPoints;
		TArray<CPolygon2D *> m_Holes;
	};

class CArc2D
	{
	public:
		CArc2D (void) :
				m_rRadius(0.0),
				m_rStart(0.0),
				m_rEnd(0.0)
			{ }

		CArc2D (const CVector2D &vCenter, double rRadius, double rStart, double rEnd) :
				m_vCenter(vCenter),
				m_rRadius(rRadius),
				m_rStart(rStart),
				m_rEnd(rEnd)
			{ }

		inline const CVector2D &GetCenter (void) const { return m_vCenter; }
		inline double GetEnd (void) const { return m_rEnd; }
		inline double GetRadius (void) const { return m_rRadius; }
		inline double GetStart (void) const { return m_rStart; }
		inline void SetCenter (const CVector2D &vCenter) { m_vCenter = vCenter; }
		inline void SetEnd (double rEnd) { m_rEnd = rEnd; }
		inline void SetRadius (double rRadius) { m_rRadius = rRadius; }
		inline void SetStart (double rStart) { m_rStart = rStart; }

	private:
		CVector2D m_vCenter;
		double m_rRadius;
		double m_rStart;
		double m_rEnd;
	};

class CArcOutline
	{
	public:
		void AddCircle (const CCircle2D &NewCircle);
		void GetSortedArcList (TArray<CArc2D> *retList);

	private:
		enum EResultTypes
			{
			resultNoIntersection,
			resultIntersect,
			resultOverlapped1,
			resultOverlapped2,
			};

		struct SArc
			{
			double rStart;
			double rEnd;
			};

		struct SCircle
			{
			SCircle (void) :
					bDelete(false)
				{ }

			CVector2D vCenter;
			double rRadius;

			TArray<SArc> Arcs;
			bool bDelete;
			};

		struct SArcEntry
			{
			SCircle *pCircle;
			SArc *pArc;

			CVector2D vStart;
			CVector2D vEnd;

			SArcEntry *pNext;
			};

		void CombineArc (SCircle *pCircle, const SArc &Arc);
		EResultTypes Intersect (SCircle *pC1, SCircle *pC2, SArc *retC1Arc, SArc *retC2Arc);

		TArray<SCircle> m_Circles;
	};

//	Voronoi tessellation

class CVoronoiTessellation
	{
	public:
		inline int GetNeighbor (int iSiteIndex, int iIndex) const { return m_Sites[iSiteIndex].Neighbors[iIndex]; }
		inline int GetNeighborCount (int iSiteIndex) const { return m_Sites[iSiteIndex].Neighbors.GetCount(); }
		inline const CLine2D &GetNeighborEdge (int iSiteIndex, int iIndex) const { return m_Edges[m_Sites[iSiteIndex].Edges[iIndex]].Edge; }
		inline int GetSiteCount (void) const { return m_Sites.GetCount(); }
		inline DWORD GetSiteData (int iIndex) const { return m_Sites[iIndex].dwData; }
		inline void GetSiteNeighbors (int iIndex, TArray<int> *retNeighbors) const { *retNeighbors = m_Sites[iIndex].Neighbors; }
		inline const CVector2D &GetSiteOrigin (int iIndex) const { return m_Sites[iIndex].Origin; }
		inline void GetSitePolygon (int iIndex, TArray<CVector2D> *retPoints) const { *retPoints = m_Sites[iIndex].Polygon; }
		inline void GetSitePolygon (int iIndex, CPolygon2D *retPoly) const { *retPoly = CPolygon2D(m_Sites[iIndex].Polygon); }
		void Init (const TArray<CVector2D> &Points, double rWidth = 0.0, double rHeight = 0.0);
		inline void SetSiteData (int iIndex, DWORD dwData) { m_Sites[iIndex].dwData = dwData; }

	private:
		struct SEdge
			{
			CLine2D Edge;
			int iSite1;
			int iSite2;
			};

		struct SSite
			{
			CVector2D Origin;				//	Original point
			TArray<CVector2D> Polygon;		//	Polygon points
			TArray<int> Edges;				//	Index into edges table
			TArray<int> Neighbors;			//	Index into sites table

			DWORD dwData;					//	User data

			TArray<int> LeftToMatch;
			};

		void InsertEdge (SSite &Site, int iEdge);
		bool InsertEdgeToPolygon (SSite &Site, int iEdge, bool bDefer = true);
		void ValidatePolygon (SSite &Site);

		TArray<SSite> m_Sites;
		TArray<SEdge> m_Edges;

		double m_rWidth;
		double m_rHeight;
	};

class CVoronoiNavigator
	{
	public:
		CVoronoiNavigator (CVoronoiTessellation &Graph);

		void GetOutlines (TArray<CPolygon2D> *retResult);
		void SelectSites (const TArray<int> &SiteList);

	private:
		struct SSiteInfo
			{
			int iIndex;						//	Site index in m_Graph
			bool bMarked;					//	Entire site has been processed
			DWORD dwEdgesProcessed;			//	Set of edges to process
			};

		void InitEdgesToProcess (void);
		bool FindNextEdge (int iSite, CVector2D &vNextPoint, int *retiEdge);
		bool FindNextOutlinePoint (int &iNextSite, CVector2D &vNextPoint, const CVector2D &vEndPoint, int *retiEdge);
		int FindStartEdge (int iSiteIndex);
		bool IsSiteEdge (int iSiteIndex);
		void SetEdge (SSiteInfo &Site, int iEdge);

		CVoronoiTessellation &m_Graph;
		TArray<SSiteInfo> m_Selection;
	};

struct SVoronoiEdge
	{
	CLine2D Edge;
	int iSite1;
	int iSite2;
	};

void mathVoronoi (const TArray<CVector2D> &Points, TArray<CLine2D> *retSegments, double rWidth = 0.0, double rHeight = 0.0);
void mathVoronoiEx (const TArray<CVector2D> &Points, TArray<SVoronoiEdge> *retSegments, double rWidth = 0.0, double rHeight = 0.0);

//	3D Objects -----------------------------------------------------------------

//	Functions ------------------------------------------------------------------

inline double mathAngleMod (double rAngle) { if (rAngle >= 0.0) return fmod(rAngle, TAU); else return TAU - fmod(-rAngle, TAU); }
inline int mathAngleMod (int iAngle) { if (iAngle >= 0) return (iAngle % 360); else return (360 - (-iAngle % 360)); }
inline double mathAngleDiff (double rFrom, double rTo) { return mathAngleMod(rTo - rFrom); }
inline double mathDegreesToRadians (int iAngle) { return iAngle * PI / 180.0; }
inline double mathDegreesToRadians (double rAngle) { return rAngle * PI / 180.0; }
bool mathLinesIntersect (const CVector2D &vA, const CVector2D &vB, const CVector2D &vC, const CVector2D &vD);
inline double mathRadiansToDegrees (double rAngle) { return 180.0 * rAngle / PI; }
