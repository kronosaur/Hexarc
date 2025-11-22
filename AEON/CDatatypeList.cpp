//	CDatatypeList.cpp
//
//	CDatatypeList class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatatypeList::CDatatypeList (std::initializer_list<CDatum> List)

//	CDatatypeList constructor

	{
	m_Types.GrowToFit((int)List.size());

	for (auto entry : List)
		{
		if (entry.GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);

		m_Types.Insert(entry);
		}
	}

CDatatypeList::CDatatypeList (std::initializer_list<DWORD> List)

//	CDatatypeList constructor

	{
	m_Types.GrowToFit((int)List.size());

	for (DWORD dwID : List)
		{
		CDatum dType = CAEONTypes::Get(dwID);
		if (dType.IsNil())
			throw CException(errFail);

		m_Types.Insert(dType);
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

void CDatatypeList::DebugDump () const
	{
	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		printf("Type %d: %s\n", i, (LPSTR)m_Types[i].AsString());
		}
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
		if (!CComplexDatatype::CreateFromStream(Stream, Result.m_Types[i]))
			return false;
		}

	retList = std::move(Result);
	return true;
	}

bool CDatatypeList::DeserializeAEON (IByteStream& Stream, CAEONSerializedMap &Serialized, CDatatypeList& retList)
	{
	CDatatypeList Result;

	DWORD dwCount = Stream.ReadDWORD();
	Result.m_Types.InsertEmpty(dwCount);

	for (int i = 0; i < Result.m_Types.GetCount(); i++)
		{
		Result.m_Types[i] = CDatum::DeserializeAEON(Stream, Serialized);
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

void CDatatypeList::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	Write each type.

	Stream.Write(m_Types.GetCount());
	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		m_Types[i].SerializeAEON(Stream, Serialized);
		}
	}
