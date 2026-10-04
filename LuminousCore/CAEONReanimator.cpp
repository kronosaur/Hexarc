//	CAEONReanimator.cpp
//
//	CAEONReanimator Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"
#include <cmath>

DECLARE_CONST_STRING(STR_CAEONREANIMATOR_POINTS,	"points");

DECLARE_CONST_STRING(FIELD_BACKGROUND,				"background");
DECLARE_CONST_STRING(FIELD_END_FRAME,				"endFrame");
DECLARE_CONST_STRING(FIELD_EXTENTS,					"extents");
DECLARE_CONST_STRING(FIELD_FPS,						"fps");
DECLARE_CONST_STRING(FIELD_FRAME,					"frame");
DECLARE_CONST_STRING(FIELD_FRAME_COUNT,				"frameCount");
DECLARE_CONST_STRING(FIELD_FROM,					"from");
DECLARE_CONST_STRING(FIELD_HEIGHT,					"height");
DECLARE_CONST_STRING(FIELD_ID,						"id");
DECLARE_CONST_STRING(FIELD_MODE,					"mode");
DECLARE_CONST_STRING(FIELD_OBJECTS,					"objects");
DECLARE_CONST_STRING(FIELD_ORIGIN,					"origin");
DECLARE_CONST_STRING(FIELD_PARENT_ID,				"parentID");
DECLARE_CONST_STRING(FIELD_SEQ,						"seq");
DECLARE_CONST_STRING(FIELD_TO,						"to");
DECLARE_CONST_STRING(FIELD_TYPE,					"type");
DECLARE_CONST_STRING(FIELD_VALUE,					"value");
DECLARE_CONST_STRING(FIELD_VALUES,					"values");
DECLARE_CONST_STRING(FIELD_WIDTH,					"width");

DECLARE_CONST_STRING(FIELD_ANIM,					"anim");
DECLARE_CONST_STRING(FIELD_FRAMES,					"frames");
DECLARE_CONST_STRING(FIELD_START_FRAME,				"startFrame");
DECLARE_CONST_STRING(FIELD_TYPES,					"types");
DECLARE_CONST_STRING(FIELD_INITIAL_POINTS,			"initialPoints");
DECLARE_CONST_STRING(FIELD_DIFFS,					"diffs");

DECLARE_CONST_STRING(ANIM_DENSE,					"dense");
DECLARE_CONST_STRING(ANIM_SPARSE,					"sparse");
DECLARE_CONST_STRING(ANIM_TRAIL_POINTS,				"trailPoints");

DECLARE_CONST_STRING(TYPENAME_REANIMATOR,			"reanimator");

DECLARE_CONST_STRING(OBJ_TYPE_TEXT, "text");
DECLARE_CONST_STRING(OBJ_TYPE_CIRCLE,				"circle");
DECLARE_CONST_STRING(OBJ_TYPE_RECTANGLE,			"rectangle");
DECLARE_CONST_STRING(OBJ_TYPE_LINE,				"line");
DECLARE_CONST_STRING(OBJ_TYPE_TRAIL,				"trail");

DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY,			"Unknown property: %s.");

static constexpr double DEFAULT_CIRCLE_RADIUS = 50.0;
static constexpr double DEFAULT_RECT_WIDTH = 100.0;
static constexpr double DEFAULT_RECT_HEIGHT = 100.0;
static constexpr double DEFAULT_LINE_LINE_WIDTH = 1.0;
static constexpr int DEFAULT_TRAIL_MAX_POINTS = 32;
static constexpr double DEFAULT_TRAIL_LINE_WIDTH = 1.0;
static constexpr double CENTI_PIXELS_PER_PIXEL = 100.0;

static Obj2DProp ParseObjProperty (const ILuminousObj2D* pObj, const CString& sProperty);

static bool IsOptionalTextScalar (Obj2DProp iProp)
	{
	return iProp == Obj2DProp::MaxWidth || iProp == Obj2DProp::MinFontSize || iProp == Obj2DProp::MaxFontSize;
	}

// Validate on a detached object before replacing a value or removing its track.
static bool ValidateTextValue (Obj2DProp iProp, CDatum dValue)
	{
	CLuminousScene2D ValidationScene;
	CObj2DText Candidate(ValidationScene, 0, NULL);
	switch (ILuminousObj2D::GetPropertyDesc(iProp).iType)
		{
		case ObjPropType::String:
			if (dValue.GetBasicType() != CDatum::typeString && !(iProp == Obj2DProp::Text && dValue.IsNil())) return false;
			return Candidate.SetPropertyString(iProp, dValue.AsString());
		case ObjPropType::Scalar:
			{
			double rValue = (IsOptionalTextScalar(iProp) && dValue.IsNil()) ? -1.0 : (double)dValue;
			if (!dValue.IsNil() && !dValue.IsNumber()) return false;
			if (!std::isfinite(rValue) || (!dValue.IsNil() && IsOptionalTextScalar(iProp) && rValue < 0.0)) return false;
			return Candidate.SetPropertyScalar(iProp, rValue);
			}
		case ObjPropType::Color: return Candidate.SetPropertyColor(iProp, CAEONLuminous::AsColor(dValue));
		case ObjPropType::Bool: return iProp == Obj2DProp::Visible;
		case ObjPropType::Vector:
			if (dValue.GetCount() != 2 || !dValue.GetElement(0).IsNumber() || !dValue.GetElement(1).IsNumber()) return false;
			if (!std::isfinite((double)dValue.GetElement(0)) || !std::isfinite((double)dValue.GetElement(1))) return false;
			return Candidate.SetPropertyVector(iProp, CVector2D(dValue.GetElement(0), dValue.GetElement(1)));
		default: return false;
		}
	}

