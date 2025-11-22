//	CAEONLuminousBitmap.cpp
//
//	CAEONLuminousBitmap Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(TYPENAME_LUMINOUS_BITMAP,		"luminousBitmap");

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_IMAGE,	"Unable to create image.");

TDatumPropertyHandler<CAEONLuminousBitmap> CAEONLuminousBitmap::m_Properties = {
	{
		"height",
		"I",
		"Returns the height of the image (in pixels).",
		[](const CAEONLuminousBitmap &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_Image.GetHeight());
			},
		NULL,
		},
	{
		"width",
		"I",
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
		".arc(x, y, radius, startAngle, endAngle[, counterclockwise]) -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			//	LATER

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"closePath",
		"*",
		".closePath() -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			Obj.m_DrawCtx.Path().ClosePath();

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"drawImage",
		"*",
		".drawImage(image, x, y) -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dImage = LocalEnv.GetArgument(1);
			double x = LocalEnv.GetArgument(2);
			double y = LocalEnv.GetArgument(3);

			//	LATER

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"fill",
		"*",
		".fill() -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.FillStyle = Obj.m_DrawCtx.Fill();

			//	LATER

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"fillRect",
		"*",
		".fillRect(x, y, width, height) -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			double x = LocalEnv.GetArgument(1);
			double y = LocalEnv.GetArgument(2);
			double cx = LocalEnv.GetArgument(3);
			double cy = LocalEnv.GetArgument(4);

			//	LATER: Use a graphics library.

			CImageDraw::Rectangle(Obj.m_Image, (int)x, (int)y, (int)cx, (int)cy, Obj.m_DrawCtx.Fill().GetColor().GetSolidColor());

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"getScaled",
		"*",
		".getScaled(width, height) -> image",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int cx = (int)LocalEnv.GetArgument(1);
			int cy = (int)LocalEnv.GetArgument(2);

			if (cx <= 0 || cy <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CRGBA32Image Result;
			if (!Result.Create(cx, cy, Obj.m_Image.GetAlphaType(), CRGBA32(0, 0, 0, 0)))
				{
				retResult.dResult = ERR_UNABLE_TO_CREATE_IMAGE;
				return false;
				}

			CImageDraw::CopyScaled(Result, 0, 0, cx, cy, Obj.m_Image);
			retResult.dResult = CAEONLuminousBitmap::Create(std::move(Result));
			return true;
			},
		},
	{
		"getSlice",
		"*",
		".getSlice(x, y, width, height) -> image",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int x = (int)LocalEnv.GetArgument(1);
			int y = (int)LocalEnv.GetArgument(2);
			int cx = Min((int)LocalEnv.GetArgument(3), Obj.m_Image.GetWidth() - x);
			int cy = Min((int)LocalEnv.GetArgument(4), Obj.m_Image.GetHeight() - y);

			if (cx <= 0 || cy <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CRGBA32Image Result;
			if (!Result.Create(cx, cy, Obj.m_Image.GetAlphaType()))
				{
				retResult.dResult = ERR_UNABLE_TO_CREATE_IMAGE;
				return false;
				}

			CImageDraw::Copy(Result, 0, 0, Obj.m_Image, x, y, cx, cy);
			retResult.dResult = CAEONLuminousBitmap::Create(std::move(Result));
			return true;
			},
		},
	{
		"lineTo",
		"*",
		".lineTo(x, y) -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
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
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CLuminousCanvasModel::SShapeOptions Options;
			Options.Path = Obj.m_DrawCtx.Path();
			Options.LineStyle = Obj.m_DrawCtx.Line();

			//	LATER

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"rect",
		"*",
		".rect(x, y, width, height) -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
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
		".setFillStyle(style) -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
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
		".setLineStyle(style) -> true/false",
		0,
		[](CAEONLuminousBitmap& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dStyle = LocalEnv.GetArgument(1);

			Obj.m_DrawCtx.Line().SetColor(CLuminousColor(CRGBA32::Parse(dStyle.AsString())));

			retResult.dResult = CDatum(true);
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
			
			if ((int)dwWidth > 0 && (int)dwHeight > 0)
				{
				DWORD dwBufferSize;
				Stream.Read(&dwBufferSize, sizeof(DWORD));

				CBuffer Buffer;
				Buffer.SetLength(dwBufferSize);
				Stream.Read(Buffer.GetPointer(), Buffer.GetLength());

				if (!CPNG::Load(Buffer, m_Image))
					return false;
				}

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

			//	PNG (but only if non-zero, because this code doesn't seem to
			//	handle 0-sized bitmaps).

			if (m_Image.GetHeight() > 0 && m_Image.GetWidth() > 0)
				{
				CBuffer PNG;
				CPNG::Save(m_Image, PNG);

				dwSave = PNG.GetLength();
				Stream.Write(&dwSave, sizeof(DWORD));

				Stream.Write(PNG);
				}
			break;
			}
		}
	}

