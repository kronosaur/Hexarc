//	LuminousCore.h
//
//	LuminousCore Classes
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "Foundation.h"

//	ILuminousElement
//
//	This is the base class for all Luminous objects. It handles containment and
//	properties.

class ILuminousElement
	{
	public:

		ILuminousElement (DWORD dwID, const CString& sName) :
				m_dwID(dwID),
				m_sName(sName)
			{ }

		virtual ~ILuminousElement () { }

		DWORD GetID () const { return m_dwID; }
		const CString& GetName () const { return m_sName; }
		SequenceNumber GetSeq () const { return m_Seq; }
		void SetID (DWORD dwID) { m_dwID = dwID; }
		void SetName (const CString& sName) { m_sName = sName; }
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }

	private:

		DWORD m_dwID = 0;
		CString m_sName;
		SequenceNumber m_Seq = 1;
	};

//	Animation ------------------------------------------------------------------

class ILuminousAnimator : public ILuminousElement
	{
	};

class CLuminousTimeline
	{
	};

//	2D Canvas ------------------------------------------------------------------
//
//	A canvas is a collection of 2D ILuminousGraphic elements. Each graphic is
//	a component to be drawn in the final render. All graphics have a 2D position
//	on the canvas, and a z-order.
//
//	A canvas always has its origin (0, 0) at the upper-left, and the y-axis is
//	positive going down (like a standard bitmapped screen). At render time, we
//	can remap to different resolutions and coordinates.

class CLuminousColor
	{
	public:

		enum class EType
			{
			None,

			Clear,
			Solid,
			LinearGradient,
			RadialGradient,
			ImagePattern,
			};

		CLuminousColor () {}
		CLuminousColor (EType iType) : m_iType(iType)
			{ }

		CLuminousColor (const CRGBA32& rgbColor) :
				m_iType(EType::Solid),
				m_rgbColor(rgbColor)
			{ }

		bool operator== (const CLuminousColor &Src) const;
		bool operator!= (const CLuminousColor &Src) const { return !(*this == Src); }

		const CRGBA32& GetSolidColor () const { return m_rgbColor; }
		EType GetType () const { return m_iType; }
		bool IsEmpty () const { return m_iType == EType::None; }

		static const CLuminousColor Null;

	private:

		EType m_iType = EType::None;
		CRGBA32 m_rgbColor = CRGBA32(0, 0, 0);
	};

class CLuminousFillStyle
	{
	public:

		CLuminousFillStyle () { }
		CLuminousFillStyle (const CLuminousColor& Color) :
				m_Color(Color)
			{ }

		bool operator== (const CLuminousFillStyle &Src) const;
		bool operator!= (const CLuminousFillStyle &Src) const { return !(*this == Src); }

		const CLuminousColor& GetColor () const { return m_Color; }
		bool IsEmpty () const { return m_Color.IsEmpty(); }
		void SetColor (const CLuminousColor& Color) { m_Color = Color; }

		static const CLuminousFillStyle Null;

	private:

		CLuminousColor m_Color;
	};

class CLuminousLineStyle
	{
	public:

		enum class ELineCap
			{
			Unknown,

			Butt,				//	A flat edge is added  to each end of the line.
			Round,				//	A rounded end cap is added
			Square,				//	A square end cap is added
			};

		enum class ELineJoin
			{
			Unknown,

			Bevel,				//	A beveled corner
			Round,				//	A rounded corner
			Miter,				//	Creates a sharp corner
			};

		CLuminousLineStyle () { }
		CLuminousLineStyle (const CLuminousColor& Color, 
							double rWidth = 1.0, 
							ELineJoin iLineJoin = ELineJoin::Miter,
							ELineCap iLineCap = ELineCap::Butt,
							double rMiterLimit = 10.0) :
				m_Color(Color),
				m_rLineWidth(rWidth),
				m_iLineJoin(iLineJoin),
				m_iLineCap(iLineCap),
				m_rMiterLimit(rMiterLimit)
			{ }

		bool operator== (const CLuminousLineStyle &Src) const;
		bool operator!= (const CLuminousLineStyle &Src) const { return !(*this == Src); }

		const CLuminousColor& GetColor () const { return m_Color; }
		ELineCap GetLineCap () const { return m_iLineCap; }
		ELineJoin GetLineJoin () const { return m_iLineJoin; }
		double GetLineWidth () const { return m_rLineWidth; }
		double GetMiterLimit () const { return m_rMiterLimit; }
		bool IsEmpty () const { return m_Color.IsEmpty(); }
		void SetColor (const CLuminousColor& Color) { m_Color = Color; }
		void SetLineWidth (double rWidth) { m_rLineWidth = Max(0.0, rWidth); }

		static const CLuminousLineStyle Null;

	private:

		CLuminousColor m_Color;
		double m_rLineWidth = 1.0;
		ELineCap m_iLineCap = ELineCap::Butt;
		ELineJoin m_iLineJoin = ELineJoin::Miter;
		double m_rMiterLimit = 10.0;
	};

