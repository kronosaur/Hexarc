//	CLuminousScene2D.cpp
//
//	CLuminousScene2D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(MODE_DEFAULT,					"default");
DECLARE_CONST_STRING(MODE_LOOP,						"loop");
DECLARE_CONST_STRING(MODE_REALTIME,					"realtime");
DECLARE_CONST_STRING(MODE_STREAM,					"stream");

DECLARE_CONST_STRING(ORIGIN_CENTER,					"center");
DECLARE_CONST_STRING(ORIGIN_UPPER_LEFT,				"upperLeft");
DECLARE_CONST_STRING(ORIGIN_LOWER_LEFT,				"lowerLeft");

const CString& CLuminousScene2D::AsID (EMode iMode)
	{
	switch (iMode)
		{
		case EMode::Default:
			return MODE_DEFAULT;

		case EMode::Loop:
			return MODE_LOOP;

		case EMode::Realtime:
			return MODE_REALTIME;

		case EMode::Stream:
			return MODE_STREAM;

		default:
			throw CException(errFail);
		}
	}

CLuminousScene2D::EMode CLuminousScene2D::AsMode (const CString& sValue)
	{
	if (sValue.IsEmpty() || strEqualsNoCase(sValue, MODE_DEFAULT))
		return EMode::Default;
	else if (strEqualsNoCase(sValue, MODE_LOOP))
		return EMode::Loop;
	else if (strEqualsNoCase(sValue, MODE_REALTIME))
		return EMode::Realtime;
	else if (strEqualsNoCase(sValue, MODE_STREAM))
		return EMode::Stream;
	else
		return EMode::Unknown;
	}

CLuminousScene2D::EOrigin CLuminousScene2D::AsOrigin (const CString& sValue)
	{
	if (sValue.IsEmpty() || strEqualsNoCase(sValue, ORIGIN_CENTER))
		return EOrigin::Center;
	else if (strEqualsNoCase(sValue, ORIGIN_UPPER_LEFT))
		return EOrigin::UpperLeft;
	else if (strEqualsNoCase(sValue, ORIGIN_LOWER_LEFT))
		return EOrigin::LowerLeft;
	else
		return EOrigin::Unknown;
	}

const CString& CLuminousScene2D::AsOriginID (EOrigin iOrigin)
	{
	switch (iOrigin)
		{
		case EOrigin::Center:
			return ORIGIN_CENTER;

		case EOrigin::UpperLeft:
			return ORIGIN_UPPER_LEFT;

		case EOrigin::LowerLeft:
			return ORIGIN_LOWER_LEFT;

		default:
			return ORIGIN_CENTER;
		}
	}

void CLuminousScene2D::Copy (const CLuminousScene2D& Src)
	{
	m_iFPS = Src.m_iFPS;
	m_iFrameCount = Src.m_iFrameCount;
	m_iOrigin = Src.m_iOrigin;
	m_vExtents = Src.m_vExtents;
	m_iMode = Src.m_iMode;
	m_Background = Src.m_Background;
	m_dwNextID = Src.m_dwNextID;
	m_Seq = Src.m_Seq;
	m_iKeyframeFrame = Src.m_iKeyframeFrame;
	m_iKeyframeType = Src.m_iKeyframeType;
	m_iStreamFrame = Src.m_iStreamFrame;

	//	Copy objects

	for (int i = 0; i < Src.m_Objs.GetCount(); i++)
		{
		const TUniquePtr<ILuminousObj2D>& pSrcObj = Src.m_Objs[i];
		TUniquePtr<ILuminousObj2D> pNewObj = pSrcObj->Clone();
		m_Objs.SetAt(pNewObj->GetID(), std::move(pNewObj));
		}
	RebindObjects();
	}

