//	CDatumStringTable.cpp
//
//	CDatumStringTable class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatumStringTable::CDatumStringTable (int iCount)
	{
	if (iCount > 0)
		{
		m_StringToID.reserve(iCount + 1);
		m_Strings.GrowToFit(iCount + 1);
		}

	//	Add a null value as index 0. This allows us to use 0 as a sentinel value for "no string".

	m_Strings.Insert(CDatum());

	//	Reserve

	m_StringToID.max_load_factor(0.77f);
	m_StringToID.emplace(CDatum(), 0);
	}

DWORD CDatumStringTable::AddString (CDatum dString)
	{
	auto found = m_StringToID.find(dString);
	if (found != m_StringToID.end())
		return found->second;

	if (m_Strings.GetCount() == MAXINT32)
		throw CException(errFail);

	m_Strings.Insert(dString);
	DWORD dwID = (DWORD)(m_Strings.GetCount() - 1);
	m_StringToID.emplace(dString, dwID);

	return dwID;
	}

DWORD CDatumStringTable::GetIDByIndex (int iIndex) const
	{
	if (iIndex < 0 || iIndex >= m_Strings.GetCount())
		throw CException(errFail);

	return (DWORD)iIndex;
	}

CDatum CDatumStringTable::GetStringByIndex (int iIndex) const
	{
	if (iIndex < 0 || iIndex >= m_Strings.GetCount())
		throw CException(errFail);

	return m_Strings[iIndex];
	}

CDatum CDatumStringTable::GetString (DWORD dwID) const
	{
	if ((int)dwID >= m_Strings.GetCount())
		throw CException(errFail);

	return m_Strings[(int)dwID];
	}

void CDatumStringTable::GrowToFit (int iCount)
	{
	m_Strings.GrowToFit(iCount);
	m_StringToID.reserve(m_StringToID.size() + (size_t)iCount);
	}

