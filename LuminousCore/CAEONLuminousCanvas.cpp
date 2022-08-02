//	CAEONLuminousCanvas.cpp
//
//	CAEONLuminousCanvas Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(FIELD_BLUR,					"blur");
DECLARE_CONST_STRING(FIELD_COLOR,					"color");
DECLARE_CONST_STRING(FIELD_FILL_STYLE,				"fillStyle");
DECLARE_CONST_STRING(FIELD_IMAGE,					"image");
DECLARE_CONST_STRING(FIELD_LINE_CAP,				"lineCap");
DECLARE_CONST_STRING(FIELD_LINE_JOIN,				"lineJoin");
DECLARE_CONST_STRING(FIELD_LINE_STYLE,				"lineStyle");
DECLARE_CONST_STRING(FIELD_LINE_WIDTH,				"lineWidth");
DECLARE_CONST_STRING(FIELD_LR,						"lr");
DECLARE_CONST_STRING(FIELD_MITER_LIMIT,				"miterLimit");
DECLARE_CONST_STRING(FIELD_OFFSET_XY,				"offset");
DECLARE_CONST_STRING(FIELD_PATH,					"path");
DECLARE_CONST_STRING(FIELD_POS,						"pos");
DECLARE_CONST_STRING(FIELD_SEQ,						"seq");
DECLARE_CONST_STRING(FIELD_SHADOW_STYLE,			"shadowStyle");
DECLARE_CONST_STRING(FIELD_TYPE,					"type");
DECLARE_CONST_STRING(FIELD_UL,						"ul");

DECLARE_CONST_STRING(TYPE_ARC,						"arc");
DECLARE_CONST_STRING(TYPE_ARC_TO,					"arcTo");
DECLARE_CONST_STRING(TYPE_BEGIN_UPDATE,				"beginUpdate");
DECLARE_CONST_STRING(TYPE_CLEAR_RECT,				"clearRect");
DECLARE_CONST_STRING(TYPE_CLOSE_PATH,				"closePath");
DECLARE_CONST_STRING(TYPE_IMAGE,					"image");
DECLARE_CONST_STRING(TYPE_LINE_TO,					"lineTo");
DECLARE_CONST_STRING(TYPE_MOVE_TO,					"moveTo");
DECLARE_CONST_STRING(TYPE_RECT,						"rect");
DECLARE_CONST_STRING(TYPE_SHAPE,					"shape");
DECLARE_CONST_STRING(TYPE_SOLID,					"solid");

DECLARE_CONST_STRING(TYPENAME_LUMINOUS_CANVAS,		"luminousCanvas");

DECLARE_CONST_STRING(ERR_INVALID_SPRITE_DESC,		"Invalid sprite descriptor.");
DECLARE_CONST_STRING(ERR_INVALID_RESOURCE_NAME,		"Invalid resource name.");
DECLARE_CONST_STRING(ERR_RESOURCE_NOT_FOUND,		"Unable to find canvas resource: %s.");

TDatumPropertyHandler<CAEONLuminousCanvas> CAEONLuminousCanvas::m_Properties = {
	{
		"fillStyle",
		"The fill style for future shapes.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(CAEONLuminousCanvas::AsDatum(Obj.m_DrawCtx.Fill().GetColor()));
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_DrawCtx.Fill().SetColor(CLuminousColor(CRGBA32::Parse(dValue.AsString())));
			return true;
			},
		},
	{
		"length",
		"Returns the number of rows in the table.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(1);
			},
		NULL,
		},
	{
		"lineStyle",
		"The line style for future shapes.",
		[](const CAEONLuminousCanvas &Obj, const CString &sProperty)
			{
			return CDatum(CAEONLuminousCanvas::AsDatum(Obj.m_DrawCtx.Line().GetColor()));
			},
		[](CAEONLuminousCanvas &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_DrawCtx.Line().SetColor(CLuminousColor(CRGBA32::Parse(dValue.AsString())));
			return true;
			},
		},
	{
		"lineWidth",
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
	};

