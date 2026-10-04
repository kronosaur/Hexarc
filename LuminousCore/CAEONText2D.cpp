//	CAEONText2D.cpp
//
//	CAEONText2D Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(STR_CAEONTEXT2_D_INVALID_SCENE_REFERENCE,	"Invalid scene reference.");

DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY,			"Unknown property: %s.");

DECLARE_CONST_STRING(TYPENAME_TEXT2D,				"text2D");

static TDatumPropertyHandler<CAEONText2D>::SDef TextProperty (LPCSTR pName, Obj2DProp iProp)
	{
	return { pName, "?", "Gets/sets a text property; assignments support scene keyframes.",
		[iProp](const CAEONText2D& Obj, const CString& sProperty) -> CDatum
			{
			auto* pScene = CAEONReanimator::Upconvert(Obj.GetScene());
			auto* pObj = pScene ? pScene->FindObj(Obj.GetID()) : NULL;
			if (!pObj) return CDatum();
			switch (ILuminousObj2D::GetPropertyDesc(iProp).iType)
				{
				case ObjPropType::String: return CDatum(pObj->GetPropertyString(iProp));
				case ObjPropType::Scalar:
					{
					double rValue = pObj->GetPropertyScalar(iProp);
					if ((iProp == Obj2DProp::MaxWidth || iProp == Obj2DProp::MinFontSize || iProp == Obj2DProp::MaxFontSize) && rValue < 0.0) return CDatum();
					return CDatum(rValue);
					}
				case ObjPropType::Color: return CAEONLuminous::AsDatum(pObj->GetPropertyColor(iProp));
				case ObjPropType::Vector: return CDatum(pObj->GetPropertyVector(iProp));
				case ObjPropType::Bool: return CDatum(pObj->GetPropertyBool(iProp));
				default: return CDatum();
				}
			},
		[iProp](CAEONText2D& Obj, const CString& sProperty, CDatum dValue, CString* retsError)
			{
			auto* pScene = CAEONReanimator::Upconvert(Obj.GetScene());
			bool bSuccess = pScene && pScene->SetObjProperty(Obj.GetID(), iProp, dValue);
			if (!bSuccess && retsError) *retsError = strPattern("Invalid text property: %s.", sProperty);
			return bSuccess;
			},
		};
	}

TDatumPropertyHandler<CAEONText2D> CAEONText2D::m_Properties = {
	{
		"id",
		"?",
		"Returns the text ID.",
		[](const CAEONText2D &Obj, const CString &sProperty)
			{
			return CDatum((int)Obj.m_dwID);
			},
		NULL,
		},
	{
		"data",
		"?",
		"Gets/sets arbitrary user data on the text.",
		[](const CAEONText2D &Obj, const CString &sProperty)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return CDatum();

			return pScene->GetObjData(Obj.m_dwID);
			},
		[](CAEONText2D &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				return false;

			pScene->SetObjData(Obj.m_dwID, dValue);
			return true;
			},
		},
	TextProperty("pos", Obj2DProp::Pos),
	TextProperty("opacity", Obj2DProp::Opacity),
	TextProperty("rotation", Obj2DProp::Rot),
	TextProperty("rotationCenter", Obj2DProp::RotCenter),
	TextProperty("scale", Obj2DProp::Scale),
	TextProperty("visible", Obj2DProp::Visible),
	TextProperty("text", Obj2DProp::Text),
	TextProperty("font", Obj2DProp::Font),
	TextProperty("textAlign", Obj2DProp::TextAlign),
	TextProperty("textBaseline", Obj2DProp::TextBaseline),
	TextProperty("direction", Obj2DProp::Direction),
	TextProperty("textFit", Obj2DProp::TextFit),
	TextProperty("maxWidth", Obj2DProp::MaxWidth),
	TextProperty("minFontSize", Obj2DProp::MinFontSize),
	TextProperty("maxFontSize", Obj2DProp::MaxFontSize),
	TextProperty("lineWidth", Obj2DProp::LineWidth),
	TextProperty("shadowBlur", Obj2DProp::ShadowBlur),
	TextProperty("shadowOffsetX", Obj2DProp::ShadowOffsetX),
	TextProperty("shadowOffsetY", Obj2DProp::ShadowOffsetY),
	TextProperty("fillColor", Obj2DProp::FillColor),
	TextProperty("lineColor", Obj2DProp::LineColor),
	TextProperty("shadowColor", Obj2DProp::ShadowColor),
	};

TDatumMethodHandler<CAEONText2D> CAEONText2D::m_Methods = {

	{
		"addKeyframe",
		"*",
		".addKeyframe(prop, frame, desc) -> true/false",
		0,
		[](CAEONText2D &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pScene = CAEONReanimator::Upconvert(Obj.m_dScene);
			if (!pScene)
				{
				retResult.iResult = CDatum::InvokeResult::error;
				retResult.dResult = STR_CAEONTEXT2_D_INVALID_SCENE_REFERENCE;
				return false;
				}

			Obj2DProp iProp = ILuminousObj2D::ParseProperty(LocalEnv.GetArgument(1).AsStringView());
			if (iProp == Obj2DProp::Unknown)
				{
				retResult.iResult = CDatum::InvokeResult::error;
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

const CString &CAEONText2D::StaticGetTypename (void) { return TYPENAME_TEXT2D; }

CDatum CAEONText2D::Create (CDatum dScene, DWORD dwID)

//	Create
//
//	Creates a Text2D wrapper.

	{
	auto *pScene = CAEONReanimator::Upconvert(dScene);
	if (!pScene)
		throw CException(errFail);

	return CDatum(new CAEONText2D(dScene, dwID));
	}

CDatum CAEONText2D::GetDatatype () const
	{
	return CAEONTypes::Get(CAEONLuminous::TEXT2D_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONText2D::GetMembers (void)

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

size_t CAEONText2D::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	throw CException(errFail);
	}

bool CAEONText2D::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)
	{
	return 0;
	}

void CAEONText2D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)

//	DeserializeAEONExternal
//
//	Deserializes the scene reference and object ID.

	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONText2D::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	m_dScene.Mark();
	}

void CAEONText2D::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const
	{
	throw CException(errFail);
	}

void CAEONText2D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const

//	SerializeAEONExternal
//
//	Serializes the scene reference and object ID.

	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}
