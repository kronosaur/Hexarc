//	CAEONLuminousCanvas.cpp
//
//	CAEONLuminousCanvas Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(FIELD_ALIGN,					"align");
DECLARE_CONST_STRING(FIELD_BASELINE,				"baseline");
DECLARE_CONST_STRING(FIELD_BLUR,					"blur");
DECLARE_CONST_STRING(FIELD_COLOR,					"color");
DECLARE_CONST_STRING(FIELD_DIRECTION,				"direction");
DECLARE_CONST_STRING(FIELD_FILL_STYLE,				"fillStyle");
DECLARE_CONST_STRING(FIELD_FONT_STYLE,				"fontStyle");
DECLARE_CONST_STRING(FIELD_IMAGE,					"image");
DECLARE_CONST_STRING(FIELD_LINE_CAP,				"lineCap");
DECLARE_CONST_STRING(FIELD_LINE_JOIN,				"lineJoin");
DECLARE_CONST_STRING(FIELD_LINE_HEIGHT,				"lineHeight");
DECLARE_CONST_STRING(FIELD_LINE_STYLE,				"lineStyle");
DECLARE_CONST_STRING(FIELD_LINE_WIDTH,				"lineWidth");
DECLARE_CONST_STRING(FIELD_LR,						"lr");
DECLARE_CONST_STRING(FIELD_MITER_LIMIT,				"miterLimit");
DECLARE_CONST_STRING(FIELD_OFFSET_XY,				"offset");
DECLARE_CONST_STRING(FIELD_PATH,					"path");
DECLARE_CONST_STRING(FIELD_POS,						"pos");
DECLARE_CONST_STRING(FIELD_POS_X,					"posX");
DECLARE_CONST_STRING(FIELD_POS_Y,					"posY");
DECLARE_CONST_STRING(FIELD_RESOURCE_ID,				"resourceID");
DECLARE_CONST_STRING(FIELD_SEQ,						"seq");
DECLARE_CONST_STRING(FIELD_SHADOW_STYLE,			"shadowStyle");
DECLARE_CONST_STRING(FIELD_SIZE,					"size");
DECLARE_CONST_STRING(FIELD_STRETCH,					"stretch");
DECLARE_CONST_STRING(FIELD_STYLE,					"style");
DECLARE_CONST_STRING(FIELD_TEXT,					"text");
DECLARE_CONST_STRING(FIELD_TEXT_ALIGN,				"textAlign");
DECLARE_CONST_STRING(FIELD_TYPE,					"type");
DECLARE_CONST_STRING(FIELD_TYPEFACE,				"typeface");
DECLARE_CONST_STRING(FIELD_UL,						"ul");
DECLARE_CONST_STRING(FIELD_WEIGHT,					"weight");

DECLARE_CONST_STRING(TYPE_ARC,						"arc");
DECLARE_CONST_STRING(TYPE_ARC_TO,					"arcTo");
DECLARE_CONST_STRING(TYPE_BEGIN_UPDATE,				"beginUpdate");
DECLARE_CONST_STRING(TYPE_CLEAR_RECT,				"clearRect");
DECLARE_CONST_STRING(TYPE_CLOSE_PATH,				"closePath");
DECLARE_CONST_STRING(TYPE_IMAGE,					"image");
DECLARE_CONST_STRING(TYPE_LINE_TO,					"lineTo");
DECLARE_CONST_STRING(TYPE_MOVE_TO,					"moveTo");
DECLARE_CONST_STRING(TYPE_RECT,						"rect");
DECLARE_CONST_STRING(TYPE_SET_RESOURCE,				"setResource");
DECLARE_CONST_STRING(TYPE_SHAPE,					"shape");
DECLARE_CONST_STRING(TYPE_SOLID,					"solid");
DECLARE_CONST_STRING(TYPE_TEXT,						"text");

DECLARE_CONST_STRING(TYPENAME_LUMINOUS_CANVAS,		"luminousCanvas");

