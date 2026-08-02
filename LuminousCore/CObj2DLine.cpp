//	CObj2DLine.cpp
//
//	CObj2DLine Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_LINE,					"line");
DECLARE_CONST_STRING(FIELD_POINTS,					"points");

void CObj2DLine::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	const IAnimator2D* pPointsAnimator = GetPropertyAnimator(Obj2DProp::LinePoints);
	if (!pPointsAnimator)
		pPointsAnimator = GetPropertyAnimator(Obj2DProp::Points);

	Result.Insert({ Obj2DProp::LinePoints, ObjPropType::VectorList, FIELD_POINTS, pPointsAnimator });

	const IAnimator2D* pLineColor = GetPropertyAnimator(Obj2DProp::LineColor);
	const IAnimator2D* pLineWidth = GetPropertyAnimator(Obj2DProp::LineWidth);
	if (pLineColor || pLineWidth || !m_OutlineStyle.IsEmpty())
		{
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineColor), pLineColor, Result);
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::LineWidth), pLineWidth, Result);
		}
	}

TUniquePtr<ILuminousObj2D> CObj2DLine::OnClone () const
	{
	return TUniquePtr<ILuminousObj2D>(new CObj2DLine(*this));
	}

const CString& CObj2DLine::OnGetObjType () const
	{
	return TYPE_LINE;
	}

CLuminousColor CObj2DLine::OnGetPropertyColor (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::LineColor:
			return m_OutlineStyle.GetColor();

		default:
			return CLuminousColor();
		}
	}

double CObj2DLine::OnGetPropertyScalar (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::LineWidth:
			return m_OutlineStyle.GetLineWidth();

		case Obj2DProp::PointCount:
			return (double)m_Points.GetCount();

		default:
			return 0.0;
		}
	}

const TArray<CVector2D>& CObj2DLine::OnGetPropertyVectorQueue (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::LinePoints:
		case Obj2DProp::Points:
			return m_Points;

		default:
			return GetNullVectorQueueValue();
		}
	}

void CObj2DLine::OnRead (IByteStream& Stream)
	{
	int iCount = Stream.ReadInt();
	m_Points.DeleteAll();
	m_Points.InsertEmpty(iCount);
	for (int i = 0; i < iCount; i++)
		m_Points[i].Read(Stream);

	m_OutlineStyle.Read(Stream);
	}

void CObj2DLine::OnWrite (IByteStream& Stream) const
	{
	Stream.Write(m_Points.GetCount());
	for (int i = 0; i < m_Points.GetCount(); i++)
		m_Points[i].Write(Stream);

	m_OutlineStyle.Write(Stream);
	}

bool CObj2DLine::OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value)
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

bool CObj2DLine::OnSetPropertyScalar (Obj2DProp iProp, double rValue)
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

		case Obj2DProp::PointCount:
			return ((int)rValue == m_Points.GetCount());

		default:
			return false;
		}
	}

bool CObj2DLine::OnSetPropertyVectorQueue (Obj2DProp iProp, const TArray<CVector2D>& Value)
	{
	switch (iProp)
		{
		case Obj2DProp::LinePoints:
		case Obj2DProp::Points:
			if (m_Points.GetCount() != 0 && Value.GetCount() != m_Points.GetCount())
				return false;

			m_Points = Value;
			return true;

		default:
			return false;
		}
	}
