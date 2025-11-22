//	CDatatypeSchema.cpp
//
//	CDatatypeSchema class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ORDINAL,						"ordinal");

DECLARE_CONST_STRING(ERR_DUPLICATE_MEMBER,				"Duplicate member definition: %s.")
DECLARE_CONST_STRING(ERR_BLANK_MEMBER,					"Member name cannot be blank.")

bool CDatatypeSchema::OnAddImplementation (CDatum dType)

//	OnAddImplementation
//
//	Adds an implementation.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	m_Implements.AddType(dType);
	return true;
	}

bool CDatatypeSchema::OnAddMember (const SMemberDesc& Desc, CString *retsError)

//	AddMember
//
//	Adds a member.

	{
	if (Desc.dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	bool bKey = false;
	if (Desc.iType == EMemberType::InstanceVar)
		{ }
	else if (Desc.iType == EMemberType::InstanceKeyVar)
		bKey = true;
	else
		throw CException(errFail);

	if (Desc.sID.IsEmpty())
		{
		if (retsError) *retsError = ERR_BLANK_MEMBER;
		return false;
		}

	int iOrdinal = m_Columns.GetCount();

	//	Make sure this name doesn't already exist

	bool bNew;
	auto pIndex = m_ColumnsByName.SetAt(strToLower(Desc.sID), &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_DUPLICATE_MEMBER, Desc.sID);
		return false;
		}

	//	Add

	*pIndex = iOrdinal;

	auto pEntry = m_Columns.Insert();

	pEntry->iOrdinal = iOrdinal;
	pEntry->dType = Desc.dType;
	pEntry->sID = Desc.sID;
	pEntry->iDisplay = Desc.iDisplay;
	pEntry->sLabel = Desc.sLabel;
	if (pEntry->sLabel.IsEmpty())
		pEntry->sLabel = Desc.sID;
	pEntry->sFormat = Desc.sFormat;
	pEntry->bKey = bKey;

	return true;
	}

bool CDatatypeSchema::OnCanBeConstructedFrom (CDatum dType) const
	{
	const IDatatype& Type = dType;

	//	We always support construction from Any. This allows us to have type
	//	annotations be optional.

	if (Type.IsAny())
		return true;

	//	If the type is a schema, then we can construct from it.

	if (Type.IsA(*this))
		return true;

	//	We also allow constructing from a generic Struct.

	if (Type.IsA(IDatatype::STRUCT))
		return true;

	//	Otherwise, we cannot construct from this type.

	return false;
	}

bool CDatatypeSchema::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	SetCoreType(Stream.ReadDWORD());

	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	DWORD dwCount;
	Stream.Read(&dwCount, sizeof(DWORD));
	m_Columns.InsertEmpty(dwCount);

	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		DWORD dwFlags;
		Stream.Read(&dwFlags, sizeof(DWORD));
		m_Columns[i].bKey = ((dwFlags & 0x00000001) ? true : false);
		bool bHasFormat =	((dwFlags & 0x00000002) ? true : false);
		bool bHidden =		((dwFlags & 0x00000004) ? true : false);
		bool bReadOnly =	((dwFlags & 0x00000008) ? true : false);
		bool bEditable =	((dwFlags & 0x00000010) ? true : false);

		m_Columns[i].sID = CString::Deserialize(Stream);

		if (bEditable)
			m_Columns[i].iDisplay = EDisplay::Editable;
		else if (bReadOnly)
			m_Columns[i].iDisplay = EDisplay::ReadOnly;
		else if (bHidden)
			m_Columns[i].iDisplay = EDisplay::Hidden;
		else
			m_Columns[i].iDisplay = EDisplay::Default;

		if (dwVersion >= 2)
			m_Columns[i].sLabel = CString::Deserialize(Stream);
		else
			m_Columns[i].sLabel = m_Columns[i].sID;

		if (bHasFormat)
			m_Columns[i].sFormat = CString::Deserialize(Stream);

		Stream.Read(&m_Columns[i].iOrdinal, sizeof(DWORD));

		if (!CComplexDatatype::CreateFromStream(Stream, m_Columns[i].dType))
			return false;

		m_ColumnsByName.SetAt(strToLower(m_Columns[i].sID), i);
		}

	return true;
	}