TDatumMethodHandler<CAEONLuminousCanvas> CAEONLuminousCanvas::m_Methods = {
	{
		"arc",
		"*",
		".arc(x, y, radius, startAngle, endAngle[, counterclockwise]) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			double x = dLocalEnv.GetElement(1);
			double y = dLocalEnv.GetElement(2);
			double radius = dLocalEnv.GetElement(3);
			double startAngle = dLocalEnv.GetElement(4);
			double endAngle = dLocalEnv.GetElement(5);
			bool bCounterclock = !dLocalEnv.GetElement(6).IsNil();

			Obj.m_DrawCtx.Path().Arc(CVector2D(x, y), radius, startAngle, endAngle, bCounterclock);

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"beginPath",
		"*",
		".beginPath() -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			Obj.m_DrawCtx.Path().DeleteAll();

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"circle",
		"*",
		".circle(x, y, radius) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			double x = dLocalEnv.GetElement(1);
			double y = dLocalEnv.GetElement(2);
			double radius = dLocalEnv.GetElement(3);

			Obj.m_DrawCtx.Path().Arc(CVector2D(x, y), radius, 0.0, TAU, true);

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"clearRect",
		"*",
		".clearRect() -> true/null\n"
		".clearRect(x, y, width, height) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			Obj.OnModify();

			CDatum dCmdData;
			if (dLocalEnv.GetCount() == 1)
				Obj.m_Model.DeleteAll();
			else
				{
				double x = dLocalEnv.GetElement(1);
				double y = dLocalEnv.GetElement(2);
				double width = dLocalEnv.GetElement(3);
				double height = dLocalEnv.GetElement(4);
				if (width <= 0.0 || height <= 0.0)
					{
					retdResult = CDatum();
					return true;
					}

				Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateClearRect(CVector2D(x, y), CVector2D(x + width, y + height)));
				}

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"closePath",
		"*",
		".closePath() -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			Obj.m_DrawCtx.Path().ClosePath();

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"createSprite",
		"*",
		".createSprite(desc, [options]) -> sprite",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dDesc = dLocalEnv.GetElement(1);
			CDatum dOptions = dLocalEnv.GetElement(2);
			CDatum dSprite = Obj.CreateSprite(dDesc, dOptions);
			if (dSprite.IsNil())
				{
				retdResult = ERR_INVALID_SPRITE_DESC;
				return false;
				}

			retdResult = dSprite;
			return true;
			},
		},
	{
		"drawImage",
		"*",
		".drawImage(image, x, y) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dImage = dLocalEnv.GetElement(1);
			double x = dLocalEnv.GetElement(2);
			double y = dLocalEnv.GetElement(3);

			Obj.OnModify();

			if (!((const CRGBA32Image&)dImage).IsEmpty())
				{
				auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateImage(dImage));
				Graphic.SetPos(CVector2D(x, y));

				Obj.m_Resources.AddResource(Graphic, dImage);

				retdResult = CDatum(true);
				return true;
				}
			else
				{
				const CString& sResourceID = dImage;
				if (Obj.m_Resources.FindNamedResource(sResourceID) == -1)
					{
					retdResult = strPattern(ERR_RESOURCE_NOT_FOUND, sResourceID);
					return false;
					}

				auto& Graphic = Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateResource(sResourceID));
				Graphic.SetPos(CVector2D(x, y));

				retdResult = CDatum(true);
				return true;
				}
			},
		},
	{
		"fill",
		"*",
		".fill() -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.FillStyle = Obj.m_DrawCtx.Fill();

			Obj.OnModify();
			Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"fillRect",
		"*",
		".fillRect(x, y, width, height) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			double x = dLocalEnv.GetElement(1);
			double y = dLocalEnv.GetElement(2);
			double cx = dLocalEnv.GetElement(3);
			double cy = dLocalEnv.GetElement(4);

			Obj.OnModify();

			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path.Rect(CVector2D(x, y), CVector2D(x + cx, y + cy));
			Options.FillStyle = Obj.m_DrawCtx.Fill();
			Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"lineTo",
		"*",
		".lineTo(x, y) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			double x = dLocalEnv.GetElement(1);
			double y = dLocalEnv.GetElement(2);

			Obj.m_DrawCtx.Path().LineTo(CVector2D(x, y));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"moveTo",
		"*",
		".moveTo(x, y) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			double x = dLocalEnv.GetElement(1);
			double y = dLocalEnv.GetElement(2);

			Obj.m_DrawCtx.Path().MoveTo(CVector2D(x, y));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"outline",
		"*",
		".outline() -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.LineStyle = Obj.m_DrawCtx.Line();

			Obj.OnModify();

			Obj.m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"rect",
		"*",
		".rect(x, y, width, height) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			double x = dLocalEnv.GetElement(1);
			double y = dLocalEnv.GetElement(2);
			double cx = dLocalEnv.GetElement(3);
			double cy = dLocalEnv.GetElement(4);

			Obj.m_DrawCtx.Path().Rect(CVector2D(x, y), CVector2D(x + cx, y + cy));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"setFillStyle",
		"*",
		"DEPRECATED: Use .fillStyle property instead.",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dStyle = dLocalEnv.GetElement(1);

			Obj.m_DrawCtx.Fill().SetColor(CLuminousColor(CRGBA32::Parse(dStyle.AsString())));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"setLineStyle",
		"*",
		"DEPRECATED: Use .lineStyle property instead.",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dStyle = dLocalEnv.GetElement(1);

			Obj.m_DrawCtx.Line().SetColor(CLuminousColor(CRGBA32::Parse(dStyle.AsString())));

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"setResource",
		"*",
		".setResource(name, image) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			const CString& sName = dLocalEnv.GetElement(1);
			CDatum dResource = dLocalEnv.GetElement(2);

			if (sName.IsEmpty())
				{
				retdResult = ERR_INVALID_RESOURCE_NAME;
				return false;
				}

			Obj.m_Resources.AddNamedResource(sName, dResource);

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"sprite_moveTo",
		"*",
		".sprite_moveTo(spriteID, pos) -> true/null",
		0,
		[](CAEONLuminousCanvas& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			DWORD dwID = dLocalEnv.GetElement(1);
			CVector2D vPos = dLocalEnv.GetElement(2);

			auto& Graphic = Obj.m_Model.GetGraphicByID(dwID);
			Graphic.SetPos(vPos);

			Obj.OnModify();

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"renderHTMLCanvasCommands",
		"*",
		".renderHTMLCanvasCommands() -> desc",
		0,
		[](CAEONLuminousCanvas &Obj, IInvokeCtx &Ctx, const CString &sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum &retdResult)
			{
			retdResult = Obj.RenderAsHTMLCanvasCommands();
			return true;
			},
		},
	};

