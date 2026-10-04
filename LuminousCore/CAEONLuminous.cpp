//	CAEONLuminous.cpp
//
//	CAEONLuminous Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(BASELINE_ALPHABETIC,			"alphabetic");

DECLARE_CONST_STRING(DATUM_TYPENAME_LUMINOUS_BITMAP,"luminousBitmap");
DECLARE_CONST_STRING(DATUM_TYPENAME_REANIMATOR,		"reanimator");
DECLARE_CONST_STRING(DATUM_TYPENAME_REANIMATOR3D,	"reanimator3D");
DECLARE_CONST_STRING(DATUM_TYPENAME_TEXTURE3D,		"texture3D");
DECLARE_CONST_STRING(DATUM_TYPENAME_MATERIAL3D,		"material3D");
DECLARE_CONST_STRING(DATUM_TYPENAME_OBJ3D,			"obj3D");
DECLARE_CONST_STRING(DATUM_TYPENAME_CIRCLE2D,		"circle2D");
DECLARE_CONST_STRING(DATUM_TYPENAME_TEXT2D,		"text2D");
DECLARE_CONST_STRING(DATUM_TYPENAME_RECT2D,			"rect2D");
DECLARE_CONST_STRING(DATUM_TYPENAME_LINE2D,			"line2D");
DECLARE_CONST_STRING(DATUM_TYPENAME_TRAIL2D,			"trail2D");

DECLARE_CONST_STRING(FIELD_ALIGN,					"align");
DECLARE_CONST_STRING(FIELD_BASELINE,				"baseline");
DECLARE_CONST_STRING(FIELD_BLUR,					"blur");
DECLARE_CONST_STRING(FIELD_COLOR,					"color");
DECLARE_CONST_STRING(FIELD_DIRECTION,				"direction");
DECLARE_CONST_STRING(FIELD_LINE_CAP,				"lineCap");
DECLARE_CONST_STRING(FIELD_LINE_JOIN,				"lineJoin");
DECLARE_CONST_STRING(FIELD_LINE_HEIGHT,				"lineHeight");
DECLARE_CONST_STRING(FIELD_LINE_STYLE,				"lineStyle");
DECLARE_CONST_STRING(FIELD_LINE_WIDTH,				"lineWidth");
DECLARE_CONST_STRING(FIELD_MITER_LIMIT,				"miterLimit");
DECLARE_CONST_STRING(FIELD_OFFSET_XY,				"offset");
DECLARE_CONST_STRING(FIELD_SIZE,					"size");
DECLARE_CONST_STRING(FIELD_STRETCH,					"stretch");
DECLARE_CONST_STRING(FIELD_STYLE,					"style");
DECLARE_CONST_STRING(FIELD_TYPEFACE,				"typeface");
DECLARE_CONST_STRING(FIELD_WEIGHT,					"weight");

DECLARE_CONST_STRING(PIXEL_FORMAT_GRAY8_ID,			"gray8");
DECLARE_CONST_STRING(PIXEL_FORMAT_GRAY8_LABEL,		"Grayscale (8-bit)");
DECLARE_CONST_STRING(PIXEL_FORMAT_MONO_ID,			"mono");
DECLARE_CONST_STRING(PIXEL_FORMAT_MONO_LABEL,		"Monochrome");
DECLARE_CONST_STRING(PIXEL_FORMAT_RGBA8_ID,			"RGBA8");
DECLARE_CONST_STRING(PIXEL_FORMAT_RGBA8_LABEL,		"RGBA (8-bit channels)");

DECLARE_CONST_STRING(TYPE_ARC,						"arc");
DECLARE_CONST_STRING(TYPE_ARC_TO,					"arcTo");
DECLARE_CONST_STRING(TYPE_CLOSE_PATH,				"closePath");
DECLARE_CONST_STRING(TYPE_LINE_TO,					"lineTo");
DECLARE_CONST_STRING(TYPE_MOVE_TO,					"moveTo");
DECLARE_CONST_STRING(TYPE_RECT,						"rect");
DECLARE_CONST_STRING(TYPE_SOLID,					"solid");

