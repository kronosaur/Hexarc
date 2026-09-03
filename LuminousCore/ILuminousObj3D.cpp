//	ILuminousObj3D.cpp
//
//	ILuminousObj3D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_CUBE, "cube");
DECLARE_CONST_STRING(TYPE_MODEL_3DS, "3ds");
DECLARE_CONST_STRING(TYPE_MODEL_GLTF, "gltf");
DECLARE_CONST_STRING(TYPE_POINT_LIGHT, "pointLight");
DECLARE_CONST_STRING(TYPE_PERSPECTIVE_CAMERA, "perspectiveCamera");

TArray<ILuminousObj3D::SPropertyDesc> ILuminousObj3D::m_Properties = std::initializer_list<ILuminousObj3D::SPropertyDesc> {
	{ Obj3DProp::Unknown, Obj3DPropType::Unknown, "" },
	{ Obj3DProp::Visible, Obj3DPropType::Bool, "visible" },
	{ Obj3DProp::Opacity, Obj3DPropType::Scalar, "opacity" },
	{ Obj3DProp::Pos, Obj3DPropType::Vector, "pos" },
	{ Obj3DProp::Scale, Obj3DPropType::Vector, "scale" },
	{ Obj3DProp::Rot, Obj3DPropType::Vector, "rotation" },
	{ Obj3DProp::Color, Obj3DPropType::Color, "color" },
	{ Obj3DProp::Intensity, Obj3DPropType::Scalar, "intensity" },
	{ Obj3DProp::Distance, Obj3DPropType::Scalar, "distance" },
	{ Obj3DProp::Decay, Obj3DPropType::Scalar, "decay" },
	{ Obj3DProp::FOV, Obj3DPropType::Scalar, "fov" },
	{ Obj3DProp::Near, Obj3DPropType::Scalar, "near" },
	{ Obj3DProp::Far, Obj3DPropType::Scalar, "far" },
	{ Obj3DProp::GridID, Obj3DPropType::String, "gridID" },
	{ Obj3DProp::ResourcePath, Obj3DPropType::String, "resourcePath" },
};

TSortMap<CString, Obj3DProp> ILuminousObj3D::m_PropLookup;

bool ILuminousObj3D::AnimateBoolConstant (Obj3DProp iProp, int iFrame, bool bValue)
	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Bool)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		m_Animators.GetAnimatorBool(iProp, bValue);
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorBool(iProp, GetPropertyBool(iProp));
		Animator.AddKeyframeBool({ iFrame, IAnimator3D::Type::Constant }, bValue);
		}

	m_pScene->OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateColorConstant (Obj3DProp iProp, int iFrame, const CLuminousColor& Value)
	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Color)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		m_Animators.GetAnimatorColor(iProp, Value);
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorColor(iProp, GetPropertyColor(iProp));
		Animator.AddKeyframeColor({ iFrame, IAnimator3D::Type::Constant }, Value);
		}
	m_pScene->OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateScalarConstant (Obj3DProp iProp, int iFrame, double rValue)
	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Scalar)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		m_Animators.GetAnimatorScalar(iProp, rValue);
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorScalar(iProp, GetPropertyScalar(iProp));
		Animator.AddKeyframeScalar({ iFrame, IAnimator3D::Type::Constant }, rValue);
		}
	m_pScene->OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateScalarLinear (Obj3DProp iProp, int iFrame, double rValue)
	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Scalar)
		return false;

	//	The first keyframe establishes the value at frame 0. Its interpolation
	//	type is immaterial because interpolation is selected by the destination
	//	keyframe, so avoid creating two entries at the same frame.

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		m_Animators.GetAnimatorScalar(iProp, rValue);
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorScalar(iProp, GetPropertyScalar(iProp));
		Animator.AddKeyframeScalar({ iFrame, IAnimator3D::Type::Linear }, rValue);
		}
	m_pScene->OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateStringConstant (Obj3DProp iProp, int iFrame, const CString& sValue)
	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::String)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		m_Animators.GetAnimatorString(iProp, sValue);
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorString(iProp, GetPropertyString(iProp));
		Animator.AddKeyframeString({ iFrame, IAnimator3D::Type::Constant }, sValue);
		}
	m_pScene->OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateVectorConstant (Obj3DProp iProp, int iFrame, const CVector3D& Value)
	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Vector)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		m_Animators.GetAnimatorVector(iProp, Value);
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorVector(iProp, GetPropertyVector(iProp));
		Animator.AddKeyframeVector({ iFrame, IAnimator3D::Type::Constant }, Value);
		}
	m_pScene->OnObjModified(*this);
	return true;
	}

