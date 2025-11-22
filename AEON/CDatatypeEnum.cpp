//	CDatatypeEnum.cpp
//
//	CDatatypeEnum class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ORDINAL,						"ordinal");

DECLARE_CONST_STRING(ERR_DUPLICATE_MEMBER,				"Duplicate enum definition: %s.")

bool CDatatypeEnum::IsEqual (const CDatatypeEnum& Other) const

//	IsEqual
//
//	Returns TRUE if equal.

	{
	if (m_Entries.GetCount() != Other.m_Entries.GetCount())
		return false;

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		auto OtherEntry = Other.m_Entries[i];
		if (!strEquals(m_Entries[i].sID, OtherEntry.sID))
			return false;

		if (!strEquals(m_Entries[i].sLabel, OtherEntry.sLabel))
			return false;
		}

	return true;
	}

bool CDatatypeEnum::OnAddMember (const SMemberDesc& Desc, CString *retsError)

//	OnAddMember
//
//	Add a member.

	{
	if (Desc.iType != EMemberType::EnumValue)
		throw CException(errFail);

	int iIndex = m_Entries.GetCount();

	//	LATER: Load ordinal from dType (or something).

	int iOrdinal = iIndex;

	//	Make sure this name doesn't already exist

	bool bNew;
	m_EntriesByName.SetAt(strToLower(Desc.sID), iIndex, &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_DUPLICATE_MEMBER, Desc.sID);
		return false;
		}

	//	Add

	auto pEntry = m_Entries.Insert();

	pEntry->iOrdinal = iOrdinal;
	pEntry->sID = Desc.sID;
	pEntry->sLabel = Desc.sLabel;	//	May be blank

	return true;
	}

bool CDatatypeEnum::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)

//	OnDeserialize
//
//	Load from a serialization.

	{
	DWORD dwCount;
	Stream.Read(&dwCount, sizeof(DWORD));
	m_Entries.InsertEmpty(dwCount);

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		DWORD dwFlags;
		Stream.Read(&dwFlags, sizeof(DWORD));

		m_Entries[i].sID = CString::Deserialize(Stream);

		if (dwVersion >= 2)
			m_Entries[i].sLabel = CString::Deserialize(Stream);

		Stream.Read(&m_Entries[i].iOrdinal, sizeof(DWORD));

		m_EntriesByName.SetAt(strToLower(m_Entries[i].sID), i);
		}

	return true;
	}

bool CDatatypeEnum::OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized)
	{
	DWORD dwCount = Stream.ReadDWORD();
	m_Entries.InsertEmpty(dwCount);

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		DWORD dwFlags = Stream.ReadDWORD();

		m_Entries[i].sID = CString::Deserialize(Stream);

		if (dwVerson >= 2)
			m_Entries[i].sLabel = CString::Deserialize(Stream);

		m_Entries[i].iOrdinal = (int)Stream.ReadDWORD();

		m_EntriesByName.SetAt(strToLower(m_Entries[i].sID), i);
		}

	return true;
	}

bool CDatatypeEnum::OnEquals (const IDatatype &Src) const

//	OnEquals
//
//	Returns TRUE if we're equal to Src.

	{
	if (Src.GetClass() != ECategory::Enum)
		return false;

	return IsEqual((const CDatatypeEnum&)Src);
	}

int CDatatypeEnum::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Return the index of the member (or -1)

	{
	auto pIndex = m_EntriesByName.GetAt(strToLower(sName));
	if (!pIndex)
		return -1;

	return *pIndex;
	}

int CDatatypeEnum::OnFindMemberByOrdinal (int iOrdinal) const

//	OnFindMemberByOrdinal
//
//	Return the index of the member (or -1)

	{
	for (int i = 0; i < m_Entries.GetCount(); i++)
		if (m_Entries[i].iOrdinal == iOrdinal)
			return i;

	return -1;
	}

IDatatype::SMemberDesc CDatatypeEnum::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Returns member info.

	{
	if (iIndex < 0 || iIndex >= m_Entries.GetCount())
		throw CException(errFail);
	
	return SMemberDesc({ 
			EMemberType::EnumValue,
			m_Entries[iIndex].sID,
			CDatum(),
			m_Entries[iIndex].iOrdinal,
			GetLabel(m_Entries[iIndex])
		});
	}

IDatatype::EMemberType CDatatypeEnum::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	Returns the member type.

	{
	auto pEntry = m_EntriesByName.GetAt(strToLower(sName));
	if (!pEntry)
		return EMemberType::None;

	int iIndex = *pEntry;

	if (retdType)
		*retdType = CDatum();

	if (retiOrdinal)
		*retiOrdinal = m_Entries[iIndex].iOrdinal;

	return EMemberType::EnumValue;
	}

bool CDatatypeEnum::OnIsEnum (const TArray<IDatatype::SMemberDesc>& Values) const

//	OnIsEnum
//
//	Returns TRUE if we match the given enum.

	{
	if (m_Entries.GetCount() != Values.GetCount())
		return false;

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		auto& Entry = m_Entries[i];
		auto& Value = Values[i];

		if (!strEquals(Entry.sID, Value.sID))
			return false;

		if (!strEquals(Entry.sLabel, Value.sLabel))
			return false;
		}

	return true;
	}

CDatum CDatatypeEnum::OnIteratorGetKey (CDatum dThisType, CDatum dIterator) const 
	{
	int iIndex = (int)dIterator;
	if (iIndex < 0 || iIndex >= m_Entries.GetCount())
		return CDatum();

	return m_Entries[iIndex].iOrdinal;
	}

CDatum CDatatypeEnum::OnIteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dThisType, CDatum dIterator) const
	{
	int iIndex = (int)dIterator;
	if (iIndex < 0 || iIndex >= m_Entries.GetCount())
		return CDatum();

	return CDatum::CreateEnum(m_Entries[iIndex].iOrdinal, dThisType);
	}

void CDatatypeEnum::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(m_Entries.GetCount());
	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		DWORD dwFlags = 0;
		Stream.Write(dwFlags);

		m_Entries[i].sID.Serialize(Stream);
		m_Entries[i].sLabel.Serialize(Stream);

		Stream.Write(m_Entries[i].iOrdinal);
		}
	}
