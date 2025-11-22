//	ILuminousObj2D.cpp
//
//	ILuminousObj2D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

TArray<ILuminousObj2D::SPropertyDesc> ILuminousObj2D::m_Properties = std::initializer_list<ILuminousObj2D::SPropertyDesc> {
	{	Obj2DProp::Unknown,					ObjPropType::Unknown,		""	},
	{	Obj2DProp::Visible,					ObjPropType::Bool,			"visible"	},
	{	Obj2DProp::Opacity,					ObjPropType::Scalar,		"opacity"	},

	{	Obj2DProp::Pos,						ObjPropType::Vector,		"pos"	},
	{	Obj2DProp::Scale,					ObjPropType::Vector,		"scale"	},
	{	Obj2DProp::Rot,						ObjPropType::Scalar,		"rotation"	},
	{	Obj2DProp::RotCenter,				ObjPropType::Vector,		"rotationCenter"	},

	{	Obj2DProp::Height,					ObjPropType::Scalar,		"height"	},
	{	Obj2DProp::Radius,					ObjPropType::Scalar,		"radius"	},
	{	Obj2DProp::Width,					ObjPropType::Scalar,		"width"	},

	{	Obj2DProp::CornerRadius,			ObjPropType::Scalar,		"cornerRadius"	},
	{	Obj2DProp::CornerRadiusBottomLeft,	ObjPropType::Scalar,		"cornerRadiusBL"	},
	{	Obj2DProp::CornerRadiusBottomRight,	ObjPropType::Scalar,		"cornerRadiusBR"	},
	{	Obj2DProp::CornerRadiusTopLeft,		ObjPropType::Scalar,		"cornerRadiusTL"	},
	{	Obj2DProp::CornerRadiusTopRight,	ObjPropType::Scalar,		"cornerRadiusTR"	},
	{	Obj2DProp::FillColor,				ObjPropType::Color,			"fillColor"	},
	{	Obj2DProp::LineColor,				ObjPropType::Color,			"lineColor"	},
	{	Obj2DProp::LineWidth,				ObjPropType::Scalar,		"lineWidth"	},
};

TSortMap<CString, Obj2DProp> ILuminousObj2D::m_PropLookup;

bool ILuminousObj2D::AnimateBoolConstant (Obj2DProp iProp, int iFrame, bool bValue)

//	AnimateBoolConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != ObjPropType::Bool)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator2D& Animator = m_Animators.GetAnimatorBool(iProp, bValue);
		}
	else
		{
		IAnimator2D& Animator = m_Animators.GetAnimatorBool(iProp, GetPropertyBool(iProp));

		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator2D::Type::Constant;
		Animator.AddKeyframeBool(Desc, bValue);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj2D::AnimateColorConstant (Obj2DProp iProp, int iFrame, const CLuminousColor& Value)

//	AnimateColorConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != ObjPropType::Color)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator2D& Animator = m_Animators.GetAnimatorColor(iProp, Value);
		}
	else
		{
		IAnimator2D& Animator = m_Animators.GetAnimatorColor(iProp, GetPropertyColor(iProp));

		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator2D::Type::Constant;
		Animator.AddKeyframeColor(Desc, Value);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj2D::AnimateScalarConstant (Obj2DProp iProp, int iFrame, double rValue)

//	AnimateScalarConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != ObjPropType::Scalar)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator2D& Animator = m_Animators.GetAnimatorScalar(iProp, rValue);
		}
	else
		{
		IAnimator2D& Animator = m_Animators.GetAnimatorScalar(iProp, GetPropertyScalar(iProp));

		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator2D::Type::Constant;
		Animator.AddKeyframeScalar(Desc, rValue);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj2D::AnimateScalarLinear (Obj2DProp iProp, int iFrame, double rValue)

//	AnimateScalarLinear
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != ObjPropType::Scalar)
		return false;

	IAnimator2D& Animator = m_Animators.GetAnimatorScalar(iProp, GetPropertyScalar(iProp));

	IAnimator2D::SKeyframeDesc Desc;
	Desc.iFrame = iFrame;
	Desc.iType = IAnimator2D::Type::Linear;
	Animator.AddKeyframeScalar(Desc, rValue);

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj2D::AnimateStringConstant (Obj2DProp iProp, int iFrame, const CString& sValue)

