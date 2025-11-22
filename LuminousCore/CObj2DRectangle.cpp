//	CObj2DRectangle.cpp
//
//	CObj2DRectangle Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_RECTANGLE,				"rectangle");

void CObj2DRectangle::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const

//	OnAccumulatePropertiesToRender
//
//	Adds properties to the list that we want to render.

	{
	//	We always add height and width

	AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Height), GetPropertyAnimator(Obj2DProp::Height), Result);
	AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Width), GetPropertyAnimator(Obj2DProp::Width), Result);

	//	Add corner radius (if we have it).

	const IAnimator2D* pCornerRadiusTL = GetPropertyAnimator(Obj2DProp::CornerRadiusTopLeft);
	const IAnimator2D* pCornerRadiusTR = GetPropertyAnimator(Obj2DProp::CornerRadiusTopRight);
	const IAnimator2D* pCornerRadiusBL = GetPropertyAnimator(Obj2DProp::CornerRadiusBottomLeft);
	const IAnimator2D* pCornerRadiusBR = GetPropertyAnimator(Obj2DProp::CornerRadiusBottomRight);

	if (!m_CornerRadius.IsUniform() || pCornerRadiusTL || pCornerRadiusTR || pCornerRadiusBL || pCornerRadiusBR)
		{
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::CornerRadiusTopLeft), pCornerRadiusTL, Result);
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::CornerRadiusTopRight), pCornerRadiusTR, Result);
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::CornerRadiusBottomLeft), pCornerRadiusBL, Result);
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::CornerRadiusBottomRight), pCornerRadiusBR, Result);
		}
	else if ((pCornerRadiusTL = GetPropertyAnimator(Obj2DProp::CornerRadius)) || m_CornerRadius.GetTopLeft() != 0.0)
		{
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::CornerRadius), pCornerRadiusTL, Result);
		}

	//	Fill color

	const IAnimator2D* pAnimator;
	if ((pAnimator = GetPropertyAnimator(Obj2DProp::FillColor)) || !m_FillStyle.IsEmpty())
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::FillColor), pAnimator, Result);

	//	Fill style

	const IAnimator2D* pLineColor = GetPropertyAnimator(Obj2DProp::LineColor);
	const IAnimator2D* pLineWidth = GetPropertyAnimator(Obj2DProp::LineWidth);

	if (pLineColor || pLineWidth || !m_OutlineStyle.IsEmpty())
		{
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineColor), pLineColor, Result);
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineWidth), pLineWidth, Result);
		}
	}

TUniquePtr<ILuminousObj2D> CObj2DRectangle::OnClone () const
	{
	return TUniquePtr<ILuminousObj2D>(new CObj2DRectangle(*this));
	}

const CString& CObj2DRectangle::OnGetObjType () const
	{
	return TYPE_RECTANGLE;
	}

bool CObj2DRectangle::OnGetPropertyBool (Obj2DProp iProp) const
	{
	return false;
	}

CLuminousColor CObj2DRectangle::OnGetPropertyColor (Obj2DProp iProp) const
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

double CObj2DRectangle::OnGetPropertyScalar (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::CornerRadiusBottomLeft:
			return m_CornerRadius.GetBottomLeft();

		case Obj2DProp::CornerRadiusBottomRight:
			return m_CornerRadius.GetBottomRight();

		case Obj2DProp::CornerRadius:
		case Obj2DProp::CornerRadiusTopLeft:
			return m_CornerRadius.GetTopLeft();

		case Obj2DProp::CornerRadiusTopRight:
			return m_CornerRadius.GetTopRight();

		case Obj2DProp::Height:
			return m_Size.Y();

		case Obj2DProp::LineWidth:
			return m_OutlineStyle.GetLineWidth();

		case Obj2DProp::Width:
			return m_Size.X();

		default:
			return 0.0;
		}
	}

CVector2D CObj2DRectangle::OnGetPropertyVector (Obj2DProp iProp) const
	{
	return CVector2D();
	}

void CObj2DRectangle::OnRead (IByteStream& Stream)
	{
	m_Size.Read(Stream);
	Stream.Read(&m_CornerRadius, sizeof(m_CornerRadius));
	m_FillStyle.Read(Stream);
	m_OutlineStyle.Read(Stream);
	}

void CObj2DRectangle::OnWrite (IByteStream& Stream) const
	{
	m_Size.Write(Stream);
	Stream.Write(&m_CornerRadius, sizeof(m_CornerRadius));
	m_FillStyle.Write(Stream);
	m_OutlineStyle.Write(Stream);
	}

bool CObj2DRectangle::OnSetPropertyBool (Obj2DProp iProp, bool bValue)
	{
	return false;
	}

bool CObj2DRectangle::OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value)
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

bool CObj2DRectangle::OnSetPropertyScalar (Obj2DProp iProp, double rValue)
	{
	switch (iProp)
		{
		case Obj2DProp::CornerRadius:
			{
			if (rValue >= 0.0)
				{
				m_CornerRadius.SetBottomLeft(rValue);
				m_CornerRadius.SetBottomRight(rValue);
				m_CornerRadius.SetTopLeft(rValue);
				m_CornerRadius.SetTopRight(rValue);
				return true;
				}
			else
				return false;
			}

		case Obj2DProp::CornerRadiusBottomLeft:
			if (rValue >= 0.0)
				{
				m_CornerRadius.SetBottomLeft(rValue);
				return true;
				}
			else
				return false;
		
		case Obj2DProp::CornerRadiusBottomRight:
			if (rValue >= 0.0)
				{
				m_CornerRadius.SetBottomRight(rValue);
				return true;
				}
			else
				return false;
		
		case Obj2DProp::CornerRadiusTopLeft:
			if (rValue >= 0.0)
				{
				m_CornerRadius.SetTopLeft(rValue);
				return true;
				}
			else
				return false;
		
		case Obj2DProp::CornerRadiusTopRight:
			if (rValue >= 0.0)
				{
				m_CornerRadius.SetTopRight(rValue);
				return true;
				}
			else
				return false;
		
		case Obj2DProp::Height:
			if (rValue >= 0.0)
				{
				m_Size.SetY(rValue);
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
		
		case Obj2DProp::Width:
			if (rValue >= 0.0)
				{
				m_Size.SetX(rValue);
				return true;
				}
			else
				return false;
		
		default:
			return false;
		}
	}

bool CObj2DRectangle::OnSetPropertyVector (Obj2DProp iProp, const CVector2D& Value)
	{
	return false;
	}

