//	CAEONRect2D.cpp
//
//	CAEONRect2D Class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(STR_CAEONRECT2_D_INVALID_SCENE_REFERENCE,	"Invalid scene reference.");

DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY,			"Unknown property: %s.");

DECLARE_CONST_STRING(TYPENAME_RECT2D,				"rect2D");

TDatumPropertyHandler<CAEONRect2D> CAEONRect2D::m_Properties = {
	{
		"id",
		"?",
		"Returns the rectangle ID.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			return CDatum((int)Obj.m_dwID);
			},
		NULL,
		},
	{
		"pos",
		"?",
		"Gets/sets the position of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::Pos));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Pos, dValue);
			},
		},
	{
		"width",
		"?",
		"Gets/sets the width of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Width));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Width, dValue);
			},
		},
	{
		"height",
		"?",
		"Gets/sets the height of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Height));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Height, dValue);
			},
		},
	{
		"data",
		"?",
		"Gets/sets arbitrary user data on the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			return pScene->GetObjData(Obj.m_dwID);
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the fill color of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CAEONLuminous::AsDatum(pObj->GetPropertyColor(Obj2DProp::FillColor));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the line color of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CAEONLuminous::AsDatum(pObj->GetPropertyColor(Obj2DProp::LineColor));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the line width of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::LineWidth));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the opacity of the rectangle (0 to 1).",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Opacity));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the rotation of the rectangle (in radians).",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Rot));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the center of rotation of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::RotCenter));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the scale of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::Scale));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the visibility of the rectangle.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyBool(Obj2DProp::Visible));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Visible, dValue);
			},
		},
	{
		"cornerRadius",
		"?",
		"Gets/sets the corner radius of all corners.",
		[](const CAEONRect2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::CornerRadius));
			},
		[](CAEONRect2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::CornerRadius, dValue);
			},
		},
	};

TDatumMethodHandler<CAEONRect2D> CAEONRect2D::m_Methods = {

	{
		"addKeyframe",
		"*",
		".addKeyframe(prop, frame, desc) -> true/false",
		0,
		[](CAEONRect2D &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				{
				retResult.dResult = STR_CAEONRECT2_D_INVALID_SCENE_REFERENCE;
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

const CString &CAEONRect2D::StaticGetTypename (void) { return TYPENAME_RECT2D; }

CDatum CAEONRect2D::Create (CDatum dScene, DWORD dwID)

//	Create
//
//	Creates a Rect2D wrapper.

	{
	auto *pScene = CAEONReanimator::Upconvert(dScene);
	if (!pScene)
		throw CException(errFail);

	return CDatum(new CAEONRect2D(dScene, dwID));
	}

CDatum CAEONRect2D::GetDatatype () const
	{
	return CAEONTypes::Get(CAEONLuminous::RECT2D_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONRect2D::GetMembers (void)

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

size_t CAEONRect2D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	throw CException(errFail);
	}

bool CAEONRect2D::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)
	{
	return 0;
	}

void CAEONRect2D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)

//	DeserializeAEONExternal
//
//	Deserializes the scene reference and object ID.

	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONRect2D::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	m_dScene.Mark();
	}

void CAEONRect2D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const
	{
	throw CException(errFail);
	}

void CAEONRect2D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const

//	SerializeAEONExternal
//
//	Serializes the scene reference and object ID. The serialization system
//	handles link deduplication, so if multiple Rect2D objects reference the
//	same scene, it is only serialized once.

	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}
