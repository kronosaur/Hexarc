//	CObj2DTrail.cpp
//
//	CObj2DTrail Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_TRAIL,					"trail");

void CObj2DTrail::AddPoint (const CVector2D& vPoint)
	{
	m_Points.Insert(vPoint);

	if (m_iMaxPoints > 0)
		{
		while (m_Points.GetCount() > m_iMaxPoints)
			m_Points.Delete(0);
		}

	if (GetScene().IsStreamMode())
		MarkPropertyDirty(Obj2DProp::Points);
	else
		GetScene().OnObjModified(*this);
	}

void CObj2DTrail::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Points), GetPropertyAnimator(Obj2DProp::Points), Result);

	const IAnimator2D* pLineColor = GetPropertyAnimator(Obj2DProp::LineColor);
	const IAnimator2D* pLineWidth = GetPropertyAnimator(Obj2DProp::LineWidth);
	if (pLineColor || pLineWidth || !m_OutlineStyle.IsEmpty())
		{
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineColor), pLineColor, Result);
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineWidth), pLineWidth, Result);
		}
	}

TUniquePtr<ILuminousObj2D> CObj2DTrail::OnClone () const
	{
	return TUniquePtr<ILuminousObj2D>(new CObj2DTrail(*this));
	}

const CString& CObj2DTrail::OnGetObjType () const
	{
	return TYPE_TRAIL;
	}

CLuminousColor CObj2DTrail::OnGetPropertyColor (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::LineColor:
			return m_OutlineStyle.GetColor();

		default:
			return CLuminousColor();
		}
	}

double CObj2DTrail::OnGetPropertyScalar (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::LineWidth:
			return m_OutlineStyle.GetLineWidth();

		case Obj2DProp::MaxPoints:
			return (double)m_iMaxPoints;

		default:
			return 0.0;
		}
	}

const TArray<CVector2D>& CObj2DTrail::OnGetPropertyVectorQueue (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::Points:
			return m_Points;

		default:
			return GetNullVectorQueueValue();
		}
	}

void CObj2DTrail::OnRead (IByteStream& Stream)
	{
	m_iMaxPoints = Stream.ReadInt();

	int iCount = Stream.ReadInt();
	m_Points.DeleteAll();
	m_Points.InsertEmpty(iCount);
	for (int i = 0; i < iCount; i++)
		m_Points[i].Read(Stream);

	m_OutlineStyle.Read(Stream);
	}

void CObj2DTrail::OnWrite (IByteStream& Stream) const
	{
	Stream.Write(m_iMaxPoints);

	Stream.Write(m_Points.GetCount());
	for (int i = 0; i < m_Points.GetCount(); i++)
		m_Points[i].Write(Stream);

	m_OutlineStyle.Write(Stream);
	}

bool CObj2DTrail::OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value)
	{
	switch (iProp)
		{
		case Obj2DProp::LineColor:
			m_OutlineStyle.SetColor(Value);
			return true;

		default:
			return false;
		}
	}

bool CObj2DTrail::OnSetPropertyScalar (Obj2DProp iProp, double rValue)
	{
	switch (iProp)
		{
		case Obj2DProp::LineWidth:
			if (rValue >= 0.0)
				{
				m_OutlineStyle.SetLineWidth(rValue);
				return true;
				}
			else
				return false;

		case Obj2DProp::MaxPoints:
			if (rValue >= 0.0)
				{
				m_iMaxPoints = (int)rValue;
				while (m_iMaxPoints > 0 && m_Points.GetCount() > m_iMaxPoints)
					m_Points.Delete(0);
				return true;
				}
			else
				return false;

		default:
			return false;
		}
	}

bool CObj2DTrail::OnSetPropertyVectorQueue (Obj2DProp iProp, const TArray<CVector2D>& Value)
	{
	switch (iProp)
		{
		case Obj2DProp::Points:
			m_Points = Value;
			while (m_iMaxPoints > 0 && m_Points.GetCount() > m_iMaxPoints)
				m_Points.Delete(0);
			return true;

		default:
			return false;
		}
	}