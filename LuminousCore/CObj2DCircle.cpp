//	CObj2DCircle.cpp
//
//	CObj2DCircle Class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_CIRCLE,					"circle");

void CObj2DCircle::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const

//	OnAccumulatePropertiesToRender
//
//	Adds properties to the list that we want to render.

	{
	//	We always add radius.

	AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Radius), GetPropertyAnimator(Obj2DProp::Radius), Result);

	//	Fill color

	const IAnimator2D* pAnimator;
	if ((pAnimator = GetPropertyAnimator(Obj2DProp::FillColor)) || !m_FillStyle.IsEmpty())
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::FillColor), pAnimator, Result);

	//	Line style

	const IAnimator2D* pLineColor = GetPropertyAnimator(Obj2DProp::LineColor);
	const IAnimator2D* pLineWidth = GetPropertyAnimator(Obj2DProp::LineWidth);

	if (pLineColor || pLineWidth || !m_OutlineStyle.IsEmpty())
		{
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineColor), pLineColor, Result);
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineWidth), pLineWidth, Result);
		}
	}

TUniquePtr<ILuminousObj2D> CObj2DCircle::OnClone () const
	{
	return TUniquePtr<ILuminousObj2D>(new CObj2DCircle(*this));
	}

const CString& CObj2DCircle::OnGetObjType () const
	{
	return TYPE_CIRCLE;
	}

CLuminousColor CObj2DCircle::OnGetPropertyColor (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::FillColor:
			return m_FillStyle.GetColor();

		case Obj2DProp::LineColor:
			return m_OutlineStyle.GetColor();

		default:
			return CLuminousColor();
		}
	}

double CObj2DCircle::OnGetPropertyScalar (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::Radius:
			return m_rRadius;

		case Obj2DProp::LineWidth:
			return m_OutlineStyle.GetLineWidth();

		default:
			return 0.0;
		}
	}

void CObj2DCircle::OnRead (IByteStream& Stream)
	{
	m_rRadius = Stream.ReadDouble();
	m_FillStyle.Read(Stream);
	m_OutlineStyle.Read(Stream);
	}

void CObj2DCircle::OnWrite (IByteStream& Stream) const
	{
	Stream.Write(m_rRadius);
	m_FillStyle.Write(Stream);
	m_OutlineStyle.Write(Stream);
	}

bool CObj2DCircle::OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value)
	{
	switch (iProp)
		{
		case Obj2DProp::FillColor:
			m_FillStyle.SetColor(Value);
			return true;

		case Obj2DProp::LineColor:
			m_OutlineStyle.SetColor(Value);
			return true;

		default:
			return false;
		}
	}

bool CObj2DCircle::OnSetPropertyScalar (Obj2DProp iProp, double rValue)
	{
	switch (iProp)
		{
		case Obj2DProp::Radius:
			if (rValue >= 0.0)
				{
				m_rRadius = rValue;
				return true;
				}
			else
				return false;

		case Obj2DProp::LineWidth:
			if (rValue >= 0.0)
				{
				m_OutlineStyle.SetLineWidth(rValue);
				return true;
				}
			else
				return false;

		default:
			return false;
		}
	}
