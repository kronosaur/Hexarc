//	CAEONCircle2D.cpp
//
//	CAEONCircle2D Class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(STR_CAEONCIRCLE2_D_INVALID_SCENE_REFERENCE,	"Invalid scene reference.");

DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY,			"Unknown property: %s.");

DECLARE_CONST_STRING(TYPENAME_CIRCLE2D,				"circle2D");

TDatumPropertyHandler<CAEONCircle2D> CAEONCircle2D::m_Properties = {
	{
		"id",
		"?",
		"Returns the circle ID.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			return CDatum((int)Obj.m_dwID);
			},
		NULL,
		},
	{
		"pos",
		"?",
		"Gets/sets the position of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::Pos));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Pos, dValue);
			},
		},
	{
		"radius",
		"?",
		"Gets/sets the radius of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Radius));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Radius, dValue);
			},
		},
	{
		"data",
		"?",
		"Gets/sets arbitrary user data on the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			return pScene->GetObjData(Obj.m_dwID);
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			pScene->SetObjData(Obj.m_dwID, dValue);
			return true;
			},
		},
	{
		"fillColor",
		"?",
		"Gets/sets the fill color of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CAEONLuminous::AsDatum(pObj->GetPropertyColor(Obj2DProp::FillColor));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::FillColor, dValue);
			},
		},
	{
		"lineColor",
		"?",
		"Gets/sets the line color of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CAEONLuminous::AsDatum(pObj->GetPropertyColor(Obj2DProp::LineColor));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::LineColor, dValue);
			},
		},
	{
		"lineWidth",
		"?",
		"Gets/sets the line width of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::LineWidth));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::LineWidth, dValue);
			},
		},
	{
		"opacity",
		"?",
		"Gets/sets the opacity of the circle (0 to 1).",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Opacity));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Opacity, dValue);
			},
		},
	{
		"rotation",
		"?",
		"Gets/sets the rotation of the circle (in radians).",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Rot));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Rot, dValue);
			},
		},
	{
		"rotationCenter",
		"?",
		"Gets/sets the center of rotation of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::RotCenter));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::RotCenter, dValue);
			},
		},
	{
		"scale",
		"?",
		"Gets/sets the scale of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::Scale));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Scale, dValue);
			},
		},
	{
		"visible",
		"?",
		"Gets/sets the visibility of the circle.",
		[](const CAEONCircle2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyBool(Obj2DProp::Visible));
			},
		[](CAEONCircle2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Visible, dValue);
			},
		},
	};

TDatumMethodHandler<CAEONCircle2D> CAEONCircle2D::m_Methods = {

	{
		"addKeyframe",
		"*",
		".addKeyframe(prop, frame, desc) -> true/false",
		0,
		[](CAEONCircle2D &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				{
				retResult.dResult = STR_CAEONCIRCLE2_D_INVALID_SCENE_REFERENCE;
				return false;
				}

			Obj2DProp iProp = ILuminousObj2D::ParseProperty(LocalEnv.GetArgument(1).AsStringView());
			if (iProp == Obj2DProp::Unknown)
				{
				retResult.dResult = strPattern(ERR_UNKNOWN_PROPERTY, LocalEnv.GetArgument(1).AsString());
				return false;
				}

			int iFrame = LocalEnv.GetArgument(2);
			CDatum dDesc = LocalEnv.GetArgument(3);
			retResult.dResult = pScene->AnimateObjProperty(Obj.m_dwID, iProp, iFrame, dDesc);
			return true;
			},
		},
	};

const CString &CAEONCircle2D::StaticGetTypename (void) { return TYPENAME_CIRCLE2D; }

CDatum CAEONCircle2D::Create (CDatum dScene, DWORD dwID)

//	Create
//
//	Creates a Circle2D wrapper.

	{
	auto *pScene = CAEONReanimator::Upconvert(dScene);
	if (!pScene)
		throw CException(errFail);

	return CDatum(new CAEONCircle2D(dScene, dwID));
	}

CDatum CAEONCircle2D::GetDatatype () const
	{
	return CAEONTypes::Get(CAEONLuminous::CIRCLE2D_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONCircle2D::GetMembers (void)

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

size_t CAEONCircle2D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	throw CException(errFail);
	}

bool CAEONCircle2D::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)
	{
	return 0;
	}

void CAEONCircle2D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)

//	DeserializeAEONExternal
//
//	Deserializes the scene reference and object ID.

	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONCircle2D::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	m_dScene.Mark();
	}

void CAEONCircle2D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const
	{
	throw CException(errFail);
	}

void CAEONCircle2D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const

//	SerializeAEONExternal
//
//	Serializes the scene reference and object ID.

	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}
