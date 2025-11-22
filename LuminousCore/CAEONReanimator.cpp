//	CAEONReanimator.cpp
//
//	CAEONReanimator Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(FIELD_BACKGROUND,				"background");
DECLARE_CONST_STRING(FIELD_CUR_TIME,				"curTime");
DECLARE_CONST_STRING(FIELD_END_FRAME,				"endFrame");
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
DECLARE_CONST_STRING(FIELD_START_FRAME,				"startFrame");
DECLARE_CONST_STRING(FIELD_START_TIME,				"startTime");
DECLARE_CONST_STRING(FIELD_TO,						"to");
DECLARE_CONST_STRING(FIELD_TYPE,					"type");
DECLARE_CONST_STRING(FIELD_VALUE,					"value");
DECLARE_CONST_STRING(FIELD_WIDTH,					"width");

DECLARE_CONST_STRING(TYPENAME_REANIMATOR,			"reanimator");

DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY,			"Unknown property: %s.");

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
		"startFrame",
		"?",
		"Gets/sets the current start frame.",
		[](const CAEONReanimator &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_Model.GetStartFrame());
			},
		[](CAEONReanimator &Obj, const CString &sProperty, CDatum dValue, CString *retsError)
			{
			Obj.m_Model.SetStartFrame(dValue);
			return true;
			},
		},
	};

TDatumMethodHandler<CAEONReanimator> CAEONReanimator::m_Methods = {

	{
		"addKeyframe",
		"*",
		".addKeyframe(id, prop, frame, desc) -> true/false",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			DWORD dwID = (DWORD)LocalEnv.GetArgument(1);
			Obj2DProp iProp = ILuminousObj2D::ParseProperty(LocalEnv.GetArgument(2).AsStringView());
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
		"createRect",
		"*",
		".createRect(desc) -> id\n"
		".createRect(parentID, x, y, width, height) -> id",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			if (LocalEnv.GetCount() == 2)
				{
				CDatum dDesc = LocalEnv.GetArgument(1);
				retResult.dResult = Obj.CreateRectangle(dDesc);
				}
			else
				{
				DWORD dwParentID = (DWORD)LocalEnv.GetArgument(1);
				double rX = LocalEnv.GetArgument(2);
				double rY = LocalEnv.GetArgument(3);
				double rWidth = LocalEnv.GetArgument(4);
				double rHeight = LocalEnv.GetArgument(5);
				retResult.dResult = Obj.CreateRectangle(dwParentID, rX, rY, rWidth, rHeight);
				}

			return true;
			},
		},
	{
		"play",
		"*",
		".play(startFrame)",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int iStartFrame = LocalEnv.GetArgument(1);
			Obj.Play(iStartFrame);
			retResult.dResult = CDatum(true);
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
		"setAt",
		"*",
		".setAt(id, property, value) -> true/false",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			DWORD dwID = (DWORD)LocalEnv.GetArgument(1);
			Obj2DProp iProp = ILuminousObj2D::ParseProperty(LocalEnv.GetArgument(2).AsStringView());
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
	{
		"stop",
		"*",
		".stop()",
		0,
		[](CAEONReanimator &Obj, IInvokeCtx &Ctx, const CString &sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			Obj.Stop();
			retResult.dResult = CDatum(true);
			return true;
			},
		},
	};

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
	if (iFrame < 0)
		return false;

	IAnimator2D::Type iAnimationType = IAnimator2D::AsType(dDesc.GetElement(FIELD_TYPE).AsStringView());

	switch (ILuminousObj2D::GetPropertyDesc(iProp).iType)
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
					Obj.AnimateScalarConstant(iProp, iFrame, dDesc.GetElement(FIELD_VALUE));
					break;

				case IAnimator2D::Type::Linear:
					Obj.AnimateScalarLinear(iProp, iFrame, dDesc.GetElement(FIELD_VALUE));
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

DWORD CAEONReanimator::CreateRectangle (CDatum dDesc)

