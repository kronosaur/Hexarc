//	CHyperionOptions.cpp
//
//	CHyperionOptions class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LOG_SESSION_STATE,			"Hyperion.logSessionState")

DECLARE_CONST_STRING(VALUE_DISABLED,					"disabled")
DECLARE_CONST_STRING(VALUE_ENABLED,						"enabled")

DECLARE_CONST_STRING(ERR_INVALID_VALUE,					"Invalid value %s for option %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_OPTION,				"Unknown option: %s.")

bool CHyperionOptions::GetOptionBoolean (EOptions iOption) const

//	GetOptionBoolean
//
//	Returns a boolean option.

	{
	CSmartLock Lock(m_cs);

	switch (iOption)
		{
		case optionLogSessionState:
			return m_bLogSessionState;

		default:
			return false;
		}
	}

CDatum CHyperionOptions::GetStatus (void) const

//	GetStatus
//
//	Returns a status for each option.

	{
	CDatum dResult(CDatum::typeStruct);

	CDatum dValue = (m_bLogSessionState ? CDatum(VALUE_ENABLED) : CDatum(VALUE_DISABLED));
	dResult.SetElement(FIELD_LOG_SESSION_STATE, dValue);

	return dResult;
	}

bool CHyperionOptions::ParseBooleanValue (const CString &sOption, CDatum dValue, bool *retbValue, CString *retsError)

//	ParseBooleanValue
//
//	Parses a boolean value. Returns FALSE if we get an error.

	{
	if (dValue.IsNil())
		*retbValue = false;
	else if (dValue.GetBasicType() == CDatum::typeString)
		{
		const CString &sValue = dValue;

		if (strEqualsNoCase(sValue, VALUE_DISABLED))
			*retbValue = false;
		else if (strEqualsNoCase(sValue, VALUE_ENABLED))
			*retbValue = true;
		else
			{
			if (retsError) *retsError = strPattern(ERR_INVALID_VALUE, sValue, sOption);
			return false;
			}
		}
	else
		*retbValue = true;

	//	Success

	return true;
	}

bool CHyperionOptions::SetOption (const CString &sOption, CDatum dValue, CString *retsError)

//	SetOption
//
//	Sets the given option. Returns FALSE if we cannot set the option.

	{
	if (strEqualsNoCase(sOption, FIELD_LOG_SESSION_STATE))
		{
		if (!ParseBooleanValue(sOption, dValue, &m_bLogSessionState, retsError))
			return false;
		}
	else
		{
		if (retsError) *retsError = strPattern(ERR_UNKNOWN_OPTION, sOption);
		return false;
		}

	return true;
	}

void CHyperionOptions::SetOptionBoolean (EOptions iOption, bool bValue)

//	SetOptionBoolean
//
//	Sets the given option.

	{
	CSmartLock Lock(m_cs);

	switch (iOption)
		{
		case optionLogSessionState:
			m_bLogSessionState = bValue;
			break;

		default:
			ASSERT(false);
		}
	}
