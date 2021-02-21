//	CDBColumnDef.cpp
//
//	CDBColumnDef class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CDBColumnDef::CDBColumnDef (const CString &sID, CDBValue::ETypes iType, int iOrdinal, int iSize)

//	CDBColumnDef constructor

	{
	m_sID = GenerateID(sID);
	m_sName = sID;
	m_iType = iType;

	m_iOrdinal = iOrdinal;
	m_iSize = iSize;

	m_iDisplayType = iType;
	m_sDisplayName = sID;
	}

CString CDBColumnDef::GenerateID (const CString &sValue)

//	GenerateID
//
//	Creates an ID from the given value.

	{
	CString sID = sValue;
	char *pPos = sID.GetParsePointer();
	while (*pPos != '\0')
		{
		if (strIsWhitespace(pPos))
			*pPos = '_';
		else if (strIsASCIISymbol(pPos))
			*pPos = '$';

		pPos++;
		}

	return strToLower(sID);
	}

void CDBColumnDef::SetID (const CString &sID)

//	SetID
//
//	Sets the column ID.

	{
	m_sID = strToLower(sID);
	m_sName = sID;
	}