bool ILuminousObj3D::AnimateVectorLinear (Obj3DProp iProp, int iFrame, const CVector3D& Value)
	{
	if (GetPropertyDesc(iProp).iType != Obj3DPropType::Vector)
		return false;

	if (iFrame == 0 && !m_Animators.FindAnimator(iProp))
		m_Animators.GetAnimatorVector(iProp, Value);
	else
		{
		IAnimator3D& Animator = m_Animators.GetAnimatorVector(iProp, GetPropertyVector(iProp));
		Animator.AddKeyframeVector({ iFrame, IAnimator3D::Type::Linear }, Value);
		}
	m_pScene->OnObjModified(*this);
	return true;
	}

TUniquePtr<ILuminousObj3D> ILuminousObj3D::CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream, TSortMap<DWORD, DWORD>& retParents)
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
		case IMPL_CUBE:
			pObj.Set(new CObj3DCube(Scene, dwID, NULL));
			break;
		case IMPL_POINT_LIGHT:
			pObj.Set(new CObj3DPointLight(Scene, dwID, NULL));
			break;
		case IMPL_PERSPECTIVE_CAMERA:
			pObj.Set(new CObj3DPerspectiveCamera(Scene, dwID, NULL));
			break;
		case IMPL_MODEL_3DS:
			pObj.Set(new CObj3DModel3DS(Scene, dwID, NULL));
			break;
		case IMPL_MODEL_GLTF:
			pObj.Set(new CObj3DModelGLTF(Scene, dwID, NULL));
			break;
		default:
			throw CException(errFail);
		}

	pObj->m_Seq = Seq;
	DWORD dwFlags = Stream.ReadDWORD();
	pObj->m_bVisible = ((dwFlags & 0x00000001) != 0);
	pObj->m_vPos.Read(Stream);
	pObj->m_vScale.Read(Stream);
	pObj->m_vRotation.Read(Stream);
	pObj->m_rOpacity = Stream.ReadDouble();
	pObj->m_Animators = CAnimatorSet3D::CreateFromStream(Stream);
	pObj->OnRead(Stream);
	return pObj;
	}

TArray<ILuminousObj3D::SPropertyRenderCtx> ILuminousObj3D::GetPropertiesToRender () const
	{
	TArray<SPropertyRenderCtx> Result;
	const IAnimator3D* pAnimator;

	if ((pAnimator = GetPropertyAnimator(Obj3DProp::Visible)) || !m_bVisible)
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Visible), pAnimator, Result);
	if ((pAnimator = GetPropertyAnimator(Obj3DProp::Opacity)) || m_rOpacity != 1.0)
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Opacity), pAnimator, Result);

	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Pos), GetPropertyAnimator(Obj3DProp::Pos), Result);
	if ((pAnimator = GetPropertyAnimator(Obj3DProp::Scale)) || m_vScale != CVector3D(1.0, 1.0, 1.0))
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Scale), pAnimator, Result);
	if ((pAnimator = GetPropertyAnimator(Obj3DProp::Rot)) || m_vRotation != CVector3D())
		AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Rot), pAnimator, Result);

	OnAccumulatePropertiesToRender(Result);
	return Result;
	}

bool ILuminousObj3D::GetPropertyBool (Obj3DProp iProp) const
	{
	return (iProp == Obj3DProp::Visible ? m_bVisible : OnGetPropertyBool(iProp));
	}

CLuminousColor ILuminousObj3D::GetPropertyColor (Obj3DProp iProp) const
	{
	return OnGetPropertyColor(iProp);
	}

