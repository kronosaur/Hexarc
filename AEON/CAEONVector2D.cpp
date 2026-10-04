//	CAEONVector2D.cpp
//
//	CAEONVector2D class
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ALLOW_NULL,					"allowNull");
DECLARE_CONST_STRING(FIELD_INITIALIZE,					"initialize");
DECLARE_CONST_STRING(FIELD_X,							"x");
DECLARE_CONST_STRING(FIELD_Y,							"y");

DECLARE_CONST_STRING(TYPENAME_VECTOR_2D,				"vector2D");

const CString &CAEONVector2D::StaticGetTypename (void) { return TYPENAME_VECTOR_2D; }

TDatumPropertyHandler<CAEONVector2D> CAEONVector2D::m_Properties = {
	{
		"angle",
		"F",
		"Returns the vector angle in radians.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Polar());
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the vector type.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns the tensor rank (one for both vector types).",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(1);
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the component type (Float64).",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CAEONTypes::Get(IDatatype::FLOAT_64);
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the bounded zero-based index type.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CAEONTypes::CreateInt32SubRange(NULL_STR, 0, Obj.GetCount() - 1);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"F",
		"Returns the length of the vector.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Length());
			},
		NULL,
		},
	{
		"length2",
		"F",
		"Returns the squared length of the vector.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Length2());
			},
		NULL,
		},
	{
		"polar",
		"V2",
		"Returns the vector in polar form as [angle, radius].",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(CVector2D::ToPolar(Obj.m_vVector));
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array containing the component count.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
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
		"Returns the number of elements in the vector.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(2);
			},
		NULL,
		},
	{
		"unit",
		"V2",
		"Returns a unit vector in the same direction.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.Unit());
			},
		NULL,
		},
	{
		"x",
		"F",
		"Returns the x element.",
		[](const CAEONVector2D& Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_vVector.X());
			},
		NULL,
		},
	{
		"y",
		"F",
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
		"average",
		"F:",
		".average() -> arithmetic mean of the components.",
		IInvokeCtx::EXEC_FLAG_CONST,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.MathAverage();
			return true;
			},
		},
	{
		"cross",
		"F:v=V2",
		".cross(v) -> cross product",
		IInvokeCtx::EXEC_FLAG_CONST,
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
		IInvokeCtx::EXEC_FLAG_CONST,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(Obj.m_vVector.Dot(LocalEnv.GetArgument(1)));
			return true;
			},
		},
	{
		"joined",
		"s:|separator=?|separator=?,lastSeparator=?|separator=?,lastSeparator=?,options=?",
		".joined([separator, [lastSeparator], [options]]) -> string.",
		IInvokeCtx::EXEC_FLAG_CONST,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int iArg = 1;
			CString sSeparator = LocalEnv.GetArgument(iArg++).AsString();
			CString sLastSeparator;
			DWORD dwFlags = 0;

			if (LocalEnv.GetArgument(iArg).GetBasicType() == CDatum::typeString)
				sLastSeparator = LocalEnv.GetArgument(iArg++).AsString();

			if (LocalEnv.GetArgument(iArg).GetBasicType() == CDatum::typeStruct)
				{
				CDatum dOptions = LocalEnv.GetArgument(iArg++);
				if (dOptions.GetElement(FIELD_ALLOW_NULL).AsBool())
					dwFlags |= CDatum::FLAG_ALLOW_NULLS;
				}
			else if (sSeparator.IsEmpty() || sSeparator.Find('\n') != -1)
				dwFlags |= CDatum::FLAG_ALLOW_NULLS;

			retResult.dResult = CDatum::raw_AsComplex(&Obj).Join(sSeparator, sLastSeparator, dwFlags);
			return true;
			},
		},
	{
		"max",
		"F:",
		".max() -> largest component.",
		IInvokeCtx::EXEC_FLAG_CONST,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.MathMax();
			return true;
			},
		},
	{
		"min",
		"F:",
		".min() -> smallest component.",
		IInvokeCtx::EXEC_FLAG_CONST,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.MathMin();
			return true;
			},
		},
	{
		"sum",
		"F:",
		".sum() -> sum of the components.",
		IInvokeCtx::EXEC_FLAG_CONST,
		[](CAEONVector2D& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.MathSum();
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

TArray<IDatatype::SMemberDesc> CAEONVector2D::GetMembers ()

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
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