TDatumPropertyHandler<CAEONReanimator> CAEONReanimator::m_Properties = {
	{
		"background",
		"?",
		"The background of the canvas",
		[](const CAEONReanimator &Obj, const CString &sProperty)
			{
			return CAEONLuminous::AsDatum(Obj.m_Model.GetBackgroundColor());
			},
		[](CAEONReanimator &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_Model.SetBackgroundColor(CAEONLuminous::AsColor(dValue));
			return true;
			},
		},
	{
		"extents",
		"?",
		"Gets/sets the logical extents for fit/stretch modes.",
		[](const CAEONReanimator &Obj, const CString &sProperty)
			{
			const CVector2D& v = Obj.m_Model.GetExtents();
			if (v.X() == 0.0 && v.Y() == 0.0)
				return CDatum();
			return CDatum(v);
			},
		[](CAEONReanimator &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			if (dValue.IsNil())
				Obj.m_Model.SetExtents(CVector2D());
			else
				Obj.m_Model.SetExtents(CVector2D(dValue.GetElement(0), dValue.GetElement(1)));
			return true;
			},
		},
	{
		"fps",
		"?",
		"Gets/sets the frames per second.",
		[](const CAEONReanimator &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_Model.GetFPS());
			},
		[](CAEONReanimator &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_Model.SetFPS((int)dValue);
			return true;
			},
		},
	{
		"mode",
		"?",
		"Sets the animation mode.",
		[](const CAEONReanimator &Obj, const CString &sProperty)
			{
			return CDatum(CLuminousScene2D::AsID(Obj.m_Model.GetMode()));
			},
		[](CAEONReanimator &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto iMode = CLuminousScene2D::AsMode(dValue.AsStringView());
			if (iMode == CLuminousScene2D::EMode::Unknown)
				{
				*retsError = strPattern(ERR_UNKNOWN_PROPERTY, dValue.AsString());
				return false;
				}

			Obj.m_Model.SetMode(iMode);
			return true;
			},
		},
	{
		"objects",
		"?",
		"Returns an array of all objects in the scene.",
		[](const CAEONReanimator &Obj, const CString &sProperty)
			{
			CDatum dSelf = CDatum::raw_AsComplex(const_cast<CAEONReanimator *>(&Obj));
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < Obj.m_Model.GetObjCount(); i++)
				{
				const ILuminousObj2D& ObjRef = Obj.m_Model.GetObj(i);
				if (strEquals(ObjRef.GetObjType(), OBJ_TYPE_TEXT))
					dResult.Append(CAEONText2D::Create(dSelf, ObjRef.GetID()));
				else if (strEquals(ObjRef.GetObjType(), OBJ_TYPE_RECTANGLE))
					dResult.Append(CAEONRect2D::Create(dSelf, ObjRef.GetID()));
				else if (strEquals(ObjRef.GetObjType(), OBJ_TYPE_CIRCLE))
					dResult.Append(CAEONCircle2D::Create(dSelf, ObjRef.GetID()));
				else if (strEquals(ObjRef.GetObjType(), OBJ_TYPE_LINE))
					dResult.Append(CAEONLine2D::Create(dSelf, ObjRef.GetID()));
				else if (strEquals(ObjRef.GetObjType(), OBJ_TYPE_TRAIL))
					dResult.Append(CAEONTrail2D::Create(dSelf, ObjRef.GetID()));
				}
			return dResult;
			},
		[](CAEONReanimator &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			//	Read-only property; ignore sets.
			return true;
			},
		},
	{
		"origin",
		"?",
		"Gets/sets the origin mode (\"center\", \"upperLeft\", \"lowerLeft\").",
		[](const CAEONReanimator &Obj, const CString &sProperty)
			{
			return CDatum(CLuminousScene2D::AsOriginID(Obj.m_Model.GetOrigin()));
			},
		[](CAEONReanimator &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			auto iOrigin = CLuminousScene2D::AsOrigin(dValue.AsStringView());
			if (iOrigin == CLuminousScene2D::EOrigin::Unknown)
				{
				*retsError = strPattern(ERR_UNKNOWN_PROPERTY, dValue.AsString());
				return false;
				}

			Obj.m_Model.SetOrigin(iOrigin);
			return true;
			},
		},
	};

