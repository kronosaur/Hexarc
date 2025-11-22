//	CAI1Service.cpp
//
//	CAI1Service class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_INTERFACE,					"interface")
DECLARE_CONST_STRING(FIELD_REQUIRED_RIGHTS,				"rightsRequired")

DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")

DECLARE_CONST_STRING(PROTOCOL_AI1,						"ai1")

DECLARE_CONST_STRING(ERR_INVALID_INTERFACE,				"Invalid interface name.")
DECLARE_CONST_STRING(ERR_INTERFACE_NOT_SANDBOXED,		"Interface name is not available in sandbox: %s.")

CAI1Service *CAI1Service::AsAI1Service (IHyperionService *pService)

//	AsAI1Service
//
//	Casts a service to a CAI1Service (based on the protocol)

	{
	if (pService == NULL)
		return NULL;

	const CString &sProtocol = pService->GetProtocol();
	if (strEquals(sProtocol, PROTOCOL_AI1))
		return (CAI1Service *)pService;
	else
		return NULL;
	}

bool CAI1Service::CreateServiceClass (CDatum dServiceDef, const CHexeDocument &Package, IHyperionService **retpService, CString *retsError)

//	Creates the appropriate service class

	{
	*retpService = new CAI1Service;
	return true;
	}

bool CAI1Service::OnInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError)

//	OnInit
//
//	Initialize the service

	{
	//	Get the interface

	m_sInterface = dServiceDef.GetElement(FIELD_INTERFACE).AsStringView();
	if (m_sInterface.IsEmpty())
		{
		*retsError = strPattern(ERR_INVALID_INTERFACE);
		return false;
		}

	//	Load rights required

	dServiceDef.GetElement(FIELD_REQUIRED_RIGHTS).AsAttributeList(&m_MinUserRights);

	//	For now we do not allow anonymous access

	m_bAllowAnonymousAccess = false;

	//	If we're sandboxed then the interface must start with the sandbox prefix

	if (!IsAccessible(m_sInterface))
		{
		*retsError = strPattern(ERR_INTERFACE_NOT_SANDBOXED, m_sInterface);
		return false;
		}

	//	Initialize the process template

	m_ProcessTemplate.SetSecurityCtx(GetSecurityCtx());

	if (!m_ProcessTemplate.LoadStandardLibraries(retsError))
		return false;

	if (!m_ProcessTemplate.LoadLibrary(LIBRARY_SESSION, retsError))
		return false;

	if (!m_ProcessTemplate.LoadEntryPoints(Package, retsError))
		return false;

	if (!m_ProcessTemplate.LoadHexeDefinitions(Package, retsError))
		return false;

	//	Done

	return true;
	}

void CAI1Service::OnMark (void)

//	OnMark
//
//	Mark data in user

	{
	m_ProcessTemplate.Mark();
	}
