//	CDBColumnDef.cpp
//
//	CDBColumnDef class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CDBColumnDef::CDBColumnDef (const CString &sID, CDBValue::ETypes iType, int iOrdinal)

//	CDBColumnDef constructor

	{
	m_sID = strToLower(sID);
	m_sName = sID;
	m_iType = iType;

	m_iOrdinal = iOrdinal;

	m_iDisplayType = iType;
	m_sDisplayName = sID;
	}