void CLuminousScene2D::Move (CLuminousScene2D&& Src)
	{
	m_iFPS = Src.m_iFPS;
	m_iFrameCount = Src.m_iFrameCount;
	m_iOrigin = Src.m_iOrigin;
	m_vExtents = Src.m_vExtents;
	m_iMode = Src.m_iMode;
	m_Background = Src.m_Background;
	m_dwNextID = Src.m_dwNextID;
	m_Seq = Src.m_Seq;
	m_iKeyframeFrame = Src.m_iKeyframeFrame;
	m_iKeyframeType = Src.m_iKeyframeType;
	m_iStreamFrame = Src.m_iStreamFrame;

	m_Objs = std::move(Src.m_Objs);
	RebindObjects();
	}

void CLuminousScene2D::RebindObjects ()
	{
	// Copies and deserialization moves must point at the receiving scene.
	for (int i = 0; i < m_Objs.GetCount(); i++)
		{
		auto& Obj = *m_Objs[i];
		Obj.RebindScene(*this);
		if (Obj.GetParent()) Obj.SetParent(FindObj(Obj.GetParent()->GetID()));
		}
	}

CLuminousScene2D CLuminousScene2D::CreateFromStream (IByteStream& Stream)

//	CreateFromStream
//
//	Read from stream.

	{
	CLuminousScene2D Result;

	DWORD dwVersion = Stream.ReadDWORD();

	Result.m_iFPS = Stream.ReadInt();
	Result.m_iFrameCount = Stream.ReadInt();

	if (dwVersion == 1)
		{
		//	Version 1: m_vExtent and m_vOrigin as CVector2D.
		//	Convert to new format: treat as center origin with extents.

		CVector2D vExtent;
		vExtent.Read(Stream);

		CVector2D vOrigin;
		vOrigin.Read(Stream);

		Result.m_iOrigin = EOrigin::Center;
		Result.m_vExtents = vExtent;
		}
	else if (dwVersion == 2)
		{
		//	Version 2: origin + scale + extents. Scale was moved to
		//	CUIScene2DCtrl in version 3, so we read and discard it.

		CString sOrigin = CString::Deserialize(Stream);
		Result.m_iOrigin = AsOrigin(sOrigin);
		if (Result.m_iOrigin == EOrigin::Unknown)
			Result.m_iOrigin = EOrigin::Center;

		CString sScale = CString::Deserialize(Stream);
		//	Discard scale ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â now lives on the control.

		Result.m_vExtents.Read(Stream);
		}
	else
		{
		//	Version 3+: origin + extents (no scale).

		CString sOrigin = CString::Deserialize(Stream);
		Result.m_iOrigin = AsOrigin(sOrigin);
		if (Result.m_iOrigin == EOrigin::Unknown)
			Result.m_iOrigin = EOrigin::Center;

		Result.m_vExtents.Read(Stream);
		}

	CString sMode = CString::Deserialize(Stream);
	Result.m_iMode = AsMode(sMode);
	if (Result.m_iMode == EMode::Unknown)
		throw CException(errFail);

	Result.m_Background.Read(Stream);
	Result.m_dwNextID = Stream.ReadDWORD();
	Result.m_Seq = Stream.ReadDWORDLONG();

	int iCount = Stream.ReadInt();
	Result.m_Objs.DeleteAll();
	TSortMap<DWORD, DWORD> Parents;
	for (int i = 0; i < iCount; i++)
		{
		TUniquePtr<ILuminousObj2D> pObj = ILuminousObj2D::CreateFromStream(Result, Stream, Parents);
		Result.m_Objs.SetAt(pObj->GetID(), std::move(pObj));
		}

	//	Fix up parents

	for (int i = 0; i < Parents.GetCount(); i++)
		{
		DWORD dwID = Parents.GetKey(i);
		DWORD dwParentID = Parents[i];

		ILuminousObj2D* pObj = Result.FindObj(dwID);
		if (pObj == NULL)
			throw CException(errFail);

		ILuminousObj2D* pParent = Result.FindObj(dwParentID);
		if (pParent == NULL)
			throw CException(errFail);

		pObj->SetParent(pParent);
		}

	return Result;
	}