const CString &CAEONLuminousCanvas::StaticGetTypename (void) { return TYPENAME_LUMINOUS_CANVAS; }

CLuminousColor CAEONLuminousCanvas::AsColor (CDatum dValue)

//	AsColor
//
//	Decode from datum.

	{
	if (dValue.GetBasicType() == CDatum::typeString)
		{
		return CLuminousColor(CRGBA32::Parse(dValue));
		}
	else
		{
		const CString& sType = dValue.GetElement(0);
		if (sType.IsEmpty())
			return CLuminousColor();
		else if (strEquals(sType, TYPE_SOLID))
			{
			CRGBA32 rgbColor = CRGBA32((DWORD)dValue.GetElement(1));
			return CLuminousColor(rgbColor);
			}
		else
			return CLuminousColor();
		}
	}

CLuminousFillStyle CAEONLuminousCanvas::AsFillStyle (CDatum dValue)

//	AsFillStyle
//
//	Decode from datum.

	{
	CLuminousColor Color = AsColor(dValue.GetElement(FIELD_COLOR));
	return CLuminousFillStyle(Color);
	}

CLuminousLineStyle CAEONLuminousCanvas::AsLineStyle (CDatum dValue)

//	AsLineStyle
//
//	Decode from datum.

	{
	CLuminousColor Color = AsColor(dValue.GetElement(FIELD_COLOR));
	CLuminousLineStyle::ELineCap iLineCap = (CLuminousLineStyle::ELineCap)(int)dValue.GetElement(FIELD_LINE_CAP);
	CLuminousLineStyle::ELineJoin iLineJoin = (CLuminousLineStyle::ELineJoin)(int)dValue.GetElement(FIELD_LINE_JOIN);
	double rLineWidth = dValue.GetElement(FIELD_LINE_WIDTH);
	double rMiterLimit = dValue.GetElement(FIELD_MITER_LIMIT);

	return CLuminousLineStyle(Color, rLineWidth, iLineJoin, iLineCap, rMiterLimit);
	}

CLuminousPath2D CAEONLuminousCanvas::AsPath2D (CDatum dValue)

