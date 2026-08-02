//	CAEONTrail2D.cpp
//
//	CAEONTrail2D Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(STR_CAEONTRAIL2_D_INVALID_SCENE_REFERENCE,	"Invalid scene reference.");
DECLARE_CONST_STRING(STR_CAEONTRAIL2_D_INVALID_TRAIL_REFERENCE,	"Invalid trail reference.");

DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY,			"Unknown property: %s.");
DECLARE_CONST_STRING(TYPE_TRAIL,					"trail");
DECLARE_CONST_STRING(TYPENAME_TRAIL2D,				"trail2D");

static CDatum AsPointArray (const TArray<CVector2D>& Points)
	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < Points.GetCount(); i++)
		dResult.Append(CDatum(Points[i]));
	return dResult;
	}

TDatumPropertyHandler<CAEONTrail2D> CAEONTrail2D::m_Properties = {
	{
		"id",
		"?",
		"Returns the trail ID.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			return CDatum((int)Obj.m_dwID);
			},
		NULL,
		},
	{
		"data",
		"?",
		"Gets/sets arbitrary user data on the trail.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			return pScene->GetObjData(Obj.m_dwID);
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			pScene->SetObjData(Obj.m_dwID, dValue);
			return true;
			},
		},
	{
		"lineColor",
		"?",
		"Gets/sets the trail line color.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CAEONLuminous::AsDatum(pObj->GetPropertyColor(Obj2DProp::LineColor));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the trail line width.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::LineWidth));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::LineWidth, dValue);
			},
		},
	{
		"maxPoints",
		"?",
		"Gets/sets the maximum number of points retained in the trail.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::MaxPoints));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::MaxPoints, dValue);
			},
		},
	{
		"opacity",
		"?",
		"Gets/sets the opacity of the trail (0 to 1).",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Opacity));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Opacity, dValue);
			},
		},
	{
		"points",
		"?",
		"Gets/sets the trail points.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return AsPointArray(pObj->GetPropertyVectorQueue(Obj2DProp::Points));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Points, dValue);
			},
		},
	{
		"pos",
		"?",
		"Gets/sets the position of the trail.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::Pos));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Pos, dValue);
			},
		},
	{
		"rotation",
		"?",
		"Gets/sets the rotation of the trail (in radians).",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Rot));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the center of rotation of the trail.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::RotCenter));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the scale of the trail.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyVector(Obj2DProp::Scale));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the visibility of the trail.",
		[](const CAEONTrail2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();

			return CDatum(pObj->GetPropertyBool(Obj2DProp::Visible));
			},
		[](CAEONTrail2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Visible, dValue);
			},
		},
	};

TDatumMethodHandler<CAEONTrail2D> CAEONTrail2D::m_Methods = {
	{
		"addKeyframe",
		"*",
		".addKeyframe(prop, frame, desc) -> true/false",
		0,
		[](CAEONTrail2D &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				{
				retResult.dResult = STR_CAEONTRAIL2_D_INVALID_SCENE_REFERENCE;
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
	{
		"addPoint",
		"*",
		".addPoint(point) -> true",
		0,
		[](CAEONTrail2D &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				{
				retResult.dResult = STR_CAEONTRAIL2_D_INVALID_SCENE_REFERENCE;
				return false;
				}

			ILuminousObj2D *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj || !strEquals(pObj->GetObjType(), TYPE_TRAIL))
				{
				retResult.dResult = STR_CAEONTRAIL2_D_INVALID_TRAIL_REFERENCE;
				return false;
				}

			static_cast<CObj2DTrail *>(pObj)->AddPoint(CVector2D(LocalEnv.GetArgument(1).GetElement(0), LocalEnv.GetArgument(1).GetElement(1)));
			retResult.dResult = CDatum(true);
			return true;
			},
		},
	};

const CString &CAEONTrail2D::StaticGetTypename (void) { return TYPENAME_TRAIL2D; }

CDatum CAEONTrail2D::Create (CDatum dScene, DWORD dwID)
	{
	auto *pScene = CAEONReanimator::Upconvert(dScene);
	if (!pScene)
		throw CException(errFail);

	return CDatum(new CAEONTrail2D(dScene, dwID));
	}

CDatum CAEONTrail2D::GetDatatype () const
	{
	return CAEONTypes::Get(CAEONLuminous::TRAIL2D_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONTrail2D::GetMembers (void)
	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

size_t CAEONTrail2D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	throw CException(errFail);
	}

bool CAEONTrail2D::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)
	{
	return 0;
	}

void CAEONTrail2D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONTrail2D::OnMarked (void)
	{
	m_dScene.Mark();
	}

void CAEONTrail2D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const
	{
	throw CException(errFail);
	}

void CAEONTrail2D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}