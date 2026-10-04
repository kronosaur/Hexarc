//	CLuminousScene3D.cpp
//
//	CLuminousScene3D Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(MODE_DEFAULT, "default");
DECLARE_CONST_STRING(MODE_LOOP, "loop");
DECLARE_CONST_STRING(MODE_REALTIME, "realtime");
DECLARE_CONST_STRING(MODE_STREAM, "stream");

CLuminousScene3D::CLuminousScene3D (bool bDefaultObjects)
	{
	if (!bDefaultObjects) return;
	ILuminousObj3D& Camera = CreatePerspectiveCamera();
	Camera.SetPropertyVector(Obj3DProp::Pos, CVector3D(0.0, 0.0, 5.0));
	m_dwActiveCameraID = Camera.GetID();

	ILuminousObj3D& Light = CreatePointLight();
	Light.SetPropertyVector(Obj3DProp::Pos, CVector3D(5.0, 5.0, 5.0));
	}

const CString& CLuminousScene3D::AsID (EMode iMode)
	{
	switch (iMode)
		{
		case EMode::Default: return MODE_DEFAULT;
		case EMode::Loop: return MODE_LOOP;
		case EMode::Realtime: return MODE_REALTIME;
		case EMode::Stream: return MODE_STREAM;
		default: throw CException(errFail);
		}
	}

CLuminousScene3D::EMode CLuminousScene3D::AsMode (const CString& sValue)
	{
	if (sValue.IsEmpty() || strEqualsNoCase(sValue, MODE_DEFAULT)) return EMode::Default;
	else if (strEqualsNoCase(sValue, MODE_LOOP)) return EMode::Loop;
	else if (strEqualsNoCase(sValue, MODE_REALTIME)) return EMode::Realtime;
	else if (strEqualsNoCase(sValue, MODE_STREAM)) return EMode::Stream;
	else return EMode::Unknown;
	}

