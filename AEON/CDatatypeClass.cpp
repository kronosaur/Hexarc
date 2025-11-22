//	CDatatypeClass.cpp
//
//	CDatatypeClass class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_DUPLICATE_MEMBER,				"Duplicate member definition: %s.")

CDatatypeClass::CDatatypeClass (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_Implements(Create.Implements)

//	CDatatype constructor

	{
	m_Members.GrowToFit((int)Create.Members.size());

	for (auto& entry : Create.Members)
		{
		if (!AddMember(entry))
			throw CException(errFail);
		}
	}

bool CDatatypeClass::OnAddImplementation (CDatum dType)

//	OnAddImplementation
//
//	Adds an implementation.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	m_Implements.AddType(dType);
	return true;
	}

bool CDatatypeClass::OnAddMember (const SMemberDesc& Desc, CString *retsError)

//	AddMember
//
//	Adds a member.

	{
	if (Desc.dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	bool bNew;
	auto pEntry = m_Members.SetAt(strToLower(Desc.sID), &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_DUPLICATE_MEMBER, Desc.sID);
		return false;
		}

	pEntry->iType = Desc.iType;
	pEntry->dType = Desc.dType;
	pEntry->sID = Desc.sID;
	pEntry->sLabel = Desc.sLabel;
	if (pEntry->sLabel.IsEmpty())
		pEntry->sLabel = Desc.sID;
	pEntry->dwFlags = Desc.dwFlags;

	return true;
	}

bool CDatatypeClass::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	DWORD dwCount;
	Stream.Read(&dwCount, sizeof(DWORD));
	m_Members.GrowToFit((int)dwCount);

	for (int i = 0; i < (int)dwCount; i++)
		{
		CString sName = CString::Deserialize(Stream);

		auto *pEntry = m_Members.SetAt(strToLower(sName));
		pEntry->sID = sName;

		if (dwVersion >= 3)
			pEntry->dwFlags = Stream.ReadDWORD();

		if (dwVersion >= 2)
			pEntry->sLabel = CString::Deserialize(Stream);
		else
			pEntry->sLabel = sName;

		DWORD dwLoad;
		Stream.Read(&dwLoad, sizeof(DWORD));
		pEntry->iType = (EMemberType)dwLoad;

		if (!CComplexDatatype::CreateFromStream(Stream, pEntry->dType))
			return false;
		}

	return true;
	}

bool CDatatypeClass::OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized)
	{
	if (!CDatatypeList::DeserializeAEON(Stream, Serialized, m_Implements))
		return false;

	DWORD dwCount = Stream.ReadDWORD();
	m_Members.GrowToFit((int)dwCount);

	for (int i = 0; i < (int)dwCount; i++)
		{
		CString sName = CString::Deserialize(Stream);
		
		auto *pEntry = m_Members.SetAt(strToLower(sName));
		pEntry->sID = sName;
		
		if (dwVersion >= 3)
			pEntry->dwFlags = Stream.ReadDWORD();

		if (dwVersion >= 2)
			pEntry->sLabel = CString::Deserialize(Stream);
		else
			pEntry->sLabel = sName;
		
		DWORD dwLoad = Stream.ReadDWORD();
		pEntry->iType = (EMemberType)dwLoad;
		
		pEntry->dType = CDatum::DeserializeAEON(Stream, Serialized);
		}

	return true;
	}

bool CDatatypeClass::OnEquals (const IDatatype &Src) const
	{
	CRecursionSmartLock Lock(m_rs);
	if (Lock.InRecursion())
		return true;

	auto &Other = (const CDatatypeClass &)Src;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_Members.GetCount() != Other.m_Members.GetCount())
		return false;

	for (int i = 0; i < m_Members.GetCount(); i++)
		{
		if (!strEqualsNoCase(m_Members[i].sID, Other.m_Members[i].sID))
			return false;

		if (m_Members[i].iType != Other.m_Members[i].iType)
			return false;

		if ((const IDatatype &)m_Members[i].dType != (const IDatatype &)Other.m_Members[i].dType)
			return false;

		//	NOTE: We don't compare labels because they are not part of the
		//	identity of the class.
		//
		//	NOTE: We don't care about flags
		}

	return true;
	}

int CDatatypeClass::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Returns the member index (or -1).

	{
	int iPos;
	if (!m_Members.FindPos(strToLower(sName), &iPos))
		return -1;

	return iPos;
	}

IDatatype::SMemberDesc CDatatypeClass::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Returns the given member.

	{
	if (iIndex >= 0 && iIndex < m_Members.GetCount())
		return {
			m_Members[iIndex].iType,
			m_Members[iIndex].sID,
			m_Members[iIndex].dType,
			0,
			m_Members[iIndex].sLabel,
			EDisplay::Default,
			NULL_STR,
			m_Members[iIndex].dwFlags
			};
	else
		throw CException(errFail);
	}

IDatatype::EMemberType CDatatypeClass::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	HasMember
//
//	Looks up a member and returns its type.

	{
	auto pEntry = m_Members.GetAt(strToLower(sName));
	if (!pEntry)
		return EMemberType::None;

	if (retdType)
		*retdType = pEntry->dType;

	if (retiOrdinal)
		*retiOrdinal = -1;

	return pEntry->iType;
	}

bool CDatatypeClass::OnIsSupersetOf (const IDatatype& Type) const

//	OnIsSupersetOf
//
//	Returns TRUE if we are a superset of the given type.

	{
	if (IsA(Type))
		return true;

	if (Type.GetClass() != GetClass())
		return false;

	for (int i = 0; i < Type.GetMemberCount(); i++)
		{
		auto Desc = Type.GetMember(i);
		int iMember = FindMember(Desc.sID);
		if (iMember == -1)
			return false;

		if (m_Members[iMember].iType != Desc.iType)
			return false;

		if (!((const IDatatype&)m_Members[iMember].dType).IsSupersetOf(Desc.dType))
			return false;
		}

	return true;
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

void CDatatypeClass::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_Implements.SerializeAEON(Stream, Serialized);

	Stream.Write(m_Members.GetCount());
	for (int i = 0; i < m_Members.GetCount(); i++)
		{
		m_Members[i].sID.Serialize(Stream);
		Stream.Write(m_Members[i].dwFlags);
		m_Members[i].sLabel.Serialize(Stream);

		Stream.Write((DWORD)m_Members[i].iType);

		m_Members[i].dType.SerializeAEON(Stream, Serialized);
		}
	}

void CDatatypeClass::OnSetMemberType (const CString& sName, CDatum dType, DWORD dwFlags)
	{
	auto *pEntry = m_Members.GetAt(strToLower(sName));
	if (!pEntry)
		throw CException(errFail);

	pEntry->dType = dType;
	pEntry->dwFlags |= dwFlags;
	}
