//	CLuminousScene3D.cpp
//
//	CLuminousScene3D Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(MODE_DEFAULT,					"default");
DECLARE_CONST_STRING(MODE_LOOP,						"loop");
DECLARE_CONST_STRING(MODE_REALTIME,					"realtime");

const CString& CLuminousScene3D::AsID (EMode iMode)
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

CLuminousScene3D::EMode CLuminousScene3D::AsMode (const CString& sValue)
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

CLuminousScene3D CLuminousScene3D::CreateFromStream (IByteStream& Stream)

//	CreateFromStream
//
//	Read from stream.

	{
	CLuminousScene3D Result;

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
		TUniquePtr<ILuminousObj3D> pObj = ILuminousObj3D::CreateFromStream(Result, Stream, Parents);
		Result.m_Objs.SetAt(pObj->GetID(), std::move(pObj));
		}

	//	Fix up parents

	for (int i = 0; i < Parents.GetCount(); i++)
		{
		DWORD dwID = Parents.GetKey(i);
		DWORD dwParentID = Parents[i];

		ILuminousObj3D* pObj = Result.FindObj(dwID);
		if (pObj == NULL)
			throw CException(errFail);

		ILuminousObj3D* pParent = Result.FindObj(dwParentID);
		if (pParent == NULL)
			throw CException(errFail);

		pObj->SetParent(pParent);
		}

	return Result;
	}

void CLuminousScene3D::OnObjModified (ILuminousObj3D& Obj)

//	OnObjModified
//
//	The object has been modified.

	{
	Obj.SetSeq(IncSeq());
	RecalcAnimation();
	}

void CLuminousScene3D::Play (int iStartFrame)

//	Play
//
//	Set play mode.

	{
	m_dwStartTime = ::sysGetTickCount64();
	m_iStartFrame = iStartFrame;
	IncSeq();
	}

void CLuminousScene3D::RecalcAnimation ()

//	RecalcAnimation
//
//	If any object animations have been added, we recalculate them.

	{
	//	We need to compute (and cache) the latest keyframe.

	m_iFrameCount = 0;
	for (int i = 0; i < m_Objs.GetCount(); i++)
		m_iFrameCount = Max(m_iFrameCount, m_Objs[i]->GetFrameCount());
	}

void CLuminousScene3D::SetMode (EMode iMode)

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

void CLuminousScene3D::Stop ()

//	Stop
//
//	Stop playing.

	{
	m_dwStartTime = 0;
	m_iStartFrame = 0;
	IncSeq();
	}

void CLuminousScene3D::Write (IByteStream& Stream) const

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
