//	CHexeGlobalEnvironment.cpp
//
//	CHexeGlobalEnvironment class
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_HEXE_GLOBAL_ENVIRONMENT,	"hexeGlobalEnvironment")
const CString &CHexeGlobalEnvironment::StaticGetTypename (void) { return TYPENAME_HEXE_GLOBAL_ENVIRONMENT; }

void CHexeGlobalEnvironment::OnMarked (void)

//	OnMarked
//
//	Mark

	{
	int i;

	for (i = 0; i < m_Env.GetCount(); i++)
		m_Env[i].Mark();
	}

void CHexeGlobalEnvironment::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize all elements for obects that are serialized as structures.

	{
	//	NOTE: We don't bother serializing anything, since we can't fully
	//	serialize primitive functions, etc.
	}
