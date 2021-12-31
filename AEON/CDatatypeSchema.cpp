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
