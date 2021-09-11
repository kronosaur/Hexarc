//	CAEONPolygon2D.cpp
//
//	CAEONPolygon2D class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_HOLES,						"holes")
DECLARE_CONST_STRING(FIELD_OUTLINE,						"outline")

DECLARE_CONST_STRING(TYPENAME_POLYGON_2D,				"polygon2D")

const CString &CAEONPolygon2D::StaticGetTypename (void) { return TYPENAME_POLYGON_2D; }

CAEONPolygon2D::CAEONPolygon2D (void)

//	CAEONPolygon2D constructor

	{
	}

CAEONPolygon2D::CAEONPolygon2D (const CPolygon2D &Polygon) : m_Polygon(Polygon)

//	CAEONPolygon2D constructor

	{
	}

CDatum CAEONPolygon2D::CreateFromHandoff (CPolygon2D &Poly)

//	CreateFromHandoff
//
//	Creates a new datum from handoff

	{
	CAEONPolygon2D *pNew = new CAEONPolygon2D;
	pNew->m_Polygon.TakeHandoff(Poly);
	return CDatum(pNew);
	}

CDatum CAEONPolygon2D::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element

	{
	switch (iIndex)
		{
		case 0:
			return CDatum(HolesAsDatum());

		case 1:
			return CDatum(OutlineAsDatum());

		default:
			return CDatum();
		}
	}

CDatum CAEONPolygon2D::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element

	{
	if (strEquals(sKey, FIELD_HOLES))
		return HolesAsDatum();
	else if (strEquals(sKey, FIELD_OUTLINE))
		return OutlineAsDatum();
	else
		return CDatum();
	}

CDatum CAEONPolygon2D::HolesAsDatum (void) const

//	HolesAsDatum
//
//	Returns an array of hole polygons

	{
	int i;

	if (m_Polygon.GetHoleCount() == 0)
		return CDatum();

	CComplexArray *pArray = new CComplexArray;
	pArray->GrowToFit(m_Polygon.GetHoleCount());

	for (i = 0; i < m_Polygon.GetHoleCount(); i++)
		pArray->Append(new CAEONPolygon2D(m_Polygon.GetHole(i)));

	return CDatum(pArray);
	}

CDatum CAEONPolygon2D::OutlineAsDatum (void) const

//	OutlineAsDatum
//
//	Returns an array of vectors

	{
	int i;

	if (m_Polygon.GetVertexCount() == 0)
		return CDatum();

	CComplexArray *pArray = new CComplexArray;
	pArray->GrowToFit(m_Polygon.GetVertexCount());

	for (i = 0; i < m_Polygon.GetVertexCount(); i++)
		pArray->Append(new CAEONVector2D(m_Polygon.GetVertex(i)));

	return CDatum(pArray);
	}

void CAEONPolygon2D::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets element

	{
	}

void CAEONPolygon2D::SetElement (const CString &sKey, CDatum dDatum)

//	SetElement
//
//	Sets element

	{
	}

void CAEONPolygon2D::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	}

size_t CAEONPolygon2D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return 0;	//	Not Yet Implemented
	}

void CAEONPolygon2D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize

	{
	}
