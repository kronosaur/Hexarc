//	PolygonIntersect.h
//
//	Classes used by Foundation
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CPolygonIntersector
	{
	public:
		enum EOpTypes
			{
			opIntersect,
			opSubtract,
			opUnion,
			};

		CPolygonIntersector (void);

		inline void DeleteAllClipRegions (void) { m_Clip.DeleteAll(); }
		inline void InsertClipRegion (const CPolygon2D &Clip) { m_Clip.Insert(&Clip); }
		CPolygon2D::EOpResult Intersect (TArray<CPolygon2D> *retResult = NULL);
		void SetClipRegion (const CPolygon2D &Clip);
		void SetClipRegion (const TArray<CPolygon2D> &Clip);
		void SetSubjectRegion (const CPolygon2D &Subject);
		CPolygon2D::EOpResult Subtract (TArray<CPolygon2D> *retResult = NULL);
		CPolygon2D::EOpResult Union (CPolygon2D *retResult = NULL);

	private:
		enum EIntersectionTypes
			{
			intersectOutside,
			intersectInside,

			intersectSubjectInsideClip,
			intersectClipInsideSubject,
			intersectEquals,
			intersectDisjoint,
			};

		enum EVertexFlags
			{
			flagSubject =			0x00000001,	//	Vertex is part of the subect
			flagClip =				0x00000002,	//	Vertex is part of the clip polygon
			flagIntersection =		0x00000004,	//	Vertex is an intersection
			flagStart =				0x00000008,	//	Vertex is the start of a polygon
			flagProcessed =			0x00000010,	//	TRUE if we already processed this vertex
			flagHole =				0x00000020,	//	Vertex is part of a hole
			flagIntResolved =		0x00000040,	//	Intersections added
			flagOriginal =			0x00000080,	//	We're an original vertex
			flagDeleted =			0x00000100,	//	Vertex deleted
			flagOutside =			0x00000200,	//	Outside intersection
			flagInside =			0x00000400,	//	Inside intersection
			flagCollinear =			0x00000800,	//	Marked during inside/outside computation
			};

		struct SVertexInfo
			{
			CVector2D vPos;					//	Position of vertex
			DWORD dwFlags;					//	Type of vertex

			SVertexInfo *pNext;				//	Next vertex in clockwise order
			SVertexInfo *pPrev;				//	Prev vertex

			SVertexInfo *pNextPoly;			//	The last vertex points to the next polygon (for holes)

			SVertexInfo *pIntersect;		//	If an intersection, pointer to matching point.
			double rX;						//	Temp variable used for insertion
			int iOrder;						//	Temp variable used for insertion
			};

		void AccumulateNonIntersecting (SVertexInfo *pPoly, TArray<SVertexInfo *> *retResult);
		void AccumulateNonIntersectingHoles (SVertexInfo *pPoly, TArray<SVertexInfo *> *retResult);
		void AddHoleToPolygons (TArray<CPolygon2D> &Result, CPolygon2D &Hole);
		void AddHoles (CPolygon2D *pDest, const TArray<SVertexInfo *> &Holes);
		void AddHolesAsPolygons (const CPolygon2D &OriginalPoly, const TArray<SVertexInfo *> &Holes, TArray<CPolygon2D> *retResult);
		void AddPolygonAndHoles (const CPolygon2D &Poly, const CPolygon2D &HoleSource, TArray<CPolygon2D> *retResult);
		SVertexInfo *AllocVertex (const CVector2D &vPos, DWORD dwFlags, SVertexInfo *pNext = NULL, SVertexInfo *pPrev = NULL);
		void CalcInsideOutsideIntersections (SVertexInfo *pPoly);
		void CollapseIntersections (SVertexInfo *pPoly);
		void CreatePolygonFromVertexList (SVertexInfo *pPoly, CPolygon2D *retResult);
		SVertexInfo *CreateVertexList (const CPolygon2D &Src, DWORD dwFlags);
		EIntersectionTypes GetContainmentRelationship (const CPolygon2D &Subject, const CPolygon2D &Clip);
		void GetIntersections (SVertexInfo *pPoly, EIntersectionTypes iType, TArray<SVertexInfo *> *retOutside);
		int GetTotalVertexCount (const CPolygon2D &Poly);
		void InitVerticesArray (void);
		void InsertIntersections (SVertexInfo *pClip, SVertexInfo *pSubject);
		CPolygon2D::EOpResult Intersect (const CPolygon2D &Clip, TArray<CPolygon2D> *retResult);
		SVertexInfo *RemoveIntersection (SVertexInfo *pIntersect);
		void ResolveIntersections (SVertexInfo *pPoly);
		CPolygon2D::EOpResult Subtract (const CPolygon2D &Clip, TArray<CPolygon2D> *retResult);
		void TraceOutline (SVertexInfo *pStart, EOpTypes iOp, TArray<CVector2D> *retResult);
		CPolygon2D::EOpResult UnionHoles (const CPolygon2D &Container, const CPolygon2D &Poly, CPolygon2D *retResult);

		static int CompareVertexIntersections (void *pCtx, const SVertexInfo *&Key1, const SVertexInfo *&Key2);

#ifdef DEBUG
		void OutputVertexList (SVertexInfo *pPoly);
#endif

		const CPolygon2D *m_pSubject;
		const CPolygon2D *m_pCurClip;
		TArray<const CPolygon2D *> m_Clip;

		TArray<SVertexInfo> m_Vertices;
		int m_iIntersectCount;				//	Total number of intersections
		int m_iVertexMatches;				//	Number of vertices that clip and subject have in common.
		int m_iVertexAlloc;
	};