//	CDatatypeList.cpp
//
//	CDatatypeList class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CDatatypeList::CDatatypeList (const std::initializer_list<CDatum> &List)

//	CGLTypeList constructor

	{
	m_Types.GrowToFit((int)List.size());

	for (auto entry : List)
		{
		if (entry.GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);

		m_Types.Insert(entry);
		}
	}

bool CDatatypeList::IsA (const IDatatype &Type) const

//	IsA
//
//	Returns TRUE if any of the types in our list is-a Type.

	{
	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		const IDatatype &Src = m_Types[i];
		if (&Src == &Type || Src.IsA(Type))
			return true;
		}

	return false;
	}

void CDatatypeList::Mark ()

//	Mark
//
//	Marks data in use

	{
	for (int i = 0; i < m_Types.GetCount(); i++)
		m_Types[i].Mark();
	}