//	AnimateStringConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != ObjPropType::String)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator2D& Animator = m_Animators.GetAnimatorString(iProp, sValue);
		}
	else
		{
		IAnimator2D& Animator = m_Animators.GetAnimatorString(iProp, GetPropertyString(iProp));

		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator2D::Type::Linear;
		Animator.AddKeyframeString(Desc, sValue);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj2D::AnimateVectorConstant (Obj2DProp iProp, int iFrame, const CVector2D& Value)

//	AnimateVectorConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != ObjPropType::Vector)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator2D& Animator = m_Animators.GetAnimatorVector(iProp, Value);
		}
	else
		{
		IAnimator2D& Animator = m_Animators.GetAnimatorVector(iProp, GetPropertyVector(iProp));

		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator2D::Type::Constant;
		Animator.AddKeyframeVector(Desc, Value);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj2D::AnimateVectorLinear (Obj2DProp iProp, int iFrame, const CVector2D& Value)

//	AnimateVectorLinear
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != ObjPropType::Vector)
		return false;

	IAnimator2D& Animator = m_Animators.GetAnimatorVector(iProp, GetPropertyVector(iProp));

	IAnimator2D::SKeyframeDesc Desc;
	Desc.iFrame = iFrame;
	Desc.iType = IAnimator2D::Type::Linear;
	Animator.AddKeyframeVector(Desc, Value);

	m_Scene.OnObjModified(*this);
	return true;
	}

TUniquePtr<ILuminousObj2D> ILuminousObj2D::CreateFromStream (CLuminousScene2D& Scene, IByteStream& Stream, TSortMap<DWORD, DWORD> &retParents)

//	CreateFromStream
//
//	Creates an object from a stream.

	{
	DWORD dwImpl = Stream.ReadDWORD();
	DWORD dwID = Stream.ReadDWORD();
	SequenceNumber Seq = Stream.ReadDWORDLONG();
	DWORD dwParentID = Stream.ReadDWORD();
	if (dwParentID)
		retParents.SetAt(dwID, dwParentID);

	TUniquePtr<ILuminousObj2D> pObj;
	switch (dwImpl)
		{
		case IMPL_RECTANGLE:
			pObj.Set(new CObj2DRectangle(Scene, dwID, NULL));
			break;

		default:
			throw CException(errFail);
		}

	pObj->m_Seq = Seq;

	DWORD dwFlags = Stream.ReadDWORD();
	pObj->m_bVisible = ((dwFlags & 0x00000001) ? true : false);

	pObj->m_vPos.Read(Stream);
	pObj->m_vScale.Read(Stream);
	pObj->m_vRotCenter.Read(Stream);
	pObj->m_rRotation = Stream.ReadDouble();
	pObj->m_rOpacity = Stream.ReadDouble();

	pObj->m_Animators = CAnimatorSet2D::CreateFromStream(Stream);

	pObj->OnRead(Stream);

	return pObj;
	}

TArray<ILuminousObj2D::SPropertyRenderCtx> ILuminousObj2D::GetPropertiesToRender () const

//	GetPropertiesToRender
//
//	Returns an array of properties for this object that are need to render.
//	This excludes any default properties.

	{
	TArray<SPropertyRenderCtx> Result;

	const IAnimator2D* pAnimator = NULL;

	//	If this object is not visible, then we don't need to render it.

	if (!m_bVisible && !(pAnimator = GetPropertyAnimator(Obj2DProp::Visible)))
		return Result;

	//	If default opacity, then no need to include it.

	if ((pAnimator = GetPropertyAnimator(Obj2DProp::Opacity)) || m_rOpacity != 1.0)
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Opacity), pAnimator, Result);

	//	Always add position.

	AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Pos), GetPropertyAnimator(Obj2DProp::Pos), Result);

	//	Add scale if it is not the default.

	if ((pAnimator = GetPropertyAnimator(Obj2DProp::Scale)) || m_vScale != CVector2D(1.0, 1.0))
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Scale), pAnimator, Result);

	//	Add rotation if it is not the default.

	if ((pAnimator = GetPropertyAnimator(Obj2DProp::Rot)) || m_rRotation != 0.0)
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::Rot), pAnimator, Result);

	//	Add rotation center if it is not the default.

	if ((pAnimator = GetPropertyAnimator(Obj2DProp::RotCenter)) || m_vRotCenter != CVector2D(0.0, 0.0))
		AccumulatePropertyToRender(GetPropertyDesc(Obj2DProp::RotCenter), pAnimator, Result);

	//	Now add object-specific properties

	OnAccumulatePropertiesToRender(Result);

	//	Done

	return Result;
	}