class CLuminousShadowStyle
	{
	public:

		CLuminousShadowStyle () { }
		CLuminousShadowStyle (const CLuminousColor& Color, double rBlur, const CVector2D& vOffset) :
				m_Color(Color),
				m_rBlur(rBlur),
				m_rOffsetX(vOffset.X()),
				m_rOffsetY(vOffset.Y())
			{ }

		bool operator== (const CLuminousShadowStyle &Src) const;
		bool operator!= (const CLuminousShadowStyle &Src) const { return !(*this == Src); }

		double GetBlur () const { return m_rBlur; }
		const CLuminousColor& GetColor () const { return m_Color; }
		CVector2D GetOffset () const { return CVector2D(m_rOffsetX, m_rOffsetY); }
		bool IsEmpty () const { return m_Color.IsEmpty(); }
		void SetColor (const CLuminousColor& Color) { m_Color = Color; }

		static const CLuminousShadowStyle Null;

	private:

		CLuminousColor m_Color;
		double m_rBlur = 0.0;
		double m_rOffsetX = 0.0;
		double m_rOffsetY = 0.0;
	};

class CLuminousPath2D
	{
	public:

		enum class ESegmentType
			{
			None,

			ArcClockwise,
			ArcCounterClockwise,
			ArcTo,
			ClosePath,
			LineTo,
			MoveTo,
			Rect,
			};

		bool operator== (const CLuminousPath2D &Src) const;
		bool operator!= (const CLuminousPath2D &Src) const { return !(*this == Src); }

		void Arc (const CVector2D& vCenter, double rRadius, double rStartAngle, double rEndAngle, bool bCounterClockwise = false);
		void ArcTo (const CVector2D& v1stTangent, const CVector2D& v2ndTangent, double rRadius);
		void ClosePath ();
		void DeleteAll () { m_Path.DeleteAll(); }
		void GetArc (int iIndex, CVector2D& retCenter, double& retRadius, double& retStartAngle, double& retEndAngle, bool& retCounterClockwise) const;
		void GetArcTo (int iIndex, CVector2D& retTangent1, CVector2D& retTangent2, double& retRadius) const;
		CVector2D GetLineTo (int iIndex) const { if (iIndex >= 0 && iIndex < m_Path.GetCount() && m_Path[iIndex].iType == ESegmentType::LineTo) return m_Path[iIndex].A; else throw CException(errFail); }
		CVector2D GetMoveTo (int iIndex) const { if (iIndex >= 0 && iIndex < m_Path.GetCount() && m_Path[iIndex].iType == ESegmentType::MoveTo) return m_Path[iIndex].A; else throw CException(errFail); }
		void GetRect (int iIndex, CVector2D& retUL, CVector2D& retLR) const;
		int GetSegmentCount () const { return m_Path.GetCount(); }
		ESegmentType GetSegmentType (int iIndex) const { if (iIndex >= 0 && iIndex < m_Path.GetCount()) return m_Path[iIndex].iType; else throw CException(errFail); }
		bool IsEmpty () const { return m_Path.GetCount() == 0; }
		void LineTo (const CVector2D& vPos);
		void MoveTo (const CVector2D& vPos);
		void Rect (const CVector2D& vUL, const CVector2D& vLR);

		static const CLuminousPath2D Null;

	private:

		//	We encode the various parameter for each segment type:
		//
		//	ArcClockwise
		//	ArcCounterClockwise
		//
		//		A = Center
		//		B.x = Radius
		//		C.x = Start angle (radians)
		//		C.y = End angle (radians)
		//
		//	ArcTo
		//
		//		A = 1st Tangent
		//		B = 2nd Tangent
		//		C.x = Radius
		//
		//	ClosePath
		//
		//	LineTo
		//
		//		A = pos
		//
		//	MoveTo
		//
		//		A = pos
		//
		//	Rect
		//
		//		A = Upper-left corner
		//		B = Lower-right corner

		struct SSegment
			{
			bool operator== (const SSegment &Src) const { return (iType == Src.iType) && (A == Src.A) && (B == Src.B) && (C == Src.C); }
			bool operator!= (const SSegment &Src) const { return !(*this == Src); }

			ESegmentType iType = ESegmentType::None;
			CVector2D A;
			CVector2D B;
			CVector2D C;
			};

		TArray<SSegment> m_Path;
	};

