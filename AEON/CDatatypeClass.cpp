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

bool CDatatypeClass::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream)
	{
	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	DWORD dwCount;
	Stream.Read(&dwCount, sizeof(DWORD));
	m_Members.GrowToFit((int)dwCount);

	for (int i = 0; i < (int)dwCount; i++)
		{
		CString sName = CString::Deserialize(Stream);

		auto *pEntry = m_Members.SetAt(sName);
		pEntry->sName = sName;

		DWORD dwLoad;
		Stream.Read(&dwLoad, sizeof(DWORD));
		pEntry->iType = (EMemberType)dwLoad;

		TUniquePtr<IDatatype> pType = IDatatype::Deserialize(iFormat, Stream);
		if (!pType)
			return false;

		pEntry->dType = CDatum(new CComplexDatatype(std::move(pType)));
		}

	return true;
	}

bool CDatatypeClass::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeClass &)Src;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_Members.GetCount() != Other.m_Members.GetCount())
		return false;

	for (int i = 0; i < m_Members.GetCount(); i++)
		{
		if (!strEquals(m_Members[i].sName, Other.m_Members[i].sName))
			return false;

		if (m_Members[i].iType != Other.m_Members[i].iType)
			return false;

		if ((const IDatatype &)m_Members[i].dType != (const IDatatype &)Other.m_Members[i].dType)
			return false;
		}

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

void CDatatypeClass::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	m_Implements.Serialize(iFormat, Stream);

	DWORD dwCount = m_Members.GetCount();
	Stream.Write(&dwCount, sizeof(DWORD));

	for (int i = 0; i < m_Members.GetCount(); i++)
		{
		m_Members[i].sName.Serialize(Stream);

		DWORD dwSave = (DWORD)m_Members[i].iType;
		Stream.Write(&dwSave, sizeof(DWORD));

		const IDatatype &Type = m_Members[i].dType;
		Type.Serialize(iFormat, Stream);
		}
	}
