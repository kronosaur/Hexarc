//	CHexarcMsgService.cpp
//
//	CHexarcMsgService class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(LIBRARY_SESSION,					"session")

bool CHexarcMsgService::CreateServiceClass (CDatum dServiceDef, const CHexeDocument &Package, IHyperionService **retpService, CString *retsError)

//	CreateServiceClass
//
//	Create the actual service class

	{
	*retpService = new CHexarcMsgService;
	return true;
	}

bool CHexarcMsgService::OnInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError)

//	OnInit
//
//	Initialize

	{
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

void CHexarcMsgService::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	m_ProcessTemplate.Mark();
	}