DECLARE_CONST_STRING(TYPENAME_BITMAP_TYPE,			"BitmapType");
DECLARE_CONST_STRING(TYPENAME_BITMAP_GRAY8_TYPE,	"BitmapGray8Type");
DECLARE_CONST_STRING(TYPENAME_BITMAP_MONO_TYPE,		"BitmapMonoType");
DECLARE_CONST_STRING(TYPENAME_BITMAP_RGBA8_TYPE,	"BitmapRGBA8Type");
DECLARE_CONST_STRING(TYPENAME_PIXEL_FORMAT,			"PixelFormatEnum");
DECLARE_CONST_STRING(TYPENAME_CIRCLE2D,				"Circle2DType");
DECLARE_CONST_STRING(TYPENAME_TEXT2D,				"Text2DType");
DECLARE_CONST_STRING(TYPENAME_RECT2D,				"Rect2DType");
DECLARE_CONST_STRING(TYPENAME_LINE2D,				"Line2DType");
DECLARE_CONST_STRING(TYPENAME_TRAIL2D,				"Trail2DType");
DECLARE_CONST_STRING(TYPENAME_SCENE2D,				"Scene2DType");
DECLARE_CONST_STRING(TYPENAME_TEXTURE3D,			"Texture3DType");
DECLARE_CONST_STRING(TYPENAME_MATERIAL3D,			"Material3DType");
DECLARE_CONST_STRING(TYPENAME_PHYSICAL_MATERIAL3D,	"PhysicalMaterial3D");
DECLARE_CONST_STRING(TYPENAME_OBJECT3D,				"Object3DType");
DECLARE_CONST_STRING(TYPENAME_SCENE3D,				"Scene3DType");

bool CAEONLuminous::m_bInitialized = false;
DWORD CAEONLuminous::BITMAP_TYPE = 0;
DWORD CAEONLuminous::BITMAP_MONO_TYPE = 0;
DWORD CAEONLuminous::BITMAP_GRAY8_TYPE = 0;
DWORD CAEONLuminous::BITMAP_RGBA8_TYPE = 0;
DWORD CAEONLuminous::PIXEL_FORMAT_ENUM = 0;
DWORD CAEONLuminous::CIRCLE2D_TYPE = 0;
DWORD CAEONLuminous::TEXT2D_TYPE = 0;
DWORD CAEONLuminous::RECT2D_TYPE = 0;
DWORD CAEONLuminous::LINE2D_TYPE = 0;
DWORD CAEONLuminous::TRAIL2D_TYPE = 0;
DWORD CAEONLuminous::SCENE2D_TYPE = 0;
DWORD CAEONLuminous::TEXTURE3D_TYPE = 0;
DWORD CAEONLuminous::MATERIAL3D_TYPE = 0;
DWORD CAEONLuminous::PHYSICAL_MATERIAL3D_TYPE = 0;
DWORD CAEONLuminous::OBJECT3D_TYPE = 0;
DWORD CAEONLuminous::SCENE3D_TYPE = 0;

bool CAEONLuminous::Boot ()

