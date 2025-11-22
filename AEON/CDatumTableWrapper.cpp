//	CDatumTableWrapper.cpp
//
//	CDatumTableWrapper class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CDatumTableWrapper::IsElementEqual (int iIndex, const CDatumTableWrapper& Src, int iSrcIndex) const

//	IsElementEqual
//
//	Returns TRUE if the element at the given index is equal to the element at 
//	the given index in the source array.

	{
	if (Src.m_pTable->GetColCount() != m_pTable->GetColCount())
		return false;

	for (int i = 0; i < m_pTable->GetColCount(); i++)
		{
		if (m_pTable->GetFieldValue(iIndex, i) != Src.m_pTable->GetFieldValue(iSrcIndex, i))
			return false;
		}

	return true;
	}
