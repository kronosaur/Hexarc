//	CHTMLCanvasRemote.cpp
//
//	CHTMLCanvasRemote Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.
//
//	This class is used to encode/decode canvas drawing commands. In general we
//	encode commands in an array, where the first element of the array is an 
//	integer (representing the command) and the subsequent elements are 
//	parameters.
//
//	For commands with no parameters, we use an integer value instead of an 
//	array.

#include "pch.h"
#include "LuminousHTMLCanvas.h"

void CHTMLCanvasRemote::AccumulateHTMLCanvasCommands (const CLuminousFillStyle& Style, const CLuminousFillStyle& PrevStyle, CDatum dResult)

//	AccumulateHTMLCanvasCommands
//
//	Adds canvas commands to set a style.

	{
	auto& Color = Style.GetColor();
	if (Color != PrevStyle.GetColor())
		{
		switch (Color.GetType())
			{
			case CLuminousColor::EType::None:
				break;

			case CLuminousColor::EType::Solid:
				dResult.Append(CmdFillStyle(Color.GetSolidColor().AsHTMLColor()));
				break;

			default:
				throw CException(errFail);
			}
		}
	}

void CHTMLCanvasRemote::AccumulateHTMLCanvasCommands (const CLuminousLineStyle& Style, const CLuminousLineStyle& PrevStyle, CDatum dResult)

//	AccumulateHTMLCanvasCommands
//
//	Adds canvas commands to set a style.

	{
	auto& Color = Style.GetColor();
	if (Color != PrevStyle.GetColor())
		{
		switch (Color.GetType())
			{
			case CLuminousColor::EType::None:
				break;

			case CLuminousColor::EType::Solid:
				dResult.Append(CmdStrokeStyle(Color.GetSolidColor().AsHTMLColor()));
				break;

			default:
				throw CException(errFail);
			}
		}

	if (Style.GetLineCap() != PrevStyle.GetLineCap())
		{
		//	LATER
		throw CException(errFail);
		}

	if (Style.GetLineJoin() != PrevStyle.GetLineJoin())
		{
		//	LATER
		throw CException(errFail);
		}

	if (Style.GetLineWidth() != PrevStyle.GetLineWidth())
		{
		dResult.Append(CmdLineWidth(Style.GetLineWidth()));
		}

	if (Style.GetMiterLimit() != PrevStyle.GetMiterLimit())
		{
		//	LATER
		throw CException(errFail);
		}
	}

void CHTMLCanvasRemote::AccumulateHTMLCanvasCommands (const CLuminousPath2D& Path, CDatum dResult)

//	AccumulateHTMLCanvasCommands
//
//	Adds canvas commands to form path.

	{
	dResult.Append(CmdBeginPath());

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

				dResult.Append(CmdArc(vCenter.X(), vCenter.Y(), rRadius, rStartAngle, rEndAngle, bCounterClockwise));
				break;
				}

			case CLuminousPath2D::ESegmentType::ClosePath:
				dResult.Append(CmdClosePath());
				break;

			case CLuminousPath2D::ESegmentType::LineTo:
				{
				CVector2D vPos = Path.GetLineTo(i);
				dResult.Append(CmdLineTo(vPos.X(), vPos.Y()));
				break;
				}

			case CLuminousPath2D::ESegmentType::MoveTo:
				{
				CVector2D vPos = Path.GetMoveTo(i);
				dResult.Append(CmdMoveTo(vPos.X(), vPos.Y()));
				break;
				}

			case CLuminousPath2D::ESegmentType::Rect:
				{
				CVector2D vUL, vLR;
				Path.GetRect(i, vUL, vLR);

				dResult.Append(CmdMoveTo(vUL.X(), vUL.Y()));
				dResult.Append(CmdLineTo(vLR.X(), vUL.Y()));
				dResult.Append(CmdLineTo(vLR.X(), vLR.Y()));
				dResult.Append(CmdLineTo(vUL.X(), vLR.Y()));
				dResult.Append(CmdClosePath());
				break;
				}

			default:
				throw CException(errFail);
			}
		}
	}

void CHTMLCanvasRemote::AccumulateHTMLCanvasCommands (const CLuminousShadowStyle& Style, const CLuminousShadowStyle& PrevStyle, CDatum dResult)

//	AccumulateHTMLCanvasCommands
//
//	Adds canvas commands to set a style.

	{
	throw CException(errFail);
	}

CDatum CHTMLCanvasRemote::CmdArc (double x, double y, double radius, double startAngle, double endAngle, bool counterClock)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::arc);
	dResult.Append(x);
	dResult.Append(y);
	dResult.Append(radius);
	dResult.Append(startAngle);
	dResult.Append(endAngle);
	dResult.Append(counterClock);

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdBeginPath ()
	{
	return CDatum((int)ECommand::beginPath);
	}

CDatum CHTMLCanvasRemote::CmdClearRect ()
	{
	return CDatum((int)ECommand::clearAll);
	}

CDatum CHTMLCanvasRemote::CmdClearRect (double x, double y, double cxWidth, double cyHeight)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::clearRect);
	dResult.Append(x);
	dResult.Append(y);
	dResult.Append(cxWidth);
	dResult.Append(cyHeight);

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdClosePath ()
	{
	return CDatum((int)ECommand::closePath);
	}

CDatum CHTMLCanvasRemote::CmdDrawImage (CDatum dImage, double x, double y)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::drawImage);
	dResult.Append(dImage);
	dResult.Append(x);
	dResult.Append(y);

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdFill ()
	{
	return CDatum((int)ECommand::fill);
	}

