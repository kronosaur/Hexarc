//	CDatatypeSchema.cpp
//
//	CDatatypeSchema class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_NAME,						"name");

DECLARE_CONST_STRING(ERR_DUPLICATE_MEMBER,				"Duplicate member definition: %s.")

bool CDatatypeSchema::OnAddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError)

//	AddMember
//
//	Adds a member.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	if (iType != EMemberType::InstanceVar)
		throw CException(errFail);

	int iOrdinal = m_Columns.GetCount();

	//	Make sure this name doesn't already exist

	bool bNew;
	auto pIndex = m_ColumnsByName.SetAt(sName, &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_DUPLICATE_MEMBER, sName);
		return false;
		}

	//	Add

	*pIndex = iOrdinal;

	auto pEntry = m_Columns.Insert();

	pEntry->iOrdinal = iOrdinal;
	pEntry->dType = dType;
	pEntry->sName = sName;

	return true;
	}

bool CDatatypeSchema::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream)
	{
	Stream.Read(&m_dwCoreType, sizeof(DWORD));

	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	DWORD dwCount;
	Stream.Read(&dwCount, sizeof(DWORD));
	m_Columns.InsertEmpty(dwCount);

	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		m_Columns[i].sName = CString::Deserialize(Stream);
		Stream.Read(&m_Columns[i].iOrdinal, sizeof(DWORD));

		TUniquePtr<IDatatype> pType = IDatatype::Deserialize(iFormat, Stream);
		if (!pType)
			return false;

		m_Columns[i].dType = CDatum(new CComplexDatatype(std::move(pType)));

		m_ColumnsByName.SetAt(m_Columns[i].sName, i);
		}

	return true;
	}

bool CDatatypeSchema::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeSchema &)Src;

	if (m_dwCoreType != Other.m_dwCoreType)
		return false;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_Columns.GetCount() != Other.m_Columns.GetCount())
		return false;

	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		if (!strEquals(m_Columns[i].sName, Other.m_Columns[i].sName))
			return false;

		if (m_Columns[i].iOrdinal != Other.m_Columns[i].iOrdinal)
			return false;

		if ((const IDatatype &)m_Columns[i].dType != (const IDatatype &)Other.m_Columns[i].dType)
			return false;
		}

	return true;
	}

CDatum CDatatypeSchema::OnGetMembersAsTable () const

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
	CDatum dSchema = CAEONTypeSystem::GetCoreType(IDatatype::SCHEMA_TABLE);
	CDatum dResult = CDatum::CreateTable(dSchema);
	dResult.GrowToFit(m_Columns.GetCount());

	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		CDatum dRow(CDatum::typeStruct);
		dRow.SetElement(FIELD_NAME, m_Columns[i].sName);
		dRow.SetElement(FIELD_DATATYPE, m_Columns[i].dType);
		dRow.SetElement(FIELD_LABEL, m_Columns[i].sName);
		dRow.SetElement(FIELD_DESCRIPTION, CDatum());

		dResult.Append(dRow);
		}

	return dResult;
	}

IDatatype::EMemberType CDatatypeSchema::OnHasMember (const CString &sName, CDatum *retdType) const

//	HasMember
//
//	Looks up a member and returns its type.

	{
	auto pEntry = m_ColumnsByName.GetAt(sName);
	if (!pEntry)
		return EMemberType::None;

	int iIndex = *pEntry;

	if (retdType)
		*retdType = m_Columns[iIndex].dType;

	return EMemberType::InstanceVar;
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

void CDatatypeSchema::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	Stream.Write(&m_dwCoreType, sizeof(DWORD));

	m_Implements.Serialize(iFormat, Stream);

	DWORD dwCount = m_Columns.GetCount();
	Stream.Write(&dwCount, sizeof(DWORD));

	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		m_Columns[i].sName.Serialize(Stream);

		Stream.Write(&m_Columns[i].iOrdinal, sizeof(DWORD));

		const IDatatype &Type = m_Columns[i].dType;
		Type.Serialize(iFormat, Stream);
		}
	}
