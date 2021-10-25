//	CHexeSecurityCtx.cpp
//
//	CHexeSecurityCtx class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin")

CDatum CHexeSecurityCtx::AsDatum (void) const

//	AsDatum
//
//	Represents as a datum

	{
	CComplexArray *pArray = new CComplexArray;

	//	Rights

	CDatum dItem;
	CDatum::CreateFromAttributeList(m_ServiceRights, &dItem);
	pArray->Insert(dItem);

	//	Sandbox

	pArray->Insert(m_sSandbox);

	//	Username

	if (!m_sUsername.IsEmpty())
		{
		pArray->Insert(m_sUsername);

		CDatum::CreateFromAttributeList(m_UserRights, &dItem);
		pArray->Insert(dItem);
		}

	//	Done

	return CDatum(pArray);
	}

bool CHexeSecurityCtx::HasServiceRightArcAdmin (void) const

//	HasServiceRightArcAdmin
//
//	Do we have admin rights?

	{
	return HasServiceRight(RIGHT_ARC_ADMIN);
	}

bool CHexeSecurityCtx::HasUserRights (const CAttributeList &Rights) const

//	HasUserRights
//
//	Returns TRUE if the user has ALL of the given rights.

	{
	int i;

	TArray<CString> All;
	Rights.GetAll(&All);

	for (i = 0; i < All.GetCount(); i++)
		if (!HasUserRight(All[i]))
			return false;

	//	If we get this far then we have all the rights

	return true;
	}

void CHexeSecurityCtx::Init (CDatum dDatum)

//	Init
//
//	Initializes from a datum

	{
	SetServiceRights(dDatum.GetElement(0));
	m_sSandbox = dDatum.GetElement(1);

	if (dDatum.GetCount() >= 4)
		{
		m_sUsername = dDatum.GetElement(2);
		SetUserRights(dDatum.GetElement(3));
		}
	else
		SetAnonymous();
	}

void CHexeSecurityCtx::SetServiceRightsArcAdmin ()

//	SetServiceRightsArcAdmin
//
//	Sets Arc.admin rights.

	{
	m_ServiceRights.DeleteAll();
	InsertServiceRight(RIGHT_ARC_ADMIN);
	}

void CHexeSecurityCtx::SetServiceSecurity (const CHexeSecurityCtx &SecurityCtx)

//	SetServiceSecurity
//
//	Initializes service security

	{
	m_ServiceRights = SecurityCtx.m_ServiceRights;
	m_sSandbox = SecurityCtx.m_sSandbox;
	}

void CHexeSecurityCtx::SetUserSecurity (const CHexeSecurityCtx &SecurityCtx)

//	SetUserSecurity
//
//	Initializes user security

	{
	m_sUsername = SecurityCtx.m_sUsername;
	m_UserRights = SecurityCtx.m_UserRights;
	m_dwExecutionRights = SecurityCtx.m_dwExecutionRights;
	}
