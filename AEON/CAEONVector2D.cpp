//	CAEONVector2D.cpp
//
//	CAEONVector2D class
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_INITIALIZE,					"initialize");
DECLARE_CONST_STRING(FIELD_X,							"x");
DECLARE_CONST_STRING(FIELD_Y,							"y");

DECLARE_CONST_STRING(TYPENAME_VECTOR_2D,				"vector2D");

const CString &CAEONVector2D::StaticGetTypename (void) { return TYPENAME_VECTOR_2D; }

TDatumPropertyHandler<CAEONVector2D> CAEONVector2D::m_Properties = {
	{
		"length",
		"I",
		"Returns the length of the vector.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Length());
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of elements in the vector.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(2);
			},
		NULL,
		},
	{
		"unit",
		"$Vector2DOfFloat64",
		"Returns the a unit vector in the same direction.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector / Obj.m_vVector.Length());
			},
		NULL,
		},
	{
		"x",
		"f",
		"Returns the x element.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.X());
			},
		NULL,
		},
	{
		"y",
		"f",
		"Returns the y element.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Y());
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONVector2D> CAEONVector2D::m_Methods = {
	{
		"cross",
		"V2:v=V2",
		".cross(v) -> cross product",
		0,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(Obj.m_vVector.Cross(LocalEnv.GetArgument(1)));
			return true;
			},
		},
	{
		"dot",
		"F:v=V2",
		".dot(v) -> dot product",
		0,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(Obj.m_vVector.Dot(LocalEnv.GetArgument(1)));
			return true;
			},
		},
	};

CAEONVector2D::CAEONVector2D (const CVector2D &vVector) : m_vVector(vVector)

//	CAEONVector2D constructor

	{
	}

CString CAEONVector2D::AsString () const
	{
	return strPattern("[%s, %s]", strFromDouble(m_vVector.X()), strFromDouble(m_vVector.Y()));
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
	pStruct->SetElement(FIELD_X, m_vVector.X());
	pStruct->SetElement(FIELD_Y, m_vVector.Y());
	}

CDatum CAEONVector2D::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CAEONVector2D* pValue = new CAEONVector2D;
	CDatum dValue(pValue);

	pValue->m_vVector.SetX(Stream.ReadDouble());
	pValue->m_vVector.SetY(Stream.ReadDouble());

	return dValue;
	}

void CAEONVector2D::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_VECTOR_2D);

	Stream.Write(m_vVector.X());
	Stream.Write(m_vVector.Y());
	}
