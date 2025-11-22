//	CAEONSerializedMap.cpp
//
//	CAEONSerializedMap class
//	Copyright (c) 2023 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CAEONSerializedMap::CAEONSerializedMap ()

//	CAEONSerializedMap constructor

	{
	m_Map.GrowToFit(1000);

	//	Fill in the 0th element so that we can use 0 as a sentinel value.
	m_Map.Insert(CDatum());
	}

DWORD CAEONSerializedMap::Add (CDatum dValue)

//	Add
//
//	Add the value to the map and return the ID.
	{
	DWORD dwID = m_Map.GetCount();
	m_Map.Insert(dValue);

	m_ReverseMap[dValue.m_dwData] = dwID;

	return dwID;
	}

void CAEONSerializedMap::Add (DWORD dwID, CDatum dValue)

//	Add
//
//	Adds the value by ID.

	{
	if ((int)dwID >= m_Map.GetCount())
		{
		m_Map.InsertEmpty(dwID - m_Map.GetCount() + 1);
		}

	m_Map[dwID] = dValue;
	}

DWORD CAEONSerializedMap::AddIfNew (CDatum dValue, bool* retbNew)

//	AddIfNew
//
//	If the given value does not exist in the map, we add it and return the ID.
//	Otherwise, we return the existing ID.

	{
	auto it = m_ReverseMap.find(dValue.m_dwData);

	//	If not found, then we insert it.

	if (it == m_ReverseMap.end())
		{
		DWORD dwID = m_Map.GetCount();
		m_Map.Insert(dValue);
		m_ReverseMap.insert({ dValue.m_dwData, dwID });

		if (retbNew) *retbNew = true;
		return dwID;
		}

	//	Otherwise, we return the value.

	else
		{
		if (retbNew) *retbNew = false;
		return it->second;
		}
	}

bool CAEONSerializedMap::Find (CDatum dValue, DWORD* retdwID) const

//	Find
//
//	Returns TRUE if the value exists in the map.
	
	{
	auto it = m_ReverseMap.find(dValue.m_dwData);
	if (it == m_ReverseMap.end())
		return false;

	if (retdwID) *retdwID = it->second;
	return true;
	}

CDatum CAEONSerializedMap::Get (DWORD dwID) const

//	Get
//
//	Get a value by ID.

	{
	if ((int)dwID < m_Map.GetCount())
		return m_Map[dwID];
	else
		return CDatum();
	}

bool CAEONSerializedMap::WriteID (IByteStream& Stream, CDatum dValue, DWORD dwType)

//	WriteID
//
//	Write out either a reference or the signature of the value.

	{
	bool bNew;
	DWORD dwID = AddIfNew(dValue, &bNew);
	if (!bNew)
		Stream.Write(CDatum::SERIALIZE_TYPE_REF | dwID);
	else
		Stream.Write(dwType | dwID);

	return bNew;
	}