TDatumMethodHandler<CAEONReanimator> CAEONReanimator::m_Methods = {
	{
		"createText",
		"$Text2DType:|desc=?",
		".createText(desc) -> Text2DType",
		0,
		[](CAEONReanimator& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.CreateTextObj(LocalEnv.GetArgument(0), LocalEnv.GetArgument(1));
			if (retResult.dResult.IsError())
				{
				retResult.iResult = CDatum::InvokeResult::error;
				return false;
				}
			return true;
			},
		},
	{
		"addKeyframe",
		"*",
		".addKeyframe(id, prop, frame, desc) -> true/false",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			DWORD dwID = ParseID(LocalEnv.GetArgument(1));
			ILuminousObj2D* pTarget = Obj.m_Model.FindObj(dwID);
			Obj2DProp iProp = ParseObjProperty(pTarget, LocalEnv.GetArgument(2).AsString());
			if (iProp == Obj2DProp::Unknown)
				{
				retResult.dResult = strPattern(ERR_UNKNOWN_PROPERTY, LocalEnv.GetArgument(2).AsString());
				return false;
				}

			int iFrame = LocalEnv.GetArgument(3);
			CDatum dDesc = LocalEnv.GetArgument(4);
			retResult.dResult = Obj.AnimateProperty(dwID, iProp, iFrame, dDesc);
			return true;
			},
		},
	{
		"createCircle",
		"$Circle2DType:|desc=?",
		".createCircle() -> Circle2DType\n"
		".createCircle(desc) -> Circle2DType",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dSelf = LocalEnv.GetArgument(0);

			if (LocalEnv.GetCount() == 1)
				{
				retResult.dResult = Obj.CreateCircleObj(dSelf);
				}
			else
				{
				CDatum dDesc = LocalEnv.GetArgument(1);
				retResult.dResult = Obj.CreateCircleObj(dSelf, dDesc);
				}

			return true;
			},
		},
	{
		"createRect",
		"$Rect2DType:|desc=?",
		".createRect() -> Rect2DType\n"
		".createRect(desc) -> Rect2DType",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dSelf = LocalEnv.GetArgument(0);

			if (LocalEnv.GetCount() == 1)
				{
				//	No arguments -- return a Rect2D object.
				retResult.dResult = Obj.CreateRectangleObj(dSelf);
				}
			else
				{
				CDatum dDesc = LocalEnv.GetArgument(1);
				retResult.dResult = Obj.CreateRectangleObj(dSelf, dDesc);
				}

			return true;
			},
		},
	{
		"createLine",
		"$Line2DType:|desc=?",
		".createLine() -> Line2DType\n"
		".createLine(desc) -> Line2DType",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dSelf = LocalEnv.GetArgument(0);

			if (LocalEnv.GetCount() == 1)
				retResult.dResult = Obj.CreateLineObj(dSelf);
			else
				retResult.dResult = Obj.CreateLineObj(dSelf, LocalEnv.GetArgument(1));

			return true;
			},
		},
	{
		"createTrail",
		"$Trail2DType:|desc=?",
		".createTrail() -> Trail2DType\n"
		".createTrail(desc) -> Trail2DType",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dSelf = LocalEnv.GetArgument(0);

			if (LocalEnv.GetCount() == 1)
				retResult.dResult = Obj.CreateTrailObj(dSelf);
			else
				retResult.dResult = Obj.CreateTrailObj(dSelf, LocalEnv.GetArgument(1));

			return true;
			},
		},
	{
		"renderHTMLCanvasCommands",
		"*",
		".renderHTMLCanvasCommands() -> desc",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dSeq = LocalEnv.GetArgument(1);
			retResult.dResult = Obj.RenderAsHTMLCanvasCommands((DWORDLONG)dSeq);
			return true;
			},
		},
	{
		"removeObject",
		"*",
		".removeObject(obj) -> true/false",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			DWORD dwID = ParseID(LocalEnv.GetArgument(1));
			retResult.dResult = CDatum(Obj.RemoveObj(dwID));
			return true;
			},
		},
	{
		"setKeyframe",
		"*",
		".setKeyframe(frame, type) -> true\n"
		".setKeyframe() -> true (clears keyframe mode)",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			if (LocalEnv.GetCount() == 1)
				{
				//	No arguments -- clear keyframe mode.
				Obj.m_Model.ClearKeyframe();
				}
			else
				{
				int iFrame = LocalEnv.GetArgument(1);
				IAnimator2D::Type iType = IAnimator2D::AsType(LocalEnv.GetArgument(2).AsStringView());
				if (iType == IAnimator2D::Type::Unknown)
					iType = IAnimator2D::Type::Linear;

				Obj.m_Model.SetKeyframe(iFrame, iType);
				}

			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"advanceFrame",
		"*",
		".advanceFrame() -> true\n"
		".advanceFrame(count) -> true",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int iCount = (LocalEnv.GetCount() > 1) ? (int)LocalEnv.GetArgument(1) : 1;
			Obj.m_Model.AdvanceFrame(iCount);
			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"trimKeyframes",
		"*",
		".trimKeyframes(frame) -> true",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int iFrame = LocalEnv.GetArgument(1);
			Obj.m_Model.TrimKeyframes(iFrame);
			retResult.dResult = CDatum(true);
			return true;
			},
		},
	{
		"setAt",
		"*",
		".setAt(id, property, value) -> true/false",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			DWORD dwID = ParseID(LocalEnv.GetArgument(1));
			ILuminousObj2D* pTarget = Obj.m_Model.FindObj(dwID);
			Obj2DProp iProp = ParseObjProperty(pTarget, LocalEnv.GetArgument(2).AsString());
			if (iProp == Obj2DProp::Unknown)
				{
				retResult.dResult = strPattern(ERR_UNKNOWN_PROPERTY, LocalEnv.GetArgument(2).AsString());
				return false;
				}

			CDatum dValue = LocalEnv.GetArgument(3);

			retResult.dResult = Obj.SetObjProperty(dwID, iProp, dValue);
			return true;
			},
		},
	};

static TArray<CVector2D> AsVectorQueue (CDatum dValue)
	{
	TArray<CVector2D> Result;
	Result.GrowToFit(dValue.GetCount());
	for (int i = 0; i < dValue.GetCount(); i++)
		{
		CDatum dPoint = dValue.GetElement(i);
		Result.Insert(CVector2D(dPoint.GetElement(0), dPoint.GetElement(1)));
		}
	return Result;
	}

static Obj2DProp ParseObjProperty (const ILuminousObj2D* pObj, const CString& sProperty)
	{
	if (pObj
			&& strEqualsNoCase(pObj->GetObjType(), OBJ_TYPE_LINE)
			&& strEqualsNoCase(sProperty, STR_CAEONREANIMATOR_POINTS))
		return Obj2DProp::LinePoints;

	return ILuminousObj2D::ParseProperty(sProperty);
	}

static CDatum AsCompactVectorQueueDatum (const TArray<CVector2D>& Points)
	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < Points.GetCount(); i++)
		{
		CDatum dPoint(CDatum::typeArray);
		dPoint.Append((int)mathRound(Points[i].X() * CENTI_PIXELS_PER_PIXEL));
		dPoint.Append((int)mathRound(Points[i].Y() * CENTI_PIXELS_PER_PIXEL));
		dResult.Append(dPoint);
		}
	return dResult;
	}

static bool IsKeyframeArrayValue (ObjPropType iPropType, CDatum dValue)
	{
	if (dValue.GetBasicType() != CDatum::typeArray && dValue.GetBasicType() != CDatum::typeTensor)
		return false;

	if (iPropType == ObjPropType::VectorQueue || iPropType == ObjPropType::VectorList)
		{
		if (dValue.GetCount() == 0)
			return false;

		return !dValue.GetElement(0).GetElement(FIELD_FRAME).IsNil();
		}

	return true;
	}

bool CAEONReanimator::AnimateProperty (DWORD dwID, Obj2DProp iProp, int iFrame, CDatum dDesc)
	{
	ILuminousObj2D* pObj = m_Model.FindObj(dwID);
	if (!pObj)
		return false;

	return AnimateProperty(*pObj, iProp, iFrame, dDesc);
	}

bool CAEONReanimator::AnimateProperty (ILuminousObj2D& Obj, Obj2DProp iProp, int iFrame, CDatum dDesc)

