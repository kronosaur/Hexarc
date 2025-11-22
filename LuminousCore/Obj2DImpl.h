//	Obj2DImpl.h
//
//	LuminousCore Classes
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "LuminousCore.h"

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