CLuminousScene3D CLuminousScene3D::CreateFromStream (IByteStream& Stream)
	{
	CLuminousScene3D Result;
	DWORD dwVersion = Stream.ReadDWORD();
	if (dwVersion < 1 || dwVersion > SERIALIZED_VERSION)
		throw CException(errFail);

	Result.m_Objs.DeleteAll();
	Result.m_iFPS = Stream.ReadInt();
	Result.m_iFrameCount = Stream.ReadInt();
	Result.m_iMode = AsMode(CString::Deserialize(Stream));
	if (Result.m_iMode == EMode::Unknown)
		throw CException(errFail);
	Result.m_Background.Read(Stream);
	if (dwVersion >= SERIALIZED_VERSION_ENVIRONMENT)
		Result.m_Environment.Read(Stream);
	Result.m_dwNextID = Stream.ReadDWORD();
	Result.m_dwActiveCameraID = Stream.ReadDWORD();
	Result.m_Seq = Stream.ReadDWORDLONG();
	Result.m_iStreamFrame = Stream.ReadInt();
	if (dwVersion >= SERIALIZED_VERSION_MATERIALS)
		{
		Result.m_dwNextMaterialID = Stream.ReadDWORD();
		int iMaterialCount = Stream.ReadInt();
		for (int i = 0; i < iMaterialCount; i++)
			{
			TUniquePtr<ILuminousMaterial3D> pMaterial = ILuminousMaterial3D::CreateFromStream(Result, Stream, dwVersion);
			Result.m_Materials.SetAt(pMaterial->GetID(), std::move(pMaterial));
			}
		}
	if (dwVersion >= SERIALIZED_VERSION_TEXTURES)
		{
		Result.m_dwNextTextureID = Stream.ReadDWORD();
		int iTextureCount = Stream.ReadInt();
		for (int i = 0; i < iTextureCount; i++)
			{
			TUniquePtr<ILuminousTexture3D> pTexture = ILuminousTexture3D::CreateFromStream(Result, Stream);
			Result.m_Textures.SetAt(pTexture->GetID(), std::move(pTexture));
			}
		}

	for (int i = 0; i < Result.m_Materials.GetCount(); i++)
		for (int j = 0; j < (int)Material3DMap::Count; j++)
			{
			DWORD dwTextureID = Result.m_Materials[i]->GetTextureID((Material3DMap)j);
			if (dwTextureID && !Result.FindTexture(dwTextureID))
				throw CException(errFail);
			}

	TSortMap<DWORD, DWORD> Parents;
	int iCount = Stream.ReadInt();
	for (int i = 0; i < iCount; i++)
		{
		TUniquePtr<ILuminousObj3D> pObj = ILuminousObj3D::CreateFromStream(Result, Stream, Parents);
		Result.m_Objs.SetAt(pObj->GetID(), std::move(pObj));
		}

	for (int i = 0; i < Parents.GetCount(); i++)
		{
		ILuminousObj3D* pObj = Result.FindObj(Parents.GetKey(i));
		ILuminousObj3D* pParent = Result.FindObj(Parents[i]);
		if (!pObj || !pParent)
			throw CException(errFail);
		pObj->SetParent(pParent);
		}

	if (Result.m_dwActiveCameraID && !Result.FindObj(Result.m_dwActiveCameraID))
		throw CException(errFail);

	if (dwVersion >= SERIALIZED_VERSION_MATERIALS)
		{
		int iAttachmentCount = Stream.ReadInt();
		for (int i = 0; i < iAttachmentCount; i++)
			{
			DWORD dwObjID = Stream.ReadDWORD();
			DWORD dwMaterialID = Stream.ReadDWORD();
			ILuminousObj3D* pObj = Result.FindObj(dwObjID);
			if (!pObj || !Result.FindMaterial(dwMaterialID) || !pObj->SetMaterialID(dwMaterialID))
				throw CException(errFail);
			}
		}

	if (dwVersion >= SERIALIZED_VERSION_TEXTURES)
		{
		int iAttachmentCount = Stream.ReadInt();
		for (int i = 0; i < iAttachmentCount; i++)
			{
			DWORD dwObjID = Stream.ReadDWORD();
			DWORD dwTextureID = Stream.ReadDWORD();
			ILuminousObj3D* pObj = Result.FindObj(dwObjID);
			if (!pObj || !Result.FindTexture(dwTextureID) || !pObj->SetColorTextureID(dwTextureID))
				throw CException(errFail);
			}
		}

	return Result;
	}

ILuminousObj3D& CLuminousScene3D::Create3DS (const CString& sGridID, DWORD dwParentID)
	{
	DWORD dwID = m_dwNextID++;
	ILuminousObj3D* pObj = new CObj3DModel3DS(*this, dwID, (dwParentID ? FindObj(dwParentID) : NULL), sGridID);
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj3D>(pObj));
	RecalcAnimation();
	return *pObj;
	}

ILuminousObj3D& CLuminousScene3D::CreateCube (DWORD dwParentID)
	{
	DWORD dwID = m_dwNextID++;
	ILuminousObj3D* pObj = new CObj3DCube(*this, dwID, (dwParentID ? FindObj(dwParentID) : NULL));
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj3D>(pObj));
	RecalcAnimation();
	return *pObj;
	}

ILuminousObj3D& CLuminousScene3D::CreatePlane (DWORD dwParentID)
	{
	DWORD dwID = m_dwNextID++;
	ILuminousObj3D* pObj = new CObj3DPlane(*this, dwID, (dwParentID ? FindObj(dwParentID) : NULL));
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj3D>(pObj));
	RecalcAnimation();
	return *pObj;
	}

ILuminousObj3D& CLuminousScene3D::CreateGLTF (const CString& sGridID, DWORD dwParentID)
	{
	DWORD dwID = m_dwNextID++;
	ILuminousObj3D* pObj = new CObj3DModelGLTF(*this, dwID, (dwParentID ? FindObj(dwParentID) : NULL), sGridID);
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj3D>(pObj));
	RecalcAnimation();
	return *pObj;
	}