bool ILuminousObj2D::GetPropertyBool (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::Visible:
			return m_bVisible;

		default:
			return OnGetPropertyBool(iProp);
		}
	}

CLuminousColor ILuminousObj2D::GetPropertyColor (Obj2DProp iProp) const
	{
#if 0
	switch (iProp)
		{
		default:
			return OnGetPropertyColor(iProp);
		}
#else
	return OnGetPropertyColor(iProp);
#endif
	}

const ILuminousObj2D::SPropertyDesc& ILuminousObj2D::GetPropertyDesc (Obj2DProp iProp)

//	GetPropertyDesc
//
//	Returns the property descriptor for the given property.

	{
	int iIndex = (int)iProp;
	if (iIndex < 1 || iIndex >= m_Properties.GetCount())
		throw CException(errFail);

	return m_Properties[iIndex];
	}

double ILuminousObj2D::GetPropertyScalar (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::Opacity:
			return m_rOpacity;

		case Obj2DProp::Rot:
			return m_rRotation;

		default:
			return OnGetPropertyScalar(iProp);
		}
	}

CString ILuminousObj2D::GetPropertyString (Obj2DProp iProp) const
	{
	return OnGetPropertyString(iProp);
	}

CVector2D ILuminousObj2D::GetPropertyVector (Obj2DProp iProp) const
	{
	switch (iProp)
		{
		case Obj2DProp::Pos:
			return m_vPos;

		case Obj2DProp::RotCenter:
			return m_vRotCenter;

		case Obj2DProp::Scale:
			return m_vScale;

		default:
			return OnGetPropertyVector(iProp);
		}
	}

Obj2DProp ILuminousObj2D::ParseProperty (const CString& sProperty)

//	ParseProperty
//
//	Parses the given property.

	{
	if (m_PropLookup.GetCount() == 0)
		{
		for (int i = 0; i < m_Properties.GetCount(); i++)
			m_PropLookup.Insert(strToLower(m_Properties[i].sID), m_Properties[i].iProp);
		}

	auto *pProp = m_PropLookup.GetAt(strToLower(sProperty));
	if (pProp == NULL)
		return Obj2DProp::Unknown;

	return *pProp;
	}

bool ILuminousObj2D::SetPropertyBool (Obj2DProp iProp, bool bValue)
	{
	switch (iProp)
		{
		case Obj2DProp::Visible:
			m_bVisible = bValue;
			return true;

		default:
			return OnSetPropertyBool(iProp, bValue);
		}
	}

bool ILuminousObj2D::SetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value)
	{
	return OnSetPropertyColor(iProp, Value);
	}

bool ILuminousObj2D::SetPropertyScalar (Obj2DProp iProp, double rValue)
	{
	switch (iProp)
		{
		case Obj2DProp::Opacity:
			if (rValue >= 0.0 && rValue <= 1.0)
				{
				m_rOpacity = rValue;
				return true;
				}
			else
				return false;

		case Obj2DProp::Rot:
			m_rRotation = rValue;
			return true;

		default:
			return OnSetPropertyScalar(iProp, rValue);
		}
	}

bool ILuminousObj2D::SetPropertyString (Obj2DProp iProp, const CString& sValue)
	{
	return OnSetPropertyString(iProp, sValue);
	}

bool ILuminousObj2D::SetPropertyVector (Obj2DProp iProp, const CVector2D& Value)
	{
	switch (iProp)
		{
		case Obj2DProp::Pos:
			m_vPos = Value;
			return true;

		case Obj2DProp::RotCenter:
			m_vRotCenter = Value;
			return true;

		case Obj2DProp::Scale:
			m_vScale = Value;
			return true;

		default:
			return OnSetPropertyVector(iProp, Value);
		}
	}

void ILuminousObj2D::Write (IByteStream& Stream) const

//	Write
//
//	Write to stream.

	{
	Stream.Write(GetImpl());
	Stream.Write(GetID());
	Stream.Write(m_Seq);
	if (m_pParent)
		Stream.Write(m_pParent->GetID());
	else
		Stream.Write((DWORD)0);

	DWORD dwFlags = 0;
	dwFlags |= (m_bVisible ? 0x00000001 : 0);
	Stream.Write(dwFlags);

	m_vPos.Write(Stream);
	m_vScale.Write(Stream);
	m_vRotCenter.Write(Stream);
	Stream.Write(m_rRotation);
	Stream.Write(m_rOpacity);

	m_Animators.Write(Stream);

	OnWrite(Stream);
	}
