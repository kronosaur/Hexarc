//	CAEONVector3D.cpp
//
//	CAEONVector3D class
//	Copyright (c) 2024 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_INITIALIZE,					"initialize");
DECLARE_CONST_STRING(FIELD_X,							"x");
DECLARE_CONST_STRING(FIELD_Y,							"y");
DECLARE_CONST_STRING(FIELD_Z,							"z");

DECLARE_CONST_STRING(TYPENAME_VECTOR_3D,				"vector3D");

const CString &CAEONVector3D::StaticGetTypename (void) { return TYPENAME_VECTOR_3D; }

TDatumPropertyHandler<CAEONVector3D> CAEONVector3D::m_Properties = {
	{
		"length",
		"I",
		"Returns the length of the vector.",
		[](const CAEONVector3D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Length());
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of elements in the vector.",
		[](const CAEONVector3D& Obj, const CString &sProperty)
			{
			return CDatum(3);
			},
		NULL,
		},
	{
		"unit",
		"$Vector3DOfFloat64",
		"Returns the a unit vector in the same direction.",
		[](const CAEONVector3D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector / Obj.m_vVector.Length());
			},
		NULL,
		},
	{
		"x",
		"f",
		"Returns the x element.",
		[](const CAEONVector3D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.X());
			},
		NULL,
		},
	{
		"y",
		"f",
		"Returns the y element.",
		[](const CAEONVector3D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Y());
			},
		NULL,
		},
	{
		"z",
		"f",
		"Returns the z element.",
		[](const CAEONVector3D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Z());
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONVector3D> CAEONVector3D::m_Methods = {
	{
		"cross",
		"V3:v=V3",
		".cross(v) -> cross product",
		0,
		[](CAEONVector3D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(Obj.m_vVector.Cross(LocalEnv.GetArgument(1)));
			return true;
			},
		},
	{
		"dot",
		"F:v=V3",
		".dot(v) -> dot product",
		0,
		[](CAEONVector3D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(Obj.m_vVector.Dot(LocalEnv.GetArgument(1)));
			return true;
			},
		},
	};

CAEONVector3D::CAEONVector3D (const CVector3D& vVector) : m_vVector(vVector)

//	CAEONVector3D constructor

	{
	}

CString CAEONVector3D::AsString () const
	{
	return strPattern("[%s, %s, %s]", strFromDouble(m_vVector.X()), strFromDouble(m_vVector.Y()), strFromDouble(m_vVector.Z()));
	}

CDatum CAEONVector3D::GetElement (int iIndex) const

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

		case 2:
			return CDatum(m_vVector.Z());

		default:
			return CDatum();
		}
	}

CDatum CAEONVector3D::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element

	{
	if (strEquals(sKey, FIELD_X))
		return CDatum(m_vVector.X());
	else if (strEquals(sKey, FIELD_Y))
		return CDatum(m_vVector.Y());
	else if (strEquals(sKey, FIELD_Z))
		return CDatum(m_vVector.Z());
	else
		return CDatum();
	}

void CAEONVector3D::SetElement (int iIndex, CDatum dDatum)

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

		case 2:
			m_vVector.SetZ(dDatum);
			break;
		}
	}

void CAEONVector3D::SetElement (const CString &sKey, CDatum dDatum)

//	SetElement
//
//	Sets element

	{
	if (strEquals(sKey, FIELD_X))
		m_vVector.SetX(dDatum);
	else if (strEquals(sKey, FIELD_Y))
		m_vVector.SetY(dDatum);
	else if (strEquals(sKey, FIELD_Z))
		m_vVector.SetZ(dDatum);
	}

size_t CAEONVector3D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return 0;	//	Not Yet Implemented
	}

void CAEONVector3D::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	}

void CAEONVector3D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize

	{
	pStruct->SetElement(FIELD_X, m_vVector.X());
	pStruct->SetElement(FIELD_Y, m_vVector.Y());
	pStruct->SetElement(FIELD_Z, m_vVector.Z());
	}

CDatum CAEONVector3D::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CAEONVector3D* pValue = new CAEONVector3D;
	CDatum dValue(pValue);

	pValue->m_vVector.SetX(Stream.ReadDouble());
	pValue->m_vVector.SetY(Stream.ReadDouble());
	pValue->m_vVector.SetZ(Stream.ReadDouble());

	return dValue;
	}

void CAEONVector3D::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_VECTOR_3D);

	Stream.Write(m_vVector.X());
	Stream.Write(m_vVector.Y());
	Stream.Write(m_vVector.Z());
	}
