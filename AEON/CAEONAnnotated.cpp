//	CAEONAnnotated.cpp
//
//	CAEONAnnotated class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_SPREAD,						"spread");

DECLARE_CONST_STRING(TYPENAME_ANNOTATED,				"annotated");

const CString &CAEONAnnotated::GetTypename () const { return TYPENAME_ANNOTATED; }

bool CAEONAnnotated::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Deserialize.

	{
	m_dValue = dStruct.GetElement(FIELD_DATA);
	m_Annotations.fSpread = !dStruct.GetElement(FIELD_SPREAD).IsNil();
	return true;
	}

void CAEONAnnotated::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize.

	{
	pStruct->SetElement(FIELD_DATA, m_dValue);
	if (m_Annotations.fSpread)
		pStruct->SetElement(FIELD_SPREAD, true);
	}

CDatum CAEONAnnotated::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new value and add it to the map.

	CAEONAnnotated *pValue = new CAEONAnnotated;
	CDatum dValue(pValue);
	Serialized.Add(dwID, dValue);

	//	Read the value

	DWORD dwFlags = Stream.ReadDWORD();
	pValue->m_Annotations.fSpread = ((dwFlags & 0x00000001) ? true : false);
	pValue->m_dValue = CDatum::DeserializeAEON(Stream, Serialized);

	return dValue;
	}

void CAEONAnnotated::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_ANNOTATED))
		return;

	//	Otherwise, write out the value

	DWORD dwFlags = 0;
	dwFlags |= (m_Annotations.fSpread ? 0x00000001 : 0);
	Stream.Write(dwFlags);

	//	Write out the value

	m_dValue.SerializeAEON(Stream, Serialized);
	}