class ILuminousGraphic : public ILuminousElement
	{
	public:

		enum class EType
			{
			None,

			BeginUpdate,					//	Marker to indicate start of frame
			ClearRect,						//	Clear a rect on the canvas
			Image,							//	Draw an image
			Shape,							//	A path filled or stroked (or both).
			};

		ILuminousGraphic (DWORD dwID, const CString& sName) : ILuminousElement(dwID, sName)
			{ }

		const CLuminousFillStyle& GetFillStyle () const { return OnGetFillStyle(); }
		const CRGBA32Image& GetImage () const { return OnGetImage(); }
		const CLuminousLineStyle& GetLineStyle () const { return OnGetLineStyle(); }
		const CVector2D& GetPos () const { return m_vPos; }
		double GetRotation () const { return m_rRotation; }
		const CVector2D& GetScale () const { return m_vScale; }
		const CLuminousShadowStyle& GetShadowStyle () const { return OnGetShadowStyle(); }
		const CLuminousPath2D& GetShapePath () const { return OnGetShapePath(); }
		EType GetType () const { return OnGetType(); }
		void SetPos (const CVector2D& vPos) { m_vPos = vPos; }
		void SetRotation (double rRotation) { m_rRotation = rRotation; }
		void SetScale (const CVector2D& vScale) { m_vScale = vScale; }

	private:

		virtual const CLuminousFillStyle& OnGetFillStyle () const { return CLuminousFillStyle::Null; }
		virtual const CRGBA32Image& OnGetImage () const { return CRGBA32Image::Null(); }
		virtual const CLuminousLineStyle& OnGetLineStyle () const { return CLuminousLineStyle::Null; }
		virtual const CLuminousShadowStyle& OnGetShadowStyle () const { return CLuminousShadowStyle::Null; }
		virtual const CLuminousPath2D& OnGetShapePath () const { return CLuminousPath2D::Null; }
		virtual EType OnGetType () const { return EType::None; }

		CVector2D m_vPos;
		CVector2D m_vScale = CVector2D(1.0, 1.0);
		double m_rRotation = 0.0;					//	Rotation in radians
	};

class CLuminousCanvasCtx
	{
	public:

		const CLuminousFillStyle& Fill () const { return m_FillStyle; }
		CLuminousFillStyle& Fill () { return m_FillStyle; }
		const CLuminousLineStyle& Line () const { return m_LineStyle; }
		CLuminousLineStyle& Line () { return m_LineStyle; }
		const CLuminousPath2D& Path () const { return m_Path; }
		CLuminousPath2D& Path () { return m_Path; }
		CLuminousShadowStyle& Shadow () { return m_ShadowStyle; }

	private:

		CLuminousPath2D m_Path;
		CLuminousFillStyle m_FillStyle = CLuminousFillStyle(CLuminousColor(CRGBA32(0, 0, 0)));
		CLuminousLineStyle m_LineStyle = CLuminousLineStyle(CLuminousColor(CRGBA32(0, 0, 0)));
		CLuminousShadowStyle m_ShadowStyle;
	};

class CLuminousCanvasModel
	{
	public:

		struct SShapeOptions
			{
			CLuminousPath2D Path;
			CLuminousFillStyle FillStyle;
			CLuminousLineStyle LineStyle;
			CLuminousShadowStyle ShadowStyle;
			};

		static TUniquePtr<ILuminousGraphic> CreateBeginUpdate (const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateClearRect (const CVector2D& vUL, const CVector2D& vLR, const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateImage (const CRGBA32Image& ImageRef, const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateShape (const SShapeOptions& Options, const CString& sName = NULL_STR);

		void DeleteAll ();
		const ILuminousGraphic* FindGraphicByID (DWORD dwID) const;
		ILuminousGraphic& GetGraphicByID (DWORD dwID);
		int GetRenderCount () const { return m_ZOrder.GetCount(); }
		ILuminousGraphic& GetRenderGraphic (int iIndex) { if (iIndex >= 0 && iIndex < m_ZOrder.GetCount()) return *m_ZOrder[iIndex]; else throw CException(errFail); }
		const ILuminousGraphic& GetRenderGraphic (int iIndex) const { if (iIndex >= 0 && iIndex < m_ZOrder.GetCount()) return *m_ZOrder[iIndex]; else throw CException(errFail); }
		ILuminousGraphic& InsertGraphic (TUniquePtr<ILuminousGraphic>&& pGraphic);
		void SetSeq (SequenceNumber Seq);

	private:

		TSortMap<DWORD, TUniquePtr<ILuminousGraphic>> m_Graphics;
		TArray<ILuminousGraphic*> m_ZOrder;
		TSortMap<CString, ILuminousGraphic*> m_ByName;

		DWORD m_dwNextID = 1;
	};

//	3D Voxels ------------------------------------------------------------------

class CLuminousVoxelModel
	{
	};

//	3D Scene -------------------------------------------------------------------

class ILuminousObject : public ILuminousElement
	{
	};

class CLuminousSceneModel
	{
	};

