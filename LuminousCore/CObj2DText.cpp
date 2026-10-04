// CObj2DText.cpp
// Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include <cmath>

DECLARE_CONST_STRING(TYPE_TEXT, "text");

const CString& CObj2DText::OnGetObjType () const { return TYPE_TEXT; }

void CObj2DText::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	// Explicit defaults also let updates clear a previous style or width limit.
	static const Obj2DProp PROPS[] = {
		Obj2DProp::Text,
		Obj2DProp::Font,
		Obj2DProp::TextAlign,
		Obj2DProp::TextBaseline,
		Obj2DProp::Direction,
		Obj2DProp::TextFit,
		Obj2DProp::MaxWidth,
		Obj2DProp::MinFontSize,
		Obj2DProp::MaxFontSize,
		Obj2DProp::LineWidth,
		Obj2DProp::ShadowBlur,
		Obj2DProp::ShadowOffsetX,
		Obj2DProp::ShadowOffsetY,
		Obj2DProp::FillColor,
		Obj2DProp::LineColor,
		Obj2DProp::ShadowColor,
		};
	for (auto iProp : PROPS)
		AccumulatePropertyToRender(GetPropertyDesc(iProp), GetPropertyAnimator(iProp), Result);
	}

CString CObj2DText::OnGetPropertyString (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::Text: return m_sText;
		case Obj2DProp::Font: return m_sFont;
		case Obj2DProp::TextAlign: return m_sTextAlign;
		case Obj2DProp::TextBaseline: return m_sTextBaseline;
		case Obj2DProp::Direction: return m_sDirection;
		case Obj2DProp::TextFit: return m_sTextFit;
		default: return NULL_STR;
		}
	}

double CObj2DText::OnGetPropertyScalar (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::MaxWidth: return m_rMaxWidth;
		case Obj2DProp::MinFontSize: return m_rMinFontSize;
		case Obj2DProp::MaxFontSize: return m_rMaxFontSize;
		case Obj2DProp::LineWidth: return m_rLineWidth;
		case Obj2DProp::ShadowBlur: return m_rShadowBlur;
		case Obj2DProp::ShadowOffsetX: return m_rShadowOffsetX;
		case Obj2DProp::ShadowOffsetY: return m_rShadowOffsetY;
		default: return 0.0;
		}
	}

CLuminousColor CObj2DText::OnGetPropertyColor (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::FillColor: return m_FillColor;
		case Obj2DProp::LineColor: return m_LineColor;
		case Obj2DProp::ShadowColor: return m_ShadowColor;
		default: return CLuminousColor();
		}
	}

bool CObj2DText::OnSetPropertyString (Obj2DProp iProp, const CString& sValue)
	{
	switch (iProp)
		{
		case Obj2DProp::Text: m_sText = sValue; return true;
		case Obj2DProp::Font:
			if (sValue.IsEmpty()) return false;
			m_sFont = sValue; return true;
		case Obj2DProp::TextAlign:
			if (strEquals(sValue, CString("start")) || strEquals(sValue, CString("end")) || strEquals(sValue, CString("left")) || strEquals(sValue, CString("right")) || strEquals(sValue, CString("center")))
				{ m_sTextAlign = sValue; return true; }
			return false;
		case Obj2DProp::TextBaseline:
			if (strEquals(sValue, CString("top")) || strEquals(sValue, CString("hanging")) || strEquals(sValue, CString("middle")) || strEquals(sValue, CString("alphabetic")) || strEquals(sValue, CString("ideographic")) || strEquals(sValue, CString("bottom")))
				{ m_sTextBaseline = sValue; return true; }
			return false;
		case Obj2DProp::Direction:
			if (strEquals(sValue, CString("ltr")) || strEquals(sValue, CString("rtl")))
				{ m_sDirection = sValue; return true; }
			return false;
		case Obj2DProp::TextFit:
			if (strEquals(sValue, CString("canvas")) || strEquals(sValue, CString("shrink")) || strEquals(sValue, CString("fit")))
				{ m_sTextFit = sValue; return true; }
			return false;
		default: return false;
		}
	}

bool CObj2DText::OnSetPropertyScalar (Obj2DProp iProp, double rValue)
	{
	if (!std::isfinite(rValue)) return false;
	switch (iProp)
		{
		case Obj2DProp::MaxWidth:
			if (rValue < 0.0 && rValue != -1.0) return false;
			m_rMaxWidth = rValue; return true;
		case Obj2DProp::MinFontSize:
			if (rValue <= 0.0 && rValue != -1.0) return false;
			m_rMinFontSize = rValue; return true;
		case Obj2DProp::MaxFontSize:
			if (rValue <= 0.0 && rValue != -1.0) return false;
			m_rMaxFontSize = rValue; return true;
		case Obj2DProp::LineWidth:
			if (rValue < 0.0) return false;
			m_rLineWidth = rValue; return true;
		case Obj2DProp::ShadowBlur:
			if (rValue < 0.0) return false;
			m_rShadowBlur = rValue; return true;
		case Obj2DProp::ShadowOffsetX:
			m_rShadowOffsetX = rValue; return true;
		case Obj2DProp::ShadowOffsetY:
			m_rShadowOffsetY = rValue; return true;
		default: return false;
		}
	}

bool CObj2DText::OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value)
	{
	switch (iProp)
		{
		case Obj2DProp::FillColor: m_FillColor = Value; return true;
		case Obj2DProp::LineColor: m_LineColor = Value; return true;
		case Obj2DProp::ShadowColor: m_ShadowColor = Value; return true;
		default: return false;
		}
	}

void CObj2DText::OnRead (IByteStream& Stream)
	{
	m_sText = CString::Deserialize(Stream);
	m_sFont = CString::Deserialize(Stream);
	m_sTextAlign = CString::Deserialize(Stream);
	m_sTextBaseline = CString::Deserialize(Stream);
	m_sDirection = CString::Deserialize(Stream);
	m_sTextFit = CString::Deserialize(Stream);
	m_rMaxWidth = Stream.ReadDouble();
	m_rMinFontSize = Stream.ReadDouble();
	m_rMaxFontSize = Stream.ReadDouble();
	m_rLineWidth = Stream.ReadDouble();
	m_rShadowBlur = Stream.ReadDouble();
	m_rShadowOffsetX = Stream.ReadDouble();
	m_rShadowOffsetY = Stream.ReadDouble();
	m_FillColor.Read(Stream);
	m_LineColor.Read(Stream);
	m_ShadowColor.Read(Stream);
	}

void CObj2DText::OnWrite (IByteStream& Stream) const
	{
	m_sText.Serialize(Stream);
	m_sFont.Serialize(Stream);
	m_sTextAlign.Serialize(Stream);
	m_sTextBaseline.Serialize(Stream);
	m_sDirection.Serialize(Stream);
	m_sTextFit.Serialize(Stream);
	Stream.Write(m_rMaxWidth);
	Stream.Write(m_rMinFontSize);
	Stream.Write(m_rMaxFontSize);
	Stream.Write(m_rLineWidth);
	Stream.Write(m_rShadowBlur);
	Stream.Write(m_rShadowOffsetX);
	Stream.Write(m_rShadowOffsetY);
	m_FillColor.Write(Stream);
	m_LineColor.Write(Stream);
	m_ShadowColor.Write(Stream);
	}
