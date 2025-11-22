//	CAnimatorSet2D.cpp
//
//	CAnimatorSet2D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

void CAnimatorSet2D::Copy (const CAnimatorSet2D& Src)

//	Copy
//
//	Copy from source

	{
	for (int i = 0; i < Src.m_Animators.GetCount(); i++)
		m_Animators.SetAt(Src.m_Animators.GetKey(i), Src.m_Animators[i]->Clone());
	}

CAnimatorSet2D CAnimatorSet2D::CreateFromStream (IByteStream& Stream)

//	CreateFromStream
//
//	Read from stream.

	{
	CAnimatorSet2D Animators;

	int iCount = Stream.ReadInt();
	for (int i = 0; i < iCount; i++)
		{
		TUniquePtr<IAnimator2D> pAnimator = IAnimator2D::CreateFromStream(Stream);
		Animators.m_Animators.SetAt(pAnimator->GetProperty(), std::move(pAnimator));
		}

	return Animators;
	}

IAnimator2D& CAnimatorSet2D::GetAnimatorBool (Obj2DProp iProp, bool bInitialValue)
	{
	TUniquePtr<IAnimator2D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CBoolAnimator2D(iProp));

		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator2D::Type::Constant;

		pAnimator->AddKeyframeBool(Desc, bInitialValue);
		}

	return *pAnimator;
	}

IAnimator2D& CAnimatorSet2D::GetAnimatorColor (Obj2DProp iProp, const CLuminousColor& InitialValue)
	{
	TUniquePtr<IAnimator2D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CColorAnimator2D(iProp));
		
		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator2D::Type::Constant;
		
		pAnimator->AddKeyframeColor(Desc, InitialValue);
		}

	return *pAnimator;
	}

IAnimator2D& CAnimatorSet2D::GetAnimatorScalar (Obj2DProp iProp, double rInitialValue)
	{
	TUniquePtr<IAnimator2D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CScalarAnimator2D(iProp));
		
		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator2D::Type::Constant;
		
		pAnimator->AddKeyframeScalar(Desc, rInitialValue);
		}

	return *pAnimator;
	}

IAnimator2D& CAnimatorSet2D::GetAnimatorString (Obj2DProp iProp, const CString& sInitialValue)
	{
	TUniquePtr<IAnimator2D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CStringAnimator2D(iProp));
				
		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator2D::Type::Constant;
				
		pAnimator->AddKeyframeString(Desc, sInitialValue);
		}

	return *pAnimator;
	}

IAnimator2D& CAnimatorSet2D::GetAnimatorVector (Obj2DProp iProp, const CVector2D& InitialValue)
	{
	TUniquePtr<IAnimator2D>& pAnimator = *m_Animators.SetAt(iProp);

	//	If we don't have an animator for this property yet, we need to add one
	//	and start with the current value.

	if (!pAnimator)
		{
		pAnimator.Set(new CVectorAnimator2D(iProp));
						
		IAnimator2D::SKeyframeDesc Desc;
		Desc.iFrame = 0;
		Desc.iType = IAnimator2D::Type::Constant;
						
		pAnimator->AddKeyframeVector(Desc, InitialValue);
		}

	return *pAnimator;
	}

int CAnimatorSet2D::GetFrameCount () const

//	GetFrameCount
//
//	Returns the last keyframe.

	{
	int iMaxFrame = 0;

	for (int i = 0; i < m_Animators.GetCount(); i++)
		{
		const IAnimator2D& Animator = *m_Animators[i];
		iMaxFrame = Max(iMaxFrame, Animator.GetFrameCount());
		}

	return iMaxFrame;
	}

bool CAnimatorSet2D::RemoveAnimation (Obj2DProp iProp)

//	RemoveAnimation
//
//	Removes the animation from this property.

	{
	m_Animators.DeleteAt(iProp);
	return true;
	}

void CAnimatorSet2D::Write (IByteStream& Stream) const

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