//	AnimateProperty
//
//	Adds an animation for the given property.

	{
	if (strEquals(Obj.GetObjType(), OBJ_TYPE_TEXT) && !ValidateTextValue(iProp, dDesc.GetElement(FIELD_VALUE))) return false;

	if (iFrame < 0)
		return false;

	IAnimator2D::Type iAnimationType = IAnimator2D::AsType(dDesc.GetElement(FIELD_TYPE).AsStringView());
	ObjPropType iPropType = ILuminousObj2D::GetPropertyDesc(iProp).iType;

	switch (iPropType)
		{
		case ObjPropType::Bool:
			{
			switch (iAnimationType)
				{
				case IAnimator2D::Type::Constant:
					Obj.AnimateBoolConstant(iProp, iFrame, !dDesc.GetElement(FIELD_VALUE).IsNil());
					break;

				default:
					return false;
				}
			break;
			}

		case ObjPropType::Color:
			{
			switch (iAnimationType)
				{
				case IAnimator2D::Type::Constant:
					Obj.AnimateColorConstant(iProp, iFrame, CAEONLuminous::AsColor(dDesc.GetElement(FIELD_VALUE)));
					break;

				default:
					return false;
				}
			break;
			}

		case ObjPropType::Scalar:
			{
			switch (iAnimationType)
				{
				case IAnimator2D::Type::Constant:
					Obj.AnimateScalarConstant(iProp, iFrame, (IsOptionalTextScalar(iProp) && dDesc.GetElement(FIELD_VALUE).IsNil()) ? -1.0 : (double)dDesc.GetElement(FIELD_VALUE));
					break;

				case IAnimator2D::Type::Linear:
					Obj.AnimateScalarLinear(iProp, iFrame, (IsOptionalTextScalar(iProp) && dDesc.GetElement(FIELD_VALUE).IsNil()) ? -1.0 : (double)dDesc.GetElement(FIELD_VALUE));
					break;

				default:
					return false;
				}
			break;
			}

		case ObjPropType::String:
			{
			switch (iAnimationType)
				{
				case IAnimator2D::Type::Constant:
					Obj.AnimateStringConstant(iProp, iFrame, dDesc.GetElement(FIELD_VALUE).AsStringView());
					break;

				default:
					return false;
				}
			break;
			}

		case ObjPropType::Vector:
			{
			switch (iAnimationType)
				{
				case IAnimator2D::Type::Constant:
					Obj.AnimateVectorConstant(iProp, iFrame, dDesc.GetElement(FIELD_VALUE));
					break;

				case IAnimator2D::Type::Linear:
					Obj.AnimateVectorLinear(iProp, iFrame, dDesc.GetElement(FIELD_VALUE));
					break;

				default:
					return false;
				}
			break;
			}

		case ObjPropType::VectorQueue:
		case ObjPropType::VectorList:
			{
			switch (iAnimationType)
				{
				case IAnimator2D::Type::Constant:
					Obj.AnimateVectorQueueConstant(iProp, iFrame, AsVectorQueue(dDesc.GetElement(FIELD_VALUE)));
					break;

				case IAnimator2D::Type::Linear:
					Obj.AnimateVectorQueueLinear(iProp, iFrame, AsVectorQueue(dDesc.GetElement(FIELD_VALUE)));
					break;

				default:
					return false;
				}
			break;
			}

		default:
			return false;
		}

	return true;
	}

CDatum CAEONReanimator::Create ()

//	Create
//
//	Creates an empty scene.

	{
	return CDatum(new CAEONReanimator);
	}

CDatum CAEONReanimator::CreateCircleObj (CDatum dSelf, CDatum dDesc)

