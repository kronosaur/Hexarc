//	CAEONVector2D.cpp
//
//	CAEONVector2D class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_INITIALIZE,					"initialize")
DECLARE_CONST_STRING(FIELD_X,							"x")
DECLARE_CONST_STRING(FIELD_Y,							"y")

DECLARE_CONST_STRING(TYPENAME_VECTOR_2D,				"vector2D")

const CString &CAEONVector2D::StaticGetTypename (void) { return TYPENAME_VECTOR_2D; }

CAEONVector2D::CAEONVector2D (const CVector2D &vVector) : m_vVector(vVector)

//	CAEONVector2D constructor

	{
	}

CDatum CAEONVector2D::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element

	{
	switch (iIndex)
		{
		case 0:
			return CDatum(m_vVector.X());

		case 1:
			return CDatum(m_vVector.Y());

		default:
			return CDatum();
		}
	}

CDatum CAEONVector2D::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element

	{
	if (strEquals(sKey, FIELD_X))
		return CDatum(m_vVector.X());
	else if (strEquals(sKey, FIELD_Y))
		return CDatum(m_vVector.Y());
	else
		return CDatum();
	}

void CAEONVector2D::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets element

	{
	switch (iIndex)
		{
		case 0:
			m_vVector.SetX(dDatum);
			break;

		case 1:
			m_vVector.SetY(dDatum);
			break;
		}
	}

void CAEONVector2D::SetElement (const CString &sKey, CDatum dDatum)

//	SetElement
//
//	Sets element

	{
	if (strEquals(sKey, FIELD_X))
		m_vVector.SetX(dDatum);
	else if (strEquals(sKey, FIELD_Y))
		m_vVector.SetY(dDatum);
	}

size_t CAEONVector2D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return 0;	//	Not Yet Implemented
	}

void CAEONVector2D::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	}

void CAEONVector2D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize

	{
	}