DECLARE_CONST_STRING(ERR_INVALID_TEXT_ALIGN,		"Invalid textAlign value: %s.");
DECLARE_CONST_STRING(ERR_INVALID_TEXT_BASELINE,		"Invalid textBaseline value: %s.");
DECLARE_CONST_STRING(ERR_INVALID_SPRITE_DESC,		"Invalid sprite descriptor.");
DECLARE_CONST_STRING(ERR_INVALID_RESOURCE_NAME,		"Invalid resource name.");
DECLARE_CONST_STRING(ERR_RESOURCE_NOT_FOUND,		"Unable to find canvas resource: %s.");

TDatumPropertyHandler<CAEONLuminousCanvas> CAEONLuminousCanvas::m_Properties = {
	{
		"fillStyle",
		"?",
		"The fill style for future shapes.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(CAEONLuminous::AsDatum(Obj.m_DrawCtx.Fill().GetColor()));
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_DrawCtx.Fill().SetColor(CLuminousColor(CRGBA32::Parse(dValue.AsString())));
			return true;
			},
		},
	{
		"fontStyle",
		"?",
		"The font style to use for future text draw.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_DrawCtx.Font().AsHTML());
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_DrawCtx.Font() = CAEONLuminous::AsFontStyle(dValue);
			return true;
			},
		},
	{
		"length",
		"?",
		"Returns the number of rows in the table.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(1);
			},
		NULL,
		},
	{
		"lineStyle",
		"?",
		"The line style for future shapes.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(CAEONLuminous::AsDatum(Obj.m_DrawCtx.Line().GetColor()));
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_DrawCtx.Line().SetColor(CLuminousColor(CRGBA32::Parse(dValue.AsString())));
			return true;
			},
		},
	{
		"lineWidth",
		"?",
		"The line width for future shapes.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_DrawCtx.Line().GetLineWidth());
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_DrawCtx.Line().SetLineWidth(dValue);
			return true;
			},
		},
	{
		"seq",
		"?",
		"Returns the current sequence number.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_Seq);
			},
		NULL,
		},
	{
		"textAlign",
		"?",
		"The horizontal alignment for future text draw.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_DrawCtx.TextAlign().GetAlignAsHTML());
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			CLuminousTextAlign::EAlign iAlign = CLuminousTextAlign::ParseAlign(dValue.AsStringView());
			if (iAlign == CLuminousTextAlign::EAlign::Unknown)
				{
				if (retsError) *retsError = strPattern(ERR_INVALID_TEXT_ALIGN, dValue.AsString());
				return false;
				}

			Obj.m_DrawCtx.TextAlign().SetAlign(iAlign);
			return true;
			},
		},
	{
		"textBaseline",
		"?",
		"The vertical alignment for future text draw.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_DrawCtx.TextAlign().GetBaselineAsHTML());
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			CLuminousTextAlign::EBaseline iBaseline = CLuminousTextAlign::ParseBaseline(dValue.AsStringView());
			if (iBaseline == CLuminousTextAlign::EBaseline::Unknown)
				{
				if (retsError) *retsError = strPattern(ERR_INVALID_TEXT_BASELINE, dValue.AsString());
				return false;
				}

			Obj.m_DrawCtx.TextAlign().SetBaseline(iBaseline);
			return true;
			},
		},
	};

