//	CAnimatorSet3D.cpp
//
//	CAnimatorSet3D Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

void CAnimatorSet3D::Copy (const CAnimatorSet3D& Src)

//	Copy
//
//	Copy from source

	{
	for (int i = 0; i < Src.m_Animators.GetCount(); i++)
		m_Animators.SetAt(Src.m_Animators.GetKey(i), Src.m_Animators[i]->Clone());
	}

CAnimatorSet3D CAnimatorSet3D::CreateFromStream (IByteStream& Stream)

//	CreateFromStream
//
//	Read from stream.

	{
	CAnimatorSet3D Animators;

	int iCount = Stream.ReadInt();
	for (int i = 0; i < iCount; i++)
		{
		TUniquePtr<IAnimator3D> pAnimator = IAnimator3D::CreateFromStream(Stream);
		Animators.m_Animators.SetAt(pAnimator->GetProperty(), std::move(pAnimator));
		}

	return Animators;
	}

IAnimator3D& CAnimatorSet3D::GetAnimatorBool (Obj3DProp iProp, bool bInitialValue)
	{
	TUniquePtr<IAnimator3D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CBoolAnimator3D(iProp));

		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator3D::Type::Constant;

		pAnimator->AddKeyframeBool(Desc, bInitialValue);
		}

	return *pAnimator;
	}

IAnimator3D& CAnimatorSet3D::GetAnimatorColor (Obj3DProp iProp, const CLuminousColor& InitialValue)
	{
	TUniquePtr<IAnimator3D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CColorAnimator3D(iProp));
		
		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator3D::Type::Constant;
		
		pAnimator->AddKeyframeColor(Desc, InitialValue);
		}

	return *pAnimator;
	}

IAnimator3D& CAnimatorSet3D::GetAnimatorScalar (Obj3DProp iProp, double rInitialValue)
	{
	TUniquePtr<IAnimator3D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CScalarAnimator3D(iProp));
		
		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator3D::Type::Constant;
		
		pAnimator->AddKeyframeScalar(Desc, rInitialValue);
		}

	return *pAnimator;
	}

IAnimator3D& CAnimatorSet3D::GetAnimatorString (Obj3DProp iProp, const CString& sInitialValue)
	{
	TUniquePtr<IAnimator3D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CStringAnimator3D(iProp));
				
		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator3D::Type::Constant;
				
		pAnimator->AddKeyframeString(Desc, sInitialValue);
		}

	return *pAnimator;
	}

IAnimator3D& CAnimatorSet3D::GetAnimatorVector (Obj3DProp iProp, const CVector3D& InitialValue)
	{
	TUniquePtr<IAnimator3D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CVectorAnimator3D(iProp));
						
		IAnimator3D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator3D::Type::Constant;
						
		pAnimator->AddKeyframeVector(Desc, InitialValue);
		}

	return *pAnimator;
	}

int CAnimatorSet3D::GetFrameCount () const

//	GetFrameCount
//
//	Returns the last keyframe.

	{
	int iMaxFrame = 0;

	for (int i = 0; i < m_Animators.GetCount(); i++)
		{
		const IAnimator3D& Animator = *m_Animators[i];
		iMaxFrame = Max(iMaxFrame, Animator.GetFrameCount());
		}

	return iMaxFrame;
	}

bool CAnimatorSet3D::RemoveAnimation (Obj3DProp iProp)

//	RemoveAnimation
//
//	Removes the animation from this property.

	{
	m_Animators.DeleteAt(iProp);
	return true;
	}

void CAnimatorSet3D::Write (IByteStream& Stream) const

//	Write
//
//	Write animators to stream.

	{
	Stream.Write(m_Animators.GetCount());
	for (int i = 0; i < m_Animators.GetCount(); i++)
		{
		m_Animators[i]->Write(Stream);
		}
	}
