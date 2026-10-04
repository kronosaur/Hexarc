//	Obj2DImpl.h
//
//	LuminousCore Classes
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "LuminousCore.h"

class CObj2DCircle : public ILuminousObj2D
	{
	public:

		CObj2DCircle (CLuminousScene2D& Scene, DWORD dwID, ILuminousObj2D* pParent) :
				ILuminousObj2D(Scene, dwID, pParent)
			{ }

		virtual DWORD GetImpl () const { return IMPL_CIRCLE; }

	private:

		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual TUniquePtr<ILuminousObj2D> OnClone () const override;
		virtual const CString& OnGetObjType () const override;
		virtual CLuminousColor OnGetPropertyColor (Obj2DProp iProp) const override;
		virtual double OnGetPropertyScalar (Obj2DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;
		virtual bool OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value) override;
		virtual bool OnSetPropertyScalar (Obj2DProp iProp, double rValue) override;

		double m_rRadius = 0.0;
		CLuminousFillStyle m_FillStyle;
		CLuminousLineStyle m_OutlineStyle;
	};

class CObj2DRectangle : public ILuminousObj2D
	{
	public:

		CObj2DRectangle (CLuminousScene2D& Scene, DWORD dwID, ILuminousObj2D* pParent) :
				ILuminousObj2D(Scene, dwID, pParent)
			{ }

		virtual DWORD GetImpl () const { return IMPL_RECTANGLE; }

	private:

		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual TUniquePtr<ILuminousObj2D> OnClone () const override;
		virtual const CString& OnGetObjType () const override;
		virtual bool OnGetPropertyBool (Obj2DProp iProp) const override;
		virtual CLuminousColor OnGetPropertyColor (Obj2DProp iProp) const override;
		virtual double OnGetPropertyScalar (Obj2DProp iProp) const override;
		virtual CVector2D OnGetPropertyVector (Obj2DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;
		virtual bool OnSetPropertyBool (Obj2DProp iProp, bool bValue) override;
		virtual bool OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value) override;
		virtual bool OnSetPropertyScalar (Obj2DProp iProp, double rValue) override;
		virtual bool OnSetPropertyVector (Obj2DProp iProp, const CVector2D& Value) override;

		CVector2D m_Size;					//	Width and height of rectangle
		CLuminousCornerRadius m_CornerRadius;
		CLuminousFillStyle m_FillStyle;
		CLuminousLineStyle m_OutlineStyle;
	};

class CObj2DTrail : public ILuminousObj2D
	{
	public:

		CObj2DTrail (CLuminousScene2D& Scene, DWORD dwID, ILuminousObj2D* pParent) :
				ILuminousObj2D(Scene, dwID, pParent)
			{ }

		virtual DWORD GetImpl () const { return IMPL_TRAIL; }

		void AddPoint (const CVector2D& vPoint);

	private:

		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual TUniquePtr<ILuminousObj2D> OnClone () const override;
		virtual const CString& OnGetObjType () const override;
		virtual CLuminousColor OnGetPropertyColor (Obj2DProp iProp) const override;
		virtual double OnGetPropertyScalar (Obj2DProp iProp) const override;
		virtual const TArray<CVector2D>& OnGetPropertyVectorQueue (Obj2DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;
		virtual bool OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value) override;
		virtual bool OnSetPropertyScalar (Obj2DProp iProp, double rValue) override;
		virtual bool OnSetPropertyVectorQueue (Obj2DProp iProp, const TArray<CVector2D>& Value) override;

		int m_iMaxPoints = 0;
		TArray<CVector2D> m_Points;
		CLuminousLineStyle m_OutlineStyle;
	};

class CObj2DLine : public ILuminousObj2D
	{
	public:

		CObj2DLine (CLuminousScene2D& Scene, DWORD dwID, ILuminousObj2D* pParent) :
				ILuminousObj2D(Scene, dwID, pParent)
			{ }

		virtual DWORD GetImpl () const { return IMPL_LINE; }

	private:

		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual TUniquePtr<ILuminousObj2D> OnClone () const override;
		virtual const CString& OnGetObjType () const override;
		virtual CLuminousColor OnGetPropertyColor (Obj2DProp iProp) const override;
		virtual double OnGetPropertyScalar (Obj2DProp iProp) const override;
		virtual const TArray<CVector2D>& OnGetPropertyVectorQueue (Obj2DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;
		virtual bool OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value) override;
		virtual bool OnSetPropertyScalar (Obj2DProp iProp, double rValue) override;
		virtual bool OnSetPropertyVectorQueue (Obj2DProp iProp, const TArray<CVector2D>& Value) override;

		TArray<CVector2D> m_Points;
		CLuminousLineStyle m_OutlineStyle;
	};

// Single-line, browser-shaped text. Font syntax is preserved for Canvas.
class CObj2DText : public ILuminousObj2D
	{
	public:
		CObj2DText (CLuminousScene2D& Scene, DWORD dwID, ILuminousObj2D* pParent) : ILuminousObj2D(Scene, dwID, pParent) { }
		virtual DWORD GetImpl () const override { return IMPL_TEXT; }

	private:
		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual TUniquePtr<ILuminousObj2D> OnClone () const override { return TUniquePtr<ILuminousObj2D>(new CObj2DText(*this)); }
		virtual const CString& OnGetObjType () const override;
		virtual CLuminousColor OnGetPropertyColor (Obj2DProp iProp) const override;
		virtual double OnGetPropertyScalar (Obj2DProp iProp) const override;
		virtual CString OnGetPropertyString (Obj2DProp iProp) const override;
		virtual bool OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value) override;
		virtual bool OnSetPropertyScalar (Obj2DProp iProp, double rValue) override;
		virtual bool OnSetPropertyString (Obj2DProp iProp, const CString& sValue) override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		CString m_sText;
		CString m_sFont = CString("10px sans-serif");
		CString m_sTextAlign = CString("start");
		CString m_sTextBaseline = CString("alphabetic");
		CString m_sDirection = CString("ltr");
		CString m_sTextFit = CString("canvas");
		double m_rMaxWidth = -1.0;
		double m_rMinFontSize = -1.0;
		double m_rMaxFontSize = -1.0;
		double m_rLineWidth = 1.0;
		double m_rShadowBlur = 0.0;
		double m_rShadowOffsetX = 0.0;
		double m_rShadowOffsetY = 0.0;
		CLuminousColor m_FillColor = CLuminousColor(CRGBA32(0, 0, 0));
		CLuminousColor m_LineColor;
		CLuminousColor m_ShadowColor;
	};
