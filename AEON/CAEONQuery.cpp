//	CAEONQuery.cpp
//
//	CAEONQuery classes
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LEFT,						"left");
DECLARE_CONST_STRING(FIELD_OP,							"op");
DECLARE_CONST_STRING(FIELD_RIGHT,						"right");

DECLARE_CONST_STRING(OP_AND,							"and");
DECLARE_CONST_STRING(OP_EQUAL_TO,						"=");
DECLARE_CONST_STRING(OP_FALSE,							"false");
DECLARE_CONST_STRING(OP_FIELD,							"field");
DECLARE_CONST_STRING(OP_GREATER_THAN,					">");
DECLARE_CONST_STRING(OP_GREATER_THAN_OR_EQUAL_TO,		">=");
DECLARE_CONST_STRING(OP_IN,								"in");
DECLARE_CONST_STRING(OP_LESS_THAN,						"<");
DECLARE_CONST_STRING(OP_LESS_THAN_OR_EQUAL_TO,			"<=");
DECLARE_CONST_STRING(OP_LITERAL,						"literal");
DECLARE_CONST_STRING(OP_OR,								"or");
DECLARE_CONST_STRING(OP_TRUE,							"true");

DECLARE_CONST_STRING(TYPENAME_QUERY,					"query");

const CString& CAEONQuery::GetTypename (void) const { return TYPENAME_QUERY; }
	
//	Constructors ---------------------------------------------------------------

CDatum CAEONQuery::BinaryOp (EOp iOp, CDatum dLeft, CDatum dRight)
	{
	switch (iOp)
		{
		case EOp::And:
		case EOp::EqualTo:
		case EOp::GreaterThan:
		case EOp::GreaterThanOrEqualTo:
		case EOp::In:
		case EOp::LessThan:
		case EOp::LessThanOrEqualTo:
		case EOp::Or:
			{
			auto *pQuery = new CAEONQuery;
			pQuery->m_iOp = iOp;
			pQuery->m_dLeft = dLeft;
			pQuery->m_dRight = dRight;

			return CDatum(pQuery);
			}

		default:
			throw CException(errFail);
		}
	}

//	Implementation -------------------------------------------------------------

CString CAEONQuery::AsID (EOp iOp)

//	AsID
//
//	Represent an op as an ID.

	{
	switch (iOp)
		{
		case EOp::None:
			return NULL_STR;

		case EOp::And:
			return OP_AND;

		case EOp::EqualTo:
			return OP_EQUAL_TO;

		case EOp::False:
			return OP_FALSE;

		case EOp::GreaterThan:
			return OP_GREATER_THAN;

		case EOp::GreaterThanOrEqualTo:
			return OP_GREATER_THAN_OR_EQUAL_TO;

		case EOp::In:
			return OP_IN;

		case EOp::LessThan:
			return OP_LESS_THAN;

		case EOp::LessThanOrEqualTo:
			return OP_LESS_THAN_OR_EQUAL_TO;

		case EOp::Literal:
			return OP_LITERAL;

		case EOp::Or:
			return OP_OR;

		case EOp::True:
			return OP_TRUE;

		default:
			throw CException(errFail);
		}
	}

CAEONQuery::EOp CAEONQuery::AsOp (const CString& sValue)

//	AsOp
//
//	Parse an op code

	{
	if (strEqualsNoCase(sValue, OP_AND))
		return EOp::And;
	else if (strEqualsNoCase(sValue, OP_EQUAL_TO))
		return EOp::EqualTo;
	else if (strEqualsNoCase(sValue, OP_FALSE))
		return EOp::False;
	else if (strEqualsNoCase(sValue, OP_GREATER_THAN))
		return EOp::GreaterThan;
	else if (strEqualsNoCase(sValue, OP_GREATER_THAN_OR_EQUAL_TO))
		return EOp::GreaterThanOrEqualTo;
	else if (strEqualsNoCase(sValue, OP_IN))
		return EOp::In;
	else if (strEqualsNoCase(sValue, OP_LESS_THAN))
		return EOp::LessThan;
	else if (strEqualsNoCase(sValue, OP_LESS_THAN_OR_EQUAL_TO))
		return EOp::LessThanOrEqualTo;
	else if (strEqualsNoCase(sValue, OP_LITERAL))
		return EOp::Literal;
	else if (strEqualsNoCase(sValue, OP_OR))
		return EOp::Or;
	else if (strEqualsNoCase(sValue, OP_TRUE))
		return EOp::True;
	else
		return EOp::None;
	}

CString CAEONQuery::AsString (void) const

//	AsString
//
//	Represent the query as a string.

	{
	//	LATER
	return CString("Query");
	}

size_t CAEONQuery::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Returns the memory size.

	{
	size_t iSize = 0;

	iSize += 10;	//	Rough constant for op.
	iSize += m_dLeft.CalcMemorySize();
	iSize += m_dRight.CalcMemorySize();

	return iSize;
	}

IComplexDatum* CAEONQuery::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Creates a clone.

	{
	auto *pClone = new CAEONQuery;
	pClone->m_iOp = m_iOp;
	pClone->m_dLeft = m_dLeft.Clone(iMode);
	pClone->m_dRight = m_dRight.Clone(iMode);

	return pClone;
	}

size_t CAEONQuery::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Compute a rough size.

	{
	return CalcMemorySize();
	}

bool CAEONQuery::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize.

	{
	m_iOp = AsOp(dStruct.GetElement(FIELD_OP));
	if (m_iOp == EOp::None)
		return false;

	switch (m_iOp)
		{
		case EOp::False:
		case EOp::True:
			break;

		case EOp::Literal:
			m_dLeft = dStruct.GetElement(FIELD_LEFT);
			break;

		case EOp::And:
		case EOp::EqualTo:
		case EOp::GreaterThan:
		case EOp::GreaterThanOrEqualTo:
		case EOp::In:
		case EOp::LessThan:
		case EOp::LessThanOrEqualTo:
		case EOp::Or:
			m_dLeft = dStruct.GetElement(FIELD_LEFT);
			m_dRight = dStruct.GetElement(FIELD_RIGHT);
			break;

		default:
			break;
		}

	return true;
	}

void CAEONQuery::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a struct.

	{
	pStruct->SetElement(FIELD_OP, AsID(m_iOp));

	switch (m_iOp)
		{
		case EOp::False:
		case EOp::True:
			break;

		case EOp::Literal:
			pStruct->SetElement(FIELD_LEFT, m_dLeft);
			break;

		case EOp::And:
		case EOp::EqualTo:
		case EOp::GreaterThan:
		case EOp::GreaterThanOrEqualTo:
		case EOp::In:
		case EOp::LessThan:
		case EOp::LessThanOrEqualTo:
		case EOp::Or:
			pStruct->SetElement(FIELD_LEFT, m_dLeft);
			pStruct->SetElement(FIELD_RIGHT, m_dRight);
			break;

		default:
			break;
		}
	}
