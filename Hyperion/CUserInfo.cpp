//	CUserInfo.cpp
//
//	CUserInfo class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CUserInfo::HasRights (const CAttributeList &Rights) const

//	HasRights
//
//	Returns TRUE if we have all the rights listed

	{
	int i;

	TArray<CString> All;
	Rights.GetAll(&All);

	for (i = 0; i < All.GetCount(); i++)
		if (!HasRight(All[i]))
			return false;

	//	If we get this far then we have all the rights

	return true;
	}