//	CreateCircleObj
//
//	Creates a circle from a descriptor and returns a Circle2D object.
//	We set defaults first, then apply the descriptor so that any properties
//	in the descriptor override the defaults.
//
//	parentID: The ID of the parent object (or Nil if no parent)
//
//	fillColor: The fill color of the circle
//	lineColor: The color of the outline of the circle
//	lineWidth: The width of the outline of the circle
//	opacity: The opacity of the circle (0 to 1)
//	pos: A vector of the position of the circle
//	radius: The radius of the circle
//	rotation: The rotation of the circle (in radians)
//	rotationCenter: A vector of the rotation center of the circle
//	scale: A vector of the scale of the circle
//	visible: True if the circle is visible

	{
	ILuminousObj2D& Obj = m_Model.CreateCircle(dDesc.GetElement(FIELD_PARENT_ID));
	SetCircleDefaults(Obj);
	if (!dDesc.IsNil())
		SetObjProperties(Obj, dDesc);
	return CAEONCircle2D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator::CreateTextObj (CDatum dSelf, CDatum dDesc)
	{
	if (!dDesc.IsNil() && dDesc.GetBasicType() != CDatum::typeStruct)
		return CDatum::CreateError(CString("Text descriptor must be a structure."));

	DWORD dwParentID = ParseID(dDesc.GetElement(FIELD_PARENT_ID));
	if (dwParentID && !m_Model.FindObj(dwParentID))
		return CDatum::CreateError(CString("Unknown text parent."));

	ILuminousObj2D& Obj = m_Model.CreateText(dwParentID);
	for (int i = 0; i < dDesc.GetCount(); i++)
		{
		CString sKey = dDesc.GetKey(i);
		if (strEquals(sKey, FIELD_PARENT_ID)) continue;
		if (strEquals(sKey, CString("data"))) { SetObjData(Obj.GetID(), dDesc.GetElement(i)); continue; }
		Obj2DProp iProp = ILuminousObj2D::ParseProperty(sKey);
		if (iProp == Obj2DProp::Unknown || !SetObjProperty(Obj, iProp, dDesc.GetElement(i)))
			{
			RemoveObj(Obj.GetID());
			return CDatum::CreateError(strPattern("Invalid text property: %s.", sKey));
			}
		}
	return CAEONText2D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator::CreateRectangleObj (CDatum dSelf, CDatum dDesc)

//	CreateRectangleObj
//
//	Creates a rectangle from a descriptor and returns a Rect2D object.
//	We set defaults first, then apply the descriptor so that any properties
//	in the descriptor override the defaults.
// 
//	parentID: The ID of the parent object (or Nil if no parent)
//
//	cornerRadius: The corner radius of all corners
//	cornerRadiusBL: The corner radius of the bottom-left corner
//	cornerRadiusBR: The corner radius of the bottom-right corner
//	cornerRadiusTL: The corner radius of the top-left corner
//	cornerRadiusTR: The corner radius of the top-right corner
//	fillColor: The color of the rectangle
//	height: The height of the rectangle
//	lineColor: The color of the outline of the rectangle
//	lineWidth: The width of the outline of the rectangle
//	opacity: The opacity of the rectangle (0 to 1)
//	pos: A vector of the position of the rectangle
//	rotation: The rotation of the rectangle (in radians)
//	rotationCenter: A vector of the rotation center of the rectangle
//	scale: A vector of the scale of the rectangle
//	visible: True if the rectangle is visible
//	width: The width of the rectangle

	{
	ILuminousObj2D& Obj = m_Model.CreateRectangle(dDesc.GetElement(FIELD_PARENT_ID));
	SetRectangleDefaults(Obj);
	if (!dDesc.IsNil())
		SetObjProperties(Obj, dDesc);
	return CAEONRect2D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator::CreateLineObj (CDatum dSelf, CDatum dDesc)

//	CreateLineObj
//
//	Creates a line from a descriptor and returns a Line2D object.

	{
	ILuminousObj2D& Obj = m_Model.CreateLine(dDesc.GetElement(FIELD_PARENT_ID));
	SetLineDefaults(Obj);
	if (!dDesc.IsNil())
		SetObjProperties(Obj, dDesc);
	return CAEONLine2D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator::CreateTrailObj (CDatum dSelf, CDatum dDesc)

//	CreateTrailObj
//
//	Creates a trail from a descriptor and returns a Trail2D object.

	{
	ILuminousObj2D& Obj = m_Model.CreateTrail(dDesc.GetElement(FIELD_PARENT_ID));
	SetTrailDefaults(Obj);
	if (!dDesc.IsNil())
		SetObjProperties(Obj, dDesc);
	return CAEONTrail2D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator::GetDatatype () const 
	{
	return CAEONTypes::Get(CAEONLuminous::SCENE2D_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONReanimator::GetMembers (void)

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

CDatum CAEONReanimator::RenderConstProperty (const ILuminousObj2D& Obj, Obj2DProp iProp, ObjPropType iPropType) const

//	RenderConstProperty
//
//	Returns the object property.

	{
	switch (iPropType)
		{
		case ObjPropType::Bool:
			return CDatum(Obj.GetPropertyBool(iProp));

		case ObjPropType::Color:
			return CAEONLuminous::AsDatum(Obj.GetPropertyColor(iProp));

		case ObjPropType::Scalar:
			if (IsOptionalTextScalar(iProp) && Obj.GetPropertyScalar(iProp) < 0.0) return CDatum();
			return CDatum(Obj.GetPropertyScalar(iProp));

		case ObjPropType::String:
			return CDatum(Obj.GetPropertyString(iProp));

		case ObjPropType::Vector:
			return CDatum(Obj.GetPropertyVector(iProp));

		case ObjPropType::VectorQueue:
		case ObjPropType::VectorList:
			return AsCompactVectorQueueDatum(Obj.GetPropertyVectorQueue(iProp));

		default:
			throw CException(errFail);
		}
	}

size_t CAEONReanimator::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	return 0;
	}

bool CAEONReanimator::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	m_Model = CLuminousScene2D::CreateFromStream(Stream);

	DWORD dwDataCount = Stream.ReadDWORD();
	for (DWORD i = 0; i < dwDataCount; i++)
		{
		DWORD dwID = Stream.ReadDWORD();
		CDatum dData;
		CDatum::Deserialize(CDatum::EFormat::AEONBinaryLocal, Stream, NULL, &dData);
		m_ObjData.SetAt(dwID, dData);
		}

	return true;
	}

void CAEONReanimator::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	for (int i = 0; i < m_ObjData.GetCount(); i++)
		m_ObjData[i].Mark();
	}

void CAEONReanimator::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize to a stream.

	{
	m_Model.Write(Stream);

	Stream.Write((DWORD)m_ObjData.GetCount());
	for (int i = 0; i < m_ObjData.GetCount(); i++)
		{
		Stream.Write(m_ObjData.GetKey(i));
		m_ObjData[i].Serialize(CDatum::EFormat::AEONBinaryLocal, Stream);
		}
	}

void CAEONReanimator::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_Model = CLuminousScene2D::CreateFromStream(Stream);

	DWORD dwDataCount = Stream.ReadDWORD();
	for (DWORD i = 0; i < dwDataCount; i++)
		{
		DWORD dwID = Stream.ReadDWORD();
		CDatum dData = CDatum::DeserializeAEON(Stream, Serialized);
		m_ObjData.SetAt(dwID, dData);
		}
	}

void CAEONReanimator::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	m_Model.Write(Stream);

	Stream.Write((DWORD)m_ObjData.GetCount());
	for (int i = 0; i < m_ObjData.GetCount(); i++)
		{
		Stream.Write(m_ObjData.GetKey(i));
		m_ObjData[i].SerializeAEON(Stream, Serialized);
		}
	}

int CAEONReanimator::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	const IAEONReanimator* pOtherObj = dValue.GetReanimatorInterface();
	if (!pOtherObj)
		return KeyCompare(AsString(), dValue.AsString());

	CDatum dThis = RenderAsHTMLCanvasCommands(0);
	CDatum dOther = pOtherObj->RenderAsHTMLCanvasCommands(0);
	return dThis.OpCompare(dOther);
	}

bool CAEONReanimator::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if we are equal to dValue.

	{
	const IAEONReanimator* pOtherObj = dValue.GetReanimatorInterface();
	if (!pOtherObj)
		return false;

	CDatum dThis = RenderAsHTMLCanvasCommands(0);
	CDatum dOther = pOtherObj->RenderAsHTMLCanvasCommands(0);
	return dThis.OpIsEqual(dOther);
	}

CDatum CAEONReanimator::RenderAsHTMLCanvasCommands (SequenceNumber Seq) const

//	RenderAsHTMLCanvasCommands
//
//	Returns a datum that represents the commands to animate. The output is an
//	array of structures, where each structure represents an object. Each object
//	has a type and a set of properties. Each property is either a constant or
//	a structure with the .animate field set to the animation type.
//
//	Example:
//
//	{
//		width: 1920,
//		height: 1080,
//		origin: { x: 0, y: 0 },
//		background: { r:0, g:0, b:0 },
//		fps: 30,
//		frameCount: 120,
//		mode: "default",
//		seq: 117,
//		
//		objects: [
//			{
//			id: 1,
//			parentID: 0,
//			seq: 101,
//			type: "rectangle",
// 			pos: { x: 100, y: 100 },
//			width: [
//				{ type:"constant", frame:0, value:100 },
//				{ type:"linear", frame:100, value:10 },
//				{ type:"linear", frame:200, value:100 },
//				],
// 			height: 100,
// 			fillColor: { r:255, g:255, b:255 },
// 			},
//		...
//		],
//	}

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_ORIGIN, CLuminousScene2D::AsOriginID(m_Model.GetOrigin()));
	const CVector2D& vExtents = m_Model.GetExtents();
	if (vExtents.X() != 0.0 || vExtents.Y() != 0.0)
		{
		CDatum dExtents(CDatum::typeArray);
		dExtents.Append((int)mathRound(vExtents.X() * CENTI_PIXELS_PER_PIXEL));
		dExtents.Append((int)mathRound(vExtents.Y() * CENTI_PIXELS_PER_PIXEL));
		dResult.SetElement(FIELD_EXTENTS, dExtents);
		}
	dResult.SetElement(FIELD_BACKGROUND, CAEONLuminous::AsDatum(m_Model.GetBackgroundColor()));
	dResult.SetElement(FIELD_FPS, m_Model.GetFPS());
	dResult.SetElement(FIELD_FRAME_COUNT, m_Model.GetFrameCount());
	dResult.SetElement(FIELD_MODE, CLuminousScene2D::AsID(m_Model.GetMode()));
	dResult.SetElement(FIELD_SEQ, m_Model.GetSeq());

	CDatum dObjects(CDatum::typeArray);
	for (int i = 0; i < m_Model.GetObjCount(); i++)
		{
		const ILuminousObj2D& Obj = m_Model.GetObj(i);
		CDatum dObj = RenderObj(Obj);
		if (dObj.IsNil())
			continue;

		dObjects.Append(dObj);
		}

	dResult.SetElement(FIELD_OBJECTS, dObjects);

	return dResult;
	}

