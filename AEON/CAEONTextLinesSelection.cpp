//	CAEONTextLinesSelection.cpp
//
//	CAEONTextLinesSelection class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_END_CHAR,					"endChar");
DECLARE_CONST_STRING(FIELD_END_LINE,					"endLine");
DECLARE_CONST_STRING(FIELD_START_CHAR,					"startChar");
DECLARE_CONST_STRING(FIELD_START_LINE,					"startLine");

CAEONTextLinesSelection::CAEONTextLinesSelection (int iLine, int iChar) :
		m_Start({ iLine, iChar })

//	CAEONTextLinesSelection constructor

	{
	if (iLine < 0 || iChar < 0)
		throw CException(errFail);
	}

CAEONTextLinesSelection::CAEONTextLinesSelection (int iStartLine, int iStartChar, int iEndLine, int iEndChar) :
		m_Start({ iStartLine, iStartChar }),
		m_End({ iEndLine, iEndChar })

//	CAEONTextLinesSelection constructor

	{
	if (iStartLine < 0 || iStartChar < 0 || iEndLine < 0 || iEndChar < 0)
		throw CException(errFail);
	}

CAEONTextLinesSelection::CAEONTextLinesSelection (CDatum dValue)

//	CAEONTextLinesSelection constructor

	{
	if (dValue.IsNil())
		{ }
	else if (dValue.GetBasicType() == CDatum::typeArray)
		{
		m_Start.iLine = Max(0, (int)dValue.GetElement(0));
		m_Start.iChar = Max(0, (int)dValue.GetElement(1));

		m_End.iLine = Max(0, (int)dValue.GetElement(2));
		m_End.iChar = Max(0, (int)dValue.GetElement(3));
		}
	else if (dValue.GetBasicType() == CDatum::typeStruct)
		{
		m_Start.iLine = Max(0, (int)dValue.GetElement(FIELD_START_LINE));
		m_Start.iChar = Max(0, (int)dValue.GetElement(FIELD_START_CHAR));

		m_End.iLine = Max(0, (int)dValue.GetElement(FIELD_END_LINE));
		m_End.iChar = Max(0, (int)dValue.GetElement(FIELD_END_CHAR));
		}
	else
		throw CException(errFail);
	}

CDatum CAEONTextLinesSelection::AsAPIDatum () const

//	AsAPIDatum
//
//	Represent as a struct.

	{
	if (IsEmpty())
		return CDatum();

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_START_LINE, m_Start.iLine);
	dResult.SetElement(FIELD_START_CHAR, m_Start.iChar);

	if (IsRange())
		{
		dResult.SetElement(FIELD_END_LINE, m_End.iLine);
		dResult.SetElement(FIELD_END_CHAR, m_End.iChar);
		}

	return dResult;
	}

CDatum CAEONTextLinesSelection::AsDatum () const

//	AsDatum
//
//	Represent as datum.

	{
	if (IsEmpty())
		return CDatum();

	CDatum dResult(CDatum::typeArray);
	dResult.Append(m_Start.iLine);
	dResult.Append(m_Start.iChar);

	if (IsRange())
		{
		dResult.Append(m_End.iLine);
		dResult.Append(m_End.iChar);
		}

	return dResult;
	}