ILuminousObj2D& CLuminousScene2D::CreateCircle (DWORD dwParentID)

//	CreateCircle
//
//	Adds a circle object.

	{
	DWORD dwID = m_dwNextID++;

	ILuminousObj2D* pParent = (dwParentID ? FindObj(dwParentID) : NULL);
	ILuminousObj2D *pObj = new CObj2DCircle(*this, dwID, pParent);
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj2D>(pObj));

	RecalcAnimation();

	return *pObj;
	}

ILuminousObj2D& CLuminousScene2D::CreateText (DWORD dwParentID)

//	CreateText
//
//	Adds a text object.

	{
	DWORD dwID = m_dwNextID++;

	ILuminousObj2D* pParent = (dwParentID ? FindObj(dwParentID) : NULL);
	ILuminousObj2D *pObj = new CObj2DText(*this, dwID, pParent);
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj2D>(pObj));

	RecalcAnimation();

	return *pObj;
	}

ILuminousObj2D& CLuminousScene2D::CreateRectangle (DWORD dwParentID)

//	CreateRectangle
//
//	Adds a rectangle object.

	{
	DWORD dwID = m_dwNextID++;

	ILuminousObj2D* pParent = (dwParentID ? FindObj(dwParentID) : NULL);
	ILuminousObj2D *pObj = new CObj2DRectangle(*this, dwID, pParent);
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj2D>(pObj));

	RecalcAnimation();

	return *pObj;
	}

ILuminousObj2D& CLuminousScene2D::CreateTrail (DWORD dwParentID)

//	CreateTrail
//
//	Adds a trail object.

	{
	DWORD dwID = m_dwNextID++;

	ILuminousObj2D* pParent = (dwParentID ? FindObj(dwParentID) : NULL);
	ILuminousObj2D *pObj = new CObj2DTrail(*this, dwID, pParent);
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj2D>(pObj));

	RecalcAnimation();

	return *pObj;
	}
ILuminousObj2D& CLuminousScene2D::CreateLine (DWORD dwParentID)

//	CreateLine
//
//	Adds a line object.

	{
	DWORD dwID = m_dwNextID++;

	ILuminousObj2D* pParent = (dwParentID ? FindObj(dwParentID) : NULL);
	ILuminousObj2D *pObj = new CObj2DLine(*this, dwID, pParent);
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj2D>(pObj));

	RecalcAnimation();

	return *pObj;
	}
void CLuminousScene2D::OnObjModified (ILuminousObj2D& Obj)

//	OnObjModified
//
//	The object has been modified.

	{
	Obj.SetSeq(IncSeq());
	RecalcAnimation();
	}

bool CLuminousScene2D::RemoveObj (DWORD dwID)

//	RemoveObj
//
//	Removes an object from the scene by ID. Returns true if successful.

	{
	ILuminousObj2D* pRemoved = FindObj(dwID);
	if (!pRemoved) return false;
	for (int i = 0; i < m_Objs.GetCount(); i++)
		if (m_Objs[i]->GetParent() == pRemoved) m_Objs[i]->SetParent(NULL);
	m_Objs.DeleteAt(dwID);

	RecalcAnimation();
	IncSeq();
	return true;
	}

void CLuminousScene2D::RecalcAnimation ()

//	RecalcAnimation
//
//	If any object animations have been added, we recalculate them.

	{
	//	We need to compute (and cache) the latest keyframe.

	m_iFrameCount = 0;
	for (int i = 0; i < m_Objs.GetCount(); i++)
		m_iFrameCount = Max(m_iFrameCount, m_Objs[i]->GetFrameCount());
	}


void CLuminousScene2D::AdvanceFrame (int iCount)