ILuminousTexture3D& CLuminousScene3D::CreateImageTexture (const CString& sGridID, const CString& sFormat, bool bFlipY)
	{
	DWORD dwID = m_dwNextTextureID++;
	ILuminousTexture3D* pTexture = new CImageTexture3D(*this, dwID, sGridID, sFormat, bFlipY);
	pTexture->SetSeq(IncSeq());
	m_Textures.SetAt(dwID, TUniquePtr<ILuminousTexture3D>(pTexture));
	return *pTexture;
	}

ILuminousMaterial3D& CLuminousScene3D::CreatePhysicalMaterial ()
	{
	DWORD dwID = m_dwNextMaterialID++;
	ILuminousMaterial3D* pMaterial = new CPhysicalMaterial3D(*this, dwID);
	pMaterial->SetSeq(IncSeq());
	m_Materials.SetAt(dwID, TUniquePtr<ILuminousMaterial3D>(pMaterial));
	return *pMaterial;
	}

ILuminousObj3D& CLuminousScene3D::CreatePointLight (DWORD dwParentID)
	{
	DWORD dwID = m_dwNextID++;
	ILuminousObj3D* pObj = new CObj3DPointLight(*this, dwID, (dwParentID ? FindObj(dwParentID) : NULL));
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj3D>(pObj));
	RecalcAnimation();
	return *pObj;
	}

ILuminousObj3D& CLuminousScene3D::CreatePerspectiveCamera (DWORD dwParentID)
	{
	DWORD dwID = m_dwNextID++;
	ILuminousObj3D* pObj = new CObj3DPerspectiveCamera(*this, dwID, (dwParentID ? FindObj(dwParentID) : NULL));
	pObj->SetSeq(IncSeq());
	m_Objs.SetAt(dwID, TUniquePtr<ILuminousObj3D>(pObj));
	if (!m_dwActiveCameraID)
		m_dwActiveCameraID = dwID;
	RecalcAnimation();
	return *pObj;
	}

void CLuminousScene3D::OnObjModified (ILuminousObj3D& Obj)
	{
	Obj.SetSeq(IncSeq());
	RecalcAnimation();
	}

void CLuminousScene3D::OnMaterialModified (ILuminousMaterial3D& Material)
	{
	Material.SetSeq(IncSeq());
	}

bool CLuminousScene3D::RemoveMaterial (DWORD dwID)
	{
	if (!FindMaterial(dwID))
		return false;

	for (int i = 0; i < m_Objs.GetCount(); i++)
		if (m_Objs[i]->GetMaterialID() == dwID)
			return false;

	if (!m_Materials.DeleteAt(dwID))
		return false;

	IncSeq();
	return true;
	}

bool CLuminousScene3D::RemoveObj (DWORD dwID)
	{
	ILuminousObj3D* pRemoved = FindObj(dwID);
	if (!pRemoved)
		return false;

	//	Children must never retain a pointer to an object after it is removed.
	//	For this first slice we preserve them and reparent them to the scene root.

	for (int i = 0; i < m_Objs.GetCount(); i++)
		if (m_Objs[i]->GetParent() == pRemoved)
			m_Objs[i]->SetParent(NULL);

	if (!m_Objs.DeleteAt(dwID))
		return false;
	if (m_dwActiveCameraID == dwID)
		m_dwActiveCameraID = 0;
	RecalcAnimation();
	IncSeq();
	return true;
	}

