//	CLuminousScene2D.cpp
//
//	CLuminousScene2D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(MODE_DEFAULT,					"default");
DECLARE_CONST_STRING(MODE_LOOP,						"loop");
DECLARE_CONST_STRING(MODE_REALTIME,					"realtime");

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
	else
		return EMode::Unknown;
	}

void CLuminousScene2D::Copy (const CLuminousScene2D& Src)
	{
	m_iFPS = Src.m_iFPS;
	m_iFrameCount = Src.m_iFrameCount;
	m_vExtent = Src.m_vExtent;
	m_vOrigin = Src.m_vOrigin;
	m_iMode = Src.m_iMode;
	m_Background = Src.m_Background;
	m_dwNextID = Src.m_dwNextID;
	m_Seq = Src.m_Seq;
	m_dwStartTime = Src.m_dwStartTime;
	m_iStartFrame = Src.m_iStartFrame;

	//	Copy objects

	for (int i = 0; i < Src.m_Objs.GetCount(); i++)
		{
		const TUniquePtr<ILuminousObj2D>& pSrcObj = Src.m_Objs[i];
		TUniquePtr<ILuminousObj2D> pNewObj = pSrcObj->Clone();
		m_Objs.SetAt(pNewObj->GetID(), std::move(pNewObj));
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
	Result.m_vExtent.Read(Stream);
	Result.m_vOrigin.Read(Stream);
	
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

void CLuminousScene2D::OnObjModified (ILuminousObj2D& Obj)

//	OnObjModified
//
//	The object has been modified.

	{
	Obj.SetSeq(IncSeq());
	RecalcAnimation();
	}

void CLuminousScene2D::Play (int iStartFrame)

//	Play
//
//	Set play mode.

	{
	m_dwStartTime = ::sysGetTickCount64();
	m_iStartFrame = iStartFrame;
	IncSeq();
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

void CLuminousScene2D::SetMode (EMode iMode)

//	SetMode
//
//	Sets the repeat mode.

	{
	if (iMode == EMode::Unknown)
		throw CException(errFail);

	if (m_iMode != iMode)
		{
		m_iMode = iMode;
		IncSeq();
		}
	}

void CLuminousScene2D::Stop ()

//	Stop
//
//	Stop playing.

	{
	m_dwStartTime = 0;
	m_iStartFrame = 0;
	IncSeq();
	}

void CLuminousScene2D::Write (IByteStream& Stream) const

//	Write
//
//	Write to stream.

	{
	Stream.Write(SERIALIZED_VERSION);

	Stream.Write(m_iFPS);
	Stream.Write(m_iFrameCount);
	m_vExtent.Write(Stream);
	m_vOrigin.Write(Stream);
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