//	CreateRectangle
//
//	Creates a rectangle from a descriptor and returns an ID (or 0 if error).
//	A descriptor has the following fields:
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
	SetObjProperties(Obj, dDesc);

	return Obj.GetID();
	}

DWORD CAEONReanimator::CreateRectangle (DWORD dwParentID, double rX, double rY, double rWidth, double rHeight)

//	CreateRectangle
//
//	Creates a rectangle and returns an ID (or 0 if error).

	{
	ILuminousObj2D& Obj = m_Model.CreateRectangle(dwParentID);
	Obj.SetPropertyVector(Obj2DProp::Pos, CVector2D(rX, rY));
	Obj.SetPropertyScalar(Obj2DProp::Width, rWidth);
	Obj.SetPropertyScalar(Obj2DProp::Height, rHeight);

	return Obj.GetID();
	}

CDatum CAEONReanimator::GetDatatype () const 
	{
	return CAEONTypeSystem::GetCoreType(CAEONLuminous::SCENE2D_TYPE);
	}

CDatum CAEONReanimator::GetObjProperty (const ILuminousObj2D& Obj, Obj2DProp iProp, ObjPropType iPropType) const

//	GetObjProperty
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
			return CDatum(Obj.GetPropertyScalar(iProp));

		case ObjPropType::String:
			return CDatum(Obj.GetPropertyString(iProp));

		case ObjPropType::Vector:
			return CDatum(Obj.GetPropertyVector(iProp));

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
	return true;
	}

void CAEONReanimator::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	}

void CAEONReanimator::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize to a stream.

	{
	m_Model.Write(Stream);
	}

void CAEONReanimator::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_Model = CLuminousScene2D::CreateFromStream(Stream);
	}

void CAEONReanimator::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	m_Model.Write(Stream);
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
//		startTime: 11101,		// millisecond tick
//		curTime: 11234			// millisecond tick,
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
	dResult.SetElement(FIELD_WIDTH, m_Model.GetWidth());
	dResult.SetElement(FIELD_HEIGHT, m_Model.GetHeight());
	dResult.SetElement(FIELD_ORIGIN, m_Model.GetOrigin());
	dResult.SetElement(FIELD_BACKGROUND, CAEONLuminous::AsDatum(m_Model.GetBackgroundColor()));
	dResult.SetElement(FIELD_FPS, m_Model.GetFPS());
	dResult.SetElement(FIELD_FRAME_COUNT, m_Model.GetFrameCount());
	dResult.SetElement(FIELD_MODE, CLuminousScene2D::AsID(m_Model.GetMode()));
	dResult.SetElement(FIELD_START_FRAME, m_Model.GetStartFrame());
	dResult.SetElement(FIELD_START_TIME, m_Model.GetStartTime());
	dResult.SetElement(FIELD_CUR_TIME, m_Model.GetCurTime());
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

CDatum CAEONReanimator::GetAnimation (const ILuminousObj2D& Obj, const IAnimator2D& Animator)