TDatumMethodHandler<CAEONLuminousCanvas> CAEONLuminousCanvas::m_Methods = {
	{
		"arc",
		"*",
		".arc(x, y, radius, startAngle, endAngle[, counterclockwise]) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			double x = LocalEnv.GetArgument(1);
			double y = LocalEnv.GetArgument(2);
			double radius = LocalEnv.GetArgument(3);
			double startAngle = LocalEnv.GetArgument(4);
			double endAngle = LocalEnv.GetArgument(5);
			bool bCounterclock = !LocalEnv.GetArgument(6).IsNil();

			Obj.m_DrawCtx.Path().Arc(CVector2D(x, y), radius, startAngle, endAngle, bCounterclock);

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"beginPath",
		"*",
		".beginPath() -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			Obj.m_DrawCtx.Path().DeleteAll();

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"circle",
		"*",
		".circle(x, y, radius) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			double x = LocalEnv.GetArgument(1);
			double y = LocalEnv.GetArgument(2);
			double radius = LocalEnv.GetArgument(3);

			Obj.m_DrawCtx.Path().Arc(CVector2D(x, y), radius, 0.0, TAU, true);

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"clearRect",
		"*",
		".clearRect() -> true/false\n"
		".clearRect(x, y, width, height) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			Obj.OnModify();

			CDatum dCmdData;
			if (LocalEnv.GetCount() == 1)
				Obj.m_Model.DeleteAll();
			else
				{
				double x = LocalEnv.GetArgument(1);
				double y = LocalEnv.GetArgument(2);
				double width = LocalEnv.GetArgument(3);
				double height = LocalEnv.GetArgument(4);
				if (width <= 0.0 || height <= 0.0)
					{
					retResult.dResult = CDatum(false);
					return true;
					}

				auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateClearRect(CVector2D(x, y), CVector2D(x + width, y + height)));
				Graphic.SetSeq(Obj.GetSeq());
				}

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"closePath",
		"*",
		".closePath() -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			Obj.m_DrawCtx.Path().ClosePath();

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"createSprite",
		"*",
		".createSprite(desc, [options]) -> sprite",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dDesc = LocalEnv.GetArgument(1);
			CDatum dOptions = LocalEnv.GetArgument(2);
			CDatum dSprite = Obj.CreateSprite(dDesc, dOptions);
			if (dSprite.IsNil())
				{
				retResult.dResult = ERR_INVALID_SPRITE_DESC;
				return false;
				}

			retResult.dResult = dSprite;
			return true;
			},
		},
	{
		"drawImage",
		"*",
		".drawImage(image, x, y) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dImage = LocalEnv.GetArgument(1);
			double x = LocalEnv.GetArgument(2);
			double y = LocalEnv.GetArgument(3);

			Obj.OnModify();

			if (!((const CRGBA32Image&)dImage).IsEmpty())
				{
				auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateImage(dImage));
				Graphic.SetSeq(Obj.GetSeq());
				Graphic.SetPos(CVector2D(x, y));

				Obj.m_Resources.AddResource(Graphic, dImage);

				retResult.dResult = CDatum(true);
				return true;
				}
			else
				{
				CStringView sResourceID = dImage;
				if (Obj.m_Resources.FindNamedResource(sResourceID) == -1)
					{
					retResult.dResult = strPattern(ERR_RESOURCE_NOT_FOUND, sResourceID);
					return false;
					}

				auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateResource(sResourceID));
				Graphic.SetSeq(Obj.GetSeq());
				Graphic.SetPos(CVector2D(x, y));

				retResult.dResult = CDatum(true);
				return true;
				}
			},
		},
	{
		"fill",
		"*",
		".fill() -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.FillStyle = Obj.m_DrawCtx.Fill();

			Obj.OnModify();
			auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));
			Graphic.SetSeq(Obj.GetSeq());

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"fillRect",
		"*",
		".fillRect(x, y, width, height) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			double x = LocalEnv.GetArgument(1);
			double y = LocalEnv.GetArgument(2);
			double cx = LocalEnv.GetArgument(3);
			double cy = LocalEnv.GetArgument(4);

			Obj.OnModify();

			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path.Rect(CVector2D(x, y), CVector2D(x + cx, y + cy));
			Options.FillStyle = Obj.m_DrawCtx.Fill();
			auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));
			Graphic.SetSeq(Obj.GetSeq());

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"fillText",
		"*",
		".fillText(text, x, y) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CString sText = LocalEnv.GetArgument(1).AsString();
			double x = LocalEnv.GetArgument(2);
			double y = LocalEnv.GetArgument(3);

			Obj.OnModify();

			CLuminousCanvasModel::STextOptions Options;
			Options.sText = sText;
			Options.Pos = CVector2D(x, y);
			Options.AlignStyle = Obj.m_DrawCtx.TextAlign();
			Options.FontStyle = Obj.m_DrawCtx.Font();
			Options.FillStyle = Obj.m_DrawCtx.Fill();
			auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateText(Options));
			Graphic.SetSeq(Obj.GetSeq());

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"lineTo",
		"*",
		".lineTo(x, y) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			double x = LocalEnv.GetArgument(1);
			double y = LocalEnv.GetArgument(2);

			Obj.m_DrawCtx.Path().LineTo(CVector2D(x, y));

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"moveTo",
		"*",
		".moveTo(x, y) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			double x = LocalEnv.GetArgument(1);
			double y = LocalEnv.GetArgument(2);

			Obj.m_DrawCtx.Path().MoveTo(CVector2D(x, y));

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"outline",
		"*",
		".outline() -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.LineStyle = Obj.m_DrawCtx.Line();

			Obj.OnModify();

			auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));
			Graphic.SetSeq(Obj.GetSeq());

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"outlineText",
		"*",
		".outlineText(text, x, y) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sText = LocalEnv.GetArgument(1);
			double x = LocalEnv.GetArgument(2);
			double y = LocalEnv.GetArgument(3);

			Obj.OnModify();

			CLuminousCanvasModel::STextOptions Options;
			Options.sText = sText;
			Options.Pos = CVector2D(x, y);
			Options.AlignStyle = Obj.m_DrawCtx.TextAlign();
			Options.FontStyle = Obj.m_DrawCtx.Font();
			Options.LineStyle = Obj.m_DrawCtx.Line();
			auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateText(Options));
			Graphic.SetSeq(Obj.GetSeq());

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"rect",
		"*",
		".rect(x, y, width, height) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			double x = LocalEnv.GetArgument(1);
			double y = LocalEnv.GetArgument(2);
			double cx = LocalEnv.GetArgument(3);
			double cy = LocalEnv.GetArgument(4);

			Obj.m_DrawCtx.Path().Rect(CVector2D(x, y), CVector2D(x + cx, y + cy));

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"setFillStyle",
		"*",
		"DEPRECATED: Use .fillStyle property instead.",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dStyle = LocalEnv.GetArgument(1);

			Obj.m_DrawCtx.Fill().SetColor(CLuminousColor(CRGBA32::Parse(dStyle.AsString())));

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"setLineStyle",
		"*",
		"DEPRECATED: Use .lineStyle property instead.",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dStyle = LocalEnv.GetArgument(1);

			Obj.m_DrawCtx.Line().SetColor(CLuminousColor(CRGBA32::Parse(dStyle.AsString())));

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"setResource",
		"*",
		".setResource(name, image) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sName = LocalEnv.GetArgument(1);
			CDatum dResource = LocalEnv.GetArgument(2);

			if (sName.IsEmpty())
				{
				retResult.dResult = ERR_INVALID_RESOURCE_NAME;
				return false;
				}

			Obj.m_Resources.AddNamedResource(sName, dResource);

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"sprite_moveTo",
		"*",
		".sprite_moveTo(spriteID, pos) -> true/false",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			DWORD dwID = LocalEnv.GetArgument(1);
			CVector2D vPos = LocalEnv.GetArgument(2);

			auto& Graphic = Obj.m_Model.GetGraphicByID(dwID);
			Graphic.SetPos(vPos);

			Obj.OnModify();

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"renderHTMLCanvasCommands",
		"*",
		".renderHTMLCanvasCommands() -> desc",
		0,
		[](CAEONLuminousCanvas &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dSeq = LocalEnv.GetArgument(1);
			retResult.dResult = Obj.RenderAsHTMLCanvasCommands((DWORDLONG)dSeq);
			return true;
			},
		},
	};