//	AsPath2D
//
//	Decode from datum.

	{
	CLuminousPath2D Path;
	for (int i = 0; i < dValue.GetCount(); i++)
		{
		CDatum dSegment = dValue.GetElement(i);
		const CString& sType = dSegment.GetElement(0);

		if (strEquals(sType, TYPE_ARC))
			{
			CVector2D vCenter = dSegment.GetElement(1);
			double rRadius = dSegment.GetElement(2);
			double rStartAngle = dSegment.GetElement(3);
			double rEndAngle = dSegment.GetElement(4);
			bool bCounterClockwise = !dSegment.GetElement(5).IsNil();

			Path.Arc(vCenter, rRadius, rStartAngle, rEndAngle, bCounterClockwise);
			}
		else if (strEquals(sType, TYPE_ARC_TO))
			{
			CVector2D vTangent1 = dSegment.GetElement(1);
			CVector2D vTangent2 = dSegment.GetElement(2);
			double rRadius = dSegment.GetElement(3);

			Path.ArcTo(vTangent1, vTangent2, rRadius);
			}
		else if (strEquals(sType, TYPE_CLOSE_PATH))
			{
			Path.ClosePath();
			}
		else if (strEquals(sType, TYPE_LINE_TO))
			{
			double rX = dSegment.GetElement(1);
			double rY = dSegment.GetElement(2);

			Path.LineTo(CVector2D(rX, rY));
			}
		else if (strEquals(sType, TYPE_MOVE_TO))
			{
			double rX = dSegment.GetElement(1);
			double rY = dSegment.GetElement(2);

			Path.MoveTo(CVector2D(rX, rY));
			}
		else if (strEquals(sType, TYPE_RECT))
			{
			double rULX = dSegment.GetElement(1);
			double rULY = dSegment.GetElement(2);
			double rLRX = dSegment.GetElement(3);
			double rLRY = dSegment.GetElement(4);

			Path.Rect(CVector2D(rULX, rULY), CVector2D(rLRX, rLRY));
			}
		else
			{
			return CLuminousPath2D();
			}
		}

	return Path;
	}

CLuminousShadowStyle CAEONLuminousCanvas::AsShadowStyle (CDatum dValue)

//	AsShadowStyle
//
//	Decode from datum.

	{
	CLuminousColor Color = AsColor(dValue.GetElement(FIELD_COLOR));
	double rBlur = dValue.GetElement(FIELD_BLUR);
	CVector2D vOffset = dValue.GetElement(FIELD_OFFSET_XY);

	return CLuminousShadowStyle(Color, rBlur, vOffset);
	}

CDatum CAEONLuminousCanvas::AsDatum (const CLuminousColor& Color)

//	AsDatum
//
//	Encodes as a datum.

	{
	switch (Color.GetType())
		{
		case CLuminousColor::EType::None:
			return CDatum();

		case CLuminousColor::EType::Solid:
			return CDatum(Color.GetSolidColor().AsHTMLColor());

		default:
			throw CException(errFail);
		}
	}

CDatum CAEONLuminousCanvas::AsDatum (const CLuminousPath2D& Path)

//	AsDatum
//
//	Encodes as a datum.

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(Path.GetSegmentCount());
	for (int i = 0; i < Path.GetSegmentCount(); i++)
		{
		switch (Path.GetSegmentType(i))
			{
			case CLuminousPath2D::ESegmentType::ArcClockwise:
			case CLuminousPath2D::ESegmentType::ArcCounterClockwise:
				{
				CVector2D vCenter;
				double rRadius;
				double rStartAngle;
				double rEndAngle;
				bool bCounterClockwise;

				Path.GetArc(i, vCenter, rRadius, rStartAngle, rEndAngle, bCounterClockwise);

				CDatum dSegment(CDatum::typeArray);
				dSegment.Append(TYPE_ARC);
				dSegment.Append(vCenter);
				dSegment.Append(rRadius);
				dSegment.Append(rStartAngle);
				dSegment.Append(rEndAngle);
				dSegment.Append(bCounterClockwise);

				dResult.Append(dSegment);
				break;
				}

			case CLuminousPath2D::ESegmentType::ArcTo:
				{
				CVector2D vTangent1;
				CVector2D vTangent2;
				double rRadius;

				Path.GetArcTo(i, vTangent1, vTangent2, rRadius);

				CDatum dSegment(CDatum::typeArray);
				dSegment.Append(TYPE_ARC_TO);
				dSegment.Append(vTangent1);
				dSegment.Append(vTangent2);
				dSegment.Append(rRadius);

				dResult.Append(dSegment);
				break;
				}

			case CLuminousPath2D::ESegmentType::ClosePath:
				dResult.Append(TYPE_CLOSE_PATH);
				break;

			case CLuminousPath2D::ESegmentType::LineTo:
				{
				CVector2D vPos = Path.GetLineTo(i);

				CDatum dSegment(CDatum::typeArray);
				dSegment.Append(TYPE_LINE_TO);
				dSegment.Append(vPos.X());
				dSegment.Append(vPos.Y());

				dResult.Append(dSegment);
				break;
				}

			case CLuminousPath2D::ESegmentType::MoveTo:
				{
				CVector2D vPos = Path.GetMoveTo(i);

				CDatum dSegment(CDatum::typeArray);
				dSegment.Append(TYPE_MOVE_TO);
				dSegment.Append(vPos.X());
				dSegment.Append(vPos.Y());

				dResult.Append(dSegment);
				break;
				}

			case CLuminousPath2D::ESegmentType::Rect:
				{
				CVector2D vUL;
				CVector2D vLR;

				Path.GetRect(i, vUL, vLR);

				CDatum dSegment(CDatum::typeArray);
				dSegment.Append(TYPE_RECT);
				dSegment.Append(vUL.X());
				dSegment.Append(vUL.Y());
				dSegment.Append(vLR.X());
				dSegment.Append(vLR.Y());

				dResult.Append(dSegment);
				break;
				}

			default:
				throw CException(errFail);
			}
		}

	return dResult;
	}

