//	CServicePermissions.cpp
//
//	CServicePermissions class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ACCESS_ALL,						"all");
DECLARE_CONST_STRING(ACCESS_NONE,						"none");

DECLARE_CONST_STRING(ERR_UNKNOWN_ACCESS,				"Unknown allowAccess type: %s.");

bool CServicePermissions::CanAccess (const CHexeSecurityCtx *pSecurityCtx) const

//	CanAccess
//
//	Returns TRUE if a package with the given security context can access us.

	{
	switch (m_iAccess)
		{
		case EServicePermission::accessAll:
			return true;

		case EServicePermission::accessNone:
		case EServicePermission::accessWhitelist:
			{
			//	NULL means that we're being called by a privileged process, so
			//	we allow access.

			if (pSecurityCtx == NULL)
				return true;

			//	If we're a service with admin rights, then we always allow 
			//	access.

			CString sSandbox = pSecurityCtx->GetSandboxName();
			if (pSecurityCtx->HasServiceRightArcAdmin() || sSandbox.IsEmpty())
				return true;

			//	If the service trying to access us is us, then we allow access.

			if (strEquals(sSandbox, m_sPackageName))
				return true;

			//	If the service is in our whitelist, allow it.

			if (m_List.Find(sSandbox))
				return true;

			//	Access denied.

			return false;
			}

		default:
			return false;
		}
	}

bool CServicePermissions::InitFromDatum (const CString &sPackageName, CDatum dDesc, CString *retsError)

//	InitFromDatum
//
//	Initializes from a datum

	{
	m_sPackageName = sPackageName;

	//	Default to no access

	if (dDesc.IsNil())
		{
		m_iAccess = EServicePermission::accessNone;
		return true;
		}

	//	If this is an array, then we expect a whitelist of packages.

	else if (dDesc.GetBasicType() == CDatum::typeArray)
		{
		m_iAccess = EServicePermission::accessWhitelist;

		for (int i = 0; i < dDesc.GetCount(); i++)
			{
			const CString &sPackage = dDesc.GetElement(i);
			if (sPackage.IsEmpty())
				continue;

			m_List.Insert(sPackage);
			}
		}

	//	Otherwise we expect a constant

	else
		{
		const CString &sValue = dDesc;
		if (strEqualsNoCase(sValue, ACCESS_NONE))
			m_iAccess = EServicePermission::accessNone;
		else if (strEqualsNoCase(sValue, ACCESS_ALL))
			m_iAccess = EServicePermission::accessAll;
		else
			{
			if (retsError)
				*retsError = strPattern(ERR_UNKNOWN_ACCESS, sValue);
			return false;
			}
		}

	return true;
	}