void CLuminousScene3D::Move (CLuminousScene3D&& Src) noexcept
	{
	m_iFPS = Src.m_iFPS;
	m_iFrameCount = Src.m_iFrameCount;
	m_iMode = Src.m_iMode;
	m_Background = Src.m_Background;
	m_Environment = Src.m_Environment;
	m_Materials = std::move(Src.m_Materials);
	m_Objs = std::move(Src.m_Objs);
	m_Textures = std::move(Src.m_Textures);
	m_dwNextMaterialID = Src.m_dwNextMaterialID;
	m_dwNextID = Src.m_dwNextID;
	m_dwNextTextureID = Src.m_dwNextTextureID;
	m_dwActiveCameraID = Src.m_dwActiveCameraID;
	m_Seq = Src.m_Seq;
	m_iKeyframeFrame = Src.m_iKeyframeFrame;
	m_iKeyframeType = Src.m_iKeyframeType;
	m_iStreamFrame = Src.m_iStreamFrame;

	//	Objects carry a back-pointer used to notify their owning scene. Moving
	//	the object map preserves object addresses but requires rebinding owner.

	for (int i = 0; i < m_Materials.GetCount(); i++)
		m_Materials[i]->SetScene(*this);
	for (int i = 0; i < m_Objs.GetCount(); i++)
		m_Objs[i]->SetScene(*this);
	for (int i = 0; i < m_Textures.GetCount(); i++)
		m_Textures[i]->SetScene(*this);
	}

void CLuminousScene3D::AdvanceFrame (int iCount)
	{
	if (iCount < 1)
		return;

	for (int i = 0; i < m_Objs.GetCount(); i++)
		{
		ILuminousObj3D& Obj = *m_Objs[i];
		DWORD dwDirty = Obj.GetDirtyProps();
		for (int j = 1; j < (int)Obj3DProp::Count; j++)
			{
			if (!(dwDirty & (1 << j)))
				continue;

			Obj3DProp iProp = (Obj3DProp)j;
			switch (ILuminousObj3D::GetPropertyDesc(iProp).iType)
				{
				case Obj3DPropType::Bool: Obj.AnimateBoolConstant(iProp, m_iStreamFrame, Obj.GetPropertyBool(iProp)); break;
				case Obj3DPropType::Color: Obj.AnimateColorConstant(iProp, m_iStreamFrame, Obj.GetPropertyColor(iProp)); break;
				case Obj3DPropType::Scalar: Obj.AnimateScalarConstant(iProp, m_iStreamFrame, Obj.GetPropertyScalar(iProp)); break;
				case Obj3DPropType::String: Obj.AnimateStringConstant(iProp, m_iStreamFrame, Obj.GetPropertyString(iProp)); break;
				case Obj3DPropType::Vector: Obj.AnimateVectorConstant(iProp, m_iStreamFrame, Obj.GetPropertyVector(iProp)); break;
				}
			}
		Obj.ClearDirtyProps();
		}

	m_iStreamFrame += iCount;
	RecalcAnimation();
	IncSeq();
	}

void CLuminousScene3D::TrimKeyframes (int iFrame)
	{
	for (int i = 0; i < m_Objs.GetCount(); i++)
		m_Objs[i]->TrimKeyframesBefore(iFrame);
	RecalcAnimation();
	IncSeq();
	}

void CLuminousScene3D::RecalcAnimation ()
	{
	m_iFrameCount = 0;
	for (int i = 0; i < m_Objs.GetCount(); i++)
		m_iFrameCount = Max(m_iFrameCount, m_Objs[i]->GetFrameCount());
	}

void CLuminousScene3D::SetActiveCameraID (DWORD dwID)
	{
	if (dwID)
		{
		const ILuminousObj3D* pObj = FindObj(dwID);
		if (!pObj || !strEquals(pObj->GetObjType(), CString("perspectiveCamera")))
			return;
		}

	if (m_dwActiveCameraID != dwID)
		{
		m_dwActiveCameraID = dwID;
		IncSeq();
		}
	}

void CLuminousScene3D::SetFPS (int iFPS)
	{
	if (iFPS > 0 && m_iFPS != iFPS)
		{
		m_iFPS = iFPS;
		IncSeq();
		}
	}

