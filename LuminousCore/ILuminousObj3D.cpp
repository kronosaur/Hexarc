//	ILuminousObj3D.cpp
//
//	ILuminousObj3D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

TArray<ILuminousObj3D::SPropertyDesc> ILuminousObj3D::m_Properties = std::initializer_list<ILuminousObj3D::SPropertyDesc> {
	{	Obj3DProp::Unknown,					Obj3DPropType::Unknown,		""	},
	{	Obj3DProp::Visible,					Obj3DPropType::Bool,		"visible"	},
	{	Obj3DProp::Opacity,					Obj3DPropType::Scalar,		"opacity"	},

	{	Obj3DProp::Pos,						Obj3DPropType::Vector,		"pos"	},
	{	Obj3DProp::Scale,					Obj3DPropType::Vector,		"scale"	},
	{	Obj3DProp::Rot,						Obj3DPropType::Scalar,		"rotation"	},
	{	Obj3DProp::RotCenter,				Obj3DPropType::Vector,		"rotationCenter"	},
};

TSortMap<CString, Obj3DProp> ILuminousObj3D::m_PropLookup;

bool ILuminousObj3D::AnimateBoolConstant (Obj3DProp iProp, int iFrame, bool bValue)

//	AnimateBoolConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Bool)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator3D& Animator = m_Animators.GetAnimatorBool(iProp, bValue);
		}
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorBool(iProp, GetPropertyBool(iProp));

		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator3D::Type::Constant;
		Animator.AddKeyframeBool(Desc, bValue);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateColorConstant (Obj3DProp iProp, int iFrame, const CLuminousColor& Value)

//	AnimateColorConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Color)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator3D& Animator = m_Animators.GetAnimatorColor(iProp, Value);
		}
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorColor(iProp, GetPropertyColor(iProp));

		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator3D::Type::Constant;
		Animator.AddKeyframeColor(Desc, Value);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateScalarConstant (Obj3DProp iProp, int iFrame, double rValue)

//	AnimateScalarConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Scalar)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator3D& Animator = m_Animators.GetAnimatorScalar(iProp, rValue);
		}
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorScalar(iProp, GetPropertyScalar(iProp));

		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator3D::Type::Constant;
		Animator.AddKeyframeScalar(Desc, rValue);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateScalarLinear (Obj3DProp iProp, int iFrame, double rValue)

//	AnimateScalarLinear
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Scalar)
		return false;

	IAnimator3D& Animator = m_Animators.GetAnimatorScalar(iProp, GetPropertyScalar(iProp));

	IAnimator3D::SKeyframeDesc Desc;
	Desc.iFrame = iFrame;
	Desc.iType = IAnimator3D::Type::Linear;
	Animator.AddKeyframeScalar(Desc, rValue);

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateStringConstant (Obj3DProp iProp, int iFrame, const CString& sValue)

//	AnimateStringConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::String)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator3D& Animator = m_Animators.GetAnimatorString(iProp, sValue);
		}
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorString(iProp, GetPropertyString(iProp));

		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator3D::Type::Linear;
		Animator.AddKeyframeString(Desc, sValue);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateVectorConstant (Obj3DProp iProp, int iFrame, const CVector3D& Value)

//	AnimateVectorConstant
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Vector)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		{
		//	This will add a constant animator if we don't already have one.
		IAnimator3D& Animator = m_Animators.GetAnimatorVector(iProp, Value);
		}
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorVector(iProp, GetPropertyVector(iProp));

		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = iFrame;
		Desc.iType = IAnimator3D::Type::Constant;
		Animator.AddKeyframeVector(Desc, Value);
		}

	m_Scene.OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateVectorLinear (Obj3DProp iProp, int iFrame, const CVector3D& Value)

//	AnimateVectorLinear
//
//	Animate the property.

	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Vector)
		return false;

	IAnimator3D& Animator = m_Animators.GetAnimatorVector(iProp, GetPropertyVector(iProp));

	IAnimator3D::SKeyframeDesc Desc;
	Desc.iFrame = iFrame;
	Desc.iType = IAnimator3D::Type::Linear;
	Animator.AddKeyframeVector(Desc, Value);

	m_Scene.OnObjModified(*this);
	return true;
	}

TUniquePtr<ILuminousObj3D> ILuminousObj3D::CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream, TSortMap<DWORD, DWORD> &retParents)

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

	TUniquePtr<ILuminousObj3D> pObj;
	switch (dwImpl)
		{
		//	LATER
		case IMPL_RECTANGLE:
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

	pObj->m_Animators = CAnimatorSet3D::CreateFromStream(Stream);

	pObj->OnRead(Stream);

	return pObj;
	}

TArray<ILuminousObj3D::SPropertyRenderCtx> ILuminousObj3D::GetPropertiesToRender () const

