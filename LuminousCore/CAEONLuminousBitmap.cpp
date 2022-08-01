//	CAEONLuminousBitmap.cpp
//
//	CAEONLuminousBitmap Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(TYPENAME_LUMINOUS_BITMAP,		"luminousBitmap");

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_IMAGE,	"Unable to create image.");

TDatumPropertyHandler<CAEONLuminousBitmap> CAEONLuminousBitmap::m_Properties = {
	{
		"height",
		"Returns the height of the image (in pixels).",
		[](const CAEONLuminousBitmap &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_Image.GetHeight());
			},
		NULL,
		},
	{
		"width",
		"Returns the width of the image (in pixels).",
		[](const CAEONLuminousBitmap &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_Image.GetWidth());
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONLuminousBitmap> CAEONLuminousBitmap::m_Methods = {
	{
		"arc",
		"*",
		".arc(x, y, radius, startAngle, endAngle[, counterclockwise]) -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			//	LATER

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"closePath",
		"*",
		".closePath() -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			Obj.m_DrawCtx.Path().ClosePath();

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"drawImage",
		"*",
		".drawImage(image, x, y) -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dImage = dLocalEnv.GetElement(1);
			double x = dLocalEnv.GetElement(2);
			double y = dLocalEnv.GetElement(3);

			//	LATER

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"fill",
		"*",
		".fill() -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.FillStyle = Obj.m_DrawCtx.Fill();

			//	LATER

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"fillRect",
		"*",
		".fillRect(x, y, width, height) -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			double x = dLocalEnv.GetElement(1);
			double y = dLocalEnv.GetElement(2);
			double cx = dLocalEnv.GetElement(3);
			double cy = dLocalEnv.GetElement(4);

			//	LATER: Use a graphics library.

			CImageDraw::Rectangle(Obj.m_Image, (int)x, (int)y, (int)cx, (int)cy, Obj.m_DrawCtx.Fill().GetColor().GetSolidColor());

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"getSlice",
		"*",
		".getSlice(x, y, width, height) -> image",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			int x = (int)dLocalEnv.GetElement(1);
			int y = (int)dLocalEnv.GetElement(2);
			int cx = Min((int)dLocalEnv.GetElement(3), Obj.m_Image.GetWidth() - x);
			int cy = Min((int)dLocalEnv.GetElement(4), Obj.m_Image.GetHeight() - y);

			if (cx <= 0 || cy <= 0)
				{
				retdResult = CDatum();
				return true;
				}

			CRGBA32Image Result;
			if (!Result.Create(cx, cy, Obj.m_Image.GetAlphaType()))
				{
				retdResult = ERR_UNABLE_TO_CREATE_IMAGE;
				return false;
				}

			CImageDraw::Copy(Result, 0, 0, Obj.m_Image, x, y, cx, cy);
			retdResult = CAEONLuminousBitmap::Create(std::move(Result));
			return true;
			},
		},
	{
		"lineTo",
		"*",
		".lineTo(x, y) -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.LineStyle = Obj.m_DrawCtx.Line();

			//	LATER

			retdResult = CDatum(true);
			return true;
			},
		},
	{
		"rect",
		"*",
		".rect(x, y, width, height) -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
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
		".setFillStyle(style) -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
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
		".setLineStyle(style) -> true/null",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dStyle = dLocalEnv.GetElement(1);

			Obj.m_DrawCtx.Line().SetColor(CLuminousColor(CRGBA32::Parse(dStyle.AsString())));

			retdResult = CDatum(true);
			return true;
			},
		},
	};

const CString &CAEONLuminousBitmap::StaticGetTypename (void) { return TYPENAME_LUMINOUS_BITMAP; }

IComplexDatum *CAEONLuminousBitmap::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clone a copy

	{
	CAEONLuminousBitmap *pClone = new CAEONLuminousBitmap;
	pClone->m_DrawCtx = m_DrawCtx;
	pClone->m_Image = m_Image;
	pClone->m_rgbBackground = m_rgbBackground;

	return pClone;
	}

CDatum CAEONLuminousBitmap::Create (const CRGBA32Image& Src)