double ILuminousObj3D::GetPropertyScalar (Obj3DProp iProp) const
	{
	return (iProp == Obj3DProp::Opacity ? m_rOpacity : OnGetPropertyScalar(iProp));
	}

CString ILuminousObj3D::GetPropertyString (Obj3DProp iProp) const
	{
	return OnGetPropertyString(iProp);
	}

CVector3D ILuminousObj3D::GetPropertyVector (Obj3DProp iProp) const
	{
	switch (iProp)
		{
		case Obj3DProp::Pos: return m_vPos;
		case Obj3DProp::Scale: return m_vScale;
		case Obj3DProp::Rot: return m_vRotation;
		default: return OnGetPropertyVector(iProp);
		}
	}

const ILuminousObj3D::SPropertyDesc& ILuminousObj3D::GetPropertyDesc (Obj3DProp iProp)
	{
	int iIndex = (int)iProp;
	if (iIndex < 1 || iIndex >= m_Properties.GetCount())
		throw CException(errFail);
	return m_Properties[iIndex];
	}

Obj3DProp ILuminousObj3D::ParseProperty (const CString& sProperty)
	{
	if (m_PropLookup.GetCount() == 0)
		for (int i = 0; i < m_Properties.GetCount(); i++)
			m_PropLookup.Insert(strToLower(m_Properties[i].sID), m_Properties[i].iProp);

	auto* pProp = m_PropLookup.GetAt(strToLower(sProperty));
	return (pProp ? *pProp : Obj3DProp::Unknown);
	}

bool ILuminousObj3D::SetPropertyBool (Obj3DProp iProp, bool bValue)
	{
	bool bSuccess;
	if (iProp == Obj3DProp::Visible)
		{
		m_bVisible = bValue;
		bSuccess = true;
		}
	else
		bSuccess = OnSetPropertyBool(iProp, bValue);

	if (bSuccess)
		{
		if (m_pScene->IsStreamMode()) MarkPropertyDirty(iProp);
		else m_pScene->OnObjModified(*this);
		}
	return bSuccess;
	}

bool ILuminousObj3D::SetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value)
	{
	bool bSuccess = OnSetPropertyColor(iProp, Value);
	if (bSuccess)
		{
		if (m_pScene->IsStreamMode()) MarkPropertyDirty(iProp);
		else m_pScene->OnObjModified(*this);
		}
	return bSuccess;
	}

bool ILuminousObj3D::SetPropertyScalar (Obj3DProp iProp, double rValue)
	{
	bool bSuccess;
	if (iProp == Obj3DProp::Opacity)
		{
		bSuccess = (rValue >= 0.0 && rValue <= 1.0);
		if (bSuccess) m_rOpacity = rValue;
		}
	else
		bSuccess = OnSetPropertyScalar(iProp, rValue);

	if (bSuccess)
		{
		if (m_pScene->IsStreamMode()) MarkPropertyDirty(iProp);
		else m_pScene->OnObjModified(*this);
		}
	return bSuccess;
	}

bool ILuminousObj3D::SetPropertyString (Obj3DProp iProp, const CString& sValue)
	{
	bool bSuccess = OnSetPropertyString(iProp, sValue);
	if (bSuccess)
		{
		//	GridID and ResourcePath select browser resources; they are scene
		//	configuration, not frame samples. Keep them constant even while other
		//	properties stream.

		if (m_pScene->IsStreamMode() && iProp != Obj3DProp::GridID && iProp != Obj3DProp::ResourcePath) MarkPropertyDirty(iProp);
		else m_pScene->OnObjModified(*this);
		}
	return bSuccess;
	}

bool ILuminousObj3D::SetPropertyVector (Obj3DProp iProp, const CVector3D& Value)
	{
	bool bSuccess = true;
	switch (iProp)
		{
		case Obj3DProp::Pos: m_vPos = Value; break;
		case Obj3DProp::Scale: m_vScale = Value; break;
		case Obj3DProp::Rot: m_vRotation = Value; break;
		default: bSuccess = OnSetPropertyVector(iProp, Value); break;
		}

	if (bSuccess)
		{
		if (m_pScene->IsStreamMode()) MarkPropertyDirty(iProp);
		else m_pScene->OnObjModified(*this);
		}
	return bSuccess;
	}

