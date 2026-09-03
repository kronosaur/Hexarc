//	ILuminousTexture3D.cpp
//
//	ILuminousTexture3D Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_IMAGE, "image");

TUniquePtr<ILuminousTexture3D> ILuminousTexture3D::CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream)
	{
	DWORD dwImpl = Stream.ReadDWORD();
	DWORD dwID = Stream.ReadDWORD();

	TUniquePtr<ILuminousTexture3D> pTexture;
	switch (dwImpl)
		{
		case IMPL_IMAGE:
			pTexture.Set(new CImageTexture3D(Scene, dwID));
			break;

		default:
			throw CException(errFail);
		}

	pTexture->m_Seq = Stream.ReadDWORDLONG();
	pTexture->OnRead(Stream);
	return pTexture;
	}

void ILuminousTexture3D::Write (IByteStream& Stream) const
	{
	Stream.Write(GetImpl());
	Stream.Write(GetID());
	Stream.Write(m_Seq);
	OnWrite(Stream);
	}

const CString& CImageTexture3D::OnGetTextureType () const
	{
	return TYPE_IMAGE;
	}

void CImageTexture3D::OnRead (IByteStream& Stream)
	{
	m_sGridID = CString::Deserialize(Stream);
	m_sFormat = CString::Deserialize(Stream);
	m_bFlipY = (Stream.ReadDWORD() != 0);
	}

void CImageTexture3D::OnWrite (IByteStream& Stream) const
	{
	m_sGridID.Serialize(Stream);
	m_sFormat.Serialize(Stream);
	Stream.Write((DWORD)(m_bFlipY ? 1 : 0));
	}
