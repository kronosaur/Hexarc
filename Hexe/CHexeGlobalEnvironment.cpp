//	CHexeGlobalEnvironment.cpp
//
//	CHexeGlobalEnvironment class
//	Copyright (c) 2012 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_HEXE_GLOBAL_ENVIRONMENT,	"hexeGlobalEnvironment")
const CString &CHexeGlobalEnvironment::StaticGetTypename (void) { return TYPENAME_HEXE_GLOBAL_ENVIRONMENT; }

bool CHexeGlobalEnvironment::AddSymbol (CStringView sIdentifier, CDatum dValue, DWORD* retdwID)

//	AddSymbol
//
//	Adds a symbol to the environment. Returns FALSE if the symbol already exists.

	{
	bool bNew;
	DWORD* pID = m_Index.SetAt(sIdentifier, &bNew);
	if (!bNew)
		{
		if (retdwID)
			*retdwID = *pID;
		return false;
		}

	m_Data.InsertEmpty();
	*pID = m_Data.GetCount() - 1;
	m_Data[*pID] = dValue;

	if (retdwID)
		*retdwID = *pID;

	return true;
	}

void CHexeGlobalEnvironment::SetAt (CStringView sSymbol, CDatum dValue)
	{
	bool bNew;
	DWORD* pID = m_Index.SetAt(sSymbol, &bNew);
	if (bNew)
		{
		m_Data.InsertEmpty();
		*pID = m_Data.GetCount() - 1;
		}

	m_Data[*pID] = dValue;
	}

void CHexeGlobalEnvironment::OnMarked (void)

//	OnMarked
//
//	Mark

	{
	int i;

	for (i = 0; i < m_Data.GetCount(); i++)
		m_Data[i].Mark();
	}

void CHexeGlobalEnvironment::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize all elements for obects that are serialized as structures.

	{
	//	NOTE: We don't bother serializing anything, since we can't fully
	//	serialize primitive functions, etc.
	}