void ILuminousObj3D::Write (IByteStream& Stream) const
	{
	Stream.Write(GetImpl());
	Stream.Write(GetID());
	Stream.Write(m_Seq);
	Stream.Write(m_pParent ? m_pParent->GetID() : (DWORD)0);
	Stream.Write((DWORD)(m_bVisible ? 0x00000001 : 0));
	m_vPos.Write(Stream);
	m_vScale.Write(Stream);
	m_vRotation.Write(Stream);
	Stream.Write(m_rOpacity);
	m_Animators.Write(Stream);
	OnWrite(Stream);
	}

//	3DS model ------------------------------------------------------------------

void CObj3DModel3DS::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::GridID), GetPropertyAnimator(Obj3DProp::GridID), Result);
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::ResourcePath), GetPropertyAnimator(Obj3DProp::ResourcePath), Result);
	}

const CString& CObj3DModel3DS::OnGetObjType () const { return TYPE_MODEL_3DS; }
CString CObj3DModel3DS::OnGetPropertyString (Obj3DProp iProp) const
	{
	switch (iProp)
		{
		case Obj3DProp::GridID: return m_sGridID;
		case Obj3DProp::ResourcePath: return (m_sResourcePath.IsEmpty() ? m_sGridID : m_sResourcePath);
		default: return NULL_STR;
		}
	}

void CObj3DModel3DS::OnRead (IByteStream& Stream)
	{
	m_sGridID = CString::Deserialize(Stream);
	m_sResourcePath = CString::Deserialize(Stream);
	}

bool CObj3DModel3DS::OnSetPropertyString (Obj3DProp iProp, const CString& sValue)
	{
	switch (iProp)
		{
		case Obj3DProp::GridID: m_sGridID = sValue; return true;
		case Obj3DProp::ResourcePath: m_sResourcePath = sValue; return true;
		default: return false;
		}
	}

void CObj3DModel3DS::OnWrite (IByteStream& Stream) const
	{
	m_sGridID.Serialize(Stream);
	m_sResourcePath.Serialize(Stream);
	}

//	GLTF model -----------------------------------------------------------------

void CObj3DModelGLTF::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::GridID), GetPropertyAnimator(Obj3DProp::GridID), Result);
	}

const CString& CObj3DModelGLTF::OnGetObjType () const { return TYPE_MODEL_GLTF; }
CString CObj3DModelGLTF::OnGetPropertyString (Obj3DProp iProp) const
	{
	return (iProp == Obj3DProp::GridID ? m_sGridID : NULL_STR);
	}

void CObj3DModelGLTF::OnRead (IByteStream& Stream)
	{
	m_sGridID = CString::Deserialize(Stream);
	}

bool CObj3DModelGLTF::OnSetPropertyString (Obj3DProp iProp, const CString& sValue)
	{
	if (iProp != Obj3DProp::GridID)
		return false;

	m_sGridID = sValue;
	return true;
	}

void CObj3DModelGLTF::OnWrite (IByteStream& Stream) const
	{
	m_sGridID.Serialize(Stream);
	}

//	Cube -----------------------------------------------------------------------

void CObj3DCube::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Color), GetPropertyAnimator(Obj3DProp::Color), Result);
	}

const CString& CObj3DCube::OnGetObjType () const { return TYPE_CUBE; }
CLuminousColor CObj3DCube::OnGetPropertyColor (Obj3DProp iProp) const { return (iProp == Obj3DProp::Color && !m_dwMaterialID && !m_dwColorTextureID ? m_Color : CLuminousColor()); }
void CObj3DCube::OnRead (IByteStream& Stream) { m_Color.Read(Stream); }
bool CObj3DCube::OnSetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value) { if (iProp != Obj3DProp::Color || m_dwMaterialID) return false; m_dwColorTextureID = 0; m_Color = Value; return true; }
void CObj3DCube::OnWrite (IByteStream& Stream) const { m_Color.Write(Stream); }

