//	CAEONVectorVector2D.cpp
//
//	CAEONVectorVector2D class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_VECTOR_VECTOR_2D,			"vector_Vector2D");

TDatumPropertyHandler<CAEONVectorVector2D> CAEONVectorVector2D::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			return CAEONTypes::Get(IDatatype::VECTOR_2D_F64);
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			return CAEONTypes::Get(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of elements in the array.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.Append(Obj.GetCount());
			return dResult;
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of elements in the array.",
		[](const CAEONVectorVector2D &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONVectorVector2D::m_pMethodsExt = NULL;

const CString &CAEONVectorVector2D::GetTypename (void) const
	{
	return TYPENAME_VECTOR_VECTOR_2D;
	}

void CAEONVectorVector2D::InsertEmpty (int iCount)
	{
	int iStart = m_Array.GetCount();
	m_Array.InsertEmpty(iCount);
	for (int i = iStart; i < iStart + iCount; i++)
		m_Array[i] = CVector2D();
	}

CDatum CAEONVectorVector2D::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorVector2D *pArray = new CAEONVectorVector2D;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		double x = Stream.ReadDouble();
		double y = Stream.ReadDouble();
		pArray->m_Array.Insert(CVector2D(x, y));
		}

	return dValue;
	}

void CAEONVectorVector2D::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_VECTOR_VECTOR2D))
		return;

	//	Otherwise, write out the full array

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		Stream.Write(m_Array[i].X());
		Stream.Write(m_Array[i].Y());
		}
	}