//	Create
//
//	Create from an image.

	{
	auto pBitmap = new CAEONLuminousBitmap;
	pBitmap->m_Image = Src;
	pBitmap->m_rgbBackground = CRGBA32(0, 0, 0);

	return CDatum(pBitmap);
	}

CDatum CAEONLuminousBitmap::Create (CRGBA32Image&& Src)

//	Create
//
//	Create from an image via move.

	{
	auto pBitmap = new CAEONLuminousBitmap;
	pBitmap->m_Image = std::move(Src);
	pBitmap->m_rgbBackground = CRGBA32(0, 0, 0);

	return CDatum(pBitmap);
	}

CDatum CAEONLuminousBitmap::Create (int cxWidth, int cyHeight)

//	Create
//
//	Creates an empty canvas.

	{
	auto pBitmap = new CAEONLuminousBitmap;
	if (cxWidth > 0 && cyHeight > 0)
		pBitmap->m_Image.Create(cxWidth, cyHeight, CRGBA32Image::alpha8);

	pBitmap->m_rgbBackground = CRGBA32(0, 0, 0);

	return CDatum(pBitmap);
	}

CDatum CAEONLuminousBitmap::Create (int cxWidth, int cyHeight, CRGBA32 rgbBackground)

//	Create
//
//	Creates an empty canvas.

	{
	auto pBitmap = new CAEONLuminousBitmap;
	if (cxWidth > 0 && cyHeight > 0)
		pBitmap->m_Image.Create(cxWidth, cyHeight, CRGBA32Image::alpha8, rgbBackground);

	pBitmap->m_rgbBackground = rgbBackground;

	return CDatum(pBitmap);
	}

size_t CAEONLuminousBitmap::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Computes heuristic size.

	{
	return CalcMemorySize();
	}

bool CAEONLuminousBitmap::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	switch (iFormat)
		{
		case CDatum::EFormat::JSON:
			{
			CBuffer Buffer;
			Buffer.SetLength(Stream.GetStreamLength());
			Stream.Read(Buffer.GetPointer(), Buffer.GetLength());

			return CPNG::Load(Buffer, m_Image);
			}

		default:
			{
			DWORD dwWidth;
			Stream.Read(&dwWidth, sizeof(DWORD));

			DWORD dwHeight;
			Stream.Read(&dwHeight, sizeof(DWORD));

			DWORD dwAlphaType;
			Stream.Read(&dwAlphaType, sizeof(DWORD));

			DWORD dwColor;
			Stream.Read(&dwColor, sizeof(DWORD));
			m_rgbBackground = CRGBA32(dwColor);
			
			DWORD dwBufferSize;
			Stream.Read(&dwBufferSize, sizeof(DWORD));

			CBuffer Buffer;
			Buffer.SetLength(dwBufferSize);
			Stream.Read(Buffer.GetPointer(), Buffer.GetLength());

			if (!CPNG::Load(Buffer, m_Image))
				return false;

			return true;
			}
		}
	}

void CAEONLuminousBitmap::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	}

void CAEONLuminousBitmap::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize to a structure.

	{
	switch (iFormat)
		{
		//	Always serialize to a PNG.

		case CDatum::EFormat::JSON:
			CPNG::Save(m_Image, Stream);
			break;

		//	Default we save a binary format

		default:
			{
			DWORD dwSave = m_Image.GetWidth();
			Stream.Write(&dwSave, sizeof(DWORD));

			dwSave = m_Image.GetHeight();
			Stream.Write(&dwSave, sizeof(DWORD));

			dwSave = (DWORD)m_Image.GetAlphaType();
			Stream.Write(&dwSave, sizeof(DWORD));

			dwSave = m_rgbBackground.AsDWORD();
			Stream.Write(&dwSave, sizeof(DWORD));

			//	PNG

			CBuffer PNG;
			CPNG::Save(m_Image, PNG);

			dwSave = PNG.GetLength();
			Stream.Write(&dwSave, sizeof(DWORD));

			Stream.Write(PNG);
			break;
			}
		}
	}