int CAEONLuminousBitmap::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	switch (iValueType)
		{
		case CDatum::typeImage32:
			{
			const CRGBA32Image& Other = dValue;
			if (m_Image.GetWidth() < Other.GetWidth())
				return -1;
			else if (m_Image.GetWidth() > Other.GetWidth())
				return 1;
			else if (m_Image.GetHeight() < Other.GetHeight())
				return -1;
			else if (m_Image.GetHeight() > Other.GetHeight())
				return 1;
			else
				{
				CRGBA32* pSrcRow = m_Image.GetPixelPos(0, 0);
				CRGBA32* pSrcRowEnd = m_Image.GetPixelPos(0, m_Image.GetHeight());
				CRGBA32* pDestRow = Other.GetPixelPos(0, 0);
				while (pSrcRow < pSrcRowEnd)
					{
					CRGBA32* pSrc = pSrcRow;
					CRGBA32* pSrcEnd = pSrcRow + m_Image.GetWidth();
					CRGBA32* pDest = pDestRow;
					while (pSrc < pSrcEnd)
						{
						if (*pSrc < *pDest)
							return -1;
						else if (*pSrc > *pDest)
							return 1;
											
						pSrc++;
						pDest++;
						}
					
					pSrcRow = m_Image.NextRow(pSrcRow);
					pDestRow = Other.NextRow(pDestRow);
					}

				return 0;
				}
			}
		
		default:
			return KeyCompare(AsString(), dValue.AsString());
		}
	}

void CAEONLuminousBitmap::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	DWORD dwWidth = Stream.ReadDWORD();
	DWORD dwHeight = Stream.ReadDWORD();
	DWORD dwAlphaType = Stream.ReadDWORD();

	DWORD dwColor = Stream.ReadDWORD();
	m_rgbBackground = CRGBA32(dwColor);
			
	if ((int)dwWidth > 0 && (int)dwHeight > 0)
		{
		DWORD dwBufferSize = Stream.ReadDWORD();

		CBuffer Buffer;
		Buffer.SetLength(dwBufferSize);
		Stream.Read(Buffer.GetPointer(), Buffer.GetLength());

		if (!CPNG::Load(Buffer, m_Image))
			return;
		}
	}

void CAEONLuminousBitmap::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	Stream.Write(m_Image.GetWidth());
	Stream.Write(m_Image.GetHeight());
	Stream.Write((DWORD)m_Image.GetAlphaType());
	Stream.Write(m_rgbBackground.AsDWORD());

	//	PNG (but only if non-zero, because this code doesn't seem to
	//	handle 0-sized bitmaps).

	if (m_Image.GetHeight() > 0 && m_Image.GetWidth() > 0)
		{
		CBuffer PNG;
		CPNG::Save(m_Image, PNG);

		Stream.Write(PNG.GetLength());
		Stream.Write(PNG);
		}
	}
