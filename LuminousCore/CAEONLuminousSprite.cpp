//	CAEONLuminousSprite.cpp
//
//	CAEONLuminousSprite Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(METHOD_SPRITE_MOVE_TO,			"sprite_moveTo");

DECLARE_CONST_STRING(TYPENAME_LUMINOUS_SPRITE,		"luminousSprite");

TDatumPropertyHandler<CAEONLuminousSprite> CAEONLuminousSprite::m_Properties = {
	{
		"id",
		"Returns the sprite ID.",
		[](const CAEONLuminousSprite &Obj, const CString &sProperty)
			{
			return Obj.m_dwID;
			},
		NULL,
		},
	{
		"pos",
		"Returns the sprite position.",
		[](const CAEONLuminousSprite &Obj, const CString &sProperty)
			{
			return Obj.GetGraphic().GetPos();
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONLuminousSprite> CAEONLuminousSprite::m_Methods = {
	{
		"moveTo",
		"*",
		".moveTo(pos) -> true/null",
		0,
		[](CAEONLuminousSprite& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dArgs(CDatum::typeArray);
			dArgs.Append(Obj.m_dCanvas);
			dArgs.Append(Obj.m_dwID);
			dArgs.Append(dLocalEnv.GetElement(0));

			return Obj.m_dCanvas.InvokeMethodImpl(METHOD_SPRITE_MOVE_TO, Ctx, dArgs, retdResult);
			},
		},
	};

const CString &CAEONLuminousSprite::StaticGetTypename (void) { return TYPENAME_LUMINOUS_SPRITE; }

CDatum CAEONLuminousSprite::Create (CDatum dCanvas, DWORD dwID)

//	Create
//
//	Creates an empty canvas.

	{
	if (!dCanvas.GetCanvasInterface())
		throw CException(errFail);

	return CDatum(new CAEONLuminousSprite(dCanvas, dwID));
	}

const ILuminousGraphic& CAEONLuminousSprite::GetGraphic () const

//	GetGraphic
//
//	Returns the graphic.

	{
	auto pCanvas = m_dCanvas.GetCanvasInterface();
	if (!pCanvas)
		throw CException(errFail);

	ILuminousGraphic* pGraphic = (ILuminousGraphic*)pCanvas->raw_GetGraphicByID(m_dwID);
	if (!pGraphic)
		throw CException(errFail);

	return *pGraphic;
	}

size_t CAEONLuminousSprite::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns the approximate size required for a serialized stream.

	{
	throw CException(errFail);
	}

bool CAEONLuminousSprite::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize from a structure

	{
	return 0;
	}

void CAEONLuminousSprite::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	m_dCanvas.Mark();
	}

void CAEONLuminousSprite::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a structure.

	{
	throw CException(errFail);
	}
