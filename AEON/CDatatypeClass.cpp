//	CDatatypeClass.cpp
//
//	CDatatypeClass class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_DUPLICATE_MEMBER,				"Duplicate member definition: %s.")

bool CDatatypeClass::OnAddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError)

//	AddMember
//
//	Adds a member.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	bool bNew;
	auto pEntry = m_Members.SetAt(sName, &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_DUPLICATE_MEMBER, sName);
		return false;
		}

	pEntry->iType = iType;
	pEntry->dType = dType;
	pEntry->sName = sName;

	return true;
	}

IDatatype::EMemberType CDatatypeClass::OnHasMember (const CString &sName, CDatum *retdType) const

//	HasMember
//
//	Looks up a member and returns its type.

	{
	auto pEntry = m_Members.GetAt(sName);
	if (!pEntry)
		return EMemberType::None;

	if (retdType)
		*retdType = pEntry->dType;

	return pEntry->iType;
	}

void CDatatypeClass::OnMark ()

//	OnMark
//
//	Mark data in use

	{
	for (int i = 0; i < m_Members.GetCount(); i++)
		m_Members[i].dType.Mark();

	m_Implements.Mark();
	}