bool CDatatypeSchema::OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized)
	{
	if (!CDatatypeList::DeserializeAEON(Stream, Serialized, m_Implements))
		return false;

	DWORD dwCount = Stream.ReadDWORD();
	m_Columns.InsertEmpty(dwCount);

	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		DWORD dwFlags = Stream.ReadDWORD();
		m_Columns[i].bKey = ((dwFlags & 0x00000001) ? true : false);
		bool bHasFormat =	((dwFlags & 0x00000002) ? true : false);
		bool bHidden =		((dwFlags & 0x00000004) ? true : false);
		bool bReadOnly =	((dwFlags & 0x00000008) ? true : false);
		bool bEditable =	((dwFlags & 0x00000010) ? true : false);

		m_Columns[i].sID = CString::Deserialize(Stream);
		m_Columns[i].sLabel = CString::Deserialize(Stream);

		if (bEditable)
			m_Columns[i].iDisplay = EDisplay::Editable;
		else if (bReadOnly)
			m_Columns[i].iDisplay = EDisplay::ReadOnly;
		else if (bHidden)
			m_Columns[i].iDisplay = EDisplay::Hidden;
		else
			m_Columns[i].iDisplay = EDisplay::Default;

		if (bHasFormat)
			m_Columns[i].sFormat = CString::Deserialize(Stream);

		m_Columns[i].iOrdinal = (int)Stream.ReadDWORD();

		m_Columns[i].dType = CDatum::DeserializeAEON(Stream, Serialized);

		m_ColumnsByName.SetAt(strToLower(m_Columns[i].sID), i);
		}

	return true;
	}

bool CDatatypeSchema::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeSchema &)Src;

	if (GetCoreType() != Other.GetCoreType())
		return false;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_Columns.GetCount() != Other.m_Columns.GetCount())
		return false;

	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		if (!strEquals(m_Columns[i].sID, Other.m_Columns[i].sID))
			return false;

		if (m_Columns[i].iDisplay != Other.m_Columns[i].iDisplay)
			return false;

		if (!strEquals(m_Columns[i].sFormat, Other.m_Columns[i].sFormat))
			return false;

		if (m_Columns[i].iOrdinal != Other.m_Columns[i].iOrdinal)
			return false;

		if ((const IDatatype &)m_Columns[i].dType != (const IDatatype &)Other.m_Columns[i].dType)
			return false;

		if (m_Columns[i].bKey != Other.m_Columns[i].bKey)
			return false;
		}

	return true;
	}

int CDatatypeSchema::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Returns the index of the given member (or -1).

	{
	auto pIndex = m_ColumnsByName.GetAt(strToLower(sName));
	if (!pIndex)
		return -1;

	return *pIndex;
	}

IDatatype::SMemberDesc CDatatypeSchema::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Returns the member by index.

	{
	if (iIndex < 0 || iIndex >= m_Columns.GetCount())
		throw CException(errFail);
	
	return SMemberDesc({ 
			(m_Columns[iIndex].bKey ? EMemberType::InstanceKeyVar : EMemberType::InstanceVar),
			m_Columns[iIndex].sID,
			m_Columns[iIndex].dType,
			iIndex,
			m_Columns[iIndex].sLabel,
			m_Columns[iIndex].iDisplay,
			m_Columns[iIndex].sFormat
			});
	}


IDatatype::EMemberType CDatatypeSchema::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	HasMember
//
//	Looks up a member and returns its type.

	{
	auto pEntry = m_ColumnsByName.GetAt(strToLower(sName));
	if (pEntry)
		{
		int iIndex = *pEntry;

		if (retdType)
			*retdType = m_Columns[iIndex].dType;

		if (retiOrdinal)
			*retiOrdinal = m_Columns[iIndex].iOrdinal;

		return (m_Columns[iIndex].bKey ? EMemberType::InstanceKeyVar : EMemberType::InstanceVar);
		}

	//	Otherwise, look for a property.

	int iPropIndex;
	if ((iPropIndex = CComplexStruct::FindPropertyByKey(sName)) != -1)
		{
		if (retdType)
			*retdType = CComplexStruct::GetPropertyType(iPropIndex);

		if (retiOrdinal)
			*retiOrdinal = -1;

		return EMemberType::InstanceProperty;
		}
	else
		return EMemberType::None;
	}

void CDatatypeSchema::OnMark ()

//	OnMark
//
//	Mark data in use

	{
	for (int i = 0; i < m_Columns.GetCount(); i++)
		m_Columns[i].dType.Mark();

	m_Implements.Mark();
	}

void CDatatypeSchema::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_Implements.SerializeAEON(Stream, Serialized);

	Stream.Write(m_Columns.GetCount());
	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		DWORD dwFlags = 0;
		dwFlags |= (m_Columns[i].bKey ?					0x00000001 : 0);
		dwFlags |= (!m_Columns[i].sFormat.IsEmpty() ?	0x00000002 : 0);

		switch (m_Columns[i].iDisplay)
			{
			case EDisplay::Default:
				break;

			case EDisplay::Editable:
				dwFlags |= 0x00000010;
				break;

			case EDisplay::ReadOnly:
				dwFlags |= 0x00000008;
				break;

			case EDisplay::Hidden:
				dwFlags |= 0x00000004;
				break;

			default:
				throw CException(errFail);
			}

		Stream.Write(dwFlags);

		m_Columns[i].sID.Serialize(Stream);
		m_Columns[i].sLabel.Serialize(Stream);

		if (!m_Columns[i].sFormat.IsEmpty())
			m_Columns[i].sFormat.Serialize(Stream);

		Stream.Write(m_Columns[i].iOrdinal);

		m_Columns[i].dType.SerializeAEON(Stream, Serialized);
		}
	}