const CString &CAEONLuminousCanvas::StaticGetTypename (void) { return TYPENAME_LUMINOUS_CANVAS; }

CDatum CAEONLuminousCanvas::Create ()

//	Create
//
//	Creates an empty canvas.

	{
	return CDatum(new CAEONLuminousCanvas);
	}

CDatum CAEONLuminousCanvas::CreateSprite (CDatum dDesc, CDatum dOptions)

//	CreateSprite
//
//	Creates a sprite from a descriptor. If the desc is not valid, then we return
//	Nil.

	{
	if (dDesc.GetBasicType() == CDatum::typeImage32)
		{
		auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateImage(dDesc));
		m_Resources.AddResource(Graphic, dDesc);

		SetSpriteOptions(Graphic, dOptions);

		return CAEONLuminousSprite::Create(CDatum::raw_AsComplex(this), Graphic.GetID());
		}
	else
		return CDatum();
	}

CDatum CAEONLuminousCanvas::GenerateBeginUpdate ()

//	GenerateBeginUpdate
//
//	Generates a graphic descriptor

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_BEGIN_UPDATE);

	return dResult;
	}

CDatum CAEONLuminousCanvas::GenerateClearRectDesc (const CVector2D& vUL, const CVector2D& vLR)

//	GenerateClearRectDesc
//
//	Generates a graphic descriptor.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_CLEAR_RECT);
	dResult.SetElement(FIELD_UL, vUL);
	dResult.SetElement(FIELD_LR, vLR);

	return dResult;
	}