//	AdvanceFrame
//
//	For each object, emit constant keyframes for any properties that have
//	been modified since the last AdvanceFrame. Then advance the stream
//	frame counter by iCount.

	{
	for (int i = 0; i < m_Objs.GetCount(); i++)
		{
		ILuminousObj2D& Obj = *m_Objs[i];
		DWORDLONG dwDirty = Obj.GetDirtyProps();
		if (dwDirty == 0)
			continue;

		for (int j = 1; j < (int)Obj2DProp::Count; j++)
			{
			if (!(dwDirty & (DWORDLONG(1) << j)))
				continue;

			Obj2DProp iProp = (Obj2DProp)j;
			const auto& Desc = ILuminousObj2D::GetPropertyDesc(iProp);

			switch (Desc.iType)
				{
				case ObjPropType::Bool:
					Obj.AnimateBoolConstant(iProp, m_iStreamFrame, Obj.GetPropertyBool(iProp));
					break;

				case ObjPropType::Color:
					Obj.AnimateColorConstant(iProp, m_iStreamFrame, Obj.GetPropertyColor(iProp));
					break;

				case ObjPropType::String:
					Obj.AnimateStringConstant(iProp, m_iStreamFrame, Obj.GetPropertyString(iProp));
					break;

				case ObjPropType::Scalar:
					Obj.AnimateScalarConstant(iProp, m_iStreamFrame, Obj.GetPropertyScalar(iProp));
					break;

				case ObjPropType::Vector:
					Obj.AnimateVectorConstant(iProp, m_iStreamFrame, Obj.GetPropertyVector(iProp));
					break;

				case ObjPropType::VectorQueue:
				case ObjPropType::VectorList:
					Obj.AnimateVectorQueueConstant(iProp, m_iStreamFrame, Obj.GetPropertyVectorQueue(iProp));
					break;
				}
			}

		Obj.ClearDirtyProps();
		}

	m_iStreamFrame += iCount;
	RecalcAnimation();
	IncSeq();
	}

void CLuminousScene2D::TrimKeyframes (int iFrame)

//	TrimKeyframes
//
//	Remove keyframes before the given frame from all objects in the scene.
//	The last keyframe before iFrame is preserved as the new starting point.

	{
	for (int i = 0; i < m_Objs.GetCount(); i++)
		m_Objs[i]->TrimKeyframesBefore(iFrame);

	RecalcAnimation();
	IncSeq();
	}

void CLuminousScene2D::SetFPS (int iFPS)

//	SetFPS
//
//	Sets the frames per second.

	{
	if (iFPS > 0 && m_iFPS != iFPS)
		{
		m_iFPS = iFPS;
		IncSeq();
		}
	}

void CLuminousScene2D::SetMode (EMode iMode)

//	SetMode
//
//	Sets the animation mode.

	{
	if (iMode == EMode::Unknown)
		throw CException(errFail);

	if (m_iMode != iMode)
		{
		//	Reset stream state when entering or leaving stream mode.

		if (iMode == EMode::Stream || m_iMode == EMode::Stream)
			{
			m_iStreamFrame = 0;

			for (int i = 0; i < m_Objs.GetCount(); i++)
				m_Objs[i]->ClearDirtyProps();
			}

		m_iMode = iMode;
		IncSeq();
		}
	}

void CLuminousScene2D::Write (IByteStream& Stream) const

//	Write
//
//	Write to stream.

	{
	Stream.Write(SERIALIZED_VERSION);

	Stream.Write(m_iFPS);
	Stream.Write(m_iFrameCount);

	CString sOrigin = AsOriginID(m_iOrigin);
	sOrigin.Serialize(Stream);
	m_vExtents.Write(Stream);

	CString sMode = AsID(m_iMode);
	sMode.Serialize(Stream);
	m_Background.Write(Stream);
	Stream.Write(m_dwNextID);
	Stream.Write(m_Seq);

	Stream.Write(m_Objs.GetCount());
	for (int i = 0; i < m_Objs.GetCount(); i++)
		{
		m_Objs[i]->Write(Stream);
		}
	}