//	GetAnimation
//
//	Returns the animation for the property in an array.

	{
	//	Get the values based on the property type

	TArray<CDatum> Values;
	switch (Animator.GetPropertyType())
		{
		case ObjPropType::Bool:
			{
			const auto& BoolValues = Animator.GetKeyframesBool();
			for (int i = 0; i < BoolValues.GetCount(); i++)
				Values.Insert(CDatum(BoolValues[i]));
			break;
			}

		case ObjPropType::Color:
			{
			const auto& ColorValues = Animator.GetKeyframesColor();
			for (int i = 0; i < ColorValues.GetCount(); i++)
				Values.Insert(CAEONLuminous::AsDatum(ColorValues[i]));
			break;
			}

		case ObjPropType::Scalar:
			{
			const auto& ScalarValues = Animator.GetKeyframesScalar();
			for (int i = 0; i < ScalarValues.GetCount(); i++)
				Values.Insert(CDatum(ScalarValues[i]));
			break;
			}

		case ObjPropType::String:
			{
			const auto& StringValues = Animator.GetKeyframesString();
			for (int i = 0; i < StringValues.GetCount(); i++)
				Values.Insert(CDatum(StringValues[i]));
			break;
			}

		case ObjPropType::Vector:
			{
			const auto& VectorValues = Animator.GetKeyframesVector();
			for (int i = 0; i < VectorValues.GetCount(); i++)
				Values.Insert(CDatum(VectorValues[i]));
			break;
			}

		default:
			throw CException(errFail);
		}

	//	Loop over all frames

	const auto& Frames = Animator.GetKeyframes();
	if (Frames.GetCount() != Values.GetCount())
		throw CException(errFail);

	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < Frames.GetCount(); i++)
		{
		const auto& Frame = Frames[i];
		
		CDatum dFrame(CDatum::typeStruct);
		dFrame.SetElement(FIELD_TYPE, IAnimator2D::AsID(Frame.iType));
		dFrame.SetElement(FIELD_FRAME, Frame.iFrame);
		dFrame.SetElement(FIELD_VALUE, Values[i]);
		
		dResult.Append(dFrame);
		}

	return dResult;
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
			dResult.SetElement(Props[i].sID, GetAnimation(Obj, *Props[i].pAnimator));
			}
		else
			{
			CDatum dInitialValue = GetObjProperty(Obj, Props[i].iProp, Props[i].iType);
			dResult.SetElement(Props[i].sID, dInitialValue);
			}
		}

	return dResult;
	}

bool CAEONReanimator::SetObjProperty (DWORD dwID, Obj2DProp iProp, CDatum dValue)
	{
	ILuminousObj2D* pObj = m_Model.FindObj(dwID);
	if (!pObj)
		return false;

	return SetObjProperty(*pObj, iProp, dValue);
	}

bool CAEONReanimator::SetObjProperty (ILuminousObj2D& Obj, Obj2DProp iProp, CDatum dValue)

//	SetObjProperty
//
//	Sets the given property.

	{
	Obj.RemoveAnimation(iProp);

	if (dValue.GetBasicType() == CDatum::typeArray || dValue.GetBasicType() == CDatum::typeTensor)
		{
		int iLastFrame = -1;
		for (int j = 0; j < dValue.GetCount(); j++)
			{
			CDatum dDesc = dValue.GetElement(j);
			int iFrame = dDesc.GetElement(FIELD_FRAME);

			AnimateProperty(Obj, iProp, iFrame, dDesc);
			}
		}

	//	Set a constant value

	else
		{
		switch (ILuminousObj2D::GetPropertyDesc(iProp).iType)
			{
			case ObjPropType::Bool:
				Obj.SetPropertyBool(iProp, !dValue.IsNil());
				break;

			case ObjPropType::Color:
				Obj.SetPropertyColor(iProp, CAEONLuminous::AsColor(dValue));
				break;

			case ObjPropType::Scalar:
				Obj.SetPropertyScalar(iProp, dValue);
				break;

			case ObjPropType::Vector:
				Obj.SetPropertyVector(iProp, CVector2D(dValue.GetElement(0), dValue.GetElement(1)));
				break;

			default:
				throw CException(errFail);
			}
		}

	return true;
	}

bool CAEONReanimator::SetObjProperties (ILuminousObj2D& Obj, CDatum dData)

//	SetObjProperties
//
//	Sets the properties for the given object from a datum.

	{
	for (int i = 0; i < dData.GetCount(); i++)
		{
		CString sKey = dData.GetKey(i);
		Obj2DProp iProp = ILuminousObj2D::ParseProperty(sKey);
		if (iProp == Obj2DProp::Unknown)
			continue;

		CDatum dValue = dData.GetElement(i);
		if (!SetObjProperty(Obj, iProp, dValue))
			return false;
		}

	return true;
	}

const CString& CAEONReanimator::StaticGetTypename (void) { return TYPENAME_REANIMATOR; }