//	GetPropertiesToRender
//
//	Returns an array of properties for this object that are need to render.
//	This excludes any default properties.

	{
	TArray<SPropertyRenderCtx> Result;

	const IAnimator3D* pAnimator = NULL;

	//	If this object is not visible, then we don't need to render it.

	if (!m_bVisible && !(pAnimator = GetPropertyAnimator(Obj3DProp::Visible)))
		return Result;

	//	If default opacity, then no need to include it.

	if ((pAnimator = GetPropertyAnimator(Obj3DProp::Opacity)) || m_rOpacity != 1.0)
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Opacity), pAnimator, Result);

	//	Always add position.

	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Pos), GetPropertyAnimator(Obj3DProp::Pos), Result);

	//	Add scale if it is not the default.

	if ((pAnimator = GetPropertyAnimator(Obj3DProp::Scale)) || m_vScale != CVector3D(1.0, 1.0, 1.0))
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Scale), pAnimator, Result);

	//	Add rotation if it is not the default.

	if ((pAnimator = GetPropertyAnimator(Obj3DProp::Rot)) || m_rRotation != 0.0)
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Rot), pAnimator, Result);

	//	Add rotation center if it is not the default.

	if ((pAnimator = GetPropertyAnimator(Obj3DProp::RotCenter)) || m_vRotCenter != CVector3D(0.0, 0.0, 0.0))
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::RotCenter), pAnimator, Result);

	//	Now add object-specific properties

	OnAccumulatePropertiesToRender(Result);

	//	Done

	return Result;
	}

bool ILuminousObj3D::GetPropertyBool (Obj3DProp iProp) const
	{
	switch (iProp)
		{
		case Obj3DProp::Visible:
			return m_bVisible;

		default:
			return OnGetPropertyBool(iProp);
		}
	}

CLuminousColor ILuminousObj3D::GetPropertyColor (Obj3DProp iProp) const
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

const ILuminousObj3D::SPropertyDesc& ILuminousObj3D::GetPropertyDesc (Obj3DProp iProp)

//	GetPropertyDesc
//
//	Returns the property descriptor for the given property.

	{
	int iIndex = (int)iProp;
	if (iIndex < 1 || iIndex >= m_Properties.GetCount())
		throw CException(errFail);

	return m_Properties[iIndex];
	}

double ILuminousObj3D::GetPropertyScalar (Obj3DProp iProp) const
	{
	switch (iProp)
		{
		case Obj3DProp::Opacity:
			return m_rOpacity;

		case Obj3DProp::Rot:
			return m_rRotation;

		default:
			return OnGetPropertyScalar(iProp);
		}
	}

CString ILuminousObj3D::GetPropertyString (Obj3DProp iProp) const
	{
	return OnGetPropertyString(iProp);
	}

CVector3D ILuminousObj3D::GetPropertyVector (Obj3DProp iProp) const
	{
	switch (iProp)
		{
		case Obj3DProp::Pos:
			return m_vPos;

		case Obj3DProp::RotCenter:
			return m_vRotCenter;

		case Obj3DProp::Scale:
			return m_vScale;

		default:
			return OnGetPropertyVector(iProp);
		}
	}

Obj3DProp ILuminousObj3D::ParseProperty (const CString& sProperty)

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
		return Obj3DProp::Unknown;

	return *pProp;
	}

bool ILuminousObj3D::SetPropertyBool (Obj3DProp iProp, bool bValue)
	{
	switch (iProp)
		{
		case Obj3DProp::Visible:
			m_bVisible = bValue;
			return true;

		default:
			return OnSetPropertyBool(iProp, bValue);
		}
	}

bool ILuminousObj3D::SetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value)
	{
	return OnSetPropertyColor(iProp, Value);
	}

bool ILuminousObj3D::SetPropertyScalar (Obj3DProp iProp, double rValue)
	{
	switch (iProp)
		{
		case Obj3DProp::Opacity:
			if (rValue >= 0.0 && rValue <= 1.0)
				{
				m_rOpacity = rValue;
				return true;
				}
			else
				return false;

		case Obj3DProp::Rot:
			m_rRotation = rValue;
			return true;

		default:
			return OnSetPropertyScalar(iProp, rValue);
		}
	}

bool ILuminousObj3D::SetPropertyString (Obj3DProp iProp, const CString& sValue)
	{
	return OnSetPropertyString(iProp, sValue);
	}

bool ILuminousObj3D::SetPropertyVector (Obj3DProp iProp, const CVector3D& Value)
	{
	switch (iProp)
		{
		case Obj3DProp::Pos:
			m_vPos = Value;
			return true;

		case Obj3DProp::RotCenter:
			m_vRotCenter = Value;
			return true;

		case Obj3DProp::Scale:
			m_vScale = Value;
			return true;

		default:
			return OnSetPropertyVector(iProp, Value);
		}
	}

void ILuminousObj3D::Write (IByteStream& Stream) const

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
