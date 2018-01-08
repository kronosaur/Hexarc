//	CHexePrimitive.cpp
//
//	CHexePrimitive class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_HEXE_PRIMITIVE,			"hexePrimitive")
const CString &CHexePrimitive::StaticGetTypename (void) { return TYPENAME_HEXE_PRIMITIVE; }

void CHexePrimitive::Create (CDatum::ECallTypes iPrimitive, CDatum *retdFunc)

//	Create
//
//	Creates the datum

	{
	CHexePrimitive *pFunc = new CHexePrimitive;
	pFunc->m_iPrimitive = iPrimitive;

	*retdFunc = CDatum(pFunc);
	}