CDatum CAEONLuminousCanvas::AsDatum (const CLuminousFillStyle& Style)

//	AsDatum
//
//	Encodes as a datum.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_COLOR, AsDatum(Style.GetColor()));

	return dResult;
	}

CDatum CAEONLuminousCanvas::AsDatum (const CLuminousLineStyle& Style)

//	AsDatum
//
//	Encodes as a datum.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_COLOR, AsDatum(Style.GetColor()));
	dResult.SetElement(FIELD_LINE_CAP, (int)Style.GetLineCap());
	dResult.SetElement(FIELD_LINE_JOIN, (int)Style.GetLineJoin());
	dResult.SetElement(FIELD_LINE_WIDTH, Style.GetLineWidth());
	dResult.SetElement(FIELD_MITER_LIMIT, Style.GetMiterLimit());

	return dResult;
	}

CDatum CAEONLuminousCanvas::AsDatum (const CLuminousShadowStyle& Style)

//	AsDatum
//
//	Encodes as a datum.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_COLOR, AsDatum(Style.GetColor()));
	dResult.SetElement(FIELD_BLUR, Style.GetBlur());
	dResult.SetElement(FIELD_OFFSET_XY, Style.GetOffset());

	return dResult;
	}

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

CDatum CAEONLuminousCanvas::GenerateBeginUpdate (SequenceNumber Seq)

//	GenerateBeginUpdate
//
//	Generates a graphic descriptor

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_BEGIN_UPDATE);
	dResult.SetElement(FIELD_SEQ, Seq);

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

CDatum CAEONLuminousCanvas::GenerateShapeDesc (const CLuminousCanvasModel::SShapeOptions& Options)

//	GenerateShapeDesc
//
//	Generates a graphic descriptor.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_SHAPE);
	dResult.SetElement(FIELD_PATH, AsDatum(Options.Path));
	dResult.SetElement(FIELD_FILL_STYLE, AsDatum(Options.FillStyle));
	dResult.SetElement(FIELD_LINE_STYLE, AsDatum(Options.LineStyle));
	dResult.SetElement(FIELD_SHADOW_STYLE, AsDatum(Options.ShadowStyle));

	return dResult;
	}

bool CAEONLuminousCanvas::InsertGraphic (CDatum dDesc)

//	InsertGraphic
//
//	Inserts a new graphic.

	{
	SequenceNumber Seq = dDesc.GetElement(FIELD_SEQ);

	OnModify();
	if (Seq == 0)
		Seq = GetSeq();

	const CString& sType = dDesc.GetElement(FIELD_TYPE);
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

		auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateImage(dImage));
		Graphic.SetPos(vPos);
		Graphic.SetSeq(Seq);

		m_Resources.AddResource(Graphic, dImage);
		}
	else if (strEquals(sType, TYPE_SHAPE))
		{
		CLuminousCanvasModel::SShapeOptions Options;
		Options.Path = AsPath2D(dDesc.GetElement(FIELD_PATH));
		Options.FillStyle = AsFillStyle(dDesc.GetElement(FIELD_FILL_STYLE));
		Options.LineStyle = AsLineStyle(dDesc.GetElement(FIELD_LINE_STYLE));
		Options.ShadowStyle = AsShadowStyle(dDesc.GetElement(FIELD_SHADOW_STYLE));

		auto& Graphic = m_Model.InsertGraphic(CLuminousCanvasModel::CreateShape(Options));
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

bool CAEONLuminousCanvas::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize

	{
	throw CException(errFail);
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

void CAEONLuminousCanvas::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize to a structure.

	{
	//	LATER
	throw CException(errFail);
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
