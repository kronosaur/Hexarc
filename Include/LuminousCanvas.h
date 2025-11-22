//	LuminousCanvas.h
//
//	LuminousCore Classes
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

//	2D Canvas ------------------------------------------------------------------
//
//	A canvas is a collection of 2D ILuminousGraphic elements. Each graphic is
//	a component to be drawn in the final render. All graphics have a 2D position
//	on the canvas, and a z-order.
//
//	A canvas always has its origin (0, 0) at the upper-left, and the y-axis is
//	positive going down (like a standard bitmapped screen). At render time, we
//	can remap to different resolutions and coordinates.

class ILuminousGraphic : public ILuminousElement
	{
	public:

		enum class EType
			{
			None,

			BeginUpdate,					//	Marker to indicate start of frame
			ClearRect,						//	Clear a rect on the canvas
			Image,							//	Draw an image
			Resource,						//	Draw a resource
			Shape,							//	A path filled or stroked (or both).
			Text,							//	Text
			};

		ILuminousGraphic (DWORD dwID, const CString& sName) : ILuminousElement(dwID, sName)
			{ }

		virtual TUniquePtr<ILuminousGraphic> Clone () const = 0;

		const CLuminousTextAlign& GetAlignStyle () const { return OnGetAlignStyle(); }
		const CLuminousFillStyle& GetFillStyle () const { return OnGetFillStyle(); }
		const CLuminousFontStyle& GetFontStyle () const { return OnGetFontStyle(); }
		const CRGBA32Image& GetImage () const { return OnGetImage(); }
		const CLuminousLineStyle& GetLineStyle () const { return OnGetLineStyle(); }
		const CVector2D& GetPos () const { return m_vPos; }
		const CString& GetResource () const { return OnGetResource(); }
		double GetRotation () const { return m_rRotation; }
		const CVector2D& GetScale () const { return m_vScale; }
		const CLuminousShadowStyle& GetShadowStyle () const { return OnGetShadowStyle(); }
		const CLuminousPath2D& GetShapePath () const { return OnGetShapePath(); }
		const CString& GetText () const { return OnGetText(); }
		EType GetType () const { return OnGetType(); }
		void SetPos (const CVector2D& vPos) { m_vPos = vPos; }
		void SetRotation (double rRotation) { m_rRotation = rRotation; }
		void SetScale (const CVector2D& vScale) { m_vScale = vScale; }

	private:

		virtual const CLuminousTextAlign& OnGetAlignStyle () const { return CLuminousTextAlign::Null; }
		virtual const CLuminousFillStyle& OnGetFillStyle () const { return CLuminousFillStyle::Null; }
		virtual const CLuminousFontStyle& OnGetFontStyle () const { return CLuminousFontStyle::Null; }
		virtual const CRGBA32Image& OnGetImage () const { return CRGBA32Image::Null(); }
		virtual const CLuminousLineStyle& OnGetLineStyle () const { return CLuminousLineStyle::Null; }
		virtual const CString& OnGetResource () const { return NULL_STR; }
		virtual const CLuminousShadowStyle& OnGetShadowStyle () const { return CLuminousShadowStyle::Null; }
		virtual const CLuminousPath2D& OnGetShapePath () const { return CLuminousPath2D::Null; }
		virtual const CString& OnGetText () const { return NULL_STR; }
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
		const CLuminousFontStyle& Font () const { return m_FontStyle; }
		CLuminousFontStyle& Font () { return m_FontStyle; }
		const CLuminousLineStyle& Line () const { return m_LineStyle; }
		CLuminousLineStyle& Line () { return m_LineStyle; }
		const CLuminousPath2D& Path () const { return m_Path; }
		CLuminousPath2D& Path () { return m_Path; }
		CLuminousShadowStyle& Shadow () { return m_ShadowStyle; }
		const CLuminousTextAlign& TextAlign () const { return m_AlignStyle; }
		CLuminousTextAlign& TextAlign () { return m_AlignStyle; }

		static CLuminousCanvasCtx Read (IByteStream& Stream);
		void Write (IByteStream& Stream) const;

	private:

		CLuminousPath2D m_Path;
		CLuminousTextAlign m_AlignStyle;
		CLuminousFillStyle m_FillStyle = CLuminousFillStyle(CLuminousColor(CRGBA32(0, 0, 0)));
		CLuminousFontStyle m_FontStyle;
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

		struct STextOptions
			{
			CString sText;
			CVector2D Pos;
			CLuminousFontStyle FontStyle;
			CLuminousTextAlign AlignStyle;
			CLuminousFillStyle FillStyle;
			CLuminousLineStyle LineStyle;
			CLuminousShadowStyle ShadowStyle;
			};

		static TUniquePtr<ILuminousGraphic> CreateBeginUpdate (const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateClearRect (const CVector2D& vUL, const CVector2D& vLR, const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateImage (const CRGBA32Image& ImageRef, const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateResource (const CString& sResourceID, const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateShape (const SShapeOptions& Options, const CString& sName = NULL_STR);
		static TUniquePtr<ILuminousGraphic> CreateText (const STextOptions& Options, const CString& sName = NULL_STR);

		CLuminousCanvasModel () { }
		CLuminousCanvasModel (const CLuminousCanvasModel& Src) { Copy(Src); }
		CLuminousCanvasModel (CLuminousCanvasModel&& Src) noexcept = default;

		CLuminousCanvasModel& operator= (const CLuminousCanvasModel& Src) { CleanUp(); Copy(Src); return *this; }
		CLuminousCanvasModel& operator= (CLuminousCanvasModel&& Src) noexcept = default;

		void DeleteAll ();
		const ILuminousGraphic* FindGraphicByID (DWORD dwID) const;
		ILuminousGraphic& GetGraphicByID (DWORD dwID);
		int GetRenderCount () const { return m_ZOrder.GetCount(); }
		ILuminousGraphic& GetRenderGraphic (int iIndex) { if (iIndex >= 0 && iIndex < m_ZOrder.GetCount()) return *m_ZOrder[iIndex]; else throw CException(errFail); }
		const ILuminousGraphic& GetRenderGraphic (int iIndex) const { if (iIndex >= 0 && iIndex < m_ZOrder.GetCount()) return *m_ZOrder[iIndex]; else throw CException(errFail); }
		ILuminousGraphic& InsertGraphic (TUniquePtr<ILuminousGraphic>&& pGraphic);
		void SetSeq (SequenceNumber Seq);

		static CLuminousCanvasModel Read (IByteStream& Stream);
		void Write (IByteStream& Stream) const;

	private:

		void CleanUp () { m_Graphics.DeleteAll(); m_ZOrder.DeleteAll(); m_ByName.DeleteAll(); m_dwNextID = 1; }
		void Copy (const CLuminousCanvasModel& Src);

		TSortMap<DWORD, TUniquePtr<ILuminousGraphic>> m_Graphics;
		TArray<ILuminousGraphic*> m_ZOrder;
		TSortMap<CString, ILuminousGraphic*> m_ByName;

		DWORD m_dwNextID = 1;
	};

