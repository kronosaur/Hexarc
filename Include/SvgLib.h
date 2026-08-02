//	SvgLib.h
//
//	SVG Library Classes
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "AEON.h"
#include "GridLang.h"

class CAEONSVGElement;
class CAEONSVGFilter;
class CAEONSVGGradient;
class CLuminousColorTheme;

class CGLSVGLibrary
	{
	public:
		enum class ESVGType
			{
			Element,
			SVG,
			Group,
			Path,
			Rect,
			Circle,
			Line,
			Polyline,
			Polygon,
			Text,
			};

		static bool Boot ();
		static CDatum AsImageView (CDatum dValue, const CLuminousColorTheme* pTheme = NULL);
		static CDatum CreateElement (ESVGType iType, CDatum dArg0 = CDatum(), CDatum dArg1 = CDatum(), CDatum dArg2 = CDatum(), CDatum dArg3 = CDatum(), CDatum dArg4 = CDatum());
		static TSharedPtr<IASTNode> GetDefinitions ();
		static const CLuminousColorTheme* GetColorTheme (IInvokeCtx& Ctx);
		static SequenceNumber GetSeq (CDatum dValue);
		static DWORD GetTypeID (ESVGType iType);
		static DWORD GetTypeIDFromTag (const CString& sTag);
		static void Register ();

		static DWORD SVG_TYPE;
		static DWORD SVG_ELEMENT_TYPE;
		static DWORD SVG_GROUP_TYPE;
		static DWORD SVG_PATH_TYPE;
		static DWORD SVG_RECT_TYPE;
		static DWORD SVG_CIRCLE_TYPE;
		static DWORD SVG_LINE_TYPE;
		static DWORD SVG_POLYLINE_TYPE;
		static DWORD SVG_POLYGON_TYPE;
		static DWORD SVG_TEXT_TYPE;
		static DWORD SVG_FILTER_TYPE;
		static DWORD SVG_GRADIENT_TYPE;

	private:
		static bool m_bBooted;
		static bool m_bRegistered;
	};