void CLuminousScene3D::SetMode (EMode iMode)
	{
	if (iMode == EMode::Unknown)
		throw CException(errFail);
	if (m_iMode == iMode)
		return;

	if (iMode == EMode::Stream || m_iMode == EMode::Stream)
		{
		m_iStreamFrame = 0;
		for (int i = 0; i < m_Objs.GetCount(); i++)
			m_Objs[i]->ClearDirtyProps();
		}
	m_iMode = iMode;
	IncSeq();
	}

bool CLuminousScene3D::SetObjMaterial (DWORD dwObjID, DWORD dwMaterialID)
	{
	ILuminousObj3D* pObj = FindObj(dwObjID);
	if (!pObj || (dwMaterialID && !FindMaterial(dwMaterialID)))
		return false;

	DWORD dwOldMaterialID = pObj->GetMaterialID();
	if (!pObj->SetMaterialID(dwMaterialID))
		return false;

	if (dwOldMaterialID != dwMaterialID)
		OnObjModified(*pObj);
	return true;
	}

bool CLuminousScene3D::SetObjColorTexture (DWORD dwObjID, DWORD dwTextureID)
	{
	ILuminousObj3D* pObj = FindObj(dwObjID);
	if (!pObj || pObj->GetMaterialID() || (dwTextureID && !FindTexture(dwTextureID)))
		return false;

	DWORD dwOldTextureID = pObj->GetColorTextureID();
	if (!pObj->SetColorTextureID(dwTextureID))
		return false;
	bool bRemovedAnimation = pObj->RemoveAnimation(Obj3DProp::Color);

	if (dwOldTextureID != dwTextureID || bRemovedAnimation)
		OnObjModified(*pObj);
	return true;
	}

void CLuminousScene3D::Write (IByteStream& Stream) const
	{
	Stream.Write(SERIALIZED_VERSION);
	Stream.Write(m_iFPS);
	Stream.Write(m_iFrameCount);
	AsID(m_iMode).Serialize(Stream);
	m_Background.Write(Stream);
	m_Environment.Write(Stream);
	Stream.Write(m_dwNextID);
	Stream.Write(m_dwActiveCameraID);
	Stream.Write(m_Seq);
	Stream.Write(m_iStreamFrame);
	Stream.Write(m_dwNextMaterialID);
	Stream.Write(m_Materials.GetCount());
	for (int i = 0; i < m_Materials.GetCount(); i++)
		m_Materials[i]->Write(Stream);
	Stream.Write(m_dwNextTextureID);
	Stream.Write(m_Textures.GetCount());
	for (int i = 0; i < m_Textures.GetCount(); i++)
		m_Textures[i]->Write(Stream);
	Stream.Write(m_Objs.GetCount());
	for (int i = 0; i < m_Objs.GetCount(); i++)
		m_Objs[i]->Write(Stream);

	int iAttachmentCount = 0;
	for (int i = 0; i < m_Objs.GetCount(); i++)
		if (m_Objs[i]->GetMaterialID())
			iAttachmentCount++;

	Stream.Write(iAttachmentCount);
	for (int i = 0; i < m_Objs.GetCount(); i++)
		if (DWORD dwMaterialID = m_Objs[i]->GetMaterialID())
			{
			Stream.Write(m_Objs[i]->GetID());
			Stream.Write(dwMaterialID);
			}

	int iColorTextureAttachmentCount = 0;
	for (int i = 0; i < m_Objs.GetCount(); i++)
		if (m_Objs[i]->GetColorTextureID())
			iColorTextureAttachmentCount++;

	Stream.Write(iColorTextureAttachmentCount);
	for (int i = 0; i < m_Objs.GetCount(); i++)
		if (DWORD dwTextureID = m_Objs[i]->GetColorTextureID())
			{
			Stream.Write(m_Objs[i]->GetID());
			Stream.Write(dwTextureID);
			}
	}
