//	CDBFormatDesc.cpp
//
//	CDBFormatDesc class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_COLOR,					"color");

DECLARE_CONST_STRING(ERR_INVALID_DESC,				"Invalid format descriptor.");

const CDBFormatDesc CDBFormatDesc::m_Null;

bool CDBFormatDesc::Parse (const CDBValue &Value, CString *retsError)

//	Parse
//
//	Parse from a descriptor.

	{
	switch (Value.GetType())
		{
		case CDBValue::typeStruct:
			return ParseFromStruct(Value, retsError);

		default:
			if (retsError) *retsError = ERR_INVALID_DESC;
			return false;
		}
	}

bool CDBFormatDesc::ParseColor (const CDBValue &Value, CRGBA32 &retColor, CString *retsError)

//	ParseColor
//
//	Parses a color value.

	{
	retColor.Parse(Value);
	return true;
	}

bool CDBFormatDesc::ParseFromStruct (const CDBValue &Value, CString *retsError)

//	ParseFromStruct
//
//	Parse from a structure.

	{
	if (const CDBValue &Color = Value.GetElement(FIELD_COLOR))
		{
		if (!ParseColor(Color, m_rgbColor, retsError))
			return false;

		m_bDefault = false;
		}

	return true;
	}