CDatum CAEONReanimator::CompactValue (ObjPropType iType, CDatum dValue)

//	CompactValue
//
//	For vectors, converts from CDatum(CVector2D) to a plain [x, y] array.
//	All other types pass through unchanged.

	{
	if (iType == ObjPropType::Vector)
		{
		CDatum dArray(CDatum::typeArray);
		dArray.Append((int)mathRound((double)dValue.GetElement(0) * CENTI_PIXELS_PER_PIXEL));
		dArray.Append((int)mathRound((double)dValue.GetElement(1) * CENTI_PIXELS_PER_PIXEL));
		return dArray;
		}

	return dValue;
	}

CDatum CAEONReanimator::RenderAnimatedProperty (const ILuminousObj2D& Obj, const IAnimator2D& Animator)

//	RenderAnimatedProperty
//
//	Returns the animation for the property. The output format depends on
//	the keyframe pattern:
//
//	Dense (all constant keyframes at consecutive frames):
//		{ anim:"dense", startFrame:N, values:[...] }
//
//	Sparse (mixed types or non-consecutive frames):
//		{ anim:"sparse", frames:[...], types:[...], values:[...] }

	{
	//	Get keyframe descriptors and validate value count.

	const auto& Frames = Animator.GetKeyframes();
	ObjPropType iPropType = Animator.GetPropertyType();
	auto GetValueCount = [&]() -> int
		{
		switch (iPropType)
			{
			case ObjPropType::Bool: return Animator.GetKeyframesBool().GetCount();
			case ObjPropType::Color: return Animator.GetKeyframesColor().GetCount();
			case ObjPropType::Scalar: return Animator.GetKeyframesScalar().GetCount();
			case ObjPropType::String: return Animator.GetKeyframesString().GetCount();
			case ObjPropType::Vector: return Animator.GetKeyframesVector().GetCount();
			case ObjPropType::VectorQueue: return Animator.GetKeyframesVectorQueue().GetCount();
			case ObjPropType::VectorList: return Animator.GetKeyframesVectorQueue().GetCount();
			default: throw CException(errFail);
			}
		};

	if (Frames.GetCount() != GetValueCount())
		throw CException(errFail);

	auto GetCompactValue = [&](int iIndex) -> CDatum
		{
		switch (iPropType)
			{
			case ObjPropType::Bool:
				return CompactValue(iPropType, CDatum(Animator.GetKeyframesBool()[iIndex]));

			case ObjPropType::Color:
				return CompactValue(iPropType, CAEONLuminous::AsDatum(Animator.GetKeyframesColor()[iIndex]));

			case ObjPropType::Scalar:
				if (IsOptionalTextScalar(Animator.GetProperty()) && Animator.GetKeyframesScalar()[iIndex] < 0.0) return CDatum();
				return CompactValue(iPropType, CDatum(Animator.GetKeyframesScalar()[iIndex]));

			case ObjPropType::String:
				return CompactValue(iPropType, CDatum(Animator.GetKeyframesString()[iIndex]));

			case ObjPropType::Vector:
				return CompactValue(iPropType, CDatum(Animator.GetKeyframesVector()[iIndex]));

			case ObjPropType::VectorQueue:
			case ObjPropType::VectorList:
				return CompactValue(iPropType, AsCompactVectorQueueDatum(Animator.GetKeyframesVectorQueue()[iIndex]));

			default:
				throw CException(errFail);
			}
		};

	//	Detect whether this qualifies as a dense animation (all constant
	//	keyframes at consecutive frame numbers).

	bool bDense = (Frames.GetCount() > 0);
	for (int i = 0; i < Frames.GetCount() && bDense; i++)
		{
		if (Frames[i].iType != IAnimator2D::Type::Constant)
			bDense = false;
		else if (i > 0 && Frames[i].iFrame != Frames[i - 1].iFrame + 1)
			bDense = false;
		}

	if (bDense)
		{
		if (iPropType == ObjPropType::VectorQueue
				&& strEqualsNoCase(Obj.GetObjType(), OBJ_TYPE_TRAIL))
			{
			const auto& Values = Animator.GetKeyframesVectorQueue();

			auto AsCompactPoint = [](const CVector2D& vPoint) -> CDatum
				{
				CDatum dPoint(CDatum::typeArray);
				dPoint.Append((int)mathRound(vPoint.X() * CENTI_PIXELS_PER_PIXEL));
				dPoint.Append((int)mathRound(vPoint.Y() * CENTI_PIXELS_PER_PIXEL));
				return dPoint;
				};

			auto CanTransition = [&](const TArray<CVector2D>& Prev, const TArray<CVector2D>& Cur, int& retRemoveCount, CDatum& retAddPoints) -> bool
				{
				for (int iRemoveCount = 0; iRemoveCount <= Prev.GetCount(); iRemoveCount++)
					{
					int iRetainedCount = Prev.GetCount() - iRemoveCount;
					if (iRetainedCount > Cur.GetCount())
						continue;

					bool bMatch = true;
					for (int i = 0; i < iRetainedCount; i++)
						{
						if (!(Prev[iRemoveCount + i] == Cur[i]))
							{
							bMatch = false;
							break;
							}
						}

					if (!bMatch)
						continue;

					retRemoveCount = iRemoveCount;
					retAddPoints = CDatum(CDatum::typeArray);
					for (int i = iRetainedCount; i < Cur.GetCount(); i++)
						retAddPoints.Append(AsCompactPoint(Cur[i]));

					return true;
					}

				return false;
				};

			CDatum dDiffs(CDatum::typeArray);
			bool bCanEncodeAsTrailPoints = true;
			for (int i = 1; i < Values.GetCount(); i++)
				{
				int iRemoveCount = 0;
				CDatum dAddPoints;
				if (!CanTransition(Values[i - 1], Values[i], iRemoveCount, dAddPoints))
					{
					bCanEncodeAsTrailPoints = false;
					break;
					}

				if (iRemoveCount > 0 || dAddPoints.GetCount() > 0)
					{
					//	Tuple diff format: [frame, removeCount, addPoints]
					CDatum dDiff(CDatum::typeArray);
					dDiff.Append(Frames[i].iFrame);
					dDiff.Append(iRemoveCount);
					dDiff.Append(dAddPoints);

					dDiffs.Append(dDiff);
					}
				}

			if (bCanEncodeAsTrailPoints)
				{
				CDatum dResult(CDatum::typeStruct);
				dResult.SetElement(FIELD_ANIM, ANIM_TRAIL_POINTS);
				dResult.SetElement(FIELD_START_FRAME, Frames[0].iFrame);
				dResult.SetElement(FIELD_INITIAL_POINTS, AsCompactVectorQueueDatum(Values[0]));
				dResult.SetElement(FIELD_DIFFS, dDiffs);
				return dResult;
				}
			}

		//	Dense format: { anim:"dense", startFrame:N, values:[...] }

		CDatum dValues(CDatum::typeArray);
		for (int i = 0; i < Frames.GetCount(); i++)
			dValues.Append(GetCompactValue(i));

		CDatum dResult(CDatum::typeStruct);
		dResult.SetElement(FIELD_ANIM, ANIM_DENSE);
		dResult.SetElement(FIELD_START_FRAME, Frames[0].iFrame);
		dResult.SetElement(FIELD_VALUES, dValues);
		return dResult;
		}
	else
		{
		//	Sparse format: { anim:"sparse", frames:[...], types:[...], values:[...] }

		CDatum dFrames(CDatum::typeArray);
		CDatum dTypes(CDatum::typeArray);
		CDatum dValues(CDatum::typeArray);

		for (int i = 0; i < Frames.GetCount(); i++)
			{
			dFrames.Append(CDatum(Frames[i].iFrame));

			int iTypeCode;
			switch (Frames[i].iType)
				{
				case IAnimator2D::Type::Linear:		iTypeCode = 1; break;
				case IAnimator2D::Type::Blink:		iTypeCode = 2; break;
				default:							iTypeCode = 0; break;
				}
			dTypes.Append(CDatum(iTypeCode));

			dValues.Append(GetCompactValue(i));
			}

		CDatum dResult(CDatum::typeStruct);
		dResult.SetElement(FIELD_ANIM, ANIM_SPARSE);
		dResult.SetElement(FIELD_FRAMES, dFrames);
		dResult.SetElement(FIELD_TYPES, dTypes);
		dResult.SetElement(FIELD_VALUES, dValues);
		return dResult;
		}
	}