CDatum CAEONLuminousCanvas::GenerateImageDesc (const CVector2D& vPos, CDatum dImage)

//	GenerateImageDesc
//
//	Generates a graphic descriptor.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_IMAGE);
	dResult.SetElement(FIELD_UL, vPos);
	dResult.SetElement(FIELD_IMAGE, dImage);

	return dResult;
	}

CDatum CAEONLuminousCanvas::GenerateSetResourceDesc (const CString& sResourceID, CDatum dImage)

//	GenerateSetResourceDesc
//
//	Generates a set resource descriptor.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_SET_RESOURCE);
	dResult.SetElement(FIELD_RESOURCE_ID, sResourceID);
	dResult.SetElement(FIELD_IMAGE, dImage);

	return dResult;
	}

CDatum CAEONLuminousCanvas::GenerateShapeDesc (const CLuminousCanvasModel::SShapeOptions& Options)

//	GenerateShapeDesc
//
//	Generates a graphic descriptor.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_SHAPE);
	dResult.SetElement(FIELD_PATH, CAEONLuminous::AsDatum(Options.Path));
	dResult.SetElement(FIELD_FILL_STYLE, CAEONLuminous::AsDatum(Options.FillStyle));
	dResult.SetElement(FIELD_LINE_STYLE, CAEONLuminous::AsDatum(Options.LineStyle));
	dResult.SetElement(FIELD_SHADOW_STYLE, CAEONLuminous::AsDatum(Options.ShadowStyle));

	return dResult;
	}

CDatum CAEONLuminousCanvas::GenerateTextDesc (const CLuminousCanvasModel::STextOptions& Options)

//	GenerateTextDesc
//
//	Generates a text descriptor

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_TEXT);
	dResult.SetElement(FIELD_TEXT, Options.sText);
	dResult.SetElement(FIELD_POS_X, Options.Pos.X());
	dResult.SetElement(FIELD_POS_Y, Options.Pos.Y());
	dResult.SetElement(FIELD_FONT_STYLE, CAEONLuminous::AsDatum(Options.FontStyle));
	dResult.SetElement(FIELD_TEXT_ALIGN, CAEONLuminous::AsDatum(Options.AlignStyle));
	dResult.SetElement(FIELD_FILL_STYLE, CAEONLuminous::AsDatum(Options.FillStyle));
	dResult.SetElement(FIELD_LINE_STYLE, CAEONLuminous::AsDatum(Options.LineStyle));
	dResult.SetElement(FIELD_SHADOW_STYLE, CAEONLuminous::AsDatum(Options.ShadowStyle));

	return dResult;
	}

