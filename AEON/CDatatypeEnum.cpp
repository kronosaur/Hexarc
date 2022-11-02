//	CDatatypeEnum.cpp
//
//	CDatatypeEnum class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_KEY_COLUMN,					"keyColumn");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ORDINAL,						"ordinal");

DECLARE_CONST_STRING(ERR_DUPLICATE_MEMBER,				"Duplicate enum definition: %s.")

bool CDatatypeEnum::OnAddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError)

//	OnAddMember
//
//	Add a member.

	{
	if (iType != EMemberType::EnumValue)
		throw CException(errFail);

	int iIndex = m_Entries.GetCount();

	//	LATER: Load ordinal from dType (or something).

	int iOrdinal = iIndex;

	//	Make sure this name doesn't already exist

	bool bNew;
	m_EntriesByName.SetAt(strToLower(sName), iIndex, &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_DUPLICATE_MEMBER, sName);
		return false;
		}

	//	Add

	auto pEntry = m_Entries.Insert();

	pEntry->iOrdinal = iOrdinal;
	pEntry->sName = sName;

	return true;
	}

bool CDatatypeEnum::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream)

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

		m_Entries[i].sName = CString::Deserialize(Stream);
		Stream.Read(&m_Entries[i].iOrdinal, sizeof(DWORD));

		m_EntriesByName.SetAt(strToLower(m_Entries[i].sName), i);
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

	if (m_Entries.GetCount() != Src.GetMemberCount())
		return false;

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		auto Other = Src.GetMember(i);
		if (!strEquals(m_Entries[i].sName, Other.sName))
			return false;

		if (m_Entries[i].iOrdinal != Other.iOrdinal)
			return false;
		}

	return true;
	}

int CDatatypeEnum::OnFindMember (const CString &sName) const

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
			m_Entries[iIndex].sName,
			CDatum(),
			m_Entries[iIndex].iOrdinal
		});
	}

CDatum CDatatypeEnum::OnGetMembersAsTable () const

//	OnGetMembersAsTable
//
//	Returns a table describing all the members of the schema. The resulting 
//	table has the following fields:
//
//	name (string): The name of the field/column.
//	datatype (datatype): The datatype of the field.
//	label (string): The human-readable name.
//	description (string): A description.

	{
	CDatum dSchema = CAEONTypeSystem::GetCoreType(IDatatype::MEMBER_TABLE);
	CDatum dResult = CDatum::CreateTable(dSchema);
	dResult.GrowToFit(m_Entries.GetCount());

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		CDatum dRow(CDatum::typeStruct);
		dRow.SetElement(FIELD_NAME, m_Entries[i].sName);
		dRow.SetElement(FIELD_DATATYPE, CDatum());
		dRow.SetElement(FIELD_LABEL, m_Entries[i].sName);
		dRow.SetElement(FIELD_DESCRIPTION, CDatum());
		dRow.SetElement(FIELD_KEY_COLUMN, CDatum());
		dRow.SetElement(FIELD_ORDINAL, m_Entries[i].iOrdinal);

		dResult.Append(dRow);
		}

	return dResult;
	}

IDatatype::EMemberType CDatatypeEnum::OnHasMember (const CString &sName, CDatum *retdType) const

//	Returns the member type.

	{
	auto pEntry = m_EntriesByName.GetAt(strToLower(sName));
	if (!pEntry)
		return EMemberType::None;

	int iIndex = *pEntry;

	if (retdType)
		*retdType = CDatum();

	return EMemberType::EnumValue;
	}

void CDatatypeEnum::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serializes to a stream.

	{
	DWORD dwCount = m_Entries.GetCount();
	Stream.Write(&dwCount, sizeof(DWORD));

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		DWORD dwFlags = 0;
		Stream.Write(&dwFlags, sizeof(DWORD));

		m_Entries[i].sName.Serialize(Stream);

		Stream.Write(&m_Entries[i].iOrdinal, sizeof(DWORD));
		}
	}
