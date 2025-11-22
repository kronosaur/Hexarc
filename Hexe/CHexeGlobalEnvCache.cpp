//	CHexeGlobalEnvCache.cpp
//
//	CHexeGlobalEnvCache class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CHexeGlobalEnvCache::SetID (int iIndex, DWORD dwID)

//	SetID
//
//	Sets the ID for the given index

	{
	if (iIndex >= m_IDs.GetCount())
		{
		int iOldCount = m_IDs.GetCount();
		m_IDs.InsertEmpty(iIndex - m_IDs.GetCount() + 1);

		for (int i = iOldCount; i < iIndex; i++)
			m_IDs[i] = INVALID_ID;
		}

	m_IDs[iIndex] = dwID;
	}