CDatum CHTMLCanvasRemote::CmdFillRect (double x, double y, double cxWidth, double cyHeight)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::fillRect);
	dResult.Append(x);
	dResult.Append(y);
	dResult.Append(cxWidth);
	dResult.Append(cyHeight);

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdFillStyle (const CString &sStyle)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::fillStyle);
	dResult.Append(sStyle);

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdBeginUpdate ()
	{
	return CDatum((int)ECommand::beginUpdate);
	}

CDatum CHTMLCanvasRemote::CmdLineTo (double x, double y)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::lineTo);

	//	Encode in 1/10th of a pixel
	dResult.Append((int)mathRound(x * 10.0));
	dResult.Append((int)mathRound(y * 10.0));

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdLineWidth (double rWidth)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::lineWidth);
	dResult.Append(rWidth);

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdMoveTo (double x, double y)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::moveTo);
	dResult.Append((int)mathRound(x * 10.0));
	dResult.Append((int)mathRound(y * 10.0));

	return dResult;
	}

CDatum CHTMLCanvasRemote::CmdStroke ()
	{
	return CDatum((int)ECommand::stroke);
	}

CDatum CHTMLCanvasRemote::CmdStrokeStyle (const CString &sStyle)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)ECommand::strokeStyle);
	dResult.Append(sStyle);

	return dResult;
	}

CHTMLCanvasRemote::ECommand CHTMLCanvasRemote::GetCommand (CDatum dData)
	{
	if (dData.IsContainer())
		return (ECommand)(int)dData.GetElement(0);
	else
		return (ECommand)(int)dData;
	}

bool CHTMLCanvasRemote::IsClearAll (CDatum dData)
	{
	return (dData.GetCount() == 1) && ((int)dData == (int)ECommand::clearAll);
	}

bool CHTMLCanvasRemote::IsDrawCommand (ECommand iCmd)
	{
	switch (iCmd)
		{
		//	Rectangles

		case ECommand::rect:
		case ECommand::fillRect:
		case ECommand::strokeRect:
		case ECommand::clearRect:
		case ECommand::clearAll:

		//	Path Commands

		case ECommand::arc:
		case ECommand::arcTo:
		case ECommand::beginPath:
		case ECommand::bezierCurveTo:
		case ECommand::closePath:
		case ECommand::fill:
		case ECommand::lineTo:
		case ECommand::moveTo:
		case ECommand::quadraticCurveTo:
		case ECommand::stroke:

		//	Image Drawing

		case ECommand::drawImage:
			return true;

		default:
			return false;
		}
	}

CDatum CHTMLCanvasRemote::RenderAsHTMLCanvasCommands (const CLuminousCanvasModel& Model, const CLuminousCanvasResources& Resources, SequenceNumber Seq)

//	RenderAsHTMLCanvasCommands
//
//	Returns an array of canvas commands.

	{
	CDatum dResult(CDatum::typeArray);

	CLuminousPath2D CurPath;
	CLuminousFillStyle CurFillStyle;
	CLuminousLineStyle CurLineStyle;
	CLuminousShadowStyle CurShadowStyle;

	for (int i = 0; i < Model.GetRenderCount(); i++)
		{
		auto &Graphic = Model.GetRenderGraphic(i);

		switch (Graphic.GetType())
			{
			case ILuminousGraphic::EType::BeginUpdate:
				if (Graphic.GetSeq() > Seq)
					dResult.Append(CmdBeginUpdate());
				break;

			case ILuminousGraphic::EType::ClearRect:
				if (Graphic.GetSeq() > Seq)
					{
					auto &ShapePath = Graphic.GetShapePath();
					if (ShapePath.IsEmpty())
						continue;

					CVector2D vUL;
					CVector2D vLR;
					ShapePath.GetRect(0, vUL, vLR);
					dResult.Append(CmdClearRect(vUL.X(), vUL.Y(), vLR.X() - vUL.X(), vLR.Y() - vUL.Y()));
					}
				break;

			case ILuminousGraphic::EType::Image:
				if (Graphic.GetSeq() > Seq)
					{
					auto& vPos = Graphic.GetPos();
					CDatum dImage = Resources.GetResource(Graphic);

					dResult.Append(CmdDrawImage(dImage, vPos.X(), vPos.Y()));
					}
				break;

			case ILuminousGraphic::EType::Shape:
				{
				auto &ShapePath = Graphic.GetShapePath();
				if (ShapePath.IsEmpty())
					continue;

				auto &FillStyle = Graphic.GetFillStyle();
				auto &LineStyle = Graphic.GetLineStyle();
				auto &ShadowStyle = Graphic.GetShadowStyle();

				//	Generate

				if (Graphic.GetSeq() > Seq)
					{
					//	Apply styles.

					if (CurFillStyle != FillStyle)
						AccumulateHTMLCanvasCommands(FillStyle, CurFillStyle, dResult);

					if (CurLineStyle != LineStyle)
						AccumulateHTMLCanvasCommands(LineStyle, CurLineStyle, dResult);

					if (CurShadowStyle != ShadowStyle)
						AccumulateHTMLCanvasCommands(ShadowStyle, CurShadowStyle, dResult);

					//	If this path is different from the current path, then we need to 
					//	apply it.

					if (CurPath != ShapePath)
						AccumulateHTMLCanvasCommands(ShapePath, dResult);

					//	Now fill and/or stroke

					if (!FillStyle.IsEmpty())
						dResult.Append(CmdFill());

					if (!LineStyle.IsEmpty())
						dResult.Append(CmdStroke());
					}

				//	No matter what, we remember the style, because we need
				//	to know the state of the ctx for later graphics.

				CurFillStyle = FillStyle;
				CurLineStyle = LineStyle;
				CurShadowStyle = ShadowStyle;
				CurPath = ShapePath;
				break;
				}

			default:
				throw CException(errFail);
			}

		}

	return dResult;
	}