bool CAEONLuminousCanvas::InsertGraphic (CDatum dDesc)

//	InsertGraphic
//
//	Inserts a new graphic.

	{
	OnModify();
	SequenceNumber Seq = GetSeq();

	CStringView sType = dDesc.GetElement(FIELD_TYPE);
	if (strEquals(sType, TYPE_BEGIN_UPDATE))
		{
		auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateBeginUpdate());
		Graphic.SetSeq(Seq);
		}
	else if (strEquals(sType, TYPE_CLEAR_RECT))
		{
		CVector2D vUL = dDesc.GetElement(FIELD_UL);
		CVector2D vLR = dDesc.GetElement(FIELD_LR);

		auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateClearRect(vUL, vLR));
		Graphic.SetSeq(Seq);
		}
	else if (strEquals(sType, TYPE_IMAGE))
		{
		CVector2D vPos = dDesc.GetElement(FIELD_UL);
		CDatum dImage = dDesc.GetElement(FIELD_IMAGE);

		if (!((const CRGBA32Image&)dImage).IsEmpty())
			{
			auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateImage(dImage));
			Graphic.SetPos(vPos);
			Graphic.SetSeq(Seq);

			m_Resources.AddResource(Graphic, dImage);
			}
		else
			{
			CStringView sResourceID = dImage;
			if (m_Resources.FindNamedResource(sResourceID) == -1)
				return false;

			auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateResource(sResourceID));
			Graphic.SetPos(vPos);
			Graphic.SetSeq(Seq);
			}
		}
	else if (strEquals(sType, TYPE_SET_RESOURCE))
		{
		CStringView sResourceID = dDesc.GetElement(FIELD_RESOURCE_ID);
		CDatum dImage = dDesc.GetElement(FIELD_IMAGE);

		m_Resources.AddNamedResource(sResourceID, dImage, Seq);
		}
	else if (strEquals(sType, TYPE_SHAPE))
		{
		CLuminousCanvasModel::SShapeOptions Options;
		Options.Path = CAEONLuminous::AsPath2D(dDesc.GetElement(FIELD_PATH));
		Options.FillStyle = CAEONLuminous::AsFillStyle(dDesc.GetElement(FIELD_FILL_STYLE));
		Options.LineStyle = CAEONLuminous::AsLineStyle(dDesc.GetElement(FIELD_LINE_STYLE));
		Options.ShadowStyle = CAEONLuminous::AsShadowStyle(dDesc.GetElement(FIELD_SHADOW_STYLE));

		auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));
		Graphic.SetSeq(Seq);
		}
	else if (strEquals(sType, TYPE_TEXT))
		{
		CLuminousCanvasModel::STextOptions Options;
		Options.sText = dDesc.GetElement(FIELD_TEXT).AsStringView();
		Options.Pos = CVector2D(dDesc.GetElement(FIELD_POS_X), dDesc.GetElement(FIELD_POS_Y));
		Options.FontStyle = CAEONLuminous::AsFontStyle(dDesc.GetElement(FIELD_FONT_STYLE));
		Options.AlignStyle = CAEONLuminous::AsTextAlign(dDesc.GetElement(FIELD_TEXT_ALIGN));
		Options.FillStyle = CAEONLuminous::AsFillStyle(dDesc.GetElement(FIELD_FILL_STYLE));
		Options.LineStyle = CAEONLuminous::AsLineStyle(dDesc.GetElement(FIELD_LINE_STYLE));
		Options.ShadowStyle = CAEONLuminous::AsShadowStyle(dDesc.GetElement(FIELD_SHADOW_STYLE));

		auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateText(Options));
		Graphic.SetSeq(Seq);
		}
	else
		return false;

	return true;
	}

size_t CAEONLuminousCanvas::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Computes heuristic size.

	{
	return 0;
	}

