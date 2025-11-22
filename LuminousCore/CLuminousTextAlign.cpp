//	CLuminousTextAlign.cpp
//
//	CLuminousTextAlign Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(ALIGN_CENTER,					"center");
DECLARE_CONST_STRING(ALIGN_END,						"end");
DECLARE_CONST_STRING(ALIGN_LEFT,					"left");
DECLARE_CONST_STRING(ALIGN_RIGHT,					"right");
DECLARE_CONST_STRING(ALIGN_START,					"start");

DECLARE_CONST_STRING(BASELINE_ALPHABETIC,			"alphabetic");
DECLARE_CONST_STRING(BASELINE_BOTTOM,				"bottom");
DECLARE_CONST_STRING(BASELINE_HANGING,				"hanging");
DECLARE_CONST_STRING(BASELINE_IDEOGRAPHIC,			"ideographic");
DECLARE_CONST_STRING(BASELINE_MIDDLE,				"middle");
DECLARE_CONST_STRING(BASELINE_TOP,					"top");

DECLARE_CONST_STRING(DIRECTION_DEFAULT,				"default");
DECLARE_CONST_STRING(DIRECTION_LTR,					"LtR");
DECLARE_CONST_STRING(DIRECTION_RTL,					"RtL");

const CLuminousTextAlign CLuminousTextAlign::Null;

CString CLuminousTextAlign::GetAlignAsHTML (EAlign iAlign)

//	GetAlignAsHTML
//
//	Returns the HTML align value.

	{
	switch (iAlign)
		{
		case EAlign::Center:
			return ALIGN_CENTER;

		case EAlign::End:
			return ALIGN_END;

		case EAlign::Left:
			return ALIGN_LEFT;

		case EAlign::Right:
			return ALIGN_RIGHT;

		case EAlign::Start:
			return ALIGN_START;

		default:
			return NULL_STR;
		}
	}

CString CLuminousTextAlign::GetBaselineAsHTML (EBaseline iBaseline)

//	GetBaselineAsHTML
//
//	Returns the HTML baseline value.

	{
	switch (iBaseline)
		{
		case EBaseline::Alphabetic:
			return BASELINE_ALPHABETIC;
			
		case EBaseline::Bottom:
			return BASELINE_BOTTOM;
			
		case EBaseline::Hanging:
			return BASELINE_HANGING;
			
		case EBaseline::Ideographic:
			return BASELINE_IDEOGRAPHIC;
			
		case EBaseline::Middle:
			return BASELINE_MIDDLE;
			
		case EBaseline::Top:
			return BASELINE_TOP;
			
		default:
			return NULL_STR;
		}
	}

CString CLuminousTextAlign::GetDirectionAsHTML (EDirection iDirection)

//	GetDirectionAsHTML
//
//	Returns the HTML direction value.

	{
	switch (iDirection)
		{
		case EDirection::Default:
			return DIRECTION_DEFAULT;

		case EDirection::LtR:
			return DIRECTION_LTR;
						
		case EDirection::RtL:
			return DIRECTION_RTL;
						
		default:
			return NULL_STR;
		}
	}

CLuminousTextAlign::EAlign CLuminousTextAlign::ParseAlign (const CString& sValue)

//	ParseAlign
//
//	Parse an align type.

	{
	if (strEqualsNoCase(sValue, ALIGN_CENTER))
		return EAlign::Center;
	else if (strEqualsNoCase(sValue, ALIGN_END))
		return EAlign::End;
	else if (strEqualsNoCase(sValue, ALIGN_LEFT) || sValue.IsEmpty())
		return EAlign::Left;
	else if (strEqualsNoCase(sValue, ALIGN_RIGHT))
		return EAlign::Right;
	else if (strEqualsNoCase(sValue, ALIGN_START))
		return EAlign::Start;
	else
		return EAlign::Unknown;
	}

CLuminousTextAlign::EBaseline CLuminousTextAlign::ParseBaseline (const CString& sValue)

//	ParseBaseline
//
//	Parse text baseline type.

	{
	if (strEqualsNoCase(sValue, BASELINE_ALPHABETIC) || sValue.IsEmpty())
		return EBaseline::Alphabetic;
	else if (strEqualsNoCase(sValue, BASELINE_BOTTOM))
		return EBaseline::Bottom;
	else if (strEqualsNoCase(sValue, BASELINE_HANGING))
		return EBaseline::Hanging;
	else if (strEqualsNoCase(sValue, BASELINE_IDEOGRAPHIC))
		return EBaseline::Ideographic;
	else if (strEqualsNoCase(sValue, BASELINE_MIDDLE))
		return EBaseline::Middle;
	else if (strEqualsNoCase(sValue, BASELINE_TOP))
		return EBaseline::Top;
	else
		return EBaseline::Unknown;
	}

CLuminousTextAlign::EDirection CLuminousTextAlign::ParseDirection (const CString& sValue)

//	ParseDirection
//
//	Parses direction type.

	{
	if (strEqualsNoCase(sValue, DIRECTION_DEFAULT) || sValue.IsEmpty())
		return EDirection::Default;
	else if (strEqualsNoCase(sValue, DIRECTION_LTR))
		return EDirection::LtR;
	else if (strEqualsNoCase(sValue, DIRECTION_RTL))
		return EDirection::RtL;
	else
		return EDirection::Unknown;
	}
