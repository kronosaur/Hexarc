//	CHexeError.cpp
//
//	CHexeError class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_DEFAULT_ERROR_CODE,		"Error")
DECLARE_CONST_STRING(STR_DEFAULT_ERROR_DESC,		"An error has occurred.")

DECLARE_CONST_STRING(TYPENAME_HEXE_ERROR,			"hexeError")
const CString &CHexeError::StaticGetTypename (void) { return TYPENAME_HEXE_ERROR; }

void CHexeError::Create (const CString &sErrorCode, const CString &sErrorDesc, CDatum *retdDatum)

//	Create
//
//	Creates a new one

	{
	CHexeError *pError = new CHexeError;
	pError->m_sError = (sErrorCode.IsEmpty() ? STR_DEFAULT_ERROR_CODE : sErrorCode);
	pError->m_sDescription = (sErrorDesc.IsEmpty() ? STR_DEFAULT_ERROR_DESC : sErrorDesc);
	*retdDatum = CDatum(pError);
	}

bool CHexeError::OnDeserialize (CDatum::ESerializationFormats iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize from stream

	{
	m_sError = CString::Deserialize(Stream);
	m_sDescription = CString::Deserialize(Stream);
	return true;
	}

void CHexeError::OnSerialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize to stream

	{
	m_sError.Serialize(Stream);
	m_sDescription.Serialize(Stream);
	}

void CHexeError::Serialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const

//	SerializeJSON
//
//	Serialize to JSON

	{
	switch (iFormat)
		{
		case CDatum::formatJSON:
			{
			Stream.Write("[\"AEON2011:", 11);
			Stream.Write(GetTypename());
			Stream.Write(":v1\", \"", 7);

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
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}