//	Boot
//
//	Boot the AEON factory classes.

	{
	if (!m_bInitialized)
		{
		CAEONLuminousBitmap::RegisterFactory();
		CAEONLuminousCanvas::RegisterFactory();
		CAEONCircle2D::RegisterFactory();
		CAEONText2D::RegisterFactory();
		CAEONRect2D::RegisterFactory();
		CAEONLine2D::RegisterFactory();
		CAEONTrail2D::RegisterFactory();
		CAEONReanimator::RegisterFactory();
		CAEONTexture3D::RegisterFactory();
		CAEONMaterial3D::RegisterFactory();
		CAEONObj3D::RegisterFactory();
		CAEONReanimator3D::RegisterFactory();

		//	Register some AEON data types

		TArray<IDatatype::SMemberDesc> PixelFormatEnum;
		PixelFormatEnum.Insert({ IDatatype::EMemberType::EnumValue, PIXEL_FORMAT_MONO_ID, CDatum(), INT_MIN, PIXEL_FORMAT_MONO_LABEL });
		PixelFormatEnum.Insert({ IDatatype::EMemberType::EnumValue, PIXEL_FORMAT_GRAY8_ID, CDatum(), INT_MIN, PIXEL_FORMAT_GRAY8_LABEL });
		PixelFormatEnum.Insert({ IDatatype::EMemberType::EnumValue, PIXEL_FORMAT_RGBA8_ID, CDatum(), INT_MIN, PIXEL_FORMAT_RGBA8_LABEL });
		PIXEL_FORMAT_ENUM = CAEONTypes::RegisterEnum(TYPENAME_PIXEL_FORMAT, PixelFormatEnum);

		BITMAP_TYPE = CAEONTypes::RegisterAEON(TYPENAME_BITMAP_TYPE, CDatatypeList(), DATUM_TYPENAME_LUMINOUS_BITMAP, CAEONLuminousBitmap::GetMembers);
		BITMAP_MONO_TYPE = CAEONTypes::RegisterAEON(TYPENAME_BITMAP_MONO_TYPE, CDatatypeList({ CAEONTypes::Get(BITMAP_TYPE) }), DATUM_TYPENAME_LUMINOUS_BITMAP, CAEONLuminousBitmap::GetMembers);
		BITMAP_GRAY8_TYPE = CAEONTypes::RegisterAEON(TYPENAME_BITMAP_GRAY8_TYPE, CDatatypeList({ CAEONTypes::Get(BITMAP_TYPE) }), DATUM_TYPENAME_LUMINOUS_BITMAP, CAEONLuminousBitmap::GetMembers);
		BITMAP_RGBA8_TYPE = CAEONTypes::RegisterAEON(TYPENAME_BITMAP_RGBA8_TYPE, CDatatypeList({ CAEONTypes::Get(BITMAP_TYPE) }), DATUM_TYPENAME_LUMINOUS_BITMAP, CAEONLuminousBitmap::GetMembers);
		CIRCLE2D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_CIRCLE2D, CDatatypeList(), DATUM_TYPENAME_CIRCLE2D, CAEONCircle2D::GetMembers);
		TEXT2D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_TEXT2D, CDatatypeList(), DATUM_TYPENAME_TEXT2D, CAEONText2D::GetMembers);
		RECT2D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_RECT2D, CDatatypeList(), DATUM_TYPENAME_RECT2D, CAEONRect2D::GetMembers);
		LINE2D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_LINE2D, CDatatypeList(), DATUM_TYPENAME_LINE2D, CAEONLine2D::GetMembers);
		TRAIL2D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_TRAIL2D, CDatatypeList(), DATUM_TYPENAME_TRAIL2D, CAEONTrail2D::GetMembers);
		SCENE2D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_SCENE2D, CDatatypeList(), DATUM_TYPENAME_REANIMATOR, CAEONReanimator::GetMembers);
		TEXTURE3D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_TEXTURE3D, CDatatypeList(), DATUM_TYPENAME_TEXTURE3D, CAEONTexture3D::GetMembers);
		MATERIAL3D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_MATERIAL3D, CDatatypeList(), DATUM_TYPENAME_MATERIAL3D, CAEONMaterial3D::GetMembers);
		PHYSICAL_MATERIAL3D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_PHYSICAL_MATERIAL3D, CDatatypeList({ CAEONTypes::Get(MATERIAL3D_TYPE) }), DATUM_TYPENAME_MATERIAL3D, CAEONMaterial3D::GetMembers);
		OBJECT3D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_OBJECT3D, CDatatypeList(), DATUM_TYPENAME_OBJ3D, CAEONObj3D::GetMembers);
		SCENE3D_TYPE = CAEONTypes::RegisterAEON(TYPENAME_SCENE3D, CDatatypeList(), DATUM_TYPENAME_REANIMATOR3D, CAEONReanimator3D::GetMembers);

		//	Done

		m_bInitialized = true;
		}

	return true;
	}

CLuminousColor CAEONLuminous::AsColor (CDatum dValue, const CLuminousColor& Default)

//	AsColor
//
//	Decode from datum.

	{
	if (dValue.IsNil())
		return Default;
	else if (dValue.GetBasicType() == CDatum::typeString)
		{
		CStringView sValue = dValue;

		//	A leading dot means a theme color.

		if (*sValue.GetParsePointer() == '.')
			{
			return CLuminousColor::ParseThemeColor(strSubString(sValue, 1), Default);
			}

		//	Otherwise, either a CSS color or a theme color.

		else
			{
			bool bFail;
			CRGBA32 rgbColor = CRGBA32::Parse(sValue, &bFail);
			if (!bFail)
				return CLuminousColor(rgbColor);

			//	Otherwise, we expect a theme color.

			else
				{
				CRGBA32 rgbColor = CRGBA32::ParseHTMLColor(sValue, &bFail);

				//	If this is an HTML color, then see if it is also a theme 
				//	color (and take the theme color). This happens because some
				//	theme colors (like "Lime") are also HTML colors.

				if (!bFail)
					return CLuminousColor::ParseThemeColor(sValue, CLuminousColor(rgbColor));
				else
					return CLuminousColor::ParseThemeColor(sValue, Default);
				}
			}
		}
	else
		{
		CStringView sType = dValue.GetElement(0);
		if (sType.IsEmpty())
			return Default;
		else if (strEquals(sType, TYPE_SOLID))
			{
			CRGBA32 rgbColor = CRGBA32((DWORD)dValue.GetElement(1));
			return CLuminousColor(rgbColor);
			}
		else
			return Default;
		}
	}

CDatum CAEONLuminous::AsDatum (const CLuminousColor& Color, const CLuminousColor& Default)

//	AsDatum
//
//	Encodes as a datum.

	{
	switch (Color.GetType())
		{
		case CLuminousColor::EType::None:
			if (!Default.IsEmpty())
				return AsDatum(Default);
			else
				return CDatum();

		case CLuminousColor::EType::Solid:
			return CDatum(Color.GetSolidColor().AsHTMLColor());

		case CLuminousColor::EType::Theme:
		case CLuminousColor::EType::Role:
			//	Add a leading dot so we can distinguish on the client.
			return CDatum(strPattern(".%s", Color.GetThemeColorID()));

		default:
			throw CException(errFail);
		}
	}

