//	CDatatypeAEON.cpp
//
//	CDatatypeAEON class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatatypeAEON::CDatatypeAEON (CStringView sFullyQualifiedName, DWORD dwCoreID, const CDatatypeList& Implements, CStringView sDatumTypename, TArray<SMemberDesc>&& Members, bool bCanBeNull) : IDatatype(sFullyQualifiedName, dwCoreID),
				m_Implements(Implements),
				m_Members(std::move(Members)),
				m_sDatumTypename(sDatumTypename),
				m_bCanBeNull(bCanBeNull)

//	CDatatypeAEON constructor

	{
	//	Must always have a CoreID; otherwise how would we implement the
	//	custom datum type?

	ASSERT(dwCoreID != 0);

	//	If we have a factory, remember it.

	if (!CDatum::FindExternalType(sDatumTypename, &m_pFactory))
		m_pFactory = NULL;
	}

CDatum CDatatypeAEON::OnCreateAsType (CDatum dValue) const

//	OnCreateAsType
//
//	Creates a new value of the given type.

	{
	//	If no factory, try to find it. Sometimes we can't find it in the constructor
	//	LATER: This is only necessary because GridNameType is defined in AEON, but 
	//	the factory is not registered until CGridWhaleUtil initializes. We should
	//	probably move both to GridLang (because it is used by the Core Library).

	if (!m_pFactory)
		{
		CDatum::FindExternalType(m_sDatumTypename, const_cast<IComplexFactory**>(&m_pFactory));
		}

	//	If no factory or if the value is already this type, then we can just
	//	continue.

	if (!m_pFactory 
			|| strEquals(dValue.GetTypename(), m_sDatumTypename))
		return dValue;

	//	Ask the factory to create the value. If it fails, then just return the
	//	input value.

	CDatum dNewValue = m_pFactory->CreateAsType(dValue);
	if (dNewValue.IsIdenticalToNil())
		return dValue;

	return dNewValue;
	}

bool CDatatypeAEON::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeSimple &)Src;

	if (GetCoreType() != Other.GetCoreType())
		return false;

	return true;
	}

int CDatatypeAEON::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Returns the index of the given member (by ID) or returns -1.

	{
	for (int i = 0; i < m_Members.GetCount(); i++)
		{
		if (strEqualsNoCase(sName, m_Members[i].sID))
			{
			return i;
			}
		}

	return -1;
	}

IDatatype::SMemberDesc CDatatypeAEON::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Get the given member.

	{
	if (iIndex < 0 || iIndex >= GetMemberCount())
		throw CException(errFail);

	return m_Members[iIndex];
	}

IDatatype::EMemberType CDatatypeAEON::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	OnHasMember
//
//	Looks for the given member and returns the type.

	{
	int iIndex = FindMember(sName);
	if (iIndex == -1)
		return EMemberType::None;

	if (retdType)
		*retdType = m_Members[iIndex].dType;

	if (retiOrdinal)
		*retiOrdinal = m_Members[iIndex].iOrdinal;

	return m_Members[iIndex].iType;
	}

bool CDatatypeAEON::OnIsA (const IDatatype &Type) const

//	OnIsA
//
//	Returns TRUE if we are the given type or a subtype.
	
	{
	if (m_Implements.IsA(Type))
		return true;

	else
		return false;
	}

void CDatatypeAEON::OnMark ()
	{
	for (int i = 0; i < m_Members.GetCount(); i++)
		m_Members[i].dType.Mark();
	}