bool CAEONReanimator::RemoveObj (DWORD dwID)

//	RemoveObj
//
//	Removes the given object from the scene and cleans up associated data.

	{
	if (!m_Model.RemoveObj(dwID))
		return false;

	m_ObjData.DeleteAt(dwID);
	return true;
	}

DWORD CAEONReanimator::ParseID (CDatum dValue)

//	ParseID
//
//	Returns an ID from either an integer or an object reference.

	{
	if (auto* pText = CAEONText2D::Upconvert(dValue))
		return pText->GetID();
	else if (auto* pCircle = CAEONCircle2D::Upconvert(dValue))
		return pCircle->GetID();
	else if (auto* pRect = CAEONRect2D::Upconvert(dValue))
		return pRect->GetID();
	else if (auto* pLine = CAEONLine2D::Upconvert(dValue))
		return pLine->GetID();
	else if (auto* pTrail = CAEONTrail2D::Upconvert(dValue))
		return pTrail->GetID();
	else
		return (DWORD)dValue;
	}

CDatum CAEONReanimator::RenderObj (const ILuminousObj2D& Obj) const

//	RenderObj
//
//	Returns a datum containing information required to render the object. If 
//	this object does not need to be drawn, we return Nil.

	{
	TArray<ILuminousObj2D::SPropertyRenderCtx> Props = Obj.GetPropertiesToRender();
	if (Props.GetCount() == 0)
		return CDatum();

	CDatum dResult(CDatum::typeStruct);

	dResult.SetElement(FIELD_ID, Obj.GetID());
	if (Obj.GetParent())
		dResult.SetElement(FIELD_PARENT_ID, Obj.GetParent()->GetID());
	dResult.SetElement(FIELD_SEQ, Obj.GetSeq());
	dResult.SetElement(FIELD_TYPE, Obj.GetObjType());

	for (int i = 0; i < Props.GetCount(); i++)
		{
		if (Props[i].pAnimator)
			{
			dResult.SetElement(Props[i].sID, RenderAnimatedProperty(Obj, *Props[i].pAnimator));
			}
		else
			{
			CDatum dInitialValue = RenderConstProperty(Obj, Props[i].iProp, Props[i].iType);
			dResult.SetElement(Props[i].sID, CompactValue(Props[i].iType, dInitialValue));
			}
		}

	return dResult;
	}

bool CAEONReanimator::SetObjProperty (DWORD dwID, Obj2DProp iProp, CDatum dValue)
	{
	ILuminousObj2D* pObj = m_Model.FindObj(dwID);
	if (!pObj)
		return false;

	SequenceNumber Seq = m_Model.GetSeq();
	bool bSuccess = SetObjProperty(*pObj, iProp, dValue);
	if (bSuccess && strEquals(pObj->GetObjType(), OBJ_TYPE_TEXT) && m_Model.GetSeq() == Seq)
		m_Model.OnObjModified(*pObj);
	return bSuccess;
	}

