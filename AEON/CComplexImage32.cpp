//	CComplexImage32.cpp
//
//	CComplexImage32 class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_IMAGE32,				"image32")

IComplexDatum *CComplexImage32::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clone a copy

	{
	return new CComplexImage32(m_Image);
	}

const CString &CComplexImage32::GetTypename (void) const

//	GetTypename
//
//	Returns the typename of the object.

	{
	return TYPENAME_IMAGE32;
	}

size_t CComplexImage32::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return CalcMemorySize();
	}

bool CComplexImage32::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	switch (iFormat)
		{
		case CDatum::EFormat::JSON:
			{
			CBuffer Buffer;
			Buffer.SetLength(Stream.GetStreamLength());
			Stream.Read(Buffer.GetPointer(), Buffer.GetLength());

			return CPNG::Load(Buffer, m_Image);
			}

		default:
			{
			DWORD dwWidth;
			Stream.Read(&dwWidth, sizeof(DWORD));

			DWORD dwHeight;
			Stream.Read(&dwHeight, sizeof(DWORD));

			DWORD dwAlphaType;
			Stream.Read(&dwAlphaType, sizeof(DWORD));

			m_Image.Create(dwWidth, dwHeight, (CRGBA32Image::EAlphaTypes)dwAlphaType);
			Stream.Read(m_Image.GetPixelPos(0, 0), dwWidth * dwHeight * sizeof(DWORD));
			return true;
			}
		}
	}

void CComplexImage32::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	switch (iFormat)
		{
		//	Always serialize to a PNG.

		case CDatum::EFormat::JSON:
			CPNG::Save(m_Image, Stream);
			break;

		//	Default we save a binary format

		default:
			{
			DWORD dwSave = m_Image.GetWidth();
			Stream.Write(&dwSave, sizeof(DWORD));

			dwSave = m_Image.GetHeight();
			Stream.Write(&dwSave, sizeof(DWORD));

			dwSave = (DWORD)m_Image.GetAlphaType();
			Stream.Write(&dwSave, sizeof(DWORD));

			DWORD dwLength = m_Image.GetWidth() * m_Image.GetHeight() * sizeof(DWORD);
			Stream.Write((char *)m_Image.GetPixelPos(0, 0), dwLength);

			break;
			}
		}
	}
