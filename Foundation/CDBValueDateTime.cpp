//	CDBValueDateTime.cpp
//
//	CDBValueDateTime class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "DBValueObjectImpl.h"

DECLARE_CONST_STRING(FIELD_YEAR,				"year");
DECLARE_CONST_STRING(FIELD_MONTH,				"month");
DECLARE_CONST_STRING(FIELD_DAY,					"day");
DECLARE_CONST_STRING(FIELD_HOUR,				"hour");
DECLARE_CONST_STRING(FIELD_MINUTE,				"minute");
DECLARE_CONST_STRING(FIELD_SECOND,				"second");
DECLARE_CONST_STRING(FIELD_MILLISECOND,			"millisecond");

CDBValue CDBValueDateTime::GetProperty (const CString &sProperty) const

//	GetElement
//
//	Returns a part of the data time.

	{
	if (strEqualsNoCase(sProperty, FIELD_YEAR))
		return m_Value.Year();
	else if (strEqualsNoCase(sProperty, FIELD_MONTH))
		return m_Value.Month();
	else if (strEqualsNoCase(sProperty, FIELD_DAY))
		return m_Value.Day();
	else if (strEqualsNoCase(sProperty, FIELD_HOUR))
		return m_Value.Hour();
	else if (strEqualsNoCase(sProperty, FIELD_MINUTE))
		return m_Value.Minute();
	else if (strEqualsNoCase(sProperty, FIELD_SECOND))
		return m_Value.Second();
	else if (strEqualsNoCase(sProperty, FIELD_MILLISECOND))
		return m_Value.Millisecond();
	else
		return CDBValue();
	}