bool CAEONReanimator::SetObjProperty (ILuminousObj2D& Obj, Obj2DProp iProp, CDatum dValue)

//	SetObjProperty
//
//	Sets the given property. If we're in keyframe mode and the value is a
//	simple value (not an array of keyframe descriptors), we add a keyframe
//	instead of setting a constant.

	{
	//	If the value is an array, treat it as an array of keyframe descriptors
	//	(regardless of keyframe mode).

	ObjPropType iPropType = ILuminousObj2D::GetPropertyDesc(iProp).iType;

	if (!m_Model.IsStreamMode() && IsKeyframeArrayValue(iPropType, dValue))
		{
		if (strEquals(Obj.GetObjType(), OBJ_TYPE_TEXT))
			{
			for (int j = 0; j < dValue.GetCount(); j++)
				{
				CDatum dFrame = dValue.GetElement(j);
				auto iType = IAnimator2D::AsType(dFrame.GetElement(FIELD_TYPE).AsString());
				if ((int)dFrame.GetElement(FIELD_FRAME) < 0 || !ValidateTextValue(iProp, dFrame.GetElement(FIELD_VALUE))) return false;
				if (iType != IAnimator2D::Type::Constant && !(iType == IAnimator2D::Type::Linear && (iPropType == ObjPropType::Scalar || iPropType == ObjPropType::Vector))) return false;
				}
			}
		Obj.RemoveAnimation(iProp);

		for (int j = 0; j < dValue.GetCount(); j++)
			{
			CDatum dDesc = dValue.GetElement(j);
			int iFrame = dDesc.GetElement(FIELD_FRAME);

			if (!AnimateProperty(Obj, iProp, iFrame, dDesc))
				return false;
			}
		}

	//	If we're in keyframe mode, add a keyframe at the current frame pointer.

	else if (m_Model.IsKeyframeMode())
		{
		CDatum dDesc(CDatum::typeStruct);
		dDesc.SetElement(FIELD_TYPE, IAnimator2D::AsID(iPropType == ObjPropType::String ? IAnimator2D::Type::Constant : m_Model.GetKeyframeType()));
		dDesc.SetElement(FIELD_VALUE, dValue);

		return AnimateProperty(Obj, iProp, m_Model.GetKeyframeFrame(), dDesc);
		}

	//	Otherwise, set a constant value.

	else
		{
		if (strEquals(Obj.GetObjType(), OBJ_TYPE_TEXT) && !ValidateTextValue(iProp, dValue)) return false;
		if (!m_Model.IsStreamMode())
			Obj.RemoveAnimation(iProp);

		switch (iPropType)
			{
			case ObjPropType::Bool:
				return Obj.SetPropertyBool(iProp, !dValue.IsNil());

			case ObjPropType::Color:
				return Obj.SetPropertyColor(iProp, CAEONLuminous::AsColor(dValue));

			case ObjPropType::String:
				return Obj.SetPropertyString(iProp, dValue.AsString());

			case ObjPropType::Scalar:
				return Obj.SetPropertyScalar(iProp, (IsOptionalTextScalar(iProp) && dValue.IsNil()) ? -1.0 : (double)dValue);

			case ObjPropType::Vector:
				return Obj.SetPropertyVector(iProp, CVector2D(dValue.GetElement(0), dValue.GetElement(1)));

			case ObjPropType::VectorQueue:
			case ObjPropType::VectorList:
				return Obj.SetPropertyVectorQueue(iProp, AsVectorQueue(dValue));

			default:
				throw CException(errFail);
			}
		}

	return true;
	}

void CAEONReanimator::SetCircleDefaults (ILuminousObj2D& Obj)

//	SetCircleDefaults
//
//	Sets default property values for a circle so that it is visible
//	without any explicit property assignments.

	{
	Obj.SetPropertyScalar(Obj2DProp::Radius, DEFAULT_CIRCLE_RADIUS);
	Obj.SetPropertyColor(Obj2DProp::FillColor, CLuminousColor(CRGBA32(0x80, 0x80, 0x80)));
	}

void CAEONReanimator::SetRectangleDefaults (ILuminousObj2D& Obj)

//	SetRectangleDefaults
//
//	Sets default property values for a rectangle so that it is visible
//	without any explicit property assignments.

	{
	Obj.SetPropertyScalar(Obj2DProp::Width, DEFAULT_RECT_WIDTH);
	Obj.SetPropertyScalar(Obj2DProp::Height, DEFAULT_RECT_HEIGHT);
	Obj.SetPropertyColor(Obj2DProp::FillColor, CLuminousColor(CRGBA32(0x80, 0x80, 0x80)));
	}

void CAEONReanimator::SetLineDefaults (ILuminousObj2D& Obj)
	{
	Obj.SetPropertyScalar(Obj2DProp::LineWidth, DEFAULT_LINE_LINE_WIDTH);
	Obj.SetPropertyColor(Obj2DProp::LineColor, CLuminousColor(CRGBA32(0x80, 0x80, 0x80)));
	}

void CAEONReanimator::SetTrailDefaults (ILuminousObj2D& Obj)
	{
	Obj.SetPropertyScalar(Obj2DProp::MaxPoints, DEFAULT_TRAIL_MAX_POINTS);
	Obj.SetPropertyScalar(Obj2DProp::LineWidth, DEFAULT_TRAIL_LINE_WIDTH);
	Obj.SetPropertyColor(Obj2DProp::LineColor, CLuminousColor(CRGBA32(0x80, 0x80, 0x80)));
	}
bool CAEONReanimator::SetObjProperties (ILuminousObj2D& Obj, CDatum dData)

//	SetObjProperties
//
//	Sets the properties for the given object from a datum.

	{
	for (int i = 0; i < dData.GetCount(); i++)
		{
		CString sKey = dData.GetKey(i);
		Obj2DProp iProp = ParseObjProperty(&Obj, sKey);
		if (iProp == Obj2DProp::Unknown)
			continue;

		CDatum dValue = dData.GetElement(i);
		if (!SetObjProperty(Obj, iProp, dValue))
			return false;
		}

	return true;
	}

const CString& CAEONReanimator::StaticGetTypename (void) { return TYPENAME_REANIMATOR; }
