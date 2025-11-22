//	CDBFormatXLS97SST.cpp
//
//	CDBFormatXLS97SST class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CDBFormatXLS97SST::AddString (const CString &sValue)

//	AddString
//
//	Adds a string to the shared string table.

	{
	bool bNew;
	int *pIndex = m_Index.SetAt(sValue, &bNew);
	if (bNew)
		{
		*pIndex = m_SST.GetCount();

		SEntry *pNew = m_SST.Insert();
		pNew->sValue = sValue;
		}

	m_iTotalStringInstances++;
	}

int CDBFormatXLS97SST::GetStringIndex (const CString &sValue) const

//	GetStringIndex
//
//	Returns the index.

	{
	int *pIndex = m_Index.GetAt(sValue);
	if (!pIndex)
		throw CException(errFail);

	return *pIndex;
	}
