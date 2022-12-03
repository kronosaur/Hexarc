//	CAEONAnnotated.cpp
//
//	CAEONAnnotated class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

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