//	Point light ----------------------------------------------------------------

void CObj3DPointLight::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Color), GetPropertyAnimator(Obj3DProp::Color), Result);
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Intensity), GetPropertyAnimator(Obj3DProp::Intensity), Result);
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Distance), GetPropertyAnimator(Obj3DProp::Distance), Result);
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Decay), GetPropertyAnimator(Obj3DProp::Decay), Result);
	}

const CString& CObj3DPointLight::OnGetObjType () const { return TYPE_POINT_LIGHT; }
CLuminousColor CObj3DPointLight::OnGetPropertyColor (Obj3DProp iProp) const { return (iProp == Obj3DProp::Color ? m_Color : CLuminousColor()); }
double CObj3DPointLight::OnGetPropertyScalar (Obj3DProp iProp) const
	{
	switch (iProp)
		{
		case Obj3DProp::Intensity: return m_rIntensity;
		case Obj3DProp::Distance: return m_rDistance;
		case Obj3DProp::Decay: return m_rDecay;
		default: return 0.0;
		}
	}
void CObj3DPointLight::OnRead (IByteStream& Stream) { m_Color.Read(Stream); m_rIntensity = Stream.ReadDouble(); m_rDistance = Stream.ReadDouble(); m_rDecay = Stream.ReadDouble(); }
bool CObj3DPointLight::OnSetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value) { if (iProp != Obj3DProp::Color) return false; m_Color = Value; return true; }
bool CObj3DPointLight::OnSetPropertyScalar (Obj3DProp iProp, double rValue)
	{
	if (rValue < 0.0) return false;
	switch (iProp)
		{
		case Obj3DProp::Intensity: m_rIntensity = rValue; return true;
		case Obj3DProp::Distance: m_rDistance = rValue; return true;
		case Obj3DProp::Decay: m_rDecay = rValue; return true;
		default: return false;
		}
	}
void CObj3DPointLight::OnWrite (IByteStream& Stream) const { m_Color.Write(Stream); Stream.Write(m_rIntensity); Stream.Write(m_rDistance); Stream.Write(m_rDecay); }

//	Perspective camera ---------------------------------------------------------

void CObj3DPerspectiveCamera::OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const
	{
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::FOV), GetPropertyAnimator(Obj3DProp::FOV), Result);
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Near), GetPropertyAnimator(Obj3DProp::Near), Result);
	AccumulatePropertyToRender(GetPropertyDesc(Obj3DProp::Far), GetPropertyAnimator(Obj3DProp::Far), Result);
	}

const CString& CObj3DPerspectiveCamera::OnGetObjType () const { return TYPE_PERSPECTIVE_CAMERA; }
double CObj3DPerspectiveCamera::OnGetPropertyScalar (Obj3DProp iProp) const
	{
	switch (iProp)
		{
		case Obj3DProp::FOV: return m_rFOV;
		case Obj3DProp::Near: return m_rNear;
		case Obj3DProp::Far: return m_rFar;
		default: return 0.0;
		}
	}
void CObj3DPerspectiveCamera::OnRead (IByteStream& Stream) { m_rFOV = Stream.ReadDouble(); m_rNear = Stream.ReadDouble(); m_rFar = Stream.ReadDouble(); }
bool CObj3DPerspectiveCamera::OnSetPropertyScalar (Obj3DProp iProp, double rValue)
	{
	switch (iProp)
		{
		case Obj3DProp::FOV: if (rValue <= 0.0 || rValue >= 180.0) return false; m_rFOV = rValue; return true;
		case Obj3DProp::Near: if (rValue <= 0.0 || rValue >= m_rFar) return false; m_rNear = rValue; return true;
		case Obj3DProp::Far: if (rValue <= m_rNear) return false; m_rFar = rValue; return true;
		default: return false;
		}
	}
void CObj3DPerspectiveCamera::OnWrite (IByteStream& Stream) const { Stream.Write(m_rFOV); Stream.Write(m_rNear); Stream.Write(m_rFar); }
