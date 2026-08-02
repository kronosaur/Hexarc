//	CAEONLine2D.cpp
//
//	CAEONLine2D Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(STR_CAEONLINE2_D_POINTS,	"points");
DECLARE_CONST_STRING(STR_CAEONLINE2_D_INVALID_SCENE_REFERENCE,	"Invalid scene reference.");

DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY,			"Unknown property: %s.");
DECLARE_CONST_STRING(TYPENAME_LINE2D,				"line2D");

static CDatum AsPointArray (const TArray<CVector2D>& Points)
	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < Points.GetCount(); i++)
		dResult.Append(CDatum(Points[i]));
	return dResult;
	}

static Obj2DProp ParseLineProperty (const CString& sProperty)
	{
	if (strEqualsNoCase(sProperty, STR_CAEONLINE2_D_POINTS))
		return Obj2DProp::LinePoints;

	return ILuminousObj2D::ParseProperty(sProperty);
	}

TDatumPropertyHandler<CAEONLine2D> CAEONLine2D::m_Properties = {
	{
		"id",
		"?",
		"Returns the line ID.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			return CDatum((int)Obj.m_dwID);
			},
		NULL,
		},
	{
		"data",
		"?",
		"Gets/sets arbitrary user data on the line.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			return pScene->GetObjData(Obj.m_dwID);
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the line color.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CAEONLuminous::AsDatum(pObj->GetPropertyColor(Obj2DProp::LineColor));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the line width.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CDatum(pObj->GetPropertyScalar(Obj2DProp::LineWidth));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;
			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::LineWidth, dValue);
			},
		},
	{
		"points",
		"?",
		"Gets/sets the line points.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return AsPointArray(pObj->GetPropertyVectorQueue(Obj2DProp::LinePoints));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;
			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::LinePoints, dValue);
			},
		},
	{
		"opacity",
		"?",
		"Gets/sets the opacity of the line.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Opacity));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;
			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Opacity, dValue);
			},
		},
	{
		"pos",
		"?",
		"Gets/sets the position of the line.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CDatum(pObj->GetPropertyVector(Obj2DProp::Pos));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the rotation of the line.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CDatum(pObj->GetPropertyScalar(Obj2DProp::Rot));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the center of rotation of the line.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CDatum(pObj->GetPropertyVector(Obj2DProp::RotCenter));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the scale of the line.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CDatum(pObj->GetPropertyVector(Obj2DProp::Scale));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
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
		"Gets/sets the visibility of the line.",
		[](const CAEONLine2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();
			auto *pObj = pScene->FindObj(Obj.m_dwID);
			if (!pObj)
				return CDatum();
			return CDatum(pObj->GetPropertyBool(Obj2DProp::Visible));
			},
		[](CAEONLine2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;
			return pScene->SetObjProperty(Obj.m_dwID, Obj2DProp::Visible, dValue);
			},
		},
	};

TDatumMethodHandler<CAEONLine2D> CAEONLine2D::m_Methods = {
	{
		"addKeyframe",
		"*",
		".addKeyframe(prop, frame, desc) -> true/false",
		0,
		[](CAEONLine2D &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				{
				retResult.dResult = STR_CAEONLINE2_D_INVALID_SCENE_REFERENCE;
				return false;
				}

			Obj2DProp iProp = ParseLineProperty(LocalEnv.GetArgument(1).AsString());
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

const CString &CAEONLine2D::StaticGetTypename (void) { return TYPENAME_LINE2D; }

CDatum CAEONLine2D::Create (CDatum dScene, DWORD dwID)
	{
	auto *pScene = CAEONReanimator::Upconvert(dScene);
	if (!pScene)
		throw CException(errFail);
	return CDatum(new CAEONLine2D(dScene, dwID));
	}

CDatum CAEONLine2D::GetDatatype () const
	{
	return CAEONTypes::Get(CAEONLuminous::LINE2D_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONLine2D::GetMembers (void)
	{
	TArray<IDatatype::SMemberDesc> Members;
	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);
	return Members;
	}

size_t CAEONLine2D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	throw CException(errFail);
	}

bool CAEONLine2D::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)
	{
	return 0;
	}

void CAEONLine2D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONLine2D::OnMarked (void)
	{
	m_dScene.Mark();
	}

void CAEONLine2D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const
	{
	throw CException(errFail);
	}

void CAEONLine2D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}
