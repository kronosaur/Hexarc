//	CAEONError.cpp
//
//	CAEONError class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_ERROR,					"error");

const CString &CAEONError::GetTypename () const { return TYPENAME_ERROR; }

bool CAEONError::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize.

	{
	m_sError = CString::Deserialize(Stream);
	m_sDescription = CString::Deserialize(Stream);
	return true;
	}

CDatum CAEONError::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CAEONError *pValue = new CAEONError;
	CDatum dValue(pValue);

	pValue->m_sError = CString::Deserialize(Stream);
	pValue->m_sDescription = CString::Deserialize(Stream);

	return dValue;
	}

void CAEONError::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize.

	{
	m_sError.Serialize(Stream);
	m_sDescription.Serialize(Stream);
	}

void CAEONError::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize

	{
	switch (iFormat)
		{
		case CDatum::EFormat::GridLang:
			CDatum::WriteGridLangString(Stream, strPattern("ERROR: %s", m_sDescription));
			break;

		case CDatum::EFormat::JSON:
			{
			//	For backwards compatibility we write this as a hexeError.

			Stream.Write("[\"AEON2011:hexeError:v1\", \"", 27);

			CBase64Encoder Encoder(&Stream);
			OnSerialize(iFormat, Encoder);
			Encoder.Close();

			//	In addition to the encoded data above we serialize a human-readable
			//	version so that JSON clients don't have to decode too much.

			Stream.Write("\", \"", 4);
			m_sError.SerializeJSON(Stream);
			Stream.Write("\", \"", 4);
			m_sDescription.SerializeJSON(Stream);

			//	Done

			Stream.Write("\"]", 2);
			break;
			}

		default:
			//	Default serialization

			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONError::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_ERROR);

	m_sError.Serialize(Stream);
	m_sDescription.Serialize(Stream);
	}