CDatum CAEONLuminous::AsDatum (const CLuminousPath2D& Path)

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

CDatum CAEONLuminous::AsDatum (const CLuminousFillStyle& Style)

//	AsDatum
//
//	Encodes as a datum.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_COLOR, AsDatum(Style.GetColor()));

	return dResult;
	}

CDatum CAEONLuminous::AsDatum (const CLuminousFontStyle& Style)

//	AsDatum
//
//	Represent as a datum.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPEFACE, Style.GetTypeface());
	dResult.SetElement(FIELD_SIZE, Style.GetSize());
	dResult.SetElement(FIELD_WEIGHT, Style.GetWeight());
	dResult.SetElement(FIELD_STYLE, Style.GetStyleAsID());
	dResult.SetElement(FIELD_STRETCH, Style.GetStretch());
	dResult.SetElement(FIELD_LINE_HEIGHT, Style.GetLineHeight());

	return dResult;
	}

CDatum CAEONLuminous::AsDatum (const CLuminousLineStyle& Style)

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

CDatum CAEONLuminous::AsDatum (const CLuminousShadowStyle& Style)

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

CDatum CAEONLuminous::AsDatum (const CLuminousTextAlign& Style)

//	AsDatum
//
//	Encodes as a datum.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_ALIGN, Style.GetAlignAsHTML());
	dResult.SetElement(FIELD_BASELINE, Style.GetBaselineAsHTML());
	dResult.SetElement(FIELD_DIRECTION, Style.GetDirectionAsHTML());
	
	return dResult;
	}

CLuminousFillStyle CAEONLuminous::AsFillStyle (CDatum dValue)

//	AsFillStyle
//
//	Decode from datum.

	{
	CLuminousColor Color = AsColor(dValue.GetElement(FIELD_COLOR));
	return CLuminousFillStyle(Color);
	}

CLuminousFontStyle CAEONLuminous::AsFontStyle (CDatum dValue)

//	ParseFontStyle
//
//	Parses a font style from a datum.

	{
	if (dValue.GetBasicType() == CDatum::typeString)
		{
		return CLuminousFontStyle::ParseHTML(dValue.AsStringView());
		}
	else if (dValue.GetBasicType() == CDatum::typeStruct)
		{
		CLuminousFontStyle Font;
		Font.SetTypeface(dValue.GetElement(FIELD_TYPEFACE).AsStringView());
		Font.SetSize(dValue.GetElement(FIELD_SIZE));
		Font.SetWeight(dValue.GetElement(FIELD_WEIGHT));
		Font.SetStyle(CLuminousFontStyle::ParseStyle(dValue.GetElement(FIELD_STYLE).AsStringView()));
		Font.SetStretch(dValue.GetElement(FIELD_STRETCH));
		Font.SetLineHeight(dValue.GetElement(FIELD_LINE_HEIGHT));

		return Font;
		}
	else
		//	LATER: Handle other formats
		return CLuminousFontStyle::Null;
	}

CLuminousLineStyle CAEONLuminous::AsLineStyle (CDatum dValue)

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

CLuminousPath2D CAEONLuminous::AsPath2D (CDatum dValue)

//	AsPath2D
//
//	Decode from datum.

	{
	CLuminousPath2D Path;
	for (int i = 0; i < dValue.GetCount(); i++)
		{
		CDatum dSegment = dValue.GetElement(i);
		CStringView sType = dSegment.GetElement(0);

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

CLuminousShadowStyle CAEONLuminous::AsShadowStyle (CDatum dValue)

//	AsShadowStyle
//
//	Decode from datum.

	{
	CLuminousColor Color = AsColor(dValue.GetElement(FIELD_COLOR));
	double rBlur = dValue.GetElement(FIELD_BLUR);
	CVector2D vOffset = dValue.GetElement(FIELD_OFFSET_XY);

	return CLuminousShadowStyle(Color, rBlur, vOffset);
	}

CLuminousTextAlign CAEONLuminous::AsTextAlign (CDatum dValue)

//	AsTextAlign
//
//	Decode from datum.

	{
	CLuminousTextAlign Align;

	Align.SetAlign(CLuminousTextAlign::ParseAlign(dValue.GetElement(FIELD_ALIGN).AsStringView()));
	Align.SetBaseline(CLuminousTextAlign::ParseBaseline(dValue.GetElement(FIELD_BASELINE).AsStringView()));
	Align.SetDirection(CLuminousTextAlign::ParseDirection(dValue.GetElement(FIELD_DIRECTION).AsStringView()));

	return Align;
	}

