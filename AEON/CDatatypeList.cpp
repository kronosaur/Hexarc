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

bool CDatatypeList::operator == (const CDatatypeList &Src) const

//	operator ==

	{
	if (m_Types.GetCount() != Src.m_Types.GetCount())
		return false;

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if ((const IDatatype &)m_Types[i] != (const IDatatype &)Src.m_Types[i])
			return false;
		}

	return true;
	}

bool CDatatypeList::Deserialize (CDatum::EFormat iFormat, IByteStream &Stream, CDatatypeList &retList)

//	Deserialize
//
//	Deserialize the list.

	{
	CDatatypeList Result;

	DWORD dwCount;
	Stream.Read(&dwCount, sizeof(DWORD));
	Result.m_Types.InsertEmpty(dwCount);

	for (int i = 0; i < Result.m_Types.GetCount(); i++)
		{
		TUniquePtr<IDatatype> pType = IDatatype::Deserialize(iFormat, Stream);
		if (!pType)
			return false;

		Result.m_Types[i] = CDatum(new CComplexDatatype(std::move(pType)));
		}

	retList = std::move(Result);
	return true;
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

void CDatatypeList::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize the list.

	{
	//	Write the count

	DWORD dwCount = m_Types.GetCount();
	Stream.Write(&dwCount, sizeof(DWORD));

	//	Write each type.

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		const IDatatype &Type = m_Types[i];
		Type.Serialize(iFormat, Stream);
		}
	}