bool CAEONLuminousCanvas::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	m_Seq = Stream.ReadDWORDLONG();
	m_Model = CLuminousCanvasModel::Read(Stream);
	m_DrawCtx = CLuminousCanvasCtx::Read(Stream);
	m_Resources = CLuminousCanvasResources::Read(Stream);
	return true;
	}

void CAEONLuminousCanvas::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	m_Resources.Mark();
	}

void CAEONLuminousCanvas::OnModify ()

//	OnModify
//
//	The canvas is about to be modified.

	{
	m_Seq++;
	}

void CAEONLuminousCanvas::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize to a structure.

	{
	Stream.Write(m_Seq);
	m_Model.Write(Stream);
	m_DrawCtx.Write(Stream);
	m_Resources.Write(Stream);
	}

void CAEONLuminousCanvas::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_Seq = Stream.ReadDWORDLONG();
	m_Model = CLuminousCanvasModel::Read(Stream);
	m_DrawCtx = CLuminousCanvasCtx::Read(Stream);
	m_Resources = CLuminousCanvasResources::Read(Stream);
	}

void CAEONLuminousCanvas::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	Stream.Write(m_Seq);
	m_Model.Write(Stream);
	m_DrawCtx.Write(Stream);
	m_Resources.Write(Stream);
	}

int CAEONLuminousCanvas::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	const IAEONCanvas *pOtherCanvas = dValue.GetCanvasInterface();
	if (!pOtherCanvas)
		return KeyCompare(AsString(), dValue.AsString());

	CDatum dThis = RenderAsHTMLCanvasCommands(0);
	CDatum dOther = pOtherCanvas->RenderAsHTMLCanvasCommands(0);
	return dThis.OpCompare(dOther);
	}

bool CAEONLuminousCanvas::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if we are equal to dValue.

	{
	const IAEONCanvas *pOtherCanvas = dValue.GetCanvasInterface();
	if (!pOtherCanvas)
		return false;

	CDatum dThis = RenderAsHTMLCanvasCommands(0);
	CDatum dOther = pOtherCanvas->RenderAsHTMLCanvasCommands(0);
	return dThis.OpIsEqual(dOther);
	}

void* CAEONLuminousCanvas::raw_GetGraphicByID (DWORD dwID) const

//	raw_GetGraphicByID
//
//	Returns a raw pointer to the graphic (or NULL).

	{
	auto pGraphic = const_cast<CLuminousCanvasModel&>(m_Model).FindGraphicByID(dwID);
	return const_cast<ILuminousGraphic*>(pGraphic);
	}

CDatum CAEONLuminousCanvas::RenderAsHTMLCanvasCommands (SequenceNumber Seq) const

//	RenderAsHTMLCanvasCommands
//
//	Returns an array of HTML canvas commands.

	{
	return CHTMLCanvasRemote::RenderAsHTMLCanvasCommands(m_Model, m_Resources, Seq);
	}

void CAEONLuminousCanvas::SetGraphicSeq (int iIndex, SequenceNumber Seq)

//	SetGraphicSeq
//
//	Sets the sequence number of the given graphic element.

	{
	if (iIndex < 0 || iIndex >= m_Model.GetRenderCount())
		return;

	m_Model.GetRenderGraphic(iIndex).SetSeq(Seq);
	}

void CAEONLuminousCanvas::SetSpriteOptions (ILuminousGraphic& Graphic, CDatum dOptions)

//	SetSpriteOptions
//
//	Sets sprite options.

	{
	for (int i = 0; i < dOptions.GetCount(); i++)
		{
		auto sKey = dOptions.GetKey(i);
		auto dValue = dOptions.GetElement(i);

		if (strEquals(sKey, FIELD_POS))
			{
			Graphic.SetPos(dValue);
			}
		else if (strEquals(sKey, FIELD_SEQ))
			{
			Graphic.SetSeq(dValue);
			}
		}
	}
